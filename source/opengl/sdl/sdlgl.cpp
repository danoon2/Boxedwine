/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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
#include "../../source/ui/mainui.h"
#include "../../platform/sdl/sdlcallback.h"

static std::atomic_int shownGlWindows;

typedef void (GLAPIENTRY *pfnglFinish)();
typedef void (GLAPIENTRY *pfnglFlush)();

static pfnglFinish pglFinish;
static pfnglFlush pglFlush;

class SDLGlWindow : public std::enable_shared_from_this<SDLGlWindow> {
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
    XDrawablePtr drawable;

    void destroy();
    void showWindow(bool show);
    static std::shared_ptr<SDLGlWindow> createWindow(const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, U32 cx, U32 cy);
};

typedef std::shared_ptr<SDLGlWindow> SDLGlWindowPtr;

void SDLGlWindow::destroy() {
    if (window) {
        DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
            if (window) {
                if (visible) {
                    shownGlWindows--;
                    if (shownGlWindows == 0) {
                        KNativeSystem::showScreen(true);
                    }
                }
                SDL_DestroyWindow(window);
                window = nullptr;
            }
        DISPATCH_MAIN_THREAD_BLOCK_END
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
        if (cx == (U32)dm.w && cy == (U32)dm.h) {
            sdlFlags |= SDL_WINDOW_BORDERLESS;
        }
    }
    S32 x = 0;
    S32 y = 0;
    KNativeSystem::getScreen()->getPos(x, y);
    SDL_Window* window = SDL_CreateWindow("OpenGL Window", x, y, cx, cy, sdlFlags);

    if (!window) {
        kwarn_fmt("Couldn't create window: %s", SDL_GetError());
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
    ~KOpenGLSdl() override;

    U32 glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContext) override;
    void glDestroyContext(KThread* thread, U32 contextId) override;
    bool glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) override;
    void glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) override;
    void glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) override;
    void glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) override;
    void glResizeWindow(const std::shared_ptr<XWindow>& wnd) override;
    bool isActive() override;

    GLPixelFormatPtr getFormat(U32 pixelFormatId) override;
    void warpMouse(int x, int y) override;
    U32 getLastUpdateTime() override;
    void hideCurrentWindow() override;

    std::weak_ptr<SDLGlWindow> currentWindow;
    U32 lastUpdateTime = 0;

    static U32 nextId;

    static BOXEDWINE_MUTEX contextMutex;
    static BHashTable<U32, SDLGlContextPtr> contextsById;    

    static BOXEDWINE_MUTEX windowMutex;
    static BHashTable<U32, SDLGlWindowPtr> sdlWindowById;    
};

typedef std::shared_ptr<KOpenGLSdl> KOpenGLSdlPtr;

U32 KOpenGLSdl::nextId = 1;

BOXEDWINE_MUTEX KOpenGLSdl::contextMutex;
BHashTable<U32, SDLGlContextPtr> KOpenGLSdl::contextsById;

BOXEDWINE_MUTEX KOpenGLSdl::windowMutex;
BHashTable<U32, SDLGlWindowPtr> KOpenGLSdl::sdlWindowById;

KOpenGLSdl::~KOpenGLSdl() {
    sdlWindowById.clear();
    contextsById.clear();
    shownGlWindows = 0;
}

void SDLGlWindow::showWindow(bool show) {        
    if (show) {        
        KOpenGLSdlPtr gl = std::dynamic_pointer_cast<KOpenGLSdl>(KNativeSystem::getOpenGL());
        if (gl) {
            gl->currentWindow = shared_from_this();
            gl->lastUpdateTime = KSystem::getMilliesSinceStart();
        }
    }
    if (show == visible) {
        return;
    }
    if (KSystem::videoOption != VIDEO_NO_WINDOW) {
        KNativeSystem::getCurrentInput()->runOnUiThread([this, show]() {
            if (!show) {
                shownGlWindows--;
                SDL_HideWindow(window);
                if (shownGlWindows == 0) {
                    KNativeSystem::showScreen(true);
                }
            } else {
                // This is a hack
                //
                // When wine mixes GDI and DirectDraw while using OpenGL, it will create pixmaps to hold the opengl and then mix it somehow with the GDI code. 
                // This code below will show the OpenGL pixmap with some artifacts from the missing GDI stuff, for example, Age of Empires 1, when creating a
                // new player, the player entry box is a GDI text box
                shownGlWindows++;
                if (shownGlWindows > 1) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KOpenGLSdl::windowMutex);
                    for (auto& w : KOpenGLSdl::sdlWindowById) {
                        if (w.value->visible) {
                            w.value->showWindow(false);
                        }
                    }
                }
                SDL_ShowWindow(window);
                SDL_RaiseWindow(window);                
                if (shownGlWindows == 1) {
                    KNativeSystem::showScreen(false);
                }
#if !defined(BOXEDWINE_DISABLE_UI) && !defined(__TEST) && defined(BOXEDWINE_UI_LAUNCH_IN_PROCESS)
                if (uiIsRunning()) {
                    uiShutdown();
                }
#endif
            }
            });
        visible = show;
    }
}

U32 KOpenGLSdl::glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContextId) {
    SDLGlWindowPtr window;
    
    KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
        window = SDLGlWindow::createWindow(pixelFormat, major, minor, profile, flags, 100, 100);
        });

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
    window->destroy(); // will run on main thread (thus block this thread for a bit)

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
        KNativeSystem::getCurrentInput()->runOnUiThread([&window, &wnd]() {
            SDL_SetWindowSize(window->window, wnd->width(), wnd->height());
            });
    }
}

void KOpenGLSdl::glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) {
    SDLGlWindowPtr window;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
        window = sdlWindowById.get(d->id);
    }
    if (window) {
        window->showWindow(true);
        SDL_GL_SwapWindow(window->window);
    }
}

void KOpenGLSdl::glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) {
    SDLGlWindowPtr window;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
        window = sdlWindowById.get(wnd->id);
    }
    if (!window) {
        KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
            window = SDLGlWindow::createWindow(cfg->glPixelFormat, 0, 0, 0, 0, wnd->width(), wnd->height());
            });
        window->drawable = wnd;
        sdlWindowById.set(wnd->id, window);
    }
}

void KOpenGLSdl::glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
    sdlWindowById.remove(wnd->id);
}

GLPixelFormatPtr KOpenGLSdl::getFormat(U32 pixelFormatId) {
    return PlatformOpenGL::getFormat(pixelFormatId);
}

void KOpenGLSdl::warpMouse(int x, int y) {
    DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
        SDLGlWindowPtr wnd = currentWindow.lock();
        if (wnd) {
            SDL_WarpMouseInWindow(wnd->window, x, y);
        }
    DISPATCH_MAIN_THREAD_BLOCK_END
}

U32 KOpenGLSdl::getLastUpdateTime() {
    return lastUpdateTime;
}

void KOpenGLSdl::hideCurrentWindow() {
    SDLGlWindowPtr wnd = currentWindow.lock();
    if (wnd) {
        currentWindow.reset();
        DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN    
        wnd->showWindow(false);
        DISPATCH_MAIN_THREAD_BLOCK_END
    }    
}

void SDLGL::iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback) {
    PlatformOpenGL::iterateFormats(callback);
}


static int sdlOpenExtensionsLoaded = false;

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)

#undef GL_FUNCTION_FMT
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG)

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
            if (context->currentWindow) {
                context->currentWindow->drawable = nullptr;
            }
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
                KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
                    window = SDLGlWindow::createWindow(context->pixelFormat, context->major, context->minor, context->profile, context->flags, d->width(), d->height());
                    });
                window->drawable = d;
                sdlWindowById.set(d->id, window);
            }
        }
        bool result = SDL_GL_MakeCurrent(window->window, context->context) == 0;
        if (result) {
            loadSdlExtensions();
            context->currentWindow = window;
            return true;
        } else {
            kwarn_fmt("KOpenGLSdl::glMakeCurrent SDL_GL_MakeCurrent failed: %s", SDL_GetError());
        }
    }
    return false;
}

// GLAPI void APIENTRY glFinish( void ) {
static void sdl_glFinish(CPU* cpu) {	
    pglFinish();
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
        if (context && context->currentWindow) {
            context->currentWindow->showWindow(true);
        }
    }
    pglFlush();	
}

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) pgl##func = (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);

#undef GL_FUNCTION_FMT
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG) pgl##func = (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) pgl##func = (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS)

static void initSdlOpenGL() {
    const char* openGL = nullptr;
    if (KSystem::openglLib.length()) {
        openGL = KSystem::openglLib.c_str();
    }
    SDL_GL_LoadLibrary(openGL);
    
#include "../glfunctions.h"

    int99Callback[Finish] = sdl_glFinish;
    int99Callback[Flush] = sdl_glFlush;

    pglFinish = (pfnglFinish)SDL_GL_GetProcAddress("glFinish");
    pglFlush = (pfnglFlush)SDL_GL_GetProcAddress("glFlush");
}

KOpenGLPtr SDLGL::create() {
    initSdlOpenGL();
    return std::make_shared<KOpenGLSdl>();
}

#endif
