/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "../../io/fsvirtualopennode.h"
#include "../../io/fsiso.h"
#include "../../io/fs.h"
#include "kerror.h"
#include "kstat.h"
#include "devcdrom.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

// cdrom_drive_status return values
#define CDS_NO_DISC     1
#define CDS_TRAY_OPEN   2
#define CDS_DISC_OK     4

// cdrom_disc_status return values
#define CDS_DATA_1      101

// CDROM_GET_CAPABILITY flags
#define CDC_CLOSE_TRAY      0x001
#define CDC_OPEN_TRAY       0x002
#define CDC_DRIVE_STATUS    0x800
#define CDC_CD_R            0x2000
#define CDC_DVD             0x8000

// cdsc_audiostatus values for CDROMSUBCHNL
#define CDROM_AUDIO_NO_STATUS 0x15

// Address format for CDROMREADTOCENTRY / CDROMSUBCHNL
#define CDROM_LBA 0x01
#define CDROM_MSF 0x02

// Special track number for lead-out in CDROMREADTOCENTRY
#define CDROM_LEADOUT 0xAA

// Frames per second for MSF addressing
#define CD_FRAMES_PER_SEC  75
#define CD_SECS_PER_MIN    60

static struct {
    BOXEDWINE_MUTEX mutex;
    std::vector<BString> discs;
    BString mountPath;
    BString driveLink;
    std::shared_ptr<FsIso> iso; // the disc in the drive, null while the tray is open
    S32 index = -1;             // which disc that is
    U32 lastInserted = 0;       // the disc in the tray, inserted again when it closes
    bool mediaChanged = false;  // for CDROM_MEDIA_CHANGED, cleared when read
} gDrive;

static std::shared_ptr<FsIso> getDisc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
    return gDrive.iso;
}

// Lets the browser page keep its disc picker in step with ejects and inserts,
// including those the guest asks for.
static void notifyHost(S32 index) {
#ifdef __EMSCRIPTEN__
#ifdef BOXEDWINE_MULTI_THREADED
    MAIN_THREAD_ASYNC_EM_ASM({
#else
    EM_ASM({
#endif
        if (typeof boxedwineCDRomChanged === "function") {
            boxedwineCDRomChanged($0);
        }
    }, index);
#endif
}

// Reads go to whichever disc is in the drive when they happen, like a real drive:
// an open /dev/sr0 reads the new disc after a swap and fails while the tray is open.
class DevCDROM : public FsVirtualOpenNode {
public:
    DevCDROM(const std::shared_ptr<FsNode>& node, U32 flags)
        : FsVirtualOpenNode(node, flags), pos(0) {}

    U32  ioctl(KThread* thread, U32 request) override;
    U32  readNative(U8* buffer, U32 len) override;
    U32  writeNative(U8* buffer, U32 len) override { return 0; }
    S64  length() override {
        std::shared_ptr<FsIso> iso = getDisc();
        return iso ? (S64)(iso->totalSectors * ISO_SECTOR_SIZE) : 0;
    }
    S64  getFilePointer() override { return pos; }
    S64  seek(S64 p) override { pos = p; return pos; }

private:
    S64 pos;
};

U32 DevCDROM::ioctl(KThread* thread, U32 request) {
    CPU* cpu = thread->cpu;
    KMemory* memory = thread->memory;
    std::shared_ptr<FsIso> iso = getDisc();

    switch (request) {
    case 0x5326: // CDROM_DRIVE_STATUS
        return iso ? CDS_DISC_OK : CDS_TRAY_OPEN;

    case 0x5325: { // CDROM_MEDIA_CHANGED: 1 once after each insert or eject
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
        bool changed = gDrive.mediaChanged;
        gDrive.mediaChanged = false;
        return changed ? 1 : 0;
    }

    case 0x5309: // CDROMEJECT
        cdromEject();
        return 0;

    case 0x5319: { // CDROMCLOSETRAY
        U32 lastInserted;
        bool empty;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
            lastInserted = gDrive.lastInserted;
            empty = !gDrive.iso;
        }
        if (empty) {
#if defined(__EMSCRIPTEN__) && defined(BOXEDWINE_MULTI_THREADED)
            // Inserting reads the image, which guest threads hand to the browser thread;
            // doing that here could deadlock with the page inserting a disc, so ask the page.
            MAIN_THREAD_ASYNC_EM_ASM({
                if (typeof insertCDRomDisc === "function") {
                    insertCDRomDisc($0);
                }
            }, lastInserted);
#else
            cdromInsert(lastInserted);
#endif
        }
        return 0;
    }

    case 0x5329: // CDROM_LOCKDOOR
    case 0x5307: // CDROMSTOP
    case 0x5308: // CDROMSTART
        return 0;

    case 0x5327: // CDROM_DISC_STATUS
        return iso ? CDS_DATA_1 : CDS_NO_DISC;

    case 0x5331: // CDROM_GET_CAPABILITY
        return CDC_CLOSE_TRAY | CDC_OPEN_TRAY | CDC_DRIVE_STATUS | CDC_CD_R | CDC_DVD;

    case 0x5320: // CDROM_SET_OPTIONS
    case 0x5321: // CDROM_CLEAR_OPTIONS
        return 0;

    case 0x5305: { // CDROMREADTOCHDR — writes struct cdrom_tochdr (2 bytes)
        if (!iso)
            return (U32)-K_ENOMEDIUM;
        U32 p = IOCTL_ARG1;
        memory->writeb(p,     1); // cdth_trk0: first track
        memory->writeb(p + 1, 1); // cdth_trk1: last track
        return 0;
    }

    case 0x5306: { // CDROMREADTOCENTRY — reads/writes struct cdrom_tocentry
        if (!iso)
            return (U32)-K_ENOMEDIUM;
        U32 p    = IOCTL_ARG1;
        U8 track  = memory->readb(p);     // cdte_track (input)
        U8 format = memory->readb(p + 2); // cdte_format: CDROM_LBA or CDROM_MSF (input)

        // cdte_adr:4 (low nibble) = 1 (Q sub-channel with current position)
        // cdte_ctrl:4 (high nibble) = 4 (data track)
        memory->writeb(p + 1, (0x4 << 4) | 0x1);

        U32 lba = (track == CDROM_LEADOUT) ? (U32)iso->totalSectors : 0;

        if (format == CDROM_MSF) {
            // LBA to MSF: add 150-frame (2-second) pregap offset
            U32 msf = lba + 150;
            memory->writeb(p + 4, (U8)(msf / (CD_FRAMES_PER_SEC * CD_SECS_PER_MIN))); // minute
            memory->writeb(p + 5, (U8)((msf / CD_FRAMES_PER_SEC) % CD_SECS_PER_MIN)); // second
            memory->writeb(p + 6, (U8)(msf % CD_FRAMES_PER_SEC));                      // frame
        } else {
            // CDROM_LBA: bytes 4-7 hold a 32-bit signed integer
            memory->writed(p + 4, lba);
        }

        memory->writeb(p + 8, 0x01); // cdte_datamode = Mode 1
        return 0;
    }

    case 0x530b: { // CDROMSUBCHNL — reads/writes struct cdrom_subchnl
        if (!iso)
            return (U32)-K_ENOMEDIUM;
        U32 p = IOCTL_ARG1;
        // cdsc_format is input at p+0 — no need to read it for a static response
        memory->writeb(p + 1, CDROM_AUDIO_NO_STATUS); // cdsc_audiostatus
        memory->writeb(p + 2, (0x4 << 4) | 0x1);     // cdsc_ctrl:4/cdsc_adr:4
        memory->writeb(p + 3, 1);                      // cdsc_trk = 1
        memory->writeb(p + 4, 0);                      // cdsc_ind = 0
        // cdsc_absaddr and cdsc_reladdr (offsets 8 and 12) — both zero (not playing)
        memory->writed(p + 8,  0);
        memory->writed(p + 12, 0);
        return 0;
    }

    case 0x530d: { // CDROM_GET_MCN — writes struct cdrom_mcn { char mcn[14]; }
        U32 p = IOCTL_ARG1;
        for (int i = 0; i < 14; i++)
            memory->writeb(p + i, '0');
        memory->writeb(p + 13, 0);
        return 0;
    }

    default:
        return (U32)-K_ENOTTY;
    }
}

U32 DevCDROM::readNative(U8* buffer, U32 len) {
    std::shared_ptr<FsIso> iso = getDisc();
    if (!iso)
        return 0;
    U32 result = iso->readData((U64)pos, buffer, len);
    pos += result;
    return result;
}

FsOpenNode* openDevCDROM(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevCDROM(node, flags);
}

void cdromInit(const std::vector<BString>& discs, const BString& mountPath, const BString& driveLink) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
    gDrive.discs = discs;
    gDrive.mountPath = mountPath;
    gDrive.driveLink = driveLink;
    // A virtual directory, so it can be emptied on eject whatever the host has at that path
    std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), Fs::getParentPath(mountPath), true);
    if (parent) {
        Fs::addVirtualDirectory(mountPath, K__S_IFDIR | K__S_IREAD | K__S_IEXEC | K__S_IRGRP | K__S_IXGRP | K__S_IROTH | K__S_IXOTH, parent);
    }
}

static void ejectLocked() {
    if (!gDrive.iso) {
        return;
    }
    if (!gDrive.driveLink.isEmpty()) {
        std::shared_ptr<FsNode> link = Fs::getNodeFromLocalPath(B(""), gDrive.driveLink, false);
        if (link) {
            link->removeNodeFromParent();
        }
    }
    std::shared_ptr<FsNode> mountNode = Fs::getNodeFromLocalPath(B(""), gDrive.mountPath, true);
    if (mountNode) {
        std::vector<std::shared_ptr<FsNode> > children;
        mountNode->getAllChildren(children);
        for (auto& child : children) {
            child->removeNodeFromParent();
        }
    }
    // files already open on the disc keep it alive and can still be read
    gDrive.iso = nullptr;
    gDrive.index = -1;
    gDrive.mediaChanged = true;
}

void cdromEject() {
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
        if (!gDrive.iso) {
            return;
        }
        klog_fmt("CD-ROM: ejected disc %d", gDrive.index + 1);
        ejectLocked();
    }
    notifyHost(-1);
}

bool cdromInsert(U32 index) {
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
        if (index >= gDrive.discs.size()) {
            return false;
        }
        if (gDrive.iso && gDrive.index == (S32)index) {
            return true;
        }
        ejectLocked();
        gDrive.lastInserted = index;
        std::shared_ptr<FsIso> iso = std::make_shared<FsIso>();
        if (!iso->init(gDrive.discs[index], gDrive.mountPath)) {
            klog_fmt("CD-ROM: could not insert disc %d: %s", index + 1, gDrive.discs[index].c_str());
            iso = nullptr;
        } else {
            gDrive.iso = iso;
            gDrive.index = (S32)index;
            gDrive.mediaChanged = true;
            if (!gDrive.driveLink.isEmpty()) {
                std::shared_ptr<FsNode> dosdevParent = Fs::getNodeFromLocalPath(B(""), Fs::getParentPath(gDrive.driveLink), true);
                if (dosdevParent) {
                    Fs::addFileNode(gDrive.driveLink, gDrive.mountPath, B(""), false, dosdevParent);
                }
            }
            klog_fmt("CD-ROM: inserted disc %d of %d: %s", index + 1, (U32)gDrive.discs.size(), gDrive.discs[index].c_str());
        }
    }
    S32 inserted;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
        inserted = gDrive.index;
    }
    notifyHost(inserted);
    return inserted == (S32)index;
}

void cdromNextDisc() {
    U32 next;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gDrive.mutex);
        if (gDrive.discs.empty()) {
            return;
        }
        next = (gDrive.lastInserted + 1) % (U32)gDrive.discs.size();
    }
    cdromInsert(next);
}

std::shared_ptr<FsIso> getCdromForPath(const BString& path) {
    std::shared_ptr<FsIso> iso = getDisc();
    if (iso && iso->containsPath(path)) {
        return iso;
    }
    return nullptr;
}

#ifdef __EMSCRIPTEN__
extern "C" {

// Called by the browser page, which fetches each disc image only when it is inserted.
// In multi-threaded builds this runs on the browser thread while guest threads use the
// drive, so all changes go through gDrive.mutex and the FsNode locks.
EMSCRIPTEN_KEEPALIVE
int boxedwineCDRomInsert(int index) {
    return (index >= 0 && cdromInsert((U32)index)) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
void boxedwineCDRomEject() {
    cdromEject();
}

}
#endif
