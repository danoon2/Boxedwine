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

float fARG(U32 arg) {
	struct int2Float i;
	i.i = arg;
	return i.f;
}

#define BOXED_AUDIO_DRV_GET_LATENCY 1
#define BOXED_AUDIO_DRV_GET_NOMINAL_SAMPLE_RATE 2
#define BOXED_AUDIO_DRV_SET_VOLUME 3
#define BOXED_AUDIO_DRV_START 4
#define BOXED_AUDIO_DRV_STOP 5
#define BOXED_AUDIO_DRV_GET_MILLIES 6
#define BOXED_AUDIO_DRV_SET_FORMAT 7
#define BOXED_AUDIO_DRV_SET_CALLBACK 8
#define BOXED_AUDIO_DRV_RELEASE 9
#define BOXED_AUDIO_DRV_OPEN 10
#define BOXED_AUDIO_DRV_INIT 11

#define BOXED_AUDIO_DRV_COUNT 12

Int99Callback* wine_audio_callback;
U32 wine_audio_callback_size;

static std::shared_ptr<KNativeAudio> audio;

#define DRV_SUCCESS 1
#define DRV_FAILURE 0 

static std::shared_ptr<KNativeAudio> getAudio() {
	if (!audio) {
		audio = KNativeAudio::createNativeAudio();
	}
	return audio;
}

// UInt32 boxed_getNominalSampleRate(AudioDeviceID devId)
static void boxedaudio_drv_getNominalSampleRate(CPU* cpu) {
	EAX = 44100;
}

// void boxed_setVolume(AudioDeviceID devId, Float32 volume, UInt32 channel)
static void boxedaudio_drv_set_volume(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	U32 level = ARG2;
	U32 channel = ARG3;
	getAudio()->setVolume(boxedAudioId, fARG(level), channel);
}

// UInt32 boxed_getLatency(AudioDeviceID devId)
static void boxedaudio_drv_get_latency(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	U32 latency = 0;
	EAX = getAudio()->getLatency(boxedAudioId, &latency);
}

// UInt32 boxed_start(AudioUnit devId)
static void boxedaudio_drv_start(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	getAudio()->start(boxedAudioId);
	EAX = 0;
}

// UInt32 boxed_stop(AudioUnit devId)
static void boxedaudio_drv_stop(CPU* cpu) {
	U32 boxedAudioId = ARG1;
	getAudio()->stop(boxedAudioId);
	EAX = 0;
}

// UInt32 boxed_getMillies()
static void boxedaudio_drv_getMillies(CPU* cpu) {
	EAX = KSystem::getMilliesSinceStart();
}

#define kAudioFormatLinearPCM 0x6C70636D
#define kAudioFormatULaw 0x756C6177
#define kAudioFormatALaw 0x616C6177
#define kAudioFormatFlagIsFloat 0x1
#define kAudioFormatFlagIsSignedInteger 0x4

#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_IEEE_FLOAT 3
#define WAVE_FORMAT_ALAW                0x0006
#define WAVE_FORMAT_MULAW 0x0007

// UInt32 boxed_setFormat(AudioUnit devId, AudioStreamBasicDescription* format)
static void boxedaudio_drv_setFormat(CPU* cpu) {
	BoxedWaveFormatEx format;
	KMemory* memory = cpu->memory;
	U32 address = ARG2;
	/*
	struct AudioStreamBasicDescription
	{
	0	Float64 mSampleRate;
	8	AudioFormatID mFormatID;
	12	AudioFormatFlags mFormatFlags;
	16	UInt32 mBytesPerPacket;
	20	UInt32 mFramesPerPacket;
	24	UInt32 mBytesPerFrame;
	28	UInt32 mChannelsPerFrame;
	32	UInt32 mBitsPerChannel;
	36	UInt32 mReserved;
	};
	*/
	U32 formatId = memory->readd(address + 8);
	if (formatId == kAudioFormatLinearPCM) {
		U32 flags = memory->readd(address + 12);
		if (flags == kAudioFormatFlagIsFloat) {
			format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		} else {
			format.wFormatTag = WAVE_FORMAT_PCM;
		}
	} else if (formatId == kAudioFormatULaw) {
		format.wFormatTag = WAVE_FORMAT_MULAW;
	} else if (formatId == kAudioFormatALaw) {
		format.wFormatTag = WAVE_FORMAT_ALAW;
	} else {
		EAX = 1;
		return;
	}
	
	format.nChannels = memory->readd(address + 28);
	long2Double d2l;
	d2l.l = memory->readq(address);
	format.nSamplesPerSec = (U32)d2l.d;	
	format.nBlockAlign = (U16)memory->readd(address + 16);	
	format.wBitsPerSample = (U16)memory->readd(address + 32);
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = sizeof(BoxedWaveFormatEx);
	getAudio()->setFormat(ARG1, &format);
	EAX = 0;
}

// typedef void(*BoxedwineRenderCallback)(UInt32 frames, void* buffer);
// void boxed_setCallback(AudioUnit devId, BoxedwineRenderCallback pfn, FD fd)
static void boxedaudio_drv_setCallback(CPU* cpu) {
	getAudio()->setCallback(ARG1, ARG3, ARG2);
}

// void boxed_release(AudioUnit unit)
static void boxedaudio_drv_release(CPU* cpu) {
	getAudio()->release(ARG1);
}

// UInt32 boxed_open(UInt32 isRenderer)
static void boxedaudio_drv_open(CPU* cpu) {
	EAX = getAudio()->getEndPoint(ARG1);
}

// UInt32 boxed_init(AudioUnit unit)
static void boxedaudio_drv_init(CPU* cpu) {
	EAX = getAudio()->init(cpu->thread->process, ARG1);
}

void initWineAudio() {
	if (!wine_audio_callback) {
		wine_audio_callback_size = BOXED_AUDIO_DRV_COUNT;
		wine_audio_callback = new Int99Callback[wine_audio_callback_size];
		wine_audio_callback[BOXED_AUDIO_DRV_SET_VOLUME] = boxedaudio_drv_set_volume;
		wine_audio_callback[BOXED_AUDIO_DRV_GET_LATENCY] = boxedaudio_drv_get_latency;
		wine_audio_callback[BOXED_AUDIO_DRV_START] = boxedaudio_drv_start;
		wine_audio_callback[BOXED_AUDIO_DRV_STOP] = boxedaudio_drv_stop;
		wine_audio_callback[BOXED_AUDIO_DRV_GET_NOMINAL_SAMPLE_RATE] = boxedaudio_drv_getNominalSampleRate;
		wine_audio_callback[BOXED_AUDIO_DRV_GET_MILLIES] = boxedaudio_drv_getMillies;
		wine_audio_callback[BOXED_AUDIO_DRV_SET_FORMAT] = boxedaudio_drv_setFormat;
		wine_audio_callback[BOXED_AUDIO_DRV_SET_CALLBACK] = boxedaudio_drv_setCallback;
		wine_audio_callback[BOXED_AUDIO_DRV_RELEASE] = boxedaudio_drv_release;
		wine_audio_callback[BOXED_AUDIO_DRV_OPEN] = boxedaudio_drv_open;
		wine_audio_callback[BOXED_AUDIO_DRV_INIT] = boxedaudio_drv_init;
    }
}
