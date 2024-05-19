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