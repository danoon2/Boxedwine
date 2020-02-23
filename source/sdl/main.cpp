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
#include "sdlwindow.h"

#include "mainloop.h"
#include "startupArgs.h"

void gl_init(const std::string& allowExtensions);

#ifndef __TEST

U32 getMilliesSinceStart() {
    return SDL_GetTicks();
}

U32 gensrc;

#ifdef GENERATE_SOURCE
void writeSource();
#endif

void initWine();

int boxedmain(int argc, const char **argv) {
    StartUpArgs startupArgs;                  

    klog("Starting ...");

    Platform::startMicroCounter();
#ifdef LOG_OPS
    logFile = fopen("log.txt", "w");
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

    if (!startupArgs.apply()) {
        return 1;
    }   
    
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
    if (startupArgs.showStartingWindow) {
        showSDLStartingWindow();
    }

#ifdef SDL2
    if (startupArgs.sdlFullScreen && !startupArgs.resolutionSet) {
        SDL_DisplayMode mode;
        if (!SDL_GetCurrentDisplayMode(0, &mode)) {
#ifdef __ANDROID__
            startupArgs.screenCx = mode.w/2;
            startupArgs.screenCy = mode.h/2;
#else
            startupArgs.screenCx = mode.w;
            startupArgs.screenCy = mode.h;
#endif
        }
    }
#endif

    initSDL(startupArgs.screenCx, startupArgs.screenCy, startupArgs.screenBpp, startupArgs.sdlScaleX, startupArgs.sdlScaleY, startupArgs.sdlScaleQuality, startupArgs.soundEnabled, startupArgs.videoEnabled);
    initWine();
#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
    gl_init(startupArgs.glExt);    
#endif   

    klog("Launching %s", argv[0]);
    KProcess* process = new KProcess(KSystem::nextThreadId++);    
    if (process->startProcess(startupArgs.workingDir, startupArgs.args, startupArgs.envValues, startupArgs.userId, startupArgs.groupId, startupArgs.effectiveUserId, startupArgs.effectiveGroupId)) {
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
