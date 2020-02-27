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
    std::string filename;
    std::string link;
    bool isLink;
    bool isDirectory;
    U64 length;
    U64 lastModified;
    U64 offset;
};

class FsZip {
public:
    bool init(const std::string& zipPath, const std::string& mount);
    unzFile zipfile;

    U64 lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
    U64 lastZipFileOffset;

    void setupZipRead(U64 zipOffset, U64 zipFileOffset);

    static bool readFileFromZip(const std::string& zipFile, const std::string& file, std::string& result);
};
#endif
#endif
