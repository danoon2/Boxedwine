#include "boxedwine.h"
#include "knativeaudio.h"
#include <SDL.h>
#include "../sdl/knativeaudiosdl.h"
#include <Windows.h>
#undef midiOutGetDevCaps

PACKED(
struct WineMidiHdr {
	U32       lpData;               /* pointer to locked data block */
	U32       dwBufferLength;       /* length of data in data block */
	U32       dwBytesRecorded;      /* used for input only */
	U32		  dwUser;               /* for client's use */
	U32       dwFlags;              /* assorted flags (see defines) */
	U32       lpNext;               /* reserved for driver */
	U32       reserved;             /* reserved for driver */
	U32       dwOffset;             /* Callback offset into buffer */
	U32       dwReserved[8];        /* Reserved for MMSYSTEM */

	// boxedwine stuff
	void* buffer;
	U32 bufferSize;

	void writeFlags(U32 address) {
		writed(address+16, dwFlags);
	}
}
);

class KNativeAudioWindows : public KNativeAudioSDL, public std::enable_shared_from_this<KNativeAudioWindows> {
public:
	KNativeAudioWindows() : m_out(0), prepared(false), wDevID(0), eventFd(0), hMidi(0), dwCallback(0), dwInstance(0), wFlags(0) {}
	virtual ~KNativeAudioWindows() {}

	virtual U32 midiOutOpen(U32 fd, U32 wDevID, U32 lpDesc, U32 dwFlags);
	virtual U32 midiOutClose(U32 wDevID);
	virtual U32 midiOutData(U32 wDevID, U32 dwParam);
	virtual U32 midiOutLongData(U32 wDevID, U32 lpMidiHdr, U32 dwSize);
	virtual U32 midiOutPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize);
	virtual U32 midiOutUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize);
	virtual U32 midiOutGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize);
	virtual U32 midiOutGetNumDevs();
	virtual U32 midiOutGetVolume(U32 wDevID, U32 lpdwVolume);
	virtual U32 midiOutSetVolume(U32 wDevID, U32 dwVolume);
	virtual U32 midiOutReset(U32 wDevID);

	HMIDIOUT m_out;


	MIDIHDR mhdr;
	WineMidiHdr wineMidiHdr;
	std::shared_ptr<KProcess> process;
	bool prepared;
	U32 wDevID;
	U32 eventFd;

	U32 hMidi;
	U32 dwCallback;
	U32 dwInstance;
	U32 wFlags;
};

static void write32(U8* buffer, U32 data) {
	buffer[0] = (U8)data;
	buffer[1] = (U8)(data >> 8);
	buffer[2] = (U8)(data >> 16);
	buffer[3] = (U8)(data >> 24);
}

static void CALLBACK midiCallback(HMIDIIN handle, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	KNativeAudioWindows* data = (KNativeAudioWindows*)dwInstance;
	if (data->eventFd) {
		KFileDescriptor* fd = data->process->getFileDescriptor(data->eventFd);
		if (fd) {
			U8 c = EVENT_MSG_NOTIFY;
			U8 buffer[32];
			// dwCallBack, uFlags, hDev, wMsg, dwInstance, dwParam1, dwParam2
			fd->kobject->writeNative(&c, 1);
			write32(buffer, data->dwCallback);
			write32(buffer + 4, data->wFlags);
			write32(buffer + 8, data->hMidi);
			write32(buffer + 12, uMsg);
			write32(buffer + 16, data->dwInstance);
			if (uMsg == MOM_DONE || uMsg == MOM_POSITIONCB) {
				// convert to 32-bit address lpMidiHdr 
				kpanic("TODO: need to convert lpMidiHdr");
				write32(buffer + 20, (U32)dwParam1);
			} else {
				write32(buffer + 20, (U32)dwParam1);
			}
			write32(buffer + 24, (U32)dwParam2);
			fd->kobject->writeNative(buffer, 32);
		}
	}
}

/*
typedef struct {
	HMIDI			hMidi;
	DWORD_PTR		dwCallback;
	DWORD_PTR		dwInstance;
	DWORD_PTR		dnDevNode;
	DWORD          		cIds;
	MIDIOPENSTRMID 		rgIds;
} MIDIOPENDESC, * LPMIDIOPENDESC;
*/

U32 KNativeAudioWindows::midiOutOpen(U32 wDevID, U32 lpDesc, U32 dwFlags, U32 fd) {
	this->process = KThread::currentThread()->process;
	this->wDevID = wDevID;
	this->eventFd = fd;
	if (lpDesc) {
		this->hMidi = readd(lpDesc);
		this->dwCallback = readd(lpDesc+4);
		this->dwInstance = readd(lpDesc+8);
	}
	this->wFlags = HIWORD(dwFlags & CALLBACK_TYPEMASK);

	// result should match what wine is expecting
	if (dwFlags) {
		// I was unable to find a midi player or a game that triggered this code
		kpanic("KNativeAudioWindows::midiOutOpen Midi with a callback needs to be tested");
		return ::midiOutOpen(&m_out, wDevID, (DWORD_PTR)midiCallback, (DWORD_PTR)this, CALLBACK_FUNCTION);
	}
	return ::midiOutOpen(&m_out, wDevID, NULL, 0, 0);
}

U32 KNativeAudioWindows::midiOutClose(U32 wDevID) {
	U32 result = ::midiOutClose(m_out);
	m_out = NULL;
	return result;
}

U32 KNativeAudioWindows::midiOutData(U32 wDevID, U32 dwParam) {
	return ::midiOutShortMsg(m_out, dwParam);
}

U32 KNativeAudioWindows::midiOutLongData(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	memcopyToNative(lpMidiHdr, &wineMidiHdr, dwSize);
	mhdr.dwFlags = wineMidiHdr.dwFlags;
	wineMidiHdr.dwFlags |= MHDR_INQUEUE;
	wineMidiHdr.writeFlags(lpMidiHdr);
	U32 result = ::midiOutLongMsg(m_out, &mhdr, sizeof(mhdr));
	wineMidiHdr.dwFlags = mhdr.dwFlags;
	wineMidiHdr.writeFlags(lpMidiHdr);
	return result;
}

U32 KNativeAudioWindows::midiOutPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	if (this->prepared) {
		kpanic("KNativeAudioWindows::midiOutPrepare need to add support for more than one repared midi header");
	}
	this->prepared = true;
	memset(&wineMidiHdr, 0, sizeof(wineMidiHdr));
	memset(&mhdr, 0, sizeof(mhdr));
	memcopyToNative(lpMidiHdr, &wineMidiHdr, dwSize);
	mhdr.lpData = (LPSTR)getPhysicalAddress(wineMidiHdr.lpData, mhdr.dwBufferLength);
	if (!mhdr.lpData) {
		wineMidiHdr.buffer = malloc(mhdr.dwBufferLength);
		memcopyToNative(wineMidiHdr.lpData, wineMidiHdr.buffer, mhdr.dwBufferLength);
		mhdr.lpData = (LPSTR)wineMidiHdr.buffer;
	}
	mhdr.dwBufferLength = wineMidiHdr.dwBufferLength;
	mhdr.dwFlags = wineMidiHdr.dwFlags;
	mhdr.dwBytesRecorded = wineMidiHdr.dwBytesRecorded;
	U32 result = ::midiOutPrepareHeader(m_out, &mhdr, sizeof(mhdr));
	wineMidiHdr.dwFlags = mhdr.dwFlags;
	wineMidiHdr.writeFlags(lpMidiHdr);
	return result;
}

U32 KNativeAudioWindows::midiOutUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	this->prepared = false;
	U32 result = ::midiOutUnprepareHeader(m_out, &mhdr, sizeof(mhdr));
	wineMidiHdr.dwFlags = mhdr.dwFlags;
	memcopyFromNative(lpMidiHdr, &wineMidiHdr, dwSize);
	if (wineMidiHdr.buffer) {
		::free(wineMidiHdr.buffer);
		wineMidiHdr.buffer = NULL;
	}
	return result;
}

U32 KNativeAudioWindows::midiOutGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) {
	MIDIOUTCAPSW caps;
	U32 result = ::midiOutGetDevCapsW(wDevID, &caps, sizeof(caps));
	memcopyFromNative(lpCaps, &caps, dwSize);
	return result;
}

U32 KNativeAudioWindows::midiOutGetNumDevs() {
	return ::midiOutGetNumDevs();
}

U32 KNativeAudioWindows::midiOutGetVolume(U32 wDevID, U32 lpdwVolume) {
	DWORD volume;
	U32 result = ::midiOutGetVolume(m_out, &volume);
	writed(lpdwVolume, volume);
	return result;
}

U32 KNativeAudioWindows::midiOutSetVolume(U32 wDevID, U32 dwVolume) {
	return ::midiOutSetVolume(m_out, dwVolume);
}

U32 KNativeAudioWindows::midiOutReset(U32 wDevID) {
	return ::midiOutReset(m_out);
}

void initWindowsAudio() {
	KNativeAudio::availableAudio.push_back(std::make_shared<KNativeAudioWindows>());
}