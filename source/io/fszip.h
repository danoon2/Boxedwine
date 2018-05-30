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
    static bool init(const std::string& zipPath);
    static unzFile zipfile;
};
#endif