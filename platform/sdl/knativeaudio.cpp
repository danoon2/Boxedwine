/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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
	return 0;
}

U32 KNativeAudioSDL::midiOutClose(U32 wDevID) {
	return 0;
}

U32 KNativeAudioSDL::midiOutData(U32 wDevID, U8* buffer, U32 len) {
	return 0;
}

U32 KNativeAudioSDL::midiOutLongData(U32 wDevID, U8* buffer, U32 len) {
	return 0;
}

U32 KNativeAudioSDL::midiOutGetNumDevs() {
	return 1;
}

U32 KNativeAudioSDL::midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) {
	return 0;
}

U32 KNativeAudioSDL::midiOutSetVolume(U32 wDevID, U32 dwVolume) {
	return 0;
}

U32 KNativeAudioSDL::midiOutReset(U32 wDevID) {
	return 0;
}

BString KNativeAudioSDL::midiOutGetName(U32 wDevID) {
    return B("Fake Midi");
}

bool KNativeAudioSDL::midiOutIsOpen(U32 wDevID) {
    return true;
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
