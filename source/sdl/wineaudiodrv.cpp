/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#include "knativewindow.h"
#include "knativesystem.h"
#include "knativeaudio.h"

#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)
#define ARG5 cpu->peek32(5)
#define ARG6 cpu->peek32(6)
#define ARG7 cpu->peek32(7)
#define ARG8 cpu->peek32(8)
#define ARG9 cpu->peek32(9)

struct int2Float {
	union {
		U32 i;
		float f;
	};
};

float fARG(U32 arg) {
	struct int2Float i;
	i.i = arg;
	return i.f;
}

#define BOXED_BASE 0

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
#define BOXED_AUDIO_LAST                            (BOXED_BASE+41)

Int99Callback* wine_audio_callback;
U32 wine_audio_callback_size;
U32 wine_audio_callback_base = 200;

static std::shared_ptr<KNativeAudio> audio;

#define DRV_SUCCESS 1
#define DRV_FAILURE 0 

static void boxedaudio_drv_load(CPU* cpu) {
	if (audio->load()) {
		EAX = DRV_SUCCESS;
	} else {
		EAX = DRV_FAILURE;
	}
}

static void boxedaudio_drv_free(CPU* cpu) {
	audio->free();
	EAX = DRV_SUCCESS;
}

static void boxedaudio_drv_open(CPU* cpu) {
	if (audio->open()) {
		EAX = DRV_SUCCESS;
	} else {
		EAX = DRV_FAILURE;
	}
}

static void boxedaudio_drv_close(CPU* cpu) {
	if (audio->close()) {
		EAX = DRV_SUCCESS;
	} else {
		EAX = DRV_FAILURE;
	}
}

static void boxedaudio_drv_configure(CPU* cpu) {
	if (audio->configure()) {
		EAX = DRV_SUCCESS;
	} else {
		EAX = DRV_FAILURE;
	}
}

static void boxedaudio_midi_out_open(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 pDesc = ARG2; // LPMIDIOPENDESC
	U32 dwFlags = ARG3;
	U32 fd = ARG4;
	EAX = audio->midiOutOpen(KThread::currentThread()->process.get(), wDevID, pDesc, dwFlags, fd);
}

static void boxedaudio_midi_out_close(CPU* cpu) {
	U32 wDevID = ARG1;
	EAX = audio->midiOutClose(wDevID);
}

static void boxedaudio_midi_out_data(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 dwParam = ARG2;
	EAX = audio->midiOutData(wDevID, dwParam);
}

static void boxedaudio_midi_out_long_data(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpMidiHdr = ARG2; // LPMIDIHDR
	U32 dwSize = ARG3;
	EAX = audio->midiOutLongData(cpu->thread, wDevID, lpMidiHdr, dwSize);
}

static void boxedaudio_midi_out_prepare(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpMidiHdr = ARG2; // LPMIDIHDR
	U32 dwSize = ARG3;
	EAX = audio->midiOutPrepare(cpu->thread, wDevID, lpMidiHdr, dwSize);
}

static void boxedaudio_midi_out_unprepare(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpMidiHdr = ARG2; // LPMIDIHDR
	U32 dwSize = ARG3;
	EAX = audio->midiOutUnprepare(cpu->thread, wDevID, lpMidiHdr, dwSize);
}

static void boxedaudio_midi_out_get_device_caps(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpCaps = ARG2; // LPMIDIOUTCAPSW
	U32 dwSize = ARG3;
	EAX = audio->midiOutGetDevCaps(cpu->thread, wDevID, lpCaps, dwSize);
}

static void boxedaudio_midi_out_get_number_of_devices(CPU* cpu) {
	EAX = audio->midiOutGetNumDevs();
}

static void boxedaudio_midi_out_get_volume(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpdwVolume = ARG2; // DWORD*
	EAX = audio->midiOutGetVolume(cpu->thread, wDevID, lpdwVolume);
}

static void boxedaudio_midi_out_set_volume(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 dwVolume = ARG2;
	EAX = audio->midiOutSetVolume(wDevID, dwVolume);
}

static void boxedaudio_midi_out_reset(CPU* cpu) {
	U32 wDevID = ARG1;
	EAX = audio->midiOutReset(wDevID);
}

static void boxedaudio_midi_in_open(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 pDesc = ARG2; // LPMIDIOPENDESC
	U32 dwFlags = ARG3;
	EAX = audio->midiInOpen(wDevID, pDesc, dwFlags);
}

static void boxedaudio_midi_in_close(CPU* cpu) {
	U32 wDevID = ARG1;
	EAX = audio->midiInClose(wDevID);
}

static void boxedaudio_midi_in_add_buffer(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpMidiHdr = ARG2; // LPMIDIHDR
	U32 dwSize = ARG3;
	EAX = audio->midiInAddBuffer(wDevID, lpMidiHdr, dwSize);
}

static void boxedaudio_midi_in_prepare(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpMidiHdr = ARG2; // LPMIDIHDR
	U32 dwSize = ARG3;
	EAX = audio->midiInPrepare(wDevID, lpMidiHdr, dwSize);
}

static void boxedaudio_midi_in_unprepare(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpMidiHdr = ARG2; // LPMIDIHDR
	U32 dwSize = ARG3;
	EAX = audio->midiInUnprepare(wDevID, lpMidiHdr, dwSize);
}

static void boxedaudio_midi_in_get_device_caps(CPU* cpu) {
	U32 wDevID = ARG1;
	U32 lpCaps = ARG2; // LPMIDIOUTCAPSW
	U32 dwSize = ARG3;
	EAX = audio->midiInGetDevCaps(wDevID, lpCaps, dwSize);
}

static void boxedaudio_midi_in_get_number_of_devices(CPU* cpu) {
	EAX = audio->midiInGetNumDevs();
}

static void boxedaudio_midi_in_start(CPU* cpu) {
	U32 wDevID = ARG1;
	EAX = audio->midiInStart(wDevID);
}

static void boxedaudio_midi_in_stop(CPU* cpu) {
	U32 wDevID = ARG1;
	EAX = audio->midiInStop(wDevID);
}

static void boxedaudio_midi_in_reset(CPU* cpu) {
	U32 wDevID = ARG1;
	EAX = audio->midiInReset(wDevID);
}

static void boxedaudio_drv_has_device(CPU* cpu) {
	U32 isRender = ARG1;
	if (!audio) {
		audio = KNativeAudio::createNativeAudio();
	}
	EAX = audio->hasDevice(isRender == 1);
}

static void boxedaudio_drv_get_end_point(CPU* cpu) {
	U32 isRender = ARG1;
	U32 adevid = ARG2;
	EAX = audio->getEndPoint(isRender == 1, adevid);
}

static void boxedaudio_drv_release(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	audio->release(boxedAudioId);
}

static void boxedaudio_drv_set_volume(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	U32 level = ARG2;
	U32 channel = ARG3;
	audio->setVolume(boxedAudioId, fARG(level), channel);
}

static void boxedaudio_drv_is_format_supported(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	U32 addressWaveFormat = ARG2;
	EAX = audio->isFormatSupported(cpu->thread, boxedAudioId, addressWaveFormat);
}

static void boxedaudio_drv_get_mix_format(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	U32 addressWaveFormat = ARG2;
	EAX = audio->getMixFormat(cpu->thread, boxedAudioId, addressWaveFormat);
}

static void boxedaudio_drv_lock(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	audio->lock(boxedAudioId);
}

static void boxedaudio_drv_unlock(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	audio->unlock(boxedAudioId);
}

static void boxedaudio_drv_capture_resample(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	audio->release(boxedAudioId);
}

static void boxedaudio_drv_get_latency(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	U32 addressLatency = ARG2;
	U32 latency = 0;
	EAX = audio->getLatency(boxedAudioId, &latency);
	cpu->memory->writed(addressLatency, latency);
}

static void boxedaudio_drv_init(CPU* cpu) {
	U32 isRender = ARG1;
	U32 boxedAudioId = ARG2;
	U32 addressFmt = ARG3;
	U32 addressPeriodFrames = ARG4;
	U32 addressLocalBuffer = ARG5;
	U32 addressWriOffsFrames = ARG6;
	U32 addressHeldFrames = ARG7;
	U32 addressLclOffsFrames = ARG8;
	U32 bufsizeFrames = ARG9;
	EAX = audio->init(KThread::currentThread()->process, isRender, boxedAudioId, addressFmt, addressPeriodFrames, addressLocalBuffer, addressWriOffsFrames, addressHeldFrames, addressLclOffsFrames, bufsizeFrames);
}

static void boxedaudio_drv_start(CPU* cpu) {
	U32 boxedAudioId = ARG1;
    U32 eventFd = ARG2;
	audio->start(boxedAudioId, eventFd);
}

static void boxedaudio_drv_stop(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	audio->stop(boxedAudioId);
}

static void boxedaudio_get_period(CPU* cpu) {
    //U32 minPeriodAddress = ARG1;
    //U32 defaultPeriodAddress = ARG2;
    //U64 minPeriod = readq(minPeriodAddress); // default for all wine audio drivers is 50000 (50ms)
    //U64 defaultPeriod = readq(defaultPeriodAddress); // default for all wine audio drivers is 100000 (100ms)
    //writeq(minPeriodAddress, 50000);
    //writeq(defaultPeriodAddress, 100000);
}

static void boxedaudio_use_timer(CPU* cpu) {
    // the wine audio driver for mac used a timer to trigger DSOUND_mixthread, the other linux drivers used an event that was triggered from the audio callback.  This will switch between them.  I left this in just incase I want to experiment in the future between them.

	// when this is 1, spherejongg hangs after the first sound and the held_frames is 0, it seems like the mixer isn't being signaled any more
    EAX = 1;
}

static void boxedaudio_set_priority(CPU* cpu) {
    int priority = (int)ARG1;
    if (priority>0) {
        Platform::setCurrentThreadPriorityHigh();
    }
}

void initWineAudio() {
	if (!wine_audio_callback) {
		wine_audio_callback_size = BOXED_AUDIO_LAST - BOXED_BASE + 1;
		wine_audio_callback = new Int99Callback[wine_audio_callback_size];
		wine_audio_callback[BOXED_AUDIO_DRV_LOAD] = boxedaudio_drv_load;
		wine_audio_callback[BOXED_AUDIO_DRV_FREE] = boxedaudio_drv_free;
		wine_audio_callback[BOXED_AUDIO_DRV_OPEN] = boxedaudio_drv_open;
		wine_audio_callback[BOXED_AUDIO_DRV_CLOSE] = boxedaudio_drv_close;
		wine_audio_callback[BOXED_AUDIO_DRV_CONFIGURE] = boxedaudio_drv_configure;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_OPEN] = boxedaudio_midi_out_open;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_CLOSE] = boxedaudio_midi_out_close;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_DATA] = boxedaudio_midi_out_data;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_LONGDATA] = boxedaudio_midi_out_long_data;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_PREPARE] = boxedaudio_midi_out_prepare;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_UNPREPARE] = boxedaudio_midi_out_unprepare;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_GETDEVCAPS] = boxedaudio_midi_out_get_device_caps;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_GETNUMDEVS] = boxedaudio_midi_out_get_number_of_devices;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_GETVOLUME] = boxedaudio_midi_out_get_volume;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_SETVOLUME] = boxedaudio_midi_out_set_volume;
		wine_audio_callback[BOXED_AUDIO_MIDIOUT_RESET] = boxedaudio_midi_out_reset;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_OPEN] = boxedaudio_midi_in_open;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_CLOSE] = boxedaudio_midi_in_close;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_ADDBUFFER] = boxedaudio_midi_in_add_buffer;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_PREPARE] = boxedaudio_midi_in_prepare;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_UNPREPARE] = boxedaudio_midi_in_unprepare;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_GETDEVCAPS] = boxedaudio_midi_in_get_device_caps;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_GETNUMDEVS] = boxedaudio_midi_in_get_number_of_devices;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_START] = boxedaudio_midi_in_start;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_STOP] = boxedaudio_midi_in_stop;
		wine_audio_callback[BOXED_AUDIO_MIDIIN_RESET] = boxedaudio_midi_in_reset;
		wine_audio_callback[BOXED_AUDIO_DRV_HAS_DEVICE] = boxedaudio_drv_has_device;
		wine_audio_callback[BOXED_AUDIO_DRV_GET_ENDPOINT] = boxedaudio_drv_get_end_point;
		wine_audio_callback[BOXED_AUDIO_DRV_RELEASE] = boxedaudio_drv_release;
		wine_audio_callback[BOXED_AUDIO_DRV_SET_VOLUME] = boxedaudio_drv_set_volume;
		wine_audio_callback[BOXED_AUDIO_DRV_IS_FORMAT_SUPPORTED] = boxedaudio_drv_is_format_supported;
		wine_audio_callback[BOXED_AUDIO_DRV_GET_MIX_FORMAT] = boxedaudio_drv_get_mix_format;
		wine_audio_callback[BOXED_AUDIO_DRV_LOCK] = boxedaudio_drv_lock;
		wine_audio_callback[BOXED_AUDIO_DRV_UNLOCK] = boxedaudio_drv_unlock;
		wine_audio_callback[BOXED_AUDIO_DRV_CAPTURE_RESAMPLE] = boxedaudio_drv_capture_resample;
		wine_audio_callback[BOXED_AUDIO_DRV_GET_LATENCY] = boxedaudio_drv_get_latency;
		wine_audio_callback[BOXED_AUDIO_DRV_INIT] = boxedaudio_drv_init;
		wine_audio_callback[BOXED_AUDIO_DRV_START] = boxedaudio_drv_start;
		wine_audio_callback[BOXED_AUDIO_DRV_STOP] = boxedaudio_drv_stop;
        wine_audio_callback[BOXED_AUDIO_DRV_GET_PERIOD] = boxedaudio_get_period;
        wine_audio_callback[BOXED_AUDIO_DRV_USE_TIMER] = boxedaudio_use_timer;
        wine_audio_callback[BOXED_AUDIO_DRV_SET_PRIORITY] = boxedaudio_set_priority;
    }
}
