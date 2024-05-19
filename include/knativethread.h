#ifndef __KNATIVETHREAD_H__
#define __KNATIVETHREAD_H__

typedef int (*KNativeThreadFunction) (void* data);

class KNativeThread {
protected:
	KNativeThread(KNativeThreadFunction pfn, BString name, void* data) : pfn(pfn), name(name), data(data), nativeThread(nullptr) {};
public:
	static KNativeThread* createAndStartThread(KNativeThreadFunction pfn, BString name, void* data);
	static void sleep(U32 ms);

	int wait();

	KNativeThreadFunction pfn;	
	const BString name;
	void* data;

protected:
	void* nativeThread;
};

#endif