#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../util/threadutils.h"
#include "knativethread.h"
#ifndef BOXEDWINE_UI_LAUNCH_IN_PROCESS
#include "../../../lib/tiny-process/process.hpp"
#endif

std::vector<BoxedContainer*> BoxedwineData::containers;
std::vector<BoxedWinVersion> BoxedwineData::winVersions;

BoxedWinVersion::BoxedWinVersion(BString ver, BString desc, U32 major, U32 minor, U32 build, U32 id, BString csd, U16 majorPack, U16 minorPack, BString product) : szVersion(ver), szDescription(desc), dwMajorVersion(major), dwMinorVersion(minor), dwBuildNumber(build), dwPlatformId(id), szCSDVersion(csd), wServicePackMajor(majorPack), wServicePackMinor(minorPack), szProductType(product) {
}

void BoxedwineData::init(int argc, const char **argv) {
    GlobalSettings::init(argc, argv);

    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win10"), B("Windows 10"), 10, 0, 0x42EE, VER_PLATFORM_WIN32_NT, B(""), 0, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win81"), B("Windows 8.1"), 6, 3, 0x2580, VER_PLATFORM_WIN32_NT, B(""), 0, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win8"), B("Windows 8"), 6, 2, 0x23F0, VER_PLATFORM_WIN32_NT, B(""), 0, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win2008r2"), B("Windows 2008 R2"), 6, 1, 0x1DB1, VER_PLATFORM_WIN32_NT, B("Service Pack 1"), 1, 0, B("ServerNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win7"), B("Windows 7"), 6, 1, 0x1DB1, VER_PLATFORM_WIN32_NT, B("Service Pack 1"), 1, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win2008"), B("Windows 2008"), 6, 0, 0x1772, VER_PLATFORM_WIN32_NT, B("Service Pack 2"), 2, 0, B("ServerNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("vista"), B("Windows Vista"), 6, 0, 0x1772, VER_PLATFORM_WIN32_NT, B("Service Pack 2"), 2, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win2003"), B("Windows 2003"), 5, 2, 0xECE, VER_PLATFORM_WIN32_NT, B("Service Pack 2"), 2, 0, B("ServerNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("winxp"), B("Windows XP"), 5, 1, 0xA28, VER_PLATFORM_WIN32_NT, B("Service Pack 3"), 3, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win2k"), B("Windows 2000"), 5, 0, 0x893, VER_PLATFORM_WIN32_NT, B("Service Pack 4"), 4, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("winme"), B("Windows ME"), 4, 90, 0xBB8, VER_PLATFORM_WIN32_WINDOWS, B(" "), 0, 0, B("")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win98"), B("Windows 98"), 4, 10, 0x8AE, VER_PLATFORM_WIN32_WINDOWS, B(" A "), 0, 0, B("")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win95"), B("Windows 95"), 4, 0, 0x3B6, VER_PLATFORM_WIN32_WINDOWS, B(""), 0, 0, B("")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("nt40"), B("Windows NT 4.0"), 4, 0, 0x565, VER_PLATFORM_WIN32_NT, B("Service Pack 6a"), 6, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("nt351"), B("Windows NT 3.51"), 3, 51, 0x421, VER_PLATFORM_WIN32_NT, B("Service Pack 5"), 5, 0, B("WinNT")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win31"), B("Windows 3.1"), 3, 10, 0, VER_PLATFORM_WIN32s, B("Win32s 1.3"), 0, 0, B("")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win30"), B("Windows 3.0"), 3, 0, 0, VER_PLATFORM_WIN32s, B("Win32s 1.3"), 0, 0, B("")));
    BoxedwineData::winVersions.push_back(BoxedWinVersion(B("win20"), B("Windows 2.0"), 2, 0, 0, VER_PLATFORM_WIN32s, B("Win32s 1.3"), 0, 0, B("")));    
}

BoxedWinVersion* BoxedwineData::getWinVersionFromName(BString name) {
    for (auto& win : BoxedwineData::winVersions) {
        if (win.szVersion == name) {
            return &win;
        }
    }
    return nullptr;
}

void BoxedwineData::startApp() {
#ifdef BOXEDWINE_UI_LAUNCH_IN_PROCESS
    GlobalSettings::startUpArgs.apply();
    if (!GlobalSettings::keepUIRunning && uiIsRunning()) {
        uiShutdown();
    }
#else
    std::vector<BString> a = GlobalSettings::startUpArgs.buildArgs();
    BString log;
    BString cmd = GlobalSettings::getExeFilePath();

    log = "Starting process: " + GlobalSettings::getExeFilePath() + " ";
    cmd += " ";
    for (auto& arg : a) {
        log += "\"" + arg + "\" ";
        cmd += "\"" + arg + "\" ";
    }
    log += "\n";

    BString out;
    bool windowCreated = false;
    TinyProcessLib::Process process(cmd.c_str(), "", [&out, &windowCreated](const char* bytes, size_t n) {
        for (U32 i=0;i<n;i++) {
            out += bytes[i];
            if (bytes[i] != '\n') {
                continue;
            }
            if (out.contains("Creating Window")) {
                windowCreated = true;
            }
            if (KSystem::watchTTY && out.startsWith("TTY:")) {
                BString line = out.substr(4);
                std::vector<BString> parts;
                line.split(':', parts);
                if (parts.size() >= 2 && parts[0] != "curl") {
                    line = line.substr(parts[0].length()+1);
                    KSystem::watchTTY(line);
                }
            }
            out = "";
        }
    });
    int status = 0;
    while (!process.try_get_exit_status(status)) {
        if (windowCreated) {
            if (uiIsRunning()) {
                uiShutdown();
            }
        } else {
            uiLoop();
        }
        KNativeThread::sleep(16);
    }  
    if (!GlobalSettings::keepUIRunning && uiIsRunning()) {
        uiShutdown();
    }
#endif
}

void BoxedwineData::reloadContainers() {
    loadContainers();
    GlobalSettings::reloadApps();
}

void BoxedwineData::loadContainers() {
     for (auto &container : BoxedwineData::containers) {
        delete container;
    }
    BoxedwineData::containers.clear();
    Fs::iterateAllNativeFiles(GlobalSettings::getContainerFolder(), false, true, [] (BString filepath, bool isDir)->U32 {
        if (isDir) {
            BoxedContainer* container = new BoxedContainer();
            if (container->load(filepath)) {
                BoxedwineData::containers.push_back(container);
            } else {
                delete container;
            }
        }
        return 0;
    });
    BoxedwineData::sortContainers();
}

void BoxedwineData::loadUI() {
    loadContainers();
}

BoxedContainer* BoxedwineData::getContainerByDir(BString dir) {
    for (auto& container : BoxedwineData::containers) {
        if (container->getDir()==dir) {
            return container;
        }
    }
    return nullptr;
}

void BoxedwineData::addContainer(BoxedContainer* container) { 
    BoxedwineData::containers.push_back(container); 
    BoxedwineData::sortContainers();
}

void BoxedwineData::sortContainers() {
    std::sort(BoxedwineData::containers.begin(), BoxedwineData::containers.end(), [](BoxedContainer* a, BoxedContainer* b) { 
        return a->getName().compareTo(b->getName(), true) < 0;
        });
}