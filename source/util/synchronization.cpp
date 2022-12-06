#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED

BoxedWineCriticalSection::BoxedWineCriticalSection(BoxedWineMutex* mutex) {
    this->mutex = mutex;
    this->mutex->lock();    
}
BoxedWineCriticalSection::~BoxedWineCriticalSection() {
    this->mutex->unlock();
}

BoxedWineCriticalSectionCond::BoxedWineCriticalSectionCond(BoxedWineCondition* cond) {
    this->cond = cond;
    this->cond->lock();
}
BoxedWineCriticalSectionCond::~BoxedWineCriticalSectionCond() {
    this->cond->unlock();
}

void BoxedWineMutex::lock() {
    this->m.lock();
}

bool BoxedWineMutex::tryLock() {
    return this->m.tryLock();
}

void BoxedWineMutex::unlock() {
    this->m.unlock();
}

BoxedWineCondition::BoxedWineCondition(std::string name) : name(name) {
    this->lockOwner = 0;
}

BoxedWineCondition::BoxedWineCondition() {
    this->lockOwner = 0;
}

void BoxedWineCondition::lock() {
    this->m.lock();
    if (KThread::currentThread()) {
        this->lockOwner = KThread::currentThread()->id;
    } else {
        this->lockOwner = (U32)-1;
    }
}
 
bool BoxedWineCondition::tryLock() {
    if (this->m.tryLock()) {
        if (KThread::currentThread()) {
            this->lockOwner = KThread::currentThread()->id;
        } else {
            this->lockOwner = (U32)-1;
        }
        return true;
    }
    return false;
}

void BoxedWineCondition::signal() { 
    parentsMutex.lock();
    if (this->parents.size()>0) {
        // signal will change this->parents so we can't iterate this->parents directly
        BoxedWineCondition** pp = NULL;
        int count = (int)this->parents.size();
        VECTOR_TO_ARRAY_ON_STACK(this->parents, BoxedWineCondition*, pp);
        for (int i=0;i<count;i++) {
            BoxedWineCondition* p  = pp[i];
            while(!p->tryLock()) {
                this->unlock();
                parentsMutex.unlock();
                this->lock();
                parentsMutex.lock();
            }
            if(VECTOR_CONTAINS(this->parents, p)) {
                p->signal();            
            }
            p->unlock();
        }
    } else {
        parentsMutex.unlock();
        this->c.signal();
    }
}

void BoxedWineCondition::signalAll() {
    parentsMutex.lock();
    if (this->parents.size()>0) {
        BoxedWineCondition** pp = NULL;
        int count = (int)this->parents.size();
        VECTOR_TO_ARRAY_ON_STACK(this->parents, BoxedWineCondition*, pp);
        for (int i=0;i<count;i++) {
            BoxedWineCondition* p  = pp[i];
            while(!p->tryLock()) {
                this->unlock();
                parentsMutex.unlock();
                this->lock();
                parentsMutex.lock();
            }
            if(VECTOR_CONTAINS(this->parents, p)) {
                p->signalAll();            
            }
            p->unlock();
        }
    }
    parentsMutex.unlock();
    this->c.signalAll();
}

void BoxedWineCondition::signalAllLock() {
    this->lock();
    this->signalAll();
    this->unlock();
}

void BoxedWineCondition::unlockAndRemoveChildren() {
    for (auto &child : this->children) {
        child.cond->parentsMutex.lock();
        VECTOR_REMOVE(child.cond->parents, this);
        child.cond->parentsMutex.unlock();
        child.cond->unlock();
    }
    for (auto &child : this->children) {
        if (child.doneWaitingCallback) {
            child.doneWaitingCallback();
        }
    }
    this->children.clear();
}

void BoxedWineCondition::wait() {    
    for (auto &child : this->children) {
        child.cond->unlock();
    }
    KThread* thread = KThread::currentThread();
    if (thread) {
        thread->waitingCond = this;
    }
    this->c.wait(this->m);
    if (thread) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->waitingCondSync);
        thread->waitingCond = NULL;
    }
    for (auto &child : this->children) {
        child.cond->lock();
        child.cond->parentsMutex.lock();
        VECTOR_REMOVE(child.cond->parents, this);
        child.cond->parentsMutex.unlock();
        child.cond->unlock();
    }
    this->children.clear();
}

void BoxedWineCondition::waitWithTimeout(U32 ms) {
    for (auto &child : this->children) {
        child.cond->unlock();
        child.cond->parentsMutex.lock();
        if (!VECTOR_CONTAINS(child.cond->parents, this)) {
            kpanic("BoxedWineCondition::waitWithTimeout in bad state");
        }
        child.cond->parentsMutex.unlock();
    }
    KThread* thread = KThread::currentThread();
    if (!KSystem::shutingDown && thread) {
        thread->waitingCond = this;
    }
    this->c.waitWithTimeout(this->m, KSystem::emulatedMilliesToHost(ms));
    if (!KSystem::shutingDown && thread) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->waitingCondSync);
        thread->waitingCond = NULL;
    }

    for (auto &child : this->children) {
        child.cond->lock();
        child.cond->parentsMutex.lock();
        VECTOR_REMOVE(child.cond->parents, this);
        child.cond->parentsMutex.unlock();
        child.cond->unlock();
    }
    this->children.clear();
}

void BoxedWineCondition::unlock() {
    this->lockOwner = 0;
    this->m.unlock();
}

void BoxedWineCondition::addChildCondition(BoxedWineCondition& cond, const std::function<void(void)>& doneWaitingCallback) {
    // this (parent) should be be locked while we call this
    cond.lock();
    cond.parentsMutex.lock();
    if (VECTOR_CONTAINS(cond.parents, this)) {
        kpanic("BoxedWineCondition::addChildCondition cond already has this parent");
    }
    cond.parents.push_back(this);
    cond.parentsMutex.unlock();
    this->children.push_back(BoxedWineConditionChild(&cond, doneWaitingCallback));
}

U32 BoxedWineCondition::waitCount() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parentsMutex)
    return (U32)this->parents.size();
}

#else 

bool BoxedWineConditionTimer::run() {
    this->cond->signalThread(true);
    return false; // signal will remove timer
}

BoxedWineCondition::BoxedWineCondition(std::string name) : name(name) {
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
        thread->waitingCond = NULL;
        if (thread->condTimer.active) {
            removeTimer(&thread->condTimer);
            thread->condTimer.cond = NULL;
        }
        scheduleThread(thread);
        if (!all) {
            break;
        }
    }
    for (auto &child : this->children) {
        VECTOR_REMOVE(child.cond->parents, this);
    }
    this->children.clear();
}

void BoxedWineCondition::signal() {
    if (this->parents.size()) {
        // signal will change this->parents so we can't iterate this->parents directly
        BoxedWineCondition** pp = NULL;
        VECTOR_TO_ARRAY_ON_STACK(this->parents, BoxedWineCondition*, pp);
        for (U32 i=0;i<(U32)this->parents.size();i++) {
            pp[i]->signal();
        }
    }
    this->signalThread(false);
}

void BoxedWineCondition::signalAll() {
    if (this->parents.size()) {
        // signalAll will change this->parents so we can't iterate this->parents directly
        BoxedWineCondition** pp = NULL;
        VECTOR_TO_ARRAY_ON_STACK(this->parents, BoxedWineCondition*, pp);
        for (U32 i=0;i<(U32)this->parents.size();i++) {
            pp[i]->signalAll();
        }
    }
    this->signalThread(true);
}

void BoxedWineCondition::unlockAndRemoveChildren() {
    for (auto &child : this->children) {
        VECTOR_REMOVE(child.cond->parents, this);
    }
    for (auto &child : this->children) {
        if (child.doneWaitingCallback) {
            child.doneWaitingCallback();
        }
    }
    this->children.clear();
}

U32 BoxedWineCondition::wait() {
    this->waitingThreads.addToBack(&KThread::currentThread()->waitThreadNode);
    KThread::currentThread()->waitingCond = this;
    unscheduleThread(KThread::currentThread());
    return -K_WAIT;
}

U32 BoxedWineCondition::waitWithTimeout(U32 ms) {
    KThread* thread = KThread::currentThread();
    thread->condTimer.millies = ms + KSystem::getMilliesSinceStart();
    thread->condTimer.cond = this;
    addTimer(&thread->condTimer);
    this->waitingThreads.addToBack(&KThread::currentThread()->waitThreadNode);
    KThread::currentThread()->waitingCond = this;
    unscheduleThread(KThread::currentThread());
    return -K_WAIT;    
}
 
U32 BoxedWineCondition::waitCount() {
    return this->waitingThreads.size();
}

void BoxedWineCondition::addChildCondition(BoxedWineCondition& cond, const std::function<void(void)>& doneWaitingCallback) {
    if (VECTOR_CONTAINS(cond.parents, this)) {
        kpanic("BoxedWineCondition::addChildCondition cond already has this parent");
    }
    cond.parents.push_back(this);
    this->children.push_back(BoxedWineConditionChild(&cond, doneWaitingCallback));
}

#endif
