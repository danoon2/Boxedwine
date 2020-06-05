#ifndef __KNATIVEAUDIO_H__
#define __KNATIVEAUDIO_H__

class KNativeAudio {
public:
	static std::shared_ptr<KNativeAudio> createNativeAudio();
	static void shutdown();

	virtual void openAudio(U32 format, U32 freq, U32 channels)=0;
	virtual bool isOpen()=0;
	virtual void closeAudio() = 0;
	virtual void writeAudio(U8* data, U32 len) = 0;
	virtual U32 getFragmentSize() = 0;
	virtual U32 getBufferSize() = 0;
	virtual U32 getBufferCapacity() = 0;
};

#endif