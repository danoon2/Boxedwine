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
    SdlCallback* callback = allocSdlCallback();
    callback->pfn = p;
    BOXEDWINE_CONDITION_LOCK(callback->cond);
    SDL_PushEvent(&callback->sdlEvent);
    BOXEDWINE_CONDITION_WAIT(callback->cond);
    BOXEDWINE_CONDITION_UNLOCK(callback->cond);
    U32 result = callback->result;
    freeSdlCallback(callback);    
    return result;
}

#endif