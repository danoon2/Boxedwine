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
	U32 getBufferSize() override {return this->getGuestQueuedAudioSizeWant();}
	U32 getBufferCapacity() override { return DSP_BUFFER_SIZE;}

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
	U32 openedFormat = 0;
	int cvtBufLen = 0;
	unsigned char* cvtBuf = nullptr;
	bool sameFormat = false;
	U32 dspFragSize = 4096;
	bool open = false;
	std::deque<U8> audioBuffer; // only used when KSystem::soundEnabled is false
	SDL_AudioDeviceID deviceId = 0;
#ifdef BOXEDWINE_AUDIO_DEBUG
	U32 lastStatsTime = 0;
	U32 lastWriteTime = 0;
	U32 statsWrites = 0;
	U32 statsBlocks = 0;
	U32 statsPartialWrites = 0;
	U32 statsLowQueue = 0;
	U32 statsMaxWriteGap = 0;
	U32 statsMinQueueWant = 0xFFFFFFFF;
	U32 statsMaxQueueWant = 0;
	U32 statsInBytes = 0;
	U32 statsOutBytes = 0;
	U32 statsSilenceBytes = 0;
	U32 statsConvertMs = 0;
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

#ifdef BOXEDWINE_AUDIO_DEBUG
	void recordAudioStats(U32 queuedBefore, U32 queuedAfter, U32 requestedLen, U32 acceptedLen, U32 queuedLen, U32 convertMs) {
		U32 now = KSystem::getMilliesSinceStart();
		if (!this->lastStatsTime) {
			this->lastStatsTime = now;
		}
		if (this->lastWriteTime) {
			this->statsMaxWriteGap = std::max(this->statsMaxWriteGap, now - this->lastWriteTime);
		}
		this->lastWriteTime = now;
		this->statsWrites++;
		this->statsInBytes += acceptedLen;
		this->statsOutBytes += queuedLen;
		this->statsConvertMs += convertMs;
		if (acceptedLen < requestedLen) {
			this->statsPartialWrites++;
		}
		this->statsMinQueueWant = std::min(this->statsMinQueueWant, queuedBefore);
		this->statsMaxQueueWant = std::max(this->statsMaxQueueWant, queuedAfter);
		if (queuedBefore < this->dspFragSize) {
			this->statsLowQueue++;
		}
		if (now - this->lastStatsTime >= 1000) {
			U32 statsMs = now - this->lastStatsTime;
			U32 wantBytesPerSecond = bytesPerSecondWant();
			U32 gotBytesPerSecond = bytesPerSecondGot();
			U32 minQueueMs = wantBytesPerSecond ? (U32)(((U64)this->statsMinQueueWant * 1000) / wantBytesPerSecond) : 0;
			U32 maxQueueMs = wantBytesPerSecond ? (U32)(((U64)this->statsMaxQueueWant * 1000) / wantBytesPerSecond) : 0;
			U32 sdlQueueBytes = this->deviceId ? SDL_GetQueuedAudioSize(this->deviceId) : 0;
			U32 sdlQueueMs = gotBytesPerSecond ? (U32)(((U64)sdlQueueBytes * 1000) / gotBytesPerSecond) : 0;
			U32 expectedIn = (U32)(((U64)wantBytesPerSecond * statsMs) / 1000);
			klog_fmt("audioStats: id=%d want=%d/%d/%x got=%d/%d/%x samples=%d same=%d ms=%d writes=%d in=%d expectIn=%d out=%d silence=%d block=%d partial=%d low=%d qWantMs=%d..%d qSdl=%dms/%dB gapMax=%dms cvtMs=%d",
				this->id, this->want.freq, this->want.channels, this->want.format, this->got.freq, this->got.channels, this->got.format, this->got.samples, this->sameFormat ? 1 : 0,
				statsMs, this->statsWrites, this->statsInBytes, expectedIn, this->statsOutBytes, this->statsSilenceBytes, this->statsBlocks, this->statsPartialWrites, this->statsLowQueue,
				minQueueMs, maxQueueMs, sdlQueueMs, sdlQueueBytes, this->statsMaxWriteGap, this->statsConvertMs);
			this->lastStatsTime = now;
			this->statsWrites = 0;
			this->statsBlocks = 0;
			this->statsPartialWrites = 0;
			this->statsLowQueue = 0;
			this->statsMaxWriteGap = 0;
			this->statsMinQueueWant = 0xFFFFFFFF;
			this->statsMaxQueueWant = 0;
			this->statsInBytes = 0;
			this->statsOutBytes = 0;
			this->statsSilenceBytes = 0;
			this->statsConvertMs = 0;
		}
	}

	void recordAudioBlock(U32 queuedBefore) {
		this->statsBlocks++;
		this->statsMinQueueWant = std::min(this->statsMinQueueWant, queuedBefore);
		this->statsMaxQueueWant = std::max(this->statsMaxQueueWant, queuedBefore);
		U32 now = KSystem::getMilliesSinceStart();
		if (!this->lastStatsTime) {
			this->lastStatsTime = now;
		}
		if (now - this->lastStatsTime >= 1000) {
			recordAudioStats(queuedBefore, queuedBefore, 0, 0, 0, 0);
		}
	}
#endif
};

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

void KDspAudioSdl::soundEnabled() {
	if (this->open) {
		openAudio(this->openedFormat, this->want.freq, this->want.channels);
	}
}

void KDspAudioSdl::openAudio(U32 format, U32 freq, U32 channels) {
	this->want.callback = nullptr; // SDL_QueueAudio mode
	this->want.userdata = nullptr;
	this->want.format = getSdlFormat(format);
	this->want.freq = freq;
	this->want.channels = channels;
	this->openedFormat = format;

	if (!KSystem::soundEnabled) {
		this->open = true;
		return;
	}

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
	}

    SDL_AudioSpec requested = this->want;
#ifdef __MACH__
    if (requested.freq < 44100) {
        requested.freq = 44100;
    }
#endif
	SDL_AudioDeviceID newId = SDL_OpenAudioDevice(nullptr, 0, &requested, &this->got, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (newId == 0) {
		klog_fmt("Failed to open audio: %s", SDL_GetError());
		return;
	}
	this->deviceId = newId;

	if (this->want.freq != this->got.freq || this->want.channels != this->got.channels || this->want.format != this->got.format) {
		this->sameFormat = false;
		SDL_BuildAudioCVT(&this->cvt, this->want.format, this->want.channels, this->want.freq, this->got.format, this->got.channels, this->got.freq);
	} else {
		this->sameFormat = true;
	}

	this->open = true;	
	SDL_PauseAudioDevice(this->deviceId, 0);
	klog_fmt("openAudio: freq=%d(got %d) format=%x(got %x) channels=%d(got %d)", this->want.freq, this->got.freq, this->want.format, this->got.format, this->want.channels, this->got.channels);
}

void KDspAudioSdl::closeAudio() {
	if (!KSystem::soundEnabled) {
		this->open = false;
		return;
	}
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
}

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
	if (!KSystem::soundEnabled) {
		static U32 timeSinceLastWrite;

		U32 wantBytesPerSecond = want.freq * want.channels * bytesPerSampleWant();
		U32 delay = wantBytesPerSecond / 8;
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

	if (!this->open || !this->deviceId) {
		return 0;
	}

	U32 queued = this->getGuestQueuedAudioSizeWant();
	if (queued >= DSP_BUFFER_SIZE) {
#ifdef BOXEDWINE_AUDIO_DEBUG
		recordAudioBlock(queued);
#endif
		return -K_EWOULDBLOCK;
	}
	U32 blockSize = bytesPerSampleWant() * want.channels;
#ifdef BOXEDWINE_AUDIO_DEBUG
	U32 requestedLen = len;
#endif
	len = std::min(len, (DSP_BUFFER_SIZE - queued) & ~(blockSize - 1));
	if (!len) {
#ifdef BOXEDWINE_AUDIO_DEBUG
		recordAudioBlock(queued);
#endif
		return -K_EWOULDBLOCK;
	}

#ifdef BOXEDWINE_AUDIO_DEBUG
	U32 queuedLen = len;
	U32 convertMs = 0;
#endif
	if (!this->sameFormat) {
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
#ifdef BOXEDWINE_AUDIO_DEBUG
		U32 convertStart = KSystem::getMilliesSinceStart();
#endif
		SDL_ConvertAudio(&this->cvt);
#ifdef BOXEDWINE_AUDIO_DEBUG
		convertMs = KSystem::getMilliesSinceStart() - convertStart;
		queuedLen = this->cvt.len_cvt;
#endif
		SDL_QueueAudio(this->deviceId, this->cvt.buf, this->cvt.len_cvt);
	} else {
		SDL_QueueAudio(this->deviceId, data, len);
	}
	addRealQueuedWant(len);
#ifdef BOXEDWINE_AUDIO_DEBUG
	this->statsSilenceBytes += topUpSilence();
	recordAudioStats(queued, this->getGuestQueuedAudioSizeWant(), requestedLen, len, queuedLen, convertMs);
#else
	topUpSilence();
#endif
	return len;
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
}
