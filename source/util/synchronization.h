#ifndef __SYNCHRONIZATION_H__
#define __SYNCHRONIZATION_H__

#ifdef BOXEDWINE_MULTI_THREADED
class BoxedWineMutex {
public:
    BoxedWineMutex();
    ~BoxedWineMutex();

    void lock();
    void unlock();

private:
    void* m;
};

class BoxedWineCriticalSection {
public:
    BoxedWineCriticalSection(BoxedWineMutex* mutex);
    ~BoxedWineCriticalSection();

private:
    BoxedWineMutex* mutex;
};

class BoxedWineCondition {
public:
    BoxedWineCondition();
    ~BoxedWineCondition();

    void lock();
    void signal();
    void signalAll();
    void wait();
    void unlock();

private:
    void* m;
    void* c;
};

#define BOXEDWINE_CRITICAL_SECTION static BoxedWineMutex csMutex; BoxedWineCriticalSection boxedWineCriticalSection(&csMutex);
#define BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(csMutex) BoxedWineCriticalSection boxedWineCriticalSection(&csMutex);
#define BOXEDWINE_MUTEX BoxedWineMutex
#define BOXEDWINE_MUTEX_LOCK(mutex) mutex.lock()
#define BOXEDWINE_MUTEX_UNLOCK(mutex) mutex.unlock()

#define BOXEDWINE_CONDITION BoxedWineCondition
#define BOXEDWINE_CONDITION_LOCK(cond) cond.lock()
#define BOXEDWINE_CONDITION_UNLOCK(cond) cond.unlock()
#define BOXEDWINE_CONDITION_SIGNAL(cond) cond.signal()
#define BOXEDWINE_CONDITION_SIGNAL_ALL(cond) cond.signalAll()
#define BOXEDWINE_CONDITION_WAIT(cond) cond.wait()

#else
#define BOXEDWINE_CRITICAL_SECTION
#define BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(csMutex)

typedef void* BOXEDWINE_MUTEX;
#define BOXEDWINE_MUTEX_LOCK(mutex)
#define BOXEDWINE_MUTEX_UNLOCK(mutex)

typedef void* BOXEDWINE_CONDITION;
#define BOXEDWINE_CONDITION_LOCK(x)
#define BOXEDWINE_CONDITION_UNLOCK(x)
#define BOXEDWINE_CONDITION_SIGNAL(x)
#define BOXEDWINE_CONDITION_SIGNAL_ALL(x)
#define BOXEDWINE_CONDITION_WAIT(x)
#endif

#endif