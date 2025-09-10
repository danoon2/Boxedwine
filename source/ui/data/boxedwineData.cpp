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
#include "../boxedwineui.h"
#include "../../util/threadutils.h"
#include "knativethread.h"
#include "knativesystem.h"
#ifndef BOXEDWINE_UI_LAUNCH_IN_PROCESS
#include "../../../lib/tiny-process/process.hpp"
#endif

std::vector<BoxedContainer*> BoxedwineData::containers;
std::vector<BoxedWinVersion> BoxedwineData::winVersions;

BoxedWinVersion::BoxedWinVersion(const char* ver, const char* desc, const char* currentVersion, U32 major, U32 minor, U32 build, U32 id, const char* csd, U16 majorPack, U16 minorPack, const char* product) : szVersion(ver), szDescription(desc), szCurrentVersion(currentVersion), dwMajorVersion(major), dwMinorVersion(minor), dwBuildNumber(build), dwPlatformId(id), szCSDVersion(csd), wServicePackMajor(majorPack), wServicePackMinor(minorPack), szProductType(product) {
}

void BoxedwineData::init(int argc, const char **argv) {
    GlobalSettings::init(argc, argv);

    BoxedwineData::winVersions.push_back(BoxedWinVersion("win11", "Windows 11", "6.3", 10, 0, 22000, VER_PLATFORM_WIN32_NT, "", 0, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win10", "Windows 10", "6.3", 10, 0, 19043, VER_PLATFORM_WIN32_NT, "", 0, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win81", "Windows 8.1", NULL, 6, 3, 9600, VER_PLATFORM_WIN32_NT, "", 0, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win8", "Windows 8", NULL, 6, 2, 9200, VER_PLATFORM_WIN32_NT, "", 0, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win2008r2", "Windows 2008 R2", NULL, 6, 1, 7601, VER_PLATFORM_WIN32_NT, "Service Pack 1", 1, 0, "ServerNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win7", "Windows 7", NULL, 6, 1, 7601, VER_PLATFORM_WIN32_NT, "Service Pack 1", 1, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win2008", "Windows 2008", NULL, 6, 0, 6002, VER_PLATFORM_WIN32_NT, "Service Pack 2", 2, 0, "ServerNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("vista", "Windows Vista", NULL, 6, 0, 6002, VER_PLATFORM_WIN32_NT, "Service Pack 2", 2, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win2003", "Windows 2003", NULL, 5, 2, 3790, VER_PLATFORM_WIN32_NT, "Service Pack 2", 2, 0, "ServerNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("winxp64", "Windows XP 64", NULL, 5, 2, 3790, VER_PLATFORM_WIN32_NT, "Service Pack 2", 2, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("winxp", "Windows XP", NULL, 5, 1, 2600, VER_PLATFORM_WIN32_NT, "Service Pack 3", 3, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win2k", "Windows 2000", NULL, 5, 0, 2195, VER_PLATFORM_WIN32_NT, "Service Pack 4", 4, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("winme", "Windows ME", NULL, 4, 90, 3000, VER_PLATFORM_WIN32_WINDOWS, " ", 0, 0, ""));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win98", "Windows 98", NULL, 4, 10, 2222, VER_PLATFORM_WIN32_WINDOWS, " A ", 0, 0, ""));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win95", "Windows 95", NULL, 4, 0, 950, VER_PLATFORM_WIN32_WINDOWS, "", 0, 0, ""));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("nt40", "Windows NT 4.0", NULL, 4, 0, 1381, VER_PLATFORM_WIN32_NT, "Service Pack 6a", 6, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("nt351", "Windows NT 3.51", NULL, 3, 51, 1057, VER_PLATFORM_WIN32_NT, "Service Pack 5", 5, 0, "WinNT"));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win31", "Windows 3.1", NULL, 3, 10, 0, VER_PLATFORM_WIN32s, "Win32s 1.3", 0, 0, ""));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win30", "Windows 3.0", NULL, 3, 0, 0, VER_PLATFORM_WIN32s, "Win32s 1.3", 0, 0, ""));
    BoxedwineData::winVersions.push_back(BoxedWinVersion("win20", "Windows 2.0", NULL, 2, 0, 0, VER_PLATFORM_WIN32s, "Win32s 1.3", 0, 0, ""));
}

BoxedWinVersion* BoxedwineData::getWinVersionFromName(BString name) {
    for (auto& win : BoxedwineData::winVersions) {
        if (name == win.szVersion) {
            return &win;
        }
    }
    return nullptr;
}

#if defined(__MACH__)
extern "C" {
int MacPlatformLaunchAnotherInstance(void);
int MacPlatformIsTaskRunning(void);
int MacPlatformIsTaskFinishedLaunching(void);
}
#endif

void BoxedwineData::startApp() {
#ifdef BOXEDWINE_UI_LAUNCH_IN_PROCESS
    GlobalSettings::startUpArgs.apply();
    if (!GlobalSettings::keepUIRunning && uiIsRunning()) {
        uiShutdown();
    }
#else

#if defined(__MACH__)
    std::vector<BString> args = GlobalSettings::startUpArgs.buildArgs();
    BString now;
    now.append((U32)(KSystem::getSystemTimeAsMicroSeconds() / 100000));
    args.insert(args.begin(), now);
    writeLinesToFile(GlobalSettings::getArgsPath(), args);
    MacPlatformLaunchAnotherInstance();
    BString filePath = GlobalSettings::getDataFolder().stringByApppendingPath(now+".txt");
    bool started = false;
    U32 startTime = KSystem::getMilliesSinceStart();
    bool windowCreated = false;
    
    while (true) {
        bool running = MacPlatformIsTaskRunning();
        if (!started) {
            if (running) {
                started = true;
            } else {
                if (KSystem::getMilliesSinceStart() - startTime > 10000) {
                    break;
                }
            }
        } else if (!running) {
            break; // we started but are no longer running
        }
        if (windowCreated) {
            if (uiIsRunning()) {
                uiShutdown();
            }
        } else {
            if (Fs::doesNativePathExist(filePath)) {
                windowCreated = true;
                Fs::deleteNativeFile(filePath);
            }
            uiLoop();
        }
        uiPumpEvents(); // without this, swift won't get the isTerminated msg
        KNativeThread::sleep(16);
    }
    Fs::deleteNativeFile(filePath);
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
    /*
    TinyProcessLib::Process::environment_type environment;
    
    for (auto env = environ; *env != 0; env++) {
        std::string line = *env;

        auto equality_sign_pos = line.find('=', 0);
        auto variable_name = line.substr(0, equality_sign_pos);
        auto variable_value = line.substr(equality_sign_pos + 1, line.length());

        environment[variable_name] = variable_value;
    }
    environment["LIBGL_ALWAYS_SOFTWARE"] = "1";
    */
    TinyProcessLib::Process process(cmd.c_str(), "", [&out, &windowCreated](const char* bytes, size_t n) {
        for (U32 i=0;i<n;i++) {
            out += bytes[i];
            if (bytes[i] != '\n') {
                continue;
            }
            if (out.contains("Showing Window")) {
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
