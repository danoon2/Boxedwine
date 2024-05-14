#pragma GCC diagnostic ignored "-Wreturn-type"
#define NONAMELESSUNION
#define COBJMACROS
//#include "config.h"

#include <stdarg.h>
//#include <stdlib.h>

#include "windef.h"
#include "winbase.h"
//#include "winternl.h"
#include "wingdi.h"
#include "winuser.h"
#include "mmddk.h"
#include "wine/debug.h"

#include "winnls.h"
#include "winreg.h"
#include "wine/debug.h"
//#include "wine/unicode.h"
#include "wine/list.h"

#include "ole2.h"
#include "mmdeviceapi.h"
#include "devpkey.h"
#include "dshow.h"
#include "dsound.h"
#include "shlwapi.h"

#include "initguid.h"
#include "endpointvolume.h"
#include "audioclient.h"
#include "audiopolicy.h"

//#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

int pipe(int pipefd[2]);

WINE_DEFAULT_DEBUG_CHANNEL(boxedaudio);

#define BOXED_BASE 200

#define BOXED_AUDIO_DRV_LOAD                        (BOXED_BASE)
#define BOXED_AUDIO_DRV_FREE                        (BOXED_BASE+1)
#define BOXED_AUDIO_DRV_OPEN                        (BOXED_BASE+2)
#define BOXED_AUDIO_DRV_CLOSE                       (BOXED_BASE+3)
#define BOXED_AUDIO_DRV_CONFIGURE                   (BOXED_BASE+4)
#define BOXED_AUDIO_MIDIOUT_OPEN                    (BOXED_BASE+5)
#define BOXED_AUDIO_MIDIOUT_CLOSE                   (BOXED_BASE+6)
#define BOXED_AUDIO_MIDIOUT_DATA                    (BOXED_BASE+7)
#define BOXED_AUDIO_MIDIOUT_LONGDATA                (BOXED_BASE+8)
#define BOXED_AUDIO_MIDIOUT_PREPARE                 (BOXED_BASE+9)
#define BOXED_AUDIO_MIDIOUT_UNPREPARE               (BOXED_BASE+10)
#define BOXED_AUDIO_MIDIOUT_GETDEVCAPS              (BOXED_BASE+11)
#define BOXED_AUDIO_MIDIOUT_GETNUMDEVS              (BOXED_BASE+12)
#define BOXED_AUDIO_MIDIOUT_GETVOLUME               (BOXED_BASE+13)
#define BOXED_AUDIO_MIDIOUT_SETVOLUME               (BOXED_BASE+14)
#define BOXED_AUDIO_MIDIOUT_RESET                   (BOXED_BASE+15)
#define BOXED_AUDIO_MIDIIN_OPEN                     (BOXED_BASE+16)
#define BOXED_AUDIO_MIDIIN_CLOSE                    (BOXED_BASE+17)
#define BOXED_AUDIO_MIDIIN_ADDBUFFER                (BOXED_BASE+18)
#define BOXED_AUDIO_MIDIIN_PREPARE                  (BOXED_BASE+19)
#define BOXED_AUDIO_MIDIIN_UNPREPARE                (BOXED_BASE+20)
#define BOXED_AUDIO_MIDIIN_GETDEVCAPS               (BOXED_BASE+21)
#define BOXED_AUDIO_MIDIIN_GETNUMDEVS               (BOXED_BASE+22)
#define BOXED_AUDIO_MIDIIN_START                    (BOXED_BASE+23)
#define BOXED_AUDIO_MIDIIN_STOP                     (BOXED_BASE+24)
#define BOXED_AUDIO_MIDIIN_RESET                    (BOXED_BASE+25)
#define BOXED_AUDIO_DRV_HAS_DEVICE                  (BOXED_BASE+26)
#define BOXED_AUDIO_DRV_GET_ENDPOINT                (BOXED_BASE+27)
#define BOXED_AUDIO_DRV_RELEASE                     (BOXED_BASE+28)
#define BOXED_AUDIO_DRV_SET_VOLUME                  (BOXED_BASE+29)
#define BOXED_AUDIO_DRV_IS_FORMAT_SUPPORTED         (BOXED_BASE+30)
#define BOXED_AUDIO_DRV_GET_MIX_FORMAT              (BOXED_BASE+31)
#define BOXED_AUDIO_DRV_LOCK                        (BOXED_BASE+32)
#define BOXED_AUDIO_DRV_UNLOCK                      (BOXED_BASE+33)
#define BOXED_AUDIO_DRV_CAPTURE_RESAMPLE            (BOXED_BASE+34)

#define BOXED_AUDIO_DRV_GET_LATENCY                 (BOXED_BASE+35)
#define BOXED_AUDIO_DRV_INIT                        (BOXED_BASE+36)
#define BOXED_AUDIO_DRV_START                       (BOXED_BASE+37)
#define BOXED_AUDIO_DRV_STOP                        (BOXED_BASE+38)
#define BOXED_AUDIO_DRV_GET_PERIOD                  (BOXED_BASE+39)
#define BOXED_AUDIO_DRV_USE_TIMER                   (BOXED_BASE+40)
#define BOXED_AUDIO_DRV_SET_PRIORITY                (BOXED_BASE+41)
#define BOXED_WINDOW_POS_CHANGING					(BOXED_BASE+42)

#define CALL_0(index) __asm__("push %1\n\tint $0x98\n\taddl $4, %%esp": "=a" (result):"i"(index):); 
#define CALL_1(index, arg1) __asm__("push %2\n\tpush %1\n\tint $0x98\n\taddl $8, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1):); 
#define CALL_2(index, arg1,arg2) __asm__("push %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $12, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2):);
#define CALL_3(index, arg1,arg2,arg3) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $16, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3):);
#define CALL_4(index, arg1,arg2,arg3,arg4) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $20, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4):);
#define CALL_5(index, arg1,arg2,arg3,arg4,arg5) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $24, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5):);
#define CALL_6(index, arg1,arg2,arg3,arg4,arg5,arg6) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $28, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6):);
#define CALL_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $32, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7):);
#define CALL_8(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $36, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8):);
#define CALL_9(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tint $0x98\n\taddl $40, %%esp": "=a" (result):"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8), "g"((DWORD)arg9):);

#define CALL_NORETURN_0(index) __asm__("push %0\n\tint $0x98\n\taddl $4, %%esp"::"i"(index)); 
#define CALL_NORETURN_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x98\n\taddl $8, %%esp"::"i"(index), "g"((DWORD)arg1)); 
#define CALL_NORETURN_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $12, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2)); 
#define CALL_NORETURN_3(index, arg1, arg2, arg3) __asm__("push %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $16, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3)); 
#define CALL_NORETURN_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $20, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4)); 
#define CALL_NORETURN_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\rpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $24, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5)); 
#define CALL_NORETURN_6(index, arg1,arg2,arg3,arg4,arg5,arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $28, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6));
#define CALL_NORETURN_7(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $32, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7));
#define CALL_NORETURN_8(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $36, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8));
#define CALL_NORETURN_9(index, arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x98\n\taddl $40, %%esp"::"i"(index), "g"((DWORD)arg1), "g"((DWORD)arg2), "g"((DWORD)arg3), "g"((DWORD)arg4), "g"((DWORD)arg5), "g"((DWORD)arg6), "g"((DWORD)arg7), "g"((DWORD)arg8), "g"((DWORD)arg9));

struct int2Float {
    union {
        UINT32 i;
        float f;
    };
};

int pipeFd[2];
static HANDLE thread;
static HANDLE dsoundEvent;

static unsigned char read8(void) {
    unsigned char b;
    while (read(pipeFd[0], &b, 1) == 0) {
    }
    return b;
}

static DWORD read32(void) {
    DWORD result = read8();
    result |= (read8() << 8);
    result |= (read8() << 16);
    result |= (read8() << 24);
    return result;
}

static DWORD CALLBACK msg_thread(void *p) {
    char b;

    TRACE("Start\n");
    while (read(pipeFd[0], &b, 1)>=0) {
        if (b == 1) {
            TRACE("Pump");
            if (dsoundEvent) {
                TRACE(" Event");
                SetEvent(dsoundEvent);
            }        
            TRACE("\n");
        } else if (b == 2) {
            DWORD dwCallBack, uFlags, wMsg, dwInstance , dwParam1, dwParam2;
            HDRVR hDev;
            dwCallBack = read32();
            uFlags = read32();
            hDev = (HDRVR)read32();
            wMsg = read32();
            dwInstance = read32();
            dwParam1 = read32();
            dwParam2 = read32();
            TRACE("MIDI_NotifyClient dwCallBack=%x uFlags=%X hDev=%X wMsg=%d dwInstance=%X dwParam1=%X dwParam2=%X\n", dwCallBack, uFlags, hDev, wMsg, dwInstance, dwParam1, dwParam2);
            DriverCallback(dwCallBack, uFlags, hDev, wMsg, dwInstance, dwParam1, dwParam2);
        }
    }
    TRACE("Exit\n");
    thread = NULL;
}

/**************************************************************************
* 				DriverProc (WINECOREAUDIO.1)
*/
LRESULT CALLBACK BoxedAudio_DriverProc(DWORD_PTR dwDevID, HDRVR hDriv, UINT wMsg,
	LPARAM dwParam1, LPARAM dwParam2)
{
        int result = 0;

	TRACE("(%08lX, %p, %s (%08X), %08lX, %08lX)\n",
		dwDevID, hDriv, wMsg == DRV_LOAD ? "DRV_LOAD" :
		wMsg == DRV_FREE ? "DRV_FREE" :
		wMsg == DRV_OPEN ? "DRV_OPEN" :
		wMsg == DRV_CLOSE ? "DRV_CLOSE" :
		wMsg == DRV_ENABLE ? "DRV_ENABLE" :
		wMsg == DRV_DISABLE ? "DRV_DISABLE" :
		wMsg == DRV_QUERYCONFIGURE ? "DRV_QUERYCONFIGURE" :
		wMsg == DRV_CONFIGURE ? "DRV_CONFIGURE" :
		wMsg == DRV_INSTALL ? "DRV_INSTALL" :
		wMsg == DRV_REMOVE ? "DRV_REMOVE" : "UNKNOWN",
		wMsg, dwParam1, dwParam2);

	switch (wMsg) {
	case DRV_LOAD: {
		CALL_0(BOXED_AUDIO_DRV_LOAD);
                return result == 0 ? DRV_FAILURE : DRV_SUCCESS;
            }
	case DRV_FREE: {
		CALL_0(BOXED_AUDIO_DRV_FREE);
                return result;
            }
	case DRV_OPEN: {
		CALL_1(BOXED_AUDIO_DRV_OPEN, (DWORD)(LPSTR)dwParam1);
                return result;
            }
	case DRV_CLOSE: {
		CALL_1(BOXED_AUDIO_DRV_CLOSE, (DWORD)dwDevID);
                return result;
            }
	case DRV_ENABLE:		return 1;
	case DRV_DISABLE:		return 1;
	case DRV_QUERYCONFIGURE:	return 1;
	case DRV_CONFIGURE: {
		CALL_0(BOXED_AUDIO_DRV_CONFIGURE);
                return result;
        }
	case DRV_INSTALL:		return DRVCNF_RESTART;
	case DRV_REMOVE:		return DRVCNF_RESTART;
	default:
		return DefDriverProc(dwDevID, hDriv, wMsg, dwParam1, dwParam2);
	}
}

/**************************************************************************
* 				modMessage
*/
DWORD WINAPI BoxedAudio_modMessage(UINT wDevID, UINT wMsg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
        int result = 0;
	TRACE("%d %08x %08x %08x %08x\n", wDevID, wMsg, dwUser, dwParam1, dwParam2);

	switch (wMsg) {
	case DRVM_INIT:
	case DRVM_EXIT:
	case DRVM_ENABLE:
	case DRVM_DISABLE:
		return 0;
	case MODM_OPEN: {
                if (!thread) {
                    pipe( pipeFd );
                    thread = CreateThread(0, 0, msg_thread, NULL, 0, 0);
                    TRACE("created msg thread: %d\n", (int)thread);
                }
		CALL_3(BOXED_AUDIO_MIDIOUT_OPEN, wDevID, (DWORD)(LPMIDIOPENDESC)dwParam1, dwParam2);
                return result;
	    }
        case MODM_CLOSE: {
		CALL_1(BOXED_AUDIO_MIDIOUT_CLOSE, wDevID);
                return result;
	    }
        case MODM_DATA: {
		CALL_2(BOXED_AUDIO_MIDIOUT_DATA, wDevID, dwParam1);
                return result;
	    }
        case MODM_LONGDATA: {
		CALL_3(BOXED_AUDIO_MIDIOUT_LONGDATA, wDevID, (DWORD)(LPMIDIHDR)dwParam1, dwParam2);
                return result;
	    }
        case MODM_PREPARE: {
		CALL_3(BOXED_AUDIO_MIDIOUT_PREPARE, wDevID, (DWORD)(LPMIDIHDR)dwParam1, dwParam2);
                return result;
	    }
        case MODM_UNPREPARE: {
		CALL_3(BOXED_AUDIO_MIDIOUT_UNPREPARE, wDevID, (DWORD)(LPMIDIHDR)dwParam1, dwParam2);
                return result;
	    }
        case MODM_GETDEVCAPS: {
		CALL_3(BOXED_AUDIO_MIDIOUT_GETDEVCAPS, wDevID, (DWORD)(LPMIDIOUTCAPSW)dwParam1, dwParam2);
                return result;
            }
	case MODM_GETNUMDEVS: {
		CALL_0(BOXED_AUDIO_MIDIOUT_GETNUMDEVS);
                return result;
	    }
        case MODM_GETVOLUME: {
		CALL_2(BOXED_AUDIO_MIDIOUT_GETVOLUME, wDevID, (DWORD)(DWORD*)dwParam1);
                return result;
	    }
        case MODM_SETVOLUME: {
		CALL_2(BOXED_AUDIO_MIDIOUT_SETVOLUME, wDevID, dwParam1);
                return result;
	    }
        case MODM_RESET: {
		CALL_1(BOXED_AUDIO_MIDIOUT_RESET, wDevID);
                return result;
	    }
        default:
		TRACE("Unsupported message (08%x)\n", wMsg);
	}
	return MMSYSERR_NOTSUPPORTED;
}

/**************************************************************************
* 			midMessage
*/
DWORD WINAPI BoxedAudio_midMessage(UINT wDevID, UINT wMsg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
        int result = 0;
	TRACE("%d %08x %08x %08x %08x\n", wDevID, wMsg, dwUser, dwParam1, dwParam2);
	switch (wMsg) {
	case DRVM_INIT:
	case DRVM_EXIT:
	case DRVM_ENABLE:
	case DRVM_DISABLE:
		return 0;
	case MIDM_OPEN:
		CALL_3(BOXED_AUDIO_MIDIIN_OPEN, wDevID, (DWORD)(LPMIDIOPENDESC)dwParam1, dwParam2);
		return result;
	case MIDM_CLOSE:
		CALL_1(BOXED_AUDIO_MIDIIN_CLOSE, wDevID);
		return result;
	case MIDM_ADDBUFFER:
		CALL_3(BOXED_AUDIO_MIDIIN_ADDBUFFER, wDevID, (DWORD)(LPMIDIHDR)dwParam1, dwParam2);
		return result;
	case MIDM_PREPARE:
		CALL_3(BOXED_AUDIO_MIDIIN_PREPARE, wDevID, (DWORD)(LPMIDIHDR)dwParam1, dwParam2);
		return result;
	case MIDM_UNPREPARE:
		CALL_3(BOXED_AUDIO_MIDIIN_UNPREPARE, wDevID, (DWORD)(LPMIDIHDR)dwParam1, dwParam2);
		return result;
	case MIDM_GETDEVCAPS:
		CALL_3(BOXED_AUDIO_MIDIIN_GETDEVCAPS, wDevID, (DWORD)(LPMIDIINCAPSW)dwParam1, dwParam2);
		return result;
	case MIDM_GETNUMDEVS:
		CALL_0(BOXED_AUDIO_MIDIIN_GETNUMDEVS);
		return result;
	case MIDM_START:
		CALL_1(BOXED_AUDIO_MIDIIN_START, wDevID);
		return result;
	case MIDM_STOP:
		CALL_1(BOXED_AUDIO_MIDIIN_STOP, wDevID);
		return result;
	case MIDM_RESET:
		CALL_1(BOXED_AUDIO_MIDIIN_RESET, wDevID);
		return result;
	default:
		TRACE("Unsupported message\n");
	}
	return MMSYSERR_NOTSUPPORTED;
}

#define NULL_PTR_ERR MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, RPC_X_NULL_REF_POINTER)
#define Float32 float
static REFERENCE_TIME DefaultPeriod = 100000;
static REFERENCE_TIME MinimumPeriod = 50000;

struct ACImpl;
typedef struct ACImpl ACImpl;

#define AudioDeviceID UINT32
static HRESULT audio_setvol(ACImpl *This, UINT32 index);

typedef struct _AudioSession {
	GUID guid;
	struct list clients;

	IMMDevice *device;

	float master_vol;
	UINT32 channel_count;
	float *channel_vols;
	BOOL mute;

	CRITICAL_SECTION lock;

	struct list entry;
} AudioSession;

typedef struct _AudioSessionWrapper {
	IAudioSessionControl2 IAudioSessionControl2_iface;
	IChannelAudioVolume IChannelAudioVolume_iface;
	ISimpleAudioVolume ISimpleAudioVolume_iface;

	LONG ref;

	ACImpl *client;
	AudioSession *session;
} AudioSessionWrapper;

struct ACImpl {
	IAudioClient IAudioClient_iface;
	IAudioRenderClient IAudioRenderClient_iface;
	IAudioCaptureClient IAudioCaptureClient_iface;
	IAudioClock IAudioClock_iface;
	IAudioClock2 IAudioClock2_iface;
	IAudioStreamVolume IAudioStreamVolume_iface;

	LONG ref;

	IMMDevice *parent;
	IUnknown *pUnkFTMarshal;

	WAVEFORMATEX *fmt;

	EDataFlow dataflow;
	DWORD flags;
	AUDCLNT_SHAREMODE share;
	HANDLE event;
	float *vols;

	BOOL initted;
		
	UINT boxedAudioId;

	HANDLE timer;
					
	BYTE *tmp_buffer; // used in get/release buffer

	// not used in callback
	INT32 getbuf_last;

	// set only in AudioClient_Initialize
	UINT32 bufsize_frames, period_frames, period_ms;
	//UINT32 cap_bufsize_frames;
	//BYTE *cap_buffer;
	BYTE *local_buffer;

	BOOL playing; // only set in AudioClient_Start, AudioClient_Stop

	// updated and used only in GetBuffer
	UINT32 tmp_buffer_frames;

	// updated in ReleaseBuffer
	UINT64 written_frames;
	UINT32 wri_offs_frames;

	// used in callback (NEEDS TO BE PROTECTED)
	UINT32 lcl_offs_frames, held_frames; // updated in ReleaseBuffer

	// used in capture callback (NEEDS TO BE PROTECTED)
	UINT32 wrap_bufsize_frames; // updated in AudioClient_GetCurrentPadding_nolock
	BYTE *wrap_buffer; // updated in AudioClient_GetCurrentPadding_nolock

	AudioSession *session;
	AudioSessionWrapper *session_wrapper;

	struct list entry;

	CRITICAL_SECTION lock;
        int lastSetPriority;
};

static const IAudioClientVtbl AudioClient_Vtbl;
static const IAudioRenderClientVtbl AudioRenderClient_Vtbl;
static const IAudioCaptureClientVtbl AudioCaptureClient_Vtbl;
static const IAudioSessionControl2Vtbl AudioSessionControl2_Vtbl;
static const ISimpleAudioVolumeVtbl SimpleAudioVolume_Vtbl;
static const IAudioClockVtbl AudioClock_Vtbl;
static const IAudioClock2Vtbl AudioClock2_Vtbl;
static const IAudioStreamVolumeVtbl AudioStreamVolume_Vtbl;
static const IChannelAudioVolumeVtbl ChannelAudioVolume_Vtbl;
static const IAudioSessionManager2Vtbl AudioSessionManager2_Vtbl;

typedef struct _SessionMgr {
	IAudioSessionManager2 IAudioSessionManager2_iface;

	LONG ref;

	IMMDevice *device;
} SessionMgr;

static const WCHAR drv_key_devicesW[] = { 'S','o','f','t','w','a','r','e','\\',
'W','i','n','e','\\','D','r','i','v','e','r','s','\\',
'w','i','n','e','b','o','x','e','d','a','d','i','o','.','d','r','v','\\','d','e','v','i','c','e','s',0 };
static const WCHAR guidW[] = { 'g','u','i','d',0 };

static HANDLE g_timer_q;

static CRITICAL_SECTION g_sessions_lock;
static CRITICAL_SECTION_DEBUG g_sessions_lock_debug =
{
	0, 0, &g_sessions_lock,
{ &g_sessions_lock_debug.ProcessLocksList, &g_sessions_lock_debug.ProcessLocksList },
0, 0,{ (DWORD_PTR)(__FILE__ ": g_sessions_lock") }
};
static CRITICAL_SECTION g_sessions_lock = { &g_sessions_lock_debug, -1, 0, 0, 0, 0 };
static struct list g_sessions = LIST_INIT(g_sessions);

static AudioSessionWrapper *AudioSessionWrapper_Create(ACImpl *client);

static inline ACImpl *impl_from_IAudioClient(IAudioClient *iface)
{
	return CONTAINING_RECORD(iface, ACImpl, IAudioClient_iface);
}

static inline ACImpl *impl_from_IAudioRenderClient(IAudioRenderClient *iface)
{
	return CONTAINING_RECORD(iface, ACImpl, IAudioRenderClient_iface);
}

static inline ACImpl *impl_from_IAudioCaptureClient(IAudioCaptureClient *iface)
{
	return CONTAINING_RECORD(iface, ACImpl, IAudioCaptureClient_iface);
}

static inline AudioSessionWrapper *impl_from_IAudioSessionControl2(IAudioSessionControl2 *iface)
{
	return CONTAINING_RECORD(iface, AudioSessionWrapper, IAudioSessionControl2_iface);
}

static inline AudioSessionWrapper *impl_from_ISimpleAudioVolume(ISimpleAudioVolume *iface)
{
	return CONTAINING_RECORD(iface, AudioSessionWrapper, ISimpleAudioVolume_iface);
}

static inline AudioSessionWrapper *impl_from_IChannelAudioVolume(IChannelAudioVolume *iface)
{
	return CONTAINING_RECORD(iface, AudioSessionWrapper, IChannelAudioVolume_iface);
}

static inline ACImpl *impl_from_IAudioClock(IAudioClock *iface)
{
	return CONTAINING_RECORD(iface, ACImpl, IAudioClock_iface);
}

static inline ACImpl *impl_from_IAudioClock2(IAudioClock2 *iface)
{
	return CONTAINING_RECORD(iface, ACImpl, IAudioClock2_iface);
}

static inline ACImpl *impl_from_IAudioStreamVolume(IAudioStreamVolume *iface)
{
	return CONTAINING_RECORD(iface, ACImpl, IAudioStreamVolume_iface);
}

static inline SessionMgr *impl_from_IAudioSessionManager2(IAudioSessionManager2 *iface)
{
	return CONTAINING_RECORD(iface, SessionMgr, IAudioSessionManager2_iface);
}

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, void *reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		g_timer_q = CreateTimerQueue();
		if (!g_timer_q)
			return FALSE;
		break;

	case DLL_PROCESS_DETACH:
		if (reserved) break;
		DeleteCriticalSection(&g_sessions_lock);
		break;
	}
	return TRUE;
}

/* From <dlls/mmdevapi/mmdevapi.h> */
enum DriverPriority {
	Priority_Unavailable = 0,
	Priority_Low,
	Priority_Neutral,
	Priority_Preferred
};

int WINAPI AUDDRV_GetPriority(void)
{
	return Priority_Neutral;
}

static void set_device_guid(EDataFlow flow, HKEY drv_key, const WCHAR *key_name,
	GUID *guid)
{
	HKEY key;
	BOOL opened = FALSE;
	LONG lr;

	if (!drv_key) {
		lr = RegCreateKeyExW(HKEY_CURRENT_USER, drv_key_devicesW, 0, NULL, 0, KEY_WRITE,
			NULL, &drv_key, NULL);
		if (lr != ERROR_SUCCESS) {
			ERR("RegCreateKeyEx(drv_key) failed: %u\n", lr);
			return;
		}
		opened = TRUE;
	}

	lr = RegCreateKeyExW(drv_key, key_name, 0, NULL, 0, KEY_WRITE,
		NULL, &key, NULL);
	if (lr != ERROR_SUCCESS) {
		ERR("RegCreateKeyEx(%s) failed: %u\n", wine_dbgstr_w(key_name), lr);
		goto exit;
	}

	lr = RegSetValueExW(key, guidW, 0, REG_BINARY, (BYTE*)guid,
		sizeof(GUID));
	if (lr != ERROR_SUCCESS)
		ERR("RegSetValueEx(%s\\guid) failed: %u\n", wine_dbgstr_w(key_name), lr);

	RegCloseKey(key);
exit:
	if (opened)
		RegCloseKey(drv_key);
}

static GUID guid0;
static GUID guid1;

void WINAPI get_device_guid(EDataFlow flow, AudioDeviceID device, GUID *guid)
{
	HKEY key = NULL, dev_key;
	DWORD type, size = sizeof(*guid);
	WCHAR key_name[256];

	static const WCHAR key_fmt[] = { '%','u',0 };

	if (flow == eCapture)
		key_name[0] = '1';
	else
		key_name[0] = '0';
	key_name[1] = ',';

	wsprintfW(key_name + 2, key_fmt, device);

	if (RegOpenKeyExW(HKEY_CURRENT_USER, drv_key_devicesW, 0, KEY_WRITE | KEY_READ, &key) == ERROR_SUCCESS) {
		if (RegOpenKeyExW(key, key_name, 0, KEY_READ, &dev_key) == ERROR_SUCCESS) {
			if (RegQueryValueExW(dev_key, guidW, 0, &type,
				(BYTE*)guid, &size) == ERROR_SUCCESS) {
				if (type == REG_BINARY) {
					RegCloseKey(dev_key);
					RegCloseKey(key);
					return;
				}
				ERR("Invalid type for device %s GUID: %u; ignoring and overwriting\n",
					wine_dbgstr_w(key_name), type);
			}
			RegCloseKey(dev_key);
		}
	}

	CoCreateGuid(guid);

	set_device_guid(flow, key, key_name, guid);

	if (key)
		RegCloseKey(key);
	if (flow == 0) {
		guid0 = guid;
	} else {
		guid1 = guid;
	}
}

static const WCHAR deviceName[] = { 'B','o','x','e', 'd', 'w', 'i', 'n', 'e', ' ', 'a', 'u', 'd', 'i', 'o', 0 };
HRESULT WINAPI AUDDRV_GetEndpointIDs(EDataFlow flow, WCHAR ***ids,
	GUID **guids, UINT *num, UINT *def_index)
{
	UINT hasDevice;
	UINT result = 0;

	TRACE("%d %p %p %p\n", flow, ids, num, def_index);

	CALL_1(BOXED_AUDIO_DRV_HAS_DEVICE, flow == eRender ? 1 : 0);
        hasDevice = result;
	if (hasDevice != 0) {
		return hasDevice;
	}

	*ids = HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR *));
	if (!*ids) {
		return E_OUTOFMEMORY;
	}

	*guids = HeapAlloc(GetProcessHeap(), 0, sizeof(GUID));
	if (!*guids) {
		HeapFree(GetProcessHeap(), 0, *ids);
		return E_OUTOFMEMORY;
	}

	*num = 1;
	*def_index = 0;
    (*ids)[0] = HeapAlloc(GetProcessHeap(), 0, sizeof(deviceName));
	if (!(*ids)[0]) {
		HeapFree(GetProcessHeap(), 0, *ids);
		HeapFree(GetProcessHeap(), 0, *guids);
		return E_OUTOFMEMORY;
	}
	lstrcpyW((*ids)[0], deviceName);
	get_device_guid(flow, 1, &(*guids)[0]);

	TRACE("device 0: id %s key 1(default)\n", debugstr_w(deviceName));

	return S_OK;
}

static BOOL get_deviceid_by_guid(GUID *guid, AudioDeviceID *id, EDataFlow *flow)
{
	HKEY devices_key;
	UINT i = 0;
	WCHAR key_name[256];
	DWORD key_name_size;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, drv_key_devicesW, 0, KEY_READ, &devices_key) != ERROR_SUCCESS) {
		ERR("No devices in registry?\n");
		return FALSE;
	}

	while (1) {
		HKEY key;
		DWORD size, type;
		GUID reg_guid;

		key_name_size = sizeof(key_name);
		if (RegEnumKeyExW(devices_key, i++, key_name, &key_name_size, NULL,
			NULL, NULL, NULL) != ERROR_SUCCESS)
			break;

		if (RegOpenKeyExW(devices_key, key_name, 0, KEY_READ, &key) != ERROR_SUCCESS) {
			WARN("Couldn't open key: %s\n", wine_dbgstr_w(key_name));
			continue;
		}

		size = sizeof(reg_guid);
		if (RegQueryValueExW(key, guidW, 0, &type,
			(BYTE*)&reg_guid, &size) == ERROR_SUCCESS) {
			if (IsEqualGUID(&reg_guid, guid)) {
				RegCloseKey(key);
				RegCloseKey(devices_key);

				TRACE("Found matching device key: %s\n", wine_dbgstr_w(key_name));

				if (key_name[0] == '0')
					*flow = eRender;
				else if (key_name[0] == '1')
					*flow = eCapture;
				else {
					ERR("Unknown device type: %c\n", key_name[0]);
					return FALSE;
				}

				*id = wcstoul(key_name + 2, NULL, 10);

				return TRUE;
			}
		}

		RegCloseKey(key);
	}

	RegCloseKey(devices_key);

	WARN("No matching device in registry for GUID %s\n", debugstr_guid(guid));

	return FALSE;
}

HRESULT WINAPI AUDDRV_GetAudioEndpoint(GUID *guid, IMMDevice *dev, IAudioClient **out)
{
	ACImpl *This;
	AudioDeviceID adevid;
	EDataFlow dataflow;
	HRESULT hr;
	int result;

	TRACE("%s %p %p\n", debugstr_guid(guid), dev, out);

	if (!get_deviceid_by_guid(guid, &adevid, &dataflow))
		return AUDCLNT_E_DEVICE_INVALIDATED;

	This = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ACImpl));
	if (!This)
		return E_OUTOFMEMORY;

	This->IAudioClient_iface.lpVtbl = &AudioClient_Vtbl;
	This->IAudioRenderClient_iface.lpVtbl = &AudioRenderClient_Vtbl;
	This->IAudioCaptureClient_iface.lpVtbl = &AudioCaptureClient_Vtbl;
	This->IAudioClock_iface.lpVtbl = &AudioClock_Vtbl;
	This->IAudioClock2_iface.lpVtbl = &AudioClock2_Vtbl;
	This->IAudioStreamVolume_iface.lpVtbl = &AudioStreamVolume_Vtbl;

	This->dataflow = dataflow;

	if (dataflow != eRender && dataflow != eCapture) {
		HeapFree(GetProcessHeap(), 0, This);
		return E_INVALIDARG;
	}

        InitializeCriticalSection(&This->lock);

	hr = CoCreateFreeThreadedMarshaler((IUnknown *)&This->IAudioClient_iface,
		(IUnknown **)&This->pUnkFTMarshal);
	if (FAILED(hr)) {
		HeapFree(GetProcessHeap(), 0, This);
		return hr;
	}

	This->parent = dev;
	IMMDevice_AddRef(This->parent);

	CALL_2(BOXED_AUDIO_DRV_GET_ENDPOINT, (This->dataflow == eRender ? 1 : 0), adevid);
        This->boxedAudioId = result;
	if (!This->boxedAudioId) {
		HeapFree(GetProcessHeap(), 0, This);
		return AUDCLNT_E_DEVICE_INVALIDATED;
	}

	*out = &This->IAudioClient_iface;
	IAudioClient_AddRef(&This->IAudioClient_iface);

	return S_OK;
}

static HRESULT WINAPI AudioClient_QueryInterface(IAudioClient *iface,
	REFIID riid, void **ppv)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IAudioClient))
		*ppv = iface;
	else if (IsEqualIID(riid, &IID_IMarshal))
		return IUnknown_QueryInterface(This->pUnkFTMarshal, riid, ppv);

	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}
	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI AudioClient_AddRef(IAudioClient *iface)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	ULONG ref;
	ref = InterlockedIncrement(&This->ref);
	TRACE("(%p) Refcount now %u\n", This, ref);
	return ref;
}

static ULONG WINAPI AudioClient_Release(IAudioClient *iface)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	ULONG ref;
	ref = InterlockedDecrement(&This->ref);
	TRACE("(%p) Refcount now %u\n", This, ref);
	if (!ref) {
		if (This->timer) {
			HANDLE event;
			BOOL wait;
			event = CreateEventW(NULL, TRUE, FALSE, NULL);
			wait = !DeleteTimerQueueTimer(g_timer_q, This->timer, event);
			wait = wait && GetLastError() == ERROR_IO_PENDING;
			if (event && wait)
				WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		CALL_NORETURN_1(BOXED_AUDIO_DRV_RELEASE, This->boxedAudioId);
		if (This->session) {
			EnterCriticalSection(&g_sessions_lock);
			list_remove(&This->entry);
			LeaveCriticalSection(&g_sessions_lock);
		}
		HeapFree(GetProcessHeap(), 0, This->vols);
		HeapFree(GetProcessHeap(), 0, This->tmp_buffer);
		// HeapFree(GetProcessHeap(), 0, This->cap_buffer);
		HeapFree(GetProcessHeap(), 0, This->local_buffer);
		free(This->wrap_buffer);
		CoTaskMemFree(This->fmt);
		IMMDevice_Release(This->parent);
		IUnknown_Release(This->pUnkFTMarshal);
		HeapFree(GetProcessHeap(), 0, This);
	}
	return ref;
}

static void dump_fmt(const WAVEFORMATEX *fmt)
{
	TRACE("wFormatTag: 0x%x (", fmt->wFormatTag);
	switch (fmt->wFormatTag) {
	case WAVE_FORMAT_PCM:
		TRACE("WAVE_FORMAT_PCM");
		break;
	case WAVE_FORMAT_IEEE_FLOAT:
		TRACE("WAVE_FORMAT_IEEE_FLOAT");
		break;
	case WAVE_FORMAT_EXTENSIBLE:
		TRACE("WAVE_FORMAT_EXTENSIBLE");
		break;
	default:
		TRACE("Unknown");
		break;
	}
	TRACE(")\n");

	TRACE("nChannels: %u\n", fmt->nChannels);
	TRACE("nSamplesPerSec: %u\n", fmt->nSamplesPerSec);
	TRACE("nAvgBytesPerSec: %u\n", fmt->nAvgBytesPerSec);
	TRACE("nBlockAlign: %u\n", fmt->nBlockAlign);
	TRACE("wBitsPerSample: %u\n", fmt->wBitsPerSample);
	TRACE("cbSize: %u\n", fmt->cbSize);

	if (fmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		WAVEFORMATEXTENSIBLE *fmtex = (void*)fmt;
		TRACE("dwChannelMask: %08x\n", fmtex->dwChannelMask);
		TRACE("Samples: %04x\n", fmtex->Samples.wReserved);
		TRACE("SubFormat: %s\n", wine_dbgstr_guid(&fmtex->SubFormat));
	}
}

static WAVEFORMATEX *clone_format(const WAVEFORMATEX *fmt)
{
	WAVEFORMATEX *ret;
	size_t size;

	if (fmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		size = sizeof(WAVEFORMATEXTENSIBLE);
	else
		size = sizeof(WAVEFORMATEX);

	ret = CoTaskMemAlloc(size);
	if (!ret)
		return NULL;

	memcpy(ret, fmt, size);

	ret->cbSize = size - sizeof(WAVEFORMATEX);

	return ret;
}

static void session_init_vols(AudioSession *session, UINT channels)
{
	if (session->channel_count < channels) {
		UINT i;

		if (session->channel_vols)
			session->channel_vols = HeapReAlloc(GetProcessHeap(), 0,
				session->channel_vols, sizeof(float) * channels);
		else
			session->channel_vols = HeapAlloc(GetProcessHeap(), 0,
				sizeof(float) * channels);
		if (!session->channel_vols)
			return;

		for (i = session->channel_count; i < channels; ++i)
			session->channel_vols[i] = 1.f;

		session->channel_count = channels;
	}
}

static AudioSession *create_session(const GUID *guid, IMMDevice *device,
	UINT num_channels)
{
	AudioSession *ret;

	ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(AudioSession));
	if (!ret)
		return NULL;

	memcpy(&ret->guid, guid, sizeof(GUID));

	ret->device = device;

	list_init(&ret->clients);

	list_add_head(&g_sessions, &ret->entry);

	InitializeCriticalSection(&ret->lock);
	ret->lock.DebugInfo->Spare[0] = (DWORD_PTR)(__FILE__ ": AudioSession.lock");

	session_init_vols(ret, num_channels);

	ret->master_vol = 1.f;

	return ret;
}

/* if channels == 0, then this will return or create a session with
* matching dataflow and GUID. otherwise, channels must also match */
static HRESULT get_audio_session(const GUID *sessionguid,
	IMMDevice *device, UINT channels, AudioSession **out)
{
	AudioSession *session;

	if (!sessionguid || IsEqualGUID(sessionguid, &GUID_NULL)) {
		*out = create_session(&GUID_NULL, device, channels);
		if (!*out)
			return E_OUTOFMEMORY;

		return S_OK;
	}

	*out = NULL;
	LIST_FOR_EACH_ENTRY(session, &g_sessions, AudioSession, entry) {
		if (session->device == device &&
			IsEqualGUID(sessionguid, &session->guid)) {
			session_init_vols(session, channels);
			*out = session;
			break;
		}
	}

	if (!*out) {
		*out = create_session(sessionguid, device, channels);
		if (!*out)
			return E_OUTOFMEMORY;
	}

	return S_OK;
}

static void audio_wrap_buffer(BYTE *dst, UINT32 dst_offs, UINT32 dst_bytes,
	BYTE *src, UINT32 src_bytes)
{
	UINT32 chunk_bytes = dst_bytes - dst_offs;

	if (chunk_bytes < src_bytes) {
		memcpy(dst + dst_offs, src, chunk_bytes);
		memcpy(dst, src + chunk_bytes, src_bytes - chunk_bytes);
	}
	else
		memcpy(dst + dst_offs, src, src_bytes);
}

static void silence_buffer(ACImpl *This, BYTE *buffer, UINT32 frames)
{
	WAVEFORMATEXTENSIBLE *fmtex = (WAVEFORMATEXTENSIBLE*)This->fmt;
	if ((This->fmt->wFormatTag == WAVE_FORMAT_PCM ||
		(This->fmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
			IsEqualGUID(&fmtex->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM))) &&
		This->fmt->wBitsPerSample == 8)
		memset(buffer, 128, frames * This->fmt->nBlockAlign);
	else
		memset(buffer, 0, frames * This->fmt->nBlockAlign);
}

static void capture_resample(ACImpl *This) {
	CALL_NORETURN_1(BOXED_AUDIO_DRV_CAPTURE_RESAMPLE, This->boxedAudioId);
}

static HRESULT WINAPI AudioClient_Initialize(IAudioClient *iface,
	AUDCLNT_SHAREMODE mode, DWORD flags, REFERENCE_TIME duration,
	REFERENCE_TIME period, const WAVEFORMATEX *fmt,
	const GUID *sessionguid)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	HRESULT hr;
	int i;
	int result = 0;

	TRACE("(%p)->(%x, %x, %s, %s, %p, %s)\n", This, mode, flags,
		wine_dbgstr_longlong(duration), wine_dbgstr_longlong(period), fmt, debugstr_guid(sessionguid));

	if (!fmt)
		return E_POINTER;

	dump_fmt(fmt);

	if (mode != AUDCLNT_SHAREMODE_SHARED && mode != AUDCLNT_SHAREMODE_EXCLUSIVE)
		return AUDCLNT_E_NOT_INITIALIZED;

	if (flags & ~(AUDCLNT_STREAMFLAGS_CROSSPROCESS |
		AUDCLNT_STREAMFLAGS_LOOPBACK |
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
		AUDCLNT_STREAMFLAGS_NOPERSIST |
		AUDCLNT_STREAMFLAGS_RATEADJUST |
		AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED |
		AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE |
		AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED)) {
		TRACE("Unknown flags: %08x\n", flags);
		return E_INVALIDARG;
	}
        CALL_NORETURN_2(BOXED_AUDIO_DRV_GET_PERIOD, &MinimumPeriod, &DefaultPeriod);
	if (mode == AUDCLNT_SHAREMODE_SHARED) {
		period = DefaultPeriod;
		if (duration < 3 * period)
			duration = 3 * period;
	}
	else {
		if (fmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
			if (((WAVEFORMATEXTENSIBLE*)fmt)->dwChannelMask == 0 ||
				((WAVEFORMATEXTENSIBLE*)fmt)->dwChannelMask & SPEAKER_RESERVED)
				return AUDCLNT_E_UNSUPPORTED_FORMAT;
		}

		if (!period)
			period = DefaultPeriod; /* not minimum */
		if (period < MinimumPeriod || period > 5000000)
			return AUDCLNT_E_INVALID_DEVICE_PERIOD;
		if (duration > 20000000) /* the smaller the period, the lower this limit */
			return AUDCLNT_E_BUFFER_SIZE_ERROR;
		if (flags & AUDCLNT_STREAMFLAGS_EVENTCALLBACK) {
			if (duration != period)
				return AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL;
			FIXME("EXCLUSIVE mode with EVENTCALLBACK\n");
			return AUDCLNT_E_DEVICE_IN_USE;
		}
		else {
			if (duration < 8 * period)
				duration = 8 * period; /* may grow above 2s */
		}
	}

	EnterCriticalSection(&This->lock);

	if (This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_ALREADY_INITIALIZED;
	}

	This->fmt = clone_format(fmt);
	if (!This->fmt) {
		LeaveCriticalSection(&This->lock);
		return E_OUTOFMEMORY;
	}

	This->period_ms = period / 10000;
	This->period_frames = MulDiv(period, This->fmt->nSamplesPerSec, 10000000);

	This->bufsize_frames = MulDiv(duration, fmt->nSamplesPerSec, 10000000);
	if (mode == AUDCLNT_SHAREMODE_EXCLUSIVE)
		This->bufsize_frames -= This->bufsize_frames % This->period_frames;

        This->local_buffer = HeapAlloc(GetProcessHeap(), 0, This->bufsize_frames * fmt->nBlockAlign);
	CALL_9(BOXED_AUDIO_DRV_INIT, (This->dataflow == eRender ? 1 : 0), This->boxedAudioId, This->fmt, &This->period_frames, This->local_buffer, &This->wri_offs_frames, &This->held_frames, &This->lcl_offs_frames, This->bufsize_frames);
	hr = result;
	if (FAILED(hr)) {
		CoTaskMemFree(This->fmt);
		This->fmt = NULL;
		LeaveCriticalSection(&This->lock);
		return hr;
	}

	silence_buffer(This, This->local_buffer, This->bufsize_frames);

//	if (This->dataflow == eCapture) {
//		This->cap_bufsize_frames = MulDiv(duration, This->dev_desc.mSampleRate, 10000000);
//		This->cap_buffer = HeapAlloc(GetProcessHeap(), 0, This->cap_bufsize_frames * This->fmt->nBlockAlign);
//	}

	This->vols = HeapAlloc(GetProcessHeap(), 0, fmt->nChannels * sizeof(float));
	if (!This->vols) {
		CoTaskMemFree(This->fmt);
		This->fmt = NULL;
		LeaveCriticalSection(&This->lock);
		return E_OUTOFMEMORY;
	}

	for (i = 0; i < fmt->nChannels; ++i)
		This->vols[i] = 1.f;

	This->share = mode;
	This->flags = flags;

	EnterCriticalSection(&g_sessions_lock);

	hr = get_audio_session(sessionguid, This->parent, fmt->nChannels,
		&This->session);
	if (FAILED(hr)) {
		LeaveCriticalSection(&g_sessions_lock);
		CoTaskMemFree(This->fmt);
		This->fmt = NULL;
		HeapFree(GetProcessHeap(), 0, This->vols);
		This->vols = NULL;
		LeaveCriticalSection(&This->lock);
		return E_INVALIDARG;
	}

	list_add_tail(&This->session->clients, &This->entry);

	LeaveCriticalSection(&g_sessions_lock);

	audio_setvol(This, -1);

	This->initted = TRUE;

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT WINAPI AudioClient_GetBufferSize(IAudioClient *iface,
	UINT32 *frames)
{
	ACImpl *This = impl_from_IAudioClient(iface);

	TRACE("(%p)->(%p)\n", This, frames);

	if (!frames)
		return E_POINTER;

	EnterCriticalSection(&This->lock);

	if (!This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_INITIALIZED;
	}

	*frames = This->bufsize_frames;

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT WINAPI AudioClient_GetStreamLatency(IAudioClient *iface,
	REFERENCE_TIME *out)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	UINT32 latency;
	HRESULT hr;
	int result = 0;

	TRACE("(%p)->(%p)\n", This, out);

	if (!out)
		return E_POINTER;

	EnterCriticalSection(&This->lock);

	if (!This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_INITIALIZED;
	}

	CALL_2(BOXED_AUDIO_DRV_GET_LATENCY, This->boxedAudioId, &latency);
	hr = result;
	if (FAILED(hr)) {
		LeaveCriticalSection(&This->lock);
		return hr;
	}

	/* pretend we process audio in Period chunks, so max latency includes
	* the period time */
	*out = MulDiv(latency, 10000000, This->fmt->nSamplesPerSec)
		+ This->period_ms * 10000;

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT AudioClient_GetCurrentPadding_nolock(ACImpl *This,
	UINT32 *numpad)
{
	if (!This->initted)
		return AUDCLNT_E_NOT_INITIALIZED;

	if (This->dataflow == eCapture)
		capture_resample(This);

	*numpad = This->held_frames;

	return S_OK;
}

static HRESULT WINAPI AudioClient_GetCurrentPadding(IAudioClient *iface,
	UINT32 *numpad)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	HRESULT hr;

	TRACE("(%p)->(%p)\n", This, numpad);

	if (!numpad)
		return E_POINTER;

	EnterCriticalSection(&This->lock);
	
	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);
	hr = AudioClient_GetCurrentPadding_nolock(This, numpad);
	CALL_NORETURN_1(BOXED_AUDIO_DRV_UNLOCK, This->boxedAudioId);

	LeaveCriticalSection(&This->lock);

	return hr;
}

static HRESULT WINAPI AudioClient_IsFormatSupported(IAudioClient *iface,
	AUDCLNT_SHAREMODE mode, const WAVEFORMATEX *pwfx,
	WAVEFORMATEX **outpwfx)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	WAVEFORMATEXTENSIBLE *fmtex = (WAVEFORMATEXTENSIBLE*)pwfx;
	HRESULT hr;
	int result = 0;

	TRACE("(%p)->(%x, %p, %p)\n", This, mode, pwfx, outpwfx);

	if (!pwfx || (mode == AUDCLNT_SHAREMODE_SHARED && !outpwfx))
		return E_POINTER;

	if (mode != AUDCLNT_SHAREMODE_SHARED && mode != AUDCLNT_SHAREMODE_EXCLUSIVE)
		return E_INVALIDARG;

	if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
		pwfx->cbSize < sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
		return E_INVALIDARG;

	dump_fmt(pwfx);

	if (outpwfx) {
		*outpwfx = NULL;
		if (mode != AUDCLNT_SHAREMODE_SHARED)
			outpwfx = NULL;
	}

	if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		if (pwfx->nAvgBytesPerSec == 0 ||
			pwfx->nBlockAlign == 0 ||
			fmtex->Samples.wValidBitsPerSample > pwfx->wBitsPerSample)
			return E_INVALIDARG;
		if (fmtex->Samples.wValidBitsPerSample < pwfx->wBitsPerSample)
			goto unsupported;
		if (mode == AUDCLNT_SHAREMODE_EXCLUSIVE) {
			if (fmtex->dwChannelMask == 0 ||
				fmtex->dwChannelMask & SPEAKER_RESERVED)
				goto unsupported;
		}
	}

	if (pwfx->nBlockAlign != pwfx->nChannels * pwfx->wBitsPerSample / 8 ||
		pwfx->nAvgBytesPerSec != pwfx->nBlockAlign * pwfx->nSamplesPerSec)
		goto unsupported;

	if (pwfx->nChannels == 0)
		return AUDCLNT_E_UNSUPPORTED_FORMAT;

	CALL_2(BOXED_AUDIO_DRV_IS_FORMAT_SUPPORTED, This->boxedAudioId, pwfx);
	hr = result;
	if (FAILED(hr))
		goto unsupported;

	return S_OK;

unsupported:
	if (outpwfx) {
		hr = IAudioClient_GetMixFormat(&This->IAudioClient_iface, outpwfx);
		if (FAILED(hr))
			return hr;
		return S_FALSE;
	}

	return AUDCLNT_E_UNSUPPORTED_FORMAT;
}

static HRESULT WINAPI AudioClient_GetMixFormat(IAudioClient *iface,
	WAVEFORMATEX **pwfx)
{
	ACImpl *This = impl_from_IAudioClient(iface);
	int result = 0;
        WAVEFORMATEXTENSIBLE *fmt;

	TRACE("(%p)->(%p)\n", This, pwfx);

	if (!pwfx)
		return E_POINTER;
	*pwfx = NULL;

        fmt = CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE));
        if(!fmt)
            return E_OUTOFMEMORY;

	CALL_2(BOXED_AUDIO_DRV_GET_MIX_FORMAT, This->boxedAudioId, fmt);
        if (FAILED(result)) {
            CoTaskMemFree(fmt);
        } else {
            *pwfx = (WAVEFORMATEX*)fmt;
        }
        return result;
}

static HRESULT WINAPI AudioClient_GetDevicePeriod(IAudioClient *iface,
	REFERENCE_TIME *defperiod, REFERENCE_TIME *minperiod)
{
	ACImpl *This = impl_from_IAudioClient(iface);

	TRACE("(%p)->(%p, %p)\n", This, defperiod, minperiod);

	if (!defperiod && !minperiod)
		return E_POINTER;

	if (defperiod)
		*defperiod = DefaultPeriod;
	if (minperiod)
		*minperiod = MinimumPeriod;

	return S_OK;
}

void CALLBACK ca_period_cb(void *user, BOOLEAN timer)
{
	ACImpl *This = user;

	if (This->event)
		SetEvent(This->event);
}

static HRESULT WINAPI AudioClient_Start(IAudioClient *iface)
{
	ACImpl *This = impl_from_IAudioClient(iface);
        int result = 0;

	TRACE("(%p)\n", This);

	EnterCriticalSection(&This->lock);

	if (!This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_INITIALIZED;
	}

	if (This->playing) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_STOPPED;
	}

	if ((This->flags & AUDCLNT_STREAMFLAGS_EVENTCALLBACK) && !This->event) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_EVENTHANDLE_NOT_SET;
	}
        CALL_0(BOXED_AUDIO_DRV_USE_TIMER);
        if (!result) {
            if (!thread) {
                pipe( pipeFd );
                thread = CreateThread(0, 0, msg_thread, NULL, 0, 0);
                TRACE("created msg thread: %d\n", (int)thread);
            }
        } else {
            if (!This->timer) {
                TRACE("Creater callback timer\n");
		if (!CreateTimerQueueTimer(&This->timer, g_timer_q, ca_period_cb,
			This, 0, This->period_ms, WT_EXECUTEINTIMERTHREAD)) {
			This->timer = NULL;
			LeaveCriticalSection(&This->lock);
			WARN("Unable to create timer: %u\n", GetLastError());
			return E_OUTOFMEMORY;
                }
            }
        }
	This->playing = TRUE;

	LeaveCriticalSection(&This->lock);
        CALL_NORETURN_2(BOXED_AUDIO_DRV_START, This->boxedAudioId, pipeFd[1]);
	return S_OK;
}

static HRESULT WINAPI AudioClient_Stop(IAudioClient *iface)
{
	ACImpl *This = impl_from_IAudioClient(iface);

	TRACE("(%p)\n", This);

	EnterCriticalSection(&This->lock);

	if (!This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_INITIALIZED;
	}

	if (!This->playing) {
		LeaveCriticalSection(&This->lock);
		return S_FALSE;
	}

	This->playing = FALSE;

	LeaveCriticalSection(&This->lock);
        CALL_NORETURN_1(BOXED_AUDIO_DRV_STOP, This->boxedAudioId);
	return S_OK;
}

static HRESULT WINAPI AudioClient_Reset(IAudioClient *iface)
{
	ACImpl *This = impl_from_IAudioClient(iface);

	TRACE("(%p)\n", This);

	EnterCriticalSection(&This->lock);

	if (!This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_INITIALIZED;
	}

	if (This->playing) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_STOPPED;
	}

	if (This->getbuf_last) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_BUFFER_OPERATION_PENDING;
	}

	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);
	if (This->dataflow == eRender) {
		This->written_frames = 0;
	}
	else {
		This->written_frames += This->held_frames;
	}

	This->held_frames = 0;
	This->lcl_offs_frames = 0;
	This->wri_offs_frames = 0;

	CALL_NORETURN_1(BOXED_AUDIO_DRV_UNLOCK, This->boxedAudioId);
	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT WINAPI AudioClient_SetEventHandle(IAudioClient *iface,
	HANDLE event)
{
	ACImpl *This = impl_from_IAudioClient(iface);

	TRACE("(%p)->(%p)\n", This, event);

	if (!event)
		return E_INVALIDARG;

	EnterCriticalSection(&This->lock);

	if (!This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_INITIALIZED;
	}

	if (!(This->flags & AUDCLNT_STREAMFLAGS_EVENTCALLBACK)) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED;
	}

	if (This->event) {
		LeaveCriticalSection(&This->lock);
		FIXME("called twice\n");
		return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
	}

	This->event = event;
        dsoundEvent = event;

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT WINAPI AudioClient_GetService(IAudioClient *iface, REFIID riid,
	void **ppv)
{
	ACImpl *This = impl_from_IAudioClient(iface);

	TRACE("(%p)->(%s, %p)\n", This, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	EnterCriticalSection(&This->lock);

	if (!This->initted) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_NOT_INITIALIZED;
	}

	if (IsEqualIID(riid, &IID_IAudioRenderClient)) {
		if (This->dataflow != eRender) {
			LeaveCriticalSection(&This->lock);
			return AUDCLNT_E_WRONG_ENDPOINT_TYPE;
		}
		IAudioRenderClient_AddRef(&This->IAudioRenderClient_iface);
		*ppv = &This->IAudioRenderClient_iface;
	}
	else if (IsEqualIID(riid, &IID_IAudioCaptureClient)) {
		if (This->dataflow != eCapture) {
			LeaveCriticalSection(&This->lock);
			return AUDCLNT_E_WRONG_ENDPOINT_TYPE;
		}
		IAudioCaptureClient_AddRef(&This->IAudioCaptureClient_iface);
		*ppv = &This->IAudioCaptureClient_iface;
	}
	else if (IsEqualIID(riid, &IID_IAudioClock)) {
		IAudioClock_AddRef(&This->IAudioClock_iface);
		*ppv = &This->IAudioClock_iface;
	}
	else if (IsEqualIID(riid, &IID_IAudioStreamVolume)) {
		IAudioStreamVolume_AddRef(&This->IAudioStreamVolume_iface);
		*ppv = &This->IAudioStreamVolume_iface;
	}
	else if (IsEqualIID(riid, &IID_IAudioSessionControl)) {
		if (!This->session_wrapper) {
			This->session_wrapper = AudioSessionWrapper_Create(This);
			if (!This->session_wrapper) {
				LeaveCriticalSection(&This->lock);
				return E_OUTOFMEMORY;
			}
		}
		else
			IAudioSessionControl2_AddRef(&This->session_wrapper->IAudioSessionControl2_iface);

		*ppv = &This->session_wrapper->IAudioSessionControl2_iface;
	}
	else if (IsEqualIID(riid, &IID_IChannelAudioVolume)) {
		if (!This->session_wrapper) {
			This->session_wrapper = AudioSessionWrapper_Create(This);
			if (!This->session_wrapper) {
				LeaveCriticalSection(&This->lock);
				return E_OUTOFMEMORY;
			}
		}
		else
			IChannelAudioVolume_AddRef(&This->session_wrapper->IChannelAudioVolume_iface);

		*ppv = &This->session_wrapper->IChannelAudioVolume_iface;
	}
	else if (IsEqualIID(riid, &IID_ISimpleAudioVolume)) {
		if (!This->session_wrapper) {
			This->session_wrapper = AudioSessionWrapper_Create(This);
			if (!This->session_wrapper) {
				LeaveCriticalSection(&This->lock);
				return E_OUTOFMEMORY;
			}
		}
		else
			ISimpleAudioVolume_AddRef(&This->session_wrapper->ISimpleAudioVolume_iface);

		*ppv = &This->session_wrapper->ISimpleAudioVolume_iface;
	}

	if (*ppv) {
		LeaveCriticalSection(&This->lock);
		return S_OK;
	}

	LeaveCriticalSection(&This->lock);

	FIXME("stub %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static const IAudioClientVtbl AudioClient_Vtbl =
{
	AudioClient_QueryInterface,
	AudioClient_AddRef,
	AudioClient_Release,
	AudioClient_Initialize,
	AudioClient_GetBufferSize,
	AudioClient_GetStreamLatency,
	AudioClient_GetCurrentPadding,
	AudioClient_IsFormatSupported,
	AudioClient_GetMixFormat,
	AudioClient_GetDevicePeriod,
	AudioClient_Start,
	AudioClient_Stop,
	AudioClient_Reset,
	AudioClient_SetEventHandle,
	AudioClient_GetService
};

static HRESULT WINAPI AudioRenderClient_QueryInterface(
	IAudioRenderClient *iface, REFIID riid, void **ppv)
{
	ACImpl *This = impl_from_IAudioRenderClient(iface);
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IAudioRenderClient))
		*ppv = iface;
	else if (IsEqualIID(riid, &IID_IMarshal))
		return IUnknown_QueryInterface(This->pUnkFTMarshal, riid, ppv);

	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI AudioRenderClient_AddRef(IAudioRenderClient *iface)
{
	ACImpl *This = impl_from_IAudioRenderClient(iface);
	return AudioClient_AddRef(&This->IAudioClient_iface);
}

static ULONG WINAPI AudioRenderClient_Release(IAudioRenderClient *iface)
{
	ACImpl *This = impl_from_IAudioRenderClient(iface);
	return AudioClient_Release(&This->IAudioClient_iface);
}

static HRESULT WINAPI AudioRenderClient_GetBuffer(IAudioRenderClient *iface,
	UINT32 frames, BYTE **data)
{
	ACImpl *This = impl_from_IAudioRenderClient(iface);
	UINT32 pad;
	HRESULT hr;
        int priority = GetThreadPriority(GetCurrentThread());

	TRACE("(%p)->(%u, %p)\n", This, frames, data);

        if (priority != This->lastSetPriority) {
            CALL_NORETURN_1(BOXED_AUDIO_DRV_SET_PRIORITY, priority);
            This->lastSetPriority = priority;
        }        
	if (!data)
		return E_POINTER;
	*data = NULL;

	EnterCriticalSection(&This->lock);

	if (This->getbuf_last) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_OUT_OF_ORDER;
	}

	if (!frames) {
		LeaveCriticalSection(&This->lock);
		return S_OK;
	}

	hr = AudioClient_GetCurrentPadding_nolock(This, &pad);
	if (FAILED(hr)) {
		LeaveCriticalSection(&This->lock);
		return hr;
	}

	if (pad + frames > This->bufsize_frames) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_BUFFER_TOO_LARGE;
	}

	if (This->wri_offs_frames + frames > This->bufsize_frames) {
		if (This->tmp_buffer_frames < frames) {
			HeapFree(GetProcessHeap(), 0, This->tmp_buffer);
			This->tmp_buffer = HeapAlloc(GetProcessHeap(), 0, frames * This->fmt->nBlockAlign);
			if (!This->tmp_buffer) {
				LeaveCriticalSection(&This->lock);
				return E_OUTOFMEMORY;
			}
			This->tmp_buffer_frames = frames;
		}
		*data = This->tmp_buffer;
		This->getbuf_last = -frames;
	}
	else {
		*data = This->local_buffer + This->wri_offs_frames * This->fmt->nBlockAlign;
		This->getbuf_last = frames;
	}

	silence_buffer(This, *data, frames);

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT WINAPI AudioRenderClient_ReleaseBuffer(
	IAudioRenderClient *iface, UINT32 frames, DWORD flags)
{
	ACImpl *This = impl_from_IAudioRenderClient(iface);
	BYTE *buffer;

	TRACE("(%p)->(%u, %x)\n", This, frames, flags);

	EnterCriticalSection(&This->lock);

	if (!frames) {
		This->getbuf_last = 0;
		LeaveCriticalSection(&This->lock);
		return S_OK;
	}

	if (!This->getbuf_last) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_OUT_OF_ORDER;
	}

	if (frames > (This->getbuf_last >= 0 ? This->getbuf_last : -This->getbuf_last)) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_INVALID_SIZE;
	}

	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);

	if (This->getbuf_last >= 0)
		buffer = This->local_buffer + This->wri_offs_frames * This->fmt->nBlockAlign;
	else
		buffer = This->tmp_buffer;

	if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
		silence_buffer(This, buffer, frames);

	if (This->getbuf_last < 0)
		audio_wrap_buffer(This->local_buffer,
			This->wri_offs_frames * This->fmt->nBlockAlign,
			This->bufsize_frames * This->fmt->nBlockAlign,
			buffer, frames * This->fmt->nBlockAlign);


	This->wri_offs_frames += frames;
	This->wri_offs_frames %= This->bufsize_frames;
	This->held_frames += frames;
	This->written_frames += frames;
	This->getbuf_last = 0;

	CALL_NORETURN_1(BOXED_AUDIO_DRV_UNLOCK, This->boxedAudioId);
	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static const IAudioRenderClientVtbl AudioRenderClient_Vtbl = {
	AudioRenderClient_QueryInterface,
	AudioRenderClient_AddRef,
	AudioRenderClient_Release,
	AudioRenderClient_GetBuffer,
	AudioRenderClient_ReleaseBuffer
};

static HRESULT WINAPI AudioCaptureClient_QueryInterface(
	IAudioCaptureClient *iface, REFIID riid, void **ppv)
{
	ACImpl *This = impl_from_IAudioCaptureClient(iface);
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IAudioCaptureClient))
		*ppv = iface;
	else if (IsEqualIID(riid, &IID_IMarshal))
		return IUnknown_QueryInterface(This->pUnkFTMarshal, riid, ppv);

	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI AudioCaptureClient_AddRef(IAudioCaptureClient *iface)
{
	ACImpl *This = impl_from_IAudioCaptureClient(iface);
	return IAudioClient_AddRef(&This->IAudioClient_iface);
}

static ULONG WINAPI AudioCaptureClient_Release(IAudioCaptureClient *iface)
{
	ACImpl *This = impl_from_IAudioCaptureClient(iface);
	return IAudioClient_Release(&This->IAudioClient_iface);
}

static HRESULT WINAPI AudioCaptureClient_GetBuffer(IAudioCaptureClient *iface,
	BYTE **data, UINT32 *frames, DWORD *flags, UINT64 *devpos,
	UINT64 *qpcpos)
{
	ACImpl *This = impl_from_IAudioCaptureClient(iface);
	UINT32 chunk_bytes, chunk_frames;

	TRACE("(%p)->(%p, %p, %p, %p, %p)\n", This, data, frames, flags,
		devpos, qpcpos);

	if (!data || !frames || !flags)
		return E_POINTER;

	EnterCriticalSection(&This->lock);
	
	if (This->getbuf_last) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_OUT_OF_ORDER;
	}
	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);
	capture_resample(This);

	if (This->held_frames < This->period_frames) {
		*frames = 0;
		CALL_NORETURN_1(BOXED_AUDIO_DRV_UNLOCK, This->boxedAudioId);
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_S_BUFFER_EMPTY;
	}

	*flags = 0;

	chunk_frames = This->bufsize_frames - This->lcl_offs_frames;
	if (chunk_frames < This->period_frames) {
		chunk_bytes = chunk_frames * This->fmt->nBlockAlign;
		if (!This->tmp_buffer)
			This->tmp_buffer = HeapAlloc(GetProcessHeap(), 0, This->period_frames * This->fmt->nBlockAlign);
		*data = This->tmp_buffer;
		memcpy(*data, This->local_buffer + This->lcl_offs_frames * This->fmt->nBlockAlign, chunk_bytes);
		memcpy((*data) + chunk_bytes, This->local_buffer, This->period_frames * This->fmt->nBlockAlign - chunk_bytes);
	}
	else
		*data = This->local_buffer + This->lcl_offs_frames * This->fmt->nBlockAlign;

	This->getbuf_last = *frames = This->period_frames;

	if (devpos)
		*devpos = This->written_frames;
	if (qpcpos) { /* fixme: qpc of recording time */
		LARGE_INTEGER stamp, freq;
		QueryPerformanceCounter(&stamp);
		QueryPerformanceFrequency(&freq);
		*qpcpos = (stamp.QuadPart * (INT64)10000000) / freq.QuadPart;
	}
	CALL_NORETURN_1(BOXED_AUDIO_DRV_UNLOCK, This->boxedAudioId);
	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT WINAPI AudioCaptureClient_ReleaseBuffer(
	IAudioCaptureClient *iface, UINT32 done)
{
	ACImpl *This = impl_from_IAudioCaptureClient(iface);

	TRACE("(%p)->(%u)\n", This, done);

	EnterCriticalSection(&This->lock);

	if (!done) {
		This->getbuf_last = 0;
		LeaveCriticalSection(&This->lock);
		return S_OK;
	}

	if (!This->getbuf_last) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_OUT_OF_ORDER;
	}

	if (This->getbuf_last != done) {
		LeaveCriticalSection(&This->lock);
		return AUDCLNT_E_INVALID_SIZE;
	}

	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);
	This->written_frames += done;
	This->held_frames -= done;
	This->lcl_offs_frames += done;
	This->lcl_offs_frames %= This->bufsize_frames;
	This->getbuf_last = 0;
	CALL_NORETURN_1(BOXED_AUDIO_DRV_UNLOCK, This->boxedAudioId);

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static HRESULT WINAPI AudioCaptureClient_GetNextPacketSize(
	IAudioCaptureClient *iface, UINT32 *frames)
{
	ACImpl *This = impl_from_IAudioCaptureClient(iface);

	TRACE("(%p)->(%p)\n", This, frames);

	if (!frames)
		return E_POINTER;

	EnterCriticalSection(&This->lock);

	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);
	capture_resample(This);

	if (This->held_frames >= This->period_frames)
		*frames = This->period_frames;
	else
		*frames = 0;
	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static const IAudioCaptureClientVtbl AudioCaptureClient_Vtbl =
{
	AudioCaptureClient_QueryInterface,
	AudioCaptureClient_AddRef,
	AudioCaptureClient_Release,
	AudioCaptureClient_GetBuffer,
	AudioCaptureClient_ReleaseBuffer,
	AudioCaptureClient_GetNextPacketSize
};

static HRESULT WINAPI AudioClock_QueryInterface(IAudioClock *iface,
	REFIID riid, void **ppv)
{
	ACImpl *This = impl_from_IAudioClock(iface);

	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IAudioClock))
		*ppv = iface;
	else if (IsEqualIID(riid, &IID_IAudioClock2))
		*ppv = &This->IAudioClock2_iface;
	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI AudioClock_AddRef(IAudioClock *iface)
{
	ACImpl *This = impl_from_IAudioClock(iface);
	return IAudioClient_AddRef(&This->IAudioClient_iface);
}

static ULONG WINAPI AudioClock_Release(IAudioClock *iface)
{
	ACImpl *This = impl_from_IAudioClock(iface);
	return IAudioClient_Release(&This->IAudioClient_iface);
}

static HRESULT WINAPI AudioClock_GetFrequency(IAudioClock *iface, UINT64 *freq)
{
	ACImpl *This = impl_from_IAudioClock(iface);

	TRACE("(%p)->(%p)\n", This, freq);

	if (This->share == AUDCLNT_SHAREMODE_SHARED)
		*freq = (UINT64)This->fmt->nSamplesPerSec * This->fmt->nBlockAlign;
	else
		*freq = This->fmt->nSamplesPerSec;

	return S_OK;
}

static HRESULT AudioClock_GetPosition_nolock(ACImpl *This,
	UINT64 *pos, UINT64 *qpctime)
{
	*pos = This->written_frames - This->held_frames;

	if (This->share == AUDCLNT_SHAREMODE_SHARED)
		*pos *= This->fmt->nBlockAlign;

	if (qpctime) {
		LARGE_INTEGER stamp, freq;
		QueryPerformanceCounter(&stamp);
		QueryPerformanceFrequency(&freq);
		*qpctime = (stamp.QuadPart * (INT64)10000000) / freq.QuadPart;
	}

	return S_OK;
}

static HRESULT WINAPI AudioClock_GetPosition(IAudioClock *iface, UINT64 *pos,
	UINT64 *qpctime)
{
	ACImpl *This = impl_from_IAudioClock(iface);
	HRESULT hr;

	TRACE("(%p)->(%p, %p)\n", This, pos, qpctime);

	if (!pos)
		return E_POINTER;

	EnterCriticalSection(&This->lock);
	CALL_NORETURN_1(BOXED_AUDIO_DRV_LOCK, This->boxedAudioId);

	hr = AudioClock_GetPosition_nolock(This, pos, qpctime);

	CALL_NORETURN_1(BOXED_AUDIO_DRV_UNLOCK, This->boxedAudioId);
	LeaveCriticalSection(&This->lock);

	return hr;
}

static HRESULT WINAPI AudioClock_GetCharacteristics(IAudioClock *iface,
	DWORD *chars)
{
	ACImpl *This = impl_from_IAudioClock(iface);

	TRACE("(%p)->(%p)\n", This, chars);

	if (!chars)
		return E_POINTER;

	*chars = AUDIOCLOCK_CHARACTERISTIC_FIXED_FREQ;

	return S_OK;
}

static const IAudioClockVtbl AudioClock_Vtbl =
{
	AudioClock_QueryInterface,
	AudioClock_AddRef,
	AudioClock_Release,
	AudioClock_GetFrequency,
	AudioClock_GetPosition,
	AudioClock_GetCharacteristics
};

static HRESULT WINAPI AudioClock2_QueryInterface(IAudioClock2 *iface,
	REFIID riid, void **ppv)
{
	ACImpl *This = impl_from_IAudioClock2(iface);
	return IAudioClock_QueryInterface(&This->IAudioClock_iface, riid, ppv);
}

static ULONG WINAPI AudioClock2_AddRef(IAudioClock2 *iface)
{
	ACImpl *This = impl_from_IAudioClock2(iface);
	return IAudioClient_AddRef(&This->IAudioClient_iface);
}

static ULONG WINAPI AudioClock2_Release(IAudioClock2 *iface)
{
	ACImpl *This = impl_from_IAudioClock2(iface);
	return IAudioClient_Release(&This->IAudioClient_iface);
}

static HRESULT WINAPI AudioClock2_GetDevicePosition(IAudioClock2 *iface,
	UINT64 *pos, UINT64 *qpctime)
{
	ACImpl *This = impl_from_IAudioClock2(iface);

	FIXME("(%p)->(%p, %p)\n", This, pos, qpctime);

	return E_NOTIMPL;
}

static const IAudioClock2Vtbl AudioClock2_Vtbl =
{
	AudioClock2_QueryInterface,
	AudioClock2_AddRef,
	AudioClock2_Release,
	AudioClock2_GetDevicePosition
};

static AudioSessionWrapper *AudioSessionWrapper_Create(ACImpl *client)
{
	AudioSessionWrapper *ret;

	ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		sizeof(AudioSessionWrapper));
	if (!ret)
		return NULL;

	ret->IAudioSessionControl2_iface.lpVtbl = &AudioSessionControl2_Vtbl;
	ret->ISimpleAudioVolume_iface.lpVtbl = &SimpleAudioVolume_Vtbl;
	ret->IChannelAudioVolume_iface.lpVtbl = &ChannelAudioVolume_Vtbl;

	ret->ref = 1;

	ret->client = client;
	if (client) {
		ret->session = client->session;
		AudioClient_AddRef(&client->IAudioClient_iface);
	}

	return ret;
}

static HRESULT WINAPI AudioSessionControl_QueryInterface(
	IAudioSessionControl2 *iface, REFIID riid, void **ppv)
{
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IAudioSessionControl) ||
		IsEqualIID(riid, &IID_IAudioSessionControl2))
		*ppv = iface;
	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI AudioSessionControl_AddRef(IAudioSessionControl2 *iface)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);
	ULONG ref;
	ref = InterlockedIncrement(&This->ref);
	TRACE("(%p) Refcount now %u\n", This, ref);
	return ref;
}

static ULONG WINAPI AudioSessionControl_Release(IAudioSessionControl2 *iface)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);
	ULONG ref;
	ref = InterlockedDecrement(&This->ref);
	TRACE("(%p) Refcount now %u\n", This, ref);
	if (!ref) {
		if (This->client) {
			EnterCriticalSection(&This->client->lock);
			This->client->session_wrapper = NULL;
			LeaveCriticalSection(&This->client->lock);
			AudioClient_Release(&This->client->IAudioClient_iface);
		}
		HeapFree(GetProcessHeap(), 0, This);
	}
	return ref;
}

static HRESULT WINAPI AudioSessionControl_GetState(IAudioSessionControl2 *iface,
	AudioSessionState *state)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);
	ACImpl *client;

	TRACE("(%p)->(%p)\n", This, state);

	if (!state)
		return NULL_PTR_ERR;

	EnterCriticalSection(&g_sessions_lock);

	if (list_empty(&This->session->clients)) {
		*state = AudioSessionStateExpired;
		LeaveCriticalSection(&g_sessions_lock);
		return S_OK;
	}

	LIST_FOR_EACH_ENTRY(client, &This->session->clients, ACImpl, entry) {
		EnterCriticalSection(&client->lock);
		if (client->playing) {
			*state = AudioSessionStateActive;
			LeaveCriticalSection(&client->lock);
			LeaveCriticalSection(&g_sessions_lock);
			return S_OK;
		}
		LeaveCriticalSection(&client->lock);
	}

	LeaveCriticalSection(&g_sessions_lock);

	*state = AudioSessionStateInactive;

	return S_OK;
}

static HRESULT WINAPI AudioSessionControl_GetDisplayName(
	IAudioSessionControl2 *iface, WCHAR **name)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p) - stub\n", This, name);

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_SetDisplayName(
	IAudioSessionControl2 *iface, const WCHAR *name, const GUID *session)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p, %s) - stub\n", This, name, debugstr_guid(session));

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_GetIconPath(
	IAudioSessionControl2 *iface, WCHAR **path)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p) - stub\n", This, path);

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_SetIconPath(
	IAudioSessionControl2 *iface, const WCHAR *path, const GUID *session)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p, %s) - stub\n", This, path, debugstr_guid(session));

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_GetGroupingParam(
	IAudioSessionControl2 *iface, GUID *group)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p) - stub\n", This, group);

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_SetGroupingParam(
	IAudioSessionControl2 *iface, const GUID *group, const GUID *session)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%s, %s) - stub\n", This, debugstr_guid(group),
		debugstr_guid(session));

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_RegisterAudioSessionNotification(
	IAudioSessionControl2 *iface, IAudioSessionEvents *events)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p) - stub\n", This, events);

	return S_OK;
}

static HRESULT WINAPI AudioSessionControl_UnregisterAudioSessionNotification(
	IAudioSessionControl2 *iface, IAudioSessionEvents *events)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p) - stub\n", This, events);

	return S_OK;
}

static HRESULT WINAPI AudioSessionControl_GetSessionIdentifier(
	IAudioSessionControl2 *iface, WCHAR **id)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p) - stub\n", This, id);

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_GetSessionInstanceIdentifier(
	IAudioSessionControl2 *iface, WCHAR **id)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	FIXME("(%p)->(%p) - stub\n", This, id);

	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionControl_GetProcessId(
	IAudioSessionControl2 *iface, DWORD *pid)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	TRACE("(%p)->(%p)\n", This, pid);

	if (!pid)
		return E_POINTER;

	*pid = GetCurrentProcessId();

	return S_OK;
}

static HRESULT WINAPI AudioSessionControl_IsSystemSoundsSession(
	IAudioSessionControl2 *iface)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	TRACE("(%p)\n", This);

	return S_FALSE;
}

static HRESULT WINAPI AudioSessionControl_SetDuckingPreference(
	IAudioSessionControl2 *iface, BOOL optout)
{
	AudioSessionWrapper *This = impl_from_IAudioSessionControl2(iface);

	TRACE("(%p)->(%d)\n", This, optout);

	return S_OK;
}

static const IAudioSessionControl2Vtbl AudioSessionControl2_Vtbl =
{
	AudioSessionControl_QueryInterface,
	AudioSessionControl_AddRef,
	AudioSessionControl_Release,
	AudioSessionControl_GetState,
	AudioSessionControl_GetDisplayName,
	AudioSessionControl_SetDisplayName,
	AudioSessionControl_GetIconPath,
	AudioSessionControl_SetIconPath,
	AudioSessionControl_GetGroupingParam,
	AudioSessionControl_SetGroupingParam,
	AudioSessionControl_RegisterAudioSessionNotification,
	AudioSessionControl_UnregisterAudioSessionNotification,
	AudioSessionControl_GetSessionIdentifier,
	AudioSessionControl_GetSessionInstanceIdentifier,
	AudioSessionControl_GetProcessId,
	AudioSessionControl_IsSystemSoundsSession,
	AudioSessionControl_SetDuckingPreference
};

/* index == -1 means set all channels, otherwise sets only the given channel */
static HRESULT audio_setvol(ACImpl *This, UINT32 index)
{
	Float32 level;
        struct int2Float f;

	if (This->session->mute) {
		level = 0.;
	} else {
		if (index == (UINT32)-1) {
			UINT32 i;
			level = 1.;
			for (i = 0; i < This->fmt->nChannels; ++i) {
				Float32 tmp;
				tmp = This->session->master_vol *
					This->session->channel_vols[i] * This->vols[i];
				level = tmp < level ? tmp : level;
			}
		} else {
			level = This->session->master_vol * This->session->channel_vols[index] * This->vols[index];
		}
	}

        f.f = level;
	CALL_NORETURN_3(BOXED_AUDIO_DRV_SET_VOLUME, This->boxedAudioId, f.i, index);

	return S_OK;
}

static HRESULT audio_session_setvol(AudioSession *session, UINT32 index)
{
	HRESULT ret = S_OK;
	ACImpl *client;

	LIST_FOR_EACH_ENTRY(client, &session->clients, ACImpl, entry) {
		HRESULT hr;
		hr = audio_setvol(client, index);
		if (FAILED(hr))
			ret = hr;
	}

	return ret;
}

static HRESULT WINAPI SimpleAudioVolume_QueryInterface(
	ISimpleAudioVolume *iface, REFIID riid, void **ppv)
{
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_ISimpleAudioVolume))
		*ppv = iface;
	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI SimpleAudioVolume_AddRef(ISimpleAudioVolume *iface)
{
	AudioSessionWrapper *This = impl_from_ISimpleAudioVolume(iface);
	return AudioSessionControl_AddRef(&This->IAudioSessionControl2_iface);
}

static ULONG WINAPI SimpleAudioVolume_Release(ISimpleAudioVolume *iface)
{
	AudioSessionWrapper *This = impl_from_ISimpleAudioVolume(iface);
	return AudioSessionControl_Release(&This->IAudioSessionControl2_iface);
}

static HRESULT WINAPI SimpleAudioVolume_SetMasterVolume(
	ISimpleAudioVolume *iface, float level, const GUID *context)
{
	AudioSessionWrapper *This = impl_from_ISimpleAudioVolume(iface);
	AudioSession *session = This->session;
	HRESULT ret;

	TRACE("(%p)->(%f, %s)\n", session, level, wine_dbgstr_guid(context));

	if (level < 0.f || level > 1.f)
		return E_INVALIDARG;

	if (context)
		FIXME("Notifications not supported yet\n");

	EnterCriticalSection(&session->lock);

	session->master_vol = level;

	ret = audio_session_setvol(session, -1);

	LeaveCriticalSection(&session->lock);

	return ret;
}

static HRESULT WINAPI SimpleAudioVolume_GetMasterVolume(
	ISimpleAudioVolume *iface, float *level)
{
	AudioSessionWrapper *This = impl_from_ISimpleAudioVolume(iface);
	AudioSession *session = This->session;

	TRACE("(%p)->(%p)\n", session, level);

	if (!level)
		return NULL_PTR_ERR;

	*level = session->master_vol;

	return S_OK;
}

static HRESULT WINAPI SimpleAudioVolume_SetMute(ISimpleAudioVolume *iface,
	BOOL mute, const GUID *context)
{
	AudioSessionWrapper *This = impl_from_ISimpleAudioVolume(iface);
	AudioSession *session = This->session;

	TRACE("(%p)->(%u, %s)\n", session, mute, debugstr_guid(context));

	if (context)
		FIXME("Notifications not supported yet\n");

	EnterCriticalSection(&session->lock);

	session->mute = mute;

	audio_session_setvol(session, -1);

	LeaveCriticalSection(&session->lock);

	return S_OK;
}

static HRESULT WINAPI SimpleAudioVolume_GetMute(ISimpleAudioVolume *iface,
	BOOL *mute)
{
	AudioSessionWrapper *This = impl_from_ISimpleAudioVolume(iface);
	AudioSession *session = This->session;

	TRACE("(%p)->(%p)\n", session, mute);

	if (!mute)
		return NULL_PTR_ERR;

	*mute = session->mute;

	return S_OK;
}

static const ISimpleAudioVolumeVtbl SimpleAudioVolume_Vtbl =
{
	SimpleAudioVolume_QueryInterface,
	SimpleAudioVolume_AddRef,
	SimpleAudioVolume_Release,
	SimpleAudioVolume_SetMasterVolume,
	SimpleAudioVolume_GetMasterVolume,
	SimpleAudioVolume_SetMute,
	SimpleAudioVolume_GetMute
};

static HRESULT WINAPI AudioStreamVolume_QueryInterface(
	IAudioStreamVolume *iface, REFIID riid, void **ppv)
{
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IAudioStreamVolume))
		*ppv = iface;
	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI AudioStreamVolume_AddRef(IAudioStreamVolume *iface)
{
	ACImpl *This = impl_from_IAudioStreamVolume(iface);
	return IAudioClient_AddRef(&This->IAudioClient_iface);
}

static ULONG WINAPI AudioStreamVolume_Release(IAudioStreamVolume *iface)
{
	ACImpl *This = impl_from_IAudioStreamVolume(iface);
	return IAudioClient_Release(&This->IAudioClient_iface);
}

static HRESULT WINAPI AudioStreamVolume_GetChannelCount(
	IAudioStreamVolume *iface, UINT32 *out)
{
	ACImpl *This = impl_from_IAudioStreamVolume(iface);

	TRACE("(%p)->(%p)\n", This, out);

	if (!out)
		return E_POINTER;

	*out = This->fmt->nChannels;

	return S_OK;
}

static HRESULT WINAPI AudioStreamVolume_SetChannelVolume(
	IAudioStreamVolume *iface, UINT32 index, float level)
{
	ACImpl *This = impl_from_IAudioStreamVolume(iface);
	HRESULT ret;

	TRACE("(%p)->(%d, %f)\n", This, index, level);

	if (level < 0.f || level > 1.f)
		return E_INVALIDARG;

	if (index >= This->fmt->nChannels)
		return E_INVALIDARG;

	EnterCriticalSection(&This->lock);

	This->vols[index] = level;

	ret = audio_setvol(This, index);

	LeaveCriticalSection(&This->lock);

	return ret;
}

static HRESULT WINAPI AudioStreamVolume_GetChannelVolume(
	IAudioStreamVolume *iface, UINT32 index, float *level)
{
	ACImpl *This = impl_from_IAudioStreamVolume(iface);

	TRACE("(%p)->(%d, %p)\n", This, index, level);

	if (!level)
		return E_POINTER;

	if (index >= This->fmt->nChannels)
		return E_INVALIDARG;

	*level = This->vols[index];

	return S_OK;
}

static HRESULT WINAPI AudioStreamVolume_SetAllVolumes(
	IAudioStreamVolume *iface, UINT32 count, const float *levels)
{
	ACImpl *This = impl_from_IAudioStreamVolume(iface);
	int i;
	HRESULT ret;

	TRACE("(%p)->(%d, %p)\n", This, count, levels);

	if (!levels)
		return E_POINTER;

	if (count != This->fmt->nChannels)
		return E_INVALIDARG;

	EnterCriticalSection(&This->lock);

	for (i = 0; i < count; ++i)
		This->vols[i] = levels[i];

	ret = audio_setvol(This, -1);

	LeaveCriticalSection(&This->lock);

	return ret;
}

static HRESULT WINAPI AudioStreamVolume_GetAllVolumes(
	IAudioStreamVolume *iface, UINT32 count, float *levels)
{
	ACImpl *This = impl_from_IAudioStreamVolume(iface);
	int i;

	TRACE("(%p)->(%d, %p)\n", This, count, levels);

	if (!levels)
		return E_POINTER;

	if (count != This->fmt->nChannels)
		return E_INVALIDARG;

	EnterCriticalSection(&This->lock);

	for (i = 0; i < count; ++i)
		levels[i] = This->vols[i];

	LeaveCriticalSection(&This->lock);

	return S_OK;
}

static const IAudioStreamVolumeVtbl AudioStreamVolume_Vtbl =
{
	AudioStreamVolume_QueryInterface,
	AudioStreamVolume_AddRef,
	AudioStreamVolume_Release,
	AudioStreamVolume_GetChannelCount,
	AudioStreamVolume_SetChannelVolume,
	AudioStreamVolume_GetChannelVolume,
	AudioStreamVolume_SetAllVolumes,
	AudioStreamVolume_GetAllVolumes
};

static HRESULT WINAPI ChannelAudioVolume_QueryInterface(
	IChannelAudioVolume *iface, REFIID riid, void **ppv)
{
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IChannelAudioVolume))
		*ppv = iface;
	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI ChannelAudioVolume_AddRef(IChannelAudioVolume *iface)
{
	AudioSessionWrapper *This = impl_from_IChannelAudioVolume(iface);
	return AudioSessionControl_AddRef(&This->IAudioSessionControl2_iface);
}

static ULONG WINAPI ChannelAudioVolume_Release(IChannelAudioVolume *iface)
{
	AudioSessionWrapper *This = impl_from_IChannelAudioVolume(iface);
	return AudioSessionControl_Release(&This->IAudioSessionControl2_iface);
}

static HRESULT WINAPI ChannelAudioVolume_GetChannelCount(
	IChannelAudioVolume *iface, UINT32 *out)
{
	AudioSessionWrapper *This = impl_from_IChannelAudioVolume(iface);
	AudioSession *session = This->session;

	TRACE("(%p)->(%p)\n", session, out);

	if (!out)
		return NULL_PTR_ERR;

	*out = session->channel_count;

	return S_OK;
}

static HRESULT WINAPI ChannelAudioVolume_SetChannelVolume(
	IChannelAudioVolume *iface, UINT32 index, float level,
	const GUID *context)
{
	AudioSessionWrapper *This = impl_from_IChannelAudioVolume(iface);
	AudioSession *session = This->session;
	HRESULT ret;

	TRACE("(%p)->(%d, %f, %s)\n", session, index, level,
		wine_dbgstr_guid(context));

	if (level < 0.f || level > 1.f)
		return E_INVALIDARG;

	if (index >= session->channel_count)
		return E_INVALIDARG;

	if (context)
		FIXME("Notifications not supported yet\n");

	EnterCriticalSection(&session->lock);

	session->channel_vols[index] = level;

	ret = audio_session_setvol(session, index);

	LeaveCriticalSection(&session->lock);

	return ret;
}

static HRESULT WINAPI ChannelAudioVolume_GetChannelVolume(
	IChannelAudioVolume *iface, UINT32 index, float *level)
{
	AudioSessionWrapper *This = impl_from_IChannelAudioVolume(iface);
	AudioSession *session = This->session;

	TRACE("(%p)->(%d, %p)\n", session, index, level);

	if (!level)
		return NULL_PTR_ERR;

	if (index >= session->channel_count)
		return E_INVALIDARG;

	*level = session->channel_vols[index];

	return S_OK;
}

static HRESULT WINAPI ChannelAudioVolume_SetAllVolumes(
	IChannelAudioVolume *iface, UINT32 count, const float *levels,
	const GUID *context)
{
	AudioSessionWrapper *This = impl_from_IChannelAudioVolume(iface);
	AudioSession *session = This->session;
	int i;
	HRESULT ret;

	TRACE("(%p)->(%d, %p, %s)\n", session, count, levels,
		wine_dbgstr_guid(context));

	if (!levels)
		return NULL_PTR_ERR;

	if (count != session->channel_count)
		return E_INVALIDARG;

	if (context)
		FIXME("Notifications not supported yet\n");

	EnterCriticalSection(&session->lock);

	for (i = 0; i < count; ++i)
		session->channel_vols[i] = levels[i];

	ret = audio_session_setvol(session, -1);

	LeaveCriticalSection(&session->lock);

	return ret;
}

static HRESULT WINAPI ChannelAudioVolume_GetAllVolumes(
	IChannelAudioVolume *iface, UINT32 count, float *levels)
{
	AudioSessionWrapper *This = impl_from_IChannelAudioVolume(iface);
	AudioSession *session = This->session;
	int i;

	TRACE("(%p)->(%d, %p)\n", session, count, levels);

	if (!levels)
		return NULL_PTR_ERR;

	if (count != session->channel_count)
		return E_INVALIDARG;

	for (i = 0; i < count; ++i)
		levels[i] = session->channel_vols[i];

	return S_OK;
}

static const IChannelAudioVolumeVtbl ChannelAudioVolume_Vtbl =
{
	ChannelAudioVolume_QueryInterface,
	ChannelAudioVolume_AddRef,
	ChannelAudioVolume_Release,
	ChannelAudioVolume_GetChannelCount,
	ChannelAudioVolume_SetChannelVolume,
	ChannelAudioVolume_GetChannelVolume,
	ChannelAudioVolume_SetAllVolumes,
	ChannelAudioVolume_GetAllVolumes
};

static HRESULT WINAPI AudioSessionManager_QueryInterface(IAudioSessionManager2 *iface,
	REFIID riid, void **ppv)
{
	TRACE("(%p)->(%s, %p)\n", iface, debugstr_guid(riid), ppv);

	if (!ppv)
		return E_POINTER;
	*ppv = NULL;

	if (IsEqualIID(riid, &IID_IUnknown) ||
		IsEqualIID(riid, &IID_IAudioSessionManager) ||
		IsEqualIID(riid, &IID_IAudioSessionManager2))
		*ppv = iface;
	if (*ppv) {
		IUnknown_AddRef((IUnknown*)*ppv);
		return S_OK;
	}

	WARN("Unknown interface %s\n", debugstr_guid(riid));
	return E_NOINTERFACE;
}

static ULONG WINAPI AudioSessionManager_AddRef(IAudioSessionManager2 *iface)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	ULONG ref;
	ref = InterlockedIncrement(&This->ref);
	TRACE("(%p) Refcount now %u\n", This, ref);
	return ref;
}

static ULONG WINAPI AudioSessionManager_Release(IAudioSessionManager2 *iface)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	ULONG ref;
	ref = InterlockedDecrement(&This->ref);
	TRACE("(%p) Refcount now %u\n", This, ref);
	if (!ref)
		HeapFree(GetProcessHeap(), 0, This);
	return ref;
}

static HRESULT WINAPI AudioSessionManager_GetAudioSessionControl(
	IAudioSessionManager2 *iface, const GUID *session_guid, DWORD flags,
	IAudioSessionControl **out)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	AudioSession *session;
	AudioSessionWrapper *wrapper;
	HRESULT hr;

	TRACE("(%p)->(%s, %x, %p)\n", This, debugstr_guid(session_guid),
		flags, out);

	hr = get_audio_session(session_guid, This->device, 0, &session);
	if (FAILED(hr))
		return hr;

	wrapper = AudioSessionWrapper_Create(NULL);
	if (!wrapper)
		return E_OUTOFMEMORY;

	wrapper->session = session;

	*out = (IAudioSessionControl*)&wrapper->IAudioSessionControl2_iface;

	return S_OK;
}

static HRESULT WINAPI AudioSessionManager_GetSimpleAudioVolume(
	IAudioSessionManager2 *iface, const GUID *session_guid, DWORD flags,
	ISimpleAudioVolume **out)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	AudioSession *session;
	AudioSessionWrapper *wrapper;
	HRESULT hr;

	TRACE("(%p)->(%s, %x, %p)\n", This, debugstr_guid(session_guid),
		flags, out);

	hr = get_audio_session(session_guid, This->device, 0, &session);
	if (FAILED(hr))
		return hr;

	wrapper = AudioSessionWrapper_Create(NULL);
	if (!wrapper)
		return E_OUTOFMEMORY;

	wrapper->session = session;

	*out = &wrapper->ISimpleAudioVolume_iface;

	return S_OK;
}

static HRESULT WINAPI AudioSessionManager_GetSessionEnumerator(
	IAudioSessionManager2 *iface, IAudioSessionEnumerator **out)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	FIXME("(%p)->(%p) - stub\n", This, out);
	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionManager_RegisterSessionNotification(
	IAudioSessionManager2 *iface, IAudioSessionNotification *notification)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	FIXME("(%p)->(%p) - stub\n", This, notification);
	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionManager_UnregisterSessionNotification(
	IAudioSessionManager2 *iface, IAudioSessionNotification *notification)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	FIXME("(%p)->(%p) - stub\n", This, notification);
	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionManager_RegisterDuckNotification(
	IAudioSessionManager2 *iface, const WCHAR *session_id,
	IAudioVolumeDuckNotification *notification)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	FIXME("(%p)->(%p) - stub\n", This, notification);
	return E_NOTIMPL;
}

static HRESULT WINAPI AudioSessionManager_UnregisterDuckNotification(
	IAudioSessionManager2 *iface,
	IAudioVolumeDuckNotification *notification)
{
	SessionMgr *This = impl_from_IAudioSessionManager2(iface);
	FIXME("(%p)->(%p) - stub\n", This, notification);
	return E_NOTIMPL;
}

static const IAudioSessionManager2Vtbl AudioSessionManager2_Vtbl =
{
	AudioSessionManager_QueryInterface,
	AudioSessionManager_AddRef,
	AudioSessionManager_Release,
	AudioSessionManager_GetAudioSessionControl,
	AudioSessionManager_GetSimpleAudioVolume,
	AudioSessionManager_GetSessionEnumerator,
	AudioSessionManager_RegisterSessionNotification,
	AudioSessionManager_UnregisterSessionNotification,
	AudioSessionManager_RegisterDuckNotification,
	AudioSessionManager_UnregisterDuckNotification
};

HRESULT WINAPI AUDDRV_GetAudioSessionManager(IMMDevice *device,
	IAudioSessionManager2 **out)
{
	SessionMgr *This;

	This = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SessionMgr));
	if (!This)
		return E_OUTOFMEMORY;

	This->IAudioSessionManager2_iface.lpVtbl = &AudioSessionManager2_Vtbl;
	This->device = device;
	This->ref = 1;

	*out = &This->IAudioSessionManager2_iface;

	return S_OK;
}

BOOL WINAPI get_device_name_from_guid(GUID* guid, char** name, EDataFlow* flow)
{
	if (*guid == guid0) {
		*flow = 0;
		*name = strdup("Boxedaudio Render");
	} else {
		*flow = 1;
		*name = strdup("Boxedaudio Capture");
	}

	return FALSE;
}
