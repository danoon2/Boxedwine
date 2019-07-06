#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED
#include "../sdlwindow.h"
#include <SDL.h>
#include "sdlcallback.h"
#include "devfb.h"

U32 sdlCustomEvent;
SDL_threadID sdlMainThreadId;
extern U32 platformThreadCount;
extern U32 exceptionCount;
extern U32 dynamicCodeExceptionCount;
static U32 lastTitleUpdate = 0;

bool doMainLoop() {
    SDL_Event e;

    sdlCustomEvent = SDL_RegisterEvents(1);
    sdlMainThreadId = SDL_ThreadID();

    while (platformThreadCount) {
#ifdef BOXEDWINE_RECORDER
        if (Player::instance || Recorder::instance) {
            SDL_WaitEventTimeout(&e, 10);
            BOXEDWINE_RECORDER_RUN_SLICE();
        } else  {
            SDL_WaitEventTimeout(&e, 5000);
        }
#else
        SDL_WaitEventTimeout(&e, 5000);
#endif      
        U32 t = getMilliesSinceStart();
        if (lastTitleUpdate+5000 < t) {
            char tmp[256];
            lastTitleUpdate = t;
            sprintf(tmp, "BoxedWine 19R1 Beta1a %u/%u", dynamicCodeExceptionCount, exceptionCount);
            fbSetCaption(tmp, "BoxedWine");
        }
        if (e.type == sdlCustomEvent) {
            SdlCallback* callback = (SdlCallback*)e.user.data1;
            callback->result = (U32)callback->pfn();
            BOXEDWINE_CONDITION_LOCK(callback->cond);
            BOXEDWINE_CONDITION_SIGNAL(callback->cond);
            BOXEDWINE_CONDITION_UNLOCK(callback->cond);
        } else if (!handlSdlEvent(&e)) {
            return true;
        }
    };
    return true;
}

void waitForProcessToFinish(KProcess* process, KThread* thread) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
    while (!process->isTerminated()) {
        BOXEDWINE_CONDITION_WAIT(KSystem::processesCond);
    }
}

#endif