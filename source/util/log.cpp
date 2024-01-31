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
#include "knativethread.h"
#include "knativesystem.h"

#include <stdio.h>

#ifdef BOXEDWINE_MSVC
#include <Windows.h>
#endif

void internal_kpanic(BString msg) {
    if (KSystem::logFile) {
        fwrite(msg.c_str(), 1, msg.length(), KSystem::logFile);
    }
    fwrite(msg.c_str(), 1, msg.length(), stderr);

#ifdef BOXEDWINE_MSVC
    OutputDebugStringA(msg.c_str());
#endif
    if (KSystem::videoEnabled) {
#ifndef _TEST
        KNativeSystem::exit(msg.c_str(), 1);
#endif
    } else {
        KNativeThread::sleep(5000);
    }
    exit(1);
}

void internal_log(BString msg, FILE* f) {
    if (KSystem::logFile) {
        fwrite(msg.c_str(), 1, msg.length(), KSystem::logFile);
    }
    fwrite(msg.c_str(), 1, msg.length(), f);

#ifdef BOXEDWINE_MSVC
    OutputDebugStringA(msg.c_str());
#endif
}
