#ifndef __KNATIVEAUDIO_H__
#define __KNATIVEAUDIO_H__

class KNativeAudio {
public:
	static std::shared_ptr<KNativeAudio> createNativeAudio();
    static std::vector< std::shared_ptr<KNativeAudio> > availableAudio;
    static void init();
	static void shutdown();

	KNativeAudio() {}
	virtual ~KNativeAudio() {}
	virtual bool load() = 0;
    
	virtual U32 midiOutOpen(U32 wDevID) = 0;
	virtual U32 midiOutClose(U32 wDevID) = 0;
	virtual U32 midiOutData(U32 wDevID, U8* buffer, U32 len) = 0;
	virtual U32 midiOutLongData(U32 wDevID, U8* buffer, U32 len) = 0;
	virtual U32 midiOutGetNumDevs() = 0;
	virtual U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) = 0;
	virtual U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) = 0;
	virtual U32 midiOutReset(U32 wDevID) = 0;
    virtual BString midiOutGetName(U32 wDevID) = 0;
    virtual bool midiOutIsOpen(U32 wDevID) = 0;
};

typedef std::shared_ptr<KNativeAudio> KNativeAudioPtr;
#endif
