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
#include "../sdl/knativeaudiosdl.h"
#include <Windows.h>
#undef midiOutGetDevCaps

class KNativeAudioWindows : public KNativeAudioSDL, public std::enable_shared_from_this<KNativeAudioWindows> {
public:
	KNativeAudioWindows() = default;
	virtual ~KNativeAudioWindows() {}

	// from KNativeAudio
	U32 midiOutOpen(U32 wDevID) override;
	U32 midiOutClose(U32 wDevID) override;
	U32 midiOutData(U32 wDevID, U8* buffer, U32 len) override;
	U32 midiOutLongData(U32 wDevID, U8* buffer, U32 len) override;
	U32 midiOutGetNumDevs() override;
	U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) override;
	U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) override;
	U32 midiOutReset(U32 wDevID) override;
	BString midiOutGetName(U32 wDevID) override;
	bool midiOutIsOpen(U32 wDevID) override;

	BHashTable<U32, HMIDIOUT> openedDevices;
};

U32 KNativeAudioWindows::midiOutOpen(U32 wDevID) {	
	HMIDIOUT out = { 0 };
	U32 result = ::midiOutOpen(&out, wDevID, NULL, 0, 0);
	if (result == NOERROR) {
		openedDevices.set(wDevID, out);
	}
	return result;
}

U32 KNativeAudioWindows::midiOutClose(U32 wDevID) {
	HMIDIOUT out = { 0 };
	if (openedDevices.get(wDevID, out)) {
		U32 result = ::midiOutClose(out);
		openedDevices.remove(wDevID);
		return result;
	}
	return MMSYSERR_BADDEVICEID;
}

U32 KNativeAudioWindows::midiOutData(U32 wDevID, U8* buffer, U32 len) {
	HMIDIOUT out = { 0 };
	if (!openedDevices.get(wDevID, out)) {
		return MMSYSERR_BADDEVICEID;
	}

	DWORD dwParam = buffer[0];
	if (len > 1) {
		dwParam |= (buffer[1] << 8);
	}
	if (len > 2) {
		dwParam |= (buffer[2] << 16);
	}
	if (len > 3) {
		dwParam |= (buffer[3] << 24);
	}
	return ::midiOutShortMsg(out, dwParam);
}

U32 KNativeAudioWindows::midiOutLongData(U32 wDevID, U8* buffer, U32 len) {
	HMIDIOUT out = { 0 };
	if (!openedDevices.get(wDevID, out)) {
		return MMSYSERR_BADDEVICEID;
	}

	MIDIHDR nativeHeader = { 0 };
	nativeHeader.lpData = (LPSTR)buffer;
	nativeHeader.dwBufferLength = len;

	return ::midiOutLongMsg(out, &nativeHeader, sizeof(nativeHeader));
}

U32 KNativeAudioWindows::midiOutGetNumDevs() {
	return ::midiOutGetNumDevs();
}

U32 KNativeAudioWindows::midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) {
	HMIDIOUT out = { 0 };
	if (!openedDevices.get(wDevID, out)) {
		return MMSYSERR_BADDEVICEID;
	}
	DWORD volume;
	U32 result = ::midiOutGetVolume(out, &volume);
	thread->memory->writed(lpdwVolume, volume);
	return result;
}

U32 KNativeAudioWindows::midiOutSetVolume(U32 wDevID, U32 dwVolume) {
	HMIDIOUT out = { 0 };
	if (!openedDevices.get(wDevID, out)) {
		return MMSYSERR_BADDEVICEID;
	}
	return ::midiOutSetVolume(out, dwVolume);
}

U32 KNativeAudioWindows::midiOutReset(U32 wDevID) {
	HMIDIOUT out = { 0 };
	if (!openedDevices.get(wDevID, out)) {
		return MMSYSERR_BADDEVICEID;
	}
	return ::midiOutReset(out);
}

BString KNativeAudioWindows::midiOutGetName(U32 wDevID) {
	HMIDIOUT out = { 0 };
	if (!openedDevices.get(wDevID, out)) {
		return BString::empty;
	}
	MIDIOUTCAPSA caps;
	::midiOutGetDevCapsA(wDevID, &caps, sizeof(caps));
	return BString::copy(caps.szPname);
}

bool KNativeAudioWindows::midiOutIsOpen(U32 wDevID) {
	return openedDevices.contains(wDevID);
}

void initWindowsAudio() {
	KNativeAudio::availableAudio.push_back(std::make_shared<KNativeAudioWindows>());
}