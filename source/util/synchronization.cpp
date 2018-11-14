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

BoxedWineCondition::BoxedWineCondition() {
    this->m = SDL_CreateMutex();
    this->c = SDL_CreateCond();
}

BoxedWineCondition::~BoxedWineCondition() {
    SDL_DestroyMutex((SDL_mutex*)this->m);
    SDL_DestroyCond((SDL_cond*)this->c);
}

void BoxedWineCondition::lock() {
    SDL_LockMutex((SDL_mutex*)this->m);
}
 
void BoxedWineCondition::signal() {
    SDL_CondSignal((SDL_cond*)this->c);
}

void BoxedWineCondition::signalAll() {
    SDL_CondBroadcast((SDL_cond*)this->c);
}

void BoxedWineCondition::wait() {
    SDL_CondWait((SDL_cond*)this->c, (SDL_mutex*)this->m);
}

void BoxedWineCondition::unlock() {
    SDL_UnlockMutex((SDL_mutex*)this->m);
}

#endif