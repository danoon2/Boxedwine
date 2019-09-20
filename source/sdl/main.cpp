/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include MKDIR_INCLUDE

#include <SDL.h>
#include "kscheduler.h"
#include "kstat.h"
#include "devtty.h"
#include "devurandom.h"
#include "devnull.h"
#include "devzero.h"
#include "ksystem.h"
#include "meminfo.h"
#include "bufferaccess.h"
#include "devfb.h"
#include "devinput.h"
#include "devdsp.h"
#include "sdlwindow.h"
#include "devmixer.h"
#include "devsequencer.h"
#include "sdlwindow.h"
#include "procselfexe.h"
#include "../io/fsfilenode.h"
#include "recorder.h"
#include "mainloop.h"
#include "loader.h"
#include "../io/fszip.h"

void gl_init();
extern int bits_per_pixel;
extern int rel_mouse_sensitivity;
const char* glExt;

#include CURDIR_INCLUDE

extern U32 sdlFullScreen;
bool soundEnabled = true;
bool videoEnabled = true;

#ifndef __TEST

char curdir[1024];

U32 getMilliesSinceStart() {
    return SDL_GetTicks();
}

// This parses a resolution given as a string in the format of: '800x600'
// with the width being the first number
//
// This logic taken from WINE source code in desktop.c
int parse_resolution(const char *resolutionString, U32 *width, U32 *height)
{
    // Moving pointer for where the character parsing completes at
    char *end;

    // Parse the width
    *width = strtoul(resolutionString, &end, 10);

    // Width parsing failed, pointer not moved
    if (end == resolutionString) 
        return false;
	
    // If the next character is not an 'x' then it is an improper resolution format
    if (*end != 'x') 
        return false;

    // Advance the string to beyond the 'x'
    resolutionString = end + 1;

    // Attempt to parse the height
    *height = strtoul(resolutionString, &end, 10);

    // Height parsing failed, character not null (end of string)
    if (*end)
        return false;

    // Made it!  Full string was parsed
    return true;
}

extern int sdlScaleX;
extern int sdlScaleY;
extern const char* sdlScaleQuality;
U32 gensrc;

#ifdef GENERATE_SOURCE
void writeSource();
#endif

void initWine();
void initSDL();

#define mdev(x,y) ((x << 8) | y)

FsOpenNode* openKernelCommandLine(const BoxedPtr<FsNode>& node, U32 flags) {
    return new BufferAccess(node, flags, "");
}

class MountInfo {
public:
    MountInfo(const std::string& localPath, const std::string& nativePath, bool wine) : localPath(localPath), nativePath(nativePath), wine(wine){}

    std::string localPath;
    std::string nativePath;
    bool wine;
};

int boxedmain(int argc, const char **argv) {
    int i;
    std::string root = "";
    const char* ppenv[32];
    int envc=0;
    int userId = UID;
    int groupId = GID;
    int effectiveUserId = UID;
    int effectiveGroupId = GID;
    const char* workingDir = "/home/username";
    bool workingDirSet = false;
    char pwd[MAX_FILEPATH_LEN];	
    bool resolutionSet = false;
    bool euidSet = false;
    bool nozip = false;
    std::vector<MountInfo> mountInfo;
    std::vector<std::string> zips;

    bool showStartingWindow = false;

    klog("Starting ...");

    Platform::startMicroCounter();
#ifdef LOG_OPS
    logFile = fopen("log.txt", "w");
#endif
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i], "-root") && i+1<argc) {
            root = argv[i+1];
            i++;
        } else if (!strcmp(argv[i], "-zip") && i+1<argc) {
#ifdef BOXEDWINE_ZLIB
            zips.push_back(argv[i+1]);
#else
            printf("BoxedWine wasn't compiled with zlib support");
#endif
            i++;
        } else if (!strcmp(argv[i], "-nozip") && i+1<argc) {
            nozip = true;
        } else if (!strcmp(argv[i], "-m") && i+1<argc) {
            // no longer used
            i++;
        } else if (!strcmp(argv[i], "-uid") && i+1<argc) {
            userId = atoi(argv[i+1]);
            i++;
            if (!euidSet)
                effectiveUserId = userId;
        } else if (!strcmp(argv[i], "-gid") && i+1<argc) {
            groupId = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-euid") && i+1<argc) {
            effectiveUserId = atoi(argv[i+1]);
            i++;
            euidSet = true;
        } else if (!strcmp(argv[i], "-egid") && i+1<argc) {
            effectiveGroupId = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-w") && i+1<argc) {
            workingDir = argv[i+1];
            i++;
            workingDirSet = true;
        } else if (!strcmp(argv[i], "-gensrc")) {
            gensrc = 1;
		} else if (!strcmp(argv[i], "-nosound")) {
			soundEnabled = false;
        } else if (!strcmp(argv[i], "-novideo")) {
			videoEnabled = false;
        } else if (!strcmp(argv[i], "-env")) {
			ppenv[envc++] = argv[i+1];
            i++;
        } else if (!strcmp(argv[i], "-fullscreen")) {
			sdlFullScreen = true;
        } else if (!strcmp(argv[i], "-scale")) {
			sdlScaleX = atoi(argv[i+1]);
            sdlScaleY = sdlScaleX;
            if (!resolutionSet) {
                screenCx = 640;
                screenCy = 480;
            }
            i++;
        } else if (!strcmp(argv[i], "-scale_quality")) {
			sdlScaleQuality = argv[i+1];
            i++;
        } else if (!strcmp(argv[i], "-resolution")) {
            U32 width;
            U32 height;

            int success = parse_resolution(argv[i+1], &width, &height);
            if (success) {
                screenCx = width;
                screenCy = height;
                resolutionSet = true;
                klog("Resolution set to: %dx%d", screenCx, screenCy);
            }
            i++;
        } else if (!strcmp(argv[i], "-bpp")) {
            bits_per_pixel = atoi(argv[i+1]);
            i++;
            if (bits_per_pixel!=16 && bits_per_pixel!=32 && bits_per_pixel!=8) {
                klog("-bpp must be 8, 16 or 32");
                bits_per_pixel = 32;
            }
        } else if (!strcmp(argv[i], "-noexecfiles")) {
            std::vector<std::string> files;
            stringSplit(files, argv[i+1], ':');
            for (U32 f=0;f<files.size();f++) {
                FsFileNode::nonExecFileFullPaths.insert(files[f]);
            }
            i++;
        } else if (!strcmp(argv[i], "-p2")) {
            KSystem::pentiumLevel = 2;
        } else if (!strcmp(argv[i], "-p3")) {
            KSystem::pentiumLevel = 3;
        } else if (!strcmp(argv[i], "-glext")) {
            glExt = argv[i+1];
            i++;
        } else if (!strcmp(argv[i], "-rel_mouse_sensitivity")) {
            rel_mouse_sensitivity = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-mount_drive")) {
            if (strlen(argv[i+2])!=1) {
                printf("-mount_drive expects 2 parameters: <host directory to mount> <drive letter to use for wine>");
                printf("example: -mount_drive \"c:\\my games\" d\n");
            } else {
                mountInfo.push_back(MountInfo(argv[i+2], argv[i+1], true));
            }
            i+=2;
        } else if (!strcmp(argv[i], "-mount")) {
            if (argv[i+2][0]!='/') {
                printf("-mount expects 2 parameters: <host directory to mount> <full path on root file>\n");
                printf("example: -mount \"c:\\my games\" \"/home/username/my games\"\n");
            } else {
                mountInfo.push_back(MountInfo(argv[i+2], argv[i+1], false));
            }
            i+=2;
        } else if (!strcmp(argv[i], "-mount_zip")) {
            if (argv[i+2][0]!='/') {
                printf("-mount_drive expects 2 parameters: <zip file to mount> <full path on root file>");
                printf("example: -mount_zip \"c:\\my games\\mygame.zip\" d\n");
            } else {
                mountInfo.push_back(MountInfo(argv[i+2], argv[i+1], true));
            }
            i+=2;
        } else if (!strcmp(argv[i], "-showStartupWindow")) {
            showStartingWindow = true;
        }
#ifdef BOXEDWINE_RECORDER
        else if (!strcmp(argv[i], "-record")) {
            if (!Fs::doesNativePathExist(argv[i+1])) {
                MKDIR(argv[i+1]);
                if (!Fs::doesNativePathExist(argv[i+1])) {
                    klog("-record path does not exist and could not be created: %s", argv[i+1]);
                    return 1;
                }
            }
            Recorder::start(argv[i+1]);
            i++;
        }  else if (!strcmp(argv[i], "-automation")) {
            if (!Fs::doesNativePathExist(argv[i+1])) {
                klog("-automation directory does not exist %s", argv[i+1]);
                return 1;
            }
            Player::start(argv[i+1]);
            i++;
        }
#endif
        else {
            break;
        }
    }    
    char* base = getcwd(curdir, sizeof(curdir));
    char pathSeperator;

    if (strchr(base, '\\')!=0) {
        pathSeperator = '\\';
    } else {
        pathSeperator = '/';
    }
    std::string base2 = SDL_GetBasePath();
    base2 = base2.substr(0, base2.length()-1); 
    if (zips.size()==0 && !nozip) {
        std::vector<Platform::ListNodeResult> results;
        Platform::listNodes(base, results);
        for (auto&& item : results) {
            if (strstr(item.name.c_str(), "Wine") && strstr(item.name.c_str(), ".zip")) {
                zips.push_back(std::string(base) + pathSeperator + item.name);
                break;
            }
        }
        if (zips.size()==0) {
            results.clear();
            Platform::listNodes(base2, results);
            for (auto&& item : results) {
                if (strstr(item.name.c_str(), "Wine") && strstr(item.name.c_str(), ".zip")) {
                    zips.push_back(std::string(base2) + pathSeperator + item.name);
                }
            }
        }
    }
    if (!root.length()) {
        root=SDL_GetPrefPath("Boxedwine", "root");
    }   
    BOXEDWINE_RECORDER_INIT(root, zips, workingDir, &argv[i], argc-i);
    klog("Using root directory: %s", root.c_str());
    for (auto& zip : zips) {
        klog("Using zip file system: %s", zip.c_str());
    }
    if (!Fs::initFileSystem(root)) {
        kwarn("root %s does not exist", root.c_str());
        return 0;
    }
#ifdef BOXEDWINE_ZLIB
    for (auto& zip : zips) {
        FsZip* fsZip = new FsZip();
        fsZip->init(zip, "");
    }
#endif
    initSDL();
    initWine();
    gl_init();    
    strcpy(pwd, "PWD=");
    strcat(pwd, workingDir);

    ppenv[envc++] = "HOME=/home/username";
    ppenv[envc++] = "LOGNAME=username";
    ppenv[envc++] = "USERNAME=username";
    ppenv[envc++] = "USER=username";
    ppenv[envc++] = pwd;
    ppenv[envc++] = "DISPLAY=:0";
    ppenv[envc++] = "LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib:/lib/i386-linux-gnu";    
    if (userId==0)
        ppenv[envc++] = "PATH=/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin";
    else
        ppenv[envc++] = "PATH=/bin:/usr/bin:/usr/local/bin";
    //ppenv[envc++] = "LD_SHOW_AUXV=1";
    //ppenv[envc++] = "LD_DEBUG=all";
    //ppenv[envc++] = "LD_BIND_NOW=1";
    ppenv[envc++] = "WINELOADERNOEXEC=1";
    //ppenv[envc++] = "WINEDLLOVERRIDES=mscoree,mshtml=";
    //ppenv[envc++] = "WINEDLLOVERRIDES=winemenubuilder.exe=d";
    //ppenv[envc++] = "WINEDEBUG=+d3d";

    // If these are in the zip file system they will be overwritten, which is fine
    // These are just added so that the parent node of the following virtual files exist
    Fs::makeLocalDirs("/dev");
    Fs::makeLocalDirs("/proc");
    Fs::makeLocalDirs("/mnt");

    BoxedPtr<FsNode> rootNode = Fs::getNodeFromLocalPath("", "/", true);
    BoxedPtr<FsNode> devNode = Fs::addFileNode("/dev", "", "", true, rootNode);
    BoxedPtr<FsNode> inputNode = Fs::addFileNode("/dev/input", "", "", true, devNode);
    BoxedPtr<FsNode> procNode = Fs::addFileNode("/proc", "", "", true, rootNode);
    BoxedPtr<FsNode> procSelfNode = Fs::addFileNode("/proc/self", "", "", true, procNode);

    Fs::addVirtualFile("/dev/tty0", openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(4, 0), devNode);
    Fs::addVirtualFile("/dev/tty", openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(4, 0), devNode);
    Fs::addVirtualFile("/dev/tty2", openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(4, 2), devNode); // used by XOrg
    Fs::addVirtualFile("/dev/urandom", openDevURandom, K__S_IREAD|K__S_IFCHR, mdev(1, 9), devNode);
    Fs::addVirtualFile("/dev/random", openDevURandom, K__S_IREAD|K__S_IFCHR, mdev(1, 8), devNode);
    Fs::addVirtualFile("/dev/null", openDevNull, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(1, 3), devNode);
    Fs::addVirtualFile("/dev/zero", openDevZero, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(1, 5), devNode);
    Fs::addVirtualFile("/proc/meminfo", openMemInfo, K__S_IREAD, mdev(0, 0), procNode);
    Fs::addVirtualFile("/proc/self/exe", openProcSelfExe, K__S_IREAD, mdev(0, 0), procSelfNode);
    Fs::addVirtualFile("/proc/cmdline", openKernelCommandLine, K__S_IREAD, mdev(0, 0), procNode); // kernel command line
    Fs::addVirtualFile("/dev/fb0", openDevFB, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(0x1d, 0), devNode);
    Fs::addVirtualFile("/dev/input/event3", openDevInputTouch, K__S_IWRITE|K__S_IREAD|K__S_IFCHR, mdev(0xd, 0x43), inputNode);
    Fs::addVirtualFile("/dev/input/event4", openDevInputKeyboard, K__S_IWRITE|K__S_IREAD|K__S_IFCHR, mdev(0xd, 0x44), inputNode);
	Fs::addVirtualFile("/dev/dsp", openDevDsp, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, mdev(14, 3), devNode);
	Fs::addVirtualFile("/dev/mixer", openDevMixer, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, mdev(14, 0), devNode);
    Fs::addVirtualFile("/dev/sequencer", openDevSequencer, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, mdev(14, 1), devNode);
    
    for(auto&& info: mountInfo) {
        if (info.wine) {
            BoxedPtr<FsNode> mntDir = Fs::getNodeFromLocalPath("", "/mnt", true);
            BoxedPtr<FsNode> drive_d = Fs::addRootDirectoryNode("/mnt/drive_"+info.localPath, info.nativePath, mntDir);
            BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", "/home/username/.wine/dosdevices", true);
            Fs::addFileNode("/home/username/.wine/dosdevices/"+info.localPath+":", "/mnt/drive_"+info.localPath, "", false, parent); 
        } else {
            std::string ext = info.nativePath.substr(info.nativePath.length()-4);
            stringToLower(ext);
            if (ext == ".zip") {
#ifdef BOXEDWINE_ZLIB
                FsZip* z = new FsZip();
                if (!stringHasEnding(info.localPath, "/")) {
                    info.localPath+="/";
                }
                z->init(info.nativePath, info.localPath);
#else
                klog("% not mounted because zlib was not compiled in", info.nativePath.c_str());
#endif
            } else {
                BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", Fs::getParentPath(info.localPath), true);
                Fs::addRootDirectoryNode(info.localPath, info.nativePath, parent);
            }
        }
    }

    argc = argc-i;
    bool validLinuxCommand = false;
    if (argc==0) {
        argv[0]="/bin/wine";
        argv[1]="explorer";
        argv[2]="/desktop=shell";
        argc=3;
    } else {
        argv = &argv[i];
        
        BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(workingDir, argv[0], true);        

        if (node) {
            std::string interpreter;
            std::string loader;
            std::vector<std::string> interpreterArgs;
            std::vector<std::string> args;

            FsOpenNode* openNode=ElfLoader::inspectNode(workingDir, node, loader, interpreter, interpreterArgs);
            if (openNode) {
                openNode->close();
                validLinuxCommand = true;
            }
        }
        if (!validLinuxCommand) {
            if (Fs::doesNativePathExist(argv[0])) {
                std::string dir = argv[0];
                if (argv[0][dir.length()-1]=='/' || argv[0][dir.length()-1]=='\\') {
                    dir = dir.substr(0, dir.length()-1);
                }
                bool isDir = Fs::isNativePathDirectory(dir);
                BoxedPtr<FsNode> mntDir = Fs::getNodeFromLocalPath("", "/mnt", true);

                if (!isDir) {
                    dir = Fs::getNativeParentPath(dir);
                }
                
                BoxedPtr<FsNode> drive_d = Fs::addRootDirectoryNode("/mnt/drive_t", dir, mntDir);
                BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", "/home/username/.wine/dosdevices", true);
                if (parent) {
                    Fs::addFileNode("/home/username/.wine/dosdevices/t:", "/mnt/drive_t", "", false, parent);
                }

                if (!workingDirSet) {
                    workingDir = "/mnt/drive_t";
                }

                if (isDir) {
                    const char** a = new const char*[3];
                    a[0] = "/bin/wine";
                    a[1] = "explorer";
                    a[2] = "t:\\";
                    argc = 3;
                    argv = a;
                } else {
                    const char** a = new const char*[argc+1];
                    a[0] = "/bin/wine";
                    std::string path = "t:\\"+Fs::getFileNameFromNativePath(argv[0]);
                    char* c = new char[path.length()+1];
                    strcpy(c, path.c_str());
                    a[1] = c;
                    for (int i=1;i<argc;i++) {
                        a[i+1] = argv[i];
                    }
                    argc++;
                    argv = a;
                }
            }
        }
    }
#ifdef SDL2
    U32 flags = SDL_INIT_EVENTS;
#else
    U32 flags = SDL_INIT_TIMER;
#endif
    if (videoEnabled) {
        flags|=SDL_INIT_VIDEO;
    }
    if (soundEnabled) {
        flags|=SDL_INIT_AUDIO;
    }
    if (SDL_Init(flags) != 0) {
        klog("SDL_Init Error: %s", SDL_GetError());
        return 0;
    }
    if (showStartingWindow) {
        showSDLStartingWindow();
    }
    if (!validLinuxCommand && (zips.size()==0 || !Fs::doesNativePathExist(zips[0])) && !Fs::getNodeFromLocalPath("", "/bin/wine", true) ){
        if (videoEnabled) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "File system not found", "Make sure you have a valid zip file in the same folder as Boxedwine or you specify one on the commandline.", NULL);
        } else {
            klog("File system not found: Make sure you have a valid zip file in the same folder as Boxedwine or you specify one on the commandline.");
        }
    }

#ifdef SDL2
    if (sdlFullScreen && !resolutionSet) {
        SDL_DisplayMode mode;
        if (!SDL_GetCurrentDisplayMode(0, &mode)) {
            screenCx = mode.w;
            screenCy = mode.h;
        }
    }
#endif
    klog("Launching %s", argv[0]);
    KProcess* process = new KProcess(KSystem::nextThreadId++);    
    if (process->startProcess(workingDir, argc, (const char**)argv, envc, ppenv, userId, groupId, effectiveUserId, effectiveGroupId)) {
        if (!doMainLoop()) {
            return 0; // doMainLoop should have handled any cleanup, like SDL_Quit if necessary
        }
    }
#ifdef GENERATE_SOURCE
    if (gensrc)
        writeSource();
#endif
    dspShutdown();
    klog("Boxedwine shutdown");
    SDL_Quit();
    return BOXEDWINE_RECORDER_QUIT();
}

int main(int argc, char **argv) {
    return boxedmain(argc, (const char **)argv);
}

#endif
