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
	KProcessPtr process = data->process.lock();	

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

	KFileDescriptor* fd = process->getFileDescriptor(data->callbackFD);
	if (!fd) {
		memset(stream, data->got.silence, len);
	} else {
		U8 b = 1;		

		if (data->sameFormat) {
			U32 frames = len / data->fmt.nBlockAlign;
			if (data->emulatedAddressSize && (len > data->emulatedAddressSize || data->nativeAddress != stream)) {
				process->memory->unmapNativeMemory(data->emulatedAddress, data->emulatedAddressSize);
				data->emulatedAddressSize = 0;
				data->emulatedAddress = 0;
				data->nativeAddress = nullptr;
			}
			if (!data->emulatedAddressSize) {
				data->emulatedAddress = process->memory->mapNativeMemory(stream, len);
				data->emulatedAddressSize = len;
				data->nativeAddress = stream;
				klog("audio buffer mapped to %x len=%x", data->emulatedAddress, data->emulatedAddressSize);
			}
			fd->kobject->writeNative(&b, 1);
			fd->kobject->writeNative((U8*)&data->callbackAddress, 4);
			fd->kobject->writeNative((U8*)&frames, 4);
			fd->kobject->writeNative((U8*)&data->emulatedAddress, 4);
			fd->kobject->readNative(&b, 1); // this will signal that the data is ready
		} else {
			data->cvt.len = len / data->cvt.len_mult; // amount to request
			U32 bufSize = data->cvt.len * data->cvt.len_mult; // amount after conversion
			if (bufSize > data->cvtBufSize) {
				if (data->cvtBuf) {
					delete[] data->cvtBuf;
				}
				data->cvtBuf = new U8[bufSize];
				data->cvtBufSize = bufSize;
			}
			U32 frames = data->cvt.len / data->fmt.nBlockAlign; // frames to request from wine
			if (data->emulatedAddressSize && (len > data->emulatedAddressSize || data->nativeAddress != data->cvtBuf)) {
				process->memory->unmapNativeMemory(data->emulatedAddress, data->emulatedAddressSize);
				data->emulatedAddressSize = 0;
				data->emulatedAddress = 0;
				data->nativeAddress = nullptr;
			}
			if (!data->emulatedAddressSize) {
				data->emulatedAddress = process->memory->mapNativeMemory(data->cvtBuf, data->cvtBufSize);
				data->emulatedAddressSize = data->cvtBufSize;
				data->nativeAddress = data->cvtBuf;
				klog("audio buffer mapped to %x len=%x", data->emulatedAddress, data->emulatedAddressSize);
			}

			fd->kobject->writeNative(&b, 1);
			fd->kobject->writeNative((U8*)&data->callbackAddress, 4);
			fd->kobject->writeNative((U8*)&frames, 4);
			fd->kobject->writeNative((U8*)&data->emulatedAddress, 4);
			fd->kobject->readNative(&b, 1); // this will signal that the data is ready

			data->cvt.buf = data->cvtBuf;
			SDL_ConvertAudio(&data->cvt);
			memcpy(stream, data->cvt.buf, data->cvt.len_cvt);
		}
		if (!KSystem::soundEnabled) {
			// let the above process, since it will properly remove data from audioBuffer, the app can request the status of that buffer
			memset(stream, data->got.silence, len);
		}
	}
}

U32 KNativeSDLAudioData::nextId;

KNativeSDLAudioData::~KNativeSDLAudioData() {
    if (cvtBuf) {
        delete[] cvtBuf;
    }
    if (emulatedAddress && emulatedAddressSize) {
        KProcessPtr p = process.lock();
        if (p) {
            p->memory->unmapNativeMemory(emulatedAddress, emulatedAddressSize);
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

void KNativeAudioSDL::start(U32 boxedAudioId) {
	KNativeSDLAudioDataPtr data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	data->isPlaying = true;
}

void KNativeAudioSDL::stop(U32 boxedAudioId) {
	KNativeSDLAudioDataPtr data = getDataFromId(boxedAudioId);
	if (!data) {
		return;
	}
	data->isPlaying = false;
}

U32 KNativeAudioSDL::getEndPoint(bool isRender) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(dataMutex);
	KNativeSDLAudioDataPtr data = std::make_shared<KNativeSDLAudioData>();
	data->isRender = isRender;
	data->id = ++KNativeSDLAudioData::nextId;
	this->data.set(data->id, data);
	return data->id;
}

void KNativeAudioSDL::release(U32 boxedAudioId) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(dataMutex);
	data.remove(boxedAudioId);

	if (playing && playing->id == boxedAudioId) {
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		playing = nullptr;
	}
}

void KNativeAudioSDL::captureResample(U32 boxedAudioId) {	
}

U32 KNativeAudioSDL::getSdlFormat(BoxedWaveFormatEx* pFmt) {
	if (pFmt->wFormatTag == WAVE_FORMAT_PCM) {
		if (pFmt->wBitsPerSample == 8) {
			return AUDIO_U8;
		} else if (pFmt->wBitsPerSample == 16) {
			return AUDIO_S16LSB;
		} else if (pFmt->wBitsPerSample == 32) {
			return AUDIO_S32LSB;
		}
		return 0;
	} else if (pFmt->wFormatTag == WAVE_FORMAT_IEEE_FLOAT){
		if (pFmt->wBitsPerSample == 32) {
			return AUDIO_F32LSB;
		} 
		return 0;
	} else {
		return 0;
	}
}

U32 KNativeAudioSDL::init(KProcessPtr process, U32 boxedAudioId) {
	KNativeSDLAudioDataPtr data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}	
	data->process = process;

	data->want.callback = audioCallback;
	data->want.format = getSdlFormat(&data->fmt);
	data->want.freq = data->fmt.nSamplesPerSec;
	data->want.channels = (Uint8)data->fmt.nChannels;
	data->want.userdata = data.get();
	data->want.samples = 0;

	if (data->want.format == 0) {
		// wine will call this to probe supported formats
		return E_FAIL;
	}

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

	data->open = true;
	SDL_PauseAudio(0);
	playing = data;
	klog("openAudio: freq=%d(got %d) format=%x(got %x) channels=%d(got %d)", data->got.freq, data->got.freq, data->want.format, data->got.format, data->want.channels, data->got.channels);
	return S_OK;
}

void KNativeAudioSDL::setCallback(U32 boxedAudioId, U32 callbackFD, U32 callbackAddress) {
	KNativeSDLAudioDataPtr data = getDataFromId(boxedAudioId);
	if (data) {
		data->callbackFD = callbackFD;
		data->callbackAddress = callbackAddress;
	}
}

void KNativeAudioSDL::setFormat(U32 boxedAudioId, BoxedWaveFormatEx* format) {
	KNativeSDLAudioDataPtr data = getDataFromId(boxedAudioId);
	if (data) {
		data->fmt = *format;
	}
}

U32 KNativeAudioSDL::getLatency(U32 boxedAudioId, U32* pLatency) {
	KNativeSDLAudioDataPtr data = getDataFromId(boxedAudioId);
	if (!data) {
		return E_FAIL;
	}
	*pLatency = data->got.samples*2; // sdl audio is double buffered
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

KNativeSDLAudioDataPtr KNativeAudioSDL::getDataFromId(U32 boxedAudioId) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(dataMutex);
	return data.get(boxedAudioId);
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
