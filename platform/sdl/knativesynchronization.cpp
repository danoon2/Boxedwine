#include "boxedwine.h"
#include "knativesynchronization.h"

#ifdef BOXEDWINE_MULTI_THREADED
#include <SDL.h>

KNativeCondition::KNativeCondition() {
	this->c = SDL_CreateCond();
}

KNativeCondition::~KNativeCondition() {
	SDL_DestroyCond((SDL_cond*)this->c);
}

void KNativeCondition::signal() {
	SDL_CondSignal((SDL_cond*)this->c);
}

void KNativeCondition::signalAll() {
	SDL_CondBroadcast((SDL_cond*)this->c);
}

void KNativeCondition::wait(KNativeMutex& m) {
	SDL_CondWait((SDL_cond*)this->c, (SDL_mutex*)m.m);
}

void KNativeCondition::waitWithTimeout(KNativeMutex& m, U32 ms) {
	SDL_CondWaitTimeout((SDL_cond*)this->c, (SDL_mutex*)m.m, ms);
}

KNativeMutex::KNativeMutex() {
	this->m = SDL_CreateMutex();
}

KNativeMutex::~KNativeMutex() {
	SDL_DestroyMutex((SDL_mutex*)this->m);
}

void KNativeMutex::lock() {
	SDL_LockMutex((SDL_mutex*)this->m);
}

bool KNativeMutex::tryLock() {
	return (SDL_TryLockMutex((SDL_mutex*)this->m) == 0);
}

void KNativeMutex::unlock() {
	SDL_UnlockMutex((SDL_mutex*)this->m);
}
#endif