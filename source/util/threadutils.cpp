#include "boxedwine.h"
#if !defined (__EMSCRIPTEN__) && !defined (__TEST)
#include "Poco/Runnable.h"
#include "Poco/Thread.h"

class BackgroundThread : public Poco::Runnable
{
public:
    BackgroundThread(std::function<void(void)> f, Poco::Thread* thread) : Runnable(), f(f), thread(thread) {}

    virtual void run() {
        f();
        delete this->thread; // which will delete this
    }
private:
    std::function<void(void)> f;
    Poco::Thread* thread;
};

void runInBackgroundThread(std::function<void(void)> f) {
    Poco::Thread* t = new Poco::Thread();
    t->start(new BackgroundThread(f, t));
}
#endif
