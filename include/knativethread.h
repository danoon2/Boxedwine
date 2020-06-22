#ifndef __KNATIVETHREAD_H__
#define __KNATIVETHREAD_H__

typedef int (*KNativeThreadFunction) (void* data);

class KNativeThread {
protected:
	KNativeThread(KNativeThreadFunction pfn, const std::string& name, void* data) : pfn(pfn), name(name), data(data), nativeThread(NULL) {};
public:
	static KNativeThread* createAndStartThread(KNativeThreadFunction pfn, const std::string& name, void* data);
	static void sleep(U32 ms);

	int wait();

	KNativeThreadFunction pfn;	
	const std::string name;
	void* data;

protected:
	void* nativeThread;
};

#endif