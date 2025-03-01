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
#include <SDL.h>

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