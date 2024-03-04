#ifdef BOXEDWINE_ZLIB

#ifndef __FSZIP_H__
#define __FSZIP_H__

#include "platform.h"

#undef OF
#define STRICTUNZIP
extern "C"
{
#include "../../lib/zlib/contrib/minizip/unzip.h"
}
#undef OF

class fsZipInfo {
public:
    fsZipInfo() = default;
    BString filename;
    BString link;
    bool isLink = false;
    bool isDirectory = false;
    U64 length = 0;
    U64 lastModified = 0;
    U64 offset = 0;
};

class FsZip : public std::enable_shared_from_this<FsZip> {
public:
    FsZip() = default;
    ~FsZip();
    bool init(BString zipPath, BString mount);
    unzFile zipfile = nullptr;

    U64 lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
    U64 lastZipFileOffset = 0;

    BOXEDWINE_MUTEX readMutex;

    void setupZipRead(U64 zipOffset, U64 zipFileOffset);
    void remove(BString localPath);

    static bool readFileFromZip(BString zipFile, BString file, BString& result);
    static bool extractFileFromZip(BString zipFile, BString file, BString path);
    static BString unzip(BString zipFile, BString path, std::function<void(U32, BString)> percentDone);
    static bool iterateFiles(BString zipFile, std::function<void(BString)> it);
    static bool doesFileExist(BString zipFile, BString file);
private:
    BString deleteFilePath;
};
#endif
#endif
