#ifndef __KNATIVE_AUDIO_SDL_H__
#define __KNATIVE_AUDIO_SDL_H__

class KNativeSDLAudioData {
public:
	KNativeSDLAudioData() : cvtBuf(0), cvtBufSize(0), sameFormat(false), open(false), isRender(false), isPlaying(false), eventFd(0), adevid(0), cap_held_frames(0), resamp_bufsize_frames(0), resamp_buffer(0), cap_offs_frames(0), bufsize_frames(0), address_local_buffer(0), address_wri_offs_frames(0), address_held_frames(0), address_lcl_offs_frames(0), period_frames(0) {}
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
	U32 eventFd;
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
	BoxedWaveFormatExtensible fmt; // read only, doesn't change
};

class KNativeAudioSDL : public KNativeAudio, public std::enable_shared_from_this<KNativeAudioSDL> {
public:
	virtual ~KNativeAudioSDL() {}
	virtual bool load();
	virtual void free();
	virtual bool open();
	virtual bool close();
	virtual void start(U32 boxedAudioId, U32 eventFd);
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
	virtual void cleanup();

	virtual U32 midiOutOpen(U32 wDevID, U32 lpDesc, U32 dwFlags, U32 fd);
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
		return NULL;
	}
};

#endif