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
#ifdef BOXEDWINE_MULTI_THREADED

#include <SDL.h>
#include "sdlcallback.h"

extern U32 sdlCustomEvent;
extern SDL_threadID sdlMainThreadId;

static SdlCallback* freeSdlCallbacks;
static BOXEDWINE_MUTEX freeSdlCallbacksMutex;

SdlCallback* allocSdlCallback() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(freeSdlCallbacksMutex);
    if (freeSdlCallbacks) {
        SdlCallback* result = freeSdlCallbacks;
        freeSdlCallbacks = freeSdlCallbacks->next;
        return result;
    } else {
        SdlCallback* result = new SdlCallback();
        result->sdlEvent.type = sdlCustomEvent;
        result->sdlEvent.user.data1 = result;
        return result;
    }    
}

void freeSdlCallback(SdlCallback* callback) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(freeSdlCallbacksMutex);
    callback->next = freeSdlCallbacks;
    freeSdlCallbacks = callback;
}

U32 sdlDispatch(std::function<U32()> p) {
    if (isMainthread()) {
        return p();
    }
    SdlCallback* callback = allocSdlCallback();
    callback->pfn = p;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(callback->cond);
        SDL_PushEvent(&callback->sdlEvent);
        BOXEDWINE_CONDITION_WAIT(callback->cond);
    }
    U32 result = callback->result;
    callback->pfn = nullptr;
    freeSdlCallback(callback);    
    return result;
}

#endif
