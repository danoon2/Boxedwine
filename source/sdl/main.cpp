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

#include "startupArgs.h"
#ifndef BOXEDWINE_DISABLE_UI
#include "../ui/mainui.h"
#include "../ui/data/boxedwineData.h"
#include "../ui/data/globalSettings.h"
#endif
#include "knativesystem.h"

#ifdef BOXEDWINE_MSVC
#include <Windows.h>
#endif

#ifndef __TEST

U32 gensrc;

#ifdef GENERATE_SOURCE
void writeSource();
#endif

int boxedmain(int argc, const char **argv) {
    StartUpArgs startupArgs;                  

    klog("Starting ...");    
    KSystem::startMicroCounter();
    KSystem::exePath = BString::copy(argv[0]);
    if (KSystem::exePath.contains("\\")) {
        KSystem::exePath = KSystem::exePath.substr(0, KSystem::exePath.lastIndexOf('\\')+1);
    } else {
        KSystem::exePath = KSystem::exePath.substr(0, KSystem::exePath.lastIndexOf('/')+1);
    }
    if (argc == 1) {
        if (!startupArgs.loadDefaultResource(argv[0])) {
            return 1;
        }
        
    } else if (!startupArgs.parseStartupArgs(argc, argv)) {
        return 1;
    }
    
#ifdef BOXEDWINE_MSVC
#ifdef BOXEDWINE_DISABLE_UI    
    if (startupArgs.dpiAware) {
        SetProcessDPIAware();
    }
#else
    if (startupArgs.shouldStartUI() || startupArgs.dpiAware) {
        SetProcessDPIAware();
    }
#endif
#endif

#ifdef _DEBUG
    U32 cpuCount = Platform::getCpuCount();
    if (cpuCount==1) {
        klog("%d MHz CPU detected", Platform::getCpuFreqMHz());
    } else {
        klog("%dx %d MHz CPUs detected", cpuCount, Platform::getCpuFreqMHz());
    }
#endif

    Platform::init();
    if (!KNativeSystem::init(startupArgs.videoEnabled, startupArgs.soundEnabled)) {
        return 1;
    }
#ifndef BOXEDWINE_DISABLE_UI
    BoxedwineData::init(argc, argv);
#endif
    if (!startupArgs.shouldStartUI()) {
        if (!startupArgs.apply()) {
            return 1;
        }
    } else {
#ifndef BOXEDWINE_DISABLE_UI

#ifdef BOXEDWINE_MSVC
        if (StartUpArgs::uiType == UI_TYPE_UNSET) {
#ifdef BOXEDWINE_IMGUI_DX9
            StartUpArgs::uiType = UI_TYPE_DX9;
#else
            StartUpArgs::uiType = UI_TYPE_OPENGL;
#endif
        }
#else
        if (StartUpArgs::uiType == UI_TYPE_UNSET) {
            StartUpArgs::uiType = UI_TYPE_OPENGL;
        }
#endif
        while (true) {
            if (GlobalSettings::keepUIRunning) {
                GlobalSettings::keepUIRunning();
                GlobalSettings::keepUIRunning = nullptr;
                if (!uiContinue()) {
                    break;
                }
            } else {
                if (!uiShow(GlobalSettings::getExePath() + Fs::nativePathSeperator)) {
                    break;
                }
            }
            if (GlobalSettings::restartUI) {
                GlobalSettings::restartUI = false;
                if (GlobalSettings::reinit) {
                    GlobalSettings::reinit = false;
                    GlobalSettings::init(argc, argv);
                } else {
                    GlobalSettings::startUp();
                }
                continue;
            }
            BoxedwineData::startApp();
            GlobalSettings::startUpArgs.readyToLaunch = false;

            KNativeSystem::preReturnToUI();
            if (!GlobalSettings::keepUIRunning) {
                GlobalSettings::startUp(); // we we come back in after launching a game, we will need to create icons, like the demo icons
            }
        }
#endif
    }              

    klog("Boxedwine shutdown");
    KNativeSystem::cleanup();
    return BOXEDWINE_RECORDER_QUIT();
}

#endif
