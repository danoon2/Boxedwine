#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include "boxedwine.h"
#include "knativesystem.h"

#ifdef BOXEDWINE_MULTI_THREADED
#include "knativethread.h"

U32 getNextTimer();
void runTimers();

extern std::atomic<int> platformThreadCount;

static U32 lastTitleUpdate = 0;
static thread_local bool isMainThread;

extern int allocatedRamPages;

bool isMainthread() {
    return isMainThread;
}

static BString getSize(int pages)
{
    pages *= 4;
    if (pages < 2048) {
        return BString::valueOf(pages) + B("KB");
    }
    if (pages < 2048 * 1024) {
        return BString::valueOf(pages / 1024) + B("MB");
    }
    return BString::valueOf(pages / 1024 / 1024) + B("GB");
}
extern int allocatedRamPages;
void mainloop() {
    isMainThread = true;
    while (platformThreadCount) {
        U32 timeout = 250;
        U32 t = KSystem::getMilliesSinceStart();
        U32 nextTimer = getNextTimer();
        if (nextTimer == 0) {
            runTimers();
        } else if (nextTimer < timeout) {
            timeout = nextTimer;
        }

        bool timedout = KNativeSystem::getCurrentInput()->waitForEvent(timeout) == false;
           
        if (lastTitleUpdate + 5000 < t) {
            lastTitleUpdate = t;
            BString title;
            if (KSystem::title.length()) {
                title = KSystem::title;
            } else {
                title = B("BoxedWine " BOXEDWINE_VERSION_DISPLAY " ");
                title.append(getSize(allocatedRamPages));
            }

            title.append(" ");
            title.append(getSize(allocatedRamPages));

            //EM_ASM_INT(
            //    document.title = title;
            //    );
        }
        if (!KNativeSystem::getCurrentInput()->processEvents()) {
            KNativeSystem::cleanup();
            return;
        }
        if (KNativeSystem::getScreen()->presentedSinceLastCheck()) {
            break;
        }
        if (timedout) {
            break;
        }
    };
}

void waitForProcessToFinish(const std::shared_ptr<KProcess>& process, KThread* thread) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
    while (!process->isTerminated()) {
        BOXEDWINE_CONDITION_WAIT(KSystem::processesCond);
    }
}

#else

static U32 lastTitleUpdate = 0;

bool isMainthread() {
    return true;
}

void mainloop() {
    U32 startTime = KNativeSystem::getTicks();
    U32 t;
    U32 count=0;
    while (1) {
        bool ran = runSlice();

        KNativeSystem::tick();
        if (!KNativeSystem::getCurrentInput()->processEvents()) {
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
        if ((KNativeSystem::getTicks()-startTime)>250 || KNativeSystem::getScreen()->presentedSinceLastCheck()) {
            break;
        }
    };
}

#endif

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
#endif
