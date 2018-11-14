#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED

#include <SDL.h>
#include "sdlcallback.h"

extern U32 sdlCustomEvent;
extern SDL_threadID sdlMainThreadId;

static struct SdlCallback* freeSdlCallbacks;
static SDL_mutex* freeSdlCallbacksMutex;

struct SdlCallback* allocSdlCallback() {
    if (!freeSdlCallbacksMutex)
        freeSdlCallbacksMutex = SDL_CreateMutex();
    SDL_LockMutex(freeSdlCallbacksMutex);
    if (freeSdlCallbacks) {
        struct SdlCallback* result = freeSdlCallbacks;
        freeSdlCallbacks = freeSdlCallbacks->next;
        SDL_UnlockMutex(freeSdlCallbacksMutex);
        return result;
    } else {
        struct SdlCallback* result = (struct SdlCallback*)malloc(sizeof(struct SdlCallback));
        memset(result, 0, sizeof(struct SdlCallback));
        result->sdlEvent.type = sdlCustomEvent;
        result->sdlEvent.user.data1 = result;
        SDL_UnlockMutex(freeSdlCallbacksMutex);
        return result;
    }    
}

void freeSdlCallback(struct SdlCallback* callback) {
    SDL_LockMutex(freeSdlCallbacksMutex);
    callback->next = freeSdlCallbacks;
    freeSdlCallbacks = callback;
    SDL_UnlockMutex(freeSdlCallbacksMutex);
}

U32 sdlDispatch(std::function<U32()> p) {
    struct SdlCallback* callback = allocSdlCallback();
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