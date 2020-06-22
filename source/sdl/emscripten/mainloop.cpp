#include <emscripten/emscripten.h>
#include "boxedwine.h"
#include "knativewindow.h"
#include "knativesystem.h"

static U32 lastTitleUpdate = 0;

void mainloop() {
    U32 startTime = KNativeSystem::getTicks();
    U32 t;
    U32 count=0;
    while (1) {
        bool ran = runSlice();
        
        if (!KNativeWindow::getNativeWindow()->processEvents()) {
            KNativeSystem::cleanup();
            return;
        }
        t = KSystem::getMilliesSinceStart();                
        if (lastTitleUpdate+1000 < t) {
            lastTitleUpdate = t;
            EM_ASM_INT({
                document.title="BoxedWine " + $0 + " MIPS";
            }, getMIPS());
        }
        if (!ran) {
            break;
        }
        if ((KNativeSystem::getTicks()-startTime)>250 || KNativeWindow::windowUpdated) {
            KNativeWindow::windowUpdated = false;
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
