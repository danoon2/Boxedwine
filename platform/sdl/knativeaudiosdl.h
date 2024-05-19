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

	// from KNativeAudio
	bool load() override;
	void free() override;
	bool open() override;
	bool close() override;
	void start(U32 boxedAudioId, U32 eventFd) override;
	void stop(U32 boxedAudioId) override;
	bool configure() override;
	U32 hasDevice(bool isRender) override;
	U32 getEndPoint(bool isRender, U32 adevid) override;
	void release(U32 boxedAudioId) override;
	void captureResample(U32 boxedAudioId) override;
	U32 init(std::shared_ptr<KProcess> process, bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames) override;
	U32 getLatency(U32 boxedAudioId, U32* latency) override;
	void lock(U32 boxedAudioId) override;
	void unlock(U32 boxedAudioId) override;
	U32 isFormatSupported(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) override;
	U32 getMixFormat(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) override;
	void setVolume(U32 boxedAudioId, float level, U32 channel) override;
	void cleanup() override;

	U32 midiOutOpen(KProcess* process, U32 wDevID, U32 lpDesc, U32 dwFlags, U32 fd) override;
	U32 midiOutClose(U32 wDevID) override;
	U32 midiOutData(U32 wDevID, U32 dwParam) override;
	U32 midiOutLongData(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
	U32 midiOutPrepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
	U32 midiOutUnprepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
	U32 midiOutGetDevCaps(KThread* thread, U32 wDevID, U32 lpCaps, U32 dwSize) override;
	U32 midiOutGetNumDevs() override;
	U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) override;
	U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) override;
	U32 midiOutReset(U32 wDevID) override;

	U32 midiInOpen(U32 wDevID, U32 lpDesc, U32 dwFlags) override;
	U32 midiInClose(U32 wDevID) override;
	U32 midiInAddBuffer(U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
	U32 midiInPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
	U32 midiInUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) override;
	U32 midiInGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) override;
	U32 midiInGetNumDevs() override;
	U32 midiInStart(U32 wDevID) override;
	U32 midiInStop(U32 wDevID) override;
	U32 midiInReset(U32 wDevID) override;

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
