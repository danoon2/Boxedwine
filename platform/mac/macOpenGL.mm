/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Native macOS OpenGL drawable support. SDL continues to own the Cocoa
 *  window and input loop, while CGL owns contexts and off-screen pbuffers.
 */

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <atomic>
#include <cstdio>
#include "platformtypes.h"
#include "macOpenGL.h"

static NSOpenGLContext* getContext(void* context) {
    return (__bridge NSOpenGLContext*)context;
}

void* macOpenGLCreateContext(U32 nativeId, int major, int minor, int profile, int flags, void* shareContext) {
    (void)minor;
    (void)profile;
    (void)flags;

    @autoreleasepool {
        CGLPixelFormatObj pixelFormat = (CGLPixelFormatObj)macOpenGLChoosePixelFormat(nativeId, major);
        if (!pixelFormat) {
            return nullptr;
        }

        CGLContextObj sharedCglContext = shareContext ? [getContext(shareContext) CGLContextObj] : nullptr;
        CGLContextObj cglContext = nullptr;
        CGLError error = CGLCreateContext(pixelFormat, sharedCglContext, &cglContext);
        CGLReleasePixelFormat(pixelFormat);
        if (error != kCGLNoError || !cglContext) {
            std::fprintf(stderr, "CGLCreateContext failed: %d %s\n", error, CGLErrorString(error));
            return nullptr;
        }

        NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithCGLContextObj:cglContext];
        CGLReleaseContext(cglContext);
        if (!context) {
            std::fprintf(stderr, "NSOpenGLContext initWithCGLContextObj failed\n");
            return nullptr;
        }
        return (__bridge_retained void*)context;
    }
}

void macOpenGLDestroyContext(void* context) {
    if (!context) {
        return;
    }
    @autoreleasepool {
        NSOpenGLContext* nativeContext = (__bridge_transfer NSOpenGLContext*)context;
        [nativeContext clearDrawable];
    }
}

static NSView* getWindowView(SDL_Window* window) {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!window || !SDL_GetWindowWMInfo(window, &info) || info.subsystem != SDL_SYSWM_COCOA) {
        std::fprintf(stderr, "SDL_GetWindowWMInfo failed: %s\n", SDL_GetError());
        return nil;
    }
    return [info.info.cocoa.window contentView];
}

bool macOpenGLSetWindow(void* context, SDL_Window* window) {
    if (!context || !window) {
        return false;
    }
    @autoreleasepool {
        if (![NSThread isMainThread]) {
            std::fprintf(stderr, "macOpenGLSetWindow must run on the main thread\n");
            return false;
        }
        NSView* view = getWindowView(window);
        if (!view) {
            return false;
        }
        NSOpenGLContext* nativeContext = getContext(context);
        if ([nativeContext view] != view) {
            [nativeContext setView:view];
        }
        [nativeContext update];
        return true;
    }
}

bool macOpenGLMakeCurrent(void* context) {
    if (!context) {
        return false;
    }
    @autoreleasepool {
        NSOpenGLContext* nativeContext = getContext(context);
        [nativeContext makeCurrentContext];
        return CGLGetCurrentContext() == [nativeContext CGLContextObj];
    }
}

bool macOpenGLMakeCurrentPbuffer(void* context, void* pbuffer) {
    if (!context || !pbuffer) {
        return false;
    }
    @autoreleasepool {
        NSOpenGLContext* nativeContext = getContext(context);
        [nativeContext clearDrawable];
        CGLContextObj cglContext = [nativeContext CGLContextObj];
        CGLError error = CGLSetPBuffer(cglContext, (CGLPBufferObj)pbuffer, 0, 0, 0);
        if (error == kCGLNoError) {
            error = CGLSetCurrentContext(cglContext);
        }
        if (error != kCGLNoError) {
            std::fprintf(stderr, "CGLSetPBuffer failed: %d %s\n", error, CGLErrorString(error));
            return false;
        }
        return true;
    }
}

void macOpenGLClearCurrent() {
    @autoreleasepool {
        [NSOpenGLContext clearCurrentContext];
    }
}

void macOpenGLSwapBuffers(void* context) {
    if (!context) {
        return;
    }
    @autoreleasepool {
        [getContext(context) flushBuffer];
    }
}

void macOpenGLUpdateContext(void* context) {
    if (!context) {
        return;
    }
    @autoreleasepool {
        [getContext(context) update];
    }
}

void macOpenGLDetachContext(void* context) {
    if (!context) {
        return;
    }
    @autoreleasepool {
        [getContext(context) clearDrawable];
    }
}

void* macOpenGLCreatePbuffer(U32 width, U32 height) {
    static std::atomic_int nativePbufferSupport{-1};
    if (nativePbufferSupport.load() == 0) {
        return nullptr;
    }
    CGLPBufferObj pbuffer = nullptr;
    struct PbufferFormat {
        GLenum target;
        GLenum format;
    } formats[] = {
        {GL_TEXTURE_RECTANGLE_ARB, GL_RGB},
        {GL_TEXTURE_RECTANGLE_ARB, GL_RGBA},
        {GL_TEXTURE_2D, GL_RGB},
        {GL_TEXTURE_2D, GL_RGBA},
    };
    CGLError error = kCGLBadDrawable;
    for (const PbufferFormat& format : formats) {
        error = CGLCreatePBuffer(width, height, format.target, format.format, 0, &pbuffer);
        if (error == kCGLNoError && pbuffer) {
            nativePbufferSupport.store(1);
            return pbuffer;
        }
    }
    nativePbufferSupport.store(0);
    std::fprintf(stderr, "CGL pbuffers are unavailable (%d %s); using hidden Cocoa drawables\n",
                 error, CGLErrorString(error));
    return nullptr;
}

void macOpenGLDestroyPbuffer(void* pbuffer) {
    if (pbuffer) {
        CGLReleasePBuffer((CGLPBufferObj)pbuffer);
    }
}

#ifdef __TEST
extern "C" int macOpenGLPbufferSmokeTest() {
    for (U32 nativeId = 1; nativeId < 1024; nativeId++) {
        bool supportsPbuffer = false;
        bool sampleBuffers = false;
        U32 samples = 0;
        U32 maxWidth = 0;
        U32 maxHeight = 0;
        U32 maxPixels = 0;
        if (!macOpenGLGetPixelFormatInfo(nativeId, &supportsPbuffer, &sampleBuffers, &samples,
                                         &maxWidth, &maxHeight, &maxPixels)) {
            break;
        }
        if (!supportsPbuffer) {
            continue;
        }

        void* context = macOpenGLCreateContext(nativeId, 0, 0, 0, 0, nullptr);
        void* pbuffer = macOpenGLCreatePbuffer(16, 16);
        SDL_Window* fallbackWindow = nullptr;
        bool initializedVideo = false;
        bool success = context && pbuffer && macOpenGLMakeCurrentPbuffer(context, pbuffer);
        if (context && !pbuffer) {
            if (!(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO)) {
                initializedVideo = SDL_InitSubSystem(SDL_INIT_VIDEO) == 0;
            }
            fallbackWindow = SDL_CreateWindow("Boxedwine pbuffer test", 0, 0, 16, 16, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
            success = fallbackWindow && macOpenGLSetWindow(context, fallbackWindow) && macOpenGLMakeCurrent(context);
        }
        if (success) {
            glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glFinish();
            success = glGetString(GL_VERSION) != nullptr && glGetError() == GL_NO_ERROR;
        }
        macOpenGLClearCurrent();
        macOpenGLDestroyPbuffer(pbuffer);
        if (fallbackWindow) {
            SDL_DestroyWindow(fallbackWindow);
        }
        if (initializedVideo) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
        macOpenGLDestroyContext(context);
        return success ? 1 : 0;
    }
    return 0;
}
#endif
#endif
