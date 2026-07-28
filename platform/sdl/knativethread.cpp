/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <pthread.h>
#endif

static int sdl_start_thread(void* ptr) {
	KNativeThread* thread = (KNativeThread*)ptr;
	return thread->pfn(thread->data);
}

KNativeThread* KNativeThread::createAndStartThread(KNativeThreadFunction pfn, BString name, void* data) {
	KNativeThread* result = new KNativeThread(pfn, name, data);

	result->nativeThread = SDL_CreateThread(sdl_start_thread, name.c_str(), result);
	return result;
}

int KNativeThread::wait() {
	int threadReturnValue = 0;
	SDL_WaitThread((SDL_Thread*)this->nativeThread, &threadReturnValue);
	return threadReturnValue;
}

void KNativeThread::sleep(U32 ms) {
	SDL_Delay(ms);
}

#ifdef BOXEDWINE_MULTI_THREADED

#if defined(_DEBUG) && defined(BOXEDWINE_MSVC)
void platformSetThreadDescription(KThread* thread);
#endif

S32 platformStartThread(KThread* thread, PlatformThreadFunction entry) {
    CPU* cpu = thread->cpu;
#ifdef __EMSCRIPTEN__
    pthread_t cppThread;
    int result = pthread_create(&cppThread, nullptr, entry, cpu);
    if (result) {
        return result;
    }
    cpu->nativeHandle = (U64)cppThread;
#ifndef __TEST
    pthread_detach(cppThread);
#endif
#elif defined(__TEST)
    cpu->nativeHandle = (U64)new std::thread(entry, cpu);
#else
    std::thread cppThread = std::thread(entry, cpu);
    cpu->nativeHandle = (U64)cppThread.native_handle();
#if defined(_DEBUG) && defined(BOXEDWINE_MSVC)
    platformSetThreadDescription(thread);
#endif
    cppThread.detach();
#endif
    return 0;
}

#ifdef __TEST
void platformJoinThread(KThread* thread) {
#ifdef __EMSCRIPTEN__
    pthread_join((pthread_t)thread->cpu->nativeHandle, nullptr);
#else
    std::thread* cppThread = (std::thread*)thread->cpu->nativeHandle;
    cppThread->join();
    delete cppThread;
#endif
}
#endif

#endif
