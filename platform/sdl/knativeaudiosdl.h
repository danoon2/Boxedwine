#ifndef __KNATIVE_AUDIO_SDL_H__
#define __KNATIVE_AUDIO_SDL_H__

class KNativeAudioSDL : public KNativeAudio {
public:
	virtual ~KNativeAudioSDL() {}

	bool load() override;
    U32 midiOutOpen(U32 wDevID) override;
    U32 midiOutClose(U32 wDevID) override;
    U32 midiOutData(U32 wDevID, U8* buffer, U32 len) override;
    U32 midiOutLongData(U32 wDevID, U8* buffer, U32 len) override;
    U32 midiOutGetNumDevs() override;
    U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) override;
    U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) override;
    U32 midiOutReset(U32 wDevID) override;
    BString midiOutGetName(U32 wDevID) override;
    bool midiOutIsOpen(U32 wDevID) override;
};

#endif
