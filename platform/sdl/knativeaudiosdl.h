#ifndef __KNATIVE_AUDIO_SDL_H__
#define __KNATIVE_AUDIO_SDL_H__

class KNativeSDLAudioData {
public:
	KNativeSDLAudioData() = default;
	~KNativeSDLAudioData() {
		if (resamp_buffer) {
			delete[] resamp_buffer;
		}
		if (cvtBuf) {
			delete[] cvtBuf;
		}
	}

	SDL_AudioSpec want = { 0 };
	SDL_AudioSpec got = { 0 };
	SDL_AudioCVT cvt = { 0 };
	U8* cvtBuf = nullptr;
	U32 cvtBufSize = 0;

	bool sameFormat = false;
	bool open = false;
	std::weak_ptr<KProcess> process;

	bool isRender = false;
	bool isPlaying = false;
	U32 eventFd = 0;
	U32 adevid = 0;

	U32 cap_held_frames = 0;
	U32 resamp_bufsize_frames = 0;
	U8* resamp_buffer = nullptr;
	U32 cap_offs_frames = 0;
	U32 bufsize_frames = 0;

	// points to memory in the emulator, must be locked before read/write
	U32 address_local_buffer = 0;
	U32 address_wri_offs_frames = 0;
	U32 address_held_frames = 0;
	U32 address_lcl_offs_frames = 0;

	// mirrored in emulator side
	U32 period_frames = 0; // read only, doesn't change
	BoxedWaveFormatExtensible fmt; // read only, doesn't change
};

class KNativeAudioSDL : public KNativeAudio, public std::enable_shared_from_this<KNativeAudioSDL> {
public:
	virtual ~KNativeAudioSDL() {}
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

	U32 getSdlFormat(BoxedWaveFormatExtensible* pFmt);

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
		return nullptr;
	}
};

#endif
