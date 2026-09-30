/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

void FsZip::resetZipRead() {
    auto first = checkpoints.lower_bound(std::make_pair(lastZipOffset, (U64)0));
    while (first != checkpoints.end() && first->first.first == lastZipOffset) {
        first = checkpoints.erase(first);
    }
    unzCloseCurrentFile(zipfile);
    lastZipOffset = UINT64_MAX;
    lastZipFileOffset = 0;
}

void FsZip::saveCheckpoint() {
    if (!lastZipFileOffset || lastZipFileOffset % checkpointSpan) {
        return;
    }
    auto key = std::make_pair(lastZipOffset, lastZipFileOffset);
    auto found = checkpoints.find(key);
    if (found != checkpoints.end()) {
        found->second.used = ++checkpointClock;
        return;
    }
    unzFileSnapshot state = unzSaveCurrentFile(zipfile);
    if (!state) {
        return; // An index is optional (unsupported compression or allocation failure).
    }
    if (checkpoints.size() == maxCheckpoints) {
        auto oldest = checkpoints.begin();
        for (auto it = checkpoints.begin(); it != checkpoints.end(); ++it) {
            if (it->second.used < oldest->second.used) {
                oldest = it;
            }
        }
        checkpoints.erase(oldest);
    }
    checkpoints.emplace(key, Checkpoint{std::shared_ptr<unz_file_snapshot_s>(state, unzFreeCurrentFileSnapshot), ++checkpointClock});
}

S32 FsZip::readCurrent(U8* buffer, U32 len) {
    // Split even large reads/skips at checkpoint boundaries. Discarded output
    // then builds the same seek index as output returned to a caller.
    U32 count = (U32)std::min<U64>(len, checkpointSpan - lastZipFileOffset % checkpointSpan);
    S32 result = unzReadCurrentFile(zipfile, buffer, count);
    if (result > 0) {
        lastZipFileOffset += result;
        saveCheckpoint();
    }
    return result;
}

bool FsZip::setupZipRead(U64 zipOffset, U64 zipFileOffset) {
    bool currentUsable = zipOffset == lastZipOffset && lastZipFileOffset <= zipFileOffset;
    auto point = checkpoints.upper_bound(std::make_pair(zipOffset, zipFileOffset));
    bool havePoint = point != checkpoints.begin();
    if (havePoint) {
        --point;
        havePoint = point->first.first == zipOffset;
    }
    if (havePoint && (!currentUsable || point->first.second > lastZipFileOffset)) {
        if (unzRestoreCurrentFile(zipfile, point->second.state.get()) != UNZ_OK) {
            resetZipRead();
            return false;
        }
        point->second.used = ++checkpointClock;
        lastZipOffset = zipOffset;
        lastZipFileOffset = point->first.second;
    } else if (!currentUsable) {
        S32 closed = lastZipOffset != UINT64_MAX ? unzCloseCurrentFile(zipfile) : UNZ_OK;
        lastZipOffset = UINT64_MAX;
        lastZipFileOffset = 0;
        if (closed != UNZ_OK || unzSetOffset64(zipfile, zipOffset) != UNZ_OK || unzOpenCurrentFile(zipfile) != UNZ_OK) {
            return false;
        }
        lastZipOffset = zipOffset;
    }
    U8 discard[16384];
    while (lastZipFileOffset < zipFileOffset) {
        U32 count = (U32)std::min<U64>(sizeof(discard), zipFileOffset - lastZipFileOffset);
        if (readCurrent(discard, count) <= 0) {
            resetZipRead();
            return false;
        }
    }
    return true;
}

S32 FsZip::readZip(U64 zipOffset, U64 zipFileOffset, U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(readMutex);
    if (!len) {
        return 0;
    }
    if (!zipfile || !setupZipRead(zipOffset, zipFileOffset)) {
        return -K_EIO;
    }
    U32 total = 0;
    while (total < len) {
        S32 count = readCurrent(buffer + total, len - total);
        if (count <= 0) {
            resetZipRead();
            return total ? (S32)total : -K_EIO;
        }
        total += count;
    }
    if (unzeof(zipfile) == 1) {
        if (unzCloseCurrentFile(zipfile) != UNZ_OK) {
            resetZipRead();
            return -K_EIO;
        }
        lastZipOffset = UINT64_MAX;
        lastZipFileOffset = 0;
    }
    return (S32)total;
}

bool FsZip::init(BString zipPath, BString mount) {
#ifdef BOXEDWINE_ZLIB
    BString strippedMount;

    std::shared_ptr<FsNode> root = Fs::getNodeFromLocalPath(B(""), B(""), true);
    deleteFilePath = root->nativePath.stringByApppendingPath(Fs::getFileNameFromNativePath(zipPath) + ".deleted");
    if (mount.length()) {
        Fs::makeLocalDirs(mount);
        strippedMount = mount.substr(0, mount.length() - 1);
    }
    this->lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
    if (zipPath.length()) {
        unz_global_info64 global_info = {};

        this->zipPath = zipPath;
        this->zipfile = unzOpen(zipPath.c_str());
        if (!this->zipfile) {
            klog_fmt("Could not load zip file: %s", zipPath.c_str());
            return false;
        }

        if (unzGetGlobalInfo64( this->zipfile, &global_info ) != UNZ_OK) {
            klog_fmt("Could not read file global info from zip file: %s", zipPath.c_str());
            unzClose( this->zipfile );
            this->zipfile = nullptr;
            return false;
        }
        fsZipInfo* zipInfo = new fsZipInfo[global_info.number_entry];
        for (U64 i = 0; i < global_info.number_entry; ++i) {
            unz_file_info64 file_info = {};
            struct tm tm={0};
            char tmp[MAX_FILEPATH_LEN];

            tmp[0] = '/';
            if ( unzGetCurrentFileInfo64(this->zipfile, &file_info, tmp + 1, MAX_FILEPATH_LEN - 1, nullptr, 0, nullptr, 0 ) != UNZ_OK || file_info.size_filename >= MAX_FILEPATH_LEN - 1 ) {
                klog_fmt("Could not read file info from zip file: %s", zipPath.c_str());
                delete[] zipInfo;
                unzClose( zipfile );
                zipfile = nullptr;
                return false;
            }
            zipInfo[i].filename = BString::copy(tmp);
            zipInfo[i].offset = unzGetOffset64(this->zipfile);
            zipInfo[i].compressedLength = file_info.compressed_size;
            zipInfo[i].compressionMethod = (U32)file_info.compression_method;
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

            if (!zipInfo[i].isDirectory && zipInfo[i].compressionMethod == 0) {
                if (unzOpenCurrentFile(this->zipfile) == UNZ_OK) {
                    zipInfo[i].dataOffset = unzGetCurrentFileZStreamPos64(this->zipfile);
                    unzCloseCurrentFile(this->zipfile);
                }
            }

            unzGoToNextFile(this->zipfile);
        }
        std::vector<BString> deletedLocalPaths;
        readLinesFromFile(deleteFilePath, deletedLocalPaths);

        for (U64 i = 0; i < global_info.number_entry; ++i) {
            if (zipInfo[i].filename.endsWith(EXT_LINK)) {
                char tmp[MAX_FILEPATH_LEN];
                zipInfo[i].filename = zipInfo[i].filename.substr(0, zipInfo[i].filename.length() - 5);
                zipInfo[i].isLink = true;
                S32 read = -1;
                if (unzSetOffset64(zipfile, zipInfo[i].offset) == UNZ_OK && unzOpenCurrentFile(zipfile) == UNZ_OK) {
                    read = unzReadCurrentFile(zipfile, tmp, MAX_FILEPATH_LEN - 1);
                }
                S32 closed = unzCloseCurrentFile(zipfile);
                if (read < 0 || (U64)read != zipInfo[i].length || closed != UNZ_OK) {
                    delete[] zipInfo;
                    unzClose(zipfile);
                    zipfile = nullptr;
                    return false;
                }
                tmp[read] = 0;
                zipInfo[i].link = BString::copy(tmp);
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
            std::shared_ptr<FsNode> existingNode = Fs::getNodeFromLocalPath(B(""), localPath, false);
            if (!existingNode) {
                std::shared_ptr<FsFileNode> node = Fs::addFileNode(localPath, zipInfo[i].link, nativePath, zipInfo[i].isDirectory, parent);
                node->zipNode = std::make_shared<FsZipNode>(zipInfo[i], shared_from_this());
            }
        }   
        delete[] zipInfo;
    }
#endif
    return true;
}

FsZip::~FsZip() {
#ifdef BOXEDWINE_ZLIB
    checkpoints.clear();
    if (zipfile) {
        unzClose(zipfile);
    }
#endif
}

void FsZip::remove(BString localPath) {
    BOXEDWINE_CRITICAL_SECTION;
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
            BString outPath = path.stringByApppendingPath(Fs::getFileNameFromPath(file));
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
            BString dirPath = path.stringByApppendingPath(fileName);
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
        BString outPath = path.stringByApppendingPath(fileName);
#ifdef BOXEDWINE_MSVC
        if (outPath.length() > 255) {
            outPath = "\\\\?\\" + outPath;
        }
#endif
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
