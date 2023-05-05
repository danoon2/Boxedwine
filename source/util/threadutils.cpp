#include "boxedwine.h"
#if !defined (__EMSCRIPTEN__) && !defined (__TEST)
#include <thread>

void runInBackgroundThread(std::function<void(void)> f) {
    std::thread t(f);
    t.detach();
}
#endif
