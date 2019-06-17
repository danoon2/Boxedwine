#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED
#include "../sdlwindow.h"
#include <SDL.h>
#include "sdlcallback.h"

U32 sdlCustomEvent;
SDL_threadID sdlMainThreadId;
extern U32 platformThreadCount;

bool doMainLoop() {
    SDL_Event e;

    sdlCustomEvent = SDL_RegisterEvents(1);
    sdlMainThreadId = SDL_ThreadID();

    while (platformThreadCount) {
#ifdef BOXEDWINE_RECORDER
        if (Player::instance || Recorder::instance) {
            SDL_WaitEventTimeout(&e, 10);
            BOXEDWINE_RECORDER_RUN_SLICE();
        } else if (!SDL_WaitEvent(&e)) {
            break;
        }
#else
        if (!SDL_WaitEvent(&e)) {
            break;
        }
#endif        
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