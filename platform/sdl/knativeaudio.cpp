#include "boxedwine.h"
#include "knativeaudio.h"

#include <SDL.h>
#include "knativeaudiosdl.h"

#define S_OK 0
#define E_FAIL 0x80004005

#define WAVE_FORMAT_EXTENSIBLE                 0xFFFE
#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_IEEE_FLOAT 3

static const BoxedGUID SDL_KSDATAFORMAT_SUBTYPE_PCM(0x00000001, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
static const BoxedGUID SDL_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT(0x00000003, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

static bool sdlAudioOpen = false;
std::vector< std::shared_ptr<KNativeAudio> > KNativeAudio::availableAudio;

static void closeSdlAudio() {
	if (sdlAudioOpen) {
		sdlAudioOpen = false;
		SDL_PauseAudio(1);
		SDL_CloseAudio();
	}
}

static void audioCallback(void* userdata, U8* stream, S32 len) {
	KNativeSDLAudioData* data = (KNativeSDLAudioData*)userdata;
	std::shared_ptr<KProcess> process = data->process.lock();	

	if (!data->isPlaying || !process || process->terminated) {
		memset(stream, data->got.silence, len);
		return;
	}
	KThread* thread = nullptr;
	process->iterateThreads([&thread](KThread* t) {
		thread = t;
		return false;
		});
	// some memory pages that might be writen to could be copy on write, in that case KThread::currentThread() will be used to get the current KMemory that should be used.
	// we don't actually care which thread we use, hopefully the thread won't go away in the middle.
	// perhaps in the future this could be a shared_ptr or something else to guarantee the thread will be there
	ChangeThread changeThread(thread);
	KMemory* memory = process->memory;	
	U32 blockAlign = 0;
	if (data->sameFormat) {
		blockAlign = data->fmt.ex.nBlockAlign;
	} else {
		blockAlign = data->fmt.ex.nBlockAlign * data->cvt.len_mult;
	}
	U32 nframes = len / blockAlign;
	if (!process->memory->canRead(data->address_lcl_offs_frames, 4)) {
		memset(stream, data->got.silence, len);
		return;
	}
	U32 lcl_offs_frames = process->memory->readd(data->address_lcl_offs_frames);
	U32 held_frames = process->memory->readd(data->address_held_frames);

	U32 lcl_offs_bytes = lcl_offs_frames * data->fmt.ex.nBlockAlign;
	U32 to_copy_frames = nframes < held_frames ? nframes : held_frames;
	U32 to_copy_bytes = to_copy_frames * data->fmt.ex.nBlockAlign;

	U32 chunk_bytes = (data->bufsize_frames - lcl_offs_frames) * data->fmt.ex.nBlockAlign;

	if (data->sameFormat) {
		if (to_copy_bytes > chunk_bytes) {
			memory->memcpy(stream, data->address_local_buffer + lcl_offs_bytes, chunk_bytes);
			memory->memcpy(stream + chunk_bytes, data->address_local_buffer, to_copy_bytes - chunk_bytes);
		} else {
			memory->memcpy(stream, data->address_local_buffer + lcl_offs_bytes, to_copy_bytes);
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
			memory->memcpy(data->cvtBuf, data->address_local_buffer + lcl_offs_bytes, chunk_bytes);
			memory->memcpy(data->cvtBuf + chunk_bytes, data->address_local_buffer, to_copy_bytes - chunk_bytes);
		}
		else {
			memory->memcpy(data->cvtBuf, data->address_local_buffer + lcl_offs_bytes, to_copy_bytes);
		}
		data->cvt.buf = data->cvtBuf;
		SDL_ConvertAudio(&data->cvt);
		memcpy(stream, data->cvt.buf, data->cvt.len_cvt);
		stream += data->cvt.len_cvt;
	}
	lcl_offs_frames += to_copy_frames;
	lcl_offs_frames %= data->bufsize_frames;
	process->memory->writed(data->address_lcl_offs_frames, lcl_offs_frames);
	held_frames -= to_copy_frames;
	process->memory->writed(data->address_held_frames, held_frames);
	if (nframes > to_copy_frames) {
		memset(stream, data->got.silence, (nframes - to_copy_frames) * blockAlign);
	}
	if (data->eventFd) {
		KFileDescriptor* fd = process->getFileDescriptor(data->eventFd);
		if (fd) {
			U8 c = EVENT_MSG_DATA_READ;
			fd->kobject->writeNative(&c, 1);
		}
	}
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

void KNativeAudioSDL::start(U32 boxedAudioId, U32 eventFd) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	data->eventFd = eventFd;
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
}

void KNativeAudioSDL::captureResample(U32 boxedAudioId) {	
}

U32 KNativeAudioSDL::getSdlFormat(BoxedWaveFormatExtensible* pFmt) {
	if ((pFmt->ex.wFormatTag == WAVE_FORMAT_EXTENSIBLE && pFmt->SubFormat == SDL_KSDATAFORMAT_SUBTYPE_PCM) || pFmt->ex.wFormatTag == WAVE_FORMAT_PCM) {
		if (pFmt->ex.wBitsPerSample == 8) {
			return AUDIO_U8;
		} else if (pFmt->ex.wBitsPerSample == 16) {
			return AUDIO_S16LSB;
		} else if (pFmt->ex.wBitsPerSample == 32) {
			return AUDIO_S32LSB;
		}
		return 0;
	} else if ((pFmt->ex.wFormatTag == WAVE_FORMAT_EXTENSIBLE && pFmt->SubFormat == SDL_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) || pFmt->ex.wFormatTag == WAVE_FORMAT_IEEE_FLOAT){
		if (pFmt->ex.wBitsPerSample == 32) {
			return AUDIO_F32LSB;
		} 
		return 0;
	} else {
		return 0;
	}
}

U32 KNativeAudioSDL::init(std::shared_ptr<KProcess> process, bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
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

	data->want.callback = audioCallback;
	data->want.format = getSdlFormat(&data->fmt);
	data->want.freq = data->fmt.ex.nSamplesPerSec;
	data->want.channels = (Uint8)data->fmt.ex.nChannels;
	data->want.userdata = data;
	data->want.samples = 0;

	if (!KSystem::soundEnabled) {
		data->sameFormat = true;
	} else {
		// If the previous audio is still playing, it will get cut off.  If I find a game that needs this, then perhaps I should think of a mixer.
		closeSdlAudio();
		if (SDL_OpenAudio(&data->want, &data->got) < 0) {
			klog("Failed to open audio: %s", SDL_GetError());
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
	klog("openAudio: freq=%d(got %d) format=%x(got %x) channels=%d(got %d)", data->got.freq, data->got.freq, data->want.format, data->got.format, data->want.channels, data->got.channels);
	return S_OK;
}

U32 KNativeAudioSDL::getLatency(U32 boxedAudioId, U32* pLatency) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	*pLatency = data->got.samples*2; // sdl audio is double buffered
	return S_OK;
}

void KNativeAudioSDL::lock(U32 boxedAudioId) {
	SDL_LockAudio();
}

void KNativeAudioSDL::unlock(U32 boxedAudioId) {
	SDL_UnlockAudio();
}

U32 KNativeAudioSDL::isFormatSupported(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	BoxedWaveFormatExtensible fmt;
	fmt.read(thread->memory, addressWaveFormat);
	if (getSdlFormat(&fmt) != 0) {
		return S_OK;
	}
	return E_FAIL;
}

U32 KNativeAudioSDL::getMixFormat(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) {
	KNativeSDLAudioData* data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	BoxedWaveFormatExtensible fmt;
	fmt.ex.cbSize = 22;
	fmt.ex.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	fmt.ex.nChannels = 2;
	fmt.ex.wBitsPerSample = 16;
	fmt.ex.nSamplesPerSec = 44100;
	fmt.ex.nBlockAlign = (fmt.ex.wBitsPerSample * fmt.ex.nChannels) / 8;
	fmt.ex.nAvgBytesPerSec = fmt.ex.nSamplesPerSec * fmt.ex.nBlockAlign;
	fmt.wValidBitsPerSample = fmt.ex.wBitsPerSample;
	fmt.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
	fmt.SubFormat = SDL_KSDATAFORMAT_SUBTYPE_PCM;
	fmt.write(thread->memory, addressWaveFormat);
	return S_OK;
}

void KNativeAudioSDL::setVolume(U32 boxedAudioId, float level, U32 channel) {

}

U32 KNativeAudioSDL::midiOutOpen(KProcess* process, U32 wDevID, U32 lpDesc, U32 dwFlags, U32 fd) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutClose(U32 wDevID) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutData(U32 wDevID, U32 dwParam) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutLongData(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutPrepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutUnprepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutGetDevCaps(KThread* thread, U32 wDevID, U32 lpCaps, U32 dwSize) {
	return E_FAIL;
}

U32 KNativeAudioSDL::midiOutGetNumDevs() {
	return 0;
}

U32 KNativeAudioSDL::midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) {
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

void KNativeAudioSDL::cleanup() {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
}

std::shared_ptr<KNativeAudio> KNativeAudio::createNativeAudio() {
    // :TODO: maybe allow the user to choose in the future
	return KNativeAudio::availableAudio.front();
}

void KNativeAudio::shutdown() {
    KNativeAudio::availableAudio.front()->cleanup();
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
