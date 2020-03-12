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
#include "devdsp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include MKDIR_INCLUDE

#include <SDL.h>

#include "startupArgs.h"
#ifndef BOXEDWINE_DISABLE_UI
#include "../ui/mainui.h"
#include "../ui/data/boxedwineData.h"
#include "../ui/data/globalSettings.h"
#endif

#ifndef __TEST

U32 getMilliesSinceStart() {
    return SDL_GetTicks();
}

U32 gensrc;

#ifdef GENERATE_SOURCE
void writeSource();
#endif

int boxedmain(int argc, const char **argv) {
    StartUpArgs startupArgs;                  

    klog("Starting ...");

    Platform::startMicroCounter();
#ifdef LOG_OPS
    logFile = fopen("log.txt", "w");
#endif
#ifndef BOXEDWINE_DISABLE_UI
    BoxedwineData::init(argc, argv);
#endif
    if (!startupArgs.parseStartupArgs(argc, argv)) {
        return 1;
    }        

#ifdef _DEBUG
    U32 cpuCount = Platform::getCpuCount();
    if (cpuCount==1) {
        klog("%d MHz CPU detected", Platform::getCpuFreqMHz());
    } else {
        klog("%dx %d MHz CPUs detected", cpuCount, Platform::getCpuFreqMHz());
    }
#endif

    #ifdef SDL2
    U32 flags = SDL_INIT_EVENTS;
#else
    U32 flags = SDL_INIT_TIMER;
#endif
    if (startupArgs.videoEnabled) {
        flags|=SDL_INIT_VIDEO;
    }
    if (startupArgs.soundEnabled) {
        flags|=SDL_INIT_AUDIO;
    }
    if (SDL_Init(flags) != 0) {
        klog("SDL_Init Error: %s", SDL_GetError());
        return 0;
    }

    if (!startupArgs.shouldStartUI()) {
        if (!startupArgs.apply()) {
            return 1;
        }
    } else {
#ifndef BOXEDWINE_DISABLE_UI
        while (uiShow(GlobalSettings::getExePath()+Fs::nativePathSeperator)) {
            BoxedwineData::startApp();
            GlobalSettings::startUpArgs.readyToLaunch = false;

            // make sure if the user closed the SDL windows for the game/app, that it doesn't carry over into the UI
            SDL_PumpEvents();
            SDL_FlushEvent(SDL_QUIT);
        }
#endif
    }              

    klog("Boxedwine shutdown");
    SDL_Quit();
    return BOXEDWINE_RECORDER_QUIT();
}

int main(int argc, char **argv) {
    return boxedmain(argc, (const char **)argv);
}

#endif
