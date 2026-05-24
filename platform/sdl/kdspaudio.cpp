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
#include "kdspaudio.h"
#include <SDL.h>
#include <algorithm>
#include "../../source/kernel/devs/oss.h"

#define DSP_BUFFER_SIZE (1024*32)

#ifndef __EMSCRIPTEN__
static bool sdlAudioOpen;
static U8 sdlSilence;

static void closeSdlAudio() {
	if (sdlAudioOpen) {
		sdlAudioOpen = false;
		SDL_PauseAudio(1);
		SDL_CloseAudio();
	}
}
#endif

class KDspAudioSdl : public KDspAudio, public std::enable_shared_from_this<KDspAudioSdl> {
public:
	KDspAudioSdl() {
		this->want.format = AUDIO_U8;
		this->want.channels = 1;
		this->want.freq = 11025;
#ifdef __EMSCRIPTEN__
		this->want.samples = 4096; //Must be pow of 2
#else
		this->want.samples = 5512;
#endif
		this->got.channels = 1;
		this->got.freq = 11025;
#ifdef __EMSCRIPTEN__
		this->got.samples = 4096; //Must be pow of 2
#else
		this->got.samples = 5512;
#endif
	}

	virtual ~KDspAudioSdl() {
		if (this->cvtBuf) {
			SDL_free(this->cvtBuf);
		}
		if (this->stream) {
			SDL_FreeAudioStream(this->stream);
		}
		if (this->deviceId) {
			SDL_CloseAudioDevice(this->deviceId);
			this->deviceId = 0;
		}
	}

	void openAudio(U32 format, U32 freq, U32 channels) override;
	void soundEnabled() override;
	bool isOpen() override { return this->open; }
	void closeAudio() override;
	U32 writeAudio(U8* data, U32 len) override;
	U32 getFragmentSize() override {return this->dspFragSize;}
	void setFragmentSize(U32 size) override;
	U32 getBufferSize() override {
#ifdef __EMSCRIPTEN__
		return this->getGuestQueuedAudioSizeWant();
#else
		return 0;
#endif
	}
	U32 getBufferCapacity() override { return DSP_BUFFER_SIZE;}

#ifndef __EMSCRIPTEN__
	void onClose();
	void closeAudioFromAudioThread();
#endif

	U32 bytesPerSampleWant() {
		return SDL_AUDIO_BITSIZE(this->want.format) / 8;
	}

	U32 bytesPerSampleGot() {
		return SDL_AUDIO_BITSIZE(this->got.format) / 8;
	}

	U32 bytesPerSecondWant() {
		return this->want.freq * this->want.channels * bytesPerSampleWant();
	}

	U32 bytesPerSecondGot() {
		return this->got.freq * this->got.channels * bytesPerSampleGot();
	}

	U32 getQueuedAudioSizeWant() {
		if (!KSystem::soundEnabled) {
			return (U32)this->audioBuffer.size();
		}
		if (!this->deviceId) {
			return 0;
		}
		U32 gotBytesPerSecond = bytesPerSecondGot();
		if (!gotBytesPerSecond) {
			return 0;
		}
		return (U32)(((U64)SDL_GetQueuedAudioSize(this->deviceId) * bytesPerSecondWant()) / gotBytesPerSecond);
	}

	U32 getGuestQueuedAudioSizeWant() {
#ifdef __EMSCRIPTEN__
		return this->getEstimatedRealQueuedWant();
#else
		return this->getQueuedAudioSizeWant();
#endif
	}

	U32 getSdlFormat(U32 format) {
		switch (format) {
		case AFMT_MU_LAW:
		case AFMT_A_LAW:
		case AFMT_IMA_ADPCM:
		case AFMT_U8:
			return AUDIO_U8;
		case AFMT_S16_LE:
			return AUDIO_S16LSB;
		case AFMT_S16_BE:
			return AUDIO_S16MSB;
		case AFMT_S8:
			return AUDIO_S8;
		case AFMT_U16_LE:
			return AUDIO_U16LSB;
		case AFMT_U16_BE:
			return AUDIO_U16MSB;
		case AFMT_MPEG:
			return AUDIO_U8;
        case AFMT_FLOAT:
                return AUDIO_F32LSB;
		default:
			kpanic_fmt("KNativeAudioSdl Unknow audio format %d", format);
			return 0;
		}
	}
	SDL_AudioSpec want = { 0 };
	SDL_AudioSpec got = { 0 };
	SDL_AudioCVT cvt = { 0 };
	SDL_AudioStream* stream = nullptr;
	U32 openedFormat = 0;
	int cvtBufLen = 0;
#ifndef __EMSCRIPTEN__
	int cvtBufPos = 0;
#endif
	unsigned char* cvtBuf = nullptr;
	std::vector<U8> streamBuffer;
	bool sameFormat = false;
	U32 dspFragSize = 4096;
	bool open = false;
	std::deque<U8> audioBuffer;
	SDL_AudioDeviceID deviceId = 0;
#ifndef __EMSCRIPTEN__
	bool closeWhenDone = false;
#endif
	U32 realQueuedWant = 0;
	U32 lastRealQueuedTime = 0;
	std::vector<U8> silenceBuffer;

	U32 getEstimatedRealQueuedWant() {
		U32 now = KSystem::getMilliesSinceStart();
		if (!this->lastRealQueuedTime) {
			this->lastRealQueuedTime = now;
			return this->realQueuedWant;
		}
		U32 elapsed = now - this->lastRealQueuedTime;
		if (elapsed) {
			U32 consumed = (U32)(((U64)bytesPerSecondWant() * elapsed) / 1000);
			this->realQueuedWant = consumed >= this->realQueuedWant ? 0 : this->realQueuedWant - consumed;
			this->lastRealQueuedTime = now;
		}
		return this->realQueuedWant;
	}

	void addRealQueuedWant(U32 bytes) {
		U32 queued = getEstimatedRealQueuedWant();
		this->realQueuedWant = std::min((U32)DSP_BUFFER_SIZE, queued + bytes);
	}

	U32 topUpSilence() {
#ifdef __EMSCRIPTEN__
		if (!this->deviceId) {
			return 0;
		}
		U32 gotBytesPerSecond = bytesPerSecondGot();
		U32 frameSize = bytesPerSampleGot() * this->got.channels;
		if (!gotBytesPerSecond || !frameSize) {
			return 0;
		}
		U32 queued = SDL_GetQueuedAudioSize(this->deviceId);
		U32 target = (gotBytesPerSecond * 96 / 1000) & ~(frameSize - 1);
		if (queued >= target) {
			return 0;
		}
		U32 bytes = (target - queued) & ~(frameSize - 1);
		if (!bytes) {
			return 0;
		}
		if (this->silenceBuffer.size() < bytes) {
			this->silenceBuffer.resize(bytes);
		}
		SDL_memset(this->silenceBuffer.data(), this->got.silence, bytes);
		SDL_QueueAudio(this->deviceId, this->silenceBuffer.data(), bytes);
		return bytes;
#else
		return 0;
#endif
	}
};

#ifndef __EMSCRIPTEN__
// Not really a voice; currently voices are not mixed.
static std::list<std::shared_ptr<KDspAudioSdl>> voices;

static void audioCallback(void* /*userdata*/, U8* stream, S32 len) {
	if (!voices.size()) {
		memset(stream, sdlSilence, len);
		return;
	}
	std::shared_ptr<KDspAudioSdl> data = voices.front();
	if (data->closeWhenDone && data->audioBuffer.size() == 0 && (data->cvtBufPos == 0 || data->cvtBufPos >= data->cvt.len_cvt)) {
		data->closeAudioFromAudioThread();
		memset(stream, sdlSilence, len);
		return;
	}

	S32 available = (S32)data->audioBuffer.size();

	if (!data->sameFormat) {
		if (data->cvtBufPos < data->cvt.len_cvt) {
			S32 todo = data->cvt.len_cvt - data->cvtBufPos;
			if (todo > len) {
				todo = len;
			}
			memcpy(stream, data->cvt.buf + data->cvtBufPos, todo);
			data->cvtBufPos += todo;
			stream += todo;
			len -= todo;
		}
		if (len) {
			data->cvt.len = available;
			if (data->cvtBufLen && data->cvtBufLen < data->cvt.len * data->cvt.len_mult) {
				data->cvtBufLen = 0;
				SDL_free(data->cvtBuf);
				data->cvtBuf = nullptr;
			}
			if (!data->cvtBufLen) {
				data->cvtBufLen = data->cvt.len * data->cvt.len_mult;
				data->cvtBuf = (Uint8*)SDL_malloc(data->cvt.len * data->cvt.len_mult);
			}
			data->cvt.buf = data->cvtBuf;

			std::copy(data->audioBuffer.begin(), data->audioBuffer.begin() + available, data->cvt.buf);
			data->audioBuffer.erase(data->audioBuffer.begin(), data->audioBuffer.begin() + available);

			SDL_ConvertAudio(&data->cvt);
			S32 todo = data->cvt.len_cvt;
			if (todo > len) {
				todo = len;
			}
			memcpy(stream, data->cvt.buf, todo);
			stream += todo;
			len -= todo;
			data->cvtBufPos = todo;
		}
	} else {
		if (available > len) {
			available = len;
		}
		if (available) {
			std::copy(data->audioBuffer.begin(), data->audioBuffer.begin() + available, stream);
			data->audioBuffer.erase(data->audioBuffer.begin(), data->audioBuffer.begin() + available);
			len -= available;
			stream += available;
		}
	}
	if (len) {
		memset(stream, data->got.silence, len);
	}
}
#endif

#ifdef __EMSCRIPTEN__
// Voices whose closeAudio was called while audio was still queued to SDL.
// A timer polls these and finalizes the device close when the queue drains.
static std::list<std::shared_ptr<KDspAudioSdl>> pendingCloses;
static BOXEDWINE_MUTEX pendingClosesMutex;
static SDL_TimerID drainTimer = 0;

static Uint32 SDLCALL drainTimerCb(Uint32 interval, void* /*param*/) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pendingClosesMutex);
	auto it = pendingCloses.begin();
	while (it != pendingCloses.end()) {
		std::shared_ptr<KDspAudioSdl> v = *it;
		if (!v->deviceId || SDL_GetQueuedAudioSize(v->deviceId) == 0) {
			if (v->deviceId) {
				SDL_CloseAudioDevice(v->deviceId);
				v->deviceId = 0;
			}
			it = pendingCloses.erase(it);
		} else {
			++it;
		}
	}
	if (pendingCloses.empty() && drainTimer) {
		interval = 0; // stop the timer
		drainTimer = 0;
	}
	return interval;
}

static void ensureDrainTimer() {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pendingClosesMutex);
	if (!drainTimer) {
		if (!SDL_WasInit(SDL_INIT_TIMER)) {
			SDL_InitSubSystem(SDL_INIT_TIMER);
		}
		drainTimer = SDL_AddTimer(50, drainTimerCb, nullptr);
	}
}
#endif

void KDspAudioSdl::soundEnabled() {
	if (this->open) {
		openAudio(this->openedFormat, this->want.freq, this->want.channels);
	}
}

void KDspAudioSdl::openAudio(U32 format, U32 freq, U32 channels) {
#ifdef __EMSCRIPTEN__
	this->want.callback = nullptr; // SDL_QueueAudio mode
	this->want.userdata = nullptr;
#else
	this->want.callback = audioCallback;
#endif
	this->want.format = getSdlFormat(format);
	this->want.freq = freq;
	this->want.channels = channels;
	this->openedFormat = format;

	if (!KSystem::soundEnabled) {
#ifndef __EMSCRIPTEN__
		sdlAudioOpen = true;
#endif
		this->open = true;
		return;
	}

#ifdef __EMSCRIPTEN__
	{
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pendingClosesMutex);
		// Drop any pending-close entry; we're reusing this voice.
		std::shared_ptr<KDspAudioSdl> self = shared_from_this();
		for (auto it = pendingCloses.begin(); it != pendingCloses.end();) {
			if (*it == self) {
				it = pendingCloses.erase(it);
			} else {
				++it;
			}
		}
		// Close any prior device on this voice (reopening drops any still-queued audio).
		if (this->deviceId) {
			SDL_CloseAudioDevice(this->deviceId);
			this->deviceId = 0;
		}
		if (this->stream) {
			SDL_FreeAudioStream(this->stream);
			this->stream = nullptr;
		}
	}
#else
	// If the previous audio is still playing, it will get cut off. If I find a game that needs this, then perhaps I should think of a mixer.
	closeSdlAudio();
#endif

    SDL_AudioSpec requested = this->want;
#ifdef __MACH__
    if (requested.freq < 44100) {
        requested.freq = 44100;
    }
#endif
#ifdef __EMSCRIPTEN__
	SDL_AudioDeviceID newId = SDL_OpenAudioDevice(nullptr, 0, &requested, &this->got, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (newId == 0) {
		klog_fmt("Failed to open audio: %s", SDL_GetError());
		return;
	}
	this->deviceId = newId;
#else
	if (SDL_OpenAudio(&requested, &this->got) < 0) {
		klog_fmt("Failed to open audio: %s", SDL_GetError());
	}
	sdlSilence = this->got.silence;
	sdlAudioOpen = true;
#endif

	if (this->want.freq != this->got.freq || this->want.channels != this->got.channels || this->want.format != this->got.format) {
		this->sameFormat = false;
#ifdef __EMSCRIPTEN__
		this->stream = SDL_NewAudioStream(this->want.format, this->want.channels, this->want.freq, this->got.format, this->got.channels, this->got.freq);
		if (!this->stream) {
			SDL_BuildAudioCVT(&this->cvt, this->want.format, this->want.channels, this->want.freq, this->got.format, this->got.channels, this->got.freq);
		}
#else
		SDL_BuildAudioCVT(&this->cvt, this->want.format, this->want.channels, this->want.freq, this->got.format, this->got.channels, this->got.freq);
#endif
	} else {
		this->sameFormat = true;
	}

	this->open = true;	
#ifdef __EMSCRIPTEN__
	SDL_PauseAudioDevice(this->deviceId, 0);
#else
	voices.push_back(shared_from_this());
	SDL_PauseAudio(0);
	if (this->got.size) {
		//this->dspFragSize = this->got.size;
	}
#endif
	klog_fmt("openAudio: freq=%d(got %d) format=%x(got %x) channels=%d(got %d)", this->want.freq, this->got.freq, this->want.format, this->got.format, this->want.channels, this->got.channels);
}

#ifndef __EMSCRIPTEN__
void KDspAudioSdl::closeAudioFromAudioThread() {
	if (this->open) {
		this->onClose();
	}
}
#endif

void KDspAudioSdl::closeAudio() {
	if (!KSystem::soundEnabled) {
		this->open = false;
		return;
	}
#ifndef __EMSCRIPTEN__
	if (this->open) {
		bool needClose = true;
		{
			SDL_LockAudio();
			if (audioBuffer.size() || (this->cvtBufPos != 0 && this->cvtBufPos < this->cvt.len_cvt)) {
				closeWhenDone = true;
				needClose = false;
			}
			SDL_UnlockAudio();
		}
		if (needClose) {
			closeSdlAudio();
			this->onClose();
		}
	}
	return;
#else
	if (!this->open) {
		return;
	}

	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pendingClosesMutex);
	this->open = false;
	if (!this->deviceId) {
		return;
	}
	if (SDL_GetQueuedAudioSize(this->deviceId) > 0) {
		// Leave the device open so the queue can drain; the poll timer will finalize.
		ensureDrainTimer();
		pendingCloses.push_back(shared_from_this());
	} else {
		SDL_CloseAudioDevice(this->deviceId);
		this->deviceId = 0;
	}
#endif
}

#ifndef __EMSCRIPTEN__
void KDspAudioSdl::onClose() {
	if (!KSystem::soundEnabled) {
		this->open = false;
		return;
	}
	auto it = voices.begin();
	while (it != voices.end()) {
		std::shared_ptr<KDspAudioSdl> p = *it;
		if (p == shared_from_this()) {
			it = voices.erase(it);
			break;
		} else {
			it++;
		}
	}
	this->open = false;
}
#endif

void KDspAudioSdl::setFragmentSize(U32 size) {
#ifdef __EMSCRIPTEN__
	size = std::clamp(size, (U32)256, (U32)4096);
#else
	size = std::clamp(size, (U32)512, (U32)16384);
#endif
	U32 blockSize = bytesPerSampleWant() * want.channels;
	if (blockSize) {
		size &= ~(blockSize - 1);
	}
	this->dspFragSize = size ? size : blockSize;
}

U32 KDspAudioSdl::writeAudio(U8* data, U32 len) {
	U32 wantBytesPerSecond = want.freq * want.channels * bytesPerSampleWant();
	U32 delay = wantBytesPerSecond / 8;

	if (!KSystem::soundEnabled) {
		static U32 timeSinceLastWrite;

		U32 elapsedTime = KSystem::getMilliesSinceStart() - timeSinceLastWrite;
		U32 bytesToRemove = wantBytesPerSecond * elapsedTime / 1000;

		bytesToRemove = std::min((U32)this->audioBuffer.size(), bytesToRemove);
		this->audioBuffer.erase(this->audioBuffer.begin(), this->audioBuffer.begin() + bytesToRemove);

		if (this->audioBuffer.size() > delay) {
			return -K_EWOULDBLOCK;
		}
		U32 blockSize = bytesPerSampleWant() * want.channels;
		len = std::min(len, ((delay - (U32)this->audioBuffer.size()) & ~(blockSize - 1)));
		audioBuffer.insert(this->audioBuffer.end(), data, data + len);
		timeSinceLastWrite = KSystem::getMilliesSinceStart();
		return len;
	}

	if (!this->open) {
		return 0;
	}

#ifndef __EMSCRIPTEN__
	SDL_LockAudio();
	if (this->audioBuffer.size() > delay) {
		SDL_UnlockAudio();
		return -K_EWOULDBLOCK;
	}
	U32 blockSize = bytesPerSampleWant() * want.channels;
	len = std::min(len, ((delay - (U32)this->audioBuffer.size()) & ~(blockSize - 1)));
	audioBuffer.insert(this->audioBuffer.end(), data, data + len);
	SDL_UnlockAudio();
	return len;
#else
	if (!this->deviceId) {
		return 0;
	}
	U32 queued = this->getGuestQueuedAudioSizeWant();
	if (queued >= DSP_BUFFER_SIZE) {
		return -K_EWOULDBLOCK;
	}
	U32 blockSize = bytesPerSampleWant() * want.channels;
	len = std::min(len, (DSP_BUFFER_SIZE - queued) & ~(blockSize - 1));
	if (!len) {
		return -K_EWOULDBLOCK;
	}
#endif

	if (!this->sameFormat && this->stream) {
		if (SDL_AudioStreamPut(this->stream, data, (int)len) < 0) {
			return 0;
		}
		int available = SDL_AudioStreamAvailable(this->stream);
		if (available > 0) {
			if ((int)this->streamBuffer.size() < available) {
				this->streamBuffer.resize(available);
			}
			int got = SDL_AudioStreamGet(this->stream, this->streamBuffer.data(), available);
			if (got > 0) {
				SDL_QueueAudio(this->deviceId, this->streamBuffer.data(), got);
			}
		}
	} else if (!this->sameFormat) {
		int needed = (int)len * this->cvt.len_mult;
		if (this->cvtBufLen && this->cvtBufLen < needed) {
			SDL_free(this->cvtBuf);
			this->cvtBuf = nullptr;
			this->cvtBufLen = 0;
		}
		if (!this->cvtBufLen) {
			this->cvtBufLen = needed;
			this->cvtBuf = (Uint8*)SDL_malloc(this->cvtBufLen);
		}
		this->cvt.len = (int)len;
		this->cvt.buf = this->cvtBuf;
		memcpy(this->cvt.buf, data, len);
		SDL_ConvertAudio(&this->cvt);
		SDL_QueueAudio(this->deviceId, this->cvt.buf, this->cvt.len_cvt);
	} else {
		SDL_QueueAudio(this->deviceId, data, len);
	}
#ifdef __EMSCRIPTEN__
	addRealQueuedWant(len);
	topUpSilence();
	return len;
#endif
}

static U32 nextId = 0;
BOXEDWINE_MUTEX KDspAudio::mutex;
BHashTable<U32, KDspAudioWeakPtr> KDspAudio::openAudios;

KDspAudioPtr KDspAudio::createDspAudio() {
	KDspAudioPtr result = std::make_shared<KDspAudioSdl>();
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
	result->id = nextId++;
	KDspAudioWeakPtr weak = result;
	openAudios.set(result->id, weak);
	return result;
}

KDspAudio::~KDspAudio() {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
	openAudios.remove(this->id);
}

void KDspAudio::iterateOpenAudio(std::function<void(KDspAudioPtr&)> callback) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
	for (auto& it : openAudios) {
		KDspAudioPtr audio = it.value.lock();
		if (audio) {
			callback(audio);
		}
	}
}

void KDspAudio::shutdown() {
	if (!KSystem::soundEnabled) {
		return;
	}	
#ifdef __EMSCRIPTEN__
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pendingClosesMutex);
	if (drainTimer) {
		SDL_RemoveTimer(drainTimer);
		drainTimer = 0;
	}
	for (auto& v : pendingCloses) {
		if (v->deviceId) {
			SDL_CloseAudioDevice(v->deviceId);
			v->deviceId = 0;
		}
	}
	pendingCloses.clear();
#else
	SDL_PauseAudio(1);
	SDL_CloseAudio();
#endif
}
