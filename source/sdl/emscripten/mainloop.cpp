#include <emscripten/emscripten.h>
#include "boxedwine.h"
#include "../sdlwindow.h"

extern U32 sdlUpdated;

static U32 lastTitleUpdate = 0;

void mainloop() {
    U32 startTime = SDL_GetTicks();
    U32 t;
    U32 count=0;
    while (1) {
        SDL_Event e;
        bool ran = runSlice();
        
        while (SDL_PollEvent(&e)) {
            if (!handlSdlEvent(&e)) {
                SDL_Quit();
            }            
        };
        t = getMilliesSinceStart();                
        if (lastTitleUpdate+1000 < t) {
            lastTitleUpdate = t;
            EM_ASM_INT({
                document.title="BoxedWine " + $0 + " MIPS";
            }, getMIPS());
        }
        if (!ran) {
            break;
        }
        if ((SDL_GetTicks()-startTime)>250 || sdlUpdated) {
           sdlUpdated=0;
           break;
        }
    };
}

bool doMainLoop() {
    EM_ASM(
#ifndef SDL2
            SDL.defaults.copyOnLock = false;
            SDL.defaults.discardOnLock = true;
#endif
            //SDL.defaults.opaqueFrontBuffer = false;
    );
    emscripten_set_main_loop(mainloop, 0, 1);
    return false;
}