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
#include "knativeinput.h"
#include "knativeaudio.h"
#include "knativesocket.h"

#ifndef BOXEDWINE_DISABLE_UI
#include "../ui/data/globalSettings.h"
#endif

#include MKDIR_INCLUDE
#include CURDIR_INCLUDE

void gl_init(BString allowExtensions);
void vulkan_init();
void x11_init();
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
    std::shared_ptr<FsNode> devNode = Fs::addFileNode(B("/dev"), B(""), rootNode->nativePath.stringByApppendingPath("dev"), true, rootNode);
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
    if (openGlLib.length()) {
        args.push_back(B("-opengl"));
        args.push_back(openGlLib);
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
    if (!this->ddrawOverridePath.isEmpty()) {
        args.push_back(B("-ddrawOverride"));
        args.push_back(this->ddrawOverridePath);
    }
    args.push_back(B("-dxvk"));
    args.push_back(B(this->enableDXVK ? "1" : "0"));
    if (this->disableHideCursor) {
        args.push_back(B("-disableHideCursor"));
    }
    if (this->forceRelativeMouse) {
        args.push_back(B("-forceRelativeMouse"));
    }
    if (this->cacheReads) {
        args.push_back(B("-cacheReads"));
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
        klog_fmt("CPU Affinity set to %d", KSystem::cpuAffinityCountForApp);
    }
#endif
    KSystem::disableHideCursor = this->disableHideCursor;
    KSystem::forceRelativeMouse = this->forceRelativeMouse;
    KSystem::cacheReads = this->cacheReads;
    KSystem::pentiumLevel = this->pentiumLevel;
    KSystem::pollRate = this->pollRate;
    if (KSystem::pollRate < 0) {
        KSystem::pollRate = 0;
    }
    KSystem::openglLib = this->openGlLib;
    KSystem::ttyPrepend = this->ttyPrepend;
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

    klog_fmt("Using root directory: %s", root.c_str());
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
                depend = parentPath.stringByApppendingPath(depend);
            }
            if (!Fs::doesNativePathExist(depend)) {
                klog_fmt("%s depends on %s, and %s could not be found", zip.c_str(), originalDepend.c_str(), originalDepend.c_str());
            }
            depends.push_back(depend);
        }
    }
    if (depends.size()) {
        zips.insert(zips.end(), depends.begin(), depends.end());
    }
#endif
    for (auto& zip : zips) {
        klog_fmt("Using zip file system: %s", zip.c_str());
    }
    if (!Fs::initFileSystem(root)) {
        kwarn_fmt("root %s does not exist", root.c_str());
        return false;
    }
    if (this->resolutionSet) {
        klog_fmt("Resolution set to: %dx%d", this->screenCx, this->screenCy);
    }
#ifdef BOXEDWINE_ZLIB
    std::vector<std::shared_ptr<FsZip>> openZips;
    for (auto& zip : zips) {
        U64 startTime = KSystem::getMicroCounter();
        std::shared_ptr<FsZip> fsZip = std::make_shared<FsZip>();
        fsZip->init(zip, B(""));
        openZips.push_back(fsZip);
        U64 endTime = KSystem::getMicroCounter();
        klog_fmt("Loaded %s in %d ms", zip.c_str(), (U32)(endTime - startTime) / 1000);
    }

    std::shared_ptr<FsNode> wineVersionNode = Fs::getNodeFromLocalPath(B(""), B("/wineVersion.txt"), false);
    if (wineVersionNode) {
        FsOpenNode* openNode = wineVersionNode->open(K_O_RDONLY);
        if (openNode) {
            U8 tmp[64];
            U32 len = openNode->readNative(tmp, 64);
            if (len > 2) {
                BString wineVersion;
                wineVersion.append((char*)tmp, len);
                std::vector<BString> parts;
                wineVersion.split('.', parts);
                if (parts.size() == 2) {
                    KSystem::wineMajorVersion = parts[0].toInt();
                }
                if (KSystem::wineMajorVersion == 2 || KSystem::wineMajorVersion == 1) {
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

    if (!this->ddrawOverridePath.isEmpty()) {
        envValues.push_back(B("WINEDLLOVERRIDES=ddraw=n,b"));
        std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(BString::empty, this->ddrawOverridePath, true);
        if (!parent) {
            klog_fmt("-ddrawOverride %s not found", this->ddrawOverridePath.c_str());
        }
        std::shared_ptr<FsNode> ddrawParent = Fs::getNodeFromLocalPath(BString::empty, B("/home/username/.wine/drive_c/ddraw"), true);
        if (!ddrawParent) {
            klog("-ddrawOverride was specificied but /home/username/.wine/drive_c/ddraw was not found in the file system");
        }
        if (parent && ddrawParent) {
            Fs::addFileNode(this->ddrawOverridePath + "/ddraw.dll", B("/home/username/.wine/drive_c/ddraw/ddraw.dll"), ddrawParent->nativePath.stringByApppendingPath("ddraw.dll"), false, parent);
            Fs::addFileNode(this->ddrawOverridePath + "/ddraw.ini", B("/home/username/.wine/drive_c/ddraw/ddraw.ini"), ddrawParent->nativePath.stringByApppendingPath("ddraw.ini"), false, parent);
        }
    }
    if (this->enableDXVK) {
        envValues.push_back(B("DXVK_LOG_LEVEL=warn"));
        envValues.push_back(B("WINEDLLOVERRIDES=d3d11,d3d10core,d3d9,d3d8,dxgi=n,b"));
        std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(BString::empty, B("/home/username/.wine/drive_c/windows/system32"), true);
        std::shared_ptr<FsNode> dxvkParent = Fs::getNodeFromLocalPath(BString::empty, B("/home/username/.wine/drive_c/dxvk"), true);
        if (!dxvkParent) {
            klog("-dxvk was enabled but not found in the file system");
        } else {
            for (const char* pName : { "d3d8.dll", "d3d9.dll", "d3d10core.dll", "d3d11.dll", "dxgi.dll" }) {
               Fs::addFileNode(parent->path + "/" + pName, dxvkParent->path + "/" + pName, dxvkParent->nativePath + "/" + pName, false, parent);
            }
        }
    }

    // automatically tell wine to use ddraw.dll if its in the same directory as the app being launched
    // this is a quality of life issue so that the user doesn't have to manually add this with winecfg
    // gog's hellfire uses this
    bool foundWineDllOverride = false;

    for (auto& value : envValues) {
        if (value.startsWith("WINEDLLOVERRIDES")) {
            foundWineDllOverride = true;
        }
    }
    if (!foundWineDllOverride) {
        if (args.size() >= 2 && args[0] == "/bin/wine") {
            std::shared_ptr<FsNode> appNode = Fs::getNodeFromLocalPath(workingDir, args[1], true);
            if (appNode) {
                std::shared_ptr<FsNode> parentNode = appNode->getParent().lock();
                if (parentNode && parentNode->getChildByNameIgnoreCase(B("ddraw.dll"))) {
                    envValues.push_back(B("WINEDLLOVERRIDES=ddraw=n,b"));
                    klog("automatically applied WINEDLLOVERRIDES=ddraw=n,b");
                }
            }
        }
    }
    //envValues.push_back(B("WINEDEBUG=+wgl"));
     
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
                klog_fmt("Mounted %s in %d ms", info.nativePath.c_str(), (U32)(endTime - startTime) / 1000);
    #else
                klog_fmt("% not mounted because zlib was not compiled in", info.nativePath.c_str());
    #endif
            } else {
                Fs::makeLocalDirs(info.localPath);
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
    KSystem::videoOption = this->videoOption;
    KSystem::soundEnabled = this->soundEnabled;
#ifdef __EMSCRIPTEN__
    if (KSystem::soundEnabled) {
        KSystem::soundEnabled = false;
        KSystem::enableSoundAfterMouseClick = true;
    }
#endif
    KNativeSystem::initWindow(this->screenCx, this->screenCy, this->screenBpp, this->sdlScaleX, this->sdlScaleY, this->sdlScaleQuality, this->sdlFullScreen, this->vsync);
    KNativeAudio::init();
#ifdef BOXEDWINE_OPENGL
    PlatformOpenGL::init();
    gl_init(this->glExt);        
#endif   
#ifdef BOXEDWINE_VULKAN
    vulkan_init();
#endif
    x11_init();

    if (this->args.size()) {
        klog_nonewline("Launching ");
        for (U32 i=0;i<this->args.size();i++) {
            klog_nonewline_fmt("\"%s\" ", this->args[i].c_str());
        }
        klog_nonewline("\n");
        bool result = false;
        {
            KProcessPtr process = KProcess::create();// keep in this small scope so we don't hold onto it for the life of the program
            KThread* thread = process->startProcess(this->workingDir, this->args, this->envValues, this->userId, this->groupId, this->effectiveUserId, this->effectiveGroupId);
            result = thread != nullptr;
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
    KNativeSystem::shutdown();
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
        klog_nonewline_fmt(" \"%s\"", argv[i]);
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
#ifdef BOXEDWINE_MSVC
            this->videoOption = VIDEO_HIDE_WINDOW;
#else
            this->videoOption = VIDEO_NO_WINDOW;
#endif
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
                    klog_fmt("mount directory/file does not exist: %s", argv[i + 2]);
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
        } else if (!strcmp(argv[i], "-pollRate")) {
            this->pollRate = atoi(argv[i + 1]);
            i++;
        } else if (!strcmp(argv[i], "-opengl")) {
            this->openGlLib = argv[i+1];
            i++;
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
                    klog_fmt("-record path does not exist and could not be created: %s", argv[i+1]);
                    return false;
                }
            }
            this->recordAutomation = BString::copy(argv[i + 1]);
            i++;
        }  else if (!strcmp(argv[i], "-automation")) {
            if (!Fs::doesNativePathExist(BString::copy(argv[i+1]))) {
                klog_fmt("-automation directory does not exist %s", argv[i+1]);
                return false;
            }
            this->runAutomation = BString::copy(argv[i + 1]);
            i++;
        }
#endif
        else if (!strcmp(argv[i], "-ddrawOverride")) {
            this->ddrawOverridePath = argv[i + 1];
            i++;
        } else if (!strcmp(argv[i], "-disableHideCursor")) {
            this->disableHideCursor = true;
        } else if (!strcmp(argv[i], "-forceRelativeMouse")) {
            this->forceRelativeMouse = true;
        }  else if (!strcmp(argv[i], "-cacheReads")) {
            this->cacheReads = true;
        }
        else if (!strcmp(argv[i], "-dxvk")) {
            BString dxvk;
            dxvk = argv[i + 1];
            this->enableDXVK = dxvk.startsWith('t', true) || (dxvk == "1") || dxvk.startsWith('y', true);
            i++;
        } else {
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

void StartUpArgs::setOpenGlType(U32 type) {
    if (type == OPENGL_TYPE_DEFAULT) {
        type = GlobalSettings::getDefaultOpenGL();
    }
#ifdef __MACH__
    if (type != OPENGL_TYPE_NATIVE) {
        GlobalSettings::startUpArgs.openGlLib = "osmesa";
    }
#endif
#ifdef BOXEDWINE_MSVC
    if (type != OPENGL_TYPE_NATIVE) {
        GlobalSettings::startUpArgs.openGlLib = GlobalSettings::alternativeOpenGlLocation();
        if (type == OPENGL_TYPE_LLVM_PIPE) {
            putenv("GALLIUM_DRIVER=llvmpipe");
        } else if (type == OPENGL_TYPE_ON_D3D12) {
            putenv("GALLIUM_DRIVER=d3d12");
        } else if (type == OPENGL_TYPE_ON_VULKAN) {
            putenv("GALLIUM_DRIVER=zink");
        }
    }
#endif
}
