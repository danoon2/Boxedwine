/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// Linked only into ASan multithreaded WASM JIT builds. ASan replaces the
// pthread start argument with its thread object before calling the builtin
// pthread_create. Preserve the original argument for the module broker without
// bypassing ASan's thread registration or depending on its private object layout.
#include <pthread.h>
#include <cerrno>
#include <cstdint>
#include <mutex>
#include <new>
#include <unordered_map>

static thread_local void* originalStartArg;
static thread_local bool insidePthreadCreate;
static std::mutex startArgsMutex;
static std::unordered_map<uintptr_t, uintptr_t> originalStartArgs;

extern "C" int __real_pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*);
extern "C" int __real_emscripten_builtin_pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*);

extern "C" int __wrap_pthread_create(pthread_t* thread, const pthread_attr_t* attr,
        void* (*entry)(void*), void* arg) {
    void* savedArg = originalStartArg;
    bool savedInside = insidePthreadCreate;
    originalStartArg = arg;
    insidePthreadCreate = true;
    int result = __real_pthread_create(thread, attr, entry, arg);
    originalStartArg = savedArg;
    insidePthreadCreate = savedInside;
    return result;
}

extern "C" int __wrap_emscripten_builtin_pthread_create(pthread_t* thread, const pthread_attr_t* attr,
        void* (*entry)(void*), void* arg) {
    if (insidePthreadCreate) {
        try {
            std::lock_guard<std::mutex> lock(startArgsMutex);
            originalStartArgs[(uintptr_t)arg] = (uintptr_t)originalStartArg;
        } catch (const std::bad_alloc&) {
            return ENOMEM;
        }
    }
    // Dispatch can consume the mapping synchronously on the main thread.
    // Never hold startArgsMutex across pthread creation.
    int result = __real_emscripten_builtin_pthread_create(thread, attr, entry, arg);
    if (result && insidePthreadCreate) {
        std::lock_guard<std::mutex> lock(startArgsMutex);
        originalStartArgs.erase((uintptr_t)arg);
    }
    return result;
}

extern "C" uintptr_t boxedwine_asan_take_thread_start_arg(uintptr_t arg) {
    std::lock_guard<std::mutex> lock(startArgsMutex);
    auto found = originalStartArgs.find(arg);
    if (found == originalStartArgs.end()) {
        return arg;
    }
    uintptr_t original = found->second;
    originalStartArgs.erase(found);
    return original;
}
