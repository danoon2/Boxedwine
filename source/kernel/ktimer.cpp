#include "boxedwine.h"

bool KRunTimer::run() {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(timer->waitCond);
    timer->numberOfExpirations++;    
    BOXEDWINE_CONDITION_SIGNAL_ALL(timer->waitCond);
    if (!timer->microInterval) {
        return true; // remove timer
    } else {
        U64 now = KSystem::getSystemTimeAsMicroSeconds();
        timer->microNextTimer = now + timer->microInterval;
        this->millies = (U32)(timer->microInterval / 1000) + KSystem::getMilliesSinceStart();
        return false; // keep timer going
    }
}

KTimerCallback::~KTimerCallback() {
    if (this->active) {
        removeTimer(this);
    }
}

KTimer::KTimer() : KObject(KTYPE_TIMER), waitCond(std::make_shared<BoxedWineCondition>(B("KTimer::waitCond"))), timer(this) {
}

KTimer::~KTimer() {
}

// From KObject
U32 KTimer::ioctl(KThread* thread, U32 request) {
    return -K_ENOTSUP;
}

S64 KTimer::seek(S64 pos) {
    return 0;
}

S64 KTimer::length() {
    return 0;
}

S64 KTimer::getPos() {
    return 0;
}

void KTimer::setBlocking(bool blocking) {
    this->blocking = blocking;
}

bool KTimer::isBlocking() {
    return blocking;
}

void KTimer::setAsync(bool isAsync) {
}

bool KTimer::isAsync() {
    return false;
}

KFileLock* KTimer::getLock(KFileLock* lock) {
    return nullptr;
}

U32 KTimer::setLock(KFileLock* lock, bool wait) {
    return -K_ENOTSUP;
}

void KTimer::unlockAll(U32 pid) {
}

bool KTimer::supportsLocks() {
    return false;
}

bool KTimer::isOpen() {
    return true;
}

bool KTimer::isReadReady() {
    return numberOfExpirations > 0;
}

bool KTimer::isWriteReady() {
    return false;
}

void KTimer::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_ADD_PARENT(this->waitCond, parentCondition);
    } else {
        BOXEDWINE_CONDITION_REMOVE_PARENT(this->waitCond, parentCondition);
    }
}

U32 KTimer::write(KThread* thread, U32 buffer, U32 len) {
    return -K_ENOTSUP;
}

U32 KTimer::writeNative(U8* buffer, U32 len) {
    return -K_ENOTSUP;
}

U32 KTimer::read(KThread* thread, U32 buffer, U32 len) {
    U64 value = 0;
    U32 result = readNative((U8*)&value, 8);
    if ((S32)result > 0) {
        thread->memory->writeq(buffer, value);
    }
    return result;
}

U32 KTimer::readNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(waitCond);
    if (numberOfExpirations > 0) {
        *(U64*)buffer = numberOfExpirations;
        return 8;
    }
    if (!blocking) {
        return -K_EAGAIN;
    }
    while (true) {
        BOXEDWINE_CONDITION_WAIT(waitCond);
#ifdef BOXEDWINE_MULTI_THREADED
        KThread* thread = KThread::currentThread();
        if (thread->terminating) {
            return -K_EINTR;
        }
        if (thread->startSignal) {
            thread->startSignal = false;
            return -K_CONTINUE;
        }
#endif
        if (numberOfExpirations > 0) {
            *(U64*)buffer = numberOfExpirations;
            return 8;
        }
    }
}

U32 KTimer::stat(KProcess* process, U32 address, bool is64) {
    return -K_ENOTSUP;
}

U32 KTimer::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return -K_ENOTSUP;
}

bool KTimer::canMap() {
    return false;
}

BString KTimer::selfFd() {
    return B("anon_inode:[timer]");
}

void KTimer::setTimes(U64 microInterval, U64 microNextTimer) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(waitCond);
    this->microInterval = microInterval;
    this->microNextTimer = microNextTimer;
    removeTimer(&timer);
    U64 now = KSystem::getSystemTimeAsMicroSeconds();
    S64 diff = microNextTimer - now;
    if (diff < 0) {
        numberOfExpirations++;
        if (microInterval) {
            microNextTimer = now + microInterval;
            timer.millies = (U32)(microInterval / 1000) + KSystem::getMilliesSinceStart();
            addTimer(&timer);
        }
    } else {
        timer.millies = (U32)(diff / 1000) + KSystem::getMilliesSinceStart();
        addTimer(&timer);
    }
}