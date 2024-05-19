#ifndef __KNATIVEAUDIO_H__
#define __KNATIVEAUDIO_H__

// no params
#define EVENT_MSG_DATA_READ 1

// dwCallBack, uFlags, hDev, wMsg, dwInstance, dwParam1, dwParam2
#define EVENT_MSG_NOTIFY 2

#define SPEAKER_FRONT_LEFT              0x00000001
#define SPEAKER_FRONT_RIGHT             0x00000002
#define SPEAKER_FRONT_CENTER            0x00000004
#define SPEAKER_LOW_FREQUENCY           0x00000008
#define SPEAKER_BACK_LEFT               0x00000010
#define SPEAKER_BACK_RIGHT              0x00000020
#define SPEAKER_FRONT_LEFT_OF_CENTER    0x00000040
#define SPEAKER_FRONT_RIGHT_OF_CENTER   0x00000080
#define SPEAKER_BACK_CENTER             0x00000100
#define SPEAKER_SIDE_LEFT               0x00000200
#define SPEAKER_SIDE_RIGHT              0x00000400
#define SPEAKER_TOP_CENTER              0x00000800
#define SPEAKER_TOP_FRONT_LEFT          0x00001000
#define SPEAKER_TOP_FRONT_CENTER        0x00002000
#define SPEAKER_TOP_FRONT_RIGHT         0x00004000
#define SPEAKER_TOP_BACK_LEFT           0x00008000
#define SPEAKER_TOP_BACK_CENTER         0x00010000
#define SPEAKER_TOP_BACK_RIGHT          0x00020000
#define SPEAKER_RESERVED                0x7FFC0000
#define SPEAKER_ALL                     0x80000000

#define KSAUDIO_SPEAKER_DIRECTOUT 0
#define KSAUDIO_SPEAKER_MONO SPEAKER_FRONT_CENTER
#define KSAUDIO_SPEAKER_STEREO (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)
#define KSAUDIO_SPEAKER_QUAD (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
#define KSAUDIO_SPEAKER_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER)
#define KSAUDIO_SPEAKER_5POINT1 (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
/* 5:1 SIDE or BACK is not distinguished, only 0x3F shall be used (BACK) */
#define KSAUDIO_SPEAKER_5POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
#define KSAUDIO_SPEAKER_7POINT1 (KSAUDIO_SPEAKER_5POINT1 | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER)
/* 7:1 home theater 0x63F */
#define KSAUDIO_SPEAKER_7POINT1_SURROUND (KSAUDIO_SPEAKER_5POINT1 | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)

class BoxedGUID {
public:
	BoxedGUID() = default;
	BoxedGUID(U32 Data1, U16 Data2, U16 Data3, U8 Data4, U8 Data5, U8 Data6, U8 Data7, U8 Data8, U8 Data9, U8 Data10, U8 Data11) : Data1(Data1), Data2(Data2), Data3(Data3) {
		this->Data4[0] = Data4;
		this->Data4[1] = Data5;
		this->Data4[2] = Data6;
		this->Data4[3] = Data7;
		this->Data4[4] = Data8;
		this->Data4[5] = Data9;
		this->Data4[6] = Data10;
		this->Data4[7] = Data11;
	}
	bool operator==(const BoxedGUID& other) const {
		if (Data1 != other.Data1) {
			return false;
		}
		if (Data2 != other.Data2) {
			return false;
		}
		if (Data3 != other.Data3) {
			return false;
		}
		return memcmp(Data4, other.Data4, 8) == 0;
	}
	void read(KMemory* memory, U32 address) {
		Data1 = memory->readd(address); address += 4;
		Data2 = memory->readd(address); address += 2;
		Data3 = memory->readd(address); address += 2;
		memory->memcpy(Data4, address, 8);
	}
	void write(KMemory* memory, U32 address) {
		memory->writed(address, Data1); address += 4;
		memory->writew(address, Data2); address += 2;
		memory->writew(address, Data3); address += 2;
		memory->memcpy(address, Data4, 8);
	}
	U32  Data1 = 0;
	U16  Data2 = 0;
	U16  Data3 = 0;
	U8   Data4[8] = { 0 };
};

class BoxedWaveFormatEx {
public:
	BoxedWaveFormatEx() : wFormatTag(0), nChannels(0), nSamplesPerSec(0), nAvgBytesPerSec(0), nBlockAlign(0), wBitsPerSample(0), cbSize(0) {}
	U32 read(KMemory* memory, U32 address) {
		wFormatTag = memory->readw(address); address += 2;
		nChannels = memory->readw(address); address += 2;

		nSamplesPerSec = memory->readd(address); address += 4;
		nAvgBytesPerSec = memory->readd(address); address += 4;

		nBlockAlign = memory->readw(address); address += 2;
		wBitsPerSample = memory->readw(address); address += 2;
		cbSize = memory->readw(address); address += 2;
		return address;
	}
	U32 write(KMemory* memory, U32 address) {
		memory->writew(address, wFormatTag); address += 2;
		memory->writew(address, nChannels); address += 2;
		memory->writed(address, nSamplesPerSec); address += 4;
		memory->writed(address, nAvgBytesPerSec); address += 4;
		memory->writew(address, nBlockAlign); address += 2;
		memory->writew(address, wBitsPerSample); address += 2;
		memory->writew(address, cbSize); address += 2;
		return address;
	}

	U16	wFormatTag;
	U16	nChannels;
	U32	nSamplesPerSec;
	U32	nAvgBytesPerSec;
	U16	nBlockAlign;
	U16	wBitsPerSample;
	U16	cbSize;
};

class BoxedWaveFormatExtensible {
public:
	void read(KMemory* memory, U32 address) {
		address = ex.read(memory, address);
		if (ex.cbSize == 0 || ex.cbSize >= 22) {
			wValidBitsPerSample = memory->readw(address); address += 2;
			dwChannelMask = memory->readd(address); address += 4;
			SubFormat.read(memory, address);
		}
	}
	void write(KMemory* memory, U32 address) {
		address = ex.write(memory, address);
		if (ex.cbSize == 0 || ex.cbSize >= 22) {
			memory->writew(address, wValidBitsPerSample); address += 2;
			memory->writed(address, dwChannelMask); address += 4;
			SubFormat.write(memory, address);
		}
	}
	BoxedWaveFormatEx ex;
	U16 wValidBitsPerSample = 0; // union with wSamplesPerBlock
	U32 dwChannelMask = 0;
	BoxedGUID SubFormat;
};

class KNativeAudio {
public:
	static std::shared_ptr<KNativeAudio> createNativeAudio();
    static std::vector< std::shared_ptr<KNativeAudio> > availableAudio;
    static void init();
	static void shutdown();

	KNativeAudio() : memory(nullptr) {}
	virtual ~KNativeAudio() {}
	
	virtual bool load() = 0;
	virtual void free() = 0;
	virtual bool open() = 0;
	virtual bool close() = 0;
	virtual void start(U32 boxedAudioId, U32 eventFd) = 0;
	virtual void stop(U32 boxedAudioId) = 0;
	virtual bool configure() = 0;
	virtual U32 hasDevice(bool isRender) = 0;
	virtual U32 getEndPoint(bool isRender, U32 adevid) = 0;
	virtual void release(U32 boxedAudioId) = 0;
	virtual void captureResample(U32 boxedAudioId) = 0;
	virtual U32 init(std::shared_ptr<KProcess> process, bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames) = 0;
	virtual U32 getLatency(U32 boxedAudioId, U32* latency) = 0;
	virtual void lock(U32 boxedAudioId) = 0;
	virtual void unlock(U32 boxedAudioId) = 0;
	virtual U32 isFormatSupported(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) = 0;
	virtual U32 getMixFormat(KThread* thread, U32 boxedAudioId, U32 addressWaveFormat) = 0;
	virtual void setVolume(U32 boxedAudioId, float level, U32 channel) = 0;
    virtual void cleanup() = 0;
    
	virtual U32 midiOutOpen(KProcess* process, U32 wDevID, U32 lpDesc, U32 dwFlags, U32 fd) = 0;
	virtual U32 midiOutClose(U32 wDevID) = 0;
	virtual U32 midiOutData(U32 wDevID, U32 dwParam) = 0;
	virtual U32 midiOutLongData(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiOutPrepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiOutUnprepare(KThread* thread, U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiOutGetDevCaps(KThread* thread, U32 wDevID, U32 lpCaps, U32 dwSize) = 0;
	virtual U32 midiOutGetNumDevs() = 0;
	virtual U32 midiOutGetVolume(KThread* thread, U32 wDevID, U32 lpdwVolume) = 0;
	virtual U32 midiOutSetVolume(U32 wDevID, U32 dwVolume) = 0;
	virtual U32 midiOutReset(U32 wDevID) = 0;

	virtual U32 midiInOpen(U32 wDevID, U32 lpDesc, U32 dwFlags) = 0;
	virtual U32 midiInClose(U32 wDevID) = 0;
	virtual U32 midiInAddBuffer(U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiInPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiInUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiInGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) = 0;
	virtual U32 midiInGetNumDevs() = 0;
	virtual U32 midiInStart(U32 wDevID) = 0;
	virtual U32 midiInStop(U32 wDevID) = 0;
	virtual U32 midiInReset(U32 wDevID) = 0;

protected:
	KMemory* memory;
};

#endif
