/*
 * Copyright (C) 2012-2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "boxedwine.h"
#ifdef __TEST
#include "devcdrom.h"
#include "kstat.h"
#include "../cpu/testCPU.h"

namespace {
constexpr U32 SECTOR = 2048;
constexpr U32 ROOT = 20 * SECTOR;

void isoNumber(U8* target, U32 value) {
    for (U32 i = 0; i < 4; ++i) {
        target[i] = (U8)(value >> (8 * i));
        target[7 - i] = target[i];
    }
}

U32 isoRecord(U8* target, U32 lba, U32 length, const char* name, U32 nameLength, bool directory) {
    U32 size = (33 + nameLength + 1) & ~1u;
    memset(target, 0, size);
    target[0] = (U8)size;
    isoNumber(target + 2, lba);
    isoNumber(target + 10, length);
    target[25] = directory ? 2 : 0;
    target[28] = target[31] = 1;
    target[32] = (U8)nameLength;
    memcpy(target + 33, name, nameLength);
    return size;
}

std::vector<U8> isoImage(const char* contents = "FIRST") {
    std::vector<U8> image(23 * SECTOR, 0);
    U8* pvd = image.data() + 16 * SECTOR;
    memcpy(pvd, "\001CD001\001", 7);
    isoNumber(pvd + 80, 23);
    pvd[128] = pvd[131] = 0;
    pvd[129] = pvd[130] = 8;
    isoRecord(pvd + 156, 20, SECTOR, "\0", 1, true);
    memcpy(image.data() + 17 * SECTOR, "\377CD001\001", 7);
    U8* root = image.data() + ROOT;
    U32 offset = isoRecord(root, 20, SECTOR, "\0", 1, true);
    offset += isoRecord(root + offset, 20, SECTOR, "\1", 1, true);
    offset += isoRecord(root + offset, 21, 5, "FILE.TXT;1", 10, false);
    isoRecord(root + offset, 22, SECTOR, "DIR", 3, true);
    isoRecord(image.data() + 22 * SECTOR, 22, SECTOR, "\0", 1, true);
    memcpy(image.data() + 21 * SECTOR, contents, 5);
    return image;
}

struct DiscFixture {
    const BString root = B("tmp/test-cdrom-root");
    DiscFixture() {
        cdromEject();
        KSystem::setProcNode(nullptr);
        Fs::shutDown();
        Fs::deleteNativeDirAndAllFilesInDir(root);
        std::filesystem::create_directories(root.c_str());
        Fs::initFileSystem(root);
        Fs::makeLocalDirs(B("/mnt"));
    }
    ~DiscFixture() {
        cdromEject();
        Fs::shutDown();
        Fs::deleteNativeDirAndAllFilesInDir(root);
    }
    BString write(const char* name, const std::vector<U8>& image) {
        BString path = root + "/" + name;
        FILE* file = fopen(path.c_str(), "wb");
        if (!file) {
            testFail("could not create CD-ROM fixture %s", path.c_str());
            return path;
        }
        if (fwrite(image.data(), 1, image.size(), file) != image.size())
            testFail("could not write CD-ROM fixture %s", path.c_str());
        fclose(file);
        return path;
    }
};

void expectDiscContents(FsOpenNode* file, const std::shared_ptr<FsNode>& node, const char* expected) {
    if (!file || file->node != node) {
        testFail("opening a resolved CD-ROM file changed its identity");
        return;
    }
    U8 bytes[5];
    if (file->seek(0) != 0 || file->readNative(bytes, 5) != 5 || memcmp(bytes, expected, 5))
        testFail("CD-ROM read did not return the resolved disc's contents");
}
}

void testIsoFilesAndDiscChanges() {
    DiscFixture fixture;
    BString first = fixture.write("first.iso", isoImage());
    BString second = fixture.write("second.iso", isoImage("OTHER"));
    cdromInit({first, second}, B("/mnt/cdrom"), B(""));
    if (!cdromInsert(0)) {
        testFail("valid ISO did not mount");
        return;
    }
    auto node = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/FILE.TXT"), true);
    auto directory = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/DIR"), true);
    if (!node || !directory) {
        testFail("ISO directory entries missing");
        return;
    }
    if (node->getType(true) != 8 || directory->getType(true) != 4)
        testFail("ISO getdents types must be DT_REG and DT_DIR");
    std::unique_ptr<FsOpenNode> file(node->open(K_O_RDONLY));
    expectDiscContents(file.get(), node, "FIRST");
    if (!file) return;
    U8 bytes[5];
    if (file->seek(100) != 100 || file->readNative(bytes, 5) != 0 ||
            file->seek(-1) != -K_EINVAL || file->getFilePointer() != 100)
        testFail("ISO seek must allow past EOF and reject negative positions without moving");
    if (file->seek(2) != 2 || file->readNative(bytes, 5) != 3 || memcmp(bytes, "RST", 3))
        testFail("ISO seek back from past EOF did not restore reading");

    // Deterministically reproduce an eject between pathname resolution and open.
    cdromEject();
    expectDiscContents(file.get(), node, "FIRST");
    std::unique_ptr<FsOpenNode> ejected(node->open(K_O_RDONLY));
    expectDiscContents(ejected.get(), node, "FIRST");
    std::unique_ptr<FsOpenNode> ejectedDir(directory->open(K_O_RDONLY));
    if (!ejectedDir || ejectedDir->node != directory)
        testFail("opening a resolved directory after eject lost its identity");

    if (!cdromInsert(1)) {
        testFail("replacement ISO did not mount");
        return;
    }
    std::unique_ptr<FsOpenNode> replaced(node->open(K_O_RDONLY));
    expectDiscContents(replaced.get(), node, "FIRST");
    auto current = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/FILE.TXT"), true);
    if (!current || current == node) {
        testFail("disc swap did not replace the directory entry");
        return;
    }
    std::unique_ptr<FsOpenNode> currentFile(current->open(K_O_RDONLY));
    expectDiscContents(currentFile.get(), current, "OTHER");
}

void testMalformedIsoDirectories() {
    DiscFixture fixture;
    for (U32 variant = 0; variant < 6; ++variant) {
        auto image = isoImage();
        U8* root = image.data() + ROOT;
        if (variant == 0) {
            // The reported heap overread: a short record at the end of a sector.
            memset(root, 0, SECTOR);
            for (U32 offset = 0; offset < 2040; offset += 255) {
                root[offset] = 255;
                root[offset + 32] = 1;
            }
            root[2040] = 7;
            root[2047] = 1;
        } else if (variant == 1) {
            root[68 + 32] = 255; // filename extends beyond its record
        } else if (variant == 2) {
            root[68 + 32] = 0; // empty filename
        } else if (variant == 3) {
            image.resize(ROOT + 100); // only part of the advertised extent is readable
        } else if (variant == 4) {
            isoRecord(root + 112, 20, SECTOR, "LOOP", 4, true);
        } else {
            // A record may not straddle logical sectors, even in a larger extent.
            isoNumber(image.data() + 16 * SECTOR + 166, 2 * SECTOR);
            memset(root, 0, 2 * SECTOR);
            for (U32 offset = 0; offset < 2040; offset += 255) {
                root[offset] = 255;
                root[offset + 32] = 1;
            }
            isoRecord(root + 2040, 21, 5, "BAD", 3, false);
        }
        BString path = fixture.write("malformed.iso", image);
        cdromInit({path}, B("/mnt/cdrom"), B(""));
        if (cdromInsert(0)) {
            testFail("malformed ISO variant %u was accepted", variant);
            cdromEject();
        }
        auto mount = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom"), true);
        if (!mount || mount->getChildCount())
            testFail("malformed ISO variant %u left a partial mounted directory", variant);
    }
}
#endif
