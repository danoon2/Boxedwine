#include "boxedwine.h"
#include "knativeaudio.h"

#include <SDL.h>
#include "knativeaudiosdl.h"
#include "knativesystem.h"

#define E_FAIL 0x80004005

std::vector< std::shared_ptr<KNativeAudio> > KNativeAudio::availableAudio;
bool KNativeAudioSDL::load() {
	// if (CoreAudio_MIDIInit() != DRV_SUCCESS)
	//	return false;

	return true;
}
U32 KNativeAudioSDL::midiOutOpen(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutClose(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutData(U32 wDevID, U8* buffer, U32 len) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutLongData(U32 wDevID, U8* buffer, U32 len) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutGetNumDevs() {
	return 0;
}

U32 KNativeAudioSDL::midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutSetVolume(U32 wDevID, U32 dwVolume) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutReset(U32 wDevID) {
	return E_FAIL;
}

BString KNativeAudioSDL::midiOutGetName(U32 wDevID) {
    return BString::empty;
}

bool KNativeAudioSDL::midiOutIsOpen(U32 wDevID) {
    return false;
}


std::shared_ptr<KNativeAudio> KNativeAudio::createNativeAudio() {
    // :TODO: maybe allow the user to choose in the future
	return KNativeAudio::availableAudio.front();
}

void KNativeAudio::shutdown() {
    KNativeAudio::availableAudio.clear();
}

#ifdef BOXEDWINE_CORE_AUDIO
void initCoreAudio();
#endif
#ifdef BOXEDWINE_MSVC
void initWindowsAudio();
#endif

void KNativeAudio::init() {
#ifdef BOXEDWINE_CORE_AUDIO
    initCoreAudio();
#elif defined(BOXEDWINE_MSVC)
	initWindowsAudio();
#else
    KNativeAudio::availableAudio.push_back(std::make_shared<KNativeAudioSDL>());
#endif
}
