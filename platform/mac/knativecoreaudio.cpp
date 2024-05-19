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

// the core audio specific code was taken from
// https://github.com/wine-mirror/wine/tree/master/dlls/winecoreaudio.drv
// License from that file

/*
 * Copyright 2011 Andrew Eikum for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "boxedwine.h"
#include "knativeaudio.h"

#ifdef BOXEDWINE_CORE_AUDIO
#include <os/lock.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioFormat.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreMIDI/CoreMIDI.h>
#include <AudioToolbox/AudioToolbox.h>

#ifdef S_OK
#undef S_OK
#endif
#define S_OK 0

#ifdef E_FAIL
#undef E_FAIL
#endif
#define E_FAIL 0x80004005

#ifdef E_OUTOFMEMORY
#undef E_OUTOFMEMORY
#endif
#define E_OUTOFMEMORY 0x8007000E

#define WAVE_FORMAT_EXTENSIBLE                 0xFFFE
#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_IEEE_FLOAT 3
#define WAVE_FORMAT_ALAW                0x0006
#define WAVE_FORMAT_MULAW 0x0007

#ifdef FAILED
#undef FAILED
#endif
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define HRESULT S32
#define SCODE S32
#define K_MAKE_HRESULT(sev,fac,code) ((HRESULT) (((U32)(sev)<<31) | ((U32)(fac)<<16) | ((U32)(code))) )
#define K_MAKE_SCODE(sev,fac,code) ((SCODE) (((U32)(sev)<<31) | ((U32)(fac)<<16) | ((U32)(code))) )
#define SEVERITY_SUCCESS    0
#define SEVERITY_ERROR      1
#define FACILITY_AUDCLNT 0x889
#define AUDCLNT_ERR(n) K_MAKE_HRESULT(SEVERITY_ERROR, FACILITY_AUDCLNT, n)
#define AUDCLNT_SUCCESS(n) K_MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_AUDCLNT, n)

#define AUDCLNT_E_DEVICE_INVALIDATED         AUDCLNT_ERR(0x004)
#define AUDCLNT_E_UNSUPPORTED_FORMAT         AUDCLNT_ERR(0x008)

static const BoxedGUID CORE_AUDIO_KSDATAFORMAT_SUBTYPE_PCM(0x00000001, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
static const BoxedGUID CORE_AUDIO_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT(0x00000003, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
static const BoxedGUID CORE_AUDIO_KSDATAFORMAT_SUBTYPE_ALAW(0x00000006, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
static const BoxedGUID CORE_AUDIO_KSDATAFORMAT_SUBTYPE_MULAW(0x00000007, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

static U32 MIDIOut_NumDevs = 0;
//static U32 MIDIIn_NumDevs = 0;

PACKED(
struct WineMidiHdr {
    U32       lpData;               /* pointer to locked data block */
    U32       dwBufferLength;       /* length of data in data block */
    U32       dwBytesRecorded;      /* used for input only */
    U32          dwUser;               /* for client's use */
    U32       dwFlags;              /* assorted flags (see defines) */
    U32       lpNext;               /* reserved for driver */
    U32       reserved;             /* reserved for driver */
    U32       dwOffset;             /* Callback offset into buffer */
    U32       dwReserved[8];        /* Reserved for MMSYSTEM */
    
    void writeFlags(KMemory* memory, U32 address) {
        memory->writed(address+16, dwFlags);
    }
    
    void readFlags(KMemory* memory, U32 address) {
        dwFlags = memory->readd(address+16);
    }
    
    void read(KMemory* memory, U32 address, U32 dwSize) {
        memory->memcpy(this, address, dwSize);
    }
    
    void write(KMemory* memory, U32 address, U32 dwSize) {
        memory->memcpy(address, this, dwSize);
    }
}
);

class KNativeAudioCoreAudioData {
public:
	KNativeAudioCoreAudioData() : lock(OS_UNFAIR_LOCK_INIT), isRender(false), isPlaying(false), eventFd(0), cap_held_frames(0), resamp_bufsize_frames(0), resamp_buffer(0), cap_offs_frames(0), bufsize_frames(0), address_local_buffer(0), address_wri_offs_frames(0), address_held_frames(0), period_frames(0) {
    }
	~KNativeAudioCoreAudioData() {
		if (resamp_buffer) {
			delete[] resamp_buffer;
		}
	}
	os_unfair_lock lock;

	AudioComponentInstance unit;
	AudioObjectPropertyScope scope;
	AudioDeviceID adevid;
	AudioConverterRef converter;
	AudioStreamBasicDescription dev_desc; /* audio unit format, not necessarily the same as fmt */

    std::weak_ptr<KProcess> process;

    bool isRender;
    bool isPlaying;
    U32 eventFd;
    
    U32 cap_held_frames;
    U32 resamp_bufsize_frames;
    U8* resamp_buffer;
    U32 cap_offs_frames;
    U32 bufsize_frames;

    // points to memory in the emulator, must be locked before read/write
    U32 address_local_buffer;
    U32 address_wri_offs_frames;
    U32 address_held_frames;
    U32 address_lcl_offs_frames;

    // mirrored in emulator side
    U32 period_frames; // read only, doesn't change
    BoxedWaveFormatExtensible fmt; // read only, doesn't change
};

class KNativeAudioCoreAudio : public KNativeAudio, public std::enable_shared_from_this<KNativeAudioCoreAudio> {
public:
    virtual ~KNativeAudioCoreAudio() {}
    virtual bool load() override;
    virtual void free() override;
    virtual bool open() override;
    virtual bool close() override;
    virtual void start(U32 boxedAudioId, U32 eventFd) override;
    virtual void stop(U32 boxedAudioId) override;
    virtual bool configure() override;
    virtual U32 hasDevice(bool isRender) override;
    virtual U32 getEndPoint(bool isRender, U32 adevid) override;
    virtual void release(U32 boxedAudioId) override;
    virtual void captureResample(U32 boxedAudioId) override;
    virtual U32 init(std::shared_ptr<KProcess> process, bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames) override;
    virtual U32 getLatency(U32 boxedAudioId, U32* latency) override;
    virtual void lock(U32 boxedAudioId) override;
    virtual void unlock(U32 boxedAudioId) override;
    virtual U32 isFormatSupported(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) override;
    virtual U32 getMixFormat(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) override;
    virtual void setVolume(U32 boxedAudioId, float level, U32 channel) override;
    virtual void cleanup() override;
    
    virtual U32 midiOutOpen(KProcess* process, U32 wDevID, U32 lpDesc, U32 dwFlags, U32 fd) override;
    virtual U32 midiOutClose(U32 wDevID) override;
    virtual U32 midiOutData(U32 wDevID, U32 dwParam) override;
    virtual U32 midiOutLongData(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
    virtual U32 midiOutPrepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
    virtual U32 midiOutUnprepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
    virtual U32 midiOutGetDevCaps(KThread* thread, U32 wDevID, U32 lpCaps, U32 dwSize) override;
    virtual U32 midiOutGetNumDevs() override;
    virtual U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) override;
    virtual U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) override;
    virtual U32 midiOutReset(U32 wDevID) override;

    virtual U32 midiInOpen(U32 wDevID, U32 lpDesc, U32 dwFlags) override;
    virtual U32 midiInClose(U32 wDevID) override;
    virtual U32 midiInAddBuffer(U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
    virtual U32 midiInPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
    virtual U32 midiInUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
    virtual U32 midiInGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) override;
    virtual U32 midiInGetNumDevs() override;
    virtual U32 midiInStart(U32 wDevID) override;
    virtual U32 midiInStop(U32 wDevID) override;
    virtual U32 midiInReset(U32 wDevID) override;

	KNativeAudioCoreAudioData data[2];

	KNativeAudioCoreAudioData& getData(bool isRender) {
		if (isRender) {
			return data[0];
		}
		return data[1];
	}

	KNativeAudioCoreAudioData* getDataFromId(U32 boxedAudioId) {
		if (boxedAudioId == 1) {
			return &data[0];
		} else if (boxedAudioId == 2) {
			return &data[1];
		}
		return NULL;
	}
};

static bool CoreAudio_MIDIInit();
bool KNativeAudioCoreAudio::load() {
    return CoreAudio_MIDIInit();
}

static void CoreAudio_MIDIRelease();
void KNativeAudioCoreAudio::free() {
	CoreAudio_MIDIRelease();
}

bool KNativeAudioCoreAudio::open() {
	return true;
}

bool KNativeAudioCoreAudio::close() {
	return true;
}

bool KNativeAudioCoreAudio::configure() {
	// MessageBoxA(0, "CoreAudio driver!", "CoreAudio driver", MB_OK);
	return true;
}

static HRESULT osstatus_to_hresult(OSStatus sc)
{
	switch (sc) {
	case kAudioFormatUnsupportedDataFormatError:
	case kAudioFormatUnknownFormatError:
	case kAudioDeviceUnsupportedFormatError:
		return AUDCLNT_E_UNSUPPORTED_FORMAT;
	case kAudioHardwareBadDeviceError:
		return AUDCLNT_E_DEVICE_INVALIDATED;
	}
	return E_FAIL;
}

U32 KNativeAudioCoreAudio::hasDevice(bool isRender) {
	UInt32 devsize, size;
	AudioDeviceID default_id;
	AudioObjectPropertyAddress addr;
	OSStatus sc;

	addr.mScope = kAudioObjectPropertyScopeGlobal;
	addr.mElement = kAudioObjectPropertyElementMaster;
    if (isRender) {
		addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
    } else {
		addr.mSelector = kAudioHardwarePropertyDefaultInputDevice;
    }
    
	size = sizeof(default_id);
	sc = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &default_id);
	if (sc != noErr) {
		kwarn("Getting _DefaultInputDevice property failed: %x\n", (int)sc);
		default_id = -1;
	}

	addr.mSelector = kAudioHardwarePropertyDevices;
	sc = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, NULL, &devsize);
	if (sc != noErr) {
		kwarn("Getting _Devices property size failed: %x\n", (int)sc);
		return osstatus_to_hresult(sc);
	}

	return 0;
}

static AudioComponentInstance get_audiounit(bool isRender, AudioDeviceID adevid)
{
	AudioComponentInstance unit;
	AudioComponent comp;
	AudioComponentDescription desc;
	OSStatus sc;

	memset(&desc, 0, sizeof(desc));
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	if (!(comp = AudioComponentFindNext(NULL, &desc))) {
		kwarn("AudioComponentFindNext failed\n");
		return NULL;
	}

	sc = AudioComponentInstanceNew(comp, &unit);
	if (sc != noErr) {
		kwarn("AudioComponentInstanceNew failed: %x\n", (int)sc);
		return NULL;
	}

	if (!isRender) {
		UInt32 enableio;

		enableio = 1;
		sc = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &enableio, sizeof(enableio));
		if (sc != noErr) {
			kwarn("Couldn't enable I/O on input element: %x\n", (int)sc);
			AudioComponentInstanceDispose(unit);
			return NULL;
		}

		enableio = 0;
		sc = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableio, sizeof(enableio));
		if (sc != noErr) {
			kwarn("Couldn't disable I/O on output element: %x\n", (int)sc);
			AudioComponentInstanceDispose(unit);
			return NULL;
		}
	}

	sc = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &adevid, sizeof(adevid));
	if (sc != noErr) {
		kwarn("Couldn't set audio unit device\n");
		AudioComponentInstanceDispose(unit);
		return NULL;
	}

	return unit;
}

static HRESULT getDefaultAudioId(bool isRender, U32* audioId) {
    U32 devsize, size;
    AudioDeviceID *devices;
    AudioDeviceID default_id;
    AudioObjectPropertyAddress addr;
    OSStatus sc;
    int i, ndevices;

    addr.mScope = kAudioObjectPropertyScopeGlobal;
    addr.mElement = kAudioObjectPropertyElementMaster;
    if(isRender) {
        addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
    } else {
        addr.mSelector = kAudioHardwarePropertyDefaultInputDevice;
    }

    size = sizeof(default_id);
    sc = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &default_id);
    if(sc != noErr){
        kwarn("Getting _DefaultInputDevice property failed: %x\n", (int)sc);
        default_id = -1;
    } else {
        *audioId = default_id;
        return S_OK;
    }

    addr.mSelector = kAudioHardwarePropertyDevices;
    sc = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, NULL, &devsize);
    if(sc != noErr){
        kwarn("Getting _Devices property size failed: %x\n", (int)sc);
        return osstatus_to_hresult(sc);
    }
    ndevices = devsize / sizeof(AudioDeviceID);
    devices = new AudioDeviceID[ndevices];
    
    if(!devices)
        return E_OUTOFMEMORY;

    sc = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &devsize, devices);
    if(sc != noErr){
        kwarn("Getting _Devices property failed: %x\n", (int)sc);
        delete[] devices;
        return osstatus_to_hresult(sc);
    }

    for(i = 0; i < ndevices; ++i) {
        AudioBufferList *buffers;
        int j;

        addr.mSelector = kAudioDevicePropertyStreamConfiguration;
        if (isRender) {
            addr.mScope = kAudioDevicePropertyScopeOutput;
        } else {
            addr.mScope = kAudioDevicePropertyScopeInput;
        }
        addr.mElement = 0;
        sc = AudioObjectGetPropertyDataSize(devices[i], &addr, 0, NULL, &size);
        if(sc != noErr){
            kwarn("Unable to get _StreamConfiguration property size for device %u: %x\n", (unsigned int)devices[i], (int)sc);
            continue;
        }

        buffers = (AudioBufferList*)malloc(size);
        if(!buffers){
            delete[] devices;
            return E_OUTOFMEMORY;
        }

        sc = AudioObjectGetPropertyData(devices[i], &addr, 0, NULL, &size, buffers);
        if(sc != noErr){
            kwarn("Unable to get _StreamConfiguration property for device %u: %x\n", (unsigned int)devices[i], (int)sc);
            free(buffers);
            continue;
        }

        /* check that there's at least one channel in this device before
         * we claim it as usable */
        for (j = 0; j < buffers->mNumberBuffers; ++j) {
            if (buffers->mBuffers[j].mNumberChannels > 0) {
                break;
            }
        }
        if (j >= buffers->mNumberBuffers) {
            free(buffers);
            continue;
        }

        free(buffers);
        *audioId = devices[i];
        delete[] devices;
        return S_OK;
    }

    *audioId = 0;

    delete[] devices;

    return S_OK;
}

U32 KNativeAudioCoreAudio::getEndPoint(bool isRender, U32 adevid) {
	KNativeAudioCoreAudioData& data = getData(isRender);
	if (isRender) {
		data.scope = kAudioDevicePropertyScopeOutput;
	} else {
		data.scope = kAudioDevicePropertyScopeInput;
	}
	getDefaultAudioId(isRender, &data.adevid);
	if (!(getData(isRender).unit = get_audiounit(isRender, data.adevid))) {
		return 0;
	}
	return isRender ? 1 : 2;
}

void KNativeAudioCoreAudio::release(U32 boxedAudioId) {
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (data) {
		AudioOutputUnitStop(data->unit);
		AudioComponentInstanceDispose(data->unit);
		if (data->converter)
			AudioConverterDispose(data->converter);
	}
}

/*
// place data from cap_buffer into provided AudioBufferList
static OSStatus feed_cb(AudioConverterRef converter, UInt32* nframes, AudioBufferList* data,
	AudioStreamPacketDescription** packets, void* user)
{
	KNativeAudioCoreAudioData* This = (KNativeAudioCoreAudioData *)user;

	*nframes = std::min(*nframes, This->cap_held_frames);
	if (!*nframes) {
		data->mBuffers[0].mData = NULL;
		data->mBuffers[0].mDataByteSize = 0;
		data->mBuffers[0].mNumberChannels = This->fmt.nChannels;
		return noErr;
	}

	data->mBuffers[0].mDataByteSize = *nframes * This->fmt.nBlockAlign;
	data->mBuffers[0].mNumberChannels = This->fmt.nChannels;

	if (This->cap_offs_frames + *nframes > This->cap_bufsize_frames) {
		UINT32 chunk_frames = This->cap_bufsize_frames - This->cap_offs_frames;

		if (This->wrap_bufsize_frames < *nframes) {
			free(This->wrap_buffer);
			This->wrap_buffer = malloc(data->mBuffers[0].mDataByteSize);
			This->wrap_bufsize_frames = *nframes;
		}

		memcpy(This->wrap_buffer, This->cap_buffer + This->cap_offs_frames * This->fmt->nBlockAlign,
			chunk_frames * This->fmt->nBlockAlign);
		memcpy(This->wrap_buffer + chunk_frames * This->fmt->nBlockAlign, This->cap_buffer,
			(*nframes - chunk_frames) * This->fmt->nBlockAlign);

		data->mBuffers[0].mData = This->wrap_buffer;
	}
	else
		data->mBuffers[0].mData = This->cap_buffer + This->cap_offs_frames * This->fmt->nBlockAlign;

	This->cap_offs_frames += *nframes;
	This->cap_offs_frames %= This->cap_bufsize_frames;
	This->cap_held_frames -= *nframes;

	if (packets)
		*packets = NULL;

	return noErr;
}

static void ca_wrap_buffer(BYTE* dst, UINT32 dst_offs, UINT32 dst_bytes,
	BYTE* src, UINT32 src_bytes)
{
	UINT32 chunk_bytes = dst_bytes - dst_offs;

	if (chunk_bytes < src_bytes) {
		memcpy(dst + dst_offs, src, chunk_bytes);
		memcpy(dst, src + chunk_bytes, src_bytes - chunk_bytes);
	}
	else
		memcpy(dst + dst_offs, src, src_bytes);
}
*/
void KNativeAudioCoreAudio::captureResample(U32 boxedAudioId) {
    /*
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	UINT32 resamp_period_frames = MulDiv(data->period_frames, data->dev_desc.mSampleRate, data->fmt->nSamplesPerSec);
	OSStatus sc;

	// the resampling process often needs more source frames than we'd
    // guess from a straight conversion using the sample rate ratio. so
	// only convert if we have extra source data.
	while (data->cap_held_frames > resamp_period_frames * 2) {
		AudioBufferList converted_list;
		UInt32 wanted_frames = data->period_frames;

		converted_list.mNumberBuffers = 1;
		converted_list.mBuffers[0].mNumberChannels = data->fmt->nChannels;
		converted_list.mBuffers[0].mDataByteSize = wanted_frames * data->fmt->nBlockAlign;

		if (data->resamp_bufsize_frames < wanted_frames) {
			if (data->resamp_buffer) {
				delete[] data->resamp_buffer;
			}
			data->resamp_buffer = new U8[converted_list.mBuffers[0].mDataByteSize];
			data->resamp_bufsize_frames = wanted_frames;
		}

		converted_list.mBuffers[0].mData = data->resamp_buffer;

		sc = AudioConverterFillComplexBuffer(data->converter, feed_cb, data, &wanted_frames, &converted_list, NULL);
		if (sc != noErr) {
			kwarn("AudioConverterFillComplexBuffer failed: %x\n", (int)sc);
			break;
		}

		U32 wri_offs_frames = readd(data->address_wri_offs_frames);
		ca_wrap_buffer(This->local_buffer,
			wri_offs_frames * data->fmt->nBlockAlign,
			data->bufsize_frames * data->fmt->nBlockAlign,
			data->resamp_buffer, wanted_frames * data->fmt->nBlockAlign);

		wri_offs_frames += wanted_frames;
		wri_offs_frames %= data->bufsize_frames;
		writed(data->address_wri_offs_frames, wri_offs_frames);
		U32 held_frame = readd(data->address_held_frames);
		if (held_frames + wanted_frames > data->bufsize_frames) {
			This->lcl_offs_frames += buf_ptr_diff(This->lcl_offs_frames,
				This->wri_offs_frames, data->bufsize_frames);
			held_frames = data->bufsize_frames;
		}
		else
			held_frames += wanted_frames;
		writed(data->address_held_frames, held_frame);
	}
    */
}

static HRESULT ca_get_audiodesc(AudioStreamBasicDescription *desc, const BoxedWaveFormatExtensible* fmt)
{
    desc->mFormatFlags = 0;

    if (fmt->ex.wFormatTag == WAVE_FORMAT_PCM || (fmt->ex.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt->SubFormat ==  CORE_AUDIO_KSDATAFORMAT_SUBTYPE_PCM)) {
        desc->mFormatID = kAudioFormatLinearPCM;
        if(fmt->ex.wBitsPerSample > 8) {
            desc->mFormatFlags = kAudioFormatFlagIsSignedInteger;
        }
    } else if (fmt->ex.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || (fmt->ex.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt->SubFormat == CORE_AUDIO_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)){
        desc->mFormatID = kAudioFormatLinearPCM;
        desc->mFormatFlags = kAudioFormatFlagIsFloat;
    } else if (fmt->ex.wFormatTag == WAVE_FORMAT_MULAW || (fmt->ex.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt->SubFormat == CORE_AUDIO_KSDATAFORMAT_SUBTYPE_MULAW)) {
        desc->mFormatID = kAudioFormatULaw;
    } else if (fmt->ex.wFormatTag == WAVE_FORMAT_ALAW || (fmt->ex.wFormatTag == WAVE_FORMAT_EXTENSIBLE && fmt->SubFormat == CORE_AUDIO_KSDATAFORMAT_SUBTYPE_ALAW)) {
        desc->mFormatID = kAudioFormatALaw;
    } else {
        return AUDCLNT_E_UNSUPPORTED_FORMAT;
    }
    desc->mSampleRate = fmt->ex.nSamplesPerSec;
    desc->mBytesPerPacket = fmt->ex.nBlockAlign;
    desc->mFramesPerPacket = 1;
    desc->mBytesPerFrame = fmt->ex.nBlockAlign;
    desc->mChannelsPerFrame = fmt->ex.nChannels;
    desc->mBitsPerChannel = fmt->ex.wBitsPerSample;
    desc->mReserved = 0;

    return S_OK;
}

static HRESULT ca_setup_audiounit(bool isRender, AudioComponentInstance unit, const BoxedWaveFormatExtensible* fmt, AudioStreamBasicDescription* dev_desc, AudioConverterRef* converter)
{
    OSStatus sc;
    HRESULT hr;

    if (!isRender) {
        AudioStreamBasicDescription desc;
        UInt32 size;
        Float64 rate;
        fenv_t fenv;
        bool fenv_stored = true;

        hr = ca_get_audiodesc(&desc, fmt);
        if (FAILED(hr))
            return hr;
        // dump_adesc("requested", &desc);

        /* input-only units can't perform sample rate conversion, so we have to
         * set up our own AudioConverter to support arbitrary sample rates. */
        size = sizeof(*dev_desc);
        sc = AudioUnitGetProperty(unit, kAudioUnitProperty_StreamFormat,
            kAudioUnitScope_Input, 1, dev_desc, &size);
        if (sc != noErr) {
            kwarn("Couldn't get unit format: %x\n", (int)sc);
            return osstatus_to_hresult(sc);
        }
        //dump_adesc("hardware", dev_desc);

        rate = dev_desc->mSampleRate;
        *dev_desc = desc;
        dev_desc->mSampleRate = rate;

        //dump_adesc("final", dev_desc);
        sc = AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat,
            kAudioUnitScope_Output, 1, dev_desc, sizeof(*dev_desc));
        if (sc != noErr) {
            kwarn("Couldn't set unit format: %x\n", (int)sc);
            return osstatus_to_hresult(sc);
        }

        /* AudioConverterNew requires divide-by-zero SSE exceptions to be masked */
        if (feholdexcept(&fenv)) {
            kwarn("Failed to store fenv state\n");
            fenv_stored = FALSE;
        }

        sc = AudioConverterNew(dev_desc, &desc, converter);

        if (fenv_stored && fesetenv(&fenv))
            kwarn("Failed to restore fenv state\n");

        if (sc != noErr) {
            kwarn("Couldn't create audio converter: %x\n", (int)sc);
            return osstatus_to_hresult(sc);
        }
    }
    else {
        hr = ca_get_audiodesc(dev_desc, fmt);
        if (FAILED(hr))
            return hr;

        // dump_adesc("final", dev_desc);
        sc = AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, dev_desc, sizeof(*dev_desc));
        if (sc != noErr) {
            kwarn("Couldn't set format: %x\n", (int)sc);
            return osstatus_to_hresult(sc);
        }
    }

    return S_OK;
}

static void silence_buffer(KNativeAudioCoreAudioData *This, U8 *buffer, U32 frames)
{
    if((This->fmt.ex.wFormatTag == WAVE_FORMAT_PCM || (This->fmt.ex.wFormatTag == WAVE_FORMAT_EXTENSIBLE && This->fmt.SubFormat == CORE_AUDIO_KSDATAFORMAT_SUBTYPE_PCM)) && This->fmt.ex.wBitsPerSample == 8) {
        memset(buffer, 128, frames * This->fmt.ex.nBlockAlign);
    } else {
        memset(buffer, 0, frames * This->fmt.ex.nBlockAlign);
    }
}

/* CA is pulling data from us */
static OSStatus ca_render_cb(void *user, AudioUnitRenderActionFlags *flags, const AudioTimeStamp *ts, UInt32 bus, UInt32 nframes, AudioBufferList *data)
{
    KNativeAudioCoreAudioData* This = (KNativeAudioCoreAudioData*)user;
    U32 to_copy_bytes, to_copy_frames, chunk_bytes, lcl_offs_bytes;
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
    std::shared_ptr<KProcess> process = This->process.lock();
    if (!process || process->terminated) {
        silence_buffer(This, ((U8 *)data->mBuffers[0].mData), nframes);
        return noErr;
    }
    KThread* thread = process->getThread();
    ChangeThread change(thread);
    os_unfair_lock_lock(&This->lock);

    if(This->isPlaying){
        U32 held_frames = process->memory->readd(This->address_held_frames);
        U32 lcl_offs_frames = process->memory->readd(This->address_lcl_offs_frames);
        
        lcl_offs_bytes = lcl_offs_frames * This->fmt.ex.nBlockAlign;
        to_copy_frames = std::min(nframes, held_frames);
        to_copy_bytes = to_copy_frames * This->fmt.ex.nBlockAlign;

        chunk_bytes = (This->bufsize_frames - lcl_offs_frames) * This->fmt.ex.nBlockAlign;

        if (to_copy_bytes > chunk_bytes) {
            process->memory->memcpy(data->mBuffers[0].mData, This->address_local_buffer + lcl_offs_bytes, chunk_bytes);
            process->memory->memcpy((U8*)data->mBuffers[0].mData + chunk_bytes, This->address_local_buffer, to_copy_bytes - chunk_bytes);
        }
        else {
            process->memory->memcpy(data->mBuffers[0].mData, This->address_local_buffer + lcl_offs_bytes, to_copy_bytes);
        }
        
        lcl_offs_frames += to_copy_frames;
        lcl_offs_frames %= This->bufsize_frames;
        process->memory->writed(This->address_lcl_offs_frames, lcl_offs_frames);
        held_frames -= to_copy_frames;
        process->memory->writed(This->address_held_frames, held_frames);
    } else {
        to_copy_bytes = to_copy_frames = 0;
    }
    
    if(nframes > to_copy_frames) {
        silence_buffer(This, ((U8 *)data->mBuffers[0].mData) + to_copy_bytes, nframes - to_copy_frames);
    }
    if (This->eventFd) {
        KFileDescriptor* fd = process->getFileDescriptor(This->eventFd);
        if (fd) {
            U8 c = EVENT_MSG_DATA_READ;
            fd->kobject->writeNative(&c, 1);
        }
    }
    os_unfair_lock_unlock(&This->lock);
    return noErr;
}

U32 KNativeAudioCoreAudio::init(std::shared_ptr<KProcess> process, bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames) {
    KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
    OSStatus sc;
    
    if (!data) {
        return E_FAIL;
    }
    data->process = process;

    data->bufsize_frames = bufsizeFrames;
    data->period_frames = process->memory->readd(addressPeriodFrames);
    data->address_local_buffer = addressLocalBuffer;
    data->address_wri_offs_frames = addressWriOffsFrames;
    data->address_held_frames = addressHeldFrames;
    data->address_lcl_offs_frames = addressLclOffsFrames;
    data->fmt.read(process->memory, addressFmt);
    
	HRESULT hr = ca_setup_audiounit(isRender, data->unit, &data->fmt, &data->dev_desc, &data->converter);
	if (FAILED(hr)) {
		return hr;
	}

	if (!isRender) {
		AURenderCallbackStruct input;

		memset(&input, 0, sizeof(input));
		//input.inputProc = &ca_capture_cb;
		input.inputProcRefCon = data;

        sc = AudioUnitSetProperty(data->unit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Output, 1, &input, sizeof(input));
		if (sc != noErr) {
			kwarn("Couldn't set callback: %x\n", (int)sc);
			AudioConverterDispose(data->converter);
			data->converter = NULL;
			os_unfair_lock_unlock(&data->lock);
			return osstatus_to_hresult(sc);
		}
	}
	else {
		AURenderCallbackStruct input;

		memset(&input, 0, sizeof(input));
		input.inputProc = &ca_render_cb;
		input.inputProcRefCon = data;

        sc = AudioUnitSetProperty(data->unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(input));
		if (sc != noErr) {
			kwarn("Couldn't set callback: %x\n", (int)sc);
            os_unfair_lock_unlock(&data->lock);
			return osstatus_to_hresult(sc);
		}
	}

	sc = AudioUnitInitialize(data->unit);
	if (sc != noErr) {
		kwarn("Couldn't initialize: %x\n", (int)sc);
		if (data->converter) {
			AudioConverterDispose(data->converter);
			data->converter = NULL;
		}
		return osstatus_to_hresult(sc);
	}

	/* we play audio continuously because AudioOutputUnitStart sometimes takes
	 * a while to return */
	sc = AudioOutputUnitStart(data->unit);
	if (sc != noErr) {
		kwarn("Unit failed to start: %x\n", (int)sc);
		if (data->converter) {
			AudioConverterDispose(data->converter);
			data->converter = NULL;
		}
		return osstatus_to_hresult(sc);
	}
    return S_OK;
}

static HRESULT ca_get_max_stream_latency(KNativeAudioCoreAudioData* This, UInt32* max)
{
	AudioObjectPropertyAddress addr;
	AudioStreamID* ids;
	UInt32 size;
	OSStatus sc;
	int nstreams, i;

	addr.mScope = This->scope;
	addr.mElement = 0;
	addr.mSelector = kAudioDevicePropertyStreams;

	sc = AudioObjectGetPropertyDataSize(This->adevid, &addr, 0, NULL, &size);
	if (sc != noErr) {
		kwarn("Unable to get size for _Streams property: %x\n", (int)sc);
		return osstatus_to_hresult(sc);
	}

    nstreams = size / sizeof(AudioStreamID);
    ids = new AudioStreamID[nstreams];

	sc = AudioObjectGetPropertyData(This->adevid, &addr, 0, NULL, &size, ids);
	if (sc != noErr) {
		kwarn("Unable to get _Streams property: %x\n", (int)sc);
        delete[] ids;
		return osstatus_to_hresult(sc);
	}

	*max = 0;

	addr.mSelector = kAudioStreamPropertyLatency;
	for (i = 0; i < nstreams; ++i) {
		UInt32 latency;

		size = sizeof(latency);
		sc = AudioObjectGetPropertyData(ids[i], &addr, 0, NULL,
			&size, &latency);
		if (sc != noErr) {
			kwarn("Unable to get _Latency property: %x\n", (int)sc);
			continue;
		}

		if (latency > * max)
			*max = latency;
	}

    delete[] ids;

	return S_OK;
}

U32 KNativeAudioCoreAudio::getLatency(U32 boxedAudioId, U32* pLatency) {
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	UInt32 latency, stream_latency, size;
	AudioObjectPropertyAddress addr;
	OSStatus sc;
    HRESULT hr;
    
	addr.mScope = data->scope;
	addr.mSelector = kAudioDevicePropertyLatency;
	addr.mElement = 0;

	size = sizeof(latency);
	sc = AudioObjectGetPropertyData(data->adevid, &addr, 0, NULL, &size, &latency);
	if (sc != noErr) {
		kwarn("Couldn't get _Latency property: %x\n", (int)sc);
		return osstatus_to_hresult(sc);
	}

	hr = ca_get_max_stream_latency(data, &stream_latency);
	if (FAILED(hr)) {
		return hr;
	}

	latency += stream_latency;
	*pLatency = latency;
	return S_OK;
}

void KNativeAudioCoreAudio::lock(U32 boxedAudioId) {
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	os_unfair_lock_lock(&data->lock);
}

void KNativeAudioCoreAudio::unlock(U32 boxedAudioId) {
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	os_unfair_lock_unlock(&data->lock);
}

U32 KNativeAudioCoreAudio::isFormatSupported(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) {
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	AudioStreamBasicDescription dev_desc;
	AudioConverterRef converter;
	AudioComponentInstance unit;
    BoxedWaveFormatExtensible fmt;
    HRESULT hr;
    
    fmt.read(thread->memory, addressWaveFormat);
    
	unit = get_audiounit(data->isRender, data->adevid);

	converter = NULL;
	hr = ca_setup_audiounit(data->isRender, unit, &fmt, &dev_desc, &converter);
	AudioComponentInstanceDispose(unit);

	if (converter)
		AudioConverterDispose(converter);
	return hr;
}

static U32 ca_channel_layout_to_channel_mask(const AudioChannelLayout *layout)
{
    int i;
    U32 mask = 0;

    for (i = 0; i < layout->mNumberChannelDescriptions; ++i) {
        switch (layout->mChannelDescriptions[i].mChannelLabel) {
            default: klog("Unhandled channel 0x%x\n", layout->mChannelDescriptions[i].mChannelLabel); break;
            case kAudioChannelLabel_Left: mask |= SPEAKER_FRONT_LEFT; break;
            case kAudioChannelLabel_Mono:
            case kAudioChannelLabel_Center: mask |= SPEAKER_FRONT_CENTER; break;
            case kAudioChannelLabel_Right: mask |= SPEAKER_FRONT_RIGHT; break;
            case kAudioChannelLabel_LeftSurround: mask |= SPEAKER_BACK_LEFT; break;
            case kAudioChannelLabel_CenterSurround: mask |= SPEAKER_BACK_CENTER; break;
            case kAudioChannelLabel_RightSurround: mask |= SPEAKER_BACK_RIGHT; break;
            case kAudioChannelLabel_LFEScreen: mask |= SPEAKER_LOW_FREQUENCY; break;
            case kAudioChannelLabel_LeftSurroundDirect: mask |= SPEAKER_SIDE_LEFT; break;
            case kAudioChannelLabel_RightSurroundDirect: mask |= SPEAKER_SIDE_RIGHT; break;
            case kAudioChannelLabel_TopCenterSurround: mask |= SPEAKER_TOP_CENTER; break;
            case kAudioChannelLabel_VerticalHeightLeft: mask |= SPEAKER_TOP_FRONT_LEFT; break;
            case kAudioChannelLabel_VerticalHeightCenter: mask |= SPEAKER_TOP_FRONT_CENTER; break;
            case kAudioChannelLabel_VerticalHeightRight: mask |= SPEAKER_TOP_FRONT_RIGHT; break;
            case kAudioChannelLabel_TopBackLeft: mask |= SPEAKER_TOP_BACK_LEFT; break;
            case kAudioChannelLabel_TopBackCenter: mask |= SPEAKER_TOP_BACK_CENTER; break;
            case kAudioChannelLabel_TopBackRight: mask |= SPEAKER_TOP_BACK_RIGHT; break;
            case kAudioChannelLabel_LeftCenter: mask |= SPEAKER_FRONT_LEFT_OF_CENTER; break;
            case kAudioChannelLabel_RightCenter: mask |= SPEAKER_FRONT_RIGHT_OF_CENTER; break;
        }
    }

    return mask;
}

/* For most hardware on Windows, users must choose a configuration with an even
 * number of channels (stereo, quad, 5.1, 7.1). Users can then disable
 * channels, but those channels are still reported to applications from
 * GetMixFormat! Some applications behave badly if given an odd number of
 * channels (e.g. 2.1).  Here, we find the nearest configuration that Windows
 * would report for a given channel layout. */
static void convert_channel_layout(const AudioChannelLayout *ca_layout, BoxedWaveFormatExtensible *fmt)
{
    U32 ca_mask = ca_channel_layout_to_channel_mask(ca_layout);

    // klog("Got channel mask for CA: 0x%x\n", ca_mask);

    if (ca_layout->mNumberChannelDescriptions == 1)
    {
        fmt->ex.nChannels = 1;
        fmt->dwChannelMask = ca_mask;
        return;
    }

    /* compare against known configurations and find smallest configuration
     * which is a superset of the given speakers */

    if (ca_layout->mNumberChannelDescriptions <= 2 && (ca_mask & ~KSAUDIO_SPEAKER_STEREO) == 0)
    {
        fmt->ex.nChannels = 2;
        fmt->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
        return;
    }

    if (ca_layout->mNumberChannelDescriptions <= 4 && (ca_mask & ~KSAUDIO_SPEAKER_QUAD) == 0)
    {
        fmt->ex.nChannels = 4;
        fmt->dwChannelMask = KSAUDIO_SPEAKER_QUAD;
        return;
    }

    if (ca_layout->mNumberChannelDescriptions <= 4 && (ca_mask & ~KSAUDIO_SPEAKER_SURROUND) == 0)
    {
        fmt->ex.nChannels = 4;
        fmt->dwChannelMask = KSAUDIO_SPEAKER_SURROUND;
        return;
    }

    if (ca_layout->mNumberChannelDescriptions <= 6 && (ca_mask & ~KSAUDIO_SPEAKER_5POINT1) == 0)
    {
        fmt->ex.nChannels = 6;
        fmt->dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
        return;
    }

    if (ca_layout->mNumberChannelDescriptions <= 6 && (ca_mask & ~KSAUDIO_SPEAKER_5POINT1_SURROUND) == 0)
    {
        fmt->ex.nChannels = 6;
        fmt->dwChannelMask = KSAUDIO_SPEAKER_5POINT1_SURROUND;
        return;
    }

    if (ca_layout->mNumberChannelDescriptions <= 8 && (ca_mask & ~KSAUDIO_SPEAKER_7POINT1) == 0)
    {
        fmt->ex.nChannels = 8;
        fmt->dwChannelMask = KSAUDIO_SPEAKER_7POINT1;
        return;
    }

    if (ca_layout->mNumberChannelDescriptions <= 8 && (ca_mask & ~KSAUDIO_SPEAKER_7POINT1_SURROUND) == 0)
    {
        fmt->ex.nChannels = 8;
        fmt->dwChannelMask = KSAUDIO_SPEAKER_7POINT1_SURROUND;
        return;
    }

    /* oddball format, report truthfully */
    fmt->ex.nChannels = ca_layout->mNumberChannelDescriptions;
    fmt->dwChannelMask = ca_mask;
}

static U32 get_channel_mask(unsigned int channels)
{
    switch(channels){
    case 0:
        return 0;
    case 1:
        return KSAUDIO_SPEAKER_MONO;
    case 2:
        return KSAUDIO_SPEAKER_STEREO;
    case 3:
        return KSAUDIO_SPEAKER_STEREO | SPEAKER_LOW_FREQUENCY;
    case 4:
        return KSAUDIO_SPEAKER_QUAD;    /* not _SURROUND */
    case 5:
        return KSAUDIO_SPEAKER_QUAD | SPEAKER_LOW_FREQUENCY;
    case 6:
        return KSAUDIO_SPEAKER_5POINT1; /* not 5POINT1_SURROUND */
    case 7:
        return KSAUDIO_SPEAKER_5POINT1 | SPEAKER_BACK_CENTER;
    case 8:
        return KSAUDIO_SPEAKER_7POINT1_SURROUND; /* Vista deprecates 7POINT1 */
    }
    klog("Unknown speaker configuration: %u\n", channels);
    return 0;
}

U32 KNativeAudioCoreAudio::getMixFormat(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) {
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
    BoxedWaveFormatExtensible fmt;
	OSStatus sc;
	UInt32 size;
	Float64 rate;
	AudioBufferList* buffers;
	AudioChannelLayout* layout;
	AudioObjectPropertyAddress addr;

	fmt.ex.wFormatTag = WAVE_FORMAT_EXTENSIBLE;

	addr.mScope = data->scope;
	addr.mElement = 0;
	addr.mSelector = kAudioDevicePropertyPreferredChannelLayout;

    if (data->adevid == 0) {
        getDefaultAudioId(data->isRender, &data->adevid);
    }
	sc = AudioObjectGetPropertyDataSize(data->adevid, &addr, 0, NULL, &size);
	if (sc == noErr) {
		layout = (AudioChannelLayout*)malloc(size);

		sc = AudioObjectGetPropertyData(data->adevid, &addr, 0, NULL, &size, layout);
		if (sc == noErr) {
			klog("Got channel layout: {tag: 0x%x, bitmap: 0x%x, num_descs: %u}\n", layout->mChannelLayoutTag, layout->mChannelBitmap, layout->mNumberChannelDescriptions);

			if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions) {
				convert_channel_layout(layout, &fmt);
			}
			else {
				kwarn("Haven't implemented support for this layout tag: 0x%x, guessing at layout\n", layout->mChannelLayoutTag);
				fmt.ex.nChannels = 0;
			}
		}
		else {
			klog("Unable to get _PreferredChannelLayout property: %x, guessing at layout\n", (int)sc);
			fmt.ex.nChannels = 0;
		}

        ::free(layout);
	}
	else {
		kwarn("Unable to get size for _PreferredChannelLayout property: %x, guessing at layout\n", (int)sc);
		fmt.ex.nChannels = 0;
	}

	if (fmt.ex.nChannels == 0) {
		addr.mScope = data->scope;
		addr.mElement = 0;
		addr.mSelector = kAudioDevicePropertyStreamConfiguration;

		sc = AudioObjectGetPropertyDataSize(data->adevid, &addr, 0, NULL, &size);
		if (sc != noErr) {
			kwarn("Unable to get size for _StreamConfiguration property: %x\n", (int)sc);
			return osstatus_to_hresult(sc);
		}

        buffers = (AudioBufferList*)malloc(size);
		if (!buffers) {
			return E_OUTOFMEMORY;
		}

		sc = AudioObjectGetPropertyData(data->adevid, &addr, 0, NULL, &size, buffers);
		if (sc != noErr) {
            ::free(buffers);
			kwarn("Unable to get _StreamConfiguration property: %x\n", (int)sc);
			return osstatus_to_hresult(sc);
		}

		fmt.ex.nChannels = 0;
		for (int i = 0; i < buffers->mNumberBuffers; ++i)
			fmt.ex.nChannels += buffers->mBuffers[i].mNumberChannels;

        ::free(buffers);

		fmt.dwChannelMask = get_channel_mask(fmt.ex.nChannels);
	}

	addr.mSelector = kAudioDevicePropertyNominalSampleRate;
	size = sizeof(Float64);
	sc = AudioObjectGetPropertyData(data->adevid, &addr, 0, NULL, &size, &rate);
	if (sc != noErr) {
		kwarn("Unable to get _NominalSampleRate property: %x\n", (int)sc);
		return osstatus_to_hresult(sc);
	}
	fmt.ex.nSamplesPerSec = rate;

	fmt.ex.wBitsPerSample = 32;
	fmt.SubFormat = CORE_AUDIO_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

	fmt.ex.nBlockAlign = (fmt.ex.wBitsPerSample * fmt.ex.nChannels) / 8;
	fmt.ex.nAvgBytesPerSec = fmt.ex.nSamplesPerSec * fmt.ex.nBlockAlign;

	fmt.wValidBitsPerSample = fmt.ex.wBitsPerSample;
	fmt.ex.cbSize = 22;

    fmt.write(thread->memory, addressWaveFormat);
	return S_OK;
}

void KNativeAudioCoreAudio::setVolume(U32 boxedAudioId, float level, U32 channel) {
	OSStatus sc;
	KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	sc = AudioUnitSetParameter(data->unit, kHALOutputParam_Volume, kAudioUnitScope_Global, 0, level, 0);
	if (sc != noErr)
		kwarn("Couldn't set volume: %x\n", (int)sc);
}

void KNativeAudioCoreAudio::start(U32 boxedAudioId, U32 eventFd) {
    KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
    if (!data) {
        return;
    }
    data->eventFd = eventFd;
    data->isPlaying = true;
}

void KNativeAudioCoreAudio::stop(U32 boxedAudioId) {
    KNativeAudioCoreAudioData* data = getDataFromId(boxedAudioId);
    if (!data) {
        return;
    }
    data->isPlaying = false;
}

#include "coremidi.h"
static MIDIClientRef wineMIDIClient = NULL;

#define MAXPNAMELEN 32
typedef struct midioutcaps_tag {
  U16    wMid;
  U16    wPid;
  U32    vDriverVersion;
  char16_t    szPname[MAXPNAMELEN];
  U16    wTechnology;
  U16    wVoices;
  U16    wNotes;
  U16    wChannelMask;
  U32    dwSupport;
} MIDIOUTCAPS;

typedef struct midiopendesc_tag {
  U32      hMidi;
  U32      dwCallback;
  U32      dwInstance;
  //DWORD_PTR      dnDevNode;
  //DWORD          cIds;
  //MIDIOPENSTRMID rgIds[1];
} MIDIOPENDESC;

typedef struct tagMIDIDestination {
    /* graph and synth are only used for MIDI Synth */
    AUGraph graph;
    AudioUnit synth;

    MIDIEndpointRef dest;

    MIDIOUTCAPS caps;
    MIDIOPENDESC midiDesc;
    U16 wFlags;
} MIDIDestination;

static MIDIPortRef MIDIOutPort = NULL;

#define CALLBACK_TYPEMASK 0x00070000

#define HIWORD(x) (((x) >> 16) & 0xffff)

#define MAX_MIDI_SYNTHS 1
#define MIDICAPS_VOLUME 1

#define MOD_MIDIPORT 1
#define MOD_SYNTH 2

#define MMSYSERR_NOERROR 0
#define MMSYSERR_ERROR 1
#define MMSYSERR_BADDEVICEID 2
#define MMSYSERR_ALLOCATED 4
#define MMSYSERR_NOTSUPPORTED 8
#define MMSYSERR_INVALFLAG 10
#define MMSYSERR_INVALPARAM 11

#define MIDIERR_UNPREPARED 64
#define MIDIERR_STILLPLAYING 65

#define MHDR_DONE 1
#define MHDR_PREPARED 2
#define MHDR_INQUEUE 4

MIDIDestination *destinations;

int SynthUnit_CreateDefaultSynthUnit(AUGraph *graph, AudioUnit *synth);
int SynthUnit_Initialize(AudioUnit synth, AUGraph graph);
int SynthUnit_Close(AUGraph graph);
int AudioUnit_GetVolume(AudioUnit au, float *left, float *right);
int AudioUnit_SetVolume(AudioUnit au, float left, float right);

#include <codecvt>
static void toWideChar(const char* pString, char16_t* pWideString, int len) {
    std::u16string utf16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(pString);

    utf16.copy(pWideString, len, 0);
}

static bool CoreAudio_MIDIInit()
{
    int i;
    char szPname[MAXPNAMELEN] = {0};

    ItemCount numDest = MIDIGetNumberOfDestinations();
    CFStringRef name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("wineMIDIClient.%d"), getpid());

    wineMIDIClient = CoreMIDI_CreateClient( name );
    if (!wineMIDIClient)
    {
        CFRelease(name);
        klog("can't create wineMIDIClient\n");
        return false;
    }
    CFRelease(name);

    MIDIOut_NumDevs = MAX_MIDI_SYNTHS;
    MIDIOut_NumDevs += numDest;

    destinations = (MIDIDestination*)calloc(MIDIOut_NumDevs, sizeof(MIDIDestination));

    if (numDest > 0)
    {
        name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("WineOutputPort.%u"), getpid());
        MIDIOutputPortCreate(wineMIDIClient, name, &MIDIOutPort);
        CFRelease(name);
    }

    /* initialise MIDI synths */
    for (i = 0; i < MAX_MIDI_SYNTHS; i++)
    {
        snprintf(szPname, sizeof(szPname), "CoreAudio MIDI Synth %d", i);
        toWideChar(szPname, destinations[i].caps.szPname, MAXPNAMELEN);

        destinations[i].caps.wTechnology = MOD_SYNTH;
        destinations[i].caps.wChannelMask = 0xFFFF;

        destinations[i].caps.wMid = 0x00FF;     /* Manufac ID */
        destinations[i].caps.wPid = 0x0001;     /* Product ID */
        destinations[i].caps.vDriverVersion = 0x0001;
        destinations[i].caps.dwSupport = MIDICAPS_VOLUME;
        destinations[i].caps.wVoices = 16;
        destinations[i].caps.wNotes = 16;
    }
    /* initialise available destinations */
    for (i = MAX_MIDI_SYNTHS; i < numDest + MAX_MIDI_SYNTHS; i++)
    {
        destinations[i].dest = MIDIGetDestination(i - MAX_MIDI_SYNTHS);

        CoreMIDI_GetObjectName(destinations[i].dest, szPname, sizeof(szPname));
        toWideChar(szPname, destinations[i].caps.szPname, MAXPNAMELEN);

        destinations[i].caps.wTechnology = MOD_MIDIPORT;
        destinations[i].caps.wChannelMask = 0xFFFF;

        destinations[i].caps.wMid = 0x00FF;     /* Manufac ID */
        destinations[i].caps.wPid = 0x0001;
        destinations[i].caps.vDriverVersion = 0x0001;
        destinations[i].caps.dwSupport = 0;
        destinations[i].caps.wVoices = 0;
        destinations[i].caps.wNotes = 0;
    }
    return true;
}

static void CoreAudio_MIDIRelease() {
    if (wineMIDIClient) MIDIClientDispose(wineMIDIClient); /* MIDIClientDispose will close all ports */
    free(destinations);
}

U32 KNativeAudioCoreAudio::midiOutOpen(KProcess* process, U32 wDevID, U32 lpDesc, U32 dwFlags, U32 fd) {
	MIDIDestination *dest;

    if (lpDesc == 0) {
        klog("KNativeAudioCoreAudio::midiOutOpen Invalid Parameter");
        return MMSYSERR_INVALPARAM;
    }

    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutOpen bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }

    if (destinations[wDevID].midiDesc.hMidi != 0) {
        klog("KNativeAudioCoreAudio::midiOutOpen device already open !");
        return MMSYSERR_ALLOCATED;
    }

    if ((dwFlags & ~CALLBACK_TYPEMASK) != 0) {
        klog("KNativeAudioCoreAudio::midiOutOpen bad dwFlags");
        return MMSYSERR_INVALFLAG;
    }
    dest = &destinations[wDevID];

    if (dest->caps.wTechnology == MOD_SYNTH)
    {
        if (!SynthUnit_CreateDefaultSynthUnit(&dest->graph, &dest->synth))
        {
            klog("KNativeAudioCoreAudio::midiOutOpen SynthUnit_CreateDefaultSynthUnit dest=%p failed", dest);
            return MMSYSERR_ERROR;
        }

        if (!SynthUnit_Initialize(dest->synth, dest->graph))
        {
            klog("KNativeAudioCoreAudio::midiOutOpen SynthUnit_Initialise dest=%p failed", dest);
            return MMSYSERR_ERROR;
        }
    }
    dest->wFlags = HIWORD(dwFlags & CALLBACK_TYPEMASK);
    dest->midiDesc.hMidi = process->memory->readd(lpDesc);
    dest->midiDesc.dwCallback = process->memory->readd(lpDesc+4);
    dest->midiDesc.dwInstance = process->memory->readd(lpDesc+8);
    
    // MIDI_NotifyClient(wDevID, MOM_OPEN, 0L, 0L);
    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutClose(U32 wDevID) {
    U32 ret = MMSYSERR_NOERROR;

    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutClose bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }

    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH) {
        SynthUnit_Close(destinations[wDevID].graph);
    }
    destinations[wDevID].graph = 0;
    destinations[wDevID].synth = 0;

    // MIDI_NotifyClient(wDevID, MOM_CLOSE, 0L, 0L);
    destinations[wDevID].midiDesc.hMidi = 0;

    return ret;
}

U32 KNativeAudioCoreAudio::midiOutData(U32 wDevID, U32 dwParam) {
    U32 evt = dwParam & 0xFF;
    UInt8 chn = (evt & 0x0F);

    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutData bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }

    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH)
    {
        U32 d1  = (dwParam >> 8) & 0xFF;
        U32 d2  = (dwParam >> 16) & 0xFF;
        OSStatus err = noErr;

        err = MusicDeviceMIDIEvent(destinations[wDevID].synth, (evt & 0xF0) | chn, d1, d2, 0);
        if (err != noErr)
        {
            klog("KNativeAudioCoreAudio::midiOutData MusicDeviceMIDIEvent(%p, %04x, %04x, %04x, %d) return %d", destinations[wDevID].synth, (evt & 0xF0) | chn, d1, d2, 0, err);
            return MMSYSERR_ERROR;
        }
    }
    else
    {
        UInt8 buffer[3];
        buffer[0] = (evt & 0xF0) | chn;
        buffer[1] = (dwParam >> 8) & 0xFF;
        buffer[2] = (dwParam >> 16) & 0xFF;

        MIDIOut_Send(MIDIOutPort, destinations[wDevID].dest, buffer, 3);
    }

    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutLongData(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
    OSStatus err = noErr;
    WineMidiHdr wineMidiHdr;
    
    /* Note: MS doc does not say much about the dwBytesRecorded member of the MIDIHDR structure
     * but it seems to be used only for midi input.
     * Taking a look at the WAVEHDR structure (which is quite similar) confirms this assumption.
     */

    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutLongData bad device ID : %d\n", wDevID);
        return MMSYSERR_BADDEVICEID;
    }

    if (lpMidiHdr == 0) {
        klog("KNativeAudioCoreAudio::midiOutLongData Invalid Parameter\n");
        return MMSYSERR_INVALPARAM;
    }
    wineMidiHdr.read(thread->memory, lpMidiHdr, dwSize);

    if (wineMidiHdr.lpData == 0) {
        return MIDIERR_UNPREPARED;
    }
    if (!(wineMidiHdr.dwFlags & MHDR_PREPARED)) {
        return MIDIERR_UNPREPARED;
    }
    if (wineMidiHdr.dwFlags & MHDR_INQUEUE) {
        return MIDIERR_STILLPLAYING;
    }
    wineMidiHdr.dwFlags &= ~MHDR_DONE;
    wineMidiHdr.dwFlags |= MHDR_INQUEUE;
    wineMidiHdr.writeFlags(thread->memory, lpMidiHdr);
    
    U8* buffer = (U8*)thread->memory->lockReadOnlyMemory(wineMidiHdr.lpData, wineMidiHdr.dwBufferLength);
    
    /* FIXME: MS doc is not 100% clear. Will lpData only contain system exclusive
     * data, or can it also contain raw MIDI data, to be split up and sent to
     * modShortData() ?
     * If the latter is true, then the following WARNing will fire up
     */
    if (buffer[0] != 0xF0 || buffer[wineMidiHdr.dwBufferLength - 1] != 0xF7) {
        klog("KNativeAudioCoreAudio::midiOutLongDataThe allegedly system exclusive buffer is not correct\n\tPlease report with MIDI file");
    }
    if (buffer[0] != 0xF0) {
        /* System Exclusive */
        klog("KNativeAudioCoreAudio::midiOutLongData Add missing 0xF0 marker at the beginning of system exclusive byte stream");
    }
    if (buffer[wineMidiHdr.dwBufferLength - 1] != 0xF7) {
        /* Send end of System Exclusive */
        klog("KNativeAudioCoreAudio::midiOutLongData Add missing 0xF7 marker at the end of system exclusive byte stream");
    }
    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH) /* FIXME */
    {
        err = MusicDeviceSysEx(destinations[wDevID].synth, (const UInt8 *) buffer, wineMidiHdr.dwBufferLength);
        if (err != noErr)
        {
            klog("KNativeAudioCoreAudio::midiOutLongData MusicDeviceSysEx(%p, %p, %d) return %d", destinations[wDevID].synth, wineMidiHdr.lpData, wineMidiHdr.dwBufferLength, err);
            thread->memory->unlockMemory(buffer);
            return MMSYSERR_ERROR;
        }
    }
    else if (destinations[wDevID].caps.wTechnology == MOD_MIDIPORT) {
        MIDIOut_Send(MIDIOutPort, destinations[wDevID].dest, buffer, wineMidiHdr.dwBufferLength);
    }

    wineMidiHdr.dwFlags &= ~MHDR_INQUEUE;
    wineMidiHdr.dwFlags |= MHDR_DONE;
    wineMidiHdr.writeFlags(thread->memory, lpMidiHdr);
    // MIDI_NotifyClient(wDevID, MOM_DONE, (DWORD_PTR)lpMidiHdr, 0L);
    thread->memory->unlockMemory(buffer);
    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutPrepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
    WineMidiHdr wineMidiHdr;
    
    if (!lpMidiHdr) {
        return MMSYSERR_INVALPARAM;
    }
    wineMidiHdr.read(thread->memory, lpMidiHdr, dwSize);
    if (wineMidiHdr.lpData == 0) {
        return MMSYSERR_INVALPARAM;
    }
    if (wineMidiHdr.dwFlags & MHDR_PREPARED) {
        return MMSYSERR_NOERROR;
    }

    wineMidiHdr.lpNext = 0;
    wineMidiHdr.dwFlags |= MHDR_PREPARED;
    wineMidiHdr.dwFlags &= ~(MHDR_DONE|MHDR_INQUEUE); /* flags cleared since w2k */
    wineMidiHdr.write(thread->memory, lpMidiHdr, dwSize);
    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutUnprepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
    WineMidiHdr wineMidiHdr;
    
    if (!lpMidiHdr) {
        return MMSYSERR_INVALPARAM;
    }
    wineMidiHdr.read(thread->memory, lpMidiHdr, dwSize);
    if (wineMidiHdr.lpData == 0) {
        return MMSYSERR_INVALPARAM;
    }
    if (!(wineMidiHdr.dwFlags & MHDR_PREPARED)) {
        return MMSYSERR_NOERROR;
    }
    if (wineMidiHdr.dwFlags & MHDR_INQUEUE) {
        return MIDIERR_STILLPLAYING;
    }

    wineMidiHdr.dwFlags &= ~MHDR_PREPARED;
    wineMidiHdr.writeFlags(thread->memory, lpMidiHdr);
    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutGetDevCaps(KThread* thread, U32 wDevID, U32 lpCaps, U32 dwSize) {
    if (lpCaps == 0) {
        klog("KNativeAudioCoreAudio::midiOutGetDevCaps Invalid Parameter\n");
        return MMSYSERR_INVALPARAM;
    }

    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutGetDevCapsbad device ID : %d\n", wDevID);
        return MMSYSERR_BADDEVICEID;
    }
    thread->memory->memcpy(lpCaps, &destinations[wDevID].caps, std::min(dwSize, (U32)sizeof(destinations[wDevID].caps)));
    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiOutGetNumDevs() {
    return MIDIOut_NumDevs;
}

U32 KNativeAudioCoreAudio::midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) {
    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutGetVolume bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }
    if (lpdwVolume == 0) {
        klog("KNativeAudioCoreAudio::midiOutGetVolume Invalid Parameter");
        return MMSYSERR_INVALPARAM;
    }

    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH)
    {
        float left;
        float right;
        AudioUnit_GetVolume(destinations[wDevID].synth, &left, &right);

        thread->memory->writed(lpdwVolume, (U32) (left * 0xFFFF) + ((U32) (right * 0xFFFF) << 16));

        return MMSYSERR_NOERROR;
    }

    return MMSYSERR_NOTSUPPORTED;
}

U32 KNativeAudioCoreAudio::midiOutSetVolume(U32 wDevID, U32 dwVolume) {
    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutSetVolume bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }
    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH)
    {
        float left;
        float right;

        left  = (dwVolume & 0xFFFF) / 65535.0f;
        right = ((dwVolume >> 16) & 0xFFFF) / 65535.0f;
        AudioUnit_SetVolume(destinations[wDevID].synth, left, right);

        return MMSYSERR_NOERROR;
    }

    return MMSYSERR_NOTSUPPORTED;
}

U32 KNativeAudioCoreAudio::midiOutReset(U32 wDevID) {
    unsigned chn;
    
    if (wDevID >= MIDIOut_NumDevs) {
        klog("KNativeAudioCoreAudio::midiOutReset bad device ID : %d", wDevID);
        return MMSYSERR_BADDEVICEID;
    }
    if (destinations[wDevID].caps.wTechnology == MOD_SYNTH)
    {
        for (chn = 0; chn < 16; chn++) {
            /* turn off every note */
            MusicDeviceMIDIEvent(destinations[wDevID].synth, 0xB0 | chn, 0x7B, 0, 0);
            /* remove sustain on channel */
            MusicDeviceMIDIEvent(destinations[wDevID].synth, 0xB0 | chn, 0x40, 0, 0);
        }
    } else {
        //FIXME("MOD_MIDIPORT\n");
    }

    /* FIXME: the LongData buffers must also be returned to the app */
    return MMSYSERR_NOERROR;
}

U32 KNativeAudioCoreAudio::midiInOpen(U32 wDevID, U32 lpDesc, U32 dwFlags) {
	// return MIDIIn_Open(wDevID, (LPMIDIOPENDESC)dwParam1, dwParam2);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInClose(U32 wDevID) {
	// return MIDIIn_Close(wDevID);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInAddBuffer(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	// return MIDIIn_AddBuffer(wDevID, (LPMIDIHDR)dwParam1, dwParam2);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	// return MIDIIn_Prepare(wDevID, (LPMIDIHDR)dwParam1, dwParam2);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	// return MIDIIn_Unprepare(wDevID, (LPMIDIHDR)dwParam1, dwParam2);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) {
	// return MIDIIn_GetDevCaps(wDevID, (LPMIDIINCAPSW) dwParam1, dwParam2);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInGetNumDevs() {
	// return MIDIIn_GetNumDevs();
    return 0;
}

U32 KNativeAudioCoreAudio::midiInStart(U32 wDevID) {
	// return MIDIIn_Start(wDevID);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInStop(U32 wDevID) {
	// return MIDIIn_Stop(wDevID);
    return E_FAIL;
}

U32 KNativeAudioCoreAudio::midiInReset(U32 wDevID) {
	// return MIDIIn_Reset(wDevID);
    return E_FAIL;
}

void KNativeAudioCoreAudio::cleanup() {
    
}
void initCoreAudio() {
    KNativeAudio::availableAudio.push_back(std::make_shared<KNativeAudioCoreAudio>());
}

#endif
