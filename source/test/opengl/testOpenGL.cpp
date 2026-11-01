/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#if defined(__TEST) && defined(BOXEDWINE_OPENGL)
#include GLH
#include "../../opengl/glcommon.h"
#include "../../opengl/glMarshal.h"
#include "../../emulation/softmmu/kmemory_soft.h"
#include "../../emulation/softmmu/soft_ram.h"
#include "../cpu/testCPU.h"

void testOpenGLLinearMemory() {
    testNewInstruction(0);
    auto& ctx = testContext();
    KMemory* memory = ctx.memory;
    constexpr U32 address = 0xc4000000;
    // The untouched allocation must contain a complete native page group,
    // including on hosts with 64 KB pages.
    const U32 pageCount = ramPageLinearMemoryPageCount();
    const U32 length = std::max(4u, pageCount) * K_PAGE_SIZE;
    if (memory->mmap(ctx.thread, address, length, K_PROT_READ | K_PROT_WRITE,
        K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0) != address) {
        testFail("OpenGL linear test mmap failed");
        return;
    }
    U8* base = memory->getData()->linearMemoryBase;
    // Unaligned, untouched pages must be materialized before native GL reads.
    U8* direct = memory->getLinearRamPtr(address + 17, length - 34, false);
    if (base) {
        if (direct != base + address + 17) testFail("OpenGL did not return the linear alias");
        else for (U32 i = 0; i < length - 34; ++i)
            if (direct[i]) { testFail("OpenGL lazy pages were not zero initialized"); break; }
    } else if (direct) testFail("OpenGL linear path enabled without an aperture");

    memory->writed(address + K_PAGE_SIZE - 4, 0x12345678);
    memory->writed(address + K_PAGE_SIZE, 0x87654321);
    const GLuint* values = marshalArray<GLuint>(ctx.cpu, address + K_PAGE_SIZE - 4, 2);
    if (values[0] != 0x12345678 || values[1] != 0x87654321)
        testFail("OpenGL cross-page read changed data");
    if (base && (const U8*)values != base + address + K_PAGE_SIZE - 4)
        testFail("OpenGL input array copied instead of using its linear alias");
    {
        MarshalReadWrite<GLuint> output(ctx.cpu, address + K_PAGE_SIZE - 4, 2);
        GLuint* p = output.getPtr();
        if (base && (U8*)p != base + address + K_PAGE_SIZE - 4)
            testFail("OpenGL output array did not use its writable alias");
        p[0] = 0x11223344; p[1] = 0x55667788;
    }
    if (memory->readd(address + K_PAGE_SIZE) != 0x55667788)
        testFail("OpenGL direct output was not visible in guest memory");
    if (base) {
        U8* vertex = getGlContiguousMemory(ctx.cpu, address, length);
        if (vertex != base + address) testFail("OpenGL vertex span did not use linear memory");
        memory->unmap(address + K_PAGE_SIZE, K_PAGE_SIZE);
        if (memory->getLinearRamPtr(address, length, false))
            testFail("OpenGL accepted a hole in the linear span");
        memory->mmap(ctx.thread, address + K_PAGE_SIZE, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
            K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        memory->writed(address + K_PAGE_SIZE, 0xaabbccdd);
        // Replacing one guest page can make a larger native group impossible
        // to map directly. Do not dereference the old alias in that case.
        U8* remapped = memory->getLinearRamPtr(address, length, false);
        if ((pageCount == 1 && remapped != vertex) || (remapped && remapped != base + address))
            testFail("OpenGL linear address did not survive guest page reuse");
        const GLuint* reused = marshalArray<GLuint>(ctx.cpu, address, length / sizeof(GLuint));
        if (reused[K_PAGE_SIZE / sizeof(GLuint) - 1] != 0x11223344 ||
            reused[K_PAGE_SIZE / sizeof(GLuint)] != 0xaabbccdd)
            testFail("OpenGL remapped input lost data across the page boundary");
        {
            MarshalReadWrite<GLuint> output(ctx.cpu, address + K_PAGE_SIZE - 4, 2);
            GLuint* p = output.getPtr();
            p[0] = 0x99aabbcc; p[1] = 0xddeeff00;
        }
        if (memory->readd(address + K_PAGE_SIZE - 4) != 0x99aabbcc ||
            memory->readd(address + K_PAGE_SIZE) != 0xddeeff00)
            testFail("OpenGL remapped output lost data across the page boundary");
        memory->mprotect(ctx.thread, address, length, K_PROT_READ);
        if (memory->getLinearRamPtr(address, length, true))
            testFail("OpenGL exposed a read-only span for writes");
    }
    if (memory->getLinearRamPtr(0xfffffff0, 32, false) ||
        memory->getLinearRamPtr(address, 0, false))
        testFail("OpenGL accepted an empty or wrapping span");
    memory->unmap(address, length);

    {
        U32 fd = ctx.process->memfd_create(B("opengl-linear"), 0);
        if ((S32)fd < 0) { testFail("OpenGL file fixture creation failed"); return; }
        auto file = std::dynamic_pointer_cast<KFile>(ctx.process->getFileDescriptor(fd)->kobject);
        ctx.process->ftruncate64(fd, length);
        U32 original = 0x10203040;
        file->pwriteNative((U8*)&original, K_PAGE_SIZE, sizeof(original));
        memory->mmap(ctx.thread, address, length, K_PROT_READ | K_PROT_WRITE,
            K_MAP_FIXED | K_MAP_PRIVATE, fd, 0);
        // First-touch file mappings may also need the ordinary marshalling
        // path while the remaining pages in a native group are still lazy.
        const GLuint* input = marshalArray<GLuint>(ctx.cpu, address, length / sizeof(GLuint));
        if (input[K_PAGE_SIZE / sizeof(GLuint)] != original)
            testFail("OpenGL input did not load untouched file pages");
        if (base && pageCount == 1 && (const U8*)input != base + address)
            testFail("OpenGL file input did not use its linear alias");
        {
            MarshalReadWrite<GLuint> output(ctx.cpu, address + K_PAGE_SIZE, 1);
            GLuint* p = output.getPtr();
            if (base && pageCount == 1 && (U8*)p != base + address + K_PAGE_SIZE)
                testFail("OpenGL private file output did not use its writable alias");
            *p = 0x50607080;
        }
        U32 unchanged = 0;
        file->preadNative((U8*)&unchanged, K_PAGE_SIZE, sizeof(unchanged));
        if (unchanged != original || memory->readd(address + K_PAGE_SIZE) != 0x50607080)
            testFail("OpenGL private file output modified the file backing");
        memory->unmap(address, length);
        ctx.process->close(fd);
    }

    // Driver-owned native allocations are not necessarily in the aperture.
    std::vector<U8> native(3 * K_PAGE_SIZE, 0x5a);
    U32 mapped = memory->mapNativeMemory(native.data(), (U32)native.size());
    if (memory->getLinearRamPtr(mapped, (U32)native.size(), false))
        testFail("OpenGL accepted an unrepresentable native mapping");
    const GLubyte* fallback = marshalArray<GLubyte>(ctx.cpu, mapped, (U32)native.size());
    if (memcmp(fallback, native.data(), native.size())) testFail("OpenGL native fallback changed data");
    memory->unmapNativeMemory(mapped, (U32)native.size());
}

void testOpenGLLinearMemoryWrites() {
    testNewInstruction(0);
    auto& ctx = testContext();
    if (!ctx.memory->getData()->linearMemoryBase) return;
    constexpr U32 address = TEST_HEAP_ADDRESS + 0x2000;
    ctx.memory->writed(address, 0x11223344);
    KProcessPtr child = KProcess::create();
    child->memory = KMemory::create(child.get());
    child->memory->clone(ctx.memory, false);
    KThread* childThread = child->createThread();
    KThread::setCurrentThread(childThread);
    U8* output = child->memory->getLinearRamPtr(address, 4, true);
    if (!output) testFail("OpenGL writable linear span did not resolve COW");
    else *(U32*)output = 0x55667788;
    KThread::setCurrentThread(ctx.thread);
    if (ctx.memory->readd(address) != 0x11223344 || child->memory->readd(address) != 0x55667788)
        testFail("OpenGL linear output broke COW isolation");
    if (KSystem::getProcess(child->id)) KSystem::eraseProcess(child->id);
    child.reset();
    KThread::setCurrentThread(ctx.thread);

    // Native output must invalidate decoded instructions before overwriting code.
    testPushCode8(0x90); // nop
    testPushCode8(0xc3); // ret
    ctx.cpu->getNextOp();
    if (!ctx.memory->getDecodedOp(TEST_CODE_ADDRESS))
        testFail("OpenGL code-write fixture did not cache its instruction");
    {
        // Code pages may keep a read-only alias after invalidation. The output
        // helper must then use the prepared backing rather than that alias.
        MarshalReadWrite<GLubyte> output(ctx.cpu, TEST_CODE_ADDRESS, 1);
        *output.getPtr() = 0xc3;
    }
    if (ctx.memory->getDecodedOp(TEST_CODE_ADDRESS) || ctx.cpu->getNextOp()->inst != Retn32)
        testFail("OpenGL native write left stale decoded code");
}

void testOpenGLMarshalPointerWidths() {
    testNewInstruction(0);
    auto& ctx = testContext();
    for (U32 address : {TEST_HEAP_ADDRESS + 0x100, TEST_HEAP_ADDRESS + K_PAGE_SIZE - 4}) {
        ctx.memory->writed(address, 17);
        ctx.memory->writed(address + 4, 0xfffffff0);
        ctx.memory->writed(address + 8, 0x12345678);
        const GLintptr* offsets = marshalip(ctx.cpu, address, 2);
        if (offsets[0] != 17 || offsets[1] != -16)
            testFail("OpenGL GLintptr array was not widened and sign extended");
        const GLsizeiptr* sizes = marshalsip(ctx.cpu, address, 2);
        if (sizes[0] != 17 || sizes[1] != -16)
            testFail("OpenGL GLsizeiptr array was not widened and sign extended");
        U64 results[] = {0x11223344, 0x55667788};
        marshalBackArray<U64, 4>(ctx.cpu, results, address, 2);
        if (ctx.memory->readd(address) != 0x11223344 ||
            ctx.memory->readd(address + 4) != 0x55667788 ||
            ctx.memory->readd(address + 8) != 0x12345678)
            testFail("OpenGL pointer-sized output overwrote guest storage");
        ctx.memory->writed(address, TEST_HEAP_ADDRESS + 0x400);
        ctx.memory->writed(address + 4, TEST_HEAP_ADDRESS + 0x500);
        ctx.memory->writed(TEST_HEAP_ADDRESS + 0x400, 42);
        ctx.memory->writed(TEST_HEAP_ADDRESS + 0x500, 99);
        const GLvoid** pointers = marshalPointerArrayFixedSize(ctx.cpu, address, 2, 4);
        if (*(const U32*)pointers[0] != 42 || *(const U32*)pointers[1] != 99)
            testFail("OpenGL pointer array did not translate its guest elements");
        ctx.memory->writed(address, 0);
        ctx.memory->writed(address + 4, 0x20000);
        pointers = marshalPointerArrayByCount(ctx.cpu, address, 2, 0, 1, true);
        if (pointers[0] || (uintptr_t)pointers[1] != 0x20000)
            testFail("OpenGL buffer offsets were translated as guest pointers");
    }
}
#endif

#if defined(__TEST) && defined(__EMSCRIPTEN__) && defined(BOXEDWINE_MULTI_THREADED) && defined(BOXEDWINE_OPENGL_SDL) && defined(BOXEDWINE_OPENGL_BOOTSTRAP_TEST_ONLY)

#include <SDL.h>
#include GLH

#include "../../opengl/emscriptenGLProcAddress.h"
#include "../../opengl/glcommon.h"
#include "../../opengl/sdl/sdlgl.h"
#include "../cpu/testCPU.h"

void testEmscriptenMtOpenGLProcAddressBootstrap() {
    SDL_Window* window = SDL_CreateWindow(
        "BoxedWine OpenGL bootstrap test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        64,
        64,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!window) {
        testFail("Emscripten MT GL proc bootstrap could not create window: %s",
                 SDL_GetError());
        return;
    }

    if (SDL_GL_GetCurrentContext()) {
        testFail("Emscripten MT GL proc bootstrap test requires no current context");
    }

    static const char* policySupported[] = {
        "glBindVertexArrayOES",
        "glColor4f",
        "glGetString",
    };
    for (const char* name : policySupported) {
        if (boxedwineIsUnsupportedEmscriptenGLProcAddress(name)) {
            testFail("Emscripten GL proc policy rejected supported %s", name);
        }
    }

    static const char* policyUnsupported[] = {
        nullptr,
        "glBeginQueryEXT",
        "glColor4xvOES",
        "glColorTable",
        "glConvolutionFilter2D",
        "glHistogram",
        "glMinmax",
        "glSeparableFilter2D",
    };
    for (const char* name : policyUnsupported) {
        if (!boxedwineIsUnsupportedEmscriptenGLProcAddress(name)) {
            testFail("Emscripten GL proc policy accepted unsupported %s",
                     name ? name : "<null>");
        }
        if (SDLGL::testGetOpenGLProcAddress(name)) {
            testFail("Emscripten SDL GL resolver returned unsupported %s",
                     name ? name : "<null>");
        }
    }

    static const char* required[] = {
        "glGetError",
        "glGetIntegerv",
        "glGetString",
    };
    for (const char* name : required) {
        if (!SDLGL::testGetOpenGLProcAddress(name)) {
            testFail("Emscripten SDL GL resolver did not load %s", name);
        }
        if (!glcommon_testOpenGLProcAddressAvailable(name)) {
            testFail("Emscripten MT GL proc bootstrap did not advertise %s", name);
        }
    }

    static const char* unsupported[] = {
        nullptr,
        "eglGetProcAddress",
        "glConvolutionFilter2D",
        "glDefinitelyNotARealBoxedWineFunction",
        "glXGetProcAddress",
    };
    for (const char* name : unsupported) {
        if (glcommon_testOpenGLProcAddressAvailable(name)) {
            testFail("Emscripten MT GL proc bootstrap advertised unsupported %s",
                     name ? name : "<null>");
        }
    }

    SDL_DestroyWindow(window);
}

#endif
