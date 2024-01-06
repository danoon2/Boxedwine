#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED
#include "devfb.h"
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
#include "../../ui/mainui.h"
#endif
#include "knativesystem.h"
#include "knativewindow.h"

bool isFbReady();
U32 getNextTimer();
void runTimers();

extern U32 platformThreadCount;
extern U32 exceptionCount;
extern U32 dynamicCodeExceptionCount;
static U32 lastTitleUpdate = 0;

static THREAD_LOCAL bool isMainThread;
bool isMainthread() {
    return isMainThread;
}
bool doMainLoop() {
    isMainThread = true;
    while (platformThreadCount) {
        U32 timeout = 5000;
        U32 t = KSystem::getMilliesSinceStart();

        if (KSystem::killTime) {
            if (KSystem::killTime <= t) {
                KNativeSystem::cleanup();
                exit(9);
                return true;
            }
            if (t - KSystem::killTime < timeout) {
                timeout = t - KSystem::killTime;
            }
        }
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
        if (uiIsRunning()) {
            timeout = 33;
        }
#endif
#ifdef BOXEDWINE_64BIT_MMU
        if (isFbReady()) {
            timeout = 17;
            flipFB();
        }
#endif
        U32 nextTimer = getNextTimer();
        if (nextTimer == 0) {
            runTimers();
        } else if (nextTimer < timeout) {
            timeout = nextTimer;
        }
#ifdef BOXEDWINE_RECORDER
        if (Player::instance || Recorder::instance) {
            KNativeWindow::getNativeWindow()->waitForEvent(10);
            BOXEDWINE_RECORDER_RUN_SLICE();
        } else  {
            KNativeWindow::getNativeWindow()->waitForEvent(timeout);
        }
#else
        KNativeWindow::getNativeWindow()->waitForEvent(timeout);
#endif    
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
        if (uiIsRunning()) {
            uiLoop();
        }
#endif                
        if (lastTitleUpdate+5000 < t) {            
            lastTitleUpdate = t;
            BString title;
            if (KSystem::title.length()) {
                title = KSystem::title;
            } else {
                title = B("BoxedWine " BOXEDWINE_VERSION_DISPLAY);
            }
            KNativeWindow::getNativeWindow()->setTitle(title);
        }
        if (!KNativeWindow::getNativeWindow()->processEvents()) {
            return true;
        }
    };
    return true;
}

void waitForProcessToFinish(const std::shared_ptr<KProcess>& process, KThread* thread) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
    while (!process->isTerminated()) {
        BOXEDWINE_CONDITION_WAIT(KSystem::processesCond);
    }
}

#endif
