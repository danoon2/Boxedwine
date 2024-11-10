#ifndef __KDSPAUDIO_H__
#define __KDSPAUDIO_H__

class KDspAudio {
public:
	static std::shared_ptr<KDspAudio> createDspAudio();
	static void shutdown();

	virtual ~KDspAudio() {}

	virtual void openAudio(U32 format, U32 freq, U32 channels) = 0;
	virtual bool isOpen() = 0;
	virtual void closeAudio() = 0;
	virtual U32 writeAudio(U8* data, U32 len) = 0;
	virtual U32 getFragmentSize() = 0;
	virtual U32 getBufferSize() = 0;
	virtual U32 getBufferCapacity() = 0;
};

#endif