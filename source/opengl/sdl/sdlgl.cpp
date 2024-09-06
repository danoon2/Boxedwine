/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL_SDL
#include "sdlgl.h"
#include <SDL_opengl.h>
#include "../glcommon.h"

#include <SDL.h>
#include "knativesystem.h"
#include "../../x11/x11.h"
#include "../source/ui/mainui.h"

class SDLGlWindow {
public:
    SDLGlWindow(SDL_Window* window, const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags) : window(window), pixelFormat(pixelFormat), major(major), minor(minor), profile(profile), flags(flags) {}
    ~SDLGlWindow() {
        destroy();
    }
    SDL_Window* window;

    const std::shared_ptr<GLPixelFormat> pixelFormat;
    const U32 major;
    const U32 minor;
    const U32 profile;
    const U32 flags;
    bool visible = false;

    void destroy();
    void show();
    static std::shared_ptr<SDLGlWindow> createWindow(const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, U32 cx, U32 cy);
};

typedef std::shared_ptr<SDLGlWindow> SDLGlWindowPtr;

void SDLGlWindow::show() {
    if (KSystem::videoEnabled && !visible) {
        SDL_ShowWindow(window);
        SDL_RaiseWindow(window);
        visible = true;
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST) && defined(BOXEDWINE_UI_LAUNCH_IN_PROCESS)
        if (uiIsRunning()) {
            uiShutdown();
        }
#endif
    }
}
void SDLGlWindow::destroy() {
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

SDLGlWindowPtr SDLGlWindow::createWindow(const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, U32 cx, U32 cy) {
    SDL_GL_ResetAttributes();

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, pixelFormat->pf.cRedBits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, pixelFormat->pf.cGreenBits);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, pixelFormat->pf.cBlueBits);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, pixelFormat->pf.cAlphaBits);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, pixelFormat->pf.cDepthBits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, pixelFormat->pf.cStencilBits);

    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, pixelFormat->pf.cAccumRedBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, pixelFormat->pf.cAccumGreenBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, pixelFormat->pf.cAccumBlueBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, pixelFormat->pf.cAccumAlphaBits);

    if (major) {
        if (major >= 3) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        }
        //#ifdef BOXEDWINE_MSVC
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        //#endif
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, (pixelFormat->pf.dwFlags & K_PFD_DOUBLEBUFFER) ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, (pixelFormat->pf.dwFlags & K_PFD_GENERIC_FORMAT) ? 0 : 1);

    if (pixelFormat->pf.dwFlags & K_PFD_SWAP_COPY) {
        kwarn("Boxedwine: pixel format swap copy not supported");
    }

    SDL_DisplayMode dm = { 0 };
    int sdlFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;

    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        if (cx == dm.w && cy == dm.h) {
            sdlFlags |= SDL_WINDOW_BORDERLESS;
        }
    }
    SDL_Window* window = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cx, cy, sdlFlags);

    if (!window) {
        kwarn("Couldn't create window: %s", SDL_GetError());
        return nullptr;
    }

    return std::make_shared<SDLGlWindow>(window, pixelFormat, major, minor, profile, flags);
}

class SDLGlContext {
public:
    SDLGlContext(U32 id, SDL_GLContext context, const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags) : id(id), context(context), pixelFormat(pixelFormat), major(major), minor(minor), profile(profile), flags(flags) {}
    SDLGlWindowPtr currentWindow;

    const U32 id;
    const SDL_GLContext context;
    const std::shared_ptr<GLPixelFormat> pixelFormat;
    const U32 major;
    const U32 minor;
    const U32 profile;
    const U32 flags;
};

typedef std::shared_ptr<SDLGlContext> SDLGlContextPtr;

class KOpenGLSdl : public KOpenGL {
public:
    U32 glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContext) override;
    void glDestroyContext(KThread* thread, U32 contextId) override;
    bool glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) override;
    void glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) override;
    void glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) override;
    void glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) override;
    void glResizeWindow(const std::shared_ptr<XWindow>& wnd) override;
    bool isActive() override;

    GLPixelFormatPtr getFormat(U32 pixelFormatId) override;
    
    static U32 nextId;

    static BOXEDWINE_MUTEX contextMutex;
    static BHashTable<U32, SDLGlContextPtr> contextsById;    

    static BOXEDWINE_MUTEX windowMutex;
    static BHashTable<U32, SDLGlWindowPtr> sdlWindowById;
};

U32 KOpenGLSdl::nextId = 1;

BOXEDWINE_MUTEX KOpenGLSdl::contextMutex;
BHashTable<U32, SDLGlContextPtr> KOpenGLSdl::contextsById;

BOXEDWINE_MUTEX KOpenGLSdl::windowMutex;
BHashTable<U32, SDLGlWindowPtr> KOpenGLSdl::sdlWindowById;

U32 KOpenGLSdl::glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContextId) {
    SDLGlWindowPtr window = SDLGlWindow::createWindow(pixelFormat, major, minor, profile, flags, 100, 100);
    SDLGlContextPtr restoreContext;
    bool needToRestore = false;

    if (sharedContextId) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
        SDLGlContextPtr sharedContext = contextsById.get(sharedContextId);
        if (sharedContext && thread->currentContext != sharedContextId) {
            if (thread->currentContext) {
                restoreContext = contextsById.get(thread->currentContext);
            }
            needToRestore = true;
            SDL_GL_MakeCurrent(window->window, sharedContext->context);
            SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        }
    }
    // Mac requires this on the main thread, but Windows make current will fail if its not on the same thread as create context    
#ifdef BOXEDWINE_MSVC
    SDL_GLContext context = SDL_GL_CreateContext(window->window);
#else
    SDL_GLContext context;
    KNativeSystem::getCurrentInput()->runOnUiThread([&context, &window]() {
        context = SDL_GL_CreateContext(window->window);
        });
#endif    

    if (needToRestore) {
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
        if (restoreContext) {
            SDL_GL_MakeCurrent(window->window, restoreContext->context);
        } else {
            SDL_GL_MakeCurrent(window->window, nullptr);
        }
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    U32 result = nextId++;
    SDLGlContextPtr sdlContext = std::make_shared<SDLGlContext>(result, context, pixelFormat, major, minor, profile, flags);

    contextsById.set(result, sdlContext);
    return result;
}

void KOpenGLSdl::glDestroyContext(KThread* thread, U32 contextId) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    SDLGlContextPtr context = contextsById.get(contextId);

    if (context) {
        SDL_GL_DeleteContext(context->context);
    }
    contextsById.remove(contextId);
}

bool KOpenGLSdl::isActive() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    return contextsById.size() > 0;
}

void KOpenGLSdl::glResizeWindow(const std::shared_ptr<XWindow>& wnd) {
    SDLGlWindowPtr window;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
        window = sdlWindowById.get(wnd->id);
    }
    if (window) {
        SDL_SetWindowSize(window->window, wnd->width(), wnd->height());
    }
}

void KOpenGLSdl::glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) {
    SDLGlWindowPtr window;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
        window = sdlWindowById.get(d->id);
    }
    if (window) {
        if (!window->visible) {
            window->show();
        }
        SDL_GL_SwapWindow(window->window);
    }
}

void KOpenGLSdl::glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) {
}

void KOpenGLSdl::glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
    sdlWindowById.remove(wnd->id);
}

GLPixelFormatPtr KOpenGLSdl::getFormat(U32 pixelFormatId) {
    return PlatformOpenGL::getFormat(pixelFormatId);
}

void SDLGL::iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback) {
    PlatformOpenGL::iterateFormats(callback);
}


static int sdlOpenExtensionsLoaded = false;

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) ext_gl##func = (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);

void glExtensionsLoaded();

static void loadSdlExtensions() {
    if (!sdlOpenExtensionsLoaded) {
        sdlOpenExtensionsLoaded = true;
        #include "../glfunctions.h"
        glExtensionsLoaded();
    }
}

bool KOpenGLSdl::glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) {
    SDLGlContextPtr context;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
        context = contextsById.get(contextId);
    }        

    if (!context) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
        context = contextsById.get(thread->currentContext);
        if (context) {            
            context->currentWindow = nullptr;
        }
        SDL_GL_MakeCurrent(nullptr, 0);
        return true;
    } else {
        SDLGlWindowPtr window;

        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
            window = sdlWindowById.get(d->id);
            if (window && context->pixelFormat->nativeId != window->pixelFormat->nativeId) {
                SDL_DestroyWindow(window->window);
                window = nullptr;
            }
            if (!window) {
                window = SDLGlWindow::createWindow(context->pixelFormat, context->major, context->minor, context->profile, context->flags, d->width(), d->height());
                sdlWindowById.set(d->id, window);
            }
        }
        bool result = SDL_GL_MakeCurrent(window->window, context->context) == 0;
        if (result) {
            loadSdlExtensions();
            context->currentWindow = window;
            return true;
        } else {
            kwarn("KOpenGLSdl::glMakeCurrent SDL_GL_MakeCurrent failed: %s", SDL_GetError());
        }
    }
    return false;
}

// GLAPI void APIENTRY glFinish( void ) {
static void sdl_glFinish(CPU* cpu) {	
    glFinish();
}

// GLAPI void APIENTRY glFlush( void ) {
static void sdl_glFlush(CPU* cpu) {
    KThread* thread = cpu->thread;
    if (thread->currentContext) {
        SDLGlContextPtr context;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KOpenGLSdl::contextMutex);
            context = KOpenGLSdl::contextsById.get(thread->currentContext);
        }
        if (context && context->currentWindow && !context->currentWindow->visible) {
            context->currentWindow->show();
        }
    }
    glFlush();	
}

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) pgl##func = gl##func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) pgl##func = gl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS)

static void initSdlOpenGL() {
#include "../glfunctions.h"

    int99Callback[Finish] = sdl_glFinish;
    int99Callback[Flush] = sdl_glFlush;
}

KOpenGLPtr SDLGL::create() {
    initSdlOpenGL();
    return std::make_shared<KOpenGLSdl>();
}

#endif
