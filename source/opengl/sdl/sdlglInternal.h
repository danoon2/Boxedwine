/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef BOXEDWINE_SDLGL_INTERNAL_H
#define BOXEDWINE_SDLGL_INTERNAL_H

#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL_SDL
#include <SDL.h>
#include "kopengl.h"

class SDLGlWindow : public std::enable_shared_from_this<SDLGlWindow> {
public:
    SDLGlWindow(SDL_Window* window, const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, bool ownsWindow = true, const std::shared_ptr<XWindow>& inputWindow = nullptr) : window(window), pixelFormat(pixelFormat), major(major), minor(minor), profile(profile), flags(flags), ownsWindow(ownsWindow), inputWindow(inputWindow) {}
    ~SDLGlWindow() {
        destroy();
    }
    SDL_Window* window;

    const std::shared_ptr<GLPixelFormat> pixelFormat;
    const U32 major;
    const U32 minor;
    const U32 profile;
    const U32 flags;
    const bool ownsWindow;
    bool visible = false;
    std::shared_ptr<XDrawable> drawable;
    // Context unbinding clears drawable, but the presented window still owns
    // input until GDI or another GL window is shown.
    std::weak_ptr<XWindow> inputWindow;
    U32 forceForegroundUntil = 0;
#ifndef __EMSCRIPTEN__
    U64 nextSwapTime = 0;
    U64 swapLogStart = 0;
    U32 swapLogFrames = 0;
    bool swapIntervalWarning = false;
    void paceSwap(S32 interval);
#endif

    void destroy();
    void showWindow(bool show);
    static std::shared_ptr<SDLGlWindow> createWindow(const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, U32 cx, U32 cy, std::shared_ptr<XWindow> wnd);
};

typedef std::shared_ptr<SDLGlWindow> SDLGlWindowPtr;

class SDLGlContext;
typedef std::shared_ptr<SDLGlContext> SDLGlContextPtr;

class KOpenGLSdl : public KOpenGL {
public:
    ~KOpenGLSdl() override;

    U32 glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContext) override;
    void glDestroyContext(KThread* thread, U32 contextId) override;
    bool glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) override;
#if defined(__EMSCRIPTEN__) && !defined(BOXEDWINE_MULTI_THREADED)
    bool glRestoreCurrentContext(KThread* thread) override;
#endif
    void glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) override;
    void glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) override;
    void glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) override;
    void glResizeWindow(const std::shared_ptr<XWindow>& wnd) override;
    bool glCreatePbuffer(KThread* thread, const std::shared_ptr<XDrawable>& pbuffer, const CLXFBConfigPtr& cfg) override;
    void glDestroyPbuffer(KThread* thread, const std::shared_ptr<XDrawable>& pbuffer) override;
    bool isActive() override;
    bool presentedSinceLastCheck() override;

    GLPixelFormatPtr getFormat(U32 pixelFormatId) override;
    void warpMouse(int x, int y) override;
    U32 getLastUpdateTime() override;
    void hideCurrentWindow() override;

    std::weak_ptr<SDLGlWindow> currentWindow;
    U32 lastUpdateTime = 0;
    bool presented = false;

    static U32 nextId;

    static BOXEDWINE_MUTEX contextMutex;
    static BHashTable<U32, SDLGlContextPtr> contextsById;
#ifdef __EMSCRIPTEN__
    // GDI presentation runs on the UI thread. Query this without taking the
    // context lock, which a guest thread may hold while awaiting a UI callback.
    static std::atomic<bool> hasContexts;
#endif

    static BOXEDWINE_MUTEX windowMutex;
    static BHashTable<U32, SDLGlWindowPtr> sdlWindowById;

    static BOXEDWINE_MUTEX pbufferMutex;
    static BHashTable<U32, void*> pbuffersById;
};

typedef std::shared_ptr<KOpenGLSdl> KOpenGLSdlPtr;

#endif
#endif
