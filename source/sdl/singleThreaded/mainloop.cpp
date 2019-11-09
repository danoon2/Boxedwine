#include "boxedwine.h"
#ifndef BOXEDWINE_MULTI_THREADED
#include "../sdlwindow.h"
#include "recorder.h"
#include "devfb.h"
#include "knativesocket.h"
#include <SDL.h>

static U32 lastTitleUpdate = 0;

bool doMainLoop() {
    bool shouldQuit = false;

    while (KSystem::getProcessCount()>0 && !shouldQuit) {
        SDL_Event e;
        bool ran = runSlice();
        U32 t;

        BOXEDWINE_RECORDER_RUN_SLICE();
        while (SDL_PollEvent(&e)) {
            if (!handlSdlEvent(&e)) {
                shouldQuit = true;
                break;
            }
        };
        t = getMilliesSinceStart();
        if (lastTitleUpdate+5000 < t) {
            char tmp[256];
            lastTitleUpdate = t;
            sprintf(tmp, "BoxedWine 20R1a1 %u MIPS", getMIPS());
            fbSetCaption(tmp, "BoxedWine");
            checkWaitingNativeSockets(0); // just so it doesn't starve if the system is busy
        }
        if (!ran) {
            if (KSystem::getRunningProcessCount()==0) {
                break;
            }
            if (!checkWaitingNativeSockets(20))
                SDL_Delay(20);
        }
    }
    return true;
}
#endif