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
    char tmp[4096];

    if (zipOffset != lastZipOffset || zipFileOffset < lastZipFileOffset) {
        unzCloseCurrentFile(this->zipfile);
        unzSetOffset64(this->zipfile, zipOffset);
        lastZipFileOffset = 0;
        unzOpenCurrentFile(this->zipfile);
        lastZipOffset = zipOffset;
    }
    if (zipFileOffset != lastZipFileOffset) {
        U32 todo = (U32)(zipFileOffset - lastZipFileOffset);
        while (todo) {
            todo-=unzReadCurrentFile(this->zipfile, tmp, todo>4096?4096:todo);
        }
    }  
#endif
}

bool FsZip::init(const std::string& zipPath, const std::string& mount) {
#ifdef BOXEDWINE_ZLIB
    std::string strippedMount;

    if (mount.length()) {
        Fs::makeLocalDirs(mount);
        strippedMount = mount.substr(0, mount.length() - 1);
    }
    this->lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
    if (zipPath.length()) {
        unz_global_info global_info;
        U32 i;
        fsZipInfo* zipInfo;

        this->zipfile = unzOpen(zipPath.c_str());
        if (!this->zipfile) {
            klog("Could not load zip file: %s", zipPath.c_str());
        }

        if (unzGetGlobalInfo( this->zipfile, &global_info ) != UNZ_OK) {
            klog("Could not read file global info from zip file: %s", zipPath.c_str());
            unzClose( this->zipfile );
            return false;
        }
        zipInfo = new fsZipInfo[global_info.number_entry];
        for (i = 0; i < global_info.number_entry; ++i) {
            unz_file_info file_info;
            struct tm tm={0};
            char tmp[MAX_FILEPATH_LEN];

            zipInfo[i].filename="/";
            if ( unzGetCurrentFileInfo(this->zipfile, &file_info, tmp, MAX_FILEPATH_LEN, NULL, 0, NULL, 0 ) != UNZ_OK ) {
                klog("Could not read file info from zip file: %s", zipPath.c_str());
                unzClose( zipfile );
                return false;
            }
            zipInfo[i].filename.append(tmp);
            zipInfo[i].offset = unzGetOffset64(this->zipfile);
            Fs::remoteNameToLocal(zipInfo[i].filename); // converts special characters like :
            if (stringHasEnding(zipInfo[i].filename, ".link")) {
                U32 read;

                zipInfo[i].filename.resize(zipInfo[i].filename.length()-5);
                zipInfo[i].isLink = true;
                unzOpenCurrentFile(zipfile);
                read = unzReadCurrentFile(this->zipfile, tmp, MAX_FILEPATH_LEN);
                tmp[read]=0;
                zipInfo[i].link = tmp;                
                unzCloseCurrentFile(this->zipfile);
            }                       
            
            if (stringHasEnding(zipInfo[i].filename, "/")) {
                zipInfo[i].filename.resize(zipInfo[i].filename.length()-1);
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
        for (i = 0; i < global_info.number_entry; ++i) {
            std::string localZipPart = zipInfo[i].filename;
            Fs::remoteNameToLocal(localZipPart);
            std::string localPath = strippedMount + localZipPart;
            std::string parentPath = Fs::getParentPath(localPath);
            BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", parentPath, true);            
            std::string localFileName = Fs::getFileNameFromPath(localPath);
            std::string nativePath = Fs::getNativePathFromParentAndLocalFilename(parent, localFileName);
            BoxedPtr<FsFileNode> node = (FsFileNode*)Fs::addFileNode(localPath, zipInfo[i].link, nativePath, zipInfo[i].isDirectory, parent).get();
            std::shared_ptr<FsZip> thisShared = shared_from_this();
            node->zipNode = std::make_shared<FsZipNode>(zipInfo[i], thisShared);
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

bool FsZip::readFileFromZip(const std::string& zipFile, const std::string& file, std::string& result) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info;
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

        if ( unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, NULL, 0, NULL, 0 ) != UNZ_OK ) {
            unzClose( z );
            return false;
        }
        
        if (file == tmp) {
            U32 read;

            unzOpenCurrentFile(z);            
            read = unzReadCurrentFile(z, tmp, MAX_FILEPATH_LEN);
            tmp[read]=0;              
            unzCloseCurrentFile(z);
            unzClose(z);
            result = tmp;
            return true;
        }
        unzGoToNextFile(z);
    }
    unzClose(z);
    return false;
}

bool FsZip::extractFileFromZip(const std::string& zipFile, const std::string& file, const std::string& path) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info;
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

        if ( unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, NULL, 0, NULL, 0 ) != UNZ_OK ) {
            unzClose( z );
            return false;
        }
        
        if (file == tmp) {
            unzOpenCurrentFile(z);            
            if (!Fs::doesNativePathExist(path)) {
                Fs::makeNativeDirs(path);
            }
            std::string outPath = path+Fs::nativePathSeperator+Fs::getFileNameFromPath(file);
            FILE* f = fopen(outPath.c_str(), "wb");
            if (f) {
                U32 totalRead = 0;
                U8 buffer[4096];

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

bool FsZip::iterateFiles(const std::string& zipFile, std::function<void(const std::string&)> it) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info;
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

        if (unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, NULL, 0, NULL, 0) != UNZ_OK) {
            unzClose(z);
            return false;
        }
        it(tmp);
        unzGoToNextFile(z);
    }
    unzClose(z);
    return true;
}

std::string FsZip::unzip(const std::string& zipFile, const std::string& path, std::function<void(U32, std::string fileName)> percentDone) {
    unzFile z = unzOpen(zipFile.c_str());
    unz_global_info global_info;
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

        if (unzGetCurrentFileInfo(z, &file_info, tmp, MAX_FILEPATH_LEN, NULL, 0, NULL, 0) != UNZ_OK) {
            unzClose(z);
            return "Could not read file info from zip file: "+zipFile;
        }
        std::string fileName = tmp;
        if (stringHasEnding(fileName, "/")) {
            std::string dirPath = path + Fs::nativePathSeperator + fileName;
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
            stringReplaceAll(fileName, "/", Fs::nativePathSeperator);
        }
        unzOpenCurrentFile(z);        
        percentDone((U32)(compressedFileSizeProcessed * 100 / fileSize), fileName);
        std::string outPath = path + Fs::nativePathSeperator + fileName;
        FILE* f = fopen(outPath.c_str(), "wb");
        if (f) {
            U32 totalRead = 0;
            U8 buffer[4096];

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
    return "";
}
#endif
