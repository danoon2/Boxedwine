#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
#include "../../ui/mainui.h"
#endif
#include "knativesystem.h"
#include "devfb.h"
#include "../../x11/x11.h"
#include "platformOpenGL.h"

U32 getNextTimer();
void runTimers();

extern std::atomic<int> platformThreadCount;
extern U32 exceptionCount;
extern U32 dynamicCodeExceptionCount;
static U32 lastTitleUpdate = 0;

static thread_local bool isMainThread;

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
        XServer* server = XServer::getServer(true);
        if (server) {
            server->draw();
            timeout = 17;
        } 
        if (flipFB()) {
            timeout = 17;
        }
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
        else if (uiIsRunning()) {
            timeout = 33;
        }
#endif
        U32 nextTimer = getNextTimer();
        if (nextTimer == 0) {
            runTimers();
        } else if (nextTimer < timeout) {
            timeout = nextTimer;
        }
        KNativeSystem::tick();
#ifdef BOXEDWINE_RECORDER
        if (Player::instance || Recorder::instance) {
            KNativeSystem::getCurrentInput()->waitForEvent(10);
            BOXEDWINE_RECORDER_RUN_SLICE();
        } else  {
            KNativeSystem::getCurrentInput()->waitForEvent(timeout);
        }
#else
        KNativeSystem::getCurrentWindow()->waitForEvent(timeout);
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
#if defined(_DEBUG)
                title.append(" ");
                title.append(getSize(allocatedRamPages));
#endif
            } else {
                title = B("BoxedWine " BOXEDWINE_VERSION_DISPLAY " ");
                title.append(getSize(allocatedRamPages));
            }

            KNativeSystem::getScreen()->setTitle(title);
        }
        if (!KNativeSystem::getCurrentInput()->processEvents()) {
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
