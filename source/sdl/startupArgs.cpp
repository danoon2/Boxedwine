#include "boxedwine.h"
#include "startupArgs.h"

#include "devtty.h"
#include "devurandom.h"
#include "devnull.h"
#include "devzero.h"
#include "devinput.h"
#include "devdsp.h"
#include "procselfexe.h"
#include "cpuinfo.h"
#include "bufferaccess.h"
#include "meminfo.h"
#include "procstat.h"

#include "uptime.h"
#include "devmixer.h"
#include "devsequencer.h"
#include "devfb.h"
#include "mainloop.h"
#include "../io/fsfilenode.h"
#include "../io/fszip.h"
#include "loader.h"
#include "kstat.h"
#include "knativesystem.h"
#include "knativewindow.h"
#include "knativeaudio.h"
#include "knativesocket.h"

#ifndef BOXEDWINE_DISABLE_UI
#include "../ui/data/globalSettings.h"
#endif

#include MKDIR_INCLUDE
#include CURDIR_INCLUDE

void gl_init(BString allowExtensions);
void vulkan_init();
void initWine();
void initWineAudio();
void createSysfs(const std::shared_ptr<FsNode> rootNode);

U32 StartUpArgs::uiType;

FsOpenNode* openKernelCommandLine(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, B(""));
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
    Fs::makeLocalDirs(B("/dev"));
    Fs::makeLocalDirs(B("/proc"));
    Fs::makeLocalDirs(B("/mnt"));
    Fs::makeLocalDirs(B("/etc"));

    std::shared_ptr<FsNode> rootNode = Fs::getNodeFromLocalPath(B(""), B("/"), true);
    std::shared_ptr<FsNode> devNode = Fs::addFileNode(B("/dev"), B(""), rootNode->nativePath ^ "dev", true, rootNode);
    std::shared_ptr<FsNode> inputNode = Fs::addFileNode(B("/dev/input"), B(""), B(""), true, devNode);
    KSystem::procNode = Fs::addFileNode(B("/proc"), B(""), B(""), true, rootNode);
    std::shared_ptr<FsNode> procSysNode = Fs::addFileNode(B("/proc/sys"), B(""), B(""), true, KSystem::procNode);
    std::shared_ptr<FsNode> procSysKernelNode = Fs::addFileNode(B("/proc/sys/kernel"), B(""), B(""), true, procSysNode);
    Fs::addVirtualFile(B("/proc/sys/kernel/ngroups_max"), [](const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
        return new BufferAccess(node, flags, B("65536"));
        }, K__S_IREAD, k_mdev(0, 0), procSysKernelNode);
    Fs::addVirtualFile(B("/proc/filesystems"), [](const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
        return new BufferAccess(node, flags, B("nodev\tsysfs\nnodev\trootfs\nnodev\tbdev\nnodev\tproc\nnodev\tcpuset\nnodev\tcgroup\nnodev\tcgroup2\nnodev\ttmpfs\nnodev\tdevtmpfs\nnodev\tdebugfs\nnodev\ttracefs\nnodev\tsecurityfs\nnodev\tsockfs\nnodev\tdax\nnodev\tbpf\nnodev\tpipefs\nnodev\thugetlbfs\nnodev\tdevpts\nodev\tmqueue\nnodev\tpstore\next3\next2\next4\nnodev\tautofs\nfuseblk\nnodev\tfuse\nnodev\tfusectl\n"));
        }, K__S_IREAD, k_mdev(0, 0), KSystem::procNode);

    std::shared_ptr<FsNode> etcNode = Fs::getNodeFromLocalPath(B(""), B("/etc"), true);
    createSysfs(rootNode);    

    Fs::addVirtualFile(B("/dev/tty0"), openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, k_mdev(4, 0), devNode);
    Fs::addVirtualFile(B("/dev/tty"), openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, k_mdev(4, 0), devNode);
    Fs::addVirtualFile(B("/dev/tty2"), openDevTTY, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, k_mdev(4, 2), devNode); // used by XOrg
    Fs::addVirtualFile(B("/dev/urandom"), openDevURandom, K__S_IREAD|K__S_IFCHR, k_mdev(1, 9), devNode);
    Fs::addVirtualFile(B("/dev/random"), openDevURandom, K__S_IREAD|K__S_IFCHR, k_mdev(1, 8), devNode);
    Fs::addVirtualFile(B("/dev/null"), openDevNull, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, k_mdev(1, 3), devNode);
    Fs::addVirtualFile(B("/dev/zero"), openDevZero, K__S_IREAD|K__S_IWRITE|K__S_IFCHR, k_mdev(1, 5), devNode);
    Fs::addVirtualFile(B("/proc/meminfo"), openMemInfo, K__S_IREAD, k_mdev(0, 0), KSystem::procNode);
    Fs::addVirtualFile(B("/proc/stat"), openProcStat, K__S_IREAD, k_mdev(0, 0), KSystem::procNode);
    Fs::addVirtualFile(B("/proc/uptime"), openUptime, K__S_IREAD, k_mdev(0, 0), KSystem::procNode);
    Fs::addVirtualFile(B("/proc/cpuinfo"), openCpuInfo, K__S_IREAD, k_mdev(0, 0), KSystem::procNode);
    Fs::addDynamicLinkFile(B("/proc/self"), k_mdev(0, 0), KSystem::procNode, true, [] {
        return BString::valueOf(KThread::currentThread()->process->id);
        });
    Fs::addVirtualFile(B("/proc/mounts"), [](const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
        return new BufferAccess(node, flags, B("proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0\n/dev/nvme0n1p5 / ext4 rw,relatime,errors=remount-ro 0 0\nudev /dev devtmpfs rw,nosuid,relatime,size=16371216k,nr_inodes=4092804,mode=755,inode64 0 0"));
        }, K__S_IREAD, k_mdev(0, 0), KSystem::procNode);
    Fs::addVirtualFile(B("/proc/cmdline"), openKernelCommandLine, K__S_IREAD, k_mdev(0, 0), KSystem::procNode); // kernel command line
    Fs::addVirtualFile(B("/dev/fb0"), openDevFB, K__S_IREAD | K__S_IWRITE | K__S_IFCHR, k_mdev(0x1d, 0), devNode);
    Fs::addVirtualFile(B("/dev/input/mice"), openDevInputTouch, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, k_mdev(0xd, 0x43), inputNode);
    Fs::addVirtualFile(B("/dev/input/event3"), openDevInputTouch, K__S_IWRITE|K__S_IREAD|K__S_IFCHR, k_mdev(0xd, 0x43), inputNode);
    Fs::addVirtualFile(B("/dev/input/event4"), openDevInputKeyboard, K__S_IWRITE|K__S_IREAD|K__S_IFCHR, k_mdev(0xd, 0x44), inputNode);
	Fs::addVirtualFile(B("/dev/dsp"), openDevDsp, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, k_mdev(14, 3), devNode);
	Fs::addVirtualFile(B("/dev/mixer"), openDevMixer, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, k_mdev(14, 0), devNode);
    Fs::addVirtualFile(B("/dev/sequencer"), openDevSequencer, K__S_IWRITE | K__S_IREAD | K__S_IFCHR, k_mdev(14, 1), devNode);    

    Fs::addVirtualFile(B("/etc/hostname"), openHostname, K__S_IREAD, k_mdev(0, 0), etcNode);
    Fs::addVirtualFile(B("/etc/hosts"), openHosts, K__S_IREAD, k_mdev(0, 0), etcNode);
}

std::vector<BString> StartUpArgs::buildArgs() {
    std::vector<BString> args;

    if (root.length()) {
        args.push_back(B("-root"));
        args.push_back(root);
    }
    for (auto& z : zips) {
        args.push_back(B("-zip"));
        args.push_back(z);
    }
    if (title.length()) {
        args.push_back(B("-title"));
        args.push_back(title);
    }
    if (userId != UID) {
        args.push_back(B("-uid"));
        args.push_back(BString::valueOf(userId));
    }
    if (effectiveUserId != UID) {
        args.push_back(B("-euid"));
        args.push_back(BString::valueOf(effectiveUserId));
    }
    if (nozip) {
        args.push_back(B("-nozip"));
    }
    if (workingDir.length()) {
        args.push_back(B("-w"));
        args.push_back(workingDir);
    }
    if (sdlFullScreen == FULLSCREEN_STRETCH) {
        args.push_back(B("-fullscreen"));
    }
    if (vsync != VSYNC_DEFAULT) {
        args.push_back(B("-vsync"));
        args.push_back(BString::valueOf(vsync));
    }
    if (sdlFullScreen == FULLSCREEN_ASPECT) {
        args.push_back(B("-fullscreenAspect"));
    }
    if (sdlScaleX != 100) {
        args.push_back(B("-scale"));
        args.push_back(BString::valueOf(sdlScaleX));
    }
    if (sdlScaleQuality.length() && sdlScaleQuality != "0") {
        args.push_back(B("-scale_quality"));
        args.push_back(sdlScaleQuality);
    }
    args.push_back(B("-resolution"));
    args.push_back(BString::valueOf(screenCx) + "x" + BString::valueOf(screenCy));
    if (screenBpp != 32) {
        args.push_back(B("-bpp"));
        args.push_back(BString::valueOf(screenBpp));
    }
    if (glExt.length()) {
        args.push_back(B("-glext"));
        args.push_back(glExt);
    }
    if (dpiAware) {
        args.push_back(B("-dpiAware"));
    }
    if (openGlType == OPENGL_TYPE_OSMESA) {
        args.push_back(B("-mesa"));
    }
    if (ttyPrepend) {
        args.push_back(B("-ttyPrepend"));
    }
    if (recordAutomation.length()) {
        args.push_back(B("-record"));
        args.push_back(recordAutomation);
    }
    if (runAutomation.length()) {
        args.push_back(B("-automation"));
        args.push_back(runAutomation);
    }
    if (showWindowImmediately) {
        args.push_back(B("-showWindowImmediately"));
    }
    if (skipFrameFPS) {
        args.push_back(B("-skipFrameFPS"));
        args.push_back(BString::valueOf(skipFrameFPS));
    }
    if (cpuAffinity) {
        args.push_back(B("-cpuAffinity"));
        args.push_back(BString::valueOf(cpuAffinity));
    }
    if (pollRate > 0) {
        args.push_back(B("-pollRate"));
        args.push_back(BString::valueOf(this->pollRate));
    }
    for (auto& e : envValues) {
        args.push_back(B("-env"));
        args.push_back(e);
    }
    if (logPath.c_str()) {
        args.push_back(B("-log"));
        args.push_back(logPath);
    }
    for (auto& m : mountInfo) {
        if (m.wine) {
            args.push_back(B("-mount_drive"));
        } else {
            args.push_back(B("-mount"));
        }
        args.push_back(m.nativePath);
        args.push_back(m.localPath);
    }
    for (auto& a : this->args) {
        args.push_back(a);
    }
    return args;
}

bool StartUpArgs::apply() {
    KSystem::init();    
#ifdef BOXEDWINE_MULTI_THREADED
    KSystem::cpuAffinityCountForApp = this->cpuAffinity;
    if (KSystem::cpuAffinityCountForApp) {
        klog("CPU Affinity set to %d", KSystem::cpuAffinityCountForApp);
    }
#endif
    KSystem::pentiumLevel = this->pentiumLevel;
    KSystem::pollRate = this->pollRate;
    if (KSystem::pollRate < 0) {
        KSystem::pollRate = 0;
    }
    KSystem::openglType = this->openGlType;
    KSystem::ttyPrepend = this->ttyPrepend;
    KSystem::showWindowImmediately = this->showWindowImmediately;
    KSystem::skipFrameFPS = this->skipFrameFPS;
    if (!KSystem::logFile.isOpen() && this->logPath.length()) {
        KSystem::logFile.createNew(this->logPath);
    }

    for (U32 f=0;f<nonExecFileFullPaths.size();f++) {
        FsFileNode::nonExecFileFullPaths.insert(nonExecFileFullPaths[f]);
    }
#ifdef BOXEDWINE_RECORDER
    if (this->recordAutomation.length()) {
        Recorder::start(this->recordAutomation);
    }
    if (this->runAutomation.length()) {
        Player::start(this->runAutomation);
    }
    BOXEDWINE_RECORDER_INIT(this->root, this->zips, this->workingDir, this->args);
#endif

    klog("Using root directory: %s", root.c_str());
#ifdef BOXEDWINE_ZLIB
    std::vector<BString> fullfilled;
    for (auto& zip : zips) {
        BString fullfills;
        FsZip::readFileFromZip(zip, B("fullfills.txt"), fullfills);
        if (fullfills.length()) {
            fullfilled.push_back(fullfills);
        }
    }
    std::vector<BString> depends;
    for (auto& zip : zips) {
        BString depend;
        FsZip::readFileFromZip(zip, B("depends.txt"), depend);
        if (depend.length() && !vectorContainsIgnoreCase(depends, depend) && !vectorContainsIgnoreCase(zips, depend) && !vectorContainsIgnoreCase(fullfilled, depend)) {
            BString originalDepend = depend;
            if (!Fs::doesNativePathExist(depend)) {
                BString parentPath = Fs::getNativeParentPath(zip);
                depend = parentPath ^ depend;
            }
            if (!Fs::doesNativePathExist(depend)) {
                klog("%s depends on %s, and %s could not be found", zip.c_str(), originalDepend.c_str(), originalDepend.c_str());
            }
            depends.push_back(depend);
        }
    }
    if (depends.size()) {
        zips.insert(zips.end(), depends.begin(), depends.end());
    }
#endif
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
        fsZip->init(zip, B(""));
        openZips.push_back(fsZip);
        U64 endTime = KSystem::getMicroCounter();
        klog("Loaded %s in %d ms", zip.c_str(), (U32)(endTime - startTime) / 1000);
    }

    std::shared_ptr<FsNode> wineVersionNode = Fs::getNodeFromLocalPath(B(""), B("/wineVersion.txt"), false);
    if (wineVersionNode) {
        FsOpenNode* openNode = wineVersionNode->open(K_O_RDONLY);
        if (openNode) {
            U8 tmp[64];
            if (openNode->readNative(tmp, 64) > 5) {
                KSystem::wineMajorVersion = tmp[5] - '0';
                if (tmp[5] == '2' || tmp[5] == '1') {
                    std::shared_ptr<FsNode> freeTypeNode = Fs::getNodeFromLocalPath(B(""), B("/usr/lib/i386-linux-gnu/libfreetype.so.6"), false);
                    if (freeTypeNode) {
                        freeTypeNode->link = B("libfreetype.so.6.12.3");
                    }
                }
            }
            openNode->close();
        }
    }
#endif
    KSystem::title = title;

    buildVirtualFileSystem();

    envValues.push_back(B("HOME=/home/username"));
    envValues.push_back(B("LOGNAME=username"));
    envValues.push_back(B("USERNAME=username"));
    envValues.push_back(B("USER=username"));
    envValues.push_back("PWD="+this->workingDir);
    envValues.push_back(B("DISPLAY=:0"));
    envValues.push_back(B("WINE_FAKE_WAIT_VBLANK=60"));
    //envValues.push_back(B("WINEDLLOVERRIDES=mscoree,mshtml="));
    //envValues.push_back("WINEDEBUG=+ddraw");
                            
    // if this strlen is more than 88 (1 more character than now), then diablo demo will crash before we get to the menu
    // if I create more env values that are longer it doesn't crash, what is special about this one?
    
    //crashes, if I change LD_LIBRARY_PATH to LD_LIBRARY_PATT it works, so it's not the length of the env string, but rather specific to LD_LIBRARY_PATH
    //envValues.push_back("LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib:/lib/i386-linux-gnu:/usr/lib/i386-linux-gnu:/opt/wine/lib");

    //works
    //envValues.push_back("LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib:/lib/i386-linux-gnu:/usr/lib/i386-linux-gnu");        
    if (userId==0) {
        envValues.push_back(B("PATH=/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/usr/local/sbin"));
    } else {
        envValues.push_back(B("PATH=/bin:/usr/bin:/usr/local/bin"));
    }

    for(auto&& info: this->mountInfo) {
        if (info.wine) {
            std::shared_ptr<FsNode> mntDir = Fs::getNodeFromLocalPath(B(""), B("/mnt"), true);
            std::shared_ptr<FsNode> drive_d = Fs::addRootDirectoryNode("/mnt/drive_"+info.localPath, info.nativePath, mntDir);
            std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), B("/home/username/.wine/dosdevices"), true);
            Fs::addFileNode("/home/username/.wine/dosdevices/"+info.localPath+":", "/mnt/drive_"+info.localPath, B(""), false, parent); 
        } else {
            BString ext = info.nativePath.substr(info.nativePath.length()-4).toLowerCase();
            if (ext == ".zip") {
    #ifdef BOXEDWINE_ZLIB
                U64 startTime = KSystem::getMicroCounter();
                std::shared_ptr<FsZip> fsZip = std::make_shared<FsZip>();
                if (!info.localPath.endsWith("/", false)) {
                    info.localPath= info.localPath+"/";
                }
                fsZip->init(info.nativePath, info.localPath);
                openZips.push_back(fsZip);
                U64 endTime = KSystem::getMicroCounter();
                klog("Mounted %s in %d ms", info.nativePath.c_str(), (U32)(endTime - startTime) / 1000);
    #else
                klog("% not mounted because zlib was not compiled in", info.nativePath.c_str());
    #endif
            } else {
                std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), Fs::getParentPath(info.localPath), true);
                Fs::addRootDirectoryNode(info.localPath, info.nativePath, parent);
            }
        }
    }

    if (this->args.size()==0) {
        args.push_back(B("/bin/wine"));
        args.push_back(B("explorer"));
        args.push_back(B("/desktop=shell"));
    }
    if (this->args.size()) {
        std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(workingDir, this->args[0], true);        

        bool validLinuxCommand = false;
        if (node) {
            BString interpreter;
            BString loader;
            std::vector<BString> interpreterArgs;
            std::vector<BString> args;

            FsOpenNode* openNode=ElfLoader::inspectNode(workingDir, node, loader, interpreter, interpreterArgs);
            if (openNode) {
                openNode->close();
                validLinuxCommand = true;
            }
        }
        if (!validLinuxCommand) {
            if (Fs::doesNativePathExist(args[0])) {
                BString dir = args[0];
                dir = Fs::trimTrailingSlash(dir);
                bool isDir = Fs::isNativePathDirectory(dir);
                std::shared_ptr<FsNode> mntDir = Fs::getNodeFromLocalPath(B(""), B("/mnt"), true);

                if (!isDir) {
                    dir = Fs::getNativeParentPath(dir);
                }
                
                std::shared_ptr<FsNode> drive_d = Fs::addRootDirectoryNode(B("/mnt/drive_t"), dir, mntDir);
                std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), B("/home/username/.wine/dosdevices"), true);
                if (parent) {
                    Fs::addFileNode(B("/home/username/.wine/dosdevices/t:"), B("/mnt/drive_t"), B(""), false, parent);
                }

                if (!workingDirSet) {
                    workingDir = B("/mnt/drive_t");
                }
            
                if (isDir) {
                    args.clear();
                    args.push_back(B("/bin/wine"));
                    args.push_back(B("explorer"));
                    args.push_back(B("t:\\"));
                } else {
                    BString fileName = Fs::getFileNameFromNativePath(args[0]);
                    args.erase(args.begin());
                    args.insert(args.begin(), "t:\\" + fileName);
                    if (fileName.endsWith(".msi", true)) {
                        args.insert(args.begin(), B("start"));
                    }
                    args.insert(args.begin(), B("/bin/wine"));
                }
            }
        }
    }
    if (this->sdlFullScreen!=FULLSCREEN_NOTSET && !this->resolutionSet) {
        U32 width = 0;
        U32 height = 0;
        if (KNativeSystem::getScreenDimensions(&width, &height)) {
#ifdef __ANDROID__
            this->screenCx = width / 2;
            this->screenCy = height / 2;
#else
            this->screenCx = width;
            this->screenCy = height;
#endif
        }
    }
    KSystem::videoEnabled = this->videoEnabled;
    KSystem::soundEnabled = this->soundEnabled;
    KNativeWindow::init(this->screenCx, this->screenCy, this->screenBpp, this->sdlScaleX, this->sdlScaleY, this->sdlScaleQuality, this->sdlFullScreen, this->vsync);
    initWine();
    initWineAudio();
    KNativeAudio::init();
#ifdef BOXEDWINE_OPENGL
    gl_init(this->glExt);        
#endif   
#ifdef BOXEDWINE_VULKAN
    vulkan_init();
#endif

    if (this->args.size()) {
        klog_nonewline("Launching ");
        for (U32 i=0;i<this->args.size();i++) {
            klog_nonewline("\"%s\" ", this->args[i].c_str());
        }
        klog_nonewline("\n");
        bool result = false;
        {
            std::shared_ptr<KProcess> process = KProcess::create();// keep in this small scope so we don't hold onto it for the life of the program
            result = process->startProcess(this->workingDir, this->args, this->envValues, this->userId, this->groupId, this->effectiveUserId, this->effectiveGroupId);
        }
        if (result) {
            if (!doMainLoop()) {
                return false; // doMainLoop should have handled any cleanup, like SDL_Quit if necessary
            }
        }
    }
#ifdef GENERATE_SOURCE
    if (gensrc)
        writeSource();
#endif    
	KSystem::destroy();
    KNativeWindow::shutdown();
    KNativeAudio::shutdown();
    dspShutdown();

#ifdef BOXEDWINE_ZLIB
    openZips.clear();
#endif    
    return true;
}

bool StartUpArgs::loadDefaultResource(const char* app) {
    BString cmd = Platform::getResourceFilePath(B("cmd.txt"));
    static std::vector<BString> lines;
    lines.clear();
    lines.push_back(BString::copy(app));
    if (!cmd.isEmpty() && readLinesFromFile(cmd, lines)) {
        int count = (int)lines.size();
        const char** ppArgs = new const char*[count];
        for (int i=0;i<count;i++) {
            ppArgs[i] = lines[i].c_str();
            if (lines[i] == "-zip" && i+1<count) {
                if (!Fs::doesNativePathExist(lines[i+1])) {
                    BString zip = Platform::getResourceFilePath(lines[i+1]);
                    if (!zip.isEmpty() && Fs::doesNativePathExist(zip)) {
                        i++;
                        ppArgs[i] = strdup(zip.c_str()); // I'm not worried about leaking this
                    }
                }
            }
        }
        return parseStartupArgs((int)lines.size(), ppArgs);
    }
    return true;
}

bool StartUpArgs::parseStartupArgs(int argc, const char **argv) {
    int i;
    // look for -log as soon as possible so that logging is enabled as soon as possible
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-log") && i + 1 < argc) {
            this->logPath = BString::copy(argv[i + 1]);
            KSystem::logFile.createNew(logPath);
            i++;
        }
    }
    klog_nonewline("Command line arguments:");
    for (i = 0; i < argc; i++) {
        klog_nonewline(" \"%s\"", argv[i]);
    }
    klog_nonewline("\n");
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i], "-root") && i+1<argc) {
            this->setRoot(BString::copy(argv[i+1]));
            i++;
        } else if (!strcmp(argv[i], "-zip") && i+1<argc) {
#ifdef BOXEDWINE_ZLIB
            this->addZip(BString::copy(argv[i+1]));
#else
            kwarn("BoxedWine wasn't compiled with zlib support");
#endif
            i++;
        } else if (!strcmp(argv[i], "-title") && i + 1 < argc) {
            this->title = BString::copy(argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "-nozip") && i + 1 < argc) {
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
            this->setWorkingDir(BString::copy(argv[i+1]));
            i++;
            this->workingDirSet = true;
        } else if (!strcmp(argv[i], "-nosound")) {
			this->soundEnabled = false;
        } else if (!strcmp(argv[i], "-novideo")) {
			this->videoEnabled = false;
        } else if (!strcmp(argv[i], "-env")) {
			this->envValues.push_back(BString::copy(argv[i+1]));
            i++;
        } else if (!strcmp(argv[i], "-fullscreen")) {
			this->setFullscreen(FULLSCREEN_STRETCH);
        } else if (!strcmp(argv[i], "-fullscreenAspect")) {
            this->setFullscreen(FULLSCREEN_ASPECT);
        } else if (!strcmp(argv[i], "-vsync")) {
            this->vsync = atoi(argv[i + 1]);
            i++;
            if (this->vsync < 0 || this->vsync > 2) {
                klog("-vsync must be 0, 1 or 2 (0=disabled, 1=enabled, 2=adaptive)");
                this->vsync = VSYNC_DEFAULT;
            }
        } else if (!strcmp(argv[i], "-scale")) {
			this->setScale(atoi(argv[i+1]));
            if (!this->resolutionSet) {
                this->screenCx = 640;
                this->screenCy = 480;
            }
            i++;
        } else if (!strcmp(argv[i], "-scale_quality")) {
			this->setScaleQuality(BString::copy(argv[i+1]));
            i++;
        } else if (!strcmp(argv[i], "-resolution")) {
            setResolution(BString::copy(argv[i+1]));
            i++;
        } else if (!strcmp(argv[i], "-bpp")) {
            setBpp(atoi(argv[i+1]));
            i++;
            if (this->screenBpp!=16 && this->screenBpp!=32 && this->screenBpp!=8) {
                klog("-bpp must be 8, 16 or 32");
                this->screenBpp = 32;
            }
        } else if (!strcmp(argv[i], "-noexecfiles")) {
            B(argv[i + 1]).split(':', this->nonExecFileFullPaths);        
            i++;
        } else if (!strcmp(argv[i], "-p2")) {
            this->pentiumLevel = 2;
        } else if (!strcmp(argv[i], "-p3")) {
            this->pentiumLevel = 3;
        } else if (!strcmp(argv[i], "-glext")) {
            this->setAllowedGlExtension(BString::copy(argv[i+1]));
            i++;
        } else if (!strcmp(argv[i], "-rel_mouse_sensitivity")) {
            this->rel_mouse_sensitivity = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-mount_drive")) {
            if (strlen(argv[i+2])!=1) {
                klog("-mount_drive expects 2 parameters: <host directory to mount> <drive letter to use for wine>");
                klog("  example: -mount_drive \"c:\\my games\" d");
            } else {
                this->mountInfo.push_back(MountInfo(BString::copy(argv[i+2]), BString::copy(argv[i+1]), true));
            }
            i+=2;
        } else if (!strcmp(argv[i], "-mount")) {
            if (argv[i+2][0]!='/') {
                klog("-mount expects 2 parameters: <host directory to mount or zip file> <full path on root file>\n");
                klog("example: -mount \"c:\\my games\" \"/home/username/my games\"");
                klog("example: -mount_zip \"c:\\my games\\mygame.zip\" /mnt/game");
            } else {
                if (Fs::doesNativePathExist(BString::copy(argv[i + 2]))) {
                    klog("mount directory/file does not exist: %s", argv[i + 2]);
                    return false;
                }
                this->mountInfo.push_back(MountInfo(BString::copy(argv[i+2]), BString::copy(argv[i+1]), false));
            }
            i+=2;
        } else if (!strcmp(argv[i], "-showStartupWindow")) {
            // no longer used
        } else if (!strcmp(argv[i], "-ui")) {
            i++;
            if (!strcmp(argv[i], "opengl")) {
                this->uiType = UI_TYPE_OPENGL;
            }
        } else if (!strcmp(argv[i], "-dpiAware")) {
            dpiAware = true;
        } else if (!strcmp(argv[i], "-showWindowImmediately")) {
            showWindowImmediately = true;
        } else if (!strcmp(argv[i], "-pollRate")) {
            this->pollRate = atoi(argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "-mesa")) {
            this->openGlType = OPENGL_TYPE_OSMESA;
        }
        else if (!strcmp(argv[i], "-ttyPrepend")) { // used to send tty back to WaitDlg for winetricks when BOXEDWINE_UI_LAUNCH_IN_PROCESS is not defined
            this->ttyPrepend = true;
        } else if (!strcmp(argv[i], "-cpuAffinity")) {
#ifdef BOXEDWINE_MULTI_THREADED
            this->cpuAffinity = atoi(argv[i+1]);
#else
            klog("ignoring -cpuAffinity");
#endif
            i++;
        } else if (!strcmp(argv[i], "-skipFrameFPS") && i+1<argc) {
            this->skipFrameFPS = atoi(argv[i+1]);
            i++;
        } else if (!strcmp(argv[i], "-log") && i + 1 < argc) {
            this->logPath = BString::copy(argv[i + 1]);
            i++;
        } 
#ifdef BOXEDWINE_RECORDER
        else if (!strcmp(argv[i], "-record")) {
            if (!Fs::doesNativePathExist(BString::copy(argv[i+1]))) {
                static_cast<void>(MKDIR(argv[i+1])); // return result ignored
                if (!Fs::doesNativePathExist(BString::copy(argv[i+1]))) {
                    klog("-record path does not exist and could not be created: %s", argv[i+1]);
                    return false;
                }
            }
            this->recordAutomation = BString::copy(argv[i + 1]);
            i++;
        }  else if (!strcmp(argv[i], "-automation")) {
            if (!Fs::doesNativePathExist(BString::copy(argv[i+1]))) {
                klog("-automation directory does not exist %s", argv[i+1]);
                return false;
            }
            this->runAutomation = BString::copy(argv[i + 1]);
            i++;
        }
#endif
        else {
            break;
        }
    } 
    char curdir[1024];
    char* base = getcwd(curdir, sizeof(curdir));
    char pathSeperator = '/';

    if (base!=nullptr && strchr(base, '\\') != nullptr) {
        pathSeperator = '\\';
    }
    if (KNativeSystem::getAppDirectory().length()) {
        BString base2 = KNativeSystem::getAppDirectory();
        base2 = base2.substr(0, base2.length()-1); 
        if (zips.size()==0 && !nozip) {
            std::vector<Platform::ListNodeResult> results;
            if (base) {
                Platform::listNodes(BString::copy(base), results);
                for (auto&& item : results) {
                    if (strstr(item.name.c_str(), "Wine") && strstr(item.name.c_str(), ".zip")) {
                        this->zips.push_back(base + pathSeperator + item.name);
                        break;
                    }
                }
            }
            if (zips.size()==0) {
                results.clear();
                Platform::listNodes(base2, results);
                for (auto&& item : results) {
                    if (strstr(item.name.c_str(), "Wine") && strstr(item.name.c_str(), ".zip")) {
                        this->zips.push_back(BString(base2) + pathSeperator + item.name);
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
        this->root=KNativeSystem::getLocalDirectory()+"root";
#endif
    }  

    argc = argc-i;
    argv = &argv[i];
        
    for (i=0;i<argc;i++) {
        args.push_back(BString::copy(argv[i]));
    }
    return true;
}

void StartUpArgs::setResolution(BString resolution) {
    U32 width;
    U32 height;

    int success = parse_resolution(resolution.c_str(), &width, &height);
    if (success) {
        this->screenCx = width;
        this->screenCy = height;
        this->resolutionSet = true;                
    }
}
