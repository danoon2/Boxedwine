#include "boxedwine.h"
#ifndef BOXEDWINE_MULTI_THREADED
#include "recorder.h"
#include "knativesocket.h"
#include "knativewindow.h"
#include "knativethread.h"

#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
#include "../../ui/mainui.h"
#endif

static U32 lastTitleUpdate = 0;
bool isMainthread() {
    return true;
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
bool doMainLoop() {
    bool shouldQuit = false;

    while (KSystem::getProcessCount()>0 && !shouldQuit) {
        bool ran = runSlice();
        U32 t;

        BOXEDWINE_RECORDER_RUN_SLICE();
        if (!KNativeWindow::getNativeWindow()->processEvents()) {
            shouldQuit = true;
            break;
        }
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST)
        if (uiIsRunning()) {
            uiLoop();
        }
#endif
        t = KSystem::getMilliesSinceStart();

        if (KSystem::killTime && KSystem::killTime <= t) {
            return true;
        }
        if (lastTitleUpdate+5000 < t) {            
            lastTitleUpdate = t;
            if (KSystem::title.length()) {
                KNativeWindow::getNativeWindow()->setTitle(KSystem::title);
            } else {
                BString title = B("BoxedWine " BOXEDWINE_VERSION_DISPLAY );
                title.append(" MIPS");
                title.append(getMIPS());
                title.append(" : ");
                title.append(getSize(allocatedRamPages));
                KNativeWindow::getNativeWindow()->setTitle(title);
            }            
        }
        if (ran) {
            checkWaitingNativeSockets(0);
        } else {
            if (KSystem::getRunningProcessCount()==0) {
                break;
            }
            if (!checkWaitingNativeSockets(20)) {
                KNativeThread::sleep(20);
            }
        }
    }
    return true;
}
#endif
