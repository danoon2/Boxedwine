#include "boxedwine.h"
#ifdef BOXEDWINE_ZLIB
#undef OF
#define STRICTUNZIP
extern "C"
{
    #include "../../lib/zlib/contrib/minizip/unzip.h"
}
#include "fsfilenode.h"
#include "fszip.h"
#include "fszipnode.h"
#include <time.h> 

void FsZip::setupZipRead(U64 zipOffset, U64 zipFileOffset) {
#ifdef BOXEDWINE_ZLIB    
    if (zipOffset != lastZipOffset || zipFileOffset < lastZipFileOffset) {
        unzCloseCurrentFile(this->zipfile);
        unzSetOffset64(this->zipfile, zipOffset);
        lastZipFileOffset = 0;
        unzOpenCurrentFile(this->zipfile);
        lastZipOffset = zipOffset;
    }
    if (zipFileOffset != lastZipFileOffset) {
        char tmp[4096] = {};
        U32 todo = (U32)(zipFileOffset - lastZipFileOffset);
        while (todo) {
            todo-=unzReadCurrentFile(this->zipfile, tmp, todo>4096?4096:todo);
        }
    }  
#endif
}

bool FsZip::init(BString zipPath, BString mount) {
#ifdef BOXEDWINE_ZLIB
    BString strippedMount;

    std::shared_ptr<FsNode> root = Fs::getNodeFromLocalPath(B(""), B(""), true);
    deleteFilePath = root->nativePath ^ Fs::getFileNameFromNativePath(zipPath) + ".deleted";
    if (mount.length()) {
        Fs::makeLocalDirs(mount);
        strippedMount = mount.substr(0, mount.length() - 1);
    }
    this->lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
    if (zipPath.length()) {
        unz_global_info global_info = {};

        this->zipfile = unzOpen(zipPath.c_str());
        if (!this->zipfile) {
            klog("Could not load zip file: %s", zipPath.c_str());
        }

        if (unzGetGlobalInfo( this->zipfile, &global_info ) != UNZ_OK) {
            klog("Could not read file global info from zip file: %s", zipPath.c_str());
            unzClose( this->zipfile );
            return false;
        }
        fsZipInfo* zipInfo = new fsZipInfo[global_info.number_entry];
        for (U32 i = 0; i < global_info.number_entry; ++i) {
            unz_file_info file_info = {};
            struct tm tm={0};
            char tmp[MAX_FILEPATH_LEN];

            tmp[0] = '/';
            if ( unzGetCurrentFileInfo(this->zipfile, &file_info, tmp + 1, MAX_FILEPATH_LEN - 1, nullptr, 0, nullptr, 0 ) != UNZ_OK ) {
                klog("Could not read file info from zip file: %s", zipPath.c_str());
                delete[] zipInfo;
                unzClose( zipfile );
                return false;
            }
            zipInfo[i].filename = BString::copy(tmp);
            zipInfo[i].offset = unzGetOffset64(this->zipfile);
            Fs::remoteNameToLocal(zipInfo[i].filename); // converts special characters like :

            if (zipInfo[i].filename.endsWith("/")) {
                zipInfo[i].filename = zipInfo[i].filename.substr(0, zipInfo[i].filename.length() - 1);
                zipInfo[i].isDirectory = true;
            } else {
                zipInfo[i].length = file_info.uncompressed_size;
            }               
            tm.tm_sec = file_info.tmu_date.tm_sec;
            tm.tm_min = file_info.tmu_date.tm_min;
            tm.tm_hour = file_info.tmu_date.tm_hour;
            tm.tm_mday = file_info.tmu_date.tm_mday;
            tm.tm_mon = file_info.tmu_date.tm_mon;
            tm.tm_year = file_info.tmu_date.tm_year;
            if (tm.tm_year>1900)
                tm.tm_year-=1900;

            zipInfo[i].lastModified = ((U64)mktime(&tm))*1000l;
            
            unzGoToNextFile(this->zipfile);
        }
        std::vector<BString> deletedLocalPaths;
        readLinesFromFile(deleteFilePath, deletedLocalPaths);

        for (U32 i = 0; i < global_info.number_entry; ++i) {
            if (zipInfo[i].filename.endsWith(EXT_LINK)) {
                char tmp[MAX_FILEPATH_LEN];
                zipInfo[i].filename = zipInfo[i].filename.substr(0, zipInfo[i].filename.length() - 5);
                zipInfo[i].isLink = true;
                unzSetOffset64(zipfile, zipInfo[i].offset);
                unzOpenCurrentFile(zipfile);
                U32 read = unzReadCurrentFile(this->zipfile, tmp, MAX_FILEPATH_LEN);
                tmp[read] = 0;
                zipInfo[i].link = BString::copy(tmp);
                unzCloseCurrentFile(this->zipfile);
            }
            BString localZipPart = zipInfo[i].filename;
            Fs::remoteNameToLocal(localZipPart);
            BString localPath = strippedMount + localZipPart;
            if (vectorIndexOf(deletedLocalPaths, localPath) != -1) {
                continue;
            }
            BString parentPath = Fs::getParentPath(localPath);
            std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), parentPath, true);
            if (!parent) {
                Fs::makeLocalDirs(parentPath);
                parent = Fs::getNodeFromLocalPath(B(""), parentPath, true);
            }
            BString localFileName = Fs::getFileNameFromPath(localPath);
            BString nativePath = Fs::getNativePathFromParentAndLocalFilename(parent, localFileName);
            std::shared_ptr<FsFileNode> node = Fs::addFileNode(localPath, zipInfo[i].link, nativePath, zipInfo[i].isDirectory, parent);
            node->zipNode = std::make_shared<FsZipNode>(zipInfo[i], shared_from_this());
        }   
        delete[] zipInfo;
    }
#endif
    return true;
}

FsZip::~FsZip() {
#ifdef BOXEDWINE_ZLIB
    unzClose(this->zipfile);
#endif
}

void FsZip::remove(BString localPath) {
    std::vector<BString> lines;
    readLinesFromFile(deleteFilePath, lines);
    if (vectorIndexOf(lines, localPath) == -1) {
        lines.push_back(localPath);
        writeLinesToFile(deleteFilePath, lines);
    }
}

bool FsZip::doesFileExist(BString zipFile, BString file) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info = {};
    if (!z) {
        return false;
    }
    if (unzGetGlobalInfo(z, &global_info) != UNZ_OK) {
        unzClose(z);
        return false;
    }
    for (U32 i = 0; i < global_info.number_entry; ++i) {
        unz_file_info file_info;
        char tmp[MAX_FILEPATH_LEN];

        if (unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzClose(z);
            return false;
        }

        if (file == tmp) {
            unzCloseCurrentFile(z);
            unzClose(z);
            return true;
        }
        unzGoToNextFile(z);
    }
    unzClose(z);
    return false;
}

bool FsZip::readFileFromZip(BString zipFile, BString file, BString& result) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info = {};
    if (!z) {
        return false;
    }
    if (unzGetGlobalInfo( z, &global_info ) != UNZ_OK) {
        unzClose( z );
        return false;
    }
    for (U32 i = 0; i < global_info.number_entry; ++i) {
        unz_file_info file_info;
        char tmp[MAX_FILEPATH_LEN];

        if ( unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, nullptr, 0, nullptr, 0 ) != UNZ_OK ) {
            unzClose( z );
            return false;
        }
        
        if (file == tmp) {
            char* buffer = new char[file_info.uncompressed_size+1];
            unzOpenCurrentFile(z);            
            U32 read = unzReadCurrentFile(z, buffer, (unsigned)file_info.uncompressed_size);
            buffer[read]=0;
            unzCloseCurrentFile(z);
            unzClose(z);
            result = BString::copy(buffer);
            delete[] buffer;
            return true;
        }
        unzGoToNextFile(z);
    }
    unzClose(z);
    return false;
}

bool FsZip::extractFileFromZip(BString zipFile, BString file, BString path) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info = {};
    if (!z) {
        return false;
    }
    if (unzGetGlobalInfo( z, &global_info ) != UNZ_OK) {
        unzClose( z );
        return false;
    }
    for (U32 i = 0; i < global_info.number_entry; ++i) {
        unz_file_info file_info;
        char tmp[MAX_FILEPATH_LEN];

        if ( unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, nullptr, 0, nullptr, 0 ) != UNZ_OK ) {
            unzClose( z );
            return false;
        }
        
        if (file == tmp) {
            unzOpenCurrentFile(z);            
            if (!Fs::doesNativePathExist(path)) {
                Fs::makeNativeDirs(path);
            }
            BString outPath = path ^ Fs::getFileNameFromPath(file);
            FILE* f = fopen(outPath.c_str(), "wb");
            if (f) {
                U32 totalRead = 0;
                U8 buffer[4096] = {};

                while (totalRead<file_info.uncompressed_size) {
                    U32 read = unzReadCurrentFile(z, buffer, sizeof(buffer));
                    if (!read) {
                        break;
                    }
                    totalRead += read;
                    fwrite(buffer, read, 1, f);
                }
                fclose(f);
                unzCloseCurrentFile(z);
                unzClose(z);
                return totalRead==file_info.uncompressed_size;
            } else {
                unzCloseCurrentFile(z);
                unzClose(z);
                return false;
            }
        }
        unzGoToNextFile(z);
    }
    unzClose(z);
    return false;
}

bool FsZip::iterateFiles(BString zipFile, std::function<void(BString)> it) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info = {};
    if (!z) {
        return false;
    }

    if (unzGetGlobalInfo(z, &global_info) != UNZ_OK) {
        unzClose(z);
        return false;
    }

    for (U32 i = 0; i < global_info.number_entry; ++i) {
        unz_file_info file_info;
        char tmp[MAX_FILEPATH_LEN];

        if (unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzClose(z);
            return false;
        }
        it(BString::copy(tmp));
        unzGoToNextFile(z);
    }
    unzClose(z);
    return true;
}

BString FsZip::unzip(BString zipFile, BString path, std::function<void(U32, BString fileName)> percentDone) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info = {};
    if (!z) {
        return "Could not open zip file: " + zipFile;
    }
    U64 fileSize = Fs::getNativeFileSize(zipFile);
    U64 compressedFileSizeProcessed = 0;

    if (unzGetGlobalInfo(z, &global_info) != UNZ_OK) {
        unzClose(z);
        return "Could not read file global info from zip file: "+ zipFile;
    }
    if (!Fs::doesNativePathExist(path)) {
        if (!Fs::makeNativeDirs(path)) {
            return "Could not create directory: " + path + "\n\n" + strerror(errno);
        }
    }
    for (U32 i = 0; i < global_info.number_entry; ++i) {
        unz_file_info file_info;
        char tmp[MAX_FILEPATH_LEN];

        if (unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, nullptr, 0, nullptr, 0) != UNZ_OK) {
            unzClose(z);
            return "Could not read file info from zip file: "+zipFile;
        }
        BString fileName = BString::copy(tmp);
        if (fileName.endsWith("/")) {
            BString dirPath = path ^ fileName;
            if (!Fs::doesNativePathExist(dirPath)) {
                if (!Fs::makeNativeDirs(dirPath)) {
                    unzClose(z);
                    return "Could not create directory: " + dirPath + "\n\n" + strerror(errno);
                }
            }
            unzGoToNextFile(z);
            continue;
        }
        if (Fs::nativePathSeperator != "/") {
            fileName = fileName.replace("/", Fs::nativePathSeperator);
        }
        unzOpenCurrentFile(z);        
        percentDone((U32)(compressedFileSizeProcessed * 100 / fileSize), fileName);
        BString outPath = path ^ fileName;
        FILE* f = fopen(outPath.c_str(), "wb");
        if (f) {
            U32 totalRead = 0;
            U8 buffer[4096] = {};

            while (totalRead < file_info.uncompressed_size) {
                U32 read = unzReadCurrentFile(z, buffer, sizeof(buffer));
                if (!read) {
                    break;
                }
                totalRead += read;
                fwrite(buffer, read, 1, f);
            }
            fclose(f);
            unzCloseCurrentFile(z);
            compressedFileSizeProcessed += file_info.compressed_size;            
        } else {
            unzCloseCurrentFile(z);
            unzClose(z);
            return "Could not create file: " + outPath + "\n\n" + strerror(errno);
        }
        unzGoToNextFile(z);
    }
    unzClose(z);
    return B("");
}
#endif
