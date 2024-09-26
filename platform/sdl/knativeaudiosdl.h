#ifndef __KNATIVE_AUDIO_SDL_H__
#define __KNATIVE_AUDIO_SDL_H__

class KNativeSDLAudioData {
public:
	KNativeSDLAudioData() = default;
    ~KNativeSDLAudioData();
    
	U32 id;

	SDL_AudioSpec want = { 0 };
	SDL_AudioSpec got = { 0 };
	SDL_AudioCVT cvt = { 0 };
	U8* cvtBuf = nullptr;
	U32 cvtBufSize = 0;

	bool sameFormat = false;
	bool open = false;
	KProcessWeakPtr process;

	bool isRender = false;
	bool isPlaying = false;
	U32 callbackFD = 0;
	U32 callbackAddress = 0;
	U32 emulatedAddress = 0;
	U32 emulatedAddressSize = 0;
	U8* nativeAddress = nullptr;
	BoxedWaveFormatEx fmt; // read only, doesn't change

	static U32 nextId;
};

typedef std::shared_ptr<KNativeSDLAudioData> KNativeSDLAudioDataPtr;

class KNativeAudioSDL : public KNativeAudio, public std::enable_shared_from_this<KNativeAudioSDL> {
public:
	virtual ~KNativeAudioSDL() {}

	// from KNativeAudio
	bool load() override;
	void free() override;
	bool open() override;
	bool close() override;
	void start(U32 boxedAudioId) override;
	void stop(U32 boxedAudioId) override;
	U32 getEndPoint(bool isRender) override;
	void release(U32 boxedAudioId) override;
	void captureResample(U32 boxedAudioId) override;
	U32 init(KProcessPtr process, U32 boxedAudioId) override;
	void setCallback(U32 boxedAudioId, U32 callbackFD, U32 callbackAddress) override;
	void setFormat(U32 boxedAudioId, BoxedWaveFormatEx* format) override;
	U32 getLatency(U32 boxedAudioId, U32* latency) override;
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

	U32 getSdlFormat(BoxedWaveFormatEx* pFmt);

	BHashTable<U32, KNativeSDLAudioDataPtr> data;
	BOXEDWINE_MUTEX dataMutex;
	KNativeSDLAudioDataPtr getDataFromId(U32 boxedAudioId);
	KNativeSDLAudioDataPtr playing; // keep reference until we close the audio
};

#endif
