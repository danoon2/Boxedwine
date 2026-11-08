/*
 * Copyright (C) 2012-2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "boxedwine.h"
#ifdef __TEST
#include "devcdrom.h"
#include "kstat.h"
#include "../../io/fsiso.h"
#undef OF
#include "../../../lib/zlib/zlib.h"
#include "../cpu/testCPU.h"

namespace {
#include "testUdfFixture.h"
#include "testUdfSessionFixture.h"
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

std::vector<U8> udfImage(bool hybrid = false) {
    std::vector<U8> image(UDF_FIXTURE_SIZE);
    uLongf length = (uLongf)image.size();
    if (uncompress(image.data(), &length, udfFixtureCompressed, sizeof(udfFixtureCompressed)) != Z_OK ||
        length != image.size()) testFail("could not decompress UDF fixture");
    if (!hybrid) {
        memset(image.data() + 16 * SECTOR, 0, 16 * SECTOR);
        memcpy(image.data() + 16 * SECTOR, "\0BEA01\1", 7);
        memcpy(image.data() + 17 * SECTOR, "\0NSR02\1", 7);
        memcpy(image.data() + 18 * SECTOR, "\0TEA01\1", 7);
    }
    return image;
}

U32 udfU32(const U8* p) {
    return p[0] | ((U32)p[1] << 8) | ((U32)p[2] << 16) | ((U32)p[3] << 24);
}

void udfNumber(U8* p, U64 value, U32 bytes = 4) {
    for (U32 i = 0; i < bytes; ++i) p[i] = (U8)(value >> (i * 8));
}

void udfTag(U8* descriptor, U32 length) {
    U16 crc = 0;
    for (U32 i = 16; i < length; ++i) {
        crc ^= (U16)descriptor[i] << 8;
        for (U32 bit = 0; bit < 8; ++bit)
            crc = (U16)((crc << 1) ^ ((crc & 0x8000) ? 0x1021 : 0));
    }
    udfNumber(descriptor + 8, crc, 2);
    udfNumber(descriptor + 10, length - 16, 2);
    descriptor[4] = 0;
    for (U32 i = 0; i < 16; ++i) if (i != 4) descriptor[4] += descriptor[i];
}
}

void testUdfFilesAndDiscChanges() {
    DiscFixture fixture;
    for (bool hybrid : {false, true}) {
        BString path = fixture.write("udf.iso", udfImage(hybrid));
        cdromInit({path}, B("/mnt/cdrom"), B(""));
        if (!cdromInsert(0)) { testFail("UDF fixture did not mount"); return; }
        auto disc = getCdromForPath(B("/mnt/cdrom"));
        if (!disc || disc->filesystemType() != 0x15013346) testFail("UDF statfs type incorrect");
        auto node = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/DIR/caf\xc3\xa9.txt"), true);
        auto empty = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/EMPTY"), true);
        if (!node || !empty) { testFail("UDF Unicode/empty file missing"); return; }
        std::unique_ptr<FsOpenNode> file(node->open(K_O_RDONLY));
        expectDiscContents(file.get(), node, "HELLO");
        std::unique_ptr<FsOpenNode> emptyFile(empty->open(K_O_RDONLY));
        U8 bytes[16];
        if (!emptyFile || emptyFile->length() || emptyFile->readNative(bytes, sizeof(bytes)))
            testFail("empty UDF file read failed");
        for (U32 flags : {K_O_WRONLY, K_O_RDWR}) {
            errno = 0;
            std::unique_ptr<FsOpenNode> denied(node->open(flags));
            if (denied || errno != EACCES) testFail("UDF must reject write access");
        }
        if (!file) return;
        if (file->seek(100) != 100 || file->readNative(bytes, 5) ||
            file->seek(-1) != -K_EINVAL || file->getFilePointer() != 100)
            testFail("UDF past-EOF/negative seek semantics incorrect");
        if (file->seek(2) != 2 || file->readNative(bytes, 5) != 3 || memcmp(bytes, "LLO", 3))
            testFail("UDF read after seek failed");
        BString replacement = fixture.write("replacement.iso", isoImage());
        if (!cdromInsertPath(replacement)) testFail("ISO could not replace UDF disc");
        expectDiscContents(file.get(), node, "HELLO");
        std::unique_ptr<FsOpenNode> reopened(node->open(K_O_RDONLY));
        expectDiscContents(reopened.get(), node, "HELLO");
        cdromEject();
        expectDiscContents(file.get(), node, "HELLO");
    }
}

void testUdfSessionOffsets() {
    DiscFixture fixture;
    fixture.write("audio.bin", std::vector<U8>(UDF_SESSION_FIXTURE_START * 2352));
    const char cue[] = "FILE \"audio.bin\" BINARY\n  TRACK 01 AUDIO\n    INDEX 01 00:00:00\n"
        "FILE \"session.iso\" BINARY\n  TRACK 02 MODE1/2048\n    INDEX 01 00:00:00\n";
    BString path = fixture.write("session.cue", std::vector<U8>(cue, cue + sizeof(cue) - 1));
    for (U32 variant = 0; variant < 3; ++variant) {
        std::vector<U8> image(UDF_SESSION_FIXTURE_SIZE);
        uLongf length = image.size();
        if (uncompress(image.data(), &length, udfSessionFixtureCompressed,
                sizeof(udfSessionFixtureCompressed)) != Z_OK || length != image.size()) {
            testFail("could not decompress UDF session fixture"); return;
        }
        if (variant == 1) {
            // The primary anchor is relative to the data session.
            image[256 * SECTOR + 4] ^= 1;
        } else if (variant == 2) {
            // Backup anchors are relative to the absolute end of the disc.
            image[image.size() - SECTOR + 4] ^= 1;
            image[image.size() - 257 * SECTOR + 4] ^= 1;
        }
        fixture.write("session.iso", image);
        cdromInit({path}, B("/mnt/cdrom"), B(""));
        if (!cdromInsert(0)) { testFail("UDF session variant %u did not mount", variant); return; }
        auto disc = getCdromForPath(B("/mnt/cdrom"));
        if (!disc || disc->filesystemType() != 0x15013346)
            testFail("session image fell back to ISO instead of mounting UDF");
        auto node = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/FILE.TXT"), true);
        std::unique_ptr<FsOpenNode> file(node ? node->open(K_O_RDONLY) : nullptr);
        U8 bytes[8];
        if (!file || file->seek(2046) != 2046 || file->readNative(bytes, 4) != 4 || memcmp(bytes, "AABB", 4))
            testFail("UDF session data addresses were not absolute disc LBAs");
        cdromEject();
    }
}

void testUdfAllocationAndLargeFiles() {
    DiscFixture fixture;
    for (U32 variant = 0; variant < 3; ++variant) {
        auto image = udfImage();
        U8* fe = image.data() + UDF_FIXTURE_FILE_ENTRY * SECTOR;
        U32 dataLba = udfU32(fe + 180);
        if (variant == 0) {
            // Three non-contiguous extents; bytes straddle two extent boundaries.
            udfNumber(fe + 172, 24);
            for (U32 i = 0; i < 3; ++i) {
                udfNumber(fe + 176 + i * 8, i == 2 ? 1 : SECTOR);
                udfNumber(fe + 180 + i * 8, dataLba + (i == 1 ? 2 : i == 2 ? 1 : 0));
            }
            memset(image.data() + (UDF_FIXTURE_PARTITION + dataLba + 2) * SECTOR, 'B', SECTOR);
            image[(UDF_FIXTURE_PARTITION + dataLba + 1) * SECTOR] = 'C';
            udfTag(fe, 200);
        } else if (variant == 1) {
            udfNumber(fe + 34, 3, 2); // embedded allocation data
            udfNumber(fe + 56, 5, 8);
            udfNumber(fe + 172, 5);
            memcpy(fe + 176, "FIRST", 5);
            udfTag(fe, 181);
        } else {
            // A sparse file over 4 GiB without allocating a multi-gigabyte fixture.
            udfNumber(fe + 56, (1ull << 32) + 5, 8);
            udfNumber(fe + 172, 48);
            for (U32 i = 0; i < 5; ++i) {
                udfNumber(fe + 176 + i * 8, 0x80000000u | (i == 4 ? 8192 : 0x3ffff800));
                udfNumber(fe + 180 + i * 8, 0);
            }
            udfNumber(fe + 216, 5);
            udfNumber(fe + 220, dataLba);
            memcpy(image.data() + (UDF_FIXTURE_PARTITION + dataLba) * SECTOR, "FIRST", 5);
            udfTag(fe, 224);
        }
        BString path = fixture.write("allocation.iso", image);
        cdromInit({path}, B("/mnt/cdrom"), B(""));
        if (!cdromInsert(0)) { testFail("UDF allocation variant %u did not mount", variant); return; }
        auto node = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/FILE.TXT"), true);
        std::unique_ptr<FsOpenNode> file(node ? node->open(K_O_RDONLY) : nullptr);
        if (!file) { testFail("UDF allocation file missing"); return; }
        U8 bytes[16];
        if (variant == 0) {
            file->seek(2046);
            if (file->readNative(bytes, 4) != 4 || memcmp(bytes, "AABB", 4)) testFail("first UDF extent boundary failed");
            file->seek(4094);
            if (file->readNative(bytes, 8) != 3 || memcmp(bytes, "BBC", 3)) testFail("last UDF extent boundary failed");
        } else if (variant == 1) {
            expectDiscContents(file.get(), node, "FIRST");
        } else {
            if (node->length() != (1ull << 32) + 5 || file->length() != (1ll << 32) + 5)
                testFail("UDF large file size truncated");
            file->seek((1ll << 32) - 4);
            if (file->readNative(bytes, 16) != 9 || memcmp(bytes, "\0\0\0\0FIRST", 9))
                testFail("UDF read across 4 GiB failed");
        }
        cdromEject();
    }
}

void testMalformedUdfImages() {
    DiscFixture fixture;
    BString good = fixture.write("good.iso", udfImage());
    cdromInit({good}, B("/mnt/cdrom"), B(""));
    if (!cdromInsert(0)) { testFail("valid UDF did not mount"); return; }
    auto before = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/EMPTY"), true);
    for (U32 variant = 0; variant < 8; ++variant) {
        auto image = udfImage();
        U8* root = image.data() + UDF_FIXTURE_ROOT_ENTRY * SECTOR;
        if (variant == 0) {
            root[4] ^= 1; // bad descriptor tag checksum
        } else if (variant == 1) {
            U8* fe = image.data() + UDF_FIXTURE_FILE_ENTRY * SECTOR;
            udfNumber(fe + 168, 0xfffffff0); // overflowing extended-attribute size
            udfTag(fe, 184);
        } else if (variant == 2) {
            // A valid prefix followed by a truncated directory record.
            udfNumber(root + 56, udfU32(root + 56) - 1, 8);
            udfTag(root, 184);
        } else if (variant == 3) {
            // A subdirectory points back to the root directory's data.
            U8* child = image.data() + UDF_FIXTURE_DIR_ENTRY * SECTOR;
            udfNumber(child + 56, udfU32(root + 56), 8);
            memcpy(child + 176, root + 176, 8);
            udfTag(child, 184);
        } else if (variant == 4) {
            U8* fsd = image.data() + UDF_FIXTURE_FSD * SECTOR;
            udfNumber(fsd + 400, 0x3fffffff); // excessive metadata allocation
            udfTag(fsd, 512);
        } else if (variant == 5) {
            U8* fe = image.data() + UDF_FIXTURE_FILE_ENTRY * SECTOR;
            udfNumber(fe + 180, 0xfffffff0); // allocation outside the image
            udfTag(fe, 184);
        } else if (variant == 6) {
            U8* fe = image.data() + UDF_FIXTURE_FILE_ENTRY * SECTOR;
            udfNumber(fe + 176, 0xc0000800); // allocation continuation
            udfNumber(fe + 180, 300 - UDF_FIXTURE_PARTITION);
            udfTag(fe, 184);
            U8* continuation = image.data() + 300 * SECTOR;
            memset(continuation, 0, SECTOR);
            udfNumber(continuation, 258, 2);
            udfNumber(continuation + 2, 2, 2);
            udfNumber(continuation + 12, 300 - UDF_FIXTURE_PARTITION);
            udfNumber(continuation + 20, 8);
            memcpy(continuation + 24, fe + 176, 8); // points to itself
            udfTag(continuation, 32);
        } else {
            // Cyclic volume descriptor sequences, including the reserve copy.
            U8* anchor = image.data() + 256 * SECTOR;
            for (U32 offset : {20u, 28u}) {
                U32 lba = udfU32(anchor + offset);
                U8* descriptor = image.data() + lba * SECTOR;
                memset(descriptor, 0, SECTOR);
                udfNumber(descriptor, 3, 2);
                udfNumber(descriptor + 2, 2, 2);
                udfNumber(descriptor + 12, lba);
                udfNumber(descriptor + 20, SECTOR);
                udfNumber(descriptor + 24, lba);
                udfTag(descriptor, 28);
            }
        }
        BString bad = fixture.write("bad.iso", image);
        if (cdromInsertPath(bad)) testFail("malformed UDF variant %u accepted", variant);
        auto after = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/EMPTY"), true);
        if (after != before || cdromGetState().discs.size() != 1)
            testFail("failed UDF insert changed the current disc");
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
    // Wine must distinguish denied CD writes from a missing file so legacy
    // callers can retry read-only instead of requesting another disc.
    for (U32 flags : {K_O_WRONLY, K_O_RDWR}) {
        errno = ENOENT;
        std::unique_ptr<FsOpenNode> denied(node->open(flags));
        if (denied || errno != EACCES)
            testFail("write access to an ISO file must report access denied, not stale errno");
    }
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

    // Runtime selection validates the entire tree before ejecting. Exercise a
    // failure after one valid file, not just a missing file or an invalid PVD.
    auto malformed = isoImage();
    isoRecord(malformed.data() + ROOT + 112, 20, SECTOR, "LOOP", 4, true);
    BString invalid = fixture.write("invalid.iso", malformed);
    if (cdromInsertPath(invalid) || cdromInsertPath(fixture.root + "/missing.iso"))
        testFail("runtime picker accepted a malformed/missing image");
    auto preserved = Fs::getNodeFromLocalPath(B(""), B("/mnt/cdrom/FILE.TXT"), true);
    if (preserved != current || cdromGetState().index != 1 || cdromGetState().discs.size() != 2)
        testFail("failed runtime insert changed the current disc or list");
    if (preserved) {
        std::unique_ptr<FsOpenNode> preservedFile(preserved->open(K_O_RDONLY));
        expectDiscContents(preservedFile.get(), preserved, "OTHER");
    }
    if (cdromRemoveDisc(1) || !cdromRemoveDisc(0) || cdromGetState().index != 0)
        testFail("disc list removal did not protect/reindex the current disc");
    if (!cdromInsertPath(first) || cdromGetState().discs.size() != 2 || cdromGetState().index != 1)
        testFail("runtime picker could not add a new image");
    if (!cdromInsertPath(first) || cdromGetState().discs.size() != 2)
        testFail("runtime picker duplicated an existing image");
    cdromEject();
    if (cdromGetState().index != -1) testFail("snapshot did not reflect eject");
    cdromNextDisc();
    if (cdromGetState().index != 0) testFail("next disc after eject did not wrap");

    cdromInit({}, B("/mnt/cdrom"), B(""));
    if (cdromGetState().index != -1 || !cdromInsertPath(second) || cdromGetState().index != 0)
        testFail("could not insert the first image into a drive initialized empty");
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
