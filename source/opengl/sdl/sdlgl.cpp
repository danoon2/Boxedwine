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
#include "../../platform/sdl/knativescreenSDL.h"
#include "../../x11/x11.h"
#include "../../source/ui/mainui.h"
#include "../../platform/sdl/sdlcallback.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/html5_webgl.h>
#include <string>
#endif

static std::atomic_int shownGlWindows;

typedef void (GLAPIENTRY *pfnglFinish)();
typedef void (GLAPIENTRY *pfnglFlush)();

static pfnglFinish pglFinish;
static pfnglFlush pglFlush;

#ifdef __EMSCRIPTEN__
// With PROXY_TO_PTHREAD and OffscreenCanvas, SDL's Emscripten GL path still
// routes context creation through the browser main thread. Create the WebGL
// context directly on BoxedWine's SDL/main-loop pthread instead.
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE toWebGLContext(SDL_GLContext context) {
    return (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE)(uintptr_t)context;
}

static SDL_GLContext toSDLGLContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context) {
    return (SDL_GLContext)(uintptr_t)context;
}

static bool useThreadWebGLCanvas() {
    const char* value = getenv("BOXEDWINE_WEBGL_THREAD_CANVAS");
    return !value || !value[0] || value[0] != '0';
}

EM_JS(bool, boxedwineRegisterThreadWebGLCanvas, (const char* selector, int width, int height), {
    var selectorString = UTF8ToString(selector);
    var id = selectorString[0] === '#' ? selectorString.slice(1) : selectorString;
    if (!id || typeof GL === 'undefined') {
        return false;
    }
    if (typeof OffscreenCanvas !== 'undefined') {
        if (!GL.offscreenCanvases[id]) {
            var canvas = new OffscreenCanvas(width || 1, height || 1);
            canvas.id = id;
            GL.offscreenCanvases[id] = {
                canvas: canvas,
                offscreenCanvas: canvas,
                canvasSharedPtr: 0,
                id: id
            };
        } else {
            var existing = GL.offscreenCanvases[id].canvas || GL.offscreenCanvases[id].offscreenCanvas;
            if (existing) {
                var nextWidth = width || existing.width || 1;
                var nextHeight = height || existing.height || 1;
                if (existing.width !== nextWidth) {
                    existing.width = nextWidth;
                }
                if (existing.height !== nextHeight) {
                    existing.height = nextHeight;
                }
            }
        }
        if (!Module.__boxedwineOffscreenCanvasLookup && typeof findCanvasEventTarget === 'function') {
            var previousFindCanvasEventTarget = findCanvasEventTarget;
            findCanvasEventTarget = function(target) {
                var targetString = target > 2 ? UTF8ToString(target) : target;
                if (typeof targetString === 'string') {
                    var canvasId = targetString[0] === '#' ? targetString.slice(1) : targetString;
                    var offscreen = GL.offscreenCanvases && GL.offscreenCanvases[canvasId];
                    if (offscreen) {
                        return offscreen.canvas || offscreen.offscreenCanvas;
                    }
                }
                return previousFindCanvasEventTarget(target);
            };
            Module.__boxedwineOffscreenCanvasLookup = true;
        }
        return true;
    } else if (typeof document !== 'undefined') {
        var domCanvas = document.getElementById(id);
        if (!domCanvas) {
            domCanvas = document.createElement('canvas');
            domCanvas.id = id;
            domCanvas.style.display = 'none';
            document.body.appendChild(domCanvas);
        }
        domCanvas.width = width || domCanvas.width || 1;
        domCanvas.height = height || domCanvas.height || 1;
        return true;
    }
    return false;
});

EM_JS(void, boxedwineUnregisterThreadWebGLCanvas, (const char* selector), {
    if (typeof GL === 'undefined' || !GL.offscreenCanvases) {
        return;
    }
    var selectorString = UTF8ToString(selector);
    var id = selectorString[0] === '#' ? selectorString.slice(1) : selectorString;
    if (id) {
        delete GL.offscreenCanvases[id];
    }
});

EM_JS(void, boxedwinePresentCurrentWebGLFrame, (int requestedWidth, int requestedHeight), {
    if (typeof GL === 'undefined' || !GL.currentContext || !GL.currentContext.GLctx) {
        return;
    }
    var presentState = null;
    if (typeof SharedArrayBuffer !== 'undefined' && typeof Atomics !== 'undefined') {
        if (!Module.__boxedwinePresentState) {
            Module.__boxedwinePresentState = new Int32Array(new SharedArrayBuffer(4));
        }
        presentState = Module.__boxedwinePresentState;
        if (Atomics.compareExchange(presentState, 0, 0, 1) !== 0) {
            return;
        }
    }
    var source = GL.currentContext.GLctx.canvas;
    if (!source) {
        if (presentState) {
            Atomics.store(presentState, 0, 0);
        }
        return;
    }
    var sx = 0;
    var sy = 0;
    var sw = source.width;
    var sh = source.height;
    try {
        var requestedPresentWidth = requestedWidth | 0;
        var requestedPresentHeight = requestedHeight | 0;
        var screenPresentWidth = Module.__boxedwineWebGLPresentWidth | 0;
        var screenPresentHeight = Module.__boxedwineWebGLPresentHeight | 0;
        var presentWidth = 0;
        var presentHeight = 0;
        if (requestedPresentWidth > 0 && requestedPresentHeight > 0 && requestedPresentWidth <= source.width && requestedPresentHeight <= source.height) {
            presentWidth = requestedPresentWidth;
            presentHeight = requestedPresentHeight;
        }
        if (screenPresentWidth > 0 && screenPresentHeight > 0 && screenPresentWidth <= source.width && screenPresentHeight <= source.height
                && (presentWidth <= 0 || screenPresentWidth * screenPresentHeight < presentWidth * presentHeight)) {
            presentWidth = screenPresentWidth;
            presentHeight = screenPresentHeight;
        }
        if (presentWidth > 0 && presentHeight > 0 && presentWidth <= source.width && presentHeight <= source.height) {
            sw = presentWidth;
            sh = presentHeight;
            sy = Math.max(0, source.height - sh);
        } else {
            var gl = GL.currentContext.GLctx;
            var viewport = gl.getParameter(gl.VIEWPORT);
            if (viewport && viewport.length >= 4 && viewport[2] > 0 && viewport[3] > 0 && viewport[2] <= source.width && viewport[3] <= source.height) {
                sx = Math.max(0, viewport[0] | 0);
                sw = Math.min(source.width - sx, viewport[2] | 0);
                sh = Math.min(source.height, viewport[3] | 0);
                sy = Math.max(0, source.height - ((viewport[1] | 0) + sh));
            }
        }
    } catch (e) {
        sx = 0;
        sy = 0;
        sw = source.width;
        sh = source.height;
    }
    if (typeof document !== 'undefined') {
        var target = document.getElementById('boxedwine-webgl-canvas-0');
        if (target) {
            if (target.width !== sw) {
                target.width = sw;
            }
            if (target.height !== sh) {
                target.height = sh;
            }
            var canvasFrame = target.parentElement;
            if (canvasFrame) {
                canvasFrame.style.setProperty("--boxedwine-canvas-width", target.width || 800);
                canvasFrame.style.setProperty("--boxedwine-canvas-height", target.height || 600);
            }
            var context = target.getContext('2d');
            if (context) {
                context.drawImage(source, sx, sy, sw, sh, 0, 0, sw, sh);
            }
        }
        if (presentState) {
            Atomics.store(presentState, 0, 0);
        }
        return;
    }
    if (typeof source.transferToImageBitmap !== 'function') {
        if (presentState) {
            Atomics.store(presentState, 0, 0);
        }
        return;
    }
    try {
        var bitmap = source.transferToImageBitmap();
        postMessage({
            cmd: 'callHandler',
            handler: 'boxedwinePresentFrame',
            args: [bitmap, presentState, sx, sy, sw, sh]
        }, [bitmap]);
    } catch (e) {
        if (presentState) {
            Atomics.store(presentState, 0, 0);
        }
        if (!Module.__boxedwinePresentErrorLogged) {
            Module.__boxedwinePresentErrorLogged = true;
            console.error('BOXEDWINE_WEBGL present failed: ' + e);
        }
    }
});

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE createWebGLContextForTarget(const char* target, U32 major, U32 minor) {
    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);
    (void)minor;
    attributes.majorVersion = major >= 2 ? 2 : 1;
    attributes.minorVersion = 0;
    attributes.stencil = true;
    attributes.antialias = false;
    attributes.alpha = false;
    attributes.premultipliedAlpha = false;
    attributes.enableExtensionsByDefault = true;
#ifdef BOXEDWINE_MULTI_THREADED
    attributes.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW;
#endif
    return emscripten_webgl_create_context(target, &attributes);
}

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE createWebGLContext(U32 major, U32 minor) {
    return createWebGLContextForTarget("#canvas", major, minor);
}
#endif

#ifdef __EMSCRIPTEN__
static void resizeWebGLCanvas(SDL_Window* window, U32 cx, U32 cy) {
    if (!cx || !cy) {
        return;
    }
    int currentWidth = 0;
    int currentHeight = 0;
    bool canvasMatches = emscripten_get_canvas_element_size("#canvas", &currentWidth, &currentHeight) == EMSCRIPTEN_RESULT_SUCCESS
            && currentWidth == (int)cx && currentHeight == (int)cy;
    bool webglCanvasMatches = emscripten_get_canvas_element_size("#boxedwine-webgl-canvas-0", &currentWidth, &currentHeight) == EMSCRIPTEN_RESULT_SUCCESS
            && currentWidth == (int)cx && currentHeight == (int)cy;
    if (canvasMatches && webglCanvasMatches) {
        return;
    }
    SDL_SetWindowSize(window, cx, cy);
    if (!canvasMatches) {
        emscripten_set_canvas_element_size("#canvas", cx, cy);
    }
    if (!webglCanvasMatches) {
        emscripten_set_canvas_element_size("#boxedwine-webgl-canvas-0", cx, cy);
    }
}

static bool deferGlWindowShowUntilSwap() {
#ifdef BOXEDWINE_MULTI_THREADED
    return false;
#else
    return true;
#endif
}

static bool useDirectEmscriptenWebGLContext(U32 width, U32 height, const XWindowPtr& wnd) {
#ifdef BOXEDWINE_MULTI_THREADED
    return true;
#else
    (void)wnd;
    (void)width;
    (void)height;
    return false;
#endif
}

static bool useSharedThreadWebGLCanvas(U32 width, U32 height, const XWindowPtr& wnd) {
#ifdef BOXEDWINE_MULTI_THREADED
    (void)wnd;
    (void)width;
    (void)height;
    return true;
#else
    (void)wnd;
    (void)width;
    (void)height;
    return false;
#endif
}

#endif

class SDLGlWindow : public std::enable_shared_from_this<SDLGlWindow> {
public:
    SDLGlWindow(SDL_Window* window, const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, bool ownsWindow = true) : window(window), pixelFormat(pixelFormat), major(major), minor(minor), profile(profile), flags(flags), ownsWindow(ownsWindow) {}
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
    XDrawablePtr drawable;
    U32 forceForegroundUntil = 0;

    void destroy();
    void showWindow(bool show);
    static std::shared_ptr<SDLGlWindow> createWindow(const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, U32 cx, U32 cy, bool useDirectContext);
};

typedef std::shared_ptr<SDLGlWindow> SDLGlWindowPtr;

void SDLGlWindow::destroy() {
    if (window) {
        DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
            if (window) {
                if (ownsWindow && visible) {
                    shownGlWindows--;
                    if (shownGlWindows == 0) {
                        KNativeSystem::showScreen(true);
                    }
                }
                if (ownsWindow) {
                    SDL_DestroyWindow(window);
                }
                window = nullptr;
            }
        DISPATCH_MAIN_THREAD_BLOCK_END
    }
}

SDLGlWindowPtr SDLGlWindow::createWindow(const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags, U32 cx, U32 cy, bool useDirectContext) {
#ifdef __EMSCRIPTEN__
    if (useDirectContext) {
        KNativeScreenSDLPtr screen = std::dynamic_pointer_cast<KNativeScreenSDL>(KNativeSystem::getScreen());
        if (!screen || !screen->window) {
            kwarn("Couldn't get main SDL window for Emscripten WebGL");
            return nullptr;
        }
        screen->destroyTextureCache();
        if (screen->renderer) {
            SDL_DestroyRenderer(screen->renderer);
            screen->renderer = nullptr;
        }
        // The offscreen canvas can only have one browser-side owner. Reuse the
        // main SDL window record instead of creating another SDL GL window.
        resizeWebGLCanvas(screen->window, cx, cy);
        return std::make_shared<SDLGlWindow>(screen->window, pixelFormat, 3, 0, profile | BOXEDWINE_GL_PROFILE_ES, flags, false);
    }
#endif
#ifdef __EMSCRIPTEN__
    major = 3;
    minor = 0;
    profile |= BOXEDWINE_GL_PROFILE_ES;
#endif
    SDL_GL_ResetAttributes();

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#else
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
#endif

#ifdef _DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    if (major) {
        if (profile & BOXEDWINE_GL_PROFILE_ES) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        } else if (major >= 3) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        }
        //#ifdef BOXEDWINE_MSVC
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        //#endif
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, (pixelFormat->pf.dwFlags & K_PFD_DOUBLEBUFFER) ? 1 : 0);
#ifndef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, (pixelFormat->pf.dwFlags & K_PFD_GENERIC_FORMAT) ? 0 : 1);
#endif

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
#ifdef __EMSCRIPTEN__
    resizeWebGLCanvas(window, cx, cy);
#endif
    return std::make_shared<SDLGlWindow>(window, pixelFormat, major, minor, profile, flags);
}

class SDLGlContext {
public:
    SDLGlContext(U32 id, SDL_GLContext context, const std::shared_ptr<GLPixelFormat>& pixelFormat, U32 major, U32 minor, U32 profile, U32 flags) : id(id), context(context), pixelFormat(pixelFormat), major(major), minor(minor), profile(profile), flags(flags) {}
    SDLGlWindowPtr currentWindow;

    const U32 id;
    SDL_GLContext context;
    const std::shared_ptr<GLPixelFormat> pixelFormat;
    const U32 major;
    const U32 minor;
    const U32 profile;
    const U32 flags;
#ifdef __EMSCRIPTEN__
    std::string canvasSelector;
    bool threadCanvas = false;
    bool sharedThreadCanvas = false;
    bool webglContext = false;
#endif
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
    U32 now = KSystem::getMilliesSinceStart();
    if (show) {
        KOpenGLSdlPtr gl = std::dynamic_pointer_cast<KOpenGLSdl>(KNativeSystem::getOpenGL());
        if (gl) {
            gl->currentWindow = shared_from_this();
            gl->lastUpdateTime = now;
        }
    }
#ifdef __EMSCRIPTEN__
    if (!ownsWindow) {
        visible = show;
        return;
    }
#endif
    if (show == visible) {
        if (show && KSystem::videoOption != VIDEO_NO_WINDOW && (KNativeSystem::getScreen()->isVisible() || now < forceForegroundUntil)) {
            KNativeSystem::getCurrentInput()->runOnUiThread([this]() {
                SDL_RaiseWindow(window);
                if (ownsWindow && KNativeSystem::getScreen()->isVisible()) {
                    KNativeSystem::showScreen(false);
                }
                });
        }
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
                forceForegroundUntil = 0;
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
                forceForegroundUntil = KSystem::getMilliesSinceStart() + 2000;
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
#ifdef __EMSCRIPTEN__
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    U32 result = nextId++;
    SDLGlContextPtr sdlContext = std::make_shared<SDLGlContext>(result, nullptr, pixelFormat, major, minor, profile, flags);
    contextsById.set(result, sdlContext);
    return result;
#else
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
#endif
}

void KOpenGLSdl::glDestroyContext(KThread* thread, U32 contextId) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    SDLGlContextPtr context = contextsById.get(contextId);

#ifdef __EMSCRIPTEN__
    if (context && context->webglContext) {
        if (thread && thread->currentContext == contextId) {
            emscripten_webgl_make_context_current(0);
        }
        if (!context->sharedThreadCanvas) {
            emscripten_webgl_destroy_context(toWebGLContext(context->context));
        }
        if (context->threadCanvas && !context->sharedThreadCanvas && context->canvasSelector.length()) {
            boxedwineUnregisterThreadWebGLCanvas(context->canvasSelector.c_str());
        }
    }
    /* SDL's Emscripten backend, and our shared worker canvas path, may hand
     * multiple Wine GL context records the same underlying WebGL context.
     * Deleting one temporary Wine context can destroy the browser context
     * currently used by a later WineD3D caps context. Only destroy direct
     * WebGL contexts that own a private worker canvas. */
#else
    if (context) {
        SDL_GL_DeleteContext(context->context);
    }
#endif
    contextsById.remove(contextId);
}

bool KOpenGLSdl::isActive() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
    for (auto& w : sdlWindowById) {
        if (w.value->visible) {
            return true;
        }
    }
    return false;
}

bool KOpenGLSdl::presentedSinceLastCheck() {
    bool result = presented;
    presented = false;
    return result;
}

void KOpenGLSdl::glResizeWindow(const std::shared_ptr<XWindow>& wnd) {
    SDLGlWindowPtr window;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
        window = sdlWindowById.get(wnd->id);
    }
    if (window) {
        KNativeSystem::getCurrentInput()->runOnUiThread([&window, &wnd]() {
#if defined(__EMSCRIPTEN__)
            resizeWebGLCanvas(window->window, wnd->width(), wnd->height());
#else
            SDL_SetWindowSize(window->window, wnd->width(), wnd->height());
#endif
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
#ifdef __EMSCRIPTEN__
        SDLGlContextPtr currentContext;
        if (thread && thread->currentContext) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
            currentContext = contextsById.get(thread->currentContext);
        }
        bool useDirectWebGLContext = currentContext && currentContext->webglContext;
        bool usePrivateThreadCanvas = currentContext && currentContext->threadCanvas && useThreadWebGLCanvas();
#endif
#ifdef __EMSCRIPTEN__
        if (useDirectWebGLContext) {
            if (pglFlush) {
                pglFlush();
            }
            if (usePrivateThreadCanvas) {
                boxedwinePresentCurrentWebGLFrame(d->width(), d->height());
            }
        } else {
            if (pglFlush) {
                pglFlush();
            }
            SDL_GL_SwapWindow(window->window);
        }
        if (!usePrivateThreadCanvas) {
            resizeWebGLCanvas(window->window, d->width(), d->height());
        }
#else
        if (pglFlush) {
            pglFlush();
        }
        SDL_GL_SwapWindow(window->window);
#ifdef __EMSCRIPTEN__
        resizeWebGLCanvas(window->window, d->width(), d->height());
#endif
#endif
        presented = true;
#ifdef __EMSCRIPTEN__
        if (thread && thread->cpu) {
            thread->cpu->yield = true;
        }
#endif
    }
}

void KOpenGLSdl::glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) {
    SDLGlWindowPtr window;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
        window = sdlWindowById.get(wnd->id);
    }
    if (!window) {
        bool useDirectContext = useDirectEmscriptenWebGLContext(wnd->width(), wnd->height(), wnd);
        KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
            window = SDLGlWindow::createWindow(cfg->glPixelFormat, 0, 0, 0, 0, wnd->width(), wnd->height(), useDirectContext);
            });
        window->drawable = wnd;
        sdlWindowById.set(wnd->id, window);
#ifdef __EMSCRIPTEN__
        if (!deferGlWindowShowUntilSwap())
#endif
        window->showWindow(true);
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
        DISPATCH_MAIN_THREAD_BLOCK_BEGIN
        wnd->showWindow(false);
        DISPATCH_MAIN_THREAD_BLOCK_END
    }    
}

void SDLGL::iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback) {
    PlatformOpenGL::iterateFormats(callback);
}


static int sdlOpenExtensionsLoaded = false;

#ifdef __EMSCRIPTEN__
static bool isUnsupportedEmscriptenSdlProcAddress(const char* name) {
    if (!strcmp(name, "glBeginQueryEXT")
            || !strcmp(name, "glDeleteQueriesEXT")
            || !strcmp(name, "glEndQueryEXT")
            || !strcmp(name, "glGenQueriesEXT")
            || !strcmp(name, "glGetQueryivEXT")
            || !strcmp(name, "glGetQueryObjecti64vEXT")
            || !strcmp(name, "glGetQueryObjectivEXT")
            || !strcmp(name, "glGetQueryObjectui64vEXT")
            || !strcmp(name, "glGetQueryObjectuivEXT")
            || !strcmp(name, "glIsQueryEXT")
            || !strcmp(name, "glQueryCounterEXT")) {
        return true;
    }

    return false;
}
#endif

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)

#undef GL_FUNCTION_FMT
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG)

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)

#undef GL_EXT_FUNCTION
#ifdef __EMSCRIPTEN__
#define GL_EXT_FUNCTION(func, RET, PARAMS) ext_gl##func = isUnsupportedEmscriptenSdlProcAddress("gl" #func) ? nullptr : (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);
#else
#define GL_EXT_FUNCTION(func, RET, PARAMS) ext_gl##func = (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);
#endif

void glExtensionsLoaded();

static void loadSdlExtensions() {
    if (!sdlOpenExtensionsLoaded) {
        sdlOpenExtensionsLoaded = true;
        #include "../glfunctions.h"
        #include "../glfunctions_ext.h"
        glExtensionsLoaded();
    }
}

#ifdef _DEBUG
void enableDebug();
#endif

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
#ifdef __EMSCRIPTEN__
        if (context && context->webglContext) {
            emscripten_webgl_make_context_current(0);
        } else {
            KNativeSystem::getCurrentInput()->runOnUiThread([]() {
                SDL_GL_MakeCurrent(nullptr, 0);
                });
        }
#else
        SDL_GL_MakeCurrent(nullptr, 0);
#endif
        return true;
    } else {
        SDLGlWindowPtr window;

        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
            window = sdlWindowById.get(d->id);
            if (window && context->pixelFormat->nativeId != window->pixelFormat->nativeId) {
                window->destroy();
                sdlWindowById.remove(d->id);
                window = nullptr;
            }
            if (!window) {
                XWindowPtr drawableWindow;
                if (d && d->isWindow) {
                    drawableWindow = std::dynamic_pointer_cast<XWindow>(d);
                }
                bool useDirectContext = useDirectEmscriptenWebGLContext(d->width(), d->height(), drawableWindow);
                KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
                    window = SDLGlWindow::createWindow(context->pixelFormat, context->major, context->minor, context->profile, context->flags, d->width(), d->height(), useDirectContext);
                    });
                if (!window || !window->window) {
                    kwarn("KOpenGLSdl::glMakeCurrent failed to create an SDL GL window");
                    return false;
                }
                window->drawable = d;
                sdlWindowById.set(d->id, window);
            }
        }
        if (!context->context) {
#ifdef __EMSCRIPTEN__
            XWindowPtr drawableWindow;
            if (d && d->isWindow) {
                drawableWindow = std::dynamic_pointer_cast<XWindow>(d);
            }
            bool useDirectContext = useDirectEmscriptenWebGLContext(d->width(), d->height(), drawableWindow);
            if (useDirectContext) {
                if (useThreadWebGLCanvas()) {
                    context->sharedThreadCanvas = useSharedThreadWebGLCanvas(d->width(), d->height(), drawableWindow);
                    context->canvasSelector = context->sharedThreadCanvas
                            ? "#boxedwine-webgl-thread-shared"
                            : "#boxedwine-webgl-thread-" + std::to_string(context->id);
                    if (boxedwineRegisterThreadWebGLCanvas(context->canvasSelector.c_str(), d->width(), d->height())) {
                        context->context = toSDLGLContext(createWebGLContextForTarget(context->canvasSelector.c_str(), context->major, context->minor));
                        context->threadCanvas = context->context != nullptr;
                        context->webglContext = context->context != nullptr;
                    }
                }
                if (!context->context) {
                    KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
                        context->context = toSDLGLContext(createWebGLContext(context->major, context->minor));
                        });
                    context->threadCanvas = context->context != nullptr;
                    context->webglContext = context->context != nullptr;
                }
            }
            if (!context->context) {
                KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
                    context->context = SDL_GL_CreateContext(window->window);
                    });
                context->threadCanvas = false;
                context->webglContext = false;
            }
            if (!context->context) {
                kwarn("KOpenGLSdl::glMakeCurrent Emscripten GL context creation failed");
                return false;
            }
#else
            KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
                context->context = SDL_GL_CreateContext(window->window);
#if defined(__EMSCRIPTEN__) && defined(BOXEDWINE_MULTI_THREADED)
                resizeWebGLCanvas(window->window, d->width(), d->height());
#endif
                });
            if (!context->context) {
                kwarn_fmt("KOpenGLSdl::glMakeCurrent SDL_GL_CreateContext failed: %s", SDL_GetError());
                return false;
            }
#endif
        }
        bool result = false;
#ifdef __EMSCRIPTEN__
        if (context->threadCanvas && useThreadWebGLCanvas() && context->canvasSelector.length()) {
            boxedwineRegisterThreadWebGLCanvas(context->canvasSelector.c_str(), d->width(), d->height());
        }
        if (context->webglContext) {
            result = emscripten_webgl_make_context_current(toWebGLContext(context->context)) == EMSCRIPTEN_RESULT_SUCCESS;
        } else {
            KNativeSystem::getCurrentInput()->runOnUiThread([&]() {
                result = SDL_GL_MakeCurrent(window->window, context->context) == 0;
                });
        }
#else
        result = SDL_GL_MakeCurrent(window->window, context->context) == 0;
#endif
        if (result) {
            loadSdlExtensions();
#ifdef _DEBUG
            enableDebug();
#endif
            context->currentWindow = window;
            window->showWindow(true);
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
#ifdef __EMSCRIPTEN__
            if (!deferGlWindowShowUntilSwap())
#endif
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

#ifdef _DEBUG
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

void enableDebug() {
    int glFlags;
    pglGetIntegerv(GL_CONTEXT_FLAGS, &glFlags);
    if (glFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        pglEnable(GL_DEBUG_OUTPUT);
        pglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        ext_glDebugMessageCallback(glDebugOutput, nullptr);
        ext_glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
}

#endif

#endif
