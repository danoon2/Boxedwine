#ifndef __KNATIVESYNCHRONIZATION_H__
#define __KNATIVESYNCHRONIZATION_H__

class KNativeMutex;

class KNativeCondition {
public:
	KNativeCondition();
	~KNativeCondition();

	void signal();
	void signalAll();
	void wait(KNativeMutex& m);
	void waitWithTimeout(KNativeMutex& m, U32 ms);
private:
	void* c;
};

class KNativeMutex {
public:
	KNativeMutex();
	~KNativeMutex();

	void lock();
	bool tryLock();
	void unlock();
private:
	friend class KNativeCondition;
	void* m;
};

#endif