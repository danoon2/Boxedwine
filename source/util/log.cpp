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

#include <stdio.h>
#include <stdarg.h>
#include "knativethread.h"
#include "knativesystem.h"

#ifdef BOXEDWINE_MSVC
#include <Windows.h>
#endif

void kpanic(const char* msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
    
    if (KSystem::logFile) {
        va_list argptr2;
        va_copy(argptr2, argptr);
        vfprintf(KSystem::logFile, msg, argptr2);
        fflush(KSystem::logFile);
        va_end(argptr2);
    }
    vfprintf(stderr, msg, argptr);
    fprintf(stderr, "\n");
    if (KSystem::logFile) {
        fprintf(KSystem::logFile, "\n");
        fflush(KSystem::logFile);
        fclose(KSystem::logFile);
    }
    char buff[1024];
    vsnprintf(buff, sizeof(buff), msg, argptr);
#ifdef BOXEDWINE_MSVC
    OutputDebugStringA(buff);
    OutputDebugStringA("\n");
#endif
    va_end(argptr);
    if (KSystem::videoEnabled) {
        KNativeSystem::exit(buff, 1);
    } else {
        KNativeThread::sleep(5000);
    }
}

void kwarn(const char* msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
    BOXEDWINE_CRITICAL_SECTION;

    if (KSystem::logFile) {
        va_list argptr2;
        va_copy(argptr2, argptr);
        vfprintf(KSystem::logFile, msg, argptr2);
        fflush(KSystem::logFile);
        va_end(argptr2);
    }
    vfprintf(stdout, msg, argptr);
    fprintf(stdout, "\n");
    if (KSystem::logFile) {
        fprintf(KSystem::logFile, "\n");
    }
#ifdef BOXEDWINE_MSVC
    char buff[1024];
    vsnprintf(buff, sizeof(buff), msg, argptr);
    OutputDebugStringA(buff);
    OutputDebugStringA("\n");
#endif
    va_end(argptr);
}

void kdebug(const char* msg, ...) {
#ifdef _DEBUG
    va_list argptr;
    va_start(argptr, msg);
    BOXEDWINE_CRITICAL_SECTION;
    
    if (KSystem::logFile) {
        va_list argptr2;
        va_copy(argptr2, argptr);
        vfprintf(KSystem::logFile, msg, argptr2);
        fflush(KSystem::logFile);
        va_end(argptr2);
    }
    vfprintf(stderr, msg, argptr);
    fprintf(stderr, "\n");
    if (KSystem::logFile) {
        fprintf(KSystem::logFile, "\n");
    }
#ifdef BOXEDWINE_MSVC
    char buff[1024];
    vsnprintf(buff, sizeof(buff), msg, argptr);
    OutputDebugStringA(buff);
    OutputDebugStringA("\n");
#endif
    va_end(argptr);
#endif
}

void klog_nonewline(const char* msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
    BOXEDWINE_CRITICAL_SECTION;

    
    if (KSystem::logFile) {
        va_list argptr2;
        va_copy(argptr2, argptr);
        vfprintf(KSystem::logFile, msg, argptr2);
        fflush(KSystem::logFile);
        va_end(argptr2);
    }
    vfprintf(stdout, msg, argptr);
    fflush(stdout);
#ifdef BOXEDWINE_MSVC
    char buff[1024];
    vsnprintf(buff, sizeof(buff), msg, argptr);
    OutputDebugStringA(buff);
#endif
    va_end(argptr);
}

void klog(const char* msg, ...) {
    va_list argptr;
    va_start(argptr, msg);
    BOXEDWINE_CRITICAL_SECTION;
    
    if (KSystem::logFile) {
        va_list argptr2;
        va_copy(argptr2, argptr);
        vfprintf(KSystem::logFile, msg, argptr2);
        va_end(argptr2);
    }
    vfprintf(stdout, msg, argptr);
    
    fprintf(stdout, "\n");
    if (KSystem::logFile) {
        fprintf(KSystem::logFile, "\n");
        fflush(KSystem::logFile);
    }
    fflush(stdout);
#ifdef BOXEDWINE_MSVC
    char buff[1024];
    vsnprintf(buff, sizeof(buff), msg, argptr);
    OutputDebugStringA(buff);
    OutputDebugStringA("\n");
#endif
    va_end(argptr);
}
