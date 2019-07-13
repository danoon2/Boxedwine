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

#ifdef BOXEDWINE_ZLIB
unzFile FsZip::zipfile;
#endif

bool FsZip::init(const std::string& zipPath) {
#ifdef BOXEDWINE_ZLIB
    if (zipPath.length()) {
        unz_global_info global_info;
        U32 i;
        fsZipInfo* zipInfo;

        zipfile = unzOpen(zipPath.c_str());
        if (!zipfile) {
            klog("Could not load zip file: %s", zipPath.c_str());
        }

        if (unzGetGlobalInfo( zipfile, &global_info ) != UNZ_OK) {
            klog("Could not read file global info from zip file: %s", zipPath.c_str());
            unzClose( zipfile );
            return false;
        }
        zipInfo = new fsZipInfo[global_info.number_entry];
        for (i = 0; i < global_info.number_entry; ++i) {
            unz_file_info file_info;
            struct tm tm={0};
            char tmp[MAX_FILEPATH_LEN];

            zipInfo[i].filename="/";
            if ( unzGetCurrentFileInfo(zipfile, &file_info, tmp, MAX_FILEPATH_LEN, NULL, 0, NULL, 0 ) != UNZ_OK ) {
                klog("Could not read file info from zip file: %s", zipPath.c_str());
                unzClose( zipfile );
                return false;
            }
            zipInfo[i].filename.append(tmp);
            zipInfo[i].offset = unzGetOffset64(zipfile);
            Fs::remotePathToLocal(zipInfo[i].filename); // converts special characters like :
            if (stringHasEnding(zipInfo[i].filename, ".link")) {
                U32 read;

                zipInfo[i].filename.resize(zipInfo[i].filename.length()-5);
                zipInfo[i].isLink = true;
                unzOpenCurrentFile(zipfile);
                read = unzReadCurrentFile(zipfile, tmp, MAX_FILEPATH_LEN);
                tmp[read]=0;
                zipInfo[i].link = tmp;                
                unzCloseCurrentFile(zipfile);
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

            zipInfo[i].lastModified = mktime(&tm)*1000l;

            unzGoToNextFile(zipfile);
        }
        for (i = 0; i < global_info.number_entry; ++i) {
            std::string parentPath = Fs::getParentPath(zipInfo[i].filename);
            BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", parentPath, true);
            BoxedPtr<FsFileNode> node = (FsFileNode*)Fs::addFileNode(zipInfo[i].filename, zipInfo[i].link, zipInfo[i].isDirectory, parent).get();
            node->zipNode = new FsZipNode(node, zipInfo[i]);
        }   
        delete[] zipInfo;
    }
#endif
    return true;
}
#endif
