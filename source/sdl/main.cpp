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

void gl_init();
extern int bits_per_pixel;

#include CURDIR_INCLUDE

extern U32 sdlFullScreen;

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

extern int sdlScale;
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

bool soundEnabled = true;
bool videoEnabled = true;

int boxedmain(int argc, const char **argv) {
    int i;
    const char* root = ".";
    const char* zip = "";
    const char* ppenv[32];
    int envc=0;
    int userId = UID;
    int groupId = GID;
    int effectiveUserId = UID;
    int effectiveGroupId = GID;
    const char* workingDir = "/home/username";
    char pwd[MAX_FILEPATH_LEN];	
    bool resolutionSet = false;
    bool euidSet = false;

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
            zip = argv[i+1];
#else
            printf("BoxedWine wasn't compiled with zlib support");
#endif
            i++;
        } else if (!strcmp(argv[i], "-m") && i+1<argc) {
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
			sdlScale = atoi(argv[i+1]);
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
        } 
#ifdef BOXEDWINE_RECORDER
        else if (!strcmp(argv[i], "-record")) {
            if (!Fs::doesNativePathExist(argv[i+1])) {
                mkdir(argv[i+1]);
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
    if (!root) {
        char* base = getcwd(curdir, sizeof(curdir));
        int len;
        char pathSeperator;

        if (strchr(base, '\\')!=0) {
            pathSeperator = '\\';
        } else {
            pathSeperator = '/';
        }
        len = (int)strlen(base);
        if (base[len-1]!=pathSeperator) {
            base[len] = pathSeperator;
            base[len+1] = 0;
        }
        safe_strcat(base, "root", sizeof(curdir));
        root=base;
    }
    BOXEDWINE_RECORDER_INIT(root, zip, workingDir, &argv[i], argc-i);
    klog("Using root directory: %s", root);
    if (!Fs::initFileSystem(root, zip)) {
        kwarn("root %s does not exist", root);
        return 0;
    }
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
    ppenv[envc++] = "WINEDLLOVERRIDES=winemenubuilder.exe=d";
    //ppenv[envc++] = "WINEDEBUG=+loaddll,+module,+dll";

    // If these are in the zip file system they will be overwritten, which is fine
    // These are just added so that the parent node of the following virtual files exist
    Fs::makeLocalDirs("/dev");
    Fs::makeLocalDirs("/proc");
    BoxedPtr<FsNode> rootNode = Fs::getNodeFromLocalPath("", "/", true);
    BoxedPtr<FsNode> devNode = Fs::addFileNode("/dev", "", true, rootNode);
    BoxedPtr<FsNode> inputNode = Fs::addFileNode("/dev/input", "", true, devNode);
    BoxedPtr<FsNode> procNode = Fs::addFileNode("/proc", "", true, rootNode);
    BoxedPtr<FsNode> procSelfNode = Fs::addFileNode("/proc/self", "", true, procNode);

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

    argc = argc-i;
    if (argc==0) {
        argv[0]="/usr/bin/wine";
        argv[1]="/home/username/chomp/CHOMP.EXE";
        argc=2;
        //argv[0]="/init.sh";
        //argc=1;
    } else {
        argv = &argv[i];
    }
    U32 flags = SDL_INIT_EVENTS;
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
    klog("Boxedwine shutdown");
    SDL_Quit();
    return BOXEDWINE_RECORDER_QUIT();
}

int main(int argc, char **argv) {
    return boxedmain(argc, (const char **)argv);
}

#endif
