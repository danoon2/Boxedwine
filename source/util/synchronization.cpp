#include "boxedwine.h"
#ifdef BOXEDWINE_MULTI_THREADED
#include <SDL.h>

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

BoxedWineMutex::BoxedWineMutex() {
    this->m = SDL_CreateMutex();
}

BoxedWineMutex::~BoxedWineMutex() {
    SDL_DestroyMutex((SDL_mutex*)this->m);
}

void BoxedWineMutex::lock() {
    SDL_LockMutex((SDL_mutex*)this->m);    
}

void BoxedWineMutex::unlock() {
    SDL_UnlockMutex((SDL_mutex*)this->m);
}

BoxedWineCondition::BoxedWineCondition(std::string name) : name(name) {
    this->m = SDL_CreateMutex();
    this->c = SDL_CreateCond();
    this->parent = NULL;
    this->lockOwner = 0;
}

BoxedWineCondition::BoxedWineCondition() {
    this->m = SDL_CreateMutex();
    this->c = SDL_CreateCond();
    this->parent = NULL;
    this->lockOwner = 0;
}

BoxedWineCondition::~BoxedWineCondition() {
    SDL_DestroyMutex((SDL_mutex*)this->m);
    SDL_DestroyCond((SDL_cond*)this->c);
}

void BoxedWineCondition::lock() {
    SDL_LockMutex((SDL_mutex*)this->m);
    if (KThread::currentThread()) {
        this->lockOwner = KThread::currentThread()->id;
    } else {
        this->lockOwner = (U32)-1;
    }
}
 
bool BoxedWineCondition::tryLock() {
    if (SDL_TryLockMutex((SDL_mutex*)this->m)==0) {
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
    if (this->parent) {
        BoxedWineCondition* p = this->parent;
        while(!p->tryLock()) {
            this->unlock();
            this->lock();
        }
        if (p==this->parent) {
            this->parent->signal();            
        }
        p->unlock();
    } else {
        SDL_CondSignal((SDL_cond*)this->c);
    }
}

void BoxedWineCondition::signalAll() {
    if (this->parent) {
        BoxedWineCondition* p = this->parent;
        while(!p->tryLock()) {
            this->unlock();
            this->lock();
        }
        if (p==this->parent) {
            this->parent->signalAll();            
        }
        p->unlock();
    }
    SDL_CondBroadcast((SDL_cond*)this->c);
}

void BoxedWineCondition::signalAllLock() {
    this->lock();
    this->signalAll();
    this->unlock();
}

void BoxedWineCondition::unlockAndRemoveChildren() {
    for (auto &child : this->children) {
        child.cond->parent = NULL;
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

    KThread::currentThread()->waitingCond = this;
    SDL_CondWait((SDL_cond*)this->c, (SDL_mutex*)this->m);    
    KThread::currentThread()->waitingCond = NULL;

    for (auto &child : this->children) {
        child.cond->lock();
        child.cond->parent = NULL;
        child.cond->unlock();
    }
    this->children.clear();

    if (KThread::currentThread()->exiting) {
        this->unlock();
        unscheduleCurrentThread();
    }
}

void BoxedWineCondition::waitWithTimeout(U32 ms) {
    for (auto &child : this->children) {
        child.cond->unlock();
    }

    KThread::currentThread()->waitingCond = this;
    SDL_CondWaitTimeout((SDL_cond*)this->c, (SDL_mutex*)this->m, ms);
    KThread::currentThread()->waitingCond = NULL;

    for (auto &child : this->children) {
        child.cond->lock();
        child.cond->parent = NULL;
        child.cond->unlock();
    }
    this->children.clear();

    if (KThread::currentThread()->exiting) {
        this->unlock();
        unscheduleCurrentThread();
    }
}

void BoxedWineCondition::unlock() {
    this->lockOwner = 0;
    SDL_UnlockMutex((SDL_mutex*)this->m);
}

void BoxedWineCondition::addChildCondition(BoxedWineCondition& cond, const std::function<void(void)>& doneWaitingCallback) {
    // this (parent) should be be locked while we call this
    cond.lock();
    cond.parent = this;
    this->children.push_back(BoxedWineConditionChild(&cond, doneWaitingCallback));
}

U32 BoxedWineCondition::waitCount() {
    return (this->parent?1:0);
}

#else 

bool BoxedWineConditionTimer::run() {
    this->cond->signalThread(true);
    return false; // signal will remove timer
}

BoxedWineCondition::BoxedWineCondition(std::string name) : name(name), parent(NULL) {
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
        child.cond->parent = NULL;
    }
    this->children.clear();
}

void BoxedWineCondition::signal() {
    if (this->parent) {
        this->parent->signalAll();
    }
    this->signalThread(false);
}

void BoxedWineCondition::signalAll() {
    if (this->parent) {
        this->parent->signalAll();
    }
    this->signalThread(true);
}

void BoxedWineCondition::unlockAndRemoveChildren() {
    for (auto &child : this->children) {
        child.cond->parent = NULL;
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
    thread->condTimer.millies = ms+getMilliesSinceStart();
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
    cond.parent = this;
    this->children.push_back(BoxedWineConditionChild(&cond, doneWaitingCallback));
}

#endif