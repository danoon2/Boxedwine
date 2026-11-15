// Standalone regression for the production SDL callback queue. Link only the
// callback, synchronization, and string sources; no emulated process is needed.
#include "boxedwine.h"
#include <SDL.h>
#include "platform/sdl/sdlcallback.h"
#include <chrono>
#include <cstdio>
#include <stdexcept>

thread_local KThread* KThread::runningThread = nullptr;
// Only synchronization state is used here; this fixture deliberately has no
// guest CPU, memory, process registration, or thread cleanup side effects.
KThread::KThread(U32 id, const KProcessPtr& process) : id(id), process(process), memory(nullptr), cpu(nullptr) {}
KThread::~KThread() = default;
U32 sdlCustomEvent;
static std::thread::id mainThread;
bool isMainthread() { return std::this_thread::get_id() == mainThread; }
U32 KSystem::emulatedMilliesToHost(U32 ms) { return ms; }
void internal_kpanic(BString msg) { throw std::runtime_error(msg.c_str()); }
void kpanic(const char* msg) { throw std::runtime_error(msg); }

static void require(bool value, const char* message) {
    if (!value) {
        fprintf(stderr, "FAIL: %s\n", message);
        std::abort();
    }
}

static SdlCallback* takeCallback() {
    SDL_Event event{};
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    do {
        SDL_PumpEvents();
        if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, sdlCustomEvent, sdlCustomEvent) == 1) {
            return static_cast<SdlCallback*>(event.user.data1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (std::chrono::steady_clock::now() < deadline);
    require(false, "callback was not queued");
    return nullptr;
}

static void notifyWithoutCompletion(SdlCallback* callback) {
    std::lock_guard<std::mutex> lock(callback->cond->m);
    callback->cond->signalAll();
}

static void testUnrelatedWakeupAndReuse() {
    for (U32 cycle = 0; cycle != 20; ++cycle) {
        std::atomic<bool> returned{false};
        U32 calls = 0;
        U32 result = 0;
        std::thread worker([&] {
            result = sdlDispatch([&]() -> U32 { ++calls; return cycle + 42; });
            returned.store(true, std::memory_order_release);
        });
        SdlCallback* callback = takeCallback();
        notifyWithoutCompletion(callback);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        require(!returned.load(std::memory_order_acquire), "notification completed queued work early");
        callback->run();
        worker.join();
        require(calls == 1, "callback did not execute exactly once");
        require(result == cycle + 42, "reused callback returned an old result");
    }
}

static void testWakeupDuringExecution() {
    std::atomic<bool> executing{false};
    std::atomic<bool> mayFinish{false};
    std::atomic<bool> returned{false};
    U32 capturedStackValue = 0;
    U32 result = 0;
    std::thread worker([&] {
        result = sdlDispatch([&]() -> U32 {
            executing.store(true, std::memory_order_release);
            while (!mayFinish.load(std::memory_order_acquire)) std::this_thread::yield();
            capturedStackValue = 73;
            return capturedStackValue;
        });
        returned.store(true, std::memory_order_release);
    });
    SdlCallback* callback = takeCallback();
    std::thread notifier([&] {
        while (!executing.load(std::memory_order_acquire)) std::this_thread::yield();
        notifyWithoutCompletion(callback);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        require(!returned.load(std::memory_order_acquire), "notification released executing captures");
        mayFinish.store(true, std::memory_order_release);
    });
    callback->run();
    notifier.join();
    worker.join();
    require(capturedStackValue == 73 && result == 73, "callback result was not published");
}

static void testTermination(bool alreadyTerminating) {
    KThread thread(123, nullptr);
    thread.terminating = alreadyTerminating;
    std::atomic<bool> returned{false};
    U32 result = 0;
    std::thread worker([&] {
        KThread::setCurrentThread(&thread);
        result = sdlDispatch([]() -> U32 { return 83; });
        returned.store(true, std::memory_order_release);
        KThread::setCurrentThread(nullptr);
    });
    SdlCallback* callback = takeCallback();
    {
        std::lock_guard<std::recursive_mutex> lock(thread.waitingCondSync);
        thread.terminating = true;
    }
    notifyWithoutCompletion(callback);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    require(!returned.load(std::memory_order_acquire), "termination discarded pending UI work");
    callback->run();
    worker.join();
    require(result == 83 && thread.terminating, "termination or callback result was lost");
}

static int rejectCallback(void*, SDL_Event* event) {
    return event->type != sdlCustomEvent;
}

static void testQueueRejection() {
    SDL_SetEventFilter(rejectCallback, nullptr);
    bool rejected = false;
    std::thread worker([&] {
        try {
            sdlDispatch([]() -> U32 { return 99; });
        } catch (const std::runtime_error& error) {
            rejected = std::string(error.what()) == "Failed to queue SDL callback";
        }
    });
    worker.join();
    SDL_SetEventFilter(nullptr, nullptr);
    require(rejected, "queue rejection did not report a failure");
    testUnrelatedWakeupAndReuse();
}

int main() {
    mainThread = std::this_thread::get_id();
    require(SDL_Init(SDL_INIT_EVENTS) == 0, "SDL event initialization failed");
    sdlCustomEvent = SDL_RegisterEvents(1);
    require(sdlCustomEvent != static_cast<U32>(-1), "SDL custom event registration failed");
    U32 inlineCalls = 0;
    require(sdlDispatch([&]() -> U32 { ++inlineCalls; return 17; }) == 17 && inlineCalls == 1,
        "main-thread callback did not execute inline");
    testUnrelatedWakeupAndReuse();
    testWakeupDuringExecution();
    testTermination(false);
    testTermination(true);
    testQueueRejection();
    SDL_Quit();
    puts("PASS: inline execution, queued and executing wakeups, termination, callback reuse, queue rejection");
}
