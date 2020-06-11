#include "boxedwine.h"
#include "knativethread.h"
#include <SDL.h>

static int sdl_start_thread(void* ptr) {
	KNativeThread* thread = (KNativeThread*)ptr;
	return thread->pfn(thread->data);
}

KNativeThread* KNativeThread::createAndStartThread(KNativeThreadFunction pfn, const std::string& name, void* data) {
	KNativeThread* result = new KNativeThread(pfn, name, data);

	result->nativeThread = SDL_CreateThread(sdl_start_thread, name.c_str(), result);
	return result;
}

int KNativeThread::wait() {
	int threadReturnValue = 0;
	SDL_WaitThread((SDL_Thread*)this->nativeThread, &threadReturnValue);
	return threadReturnValue;
}

void KNativeThread::sleep(U32 ms) {
	SDL_Delay(ms);
}