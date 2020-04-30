#include "boxedwine.h"
#include "startupArgs.h"

#include "devtty.h"
#include "devurandom.h"
#include "devnull.h"
#include "devzero.h"
#include "devfb.h"
#include "devinput.h"
#include "devdsp.h"
#include "procselfexe.h"
#include "cpuinfo.h"
#include "syscpuonline.h"
#include "syscpumaxfreq.h"
#include "syscpuscalingcurfreq.h"
#include "syscpuscalingmaxfreq.h"
#include "bufferaccess.h"
#include "meminfo.h"
#include "devmixer.h"
#include "devsequencer.h"
#include "mainloop.h"
#include "sdlwindow.h"
#include "../io/fsfilenode.h"
#include "../io/fszip.h"
#include "loader.h"
#include "kstat.h"

#ifndef BOXEDWINE_DISABLE_UI
#include "../ui/data/globalSettings.h"
#endif

#include <SDL.h>
#include MKDIR_INCLUDE
#include CURDIR_INCLUDE

#define mdev(x,y) ((x << 8) | y)

void gl_init(const std::string& allowExtensions);
void initWine();

U32 StartUpArgs::uiType;

FsOpenNode* openKernelCommandLine(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, "");
}

// This parses a resolution given as a string in the format of: '800x600'
// with the width being the first number
//
// This logic taken from WINE source code in desktop.c
int StartUpArgs::parse_resolution(const char *resolutionString, U32 *width, U32 *height)
{
    // Moving pointer for where the character parsing completes at
    char *end;

    // Parse the width
    *width = (U32)strtoul(resolutionString, &end, 10);

    // Width parsing failed, pointer not moved
    if (end == resolutionString) 
        return false;
	
    // If the next character is not an 'x' then it is an improper resolution format
    if (*end != 'x') 
        return false;

    // Advance the string to beyond the 'x'
    resolutionString = end + 1;

    // Attempt to parse the height
    *height = (U32)strtoul(resolutionString, &end, 10);

    // Height parsing failed, character not null (end of string)
    if (*end)
        return false;

    // Made it!  Full string was parsed
    return true;
}

void StartUpArgs::buildVirtualFileSystem() {
    Fs::makeLocalDirs("/dev");
    Fs::makeLocalDirs("/proc");
    Fs::makeLocalDirs("/mnt");

    BoxedPtr<FsNode> rootNode = Fs::getNodeFromLocalPath("", "/", true);
    BoxedPtr<FsNode> devNode = Fs::addFileNode("/dev", "", rootNode->nativePath+Fs::nativePathSeperator+"dev", true, rootNode);
    BoxedPtr<FsNode> inputNode = Fs::addFileNode("/dev/input", "", "", true, devNode);
    BoxedPtr<FsNode> procNode = Fs::addFileNode("/proc", "", "", true, rootNode);
    BoxedPtr<FsNode> procSelfNode = Fs::addFileNode("/proc/self", "", "", true, procNode);
    BoxedPtr<FsNode> sysNode = Fs::getNodeFromLocalPath("", "/sys", true); 
    if (!sysNode) {
        sysNode = Fs::addFileNode("/sys", "", "", true, rootNode);
    }
    BoxedPtr<FsNode> devicesNode = Fs::getNodeFromLocalPath("", "/sys/devices", true); 
    if (!devicesNode) {
        devicesNode = Fs::addFileNode("/sys/devices", "", "", true, sysNode);
    }
    BoxedPtr<FsNode> devicesSystemNode = Fs::addFileNode("/sys/devices/system", "", "", true, devicesNode);
    BoxedPtr<FsNode> cpuNode = Fs::addFileNode("/sys/devices/system/cpu", "", "", true, devicesSystemNode);    

    Fs::addVirtualFile("/dev/tty0", openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(4, 0), devNode);
    Fs::addVirtualFile("/dev/tty", openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(4, 0), devNode);
    Fs::addVirtualFile("/dev/tty2", openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(4, 2), devNode); // used by XOrg
    Fs::addVirtualFile("/dev/urandom", openDevURandom, K__S_IREAD|K__S_IFCHR, mdev(1, 9), devNode);
    Fs::addVirtualFile("/dev/random", openDevURandom, K__S_IREAD|K__S_IFCHR, mdev(1, 8), devNode);
    Fs::addVirtualFile("/dev/null", openDevNull, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(1, 3), devNode);
    Fs::addVirtualFile("/dev/zero", openDevZero, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(1, 5), devNode);
    Fs::addVirtualFile("/proc/meminfo", openMemInfo, K__S_IREAD, mdev(0, 0), procNode);
    Fs::addVirtualFile("/proc/cpuinfo", openCpuInfo, K__S_IREAD, mdev(0, 0), procNode);
    Fs::addVirtualFile("/proc/self/exe", openProcSelfExe, K__S_IREAD, mdev(0, 0), procSelfNode);
    Fs::addVirtualFile("/proc/cmdline", openKernelCommandLine, K__S_IREAD, mdev(0, 0), procNode); // kernel command line
    Fs::addVirtualFile("/dev/fb0", openDevFB, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, mdev(0x1d, 0), devNode);
    Fs::addVirtualFile("/dev/input/event3", openDevInputTouch, K__S_IWRITE|K__S_IREAD|K__S_IFCHR, mdev(0xd, 0x43), inputNode);
    Fs::addVirtualFile("/dev/input/event4", openDevInputKeyboard, K__S_IWRITE|K__S_IREAD|K__S_IFCHR, mdev(0xd, 0x44), inputNode);
	Fs::addVirtualFile("/dev/dsp", openDevDsp, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, mdev(14, 3), devNode);
	Fs::addVirtualFile("/dev/mixer", openDevMixer, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, mdev(14, 0), devNode);
    Fs::addVirtualFile("/dev/sequencer", openDevSequencer, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, mdev(14, 1), devNode);    
    Fs::addVirtualFile("/sys/devices/system/cpu/online", openSysCpuOnline, K__S_IREAD, mdev(0, 0), cpuNode);

    if (Platform::getCpuFreqMHz()) {        
        for (U32 i=0;i<Platform::getCpuCount();i++) {
            BoxedPtr<FsNode> cpuCoreNode = Fs::addFileNode("/sys/devices/system/cpu/cpu"+std::to_string(i), "", "", true, cpuNode);    
            Fs::addVirtualFile("/sys/devices/system/cpu"+std::to_string(i)+"/scaling_cur_freq", openSysCpuScalingCurrentFrequency, K__S_IREAD, mdev(0, 0), cpuCoreNode, i);
            Fs::addVirtualFile("/sys/devices/system/cpu"+std::to_string(i)+"/cpuinfo_max_freq", openSysCpuMaxFrequency, K__S_IREAD, mdev(0, 0), cpuCoreNode, i);
            Fs::addVirtualFile("/sys/devices/system/cpu"+std::to_string(i)+"/scaling_max_freq", openSysCpuScalingMaxFrequency, K__S_IREAD, mdev(0, 0), cpuCoreNode, i);
        }
    }
}

bool StartUpArgs::apply() {
    KSystem::init();    

    KSystem::pentiumLevel = this->pentiumLevel;
    for (U32 f=0;f<nonExecFileFullPaths.size();f++) {
        FsFileNode::nonExecFileFullPaths.insert(nonExecFileFullPaths[f]);
    }
    BOXEDWINE_RECORDER_INIT(this->root, this->zips, this->workingDir, this->args);

    klog("Using root directory: %s", root.c_str());
    for (auto& zip : zips) {
        klog("Using zip file system: %s", zip.c_str());
    }
    if (!Fs::initFileSystem(root)) {
        kwarn("root %s does not exist", root.c_str());
        return false;
    }
    if (this->resolutionSet) {
        klog("Resolution set to: %dx%d", this->screenCx, this->screenCy);
    }
#ifdef BOXEDWINE_ZLIB
    std::vector<std::shared_ptr<FsZip>> openZips;
    for (auto& zip : zips) {
        U64 startTime = KSystem::getMicroCounter();
        std::shared_ptr<FsZip> fsZip = std::make_shared<FsZip>();
        fsZip->init(zip, "");
        openZips.push_back(fsZip);
        U64 endTime = KSystem::getMicroCounter();
        klog("Loaded %s in %d ms", zip.c_str(), (U32)(endTime - startTime) / 1000);
    }
#endif

    buildVirtualFileSystem();

    envValues.push_back("HOME=/home/username");
    envValues.push_back("LOGNAME=username");
    envValues.push_back("USERNAME=username");
    envValues.push_back("USER=username");
    envValues.push_back("PWD="+this->workingDir);
    envValues.push_back("DISPLAY=:0");
                            
    // if this strlen is more than 88 (1 more character than now), then diablo demo will crash before we get to the menu
    // if I create more env values that are longer it doesn't crash, what is special about this one?
    
    //crashes, if I change LD_LIBRARY_PATH to LD_LIBRARY_PATT it works, so it's not the length of the env string, but rather specific to LD_LIBRARY_PATH
    //envValues.push_back("LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib:/lib/i386-linux-gnu:/usr/lib/i386-linux-gnu:/opt/wine/lib");

    //works
    //envValues.push_back("LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib:/lib/i386-linux-gnu:/usr/lib/i386-linux-gnu");        
    if (userId==0) {
        envValues.push_back("PATH=/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin");
    } else {
        envValues.push_back("PATH=/bin:/usr/bin:/usr/local/bin");
    }

    for(auto&& info: this->mountInfo) {
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
                U64 startTime = KSystem::getMicroCounter();
                std::shared_ptr<FsZip> fsZip = std::make_shared<FsZip>();
                if (!stringHasEnding(info.localPath, "/", false)) {
                    info.localPath+="/";
                }
                fsZip->init(info.nativePath, info.localPath);
                openZips.push_back(fsZip);
                U64 endTime = KSystem::getMicroCounter();
                klog("Mounted %s in %d ms", info.nativePath.c_str(), (U32)(endTime - startTime) / 1000);
    #else
                klog("% not mounted because zlib was not compiled in", info.nativePath.c_str());
    #endif
            } else {
                BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", Fs::getParentPath(info.localPath), true);
                Fs::addRootDirectoryNode(info.localPath, info.nativePath, parent);
            }
        }
    }

    if (this->args.size()==0) {
        args.push_back("/bin/wine");
        args.push_back("explorer");
        args.push_back("/desktop=shell");
    }
    if (this->args.size()) {
        BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(workingDir, this->args[0], true);        

        bool validLinuxCommand = false;
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
            if (Fs::doesNativePathExist(args[0])) {
                std::string dir = args[0];
                if (args[0][dir.length()-1]=='/' || args[0][dir.length()-1]=='\\') {
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
                    args.clear();
                    args.push_back("/bin/wine");
                    args.push_back("explorer");
                    args.push_back("t:\\");
                } else {                
                    std::string fileName = Fs::getFileNameFromNativePath(args[0]);
                    args.erase(args.begin());
                    args.insert(args.begin(), "t:\\"+fileName);
                    args.insert(args.begin(), "/bin/wine");
                }
            }
        }
    }
#ifdef SDL2
    if (this->sdlFullScreen && !this->resolutionSet) {
        SDL_DisplayMode mode;
        if (!SDL_GetCurrentDisplayMode(0, &mode)) {
#ifdef __ANDROID__
            this->screenCx = mode.w/2;
            this->screenCy = mode.h/2;
#else
            this->screenCx = mode.w;
            this->screenCy = mode.h;
#endif
        }
    }
#endif

    initSDL(this->screenCx, this->screenCy, this->screenBpp, this->sdlScaleX, this->sdlScaleY, this->sdlScaleQuality, this->soundEnabled, this->videoEnabled, this->sdlFullScreen);
    initWine();
#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
    gl_init(this->glExt);        
#endif   

    if (this->args.size()) {
        printf("Launching ");
        for (U32 i=0;i<this->args.size();i++) {
            printf("\"%s\" ", this->args[i].c_str());
        }
        printf("\n");
        bool result = false;
        {
            std::shared_ptr<KProcess> process = KProcess::create();// keep in this small scope so we don't hold onto it for the life of the program
            result = process->startProcess(this->workingDir, this->args, this->envValues, this->userId, this->groupId, this->effectiveUserId, this->effectiveGroupId);
        }
        if (result) {
            if (!doMainLoop()) {
                return 0; // doMainLoop should have handled any cleanup, like SDL_Quit if necessary
            }
        }
    }
#ifdef GENERATE_SOURCE
    if (gensrc)
        writeSource();
#endif
	KSystem::destroy();
    destroySDL();
    dspShutdown();

#ifdef BOXEDWINE_ZLIB
    openZips.clear();
#endif
    return true;
}

bool StartUpArgs::parseStartupArgs(int argc, const char **argv) {
    int i = 1;
    for (;i<argc;i++) {
        if (!strcmp(argv[i], "-root") && i+1<argc) {
            this->setRoot(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-zip") && i+1<argc) {
#ifdef BOXEDWINE_ZLIB
            this->addZip(argv[i+1]);
#else
            printf("BoxedWine wasn't compiled with zlib support");
#endif
            i++;
        } else if (!strcmp(argv[i], "-nozip") && i+1<argc) {
            this->nozip = true;
        } else if (!strcmp(argv[i], "-m") && i+1<argc) {
            // no longer used
            i++;
        } else if (!strcmp(argv[i], "-uid") && i+1<argc) {
            this->userId = atoi(argv[i+1]);
            i++;
            if (!this->euidSet)
                this->effectiveUserId = this->userId;
        } else if (!strcmp(argv[i], "-gid") && i+1<argc) {
            this->groupId = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-euid") && i+1<argc) {
            this->effectiveUserId = atoi(argv[i+1]);
            i++;
            this->euidSet = true;
        } else if (!strcmp(argv[i], "-egid") && i+1<argc) {
            this->effectiveGroupId = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-w") && i+1<argc) {
            this->setWorkingDir(argv[i+1]);
            i++;
            this->workingDirSet = true;
        } else if (!strcmp(argv[i], "-nosound")) {
			this->soundEnabled = false;
        } else if (!strcmp(argv[i], "-novideo")) {
			this->videoEnabled = false;
        } else if (!strcmp(argv[i], "-env")) {
			this->envValues.push_back(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-fullscreen")) {
			this->setFullscreen();
        } else if (!strcmp(argv[i], "-scale")) {
			this->setScale(atoi(argv[i+1]));
            if (!this->resolutionSet) {
                this->screenCx = 640;
                this->screenCy = 480;
            }
            i++;
        } else if (!strcmp(argv[i], "-scale_quality")) {
			this->setScaleQuality(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-resolution")) {
            setResolution(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-bpp")) {
            setBpp(atoi(argv[i+1]));
            i++;
            if (this->screenBpp!=16 && this->screenBpp!=32 && this->screenBpp!=8) {
                klog("-bpp must be 8, 16 or 32");
                this->screenBpp = 32;
            }
        } else if (!strcmp(argv[i], "-noexecfiles")) {
            stringSplit(this->nonExecFileFullPaths, argv[i+1], ':');            
            i++;
        } else if (!strcmp(argv[i], "-p2")) {
            this->pentiumLevel = 2;
        } else if (!strcmp(argv[i], "-p3")) {
            this->pentiumLevel = 3;
        } else if (!strcmp(argv[i], "-glext")) {
            this->setAllowedGlExtension(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-rel_mouse_sensitivity")) {
            this->rel_mouse_sensitivity = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-mount_drive")) {
            if (strlen(argv[i+2])!=1) {
                printf("-mount_drive expects 2 parameters: <host directory to mount> <drive letter to use for wine>");
                printf("example: -mount_drive \"c:\\my games\" d\n");
            } else {
                this->mountInfo.push_back(MountInfo(argv[i+2], argv[i+1], true));
            }
            i+=2;
        } else if (!strcmp(argv[i], "-mount")) {
            if (argv[i+2][0]!='/') {
                printf("-mount expects 2 parameters: <host directory to mount or zip file> <full path on root file>\n");
                printf("example: -mount \"c:\\my games\" \"/home/username/my games\"\n");
                printf("example: -mount_zip \"c:\\my games\\mygame.zip\" /mnt/game\n");
            } else {
                if (Fs::doesNativePathExist(argv[i + 2])) {
                    klog("mount directory/file does not exist: %s", argv[i + 2]);
                    return false;
                }
                this->mountInfo.push_back(MountInfo(argv[i+2], argv[i+1], false));
            }
            i+=2;
        } else if (!strcmp(argv[i], "-showStartupWindow")) {
            // no longer used
        } else if (!strcmp(argv[i], "-ui")) {
            i++;
            if (!strcmp(argv[i], "opengl")) {
                this->uiType = UI_TYPE_OPENGL;
            }
        }
#ifdef BOXEDWINE_RECORDER
        else if (!strcmp(argv[i], "-record")) {
            if (!Fs::doesNativePathExist(argv[i+1])) {
                MKDIR(argv[i+1]);
                if (!Fs::doesNativePathExist(argv[i+1])) {
                    klog("-record path does not exist and could not be created: %s", argv[i+1]);
                    return false;
                }
            }
            Recorder::start(argv[i+1]);
            i++;
        }  else if (!strcmp(argv[i], "-automation")) {
            if (!Fs::doesNativePathExist(argv[i+1])) {
                klog("-automation directory does not exist %s", argv[i+1]);
                return false;
            }
            Player::start(argv[i+1]);
            i++;
        }
#endif
        else {
            break;
        }
    } 
    char curdir[1024];
    char* base = getcwd(curdir, sizeof(curdir));
    char pathSeperator;

    if (strchr(base, '\\')!=0) {
        pathSeperator = '\\';
    } else {
        pathSeperator = '/';
    }
    if (SDL_GetBasePath()) {
        std::string base2 = SDL_GetBasePath();
        base2 = base2.substr(0, base2.length()-1); 
        if (zips.size()==0 && !nozip) {
            std::vector<Platform::ListNodeResult> results;
            Platform::listNodes(base, results);
            for (auto&& item : results) {
                if (strstr(item.name.c_str(), "Wine") && strstr(item.name.c_str(), ".zip")) {
                    this->zips.push_back(std::string(base) + pathSeperator + item.name);
                    break;
                }
            }
            if (zips.size()==0) {
                results.clear();
                Platform::listNodes(base2, results);
                for (auto&& item : results) {
                    if (strstr(item.name.c_str(), "Wine") && strstr(item.name.c_str(), ".zip")) {
                        this->zips.push_back(std::string(base2) + pathSeperator + item.name);
                    }
                }
            }
        }
    }
    if (!this->root.length()) {
#ifdef __ANDROID__
        this->root=SDL_AndroidGetExternalStoragePath();
        this->root+="/root";
#else
        this->root=SDL_GetPrefPath("Boxedwine", "root");
#endif
    }  

    argc = argc-i;
    argv = &argv[i];
        
    for (i=0;i<argc;i++) {
        args.push_back(argv[i]);
    }
    return true;
}

void StartUpArgs::setResolution(const std::string& resolution) {
    U32 width;
    U32 height;

    int success = parse_resolution(resolution.c_str(), &width, &height);
    if (success) {
        this->screenCx = width;
        this->screenCy = height;
        this->resolutionSet = true;                
    }
}
