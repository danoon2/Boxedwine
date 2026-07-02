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

static std::shared_ptr<FsIso> gActiveCdrom;

// cdrom_drive_status return values
#define CDS_NO_DISC     1
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

class DevCDROM : public FsVirtualOpenNode {
public:
    DevCDROM(const std::shared_ptr<FsNode>& node, U32 flags, std::shared_ptr<FsIso> iso)
        : FsVirtualOpenNode(node, flags), iso(std::move(iso)), pos(0) {}

    U32  ioctl(KThread* thread, U32 request) override;
    U32  readNative(U8* buffer, U32 len) override;
    U32  writeNative(U8* buffer, U32 len) override { return 0; }
    S64  length() override { return iso ? (S64)(iso->totalSectors * ISO_SECTOR_SIZE) : 0; }
    S64  getFilePointer() override { return pos; }
    S64  seek(S64 p) override { pos = p; return pos; }

private:
    std::shared_ptr<FsIso> iso;
    S64 pos;
};

U32 DevCDROM::ioctl(KThread* thread, U32 request) {
    CPU* cpu = thread->cpu;
    KMemory* memory = thread->memory;

    switch (request) {
    case 0x5326: // CDROM_DRIVE_STATUS
        return iso ? CDS_DISC_OK : CDS_NO_DISC;

    case 0x5327: // CDROM_DISC_STATUS
        return iso ? CDS_DATA_1 : CDS_NO_DISC;

    case 0x5331: // CDROM_GET_CAPABILITY
        return CDC_CLOSE_TRAY | CDC_OPEN_TRAY | CDC_DRIVE_STATUS | CDC_CD_R | CDC_DVD;

    case 0x5320: // CDROM_SET_OPTIONS
    case 0x5321: // CDROM_CLEAR_OPTIONS
        return 0;

    case 0x5305: { // CDROMREADTOCHDR — writes struct cdrom_tochdr (2 bytes)
        if (!iso)
            return (U32)-K_ENODEV;
        U32 p = IOCTL_ARG1;
        memory->writeb(p,     1); // cdth_trk0: first track
        memory->writeb(p + 1, 1); // cdth_trk1: last track
        return 0;
    }

    case 0x5306: { // CDROMREADTOCENTRY — reads/writes struct cdrom_tocentry
        if (!iso)
            return (U32)-K_ENODEV;
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
            return (U32)-K_ENODEV;
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
    if (!iso)
        return 0;
    U32 result = iso->readData((U64)pos, buffer, len);
    pos += result;
    return result;
}

FsOpenNode* openDevCDROM(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevCDROM(node, flags, gActiveCdrom);
}

void setCdromIso(const std::shared_ptr<FsIso>& iso) {
    gActiveCdrom = iso;
}
