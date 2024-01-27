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
    fsZipInfo() : isLink(false), isDirectory(false), length(0), lastModified(0), offset(0) {}
    BString filename;
    BString link;
    bool isLink;
    bool isDirectory;
    U64 length;
    U64 lastModified;
    U64 offset;
};

class FsZip : public std::enable_shared_from_this<FsZip> {
public:
    ~FsZip();
    bool init(BString zipPath, BString mount);
    unzFile zipfile;

    U64 lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
    U64 lastZipFileOffset;

    BOXEDWINE_MUTEX readMutex;

    void setupZipRead(U64 zipOffset, U64 zipFileOffset);
    void remove(BString localPath);

    static bool readFileFromZip(BString zipFile, BString file, BString& result);
    static bool extractFileFromZip(BString zipFile, BString file, BString path);
    static BString unzip(BString zipFile, BString path, std::function<void(U32, BString)> percentDone);
    static bool iterateFiles(BString zipFile, std::function<void(BString)> it);

private:
    BString deleteFilePath;
};
#endif
#endif
