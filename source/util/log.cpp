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
#include "knativethread.h"
#include "knativesystem.h"

#include <stdio.h>
#include <cstdlib>

#ifdef BOXEDWINE_MSVC
#include <Windows.h>
#endif

static BOXEDWINE_MUTEX logMutex;

void kpanic(const char* msg) {
    internal_kpanic(BString::copy(msg));
}

void internal_kpanic(BString msg) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(logMutex);
    if (KSystem::logFile.isOpen()) {
        KSystem::logFile.write(msg);
        // std::_Exit and KNativeSystem::exit below skip stdio cleanup, so the message would stay in the buffer
        KSystem::logFile.flush();
    }
    fwrite(msg.c_str(), 1, msg.length(), stderr);
    fflush(stderr);
#if defined(BOXEDWINE_MSVC) && defined(_DEBUG)
    OutputDebugStringA(msg.c_str());
#endif
    if (KSystem::noErrorDialog) {
        // Exit right away. Other threads keep running during a sleep, so the automation player or
        // the main loop could end the process first with a different exit code.
        fflush(stdout);
        std::_Exit(1);
    }
    if (KSystem::videoOption == VIDEO_NORMAL) {
#ifndef _TEST
        KNativeSystem::exit(msg.c_str(), 1);
#endif
    } else {
        KNativeThread::sleep(5000);
    }
    exit(1);
}

void kwarn(const char* msg) {
    internal_log(BString::copy(msg) + "\n", stdout);
}

void klog(const char* msg) {
    internal_log(BString::copy(msg) + "\n", stdout);
}

void klog_nonewline(const char* msg) {
    internal_log(BString::copy(msg), stdout);
}

void internal_log(BString msg, FILE* f) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(logMutex);
    if (KSystem::logFile.isOpen()) {
        KSystem::logFile.write(msg);
    }
    fwrite(msg.c_str(), 1, msg.length(), f);
    fflush(f);
#if defined(BOXEDWINE_MSVC) && defined(_DEBUG)
    OutputDebugStringA(msg.c_str());
#endif
}

void flushLogFileIfDue() {
    // Writes stay buffered for speed; flushing about once a second bounds what a killed or crashed process loses
    static U32 lastFlush;
    U32 now = KSystem::getMilliesSinceStart();
    if (KSystem::logFile.isOpen() && now - lastFlush >= 1000) {
        lastFlush = now;
        KSystem::logFile.flush();
    }
}
