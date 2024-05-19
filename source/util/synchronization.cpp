#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED

BoxedWineCriticalSectionCond::BoxedWineCriticalSectionCond(const std::shared_ptr<BoxedWineCondition>& cond) {
    this->cond = cond;
    this->cond->lock();
}
BoxedWineCriticalSectionCond::~BoxedWineCriticalSectionCond() {
    this->cond->unlock();
}

BoxedWineCondition::BoxedWineCondition(BString name) : name(name) {
}

BoxedWineCondition::~BoxedWineCondition() {
    m.lock(); // race condition when all threads are shuting down, just make sure no one has the lock when we destroy it
    m.unlock();
}

void BoxedWineCondition::lock() {
    this->m.lock();
    if (KThread::currentThread()) {
        this->lockOwner = KThread::currentThread()->id;
    } else {
        this->lockOwner = 0xFFFFFFFF;
    }
}
 
bool BoxedWineCondition::tryLock() {
    if (this->m.try_lock()) {
        if (KThread::currentThread()) {
            this->lockOwner = KThread::currentThread()->id;
        } else {
            this->lockOwner = 0xFFFFFFFF;
        }
        return true;
    }
    return false;
}

void BoxedWineCondition::signal() {
    if (parentCount) {
        for (int i = 0; i < MAX_PARENTS; i++) {
            std::shared_ptr<BoxedWineCondition> parent;
            
            {
                const std::lock_guard<std::mutex> lock(parentsMutex);
                parent = parents[i].lock();
            }
            if (parent) {
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(parent);
                parent->signal();
                break;
            }
        }
    }
    this->c.notify_one();
}

void BoxedWineCondition::signalAll() {
    if (parentCount) {
        for (int i = 0; i < MAX_PARENTS; i++) {
            std::shared_ptr<BoxedWineCondition> parent;
            {
                const std::lock_guard<std::mutex> lock(parentsMutex);
                parent = parents[i].lock();
            }
            if (parent) {
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(parent);
                parent->signalAll();
            }
        }
    }
    this->c.notify_all();
}

void BoxedWineCondition::wait(std::unique_lock<std::mutex>& lock) {
    KThread* thread = KThread::currentThread();
    if (thread) {
        thread->waitingCond = shared_from_this();
    }
    this->c.wait(lock);
    if (thread) {
        thread->waitingCond = nullptr;
    }
    if (parentCount) {
        for (int i = 0; i < MAX_PARENTS; i++) {
            std::shared_ptr<BoxedWineCondition> parent;
            {
                const std::lock_guard<std::mutex> lock(parentsMutex);
                parent = parents[i].lock();
            }
            if (parent) {
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(parent);
                parent->signalAll();
            }
        }
    }
}

void BoxedWineCondition::waitWithTimeout(std::unique_lock<std::mutex>& lock, U32 ms) {    
    KThread* thread = KThread::currentThread();
    if (thread) {
        thread->waitingCond = shared_from_this();
    }
    this->c.wait_for(lock, std::chrono::milliseconds(KSystem::emulatedMilliesToHost(ms)));
    if (thread) {
        thread->waitingCond = nullptr;
    }    
    if (parentCount) {
        for (int i = 0; i < MAX_PARENTS; i++) {
            std::shared_ptr<BoxedWineCondition> parent;
            {
                const std::lock_guard<std::mutex> lock(parentsMutex);
                parent = parents[i].lock();
            }
            if (parent) {
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(parent);
                parent->signalAll();
            }
        }
    }
}

void BoxedWineCondition::unlock() {
    this->lockOwner = 0;
    this->m.unlock();
}

void BoxedWineCondition::addParentCondition(const std::shared_ptr<BoxedWineCondition>& parent) {
    const std::lock_guard<std::mutex> lock(parentsMutex);
    if (parentCount) {
        for (int i = 0; i < MAX_PARENTS; i++) {
            std::shared_ptr<BoxedWineCondition> p = parents[i].lock();
            if (p == parent) {
                return;
            }
        }
    }
    for (int i = 0; i < MAX_PARENTS;i++) {
        if (parents[i].expired()) {
            parents[i] = parent;
            parentCount++;
            return;
        }
    }
    kpanic("BoxedWineCondition::addParentCondition ran out of parent slots");
}

void BoxedWineCondition::removeParentCondition(const std::shared_ptr<BoxedWineCondition>& parent) {
    const std::lock_guard<std::mutex> lock(parentsMutex);
    for (int i = 0; i < MAX_PARENTS; i++) {
        std::shared_ptr<BoxedWineCondition> p = parents[i].lock();
        if (p == parent) {
            parents[i].reset();
            parentCount--;
        }
    }
}

U32 BoxedWineCondition::parentsCount() {
    const std::lock_guard<std::mutex> lock(parentsMutex);
    return parentCount;
}

#else 

bool BoxedWineConditionTimer::run() {
    this->cond->signalThread(true);
    return false; // signal will remove timer
}

BoxedWineCondition::BoxedWineCondition(BString name) : name(name) {
}

BoxedWineCondition::~BoxedWineCondition() {
}

void BoxedWineCondition::signalThread(bool all) {
    while (this->waitingThreads.size()) {
        KThread* thread = this->waitingThreads.front()->data;
        this->waitingThreads.front()->remove();
#ifdef _DEBUG
        if (!thread->waitingCond) {
            kpanic("shouldn't signal a thread that is not waiting");
        }
#endif
        thread->waitingCond = nullptr;
        if (thread->condTimer.active) {
            removeTimer(&thread->condTimer);
            thread->condTimer.cond = nullptr;
        }
        scheduleThread(thread);
        if (!all) {
            break;
        }
    }
}

void BoxedWineCondition::signal() {
    this->signalThread(false);
    for (auto& parent : parents) {
        parent->signal();
    }
}

void BoxedWineCondition::signalAll() {
    this->signalThread(true);
    for (auto& parent : parents) {
        parent->signalAll();
    }
}

U32 BoxedWineCondition::wait() {
    this->waitingThreads.addToBack(&KThread::currentThread()->waitThreadNode);
    KThread::currentThread()->waitingCond = shared_from_this();
    unscheduleThread(KThread::currentThread());
    return -K_WAIT;
}

U32 BoxedWineCondition::waitWithTimeout(U32 ms) {
    KThread* thread = KThread::currentThread();
    thread->condTimer.millies = ms + KSystem::getMilliesSinceStart();
    thread->condTimer.cond = this;
    addTimer(&thread->condTimer);
    this->waitingThreads.addToBack(&KThread::currentThread()->waitThreadNode);
    KThread::currentThread()->waitingCond = shared_from_this();
    unscheduleThread(KThread::currentThread());
    return -K_WAIT;    
}
 
U32 BoxedWineCondition::waitCount() {
    return this->waitingThreads.size() + (parents.size() > 0 ? 1 : 0);
}

void BoxedWineCondition::addParentCondition(const std::shared_ptr<BoxedWineCondition>& parent) {
    parents.insert(parent);
}

void BoxedWineCondition::removeParentCondition(const std::shared_ptr<BoxedWineCondition>& parent) {
    parents.erase(parent);
}

U32 BoxedWineCondition::parentsCount() {
    return parents.size();
}
#endif
