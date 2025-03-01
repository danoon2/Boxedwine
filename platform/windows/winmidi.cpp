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
#ifdef BOXEDWINE_MIDI
#include <Windows.h>

HMIDIOUT m_out;
MIDIHDR m_hdr;
HANDLE m_event;
bool isOpen;

bool OpenMidi() {
	if (isOpen) 
        return false;
	m_event = CreateEvent (nullptr, true, true, nullptr);
	MMRESULT res = MMSYSERR_NOERROR;
    res = midiOutOpen(&m_out, MIDI_MAPPER, (DWORD_PTR)m_event, 0, CALLBACK_EVENT);
	if (res != MMSYSERR_NOERROR) 
        return false;
	isOpen=true;
	return true;
};

void PlayMsg(U8* msg) {
    OpenMidi();
	midiOutShortMsg(m_out, *(U32*)msg);
};
void PlaySysex(U8 * sysex,U32 len) {
    OpenMidi();
	if (WaitForSingleObject (m_event, 2000) == WAIT_TIMEOUT) {
		klog("Can't send midi message");
		return;
	}		
	midiOutUnprepareHeader (m_out, &m_hdr, sizeof (m_hdr));
	
	m_hdr.lpData = (char *) sysex;
	m_hdr.dwBufferLength = len ;
	m_hdr.dwBytesRecorded = len ;
	m_hdr.dwUser = 0;

	MMRESULT result = midiOutPrepareHeader (m_out, &m_hdr, sizeof (m_hdr));
	if (result != MMSYSERR_NOERROR) return;
	ResetEvent (m_event);
	result = midiOutLongMsg (m_out,&m_hdr,sizeof(m_hdr));
	if (result != MMSYSERR_NOERROR) {
		SetEvent (m_event);
		return;
	}
}
#endif