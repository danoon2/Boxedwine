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
#ifndef BOXEDWINE_MULTI_THREADED
#include "recorder.h"
#include "knativesocket.h"
#include "knativesystem.h"
#include "knativethread.h"

#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
#include "../../ui/mainui.h"
#endif

static U32 lastTitleUpdate = 0;
bool isMainthread() {
    return true;
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
bool doMainLoop() {
    bool shouldQuit = false;

    while (KSystem::getProcessCount()>0 && !shouldQuit) {
        bool ran = runSlice();
        U32 t;

        BOXEDWINE_RECORDER_RUN_SLICE();
        if (!KNativeSystem::getCurrentInput()->processEvents()) {
            shouldQuit = true;
            break;
        }
        KNativeSystem::tick();
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
        if (uiIsRunning()) {
            uiLoop();
        }
#endif
        t = KSystem::getMilliesSinceStart();

        if (KSystem::killTime) {
            if (KSystem::killTime <= t) {
                KSystem::killTime = 0;
                KSystem::killTime2 = KSystem::getMilliesSinceStart() + 30000;
                KNativeSystem::forceShutdown();
            }
        }
        if (KSystem::killTime2) {
            if (KSystem::killTime2 <= t) {
                klog("Forced Shutdown failed, now doing a hard exit");
                return true;
            }
        }
        if (lastTitleUpdate+5000 < t) {            
            lastTitleUpdate = t;
            if (KSystem::title.length()) {
                KNativeSystem::getScreen()->setTitle(KSystem::title);
            } else {
                BString title = B("BoxedWine " BOXEDWINE_VERSION_DISPLAY );
                title.append(" MIPS");
                title.append(getMIPS());
                title.append(" : ");
                title.append(getSize(allocatedRamPages));
                KNativeSystem::getScreen()->setTitle(title);
            }            
        }
        if (ran) {
            checkWaitingNativeSockets(0);
        } else {
            if (KSystem::getRunningProcessCount()==0) {
                break;
            }
            if (!checkWaitingNativeSockets(20)) {
                KNativeThread::sleep(20);
            }
        }
    }
    return true;
}
#endif
