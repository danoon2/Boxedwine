#include "boxedwine.h"
#include "knativeaudio.h"

#include <SDL.h>

#define S_OK 0
#define E_FAIL 0x80004005

#define WAVE_FORMAT_EXTENSIBLE                 0xFFFE
#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_IEEE_FLOAT 3

static const GUID SDL_KSDATAFORMAT_SUBTYPE_PCM(0x00000001, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
static const GUID SDL_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT(0x00000003, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

static bool sdlAudioOpen = false;
static void closeSdlAudio() {
	if (sdlAudioOpen) {
		sdlAudioOpen = false;
		SDL_PauseAudio(1);
		SDL_CloseAudio();
	}
}

class KNativeSDLAudioData {
public:
	KNativeSDLAudioData() : cvtBuf(0), cvtBufSize(0), sameFormat(false), open(false), isRender(false), isPlaying(false), adevid(0), cap_held_frames(0), resamp_bufsize_frames(0), resamp_buffer(0), cap_offs_frames(0), bufsize_frames(0), address_local_buffer(0), address_wri_offs_frames(0), address_held_frames(0), address_lcl_offs_frames(0), period_frames(0) {}
	~KNativeSDLAudioData() {
		if (resamp_buffer) {
			delete[] resamp_buffer;
		}
		if (cvtBuf) {
			delete[] cvtBuf;
		}
	}
	SDL_AudioSpec want;
	SDL_AudioSpec got;
	SDL_AudioCVT cvt;
	U8* cvtBuf;
	U32 cvtBufSize;

	bool sameFormat;
	bool open;
	std::shared_ptr<KProcess> process;

	bool isRender;
	bool isPlaying;
	U32 adevid;

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
	WaveFormatExtensible fmt; // read only, doesn't change
};

class KNativeAudioSDL : public KNativeAudio, public std::enable_shared_from_this<KNativeAudioSDL> {
public:
	virtual ~KNativeAudioSDL() {}
	virtual bool load();
	virtual void free();
	virtual bool open();
	virtual bool close();
	virtual void start(U32 boxedAudioId);
	virtual void stop(U32 boxedAudioId);
	virtual bool configure();
	virtual U32 hasDevice(bool isRender);
	virtual U32 getEndPoint(bool isRender, U32 adevid);
	virtual void release(U32 boxedAudioId);
	virtual void captureResample(U32 boxedAudioId);
	virtual U32 init(bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames);
	virtual U32 getLatency(U32 boxedAudioId, U32* latency);
	virtual void lock(U32 boxedAudioId);
	virtual void unlock(U32 boxedAudioId);
	virtual U32 isFormatSupported(U32 boxedAudioId, U32 addressWaveFormat);
	virtual U32 getMixFormat(U32 boxedAudioId, U32 addressWaveFormat);
	virtual void setVolume(U32 boxedAudioId, float level, U32 channel);

	virtual U32 midiOutOpen(U32 wDevID, U32 lpDesc, U32 dwFlags);
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

	virtual U32 midiInOpen(U32 wDevID, U32 lpDesc, U32 dwFlags);
	virtual U32 midiInClose(U32 wDevID);
	virtual U32 midiInAddBuffer(U32 wDevID, U32 lpMidiHdr, U32 dwSize);
	virtual U32 midiInPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize);
	virtual U32 midiInUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize);
	virtual U32 midiInGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize);
	virtual U32 midiInGetNumDevs();
	virtual U32 midiInStart(U32 wDevID);
	virtual U32 midiInStop(U32 wDevID);
	virtual U32 midiInReset(U32 wDevID);

	U32 getSdlFormat(WaveFormatExtensible* pFmt);

	KNativeSDLAudioData data[2];

	KNativeSDLAudioData& getData(bool isRender) {
		if (isRender) {
			return data[0];
		}
		return data[1];
	}

	KNativeSDLAudioData* getDataFromId(U32 boxedAudioId) {
		if (boxedAudioId == 1) {
			return &data[0];
		}
		else if (boxedAudioId == 2) {
			return &data[1];
		}
		return NULL;
	}
};

static void audioCallback(void* userdata, U8* stream, S32 len) {
	KNativeSDLAudioData* data = (KNativeSDLAudioData*)userdata;

	if (!data->isPlaying) {
		memset(stream, data->got.silence, len);
		return;
	}
	U32 blockAlign;
	if (data->sameFormat) {
		blockAlign = data->fmt.nBlockAlign;
	} else {
		blockAlign = data->fmt.nBlockAlign * data->cvt.len_mult;
	}
	BOXEDWINE_CONDITION_LOCK(KSystem::processesCond);
	if (data->process->terminated) {
		memset(stream, data->got.silence, len);
		BOXEDWINE_CONDITION_UNLOCK(KSystem::processesCond);
		return;
	}
	U32 nframes = len / blockAlign;
	U32 to_copy_bytes, to_copy_frames, chunk_bytes, lcl_offs_bytes;
	U32 lcl_offs_frames = data->process->readd(data->address_lcl_offs_frames);
	U32 held_frames = data->process->readd(data->address_held_frames);

	lcl_offs_bytes = lcl_offs_frames * data->fmt.nBlockAlign;
	to_copy_frames = nframes < held_frames ? nframes : held_frames;
	to_copy_bytes = to_copy_frames * data->fmt.nBlockAlign;

	chunk_bytes = (data->bufsize_frames - lcl_offs_frames) * data->fmt.nBlockAlign;

	if (data->sameFormat) {
		if (to_copy_bytes > chunk_bytes) {
			data->process->memcopyToNative(data->address_local_buffer + lcl_offs_bytes, stream, chunk_bytes);
			data->process->memcopyToNative(data->address_local_buffer, stream + chunk_bytes, to_copy_bytes - chunk_bytes);
		} else {
			data->process->memcopyToNative(data->address_local_buffer + lcl_offs_bytes, stream, to_copy_bytes);
		}
		stream += to_copy_bytes;
	} else {		
		data->cvt.len = to_copy_bytes;
		U32 bufSize = data->cvt.len * data->cvt.len_mult;
		if (bufSize > data->cvtBufSize) {
			if (data->cvtBuf) {
				delete[] data->cvtBuf;
			}
			data->cvtBuf = new U8[bufSize];
			data->cvtBufSize = bufSize;
		}
		if (to_copy_bytes > chunk_bytes) {
			data->process->memcopyToNative(data->address_local_buffer + lcl_offs_bytes, data->cvtBuf, chunk_bytes);
			data->process->memcopyToNative(data->address_local_buffer, data->cvtBuf + chunk_bytes, to_copy_bytes - chunk_bytes);
		}
		else {
			data->process->memcopyToNative(data->address_local_buffer + lcl_offs_bytes, data->cvtBuf, to_copy_bytes);
		}
		data->cvt.buf = data->cvtBuf;
		SDL_ConvertAudio(&data->cvt);
		memcpy(stream, data->cvt.buf, data->cvt.len_cvt);
		stream += data->cvt.len_cvt;
	}
	lcl_offs_frames += to_copy_frames;
	lcl_offs_frames %= data->bufsize_frames;
	data->process->writed(data->address_lcl_offs_frames, lcl_offs_frames);
	held_frames -= to_copy_frames;
	data->process->writed(data->address_held_frames, held_frames);
	if (nframes > to_copy_frames) {
		memset(stream, data->got.silence, (nframes - to_copy_frames) * blockAlign);
	}
	BOXEDWINE_CONDITION_UNLOCK(KSystem::processesCond);
}

bool KNativeAudioSDL::load() {
	// if (CoreAudio_MIDIInit() != DRV_SUCCESS)
	//	return false;

	return true;
}

void KNativeAudioSDL::free() {
	// CoreAudio_MIDIRelease();
}

bool KNativeAudioSDL::open() {
	return true;
}

bool KNativeAudioSDL::close() {
	return true;
}

void KNativeAudioSDL::start(U32 boxedAudioId) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	data->isPlaying = true;
}

void KNativeAudioSDL::stop(U32 boxedAudioId) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	data->isPlaying = false;
}

bool KNativeAudioSDL::configure() {
	// MessageBoxA(0, "CoreAudio driver!", "CoreAudio driver", MB_OK);
	return true;
}

U32 KNativeAudioSDL::hasDevice(bool isRender) {
	return S_OK;
}

U32 KNativeAudioSDL::getEndPoint(bool isRender, U32 adevid) {
	KNativeSDLAudioData& data = getData(isRender);
	data.isRender = isRender;
	data.adevid = adevid;
	return isRender ? 1 : 2;
}

void KNativeAudioSDL::release(U32 boxedAudioId) {
	int ii = 0;
}

void KNativeAudioSDL::captureResample(U32 boxedAudioId) {	
}

U32 KNativeAudioSDL::getSdlFormat(WaveFormatExtensible* pFmt) {
	if ((pFmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE && pFmt->SubFormat == SDL_KSDATAFORMAT_SUBTYPE_PCM) || pFmt->wFormatTag == WAVE_FORMAT_PCM) {
		if (pFmt->wBitsPerSample == 8) {
			return AUDIO_U8;
		} else if (pFmt->wBitsPerSample == 16) {
			return AUDIO_S16LSB;
		} else if (pFmt->wBitsPerSample == 32) {
			return AUDIO_S32LSB;
		}
		return 0;
	} else if ((pFmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE && pFmt->SubFormat == SDL_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) || pFmt->wFormatTag == WAVE_FORMAT_IEEE_FLOAT){
		if (pFmt->wBitsPerSample == 32) {
			return AUDIO_F32LSB;
		} 
		return 0;
	} else {
		return 0;
	}
	
	kpanic("KNativeAudioSdl Unknow audio format");
	return 0;
}

U32 KNativeAudioSDL::init(bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	data->process = KThread::currentThread()->process;

	data->bufsize_frames = bufsizeFrames;
	data->period_frames = readd(addressPeriodFrames);
	data->address_local_buffer = addressLocalBuffer;
	data->address_wri_offs_frames = addressWriOffsFrames;
	data->address_held_frames = addressHeldFrames;
	data->address_lcl_offs_frames = addressLclOffsFrames;
	data->fmt.read(addressFmt);

	data->want.callback = audioCallback;
	data->want.format = getSdlFormat(&data->fmt);
	data->want.freq = data->fmt.nSamplesPerSec;
	data->want.channels = (Uint8)data->fmt.nChannels;
	data->want.userdata = data;
	data->want.samples = 0;

	if (!KSystem::soundEnabled) {
		data->sameFormat = true;
	}
	else if (!sdlAudioOpen) {
		// If the previous audio is still playing, it will get cut off.  If I find a game that needs this, then perhaps I should think of a mixer.
		closeSdlAudio();
		if (SDL_OpenAudio(&data->want, &data->got) < 0) {
			printf("Failed to open audio: %s\n", SDL_GetError());
		}		
		sdlAudioOpen = true;
		if (data->want.freq != data->got.freq || data->want.channels != data->got.channels || data->want.format != data->got.format) {
			data->sameFormat = false;
			SDL_BuildAudioCVT(&data->cvt, data->want.format, data->want.channels, data->want.freq, data->got.format, data->got.channels, data->got.freq);
		}
		else {
			data->sameFormat = true;
		}
	}
	data->open = true;
	if (KSystem::soundEnabled) {
		SDL_PauseAudio(0);
	}
	printf("openAudio: freq=%d(got %d) format=%x(got %x) channels=%d(got %d)\n", data->got.freq, data->got.freq, data->want.format, data->got.format, data->want.channels, data->got.channels);
	return S_OK;
}

U32 KNativeAudioSDL::getLatency(U32 boxedAudioId, U32* pLatency) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	*pLatency = data->got.samples; // sdl audio is double buffered
	return S_OK;
}

void KNativeAudioSDL::lock(U32 boxedAudioId) {
	SDL_LockAudio();
}

void KNativeAudioSDL::unlock(U32 boxedAudioId) {
	SDL_UnlockAudio();
}

U32 KNativeAudioSDL::isFormatSupported(U32 boxedAudioId, U32 addressWaveFormat) {
	WaveFormatExtensible fmt;
	fmt.read(addressWaveFormat);
	if (getSdlFormat(&fmt) != 0) {
		return S_OK;
	}
	return E_FAIL;
}

U32 KNativeAudioSDL::getMixFormat(U32 boxedAudioId, U32 addressWaveFormat) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	WaveFormatExtensible fmt;
	fmt.cbSize = 22;
	fmt.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	fmt.nChannels = 2;
	fmt.wBitsPerSample = 16;
	fmt.nSamplesPerSec = 44100;
	fmt.nBlockAlign = (fmt.wBitsPerSample * fmt.nChannels) / 8;
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;		
	fmt.wValidBitsPerSample = fmt.wBitsPerSample;
	fmt.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
	fmt.SubFormat = SDL_KSDATAFORMAT_SUBTYPE_PCM;
	fmt.write(addressWaveFormat);
	return S_OK;
}

void KNativeAudioSDL::setVolume(U32 boxedAudioId, float level, U32 channel) {

}

U32 KNativeAudioSDL::midiOutOpen(U32 wDevID, U32 lpDesc, U32 dwFlags) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutClose(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutData(U32 wDevID, U32 dwParam) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutLongData(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutGetNumDevs() {
	return 0;
}

U32 KNativeAudioSDL::midiOutGetVolume(U32 wDevID, U32 lpdwVolume) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutSetVolume(U32 wDevID, U32 dwVolume) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutReset(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInOpen(U32 wDevID, U32 lpDesc, U32 dwFlags) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInClose(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInAddBuffer(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInGetNumDevs() {
	return 0;
}

U32 KNativeAudioSDL::midiInStart(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInStop(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiInReset(U32 wDevID) {
	return E_FAIL;
}

std::shared_ptr<KNativeAudio> KNativeAudio::createNativeAudio() {
	return std::make_shared<KNativeAudioSDL>();
}

void KNativeAudio::shutdown() {
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}