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

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include "boxedwine.h"
#include "knativesocket.h"
#include "knativesystem.h"

#ifdef BOXEDWINE_MULTI_THREADED
#include "knativethread.h"

U32 getNextTimer();
void runTimers();

extern std::atomic<int> platformThreadCount;

static U32 lastTitleUpdate = 0;
static thread_local bool isMainThread;

extern int allocatedRamPages;

bool isMainthread() {
    return isMainThread;
}

static BString getSize(int pages)
{
    pages *= 4;
    if (pages < 2048) {
        return BString::valueOf(pages) + B("KB");
    }
    if (pages < 2048 * 1024) {
        return BString::valueOf(pages / 1024) + B("MB");
    }
    return BString::valueOf(pages / 1024 / 1024) + B("GB");
}
extern int allocatedRamPages;
void mainloop() {
    isMainThread = true;
        U32 t = KSystem::getMilliesSinceStart();
        U32 nextTimer = getNextTimer();
        if (nextTimer == 0) {
            runTimers();
        }
           
        if (lastTitleUpdate + 5000 < t) {
            lastTitleUpdate = t;
            BString title;
            if (KSystem::title.length()) {
                title = KSystem::title;
            } else {
                title = B("BoxedWine " BOXEDWINE_VERSION_DISPLAY);
            }

            title.append(" ");
            title.append(getSize(allocatedRamPages));
	    emscripten_set_window_title(title.c_str());
        }
        if (!KNativeSystem::getCurrentInput()->processEvents()) {
            KNativeSystem::cleanup();
        }
}

void waitForProcessToFinish(const std::shared_ptr<KProcess>& process, KThread* thread) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
    while (!process->isTerminated()) {
        BOXEDWINE_CONDITION_WAIT(KSystem::processesCond);
    }
}

#else

static U32 lastTitleUpdate = 0;
static bool mainLoopTimingConfigured = false;

bool isMainthread() {
    return true;
}

void mainloop() {
    if (!mainLoopTimingConfigured) {
        emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 1);
        mainLoopTimingConfigured = true;
    }
    U64 startTime = KSystem::getMicroCounter();
    U32 t;
    U32 count=0;
    BString mipsTitle;
    while (1) {
        bool ran;
        try {
            ran = runSlice();
        } catch (...) {
            if (!recoverRunSliceException()) {
                throw;
            }
            break;
        }

        KNativeSystem::tick();
#ifdef BOXEDWINE_MULTI_THREADED
        checkWaitingNativeSockets(0);
#endif
        if (!KNativeSystem::getCurrentInput()->processEvents()) {
            KNativeSystem::cleanup();
            return;
        }
        t = KSystem::getMilliesSinceStart();                
        if (lastTitleUpdate+1000 < t) {
            lastTitleUpdate = t;
	        mipsTitle = B("BoxedWine ");
	        mipsTitle.append(getMIPS());
	        mipsTitle.append(" MIPS");
	        emscripten_set_window_title(mipsTitle.c_str());           
        }
        if (!ran) {
            break;
        }
        U64 diff = KSystem::getMicroCounter() - startTime;

        KOpenGLPtr openGL = KNativeSystem::getOpenGL();
        if (diff>10000 || KNativeSystem::getScreen()->presentedSinceLastCheck() || (openGL && openGL->presentedSinceLastCheck())) {
            if (diff > 100000) {
                klog_fmt("ran main loop in %dms", (U32)diff / 1000);
            }
            break;
        }
    };
}

#endif

bool doMainLoop() {
    EM_ASM(
#ifndef SDL2
            SDL.defaults.copyOnLock = false;
            SDL.defaults.discardOnLock = true;
#endif
            //SDL.defaults.opaqueFrontBuffer = false;
    );
    emscripten_set_main_loop(mainloop, 0, 1);
    return false;
}
#endif
