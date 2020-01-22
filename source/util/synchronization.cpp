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
    this->lockOwner = 0;
}

BoxedWineCondition::BoxedWineCondition() {
    this->m = SDL_CreateMutex();
    this->c = SDL_CreateCond();
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
    if (this->parents.size()>0) {
        // signal will change this->parents so we can't iterate this->parents directly
        BoxedWineCondition** pp = NULL;
        VECTOR_TO_ARRAY_ON_STACK(this->parents, BoxedWineCondition*, pp);
        for (int i=0;i<this->parents.size();i++) {
            BoxedWineCondition* p  = pp[i];
            while(!p->tryLock()) {
                this->unlock();
                this->lock();
            }
            if(VECTOR_CONTAINS(this->parents, p)) {
                p->signal();            
            }
            p->unlock();
        }
    } else {
        SDL_CondSignal((SDL_cond*)this->c);
    }
}

void BoxedWineCondition::signalAll() {
    if (this->parents.size()>0) {
        BoxedWineCondition** pp = NULL;
        VECTOR_TO_ARRAY_ON_STACK(this->parents, BoxedWineCondition*, pp);
        for (int i=0;i<this->parents.size();i++) {
            BoxedWineCondition* p  = pp[i];
            while(!p->tryLock()) {
                this->unlock();
                this->lock();
            }
            if(VECTOR_CONTAINS(this->parents, p)) {
                p->signalAll();            
            }
            p->unlock();
        }
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
        VECTOR_REMOVE(child.cond->parents, this);
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
        VECTOR_REMOVE(child.cond->parents, this);
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
        if (!VECTOR_CONTAINS(child.cond->parents, this)) {
            kpanic("BoxedWineCondition::waitWithTimeout in bad state");
        }
    }

    KThread::currentThread()->waitingCond = this;
    SDL_CondWaitTimeout((SDL_cond*)this->c, (SDL_mutex*)this->m, ms);
    KThread::currentThread()->waitingCond = NULL;

    for (auto &child : this->children) {
        child.cond->lock();
        VECTOR_REMOVE(child.cond->parents, this);
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
    if (VECTOR_CONTAINS(cond.parents, this)) {
        kpanic("BoxedWineCondition::addChildCondition cond already has this parent");
    }
    cond.parents.push_back(this);
    this->children.push_back(BoxedWineConditionChild(&cond, doneWaitingCallback));
}

U32 BoxedWineCondition::waitCount() {
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
        for (U32 i=0;i<this->parents.size();i++) {
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
        for (U32 i=0;i<this->parents.size();i++) {
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
    if (VECTOR_CONTAINS(cond.parents, this)) {
        kpanic("BoxedWineCondition::addChildCondition cond already has this parent");
    }
    cond.parents.push_back(this);
    this->children.push_back(BoxedWineConditionChild(&cond, doneWaitingCallback));
}

#endif