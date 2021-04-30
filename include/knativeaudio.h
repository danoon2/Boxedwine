#ifndef __KNATIVEAUDIO_H__
#define __KNATIVEAUDIO_H__

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

#define KSAUDIO_SPEAKER_STEREO (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)

class GUID {
public:
	GUID() : Data1(0), Data2(0), Data3(0), Data4() {}
	GUID(U32 Data1, U16 Data2, U16 Data3, U8 Data4, U8 Data5, U8 Data6, U8 Data7, U8 Data8, U8 Data9, U8 Data10, U8 Data11) : Data1(Data1), Data2(Data2), Data3(Data3) {
		this->Data4[0] = Data4;
		this->Data4[1] = Data5;
		this->Data4[2] = Data6;
		this->Data4[3] = Data7;
		this->Data4[4] = Data8;
		this->Data4[5] = Data9;
		this->Data4[6] = Data10;
		this->Data4[7] = Data11;
	}
	bool operator==(const GUID& other) const {
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
	void read(U32 address) {
		Data1 = readd(address); address += 4;
		Data2 = readd(address); address += 2;
		Data3 = readd(address); address += 2;
		memcopyToNative(address, Data4, 8);
	}
	void write(U32 address) {
		writed(address, Data1); address += 4;
		writew(address, Data2); address += 2;
		writew(address, Data3); address += 2;
		memcopyFromNative(address, Data4, 8);
	}
	U32  Data1;
	U16  Data2;
	U16  Data3;
	U8   Data4[8];
};

class WaveFormatEx {
public:
	WaveFormatEx() : wFormatTag(0), nChannels(0), nSamplesPerSec(0), nAvgBytesPerSec(0), nBlockAlign(0), wBitsPerSample(0), cbSize(0) {}
	U32 read(U32 address) {
		wFormatTag = readw(address); address += 2;
		nChannels = readw(address); address += 2;

		nSamplesPerSec = readd(address); address += 4;
		nAvgBytesPerSec = readd(address); address += 4;

		nBlockAlign = readw(address); address += 2;
		wBitsPerSample = readw(address); address += 2;
		cbSize = readw(address); address += 2;
		return address;
	}
	U32 write(U32 address) {
		writew(address, wFormatTag); address += 2;
		writew(address, nChannels); address += 2;
		writed(address, nSamplesPerSec); address += 4;
		writed(address, nAvgBytesPerSec); address += 4;
		writew(address, nBlockAlign); address += 2;
		writew(address, wBitsPerSample); address += 2;
		writew(address, cbSize); address += 2;
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

class WaveFormatExtensible : public WaveFormatEx {
public:
	WaveFormatExtensible() : WaveFormatEx(), wValidBitsPerSample(0), dwChannelMask(0) {}
	void read(U32 address) {
		address = WaveFormatEx::read(address);
		if (this->cbSize == 0 || this->cbSize >= 22) {
			wValidBitsPerSample = readw(address); address += 2;
			dwChannelMask = readd(address); address += 4;
			SubFormat.read(address);
		}
	}
	void write(U32 address) {
		address = WaveFormatEx::write(address);
		if (this->cbSize == 0 || this->cbSize >= 22) {
			writew(address, wValidBitsPerSample); address += 2;
			writed(address, dwChannelMask); address += 4;
			SubFormat.write(address);
		}
	}
	U16 wValidBitsPerSample; // union with wSamplesPerBlock
	U32 dwChannelMask;
	GUID SubFormat;
};

class KNativeAudio {
public:
	static std::shared_ptr<KNativeAudio> createNativeAudio();
	static void shutdown();

	virtual ~KNativeAudio() {}

	virtual bool load() = 0;
	virtual void free() = 0;
	virtual bool open() = 0;
	virtual bool close() = 0;
	virtual void start(U32 boxedAudioId) = 0;
	virtual void stop(U32 boxedAudioId) = 0;
	virtual bool configure() = 0;
	virtual U32 hasDevice(bool isRender) = 0;
	virtual U32 getEndPoint(bool isRender, U32 adevid) = 0;
	virtual void release(U32 boxedAudioId) = 0;
	virtual void captureResample(U32 boxedAudioId) = 0;
	virtual U32 init(bool isRender, U32 boxedAudioId, U32 addressFmt, U32 addressPeriodFrames, U32 addressLocalBuffer, U32 addressWriOffsFrames, U32 addressHeldFrames, U32 addressLclOffsFrames, U32 bufsizeFrames) = 0;
	virtual U32 getLatency(U32 boxedAudioId, U32* latency) = 0;
	virtual void lock(U32 boxedAudioId) = 0;
	virtual void unlock(U32 boxedAudioId) = 0;
	virtual U32 isFormatSupported(U32 boxedAudioId, U32 addressWaveFormat) = 0;
	virtual U32 getMixFormat(U32 boxedAudioId, U32 addressWaveFormat) = 0;
	virtual void setVolume(U32 boxedAudioId, float level, U32 channel) = 0;

	virtual U32 midiOutOpen(U32 wDevID, U32 lpDesc, U32 dwFlags) = 0;
	virtual U32 midiOutClose(U32 wDevID) = 0;
	virtual U32 midiOutData(U32 wDevID, U32 dwParam) = 0;
	virtual U32 midiOutLongData(U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiOutPrepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiOutUnprepare(U32 wDevID, U32 lpMidiHdr, U32 dwSize) = 0;
	virtual U32 midiOutGetDevCaps(U32 wDevID, U32 lpCaps, U32 dwSize) = 0;
	virtual U32 midiOutGetNumDevs() = 0;
	virtual U32 midiOutGetVolume(U32 wDevID, U32 lpdwVolume) = 0;
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
};

#endif