/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "boxedwine.h"
#include "fsudf.h"
#include "fsiso.h"
#include "fsisoopennode.h"
#include "discimage.h"
#include "kstat.h"
#include "../../lib/libudfread/src/udfread.h"

namespace {
// Library handles remain attached to their original disc after an eject. The
// shared owner also keeps the sector source and its read mutex alive.
class FsUdfOpenNode : public FsIsoOpenNode {
public:
    FsUdfOpenNode(const std::shared_ptr<FsNode>& node, U32 flags,
        const std::shared_ptr<FsIso>& disc, UDFFILE* file, U64 size)
        : FsIsoOpenNode(node, flags, 0, size, disc), disc(disc), file(file), size(size) {}
    ~FsUdfOpenNode() override {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(disc->readMutex);
        udfread_file_close(file);
    }
    U32 readNative(U8* buffer, U32 len) override {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(disc->readMutex);
        U64 position = (U64)getFilePointer();
        if (position >= size || !len) return 0;
        U32 count = (U32)std::min<U64>(std::min<U32>(len, 0x7fffffff), size - position);
        if (udfread_file_seek(file, (S64)position, UDF_SEEK_SET) != (S64)position)
            return -K_EIO;
        auto read = udfread_file_read(file, buffer, count);
        if (read < 0) return -K_EIO;
        seek((S64)position + read);
        return (U32)read;
    }
private:
    std::shared_ptr<FsIso> disc;
    UDFFILE* file;
    U64 size;
};

bool validName(const BString& name) {
    // libudfread encodes embedded NUL as MUTF-8 C0 80. Reject it, separators,
    // and traversal components rather than creating ambiguous host paths.
    return !name.isEmpty() && name != B(".") && name != B("..") &&
        name.length() <= MAX_FILEPATH_LEN && name.indexOf('/') < 0 &&
        name.indexOf('\\') < 0 && strstr(name.c_str(), "\xc0\x80") == nullptr;
}
}

FsUdf::FsUdf(FsIso& disc) : disc(disc) {
    input.close = nullptr;
    input.disc = &disc;
    input.size = [](udfread_block_input* source) -> uint32_t {
        return static_cast<Input*>(source)->disc->image->leadOut;
    };
    input.read = [](udfread_block_input* source, uint32_t lba, void* buffer,
        uint32_t count, int) -> int {
        auto& in = *static_cast<Input*>(source);
        // Metadata parsing must terminate even when a malformed descriptor
        // chain points to itself. Data reads don't use this mount-time budget.
        if (!in.remainingReads || !count || count > 0x7fffffff / DISC_DATA_SECTOR_SIZE ||
            (U64)lba + count > in.disc->image->leadOut) return 0;
        if (in.remainingReads != 0xffffffff) --in.remainingReads;
        return (int)(in.disc->readData((U64)lba * DISC_DATA_SECTOR_SIZE,
            (U8*)buffer, count * DISC_DATA_SECTOR_SIZE) / DISC_DATA_SECTOR_SIZE);
    };
}

FsUdf::~FsUdf() {
    udfread_close(reader);
}

bool FsUdf::probe(DiscImage& image, U32 start) {
    U8 sector[DISC_DATA_SECTOR_SIZE];
    bool extendedArea = false;
    for (U64 lba = (U64)start + 16; lba < (U64)start + 256; ++lba) {
        if (image.readData(lba * sizeof(sector), sector, sizeof(sector)) != sizeof(sector))
            return false;
        if (!memcmp(sector, "\0\0\0\0\0\0\0", 7) || !memcmp(sector + 1, "TEA01", 5))
            return false;
        if (!memcmp(sector, "\0BEA01\1", 7)) extendedArea = true;
        if (sector[0] == 0 && sector[6] == 1 &&
            extendedArea && (!memcmp(sector + 1, "NSR02", 5) || !memcmp(sector + 1, "NSR03", 5))) return true;
    }
    return false;
}

bool FsUdf::mount(const BString& path, const std::shared_ptr<FsNode>& root, U32 sessionStart) {
    input.remainingReads = 1000000;
    reader = udfread_init();
    if (!reader || udfread_open_input_at(reader, &input, sessionStart) < 0) return false;
    std::unique_ptr<UDFDIR, decltype(&udfread_closedir)> directory(
        udfread_opendir(reader, "/"), udfread_closedir);
    if (!directory || !scanDirectory(directory.get(), path, B(""), root, 0)) return false;
    input.remainingReads = 0xffffffff;
    klog_fmt("FsIso: mounted UDF volume %s (%u entries)",
        udfread_get_volume_id(reader) ? udfread_get_volume_id(reader) : "", entries);
    return true;
}

bool FsUdf::scanDirectory(UDFDIR* directory, const BString& path,
    const BString& udfPath, const std::shared_ptr<FsNode>& parent, U32 depth) {
    if (depth >= 64) return false;
    udfread_dirent entry;
    while (udfread_readdir(directory, &entry)) {
        if (++entries > 100000) return false;
        BString name = BString::copy(entry.d_name);
        if (!validName(name) || parent->getChildByName(name)) return false;
        BString childPath = path + "/" + name;
        BString childUdfPath = udfPath + "/" + name;
        if (childPath.length() > MAX_FILEPATH_LEN) return false;
        if (entry.d_type == UDF_DT_DIR) {
            std::unique_ptr<UDFDIR, decltype(&udfread_closedir)> child(
                udfread_opendir_at(directory, entry.d_name), udfread_closedir);
            if (!child) return false;
            auto node = Fs::addVirtualDirectory(childPath, K__S_IFDIR | 0555, parent, 0);
            if (!node || !scanDirectory(child.get(), childPath, childUdfPath, node, depth + 1)) return false;
        } else if (entry.d_type == UDF_DT_REG) {
            std::unique_ptr<UDFFILE, decltype(&udfread_file_close)> file(
                udfread_file_openat(directory, entry.d_name), udfread_file_close);
            if (!file) return false;
            S64 length = udfread_file_size(file.get());
            if (length < 0) return false;
            auto owner = disc.shared_from_this();
            Fs::addVirtualFile(childPath,
                [owner, childUdfPath, length, this](const std::shared_ptr<FsNode>& node, U32 flags, U32) -> FsOpenNode* {
                    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(owner->readMutex);
                    // Reopening parses allocation descriptors; bound that work
                    // separately from ordinary file content reads.
                    input.remainingReads = 65536;
                    UDFFILE* opened = udfread_file_open(reader, childUdfPath.c_str());
                    input.remainingReads = 0xffffffff;
                    if (!opened) { errno = EIO; return nullptr; }
                    return new FsUdfOpenNode(node, flags, owner, opened, (U64)length);
                }, K__S_IFREG | 0555, 0, parent, 0, (U64)length, 0);
        } else {
            return false;
        }
    }
    return true;
}
