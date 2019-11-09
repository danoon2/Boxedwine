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

#include <SDL.h>
#include "sdlwindow.h"
#include "kscheduler.h"
#include "crc.h"
#include "devfb.h"
#include "devinput.h"
#include "kunixsocket.h"
#include "multiThreaded/sdlcallback.h"
#include "../emulation/hardmmu/hard_memory.h"

int bits_per_pixel = 32;
int default_horz_res = 800;
int default_vert_res = 600;
int default_bits_per_pixel = 32;
int sdlScaleX = 100;
int sdlScaleY = 100;
int rel_mouse_sensitivity = 100;
bool relativeMouse = false;
const char* sdlScaleQuality = "0";
extern bool videoEnabled;

static int firstWindowCreated;
#ifdef __ANDROID__
U32 sdlFullScreen = true;
#else
U32 sdlFullScreen;
#endif

static std::unordered_map<std::string, SDL_Cursor*> cursors;
static std::unordered_map<U32, Wnd*> hwndToWnd;
SDL_Color sdlPalette[256];
SDL_Color sdlSystemPalette[256] = {
    {0x00,0x00,0x00},
    {0x80,0x00,0x00},
    {0x00,0x80,0x00},
    {0x80,0x80,0x00},
    {0x00,0x00,0x80},
    {0x80,0x00,0x80},
    {0x00,0x80,0x80},
    {0xC0,0xC0,0xC0},
    {0xC0,0xDC,0xC0},
    {0xA6,0xCA,0xF0},
    {0x2A,0x3F,0xAA},
    {0x2A,0x3F,0xFF},
    {0x2A,0x5F,0x00},
    {0x2A,0x5F,0x55},
    {0x2A,0x5F,0xAA},
    {0x2A,0x5F,0xFF},
    {0x2A,0x7F,0x00},
    {0x2A,0x7F,0x55},
    {0x2A,0x7F,0xAA},
    {0x2A,0x7F,0xFF},
    {0x2A,0x9F,0x00},
    {0x2A,0x9F,0x55},
    {0x2A,0x9F,0xAA},
    {0x2A,0x9F,0xFF},
    {0x2A,0xBF,0x00},
    {0x2A,0xBF,0x55},
    {0x2A,0xBF,0xAA},
    {0x2A,0xBF,0xFF},
    {0x2A,0xDF,0x00},
    {0x2A,0xDF,0x55},
    {0x2A,0xDF,0xAA},
    {0x2A,0xDF,0xFF},
    {0x2A,0xFF,0x00},
    {0x2A,0xFF,0x55},
    {0x2A,0xFF,0xAA},
    {0x2A,0xFF,0xFF},
    {0x55,0x00,0x00},
    {0x55,0x00,0x55},
    {0x55,0x00,0xAA},
    {0x55,0x00,0xFF},
    {0x55,0x1F,0x00},
    {0x55,0x1F,0x55},
    {0x55,0x1F,0xAA},
    {0x55,0x1F,0xFF},
    {0x55,0x3F,0x00},
    {0x55,0x3F,0x55},
    {0x55,0x3F,0xAA},
    {0x55,0x3F,0xFF},
    {0x55,0x5F,0x00},
    {0x55,0x5F,0x55},
    {0x55,0x5F,0xAA},
    {0x55,0x5F,0xFF},
    {0x55,0x7F,0x00},
    {0x55,0x7F,0x55},
    {0x55,0x7F,0xAA},
    {0x55,0x7F,0xFF},
    {0x55,0x9F,0x00},
    {0x55,0x9F,0x55},
    {0x55,0x9F,0xAA},
    {0x55,0x9F,0xFF},
    {0x55,0xBF,0x00},
    {0x55,0xBF,0x55},
    {0x55,0xBF,0xAA},
    {0x55,0xBF,0xFF},
    {0x55,0xDF,0x00},
    {0x55,0xDF,0x55},
    {0x55,0xDF,0xAA},
    {0x55,0xDF,0xFF},
    {0x55,0xFF,0x00},
    {0x55,0xFF,0x55},
    {0x55,0xFF,0xAA},
    {0x55,0xFF,0xFF},
    {0x7F,0x00,0x00},
    {0x7F,0x00,0x55},
    {0x7F,0x00,0xAA},
    {0x7F,0x00,0xFF},
    {0x7F,0x1F,0x00},
    {0x7F,0x1F,0x55},
    {0x7F,0x1F,0xAA},
    {0x7F,0x1F,0xFF},
    {0x7F,0x3F,0x00},
    {0x7F,0x3F,0x55},
    {0x7F,0x3F,0xAA},
    {0x7F,0x3F,0xFF},
    {0x7F,0x5F,0x00},
    {0x7F,0x5F,0x55},
    {0x7F,0x5F,0xAA},
    {0x7F,0x5F,0xFF},
    {0x7F,0x7F,0x00},
    {0x7F,0x7F,0x55},
    {0x7F,0x7F,0xAA},
    {0x7F,0x7F,0xFF},
    {0x7F,0x9F,0x00},
    {0x7F,0x9F,0x55},
    {0x7F,0x9F,0xAA},
    {0x7F,0x9F,0xFF},
    {0x7F,0xBF,0x00},
    {0x7F,0xBF,0x55},
    {0x7F,0xBF,0xAA},
    {0x7F,0xBF,0xFF},
    {0x7F,0xDF,0x00},
    {0x7F,0xDF,0x55},
    {0x7F,0xDF,0xAA},
    {0x7F,0xDF,0xFF},
    {0x7F,0xFF,0x00},
    {0x7F,0xFF,0x55},
    {0x7F,0xFF,0xAA},
    {0x7F,0xFF,0xFF},
    {0xAA,0x00,0x00},
    {0xAA,0x00,0x55},
    {0xAA,0x00,0xAA},
    {0xAA,0x00,0xFF},
    {0xAA,0x1F,0x00},
    {0xAA,0x1F,0x55},
    {0xAA,0x1F,0xAA},
    {0xAA,0x1F,0xFF},
    {0xAA,0x3F,0x00},
    {0xAA,0x3F,0x55},
    {0xAA,0x3F,0xAA},
    {0xAA,0x3F,0xFF},
    {0xAA,0x5F,0x00},
    {0xAA,0x5F,0x55},
    {0xAA,0x5F,0xAA},
    {0xAA,0x5F,0xFF},
    {0xAA,0x7F,0x00},
    {0xAA,0x7F,0x55},
    {0xAA,0x7F,0xAA},
    {0xAA,0x7F,0xFF},
    {0xAA,0x9F,0x00},
    {0xAA,0x9F,0x55},
    {0xAA,0x9F,0xAA},
    {0xAA,0x9F,0xFF},
    {0xAA,0xBF,0x00},
    {0xAA,0xBF,0x55},
    {0xAA,0xBF,0xAA},
    {0xAA,0xBF,0xFF},
    {0xAA,0xDF,0x00},
    {0xAA,0xDF,0x55},
    {0xAA,0xDF,0xAA},
    {0xAA,0xDF,0xFF},
    {0xAA,0xFF,0x00},
    {0xAA,0xFF,0x55},
    {0xAA,0xFF,0xAA},
    {0xAA,0xFF,0xFF},
    {0xD4,0x00,0x00},
    {0xD4,0x00,0x55},
    {0xD4,0x00,0xAA},
    {0xD4,0x00,0xFF},
    {0xD4,0x1F,0x00},
    {0xD4,0x1F,0x55},
    {0xD4,0x1F,0xAA},
    {0xD4,0x1F,0xFF},
    {0xD4,0x3F,0x00},
    {0xD4,0x3F,0x55},
    {0xD4,0x3F,0xAA},
    {0xD4,0x3F,0xFF},
    {0xD4,0x5F,0x00},
    {0xD4,0x5F,0x55},
    {0xD4,0x5F,0xAA},
    {0xD4,0x5F,0xFF},
    {0xD4,0x7F,0x00},
    {0xD4,0x7F,0x55},
    {0xD4,0x7F,0xAA},
    {0xD4,0x7F,0xFF},
    {0xD4,0x9F,0x00},
    {0xD4,0x9F,0x55},
    {0xD4,0x9F,0xAA},
    {0xD4,0x9F,0xFF},
    {0xD4,0xBF,0x00},
    {0xD4,0xBF,0x55},
    {0xD4,0xBF,0xAA},
    {0xD4,0xBF,0xFF},
    {0xD4,0xDF,0x00},
    {0xD4,0xDF,0x55},
    {0xD4,0xDF,0xAA},
    {0xD4,0xDF,0xFF},
    {0xD4,0xFF,0x00},
    {0xD4,0xFF,0x55},
    {0xD4,0xFF,0xAA},
    {0xD4,0xFF,0xFF},
    {0xFF,0x00,0x55},
    {0xFF,0x00,0xAA},
    {0xFF,0x1F,0x00},
    {0xFF,0x1F,0x55},
    {0xFF,0x1F,0xAA},
    {0xFF,0x1F,0xFF},
    {0xFF,0x3F,0x00},
    {0xFF,0x3F,0x55},
    {0xFF,0x3F,0xAA},
    {0xFF,0x3F,0xFF},
    {0xFF,0x5F,0x00},
    {0xFF,0x5F,0x55},
    {0xFF,0x5F,0xAA},
    {0xFF,0x5F,0xFF},
    {0xFF,0x7F,0x00},
    {0xFF,0x7F,0x55},
    {0xFF,0x7F,0xAA},
    {0xFF,0x7F,0xFF},
    {0xFF,0x9F,0x00},
    {0xFF,0x9F,0x55},
    {0xFF,0x9F,0xAA},
    {0xFF,0x9F,0xFF},
    {0xFF,0xBF,0x00},
    {0xFF,0xBF,0x55},
    {0xFF,0xBF,0xAA},
    {0xFF,0xBF,0xFF},
    {0xFF,0xDF,0x00},
    {0xFF,0xDF,0x55},
    {0xFF,0xDF,0xAA},
    {0xFF,0xDF,0xFF},
    {0xFF,0xFF,0x55},
    {0xFF,0xFF,0xAA},
    {0xCC,0xCC,0xFF},
    {0xFF,0xCC,0xFF},
    {0x33,0xFF,0xFF},
    {0x66,0xFF,0xFF},
    {0x99,0xFF,0xFF},
    {0xCC,0xFF,0xFF},
    {0x00,0x7F,0x00},
    {0x00,0x7F,0x55},
    {0x00,0x7F,0xAA},
    {0x00,0x7F,0xFF},
    {0x00,0x9F,0x00},
    {0x00,0x9F,0x55},
    {0x00,0x9F,0xAA},
    {0x00,0x9F,0xFF},
    {0x00,0xBF,0x00},
    {0x00,0xBF,0x55},
    {0x00,0xBF,0xAA},
    {0x00,0xBF,0xFF},
    {0x00,0xDF,0x00},
    {0x00,0xDF,0x55},
    {0x00,0xDF,0xAA},
    {0x00,0xDF,0xFF},
    {0x00,0xFF,0x55},
    {0x00,0xFF,0xAA},
    {0x2A,0x00,0x00},
    {0x2A,0x00,0x55},
    {0x2A,0x00,0xAA},
    {0x2A,0x00,0xFF},
    {0x2A,0x1F,0x00},
    {0x2A,0x1F,0x55},
    {0x2A,0x1F,0xAA},
    {0x2A,0x1F,0xFF},
    {0x2A,0x3F,0x00},
    {0x2A,0x3F,0x55},
    {0xFF,0xFB,0xF0},
    {0xA0,0xA0,0xA4},
    {0x80,0x80,0x80},
    {0xFF,0x00,0x00},
    {0x00,0xFF,0x00},
    {0xFF,0xFF,0x00},
    {0x00,0x00,0xFF},
    {0xFF,0x00,0xFF},
    {0x00,0xFF,0xFF},
    {0xFF,0xFF,0xFF},
};

static void displayChanged(KThread* thread);

void initSDL() {
    default_horz_res = screenCx;
    default_vert_res = screenCy;
}

bool isBoxedWineDriverActive() {
    return hwndToWnd.size()!=0;
}

Wnd* getWnd(U32 hwnd) {
    if (hwndToWnd.count(hwnd))
        return hwndToWnd[hwnd];
    return NULL;
}

static Wnd* getWndFromPoint(int x, int y) {
    for (auto& n : hwndToWnd) {
        Wnd* wnd = n.second;
        if (x>=wnd->windowRect.left && x<=wnd->windowRect.right && y>=wnd->windowRect.top && y<=wnd->windowRect.bottom && wnd->surface) {
            return wnd;
        }
    }
    return NULL;
}

static Wnd* getFirstVisibleWnd() {
    for (auto& n : hwndToWnd) {
        Wnd* wnd = n.second;
#ifdef SDL2
        if (wnd->sdlTextureWidth || wnd->openGlContext) {
#else
        if (wnd->sdlSurface) {
#endif
            return wnd;
        }
    }
    return NULL;
}

#ifdef SDL2
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_GLContext sdlCurrentContext;
BOXEDWINE_MUTEX sdlMutex;
int contextCount;
bool sdlWindowIsGL;

static void destroySDL2(KThread* thread) {
    for (auto& n : hwndToWnd) {
        Wnd* wnd = n.second;
        if (wnd->sdlTexture) {
            SDL_DestroyTexture((SDL_Texture*)wnd->sdlTexture);
            wnd->sdlTexture = NULL;
            wnd->sdlTextureHeight = 0;
            wnd->sdlTextureWidth = 0;
        }
    }

#ifdef SDL2
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = 0;
    }
    // :TODO: what about other threads?
    thread->removeAllGlContexts();
#endif
    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = 0;
        sdlWindowIsGL = false;
    }    
    contextCount = 0;
}
#else
SDL_Surface* surface;
#endif

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
void loadExtensions();
#endif

void sdlDeleteContext(KThread* thread, U32 contextId) {    
#ifdef SDL2
    KThreadGlContext* threadContext = thread->getGlContextById(contextId);
    if (threadContext && threadContext->context) {
        SDL_GL_DeleteContext(threadContext->context);
        thread->removeGlContextById(contextId);
        contextCount--;
        if (contextCount==0) {
            DISPATCH_MAIN_THREAD_BLOCK_BEGIN
            displayChanged(thread);
            DISPATCH_MAIN_THREAD_BLOCK_END
        }
    }
#endif
}

void sdlUpdateContextForThread(KThread* thread) {
#ifdef SDL2
    if (thread->currentContext && thread->currentContext!=sdlCurrentContext) {
        SDL_GL_MakeCurrent(sdlWindow, thread->currentContext);
        sdlCurrentContext = thread->currentContext;        
    }
#endif
}

U32 sdlMakeCurrent(KThread* thread, U32 arg) {
#ifdef SDL2
    KThreadGlContext* threadContext = thread->getGlContextById(arg);
    if (threadContext && threadContext->context) {
        if (SDL_GL_MakeCurrent(sdlWindow, threadContext->context)==0) {
            threadContext->hasBeenMakeCurrent = true;
            thread->currentContext = threadContext->context;
            sdlCurrentContext = threadContext->context;
#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
            loadExtensions();
#endif
            return 1;
        } else {
            klog("sdlMakeCurrent failed: %s\n", SDL_GetError());
        }
    } else if (arg == 0) {
        SDL_GL_MakeCurrent(sdlWindow, 0);
        thread->currentContext = 0;
        sdlCurrentContext = NULL;
        return 1;
    } else {
        kpanic("Tried to make an OpenGL context current for a different thread?");
    }
    return 0;
#else
    return 1;
#endif
}

KThreadGlContext* getGlContextByIdInUnknownThread(KProcess* process, U32 id) {
    KThreadGlContext* result = NULL;

    process->iterateThreads([id, &result] (KThread* thread) {
        result = thread->getGlContextById(id);
        return result==NULL;
    });
    return result;
}

U32 sdlShareLists(KThread* thread, U32 srcContext, U32 destContext) {
#ifdef SDL2
    KThreadGlContext* src = getGlContextByIdInUnknownThread(thread->process, srcContext);
    KThreadGlContext* dst = getGlContextByIdInUnknownThread(thread->process, destContext);

    if (src && dst) {
        if (dst->hasBeenMakeCurrent) {
            klog("could not share display lists, the destination context has been current already");
            return 0;
        }
        else if (dst->sharing)
        {
            klog("could not share display lists because dest has already shared lists before\n");
            return 0;
        }
        SDL_GL_DeleteContext(dst->context);
        SDL_GLContext currentContext = (SDL_GLContext)thread->currentContext;
        bool changedContext = false;

        if (thread->currentContext!=src->context) {
            changedContext = true;
            SDL_GL_MakeCurrent(sdlWindow, (SDL_GLContext)src->context);
        }
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1); 
        dst->context = SDL_GL_CreateContext(sdlWindow);
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0); 

        if (changedContext) {
            SDL_GL_MakeCurrent(sdlWindow, currentContext);
        }
        dst->sharing = true;
        return 1;
    }
#endif
    return 0;
}

U32 sdlCreateOpenglWindow_main_thread(KThread* thread, Wnd* wnd, int major, int minor, int profile, int flags) {
    DISPATCH_MAIN_THREAD_BLOCK_BEGIN_RETURN
#ifdef SDL2
    destroySDL2(thread);

    firstWindowCreated = 1;
    SDL_GL_ResetAttributes();
#endif
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, wnd->pixelFormat->cRedBits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, wnd->pixelFormat->cGreenBits);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, wnd->pixelFormat->cBlueBits);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, wnd->pixelFormat->cAlphaBits);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, wnd->pixelFormat->cDepthBits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, wnd->pixelFormat->cStencilBits);

    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, wnd->pixelFormat->cAccumRedBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, wnd->pixelFormat->cAccumGreenBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, wnd->pixelFormat->cAccumBlueBits);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, wnd->pixelFormat->cAccumAlphaBits);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, wnd->pixelFormat->dwFlags & 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, (wnd->pixelFormat->dwFlags & 0x40)?0:1);
#ifdef SDL2
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0); 
#endif
    firstWindowCreated = 1;
#ifdef SDL2

    SDL_DisplayMode dm;
    int sdlFlags = SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN;
    int cx = wnd->windowRect.right-wnd->windowRect.left;
    int cy = wnd->windowRect.bottom-wnd->windowRect.top;

    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        if (cx == dm.w && cy == dm.h) {
            sdlFlags|=SDL_WINDOW_BORDERLESS;
        }
    }   

    sdlWindow = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cx, cy, sdlFlags);
    if (!sdlWindow) {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        displayChanged(thread);
        return 0;
    }
    sdlWindowIsGL = true;
    return thread->id;
#else
    surface = NULL;
    SDL_SetVideoMode(wnd->windowRect.right-wnd->windowRect.left, wnd->windowRect.bottom-wnd->windowRect.top, wnd->pixelFormat->cDepthBits, SDL_OPENGL);        
    sdlWindowIsGL = true;
    return 0x200;
#endif
    DISPATCH_MAIN_THREAD_BLOCK_END
}

static U32 nextGlId = 1;

// window needs to be on the main thread
// context needs to be on the current thread
U32 sdlCreateContext(KThread* thread, Wnd* wnd, int major, int minor, int profile, int flags) {
    U32 result = 1;
    
    if (!sdlWindowIsGL) {
        result = sdlCreateOpenglWindow_main_thread(thread, wnd, major, minor, profile, flags);
    }
#ifdef SDL2
    if (result) {
        SDL_GLContext context = SDL_GL_CreateContext(sdlWindow);;
        if (!context) {
            fprintf(stderr, "Couldn't create context: %s\n", SDL_GetError());
            DISPATCH_MAIN_THREAD_BLOCK_BEGIN_RETURN
            displayChanged(thread);
            return 0;
            DISPATCH_MAIN_THREAD_BLOCK_END
        }
        result = nextGlId;
        thread->addGlContext(nextGlId++, context);
        contextCount++;
        if (!wnd->openGlContext)
            wnd->openGlContext = context;       
    }
#endif
    return result;
}

void sdlScreenResized(KThread* thread) {
    DISPATCH_MAIN_THREAD_BLOCK_BEGIN
#ifdef SDL2
    if (contextCount)
        SDL_SetWindowSize(sdlWindow, screenCx, screenCy);
    else
        displayChanged(thread);
#else
    displayChanged(thread);
#endif
    DISPATCH_MAIN_THREAD_BLOCK_END
}

void showSDLStartingWindow() {
#ifdef SDL2    
        sdlWindow = SDL_CreateWindow("BoxedWine Is Starting Up", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_SHOWN);
        sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);	
#else
        SDL_WM_SetCaption("BoxedWine Is Starting Up", "BoxedWine Is Starting Up");
        surface = SDL_SetVideoMode(320, 249, 32, flags);
#endif
}

static void displayChanged(KThread* thread) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(sdlMutex);
#ifndef SDL2
    U32 flags;
#endif
    firstWindowCreated = 1;
    if (videoEnabled) {
         for (auto& n : hwndToWnd) {
            Wnd* wnd = n.second;
            wnd->openGlContext = 0;
        }
#ifdef SDL2
        destroySDL2(thread);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, sdlScaleQuality);

        int cx = screenCx*sdlScaleX/100;
        int cy = screenCy*sdlScaleY/100;
        int flags = SDL_WINDOW_SHOWN;
        if (sdlFullScreen) {
            SDL_DisplayMode dm;

            if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
            {
                 SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
                 sdlFullScreen = false;
            } else {
                cx = dm.w;
                cy = dm.h;
                flags|=SDL_WINDOW_BORDERLESS;
                sdlScaleX = dm.w * 100 / screenCx;
                sdlScaleY = dm.h * 100 / screenCy;
            }
        }
        sdlWindow = SDL_CreateWindow("BoxedWine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cx, cy, SDL_WINDOW_SHOWN);
        sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);	
#else
        flags = SDL_HWSURFACE;
        if (surface && SDL_MUSTLOCK(surface)) {
            SDL_UnlockSurface(surface);
        }
        printf("Switching to %dx%d@%d\n", screenCx, screenCy, bits_per_pixel);
        surface = SDL_SetVideoMode(screenCx, screenCy, 32, flags);
#endif
        sdlWindowIsGL = false;
    }
}

void sdlSwapBuffers(KThread* thread) {
#ifdef SDL2
    SDL_GL_SwapWindow(sdlWindow);
#else
    SDL_GL_SwapBuffers();
#endif
}

#ifdef SDL2
static S8 sdlBuffer[1024*1024*4];
#endif

void wndBlt(KThread* thread, U32 hwnd, U32 bits, S32 xOrg, S32 yOrg, U32 width, U32 height, U32 rect) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(sdlMutex);
    Wnd* wnd = getWnd(hwnd);
    wRECT r;
    U32 y;    
    int bpp = bits_per_pixel==8?32:bits_per_pixel;
    int pitch = (width*((bpp+7)/8)+3) & ~3;
    static int i;

    readRect(thread, rect, &r);

    if (!firstWindowCreated) {
        displayChanged(thread);
    }
#ifndef SDL2
    if (!surface)
        return;
#endif
    if (wnd)
#ifdef SDL2
    {
        SDL_Texture *sdlTexture = NULL;
        
        if (wnd->sdlTexture) {
            sdlTexture = (SDL_Texture*)wnd->sdlTexture;
            if (sdlTexture && (((U32)wnd->sdlTextureHeight) != height || ((U32)wnd->sdlTextureWidth) != width)) {
                SDL_DestroyTexture(sdlTexture);
                wnd->sdlTexture = NULL;
                sdlTexture = NULL;
            }
        }
        if (!sdlTexture) {
            U32 format = SDL_PIXELFORMAT_ARGB8888;
            if (bpp == 16) {
                format = SDL_PIXELFORMAT_RGB565;
            } else if (bpp == 15) {
                format = SDL_PIXELFORMAT_RGB555;
            }
            if (videoEnabled && sdlRenderer) {
                sdlTexture = SDL_CreateTexture(sdlRenderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);
                wnd->sdlTexture = sdlTexture;
            }
            wnd->sdlTextureHeight = height;
            wnd->sdlTextureWidth = width;
        }
        if (!thread->memory->isValidReadAddress(bits, height*pitch)) {
            return;
        }
#ifndef BOXEDWINE_64BIT_MMU        
        for (y = 0; y < height; y++) {
            memcopyToNative(bits+(height-y-1)*pitch, sdlBuffer+y*pitch, pitch);
        } 
#endif
        if (bits_per_pixel!=32) {
            // SDL_ConvertPixels(width, height, )
        }
#ifdef BOXEDWINE_RECORDER
        if (Recorder::instance || Player::instance) {
            U32 toCopy = pitch*height;
            if (wnd->bitsSize<toCopy) {
                if (wnd->bits) {
                    delete[] wnd->bits;
                }
                wnd->bits = new U8[toCopy];
                wnd->bitsSize = toCopy;
            }
#ifdef BOXEDWINE_64BIT_MMU
            for (y = 0; y < height; y++) {
                memcopyToNative(bits+(height-y-1)*pitch, sdlBuffer+y*pitch, pitch);
            } 
#endif
            memcpy(wnd->bits, sdlBuffer, toCopy);
        }
#endif        
        if (videoEnabled && sdlRenderer) {
#ifdef BOXEDWINE_64BIT_MMU
            SDL_UpdateTexture(sdlTexture, NULL, getNativeAddress(KThread::currentThread()->process->memory, bits), pitch);
#else
            SDL_UpdateTexture(sdlTexture, NULL, sdlBuffer, pitch);
#endif
        }
    }
#else		
    {     
        SDL_Surface* s = NULL;
        if (wnd->surface) {
            s = (SDL_Surface*)wnd->sdlSurface;
            if (s && (s->w!=width || s->h!=height || s->format->BitsPerPixel!=bpp)) {
                SDL_FreeSurface(s);
                wnd->sdlSurface = NULL;
                s = NULL;
            }
        }
        if (!s) {
            U32 rMask = 0x00FF0000;
            U32 gMask = 0x0000FF00;
            U32 bMask = 0x000000FF;

            if (bpp==15) {
                rMask = 0x7C00;
                gMask = 0x03E0;
                bMask = 0x001F;
            } else if (bpp == 16) {
                rMask = 0xF800;
                gMask = 0x07E0;
                bMask = 0x001F;
            }
            s = SDL_CreateRGBSurface(0, width, height, bpp, rMask, gMask, bMask, 0);
            wnd->sdlSurface = s;
        }
        if (SDL_MUSTLOCK(s)) {
            SDL_LockSurface(s);
        }
        for (y = 0; y < height; y++) {
            memcopyToNative(bits+(height-y-1)*pitch, (S8*)(s->pixels)+y*s->pitch, pitch);
        }   
        if (SDL_MUSTLOCK(s)) {
            SDL_UnlockSurface(s);
        }      
    }	
#endif
}

U32 sdlUpdated;

#ifdef BOXEDWINE_RECORDER
U8* recorderBuffer;
U32 recorderBufferSize;
#endif

void sdlDrawAllWindows(KThread* thread, U32 hWnd, int count) {    
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(sdlMutex);
#ifdef BOXEDWINE_RECORDER
    if (Recorder::instance || Player::instance) {
        int bpp = bits_per_pixel==8?32:bits_per_pixel;
        S32 bytesPerPixel = (bpp+7)/8;
        S32 recorderPitch = (screenCx*((bpp+7)/8)+3) & ~3;
        if (recorderPitch*screenCy>recorderBufferSize) {
            if (recorderBuffer) {
                delete[] recorderBuffer;
            }
            recorderBuffer = new U8[recorderPitch*screenCy];
            recorderBufferSize = recorderPitch*screenCy;
        }
        if (bpp==32) {
            U32* pixel = (U32*)recorderBuffer;
            for (U32 i=0;i<screenCy*screenCx;i++, pixel++) {
                *pixel = 165 | (110 << 8) | (58 << 16) | (255 << 24);
            }
        } else {
            memset(recorderBuffer, 0, recorderBufferSize);
        }        
        for (int i=count-1;i>=0;i--) {
            Wnd* wnd = getWnd(readd(hWnd+i*4));
            if (wnd && wnd->sdlTextureWidth) {
                int width = wnd->sdlTextureWidth;
                int height = wnd->sdlTextureHeight;
                S32 top = wnd->windowRect.top;
                S32 left = wnd->windowRect.left;
                S32 srcTopAdjust = 0;
                S32 srcLeftAdjust = 0;

                if (top<0) {
                    srcTopAdjust = -top;
                    top = 0;
                }
                if (top>=(S32)screenCy)
                    continue;
                if (left>=(S32)screenCx)
                    continue;
                if (left<0) {
                    srcLeftAdjust = -left;
                    left = 0;
                }
                if (top+height>(S32)screenCy)
                    height = screenCy-top;
                int pitch = (width*((bpp+7)/8)+3) & ~3;
                if (left+width>(S32)screenCx)
                    width = screenCx - left;       
                int copyPitch = (width*((bpp+7)/8)+3) & ~3;
                for (int y=0;y<height;y++) {
                    S32 offset = recorderPitch*(y+top)+(left*bytesPerPixel);
                    if (offset<0 || offset+copyPitch>(S32)recorderBufferSize || copyPitch<0) {
                        kpanic("script recorder overwrote memory when copying screen");
                    }
                    memcpy(recorderBuffer+offset, wnd->bits+pitch*(y+srcTopAdjust)+(srcLeftAdjust*bytesPerPixel), copyPitch);
                }
            }        	
        }
    }
#endif
#ifdef SDL2
    if (videoEnabled && sdlRenderer) {
        SDL_SetRenderDrawColor(sdlRenderer, 58, 110, 165, 255 );
        SDL_RenderClear(sdlRenderer);    
        for (int i=count-1;i>=0;i--) {
            Wnd* wnd = getWnd(readd(hWnd+i*4));
            if (wnd && wnd->sdlTextureWidth && wnd->sdlTexture) {
                SDL_Rect dstrect;
                dstrect.x = wnd->windowRect.left*sdlScaleX/100;
                dstrect.y = wnd->windowRect.top*sdlScaleY/100;
                dstrect.w = wnd->sdlTextureWidth*sdlScaleX/100;
                dstrect.h = wnd->sdlTextureHeight*sdlScaleY/100;

#ifdef BOXEDWINE_64BIT_MMU
                SDL_RenderCopyEx(sdlRenderer, (SDL_Texture*)wnd->sdlTexture, NULL, &dstrect, 0, NULL, SDL_FLIP_VERTICAL);
#else
               SDL_RenderCopy(sdlRenderer, (SDL_Texture*)wnd->sdlTexture, NULL, &dstrect);	            
#endif
            }
        }   
        SDL_RenderPresent(sdlRenderer);
    }
    sdlUpdated=1;
#else
    if (surface) {
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 58, 110, 165));
        for (int i=count-1;i>=0;i--) {
            Wnd* wnd = getWnd(readd(hWnd+i*4));
            if (wnd && wnd->sdlSurface) {
                SDL_Rect dstrect;
                dstrect.x = wnd->windowRect.left;
                dstrect.y = wnd->windowRect.top;
                dstrect.w = ((SDL_Surface*)(wnd->sdlSurface))->w;
                dstrect.h = ((SDL_Surface*)(wnd->sdlSurface))->h;
                //if (bpp==8)
                //    SDL_SetPalette((SDL_Surface*)wnd->sdlSurface, SDL_LOGPAL, sdlPalette, 0, 256);
                SDL_BlitSurface((SDL_Surface*)wnd->sdlSurface, NULL, surface, &dstrect);
            }        	
        }    
        SDL_UpdateRect(surface, 0, 0, 0, 0);
    }
#endif
}

Wnd* wndCreate(KThread* thread, U32 processId, U32 hwnd, U32 windowRect, U32 clientRect) {
    Wnd* wnd = new Wnd();
    readRect(thread, windowRect, &wnd->windowRect);
    readRect(thread, clientRect, &wnd->clientRect);
    wnd->processId = processId;
    wnd->hwnd = hwnd;
    hwndToWnd[hwnd] = wnd;
    return wnd;
}

void wndDestroy(U32 hwnd) {
    hwndToWnd.erase(hwnd);
}

void writeRect(KThread* thread, U32 address, wRECT* rect) {
    if (address) {
        writed(address, rect->left);
        writed(address+4, rect->top);
        writed(address+8, rect->right);
        writed(address+12, rect->bottom);
    }
}

void readRect(KThread* thread, U32 address, wRECT* rect) {
    if (address) {
        rect->left = readd(address);
        rect->top = readd(address+4);
        rect->right = readd(address+8);
        rect->bottom = readd(address+12);
    }
}

void sdlShowWnd(KThread* thread, Wnd* wnd, U32 bShow) {
#ifdef SDL2
    if (!bShow && wnd) {
        if (wnd->sdlTexture) {
            SDL_DestroyTexture((SDL_Texture*)wnd->sdlTexture);
            wnd->sdlTexture = NULL;
        }
        wnd->sdlTextureHeight = 0;
        wnd->sdlTextureWidth = 0;
    }
#else
    if (!bShow && wnd && wnd->sdlSurface) {
        SDL_FreeSurface((SDL_Surface*)wnd->sdlSurface);
        wnd->sdlSurface = NULL;
    }
#endif
}

void setWndText(Wnd* wnd, const char* text) {
    wnd->text = text;
}

void updateScreen() {
    // this mechanism probably won't work well if multiple threads are updating the screen, there could be flickering
}

U32 sdlGetGammaRamp(KThread* thread, U32 ramp) {
    U16 r[256];
    U16 g[256];
    U16 b[256];

    if (videoEnabled) {
#ifdef SDL2
        if (SDL_GetWindowGammaRamp(sdlWindow, r, g, b)==0) {
#else
        if (SDL_GetGammaRamp(r, g, b)==0) {
#endif
            int i;
            for (i=0;i<256;i++) {
                writew(ramp+i*2, r[i]);
                writew(ramp+i*2+512, g[i]);
                writew(ramp+i*2+1024, b[i]);
            }
            return 1;
        }
    }
    return 0;
}

void sdlGetPalette(KThread* thread, U32 start, U32 count, U32 entries) {
    U32 i;

    for (i=0;i<count;i++) {
        writeb(entries+4*i, sdlPalette[i+start].r);
        writeb(entries+4*i+1, sdlPalette[i+start].g);
        writeb(entries+4*i+2, sdlPalette[i+start].b);
        writeb(entries+4*i+3, 0);
    }
}

U32 sdlGetNearestColor(KThread* thread, U32 color) {
    if (!videoEnabled) {
        return color;
    }
#ifdef SDL2
    SDL_Surface* surface = SDL_GetWindowSurface(sdlWindow);
    if (surface) {
        return SDL_MapRGB(SDL_GetWindowSurface(sdlWindow)->format, color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
    }
    return color;
#else
    if (surface)
        return SDL_MapRGB(surface->format, color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
    return color;
#endif
}

U32 sdlRealizePalette(KThread* thread, U32 start, U32 numberOfEntries, U32 entries) {
    U32 i;
    int result = 0;

    if (numberOfEntries>256)
        numberOfEntries=256;
    for (i=0;i<numberOfEntries;i++) {
        sdlPalette[i+start].r = readb(entries+4*i);
        sdlPalette[i+start].g = readb(entries+4*i+1);
        sdlPalette[i+start].b = readb(entries+4*i+2);
#ifdef SDL2
        sdlPalette[i+start].a = 0;
#else
        sdlPalette[i+start].unused = 0;
#endif
    }    
    return result;
}

void sdlRealizeDefaultPalette() {
    int i;

    for (i=0;i<256;i++) {
        sdlPalette[i] = sdlSystemPalette[i];
    }
}

/*
typedef struct tagMOUSEINPUT
{
    LONG    dx;
    LONG    dy;
    DWORD   mouseData;
    DWORD   dwFlags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} MOUSEINPUT, *PMOUSEINPUT, *LPMOUSEINPUT;

typedef struct tagKEYBDINPUT
{
    WORD    wVk;
    WORD    wScan;
    DWORD   dwFlags;
    DWORD   time;
    ULONG_PTR dwExtraInfo;
} KEYBDINPUT, *PKEYBDINPUT, *LPKEYBDINPUT;

typedef struct tagHARDWAREINPUT
{
    DWORD   uMsg;
    WORD    wParamL;
    WORD    wParamH;
} HARDWAREINPUT, *PHARDWAREINPUT, *LPHARDWAREINPUT;

#define INPUT_MOUSE     0
#define INPUT_KEYBOARD  1
#define INPUT_HARDWARE  2

typedef struct tagINPUT
{
    DWORD type;
    union
    {
        MOUSEINPUT      mi;
        KEYBDINPUT      ki;
        HARDWAREINPUT   hi;
    } DUMMYUNIONNAME;
} INPUT, *PINPUT, *LPINPUT;
*/

#define MOUSEEVENTF_MOVE            0x0001
#define MOUSEEVENTF_LEFTDOWN        0x0002
#define MOUSEEVENTF_LEFTUP          0x0004
#define MOUSEEVENTF_RIGHTDOWN       0x0008
#define MOUSEEVENTF_RIGHTUP         0x0010
#define MOUSEEVENTF_MIDDLEDOWN      0x0020
#define MOUSEEVENTF_MIDDLEUP        0x0040
#define MOUSEEVENTF_XDOWN           0x0080
#define MOUSEEVENTF_XUP             0x0100
#define MOUSEEVENTF_WHEEL           0x0800
#define MOUSEEVENTF_HWHEEL          0x1000
#define MOUSEEVENTF_MOVE_NOCOALESCE 0x2000
#define MOUSEEVENTF_VIRTUALDESK     0x4000
#define MOUSEEVENTF_ABSOLUTE        0x8000

void writeLittleEndian_4(U8* buffer, U32 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
}

void writeLittleEndian_2(U8* buffer, U16 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
}

static int lastX;
static int lastY;

void sdlSetMousePos(int x, int y) {
    if (!sdlWindowIsGL) {
        x = x*sdlScaleX/100;
        y = y*sdlScaleY/100;
    }
#ifdef SDL2
    SDL_WarpMouseInWindow(sdlWindow, x, y); 
#else
    SDL_WarpMouse((U16)x, (U16)y);
#endif
}

int sdlMouseMouse(int x, int y, bool relative) {
    Wnd* wnd;
    lastX = x;
    lastY = y;

    if (!hwndToWnd.size())
        return 0;
    wnd = getWndFromPoint(x, y);
    if (!wnd)
        wnd = getFirstVisibleWnd();
    if (wnd) {
        KProcess* process = KSystem::getProcess(wnd->processId);
        if (process) {
            KFileDescriptor* fd = process->getFileDescriptor(process->eventQueueFD);
            if (fd) {
                Memory* memory = process->memory;
                U8 buffer[28];

                if (!sdlWindowIsGL) {
                    x = (x*100+sdlScaleX/2)/sdlScaleX;
                    y = (y*100+sdlScaleY/2)/sdlScaleY;
                }
                writeLittleEndian_4(buffer, 0); // INPUT_MOUSE
                writeLittleEndian_4(buffer+4, x); // dx
                writeLittleEndian_4(buffer+8, y); // dy
                writeLittleEndian_4(buffer+12, 0); // mouseData
                writeLittleEndian_4(buffer+16,  (relative?MOUSEEVENTF_MOVE:MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE)); // dwFlags
                writeLittleEndian_4(buffer+20, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+24, 0); // dwExtraInfo

                KUnixSocketObject::unixsocket_write_native_nowait(fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

int sdlMouseWheel(int amount, int x, int y) {
    Wnd* wnd;

    if (!hwndToWnd.size())
        return 0;
    wnd = getWndFromPoint(x, y);
    if (!wnd)
        wnd = getFirstVisibleWnd();
    if (wnd) {
        KProcess* process = KSystem::getProcess(wnd->processId);
        if (process) {
            KFileDescriptor* fd = process->getFileDescriptor(process->eventQueueFD);
            if (fd) {
                Memory* memory = process->memory;
                U8 buffer[28];

                if (!sdlWindowIsGL) {
                    x = (x*100+sdlScaleX/2)/sdlScaleX;
                    y = (y*100+sdlScaleY/2)/sdlScaleY;
                }
                writeLittleEndian_4(buffer, 0); // INPUT_MOUSE
                writeLittleEndian_4(buffer+4, x); // dx
                writeLittleEndian_4(buffer+8, y); // dy
                writeLittleEndian_4(buffer+12, amount); // mouseData
                writeLittleEndian_4(buffer+16, MOUSEEVENTF_WHEEL | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE); // dwFlags
                writeLittleEndian_4(buffer+20, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+24, 0); // dwExtraInfo

                KUnixSocketObject::unixsocket_write_native_nowait(fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

int sdlMouseButton(U32 down, U32 button, int x, int y) {
    Wnd* wnd;

    if (!hwndToWnd.size())
        return 0;
    wnd = getWndFromPoint(x, y);
    if (!wnd)
        wnd = getFirstVisibleWnd();
    if (wnd) {
        KProcess* process = KSystem::getProcess(wnd->processId);
        if (process) {
            KFileDescriptor* fd = process->getFileDescriptor(process->eventQueueFD);
            if (fd) {
                Memory* memory = process->memory;
                U32 flags = MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE;
                U8 buffer[28];

                if (down) {
                    switch (button) {
                        case 0: flags |= MOUSEEVENTF_LEFTDOWN; break;
                        case 1: flags |= MOUSEEVENTF_RIGHTDOWN; break;
                        case 2: flags |= MOUSEEVENTF_MIDDLEDOWN; break;
                    }
                } else {
                    switch (button) {
                        case 0: flags |= MOUSEEVENTF_LEFTUP; break;
                        case 1: flags |= MOUSEEVENTF_RIGHTUP; break;
                        case 2: flags |= MOUSEEVENTF_MIDDLEUP; break;
                    }
                }
                if (!sdlWindowIsGL) {
                    x = (x*100+sdlScaleX/2)/sdlScaleX;
                    y = (y*100+sdlScaleY/2)/sdlScaleY;
                }
                writeLittleEndian_4(buffer, 0); // INPUT_MOUSE
                writeLittleEndian_4(buffer+4, x); // dx
                writeLittleEndian_4(buffer+8, y); // dy
                writeLittleEndian_4(buffer+12, 0); // mouseData
                writeLittleEndian_4(buffer+16, flags); // dwFlags
                writeLittleEndian_4(buffer+20, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+24, 0); // dwExtraInfo

                KUnixSocketObject::unixsocket_write_native_nowait(fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

static char cursorName[1024];

const char* getCursorName(char* moduleName, char* resourceName, int resource) {
    safe_strcpy(cursorName, moduleName, 1024);
    safe_strcat(cursorName, ":", 1024);
    if (strlen(resourceName))
        safe_strcat(cursorName, resourceName, 1024);
    else {
        char tmp[10];
        SDL_itoa(resource, tmp, 16);
        safe_strcat(cursorName, tmp, 1024);
    }
    return cursorName;
}

U32 sdlSetCursor(KThread* thread, char* moduleName, char* resourceName, int resource) {
    if (!moduleName && !resourceName && !resource) {
        SDL_ShowCursor(0);
        return 1;
    } else {
        const char* name = getCursorName(moduleName, resourceName, resource);
        if (cursors.count(name) && !relativeMouse) {
            SDL_Cursor* cursor = cursors[name];
            if (!cursor)
                return 0;
            SDL_ShowCursor(1);
            SDL_SetCursor(cursor);
            return 1;
        }        
    }
    return 0;
}

void sdlCreateAndSetCursor(KThread* thread, char* moduleName, char* resourceName, int resource, U8* and_bits, U8* xor_bits, int width, int height, int hotX, int hotY) {
    SDL_Cursor* cursor;
    int byteCount = (width+31) / 31 * 4 * height;
    int dst,src,y, x;
    U8 data_bits[64*64/8];
    U8 mask_bits[64*64/8];
    U32 srcPitch = (width+31)/32*4;
    U32 dstPitch = (width+7)/8;

    // AND | XOR | Windows cursor pixel | SDL
    // --------------------------------------
    // 0  |  0  | black                 | transparent
    // 0  |  1  | white                 | White
    // 1  |  0  | transparent           | Inverted color if possible, black if not
    // 1  |  1  | invert                | Black

    // 0 0 -> 1 1
    // 0 1 -> 0 1
    // 1 0 -> 0 0
    // 1 1 -> 1 0
    for (y=0;y<height;y++) {
        dst = dstPitch*y;
        src = srcPitch*y;

        for (x=0;x<(width+7)/8;src++,dst++,x++) {
            int j;

            data_bits[dst] = 0;
            mask_bits[dst] = 0;
            for (j=0;j<8;j++) {
                U8 aBit = (and_bits[src] >> j) & 0x1;
                U8 xBit = (xor_bits[src] >> j) & 0x1;

                if (aBit && xBit) {
                    xBit = 0;
                } else if (aBit && !xBit) {
                    aBit = 0;
                } else if (!aBit && xBit) {
                
                } else if (!aBit && !xBit) {
                    aBit = 1;
                    xBit = 1;
                }
                if (aBit)
                    data_bits[dst] |= (1 << j);

                if (xBit)
                    mask_bits[dst] |= (1 << j);
            }
        }
    }

    cursor = SDL_CreateCursor(data_bits, mask_bits, width, height, hotX, hotY);
    if (cursor) {
        const char* name = getCursorName(moduleName, resourceName, resource);
        cursors[name] = cursor;
        SDL_SetCursor(cursor);
    }
}

#define KEYEVENTF_EXTENDEDKEY        0x0001
#define KEYEVENTF_KEYUP              0x0002
#define KEYEVENTF_UNICODE            0x0004
#define KEYEVENTF_SCANCODE           0x0008

#define VK_CANCEL              0x03
#define VK_BACK                0x08
#define VK_TAB                 0x09
#define VK_RETURN              0x0D
#define VK_SHIFT               0x10
#define VK_CONTROL             0x11
#define VK_MENU                0x12
#define VK_PAUSE               0x13
#define VK_CAPITAL             0x14

#define VK_ESCAPE              0x1B

#define VK_SPACE               0x20
#define VK_PRIOR               0x21
#define VK_NEXT                0x22
#define VK_END                 0x23
#define VK_HOME                0x24
#define VK_LEFT                0x25
#define VK_UP                  0x26
#define VK_RIGHT               0x27
#define VK_DOWN                0x28
#define VK_INSERT              0x2D
#define VK_DELETE              0x2E
#define VK_HELP                0x2F

#define VK_MULTIPLY            0x6A
#define VK_ADD                 0x6B
#define VK_DECIMAL             0x6E
#define VK_DIVIDE              0x6F

#define VK_F1                  0x70
#define VK_F2                  0x71
#define VK_F3                  0x72
#define VK_F4                  0x73
#define VK_F5                  0x74
#define VK_F6                  0x75
#define VK_F7                  0x76
#define VK_F8                  0x77
#define VK_F9                  0x78
#define VK_F10                 0x79
#define VK_F11                 0x7A
#define VK_F12                 0x7B
#define VK_F24                 0x87

#define VK_NUMLOCK             0x90
#define VK_SCROLL              0x91

#define VK_LSHIFT              0xA0
#define VK_RSHIFT              0xA1
#define VK_LCONTROL            0xA2
#define VK_RCONTROL            0xA3
#define VK_LMENU               0xA4
#define VK_RMENU               0xA5

#define VK_OEM_1               0xBA
#define VK_OEM_PLUS            0xBB
#define VK_OEM_COMMA           0xBC
#define VK_OEM_MINUS           0xBD
#define VK_OEM_PERIOD          0xBE
#define VK_OEM_2               0xBF
#define VK_OEM_3               0xC0
#define VK_OEM_4               0xDB
#define VK_OEM_5               0xDC
#define VK_OEM_6               0xDD
#define VK_OEM_7               0xDE

#ifdef SDL2
#define SDLK_NUMLOCK SDL_SCANCODE_NUMLOCKCLEAR
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#define SDLK_KP0 SDLK_KP_0
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP2 SDLK_KP_2
#define SDLK_KP3 SDLK_KP_3
#define SDLK_KP4 SDLK_KP_4
#define SDLK_KP5 SDLK_KP_5
#define SDLK_KP6 SDLK_KP_6
#define SDLK_KP7 SDLK_KP_7
#define SDLK_KP8 SDLK_KP_8
#define SDLK_KP9 SDLK_KP_9
#endif

int sdlKey(U32 key, U32 down) {
    Wnd* wnd;
    
    if (!hwndToWnd.size())
        return 0;
    wnd = getFirstVisibleWnd();
    if (wnd) {
        KProcess* process = KSystem::getProcess(wnd->processId);
        if (process) {
            KFileDescriptor* fd = process->getFileDescriptor(process->eventQueueFD);
            if (fd) {
                U16 vKey = 0;
                U16 scan = 0;
                Memory* memory = process->memory;
                U8 buffer[28];

                U32 flags = 0;
                if (!down) 
                    flags|=KEYEVENTF_KEYUP;

                switch (key) {
                case SDLK_ESCAPE:
                    vKey = VK_ESCAPE;
                    scan = 0x01;
                    break;
                case SDLK_1:
                    vKey = '1';
                    scan = 0x02;
                    break;
                case SDLK_2:
                    vKey = '2';
                    scan = 0x03;
                    break;
                case SDLK_3:
                    vKey = '3';
                    scan = 0x04;
                    break;
                case SDLK_4:
                    vKey = '4';
                    scan = 0x05;
                    break;
                case SDLK_5:
                    vKey = '5';
                    scan = 0x06;
                    break;
                case SDLK_6:
                    vKey = '6';
                    scan = 0x07;
                    break;
                case SDLK_7:
                    vKey = '7';
                    scan = 0x08;
                    break;
                case SDLK_8:
                    vKey = '8';
                    scan = 0x09;
                    break;
                case SDLK_9:
                    vKey = '9';
                    scan = 0x0a;
                    break;
                case SDLK_0:
                    vKey = '0';
                    scan = 0x0b;
                    break;
                case SDLK_MINUS:
                    vKey = VK_OEM_MINUS;
                    scan = 0x0c;
                    break;
                case SDLK_EQUALS:
                    vKey = VK_OEM_PLUS;
                    scan = 0x0d;
                    break;
                case SDLK_BACKSPACE:
                    vKey = VK_BACK;
                    scan = 0x0e;
                    break;
                case SDLK_TAB:
                    vKey = VK_TAB;
                    scan = 0x0f;
                    break;
                case SDLK_q:
                    vKey = 'Q';
                    scan = 0x10;
                    break;
                case SDLK_w:
                    vKey = 'W';
                    scan = 0x11;
                    break;
                case SDLK_e:
                    vKey = 'E';
                    scan = 0x12;
                    break;
                case SDLK_r:
                    vKey = 'R';
                    scan = 0x13;
                    break;
                case SDLK_t:
                    vKey = 'T';
                    scan = 0x14;
                    break;
                case SDLK_y:
                    vKey = 'Y';
                    scan = 0x15;
                    break;
                case SDLK_u:
                    vKey = 'U';
                    scan = 0x16;
                    break;
                case SDLK_i:
                    vKey = 'I';
                    scan = 0x17;
                    break;
                case SDLK_o:
                    vKey = 'O';
                    scan = 0x18;
                    break;
                case SDLK_p:
                    vKey = 'P';
                    scan = 0x19;
                    break;
                case SDLK_LEFTBRACKET:
                    vKey = VK_OEM_4;
                    scan = 0x1a;
                    break;
                case SDLK_RIGHTBRACKET:
                    vKey = VK_OEM_6;
                    scan = 0x1b;
                    break;
                case SDLK_RETURN:
                    vKey = VK_RETURN;
                    scan = 0x1c;
                    break;
                case SDLK_LCTRL:
                    vKey = VK_LCONTROL;
                    scan = 0x1d;
                    break;
                case SDLK_RCTRL:
                    vKey = VK_RCONTROL;
                    scan = 0x11d;
                    break;
                case SDLK_a:
                    vKey = 'A';
                    scan = 0x1e;
                    break;
                case SDLK_s:
                    vKey = 'S';
                    scan = 0x1f;
                    break;
                case SDLK_d:
                    vKey = 'D';
                    scan = 0x20;
                    break;
                case SDLK_f:
                    vKey = 'F';
                    scan = 0x21;
                    break;
                case SDLK_g:
                    vKey = 'G';
                    scan = 0x22;
                    break;
                case SDLK_h:
                    vKey = 'H';
                    scan = 0x23;
                    break;
                case SDLK_j:
                    vKey = 'J';
                    scan = 0x24;
                    break;
                case SDLK_k:
                    vKey = 'K';
                    scan = 0x25;
                    break;
                case SDLK_l:
                    vKey = 'L';
                    scan = 0x26;
                    break;
                case SDLK_SEMICOLON:
                    vKey = VK_OEM_1;
                    scan = 0x27;
                    break;
                case SDLK_QUOTE:
                    vKey = VK_OEM_7;
                    scan = 0x28;
                    break;
                case SDLK_BACKQUOTE:
                    vKey = VK_OEM_3;
                    scan = 0x29;
                    break;
                case SDLK_LSHIFT:
                    vKey = VK_LSHIFT;
                    scan = 0x2a;
                    break;
                case SDLK_RSHIFT:
                    vKey = VK_RSHIFT;
                    scan = 0x36;
                    break;
                case SDLK_BACKSLASH:
                    vKey = VK_OEM_5;
                    scan = 0x2b;
                    break;
                case SDLK_z:
                    vKey = 'Z';
                    scan = 0x2c;
                    break;
                case SDLK_x:
                    vKey = 'X';
                    scan = 0x2d;
                    break;
                case SDLK_c:
                    vKey = 'C';
                    scan = 0x2e;
                    break;
                case SDLK_v:
                    vKey = 'V';
                    scan = 0x2f;
                    break;
                case SDLK_b:
                    vKey = 'B';
                    scan = 0x30;
                    break;
                case SDLK_n:
                    vKey = 'N';
                    scan = 0x31;
                    break;
                case SDLK_m:
                    vKey = 'M';
                    scan = 0x32;
                    break;
                case SDLK_COMMA:
                    vKey = VK_OEM_COMMA;
                    scan = 0x33;
                    break;
                case SDLK_PERIOD:
                    vKey = VK_OEM_PERIOD;
                    scan = 0x34;
                    break;
                case SDLK_SLASH:
                    vKey = VK_OEM_2;
                    scan = 0x35;
                    break;
                case SDLK_LALT:
                    vKey = VK_LMENU;
                    scan = 0x38;
                    break;
                case SDLK_RALT:
                    vKey = VK_RMENU;
                    scan = 0x138;
                    break;
                case SDLK_SPACE:
                    vKey = VK_SPACE;
                    scan = 0x39;
                    break;
                case SDLK_CAPSLOCK:
                    vKey = VK_CAPITAL;
                    scan = 0x3a;
                    break;
                case SDLK_F1:
                    vKey = VK_F1;
                    scan = 0x3b;
                    break;
                case SDLK_F2:
                    vKey = VK_F2;
                    scan = 0x3c;
                    break;
                case SDLK_F3:
                    vKey = VK_F3;
                    scan = 0x3d;
                    break;
                case SDLK_F4:
                    vKey = VK_F4;
                    scan = 0x3e;
                    break;
                case SDLK_F5:
                    vKey = VK_F5;
                    scan = 0x3f;
                    break;
                case SDLK_F6:
                    vKey = VK_F6;
                    scan = 0x40;
                    break;
                case SDLK_F7:
                    vKey = VK_F7;
                    scan = 0x41;
                    break;
                case SDLK_F8:
                    vKey = VK_F8;
                    scan = 0x42;
                    break;
                case SDLK_F9:
                    vKey = VK_F9;
                    scan = 0x43;
                    break;
                case SDLK_F10:
                    vKey = VK_F10;
                    scan = 0x44;
                    break;
                case SDLK_NUMLOCK:
                    vKey = VK_NUMLOCK;
                    break;
                case SDLK_SCROLLOCK:
                    vKey = VK_SCROLL;
                    break;
                case SDLK_F11:
                    vKey = VK_F11;
                    scan = 0x57;
                    break;
                case SDLK_F12:
                    vKey = VK_F12;
                    scan = 0x58;
                    break;
                case SDLK_HOME:
                    vKey = VK_HOME;
                    scan = 0x147;
                    break;
                case SDLK_UP:
                    vKey = VK_UP;
                    scan = 0x148;
                    break;
                case SDLK_PAGEUP:
                    vKey = VK_PRIOR;
                    scan = 0x149;
                    break;
                case SDLK_LEFT:
                    vKey = VK_LEFT;
                    scan = 0x14b;
                    break;
                case SDLK_RIGHT:
                    vKey = VK_RIGHT;
                    scan = 0x14d;
                    break;
                case SDLK_END:
                    vKey = VK_END;
                    scan = 0x14f;
                    break;
                case SDLK_DOWN:
                    vKey = VK_DOWN;
                    scan = 0x150;
                    break;
                case SDLK_PAGEDOWN:
                    vKey = VK_NEXT;
                    scan = 0x151;
                    break;
                case SDLK_INSERT:
                    vKey = VK_INSERT;
                    scan = 0x152;
                    break;
                case SDLK_DELETE:
                    vKey = VK_DELETE;
                    scan = 0x153;
                    break;
                case SDLK_PAUSE:
                    vKey = VK_PAUSE;
                    scan = 0x154; // :TODO: is this right?
                    break;
                case SDLK_KP0:
                    scan = 0x52;
                    break;
                case SDLK_KP1:
                    vKey = VK_END;
                    scan = 0x4F;
                    break;
                case SDLK_KP2:
                    vKey = VK_DOWN;
                    scan = 0x50;
                    break;
                case SDLK_KP3:
                    vKey = VK_NEXT;
                    scan = 0x51;
                    break;
                case SDLK_KP4:
                    vKey = VK_LEFT;
                    scan = 0x4B;
                    break;
                case SDLK_KP5:
                    scan = 0x4C;
                    break;
                case SDLK_KP6:
                    vKey = VK_RIGHT;
                    scan = 0x4D;
                    break;
                case SDLK_KP7:
                    vKey = VK_HOME;
                    scan = 0x47;
                    break;
                case SDLK_KP8:
                    vKey = VK_UP;
                    scan = 0x48;
                    break;
                case SDLK_KP9:
                    vKey = VK_PRIOR;
                    scan = 0x49;
                    break;
                case SDLK_KP_PERIOD:
                    vKey = VK_DECIMAL;
                    scan = 0x53;
                    break;
                case SDLK_KP_DIVIDE:
                    vKey = VK_DIVIDE;
                    scan = 0x135;
                    break;
                case SDLK_KP_MULTIPLY:
                    vKey = VK_MULTIPLY;
                    scan = 0x137;
                    break;
                case SDLK_KP_MINUS:
                    scan = 0x4A;
                    break;
                case SDLK_KP_PLUS:
                    vKey = VK_ADD;
                    scan = 0x4E;
                    break;
                case SDLK_KP_ENTER:
                    vKey = VK_RETURN;
                    scan = 0x11C;
                    break;

                default:
                    kwarn("Unhandled key: %d", key);
                    return 1;
                }
                if (scan & 0x100)               
                    flags |= KEYEVENTF_EXTENDEDKEY;

                writeLittleEndian_4(buffer, 1); // INPUT_KEYBOARD
                writeLittleEndian_2(buffer+4, vKey); // wVk
                writeLittleEndian_2(buffer+6, scan & 0xFF); // wScan
                writeLittleEndian_4(buffer+8, flags); // dwFlags
                writeLittleEndian_4(buffer+12, getMilliesSinceStart()); // time
                writeLittleEndian_4(buffer+16, 0); // dwExtraInfo
                writeLittleEndian_4(buffer+20, 0); // pad
                writeLittleEndian_4(buffer+24, 0); // pad

                KUnixSocketObject::unixsocket_write_native_nowait(fd->kobject, buffer, 28);
            }
        }
    }
    return 1;
}

U32 sdlToUnicodeEx(KThread* thread, U32 virtKey, U32 scanCode, U32 lpKeyState, U32 bufW, U32 bufW_size, U32 flags, U32 hkl) {
    U32 ret = 0;
    U8 c = 0;
    U32 shift = readb(lpKeyState+VK_SHIFT) & 0x80;
    U32 ctrl = readb(lpKeyState+VK_CONTROL) & 0x80;

    if (!virtKey)
        goto done;

    /* UCKeyTranslate, below, terminates a dead-key sequence if passed a
       modifier key press.  We want it to effectively ignore modifier key
       presses.  I think that one isn't supposed to call it at all for modifier
       events (e.g. NSFlagsChanged or kEventRawKeyModifiersChanged), since they
       are different event types than key up/down events. */
    switch (virtKey)
    {
        case VK_SHIFT:
        case VK_CONTROL:
        case VK_MENU:
        case VK_CAPITAL:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_LMENU:
        case VK_RMENU:
            goto done;
    }

    /* There are a number of key combinations for which Windows does not
       produce characters, but Mac keyboard layouts may.  Eat them.  Do this
       here to avoid the expense of UCKeyTranslate() but also because these
       keys shouldn't terminate dead key sequences. */
    if ((VK_PRIOR <= virtKey && virtKey <= VK_HELP) || (VK_F1 <= virtKey && virtKey <= VK_F24))
        goto done;

    /* Shift + <non-digit keypad keys>. */
    if (shift && VK_MULTIPLY <= virtKey && virtKey <= VK_DIVIDE)
        goto done;

    if (ctrl)
    {
        /* Control-Tab, with or without other modifiers. */
        if (virtKey == VK_TAB)
            goto done;

        /* Control-Shift-<key>, Control-Alt-<key>, and Control-Alt-Shift-<key>
           for these keys. */
        if (shift || (readb(lpKeyState+VK_MENU)))
        {
            switch (virtKey)
            {
                case VK_CANCEL:
                case VK_BACK:
                case VK_ESCAPE:
                case VK_SPACE:
                case VK_RETURN:
                    goto done;
            }
        }
    }

    if (shift) {
        if (virtKey>='A' && virtKey<='Z') {
            c = virtKey;
        } else {
            switch (virtKey) {
            case '1': c = '!'; break;
            case '2': c = '@'; break;
            case '3': c = '#'; break;
            case '4': c = '$'; break;
            case '5': c = '%'; break;
            case '6': c = '^'; break;
            case '7': c = '&'; break;
            case '8': c = '*'; break;
            case '9': c = '('; break;
            case '0': c = ')'; break;
            case VK_OEM_MINUS: c = '_'; break;
            case VK_OEM_PLUS: c = '+'; break;
            case VK_TAB: c = '\t'; break;
            case VK_OEM_4: c = '{'; break;
            case VK_OEM_6: c = '}'; break;
            case VK_OEM_1: c = ':'; break;
            case VK_OEM_7: c = '\"'; break;
            case VK_OEM_3: c = '~'; break;
            case VK_OEM_5: c = '|'; break;
            case VK_OEM_COMMA: c = '<'; break;
            case VK_OEM_PERIOD: c = '>'; break;
            case VK_OEM_2: c = '?'; break;
            case VK_SPACE: c = ' '; break;
            case VK_RETURN: c = 13; break;
            case VK_BACK: c = 8; break;
            case VK_ADD: c = '+'; break;
            default:
                kwarn("Unhandled key: %d", virtKey);
                break;
            }
        }
    } else {
        if (virtKey>='0' && virtKey<='9') {
            c = virtKey;
        } else if (virtKey>='A' && virtKey<='Z') {
            c = virtKey-'A'+'a';
        } else {
            switch (virtKey) {
            case VK_OEM_MINUS: c = '-'; break;
            case VK_OEM_PLUS: c = '='; break;
            case VK_TAB: c = '\t'; break;
            case VK_OEM_4: c = '['; break;
            case VK_OEM_6: c = ']'; break;
            case VK_OEM_1: c = ';'; break;
            case VK_OEM_7: c = '\''; break;
            case VK_OEM_3: c = '`'; break;
            case VK_OEM_5: c = '\\'; break;
            case VK_OEM_COMMA: c = ','; break;
            case VK_OEM_PERIOD: c = '.'; break;
            case VK_OEM_2: c = '/'; break;
            case VK_SPACE: c = ' '; break;
            case VK_RETURN: c = 13; break;
            case VK_BACK: c = 8; break;
            case VK_ADD: c = '+'; break;
            default:
                kwarn("Unhandled key: %d", virtKey);
                break;
            }
        }
    }
    if (c && ctrl) {
        if (c=='@') {
            writew(bufW, 0);
            ret=1;
        } else if (c>='a' && c<='z') {
            c = c-'a'+1;
        } else if (c>='A' && c<='Z') {
            c = c-'A'+1;
        } else if (c=='[') {
            c = 27;
        } else if (c=='\\') {
            c = 28;
        } else if (c==']') {
            c = 29;
        } else if (c=='^') {
            c = 30;
        } else if (c=='_') {
            c = 31;
        }
    }

    if (c) {
        writew(bufW, c);
        ret=1;
    }
done:
    /* Null-terminate the buffer, if there's room.  MSDN clearly states that the
       caller must not assume this is done, but some programs (e.g. Audiosurf) do. */
    if (1 <= ret && ret < bufW_size)
        writew(bufW+ret*2, 0);

    return ret;
}

unsigned int sdlGetMouseState(KThread* thread,int* x, int* y) {
#ifdef BOXEDWINE_RECORDER
    if (Player::instance) {
        *x = lastX;
        *y = lastY;
        return 0;
    }
#endif
    return SDL_GetMouseState(x, y);
}

U8 vkToChar[] = {
    0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 
    0x8, 0x9, 0x0, 0x0, 0x0, 0xD, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x1B, 0x0, 0x0, 0x0, 0x0, 
    0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
    0x38, 0x39, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
    0x58, 0x59, 0x5A, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
    0x38, 0x39, 0x2A, 0x2B, 0x0, 0x2D, 0x2E, 0x2F, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x3B, 0x3D, 0x2C, 0x2D, 0x2E, 0x2F, 
    0x60, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x5B, 0x5C, 0x5D, 0x27, 0x0, 
    0x0, 0x0, 0x5C, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

U32 sdlVirtualKeyToChar(U32 virtKey) {
    U32 c=0;
    if (virtKey<sizeof(vkToChar)) {
        c=vkToChar[virtKey];
    }
    return c;
}

U8 vkToScanCode[] = {
    0x0, 0x0, 0x0, 0x46, 0x0, 0x0, 0x0, 0x0,
    0xE, 0xF, 0x0, 0x0, 0x4C, 0x1C, 0x0, 0x0,
    0x2A, 0x1D, 0x38, 0x0, 0x3A, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0,
    0x39, 0x49, 0x51, 0x4F, 0x47, 0x4B, 0x48, 0x4D,
    0x50, 0x0, 0x0, 0x0, 0x54, 0x52, 0x53, 0x63,
    0xB, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
    0x9, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22,
    0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
    0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11,
    0x2D, 0x15, 0x2C, 0x5B, 0x5C, 0x5D, 0x0, 0x5F,
    0x52, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47,
    0x48, 0x49, 0x37, 0x4E, 0x0, 0x4A, 0x53, 0x35,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42,
    0x43, 0x44, 0x57, 0x58, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x76,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x45, 0x46, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x2A, 0x36, 0x1D, 0x1D, 0x38, 0x38, 0x6A, 0x69,
    0x67, 0x68, 0x65, 0x66, 0x32, 0x20, 0x2E, 0x30,
    0x19, 0x10, 0x24, 0x22, 0x6C, 0x6D, 0x6B, 0x21,
    0x0, 0x0, 0x27, 0xD, 0x33, 0xC, 0x34, 0x35,
    0x29, 0x73, 0x7E, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1A, 0x2B, 0x1B, 0x28, 0x0,
    0x0, 0x0, 0x56, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x71, 0x5C, 0x7B, 0x0, 0x6F, 0x5A, 0x0,
    0x0, 0x5B, 0x0, 0x5F, 0x0, 0x5E, 0x0, 0x0,
    0x0, 0x5D, 0x0, 0x62, 0x0, 0x0, 0x0, 0x0
};

U32 sdlVirtualKeyToScanCode(U32 virtKey) {
    U32 c=0;
    if (virtKey<sizeof(vkToScanCode)) {
        c=vkToScanCode[virtKey];
    }
    return c;
}

U16 vkToScanCodeEx[] = {
    0x0, 0x0, 0x0, 0xE046, 0x0, 0x0, 0x0, 0x0,
    0xE, 0xF, 0x0, 0x0, 0x4C, 0x1C, 0x0, 0x0,
    0x2A, 0x1D, 0x38, 0xE11D, 0x3A, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0,
    0x39, 0x49, 0x51, 0x4F, 0x47, 0x4B, 0x48, 0x4D,
    0x50, 0x0, 0x0, 0x0, 0x54, 0x52, 0x53, 0x63,
    0xB, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
    0x9, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22,
    0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
    0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11,
    0x2D, 0x15, 0x2C, 0xE05B, 0xE05C, 0xE05D, 0x0, 0xE05F,
    0x52, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47,
    0x48, 0x49, 0x37, 0x4E, 0x0, 0x4A, 0x53, 0xE035,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42,
    0x43, 0x44, 0x57, 0x58, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x76,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x45, 0x46, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x2A, 0x36, 0x1D, 0xE01D, 0x38, 0xE038, 0xE06A, 0xE069,
    0xE067, 0xE068, 0xE065, 0xE066, 0xE032, 0xE020, 0xE02E, 0xE030,
    0xE019, 0xE010, 0xE024, 0xE022, 0xE06C, 0xE06D, 0xE06B, 0xE021,
    0x0, 0x0, 0x27, 0xD, 0x33, 0xC, 0x34, 0x35,
    0x29, 0x73, 0x7E, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1A, 0x2B, 0x1B, 0x28, 0x0,
    0x0, 0x0, 0x56, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x71, 0x5C, 0x7B, 0x0, 0x6F, 0x5A, 0x0,
    0x0, 0x5B, 0x0, 0x5F, 0x0, 0x5E, 0x0, 0x0,
    0x0, 0x5D, 0x0, 0x62, 0x0, 0x0, 0x0, 0x0
};

U32 sdlVirtualKeyToScanCodeEx(U32 virtKey) {
    U32 c=0;
    if (virtKey<sizeof(vkToScanCodeEx)/sizeof(vkToScanCodeEx[0])) {
        c=vkToScanCodeEx[virtKey];
    }
    return c;
}

U8 scanCodeToVK[] = {
    0x0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0xBD, 0xBB, 0x8, 0x9,
    0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49,
    0x4F, 0x50, 0xDB, 0xDD, 0xD, 0x11, 0x41, 0x53,
    0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0xBA,
    0xDE, 0xC0, 0x10, 0xDC, 0x5A, 0x58, 0x43, 0x56,
    0x42, 0x4E, 0x4D, 0xBC, 0xBE, 0xBF, 0x10, 0x6A,
    0x12, 0x20, 0x14, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x90, 0x91, 0x24,
    0x26, 0x21, 0x6D, 0x25, 0xC, 0x27, 0x6B, 0x23,
    0x28, 0x22, 0x2D, 0x2E, 0x2C, 0x0, 0xE2, 0x7A,
    0x7B, 0xC, 0xEE, 0xF1, 0xEA, 0xF9, 0xF5, 0xF3,
    0x0, 0x0, 0xFB, 0x2F, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0xED,
    0x0, 0xE9, 0x0, 0xC1, 0x0, 0x0, 0x87, 0x0,
    0x0, 0x0, 0x0, 0xEB, 0x9, 0x0, 0xC2, 0x0,
};

U32 sdlScanCodeToVirtualKey(U32 code) {
    U32 c=0;
    if (code<sizeof(scanCodeToVK)) {
        c=scanCodeToVK[code];
    }
    return c;
}

U8 scanCodeToVkEx[] = {
    0x0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0xBD, 0xBB, 0x8, 0x9,
    0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49,
    0x4F, 0x50, 0xDB, 0xDD, 0xD, 0x11, 0x41, 0x53,
    0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0xBA,
    0xDE, 0xC0, 0x10, 0xDC, 0x5A, 0x58, 0x43, 0x56,
    0x42, 0x4E, 0x4D, 0xBC, 0xBE, 0xBF, 0x10, 0x6A,
    0x12, 0x20, 0x14, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x90, 0x91, 0x24,
    0x26, 0x21, 0x6D, 0x25, 0xC, 0x27, 0x6B, 0x23,
    0x28, 0x22, 0x2D, 0x2E, 0x2C, 0x0, 0xE2, 0x7A,
    0x7B, 0xC, 0xEE, 0xF1, 0xEA, 0xF9, 0xF5, 0xF3,
    0x0, 0x0, 0xFB, 0x2F, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0xED,
    0x0, 0xE9, 0x0, 0xC1, 0x0, 0x0, 0x87, 0x0,
    0x0, 0x0, 0x0, 0xEB, 0x9, 0x0, 0xC2, 0x0,
};

U32 sdlScanCodeToVirtualKeyEx(U32 code) {
    U32 c=0;
    if (code<sizeof(scanCodeToVkEx)) {
        c=scanCodeToVkEx[code];
    }
    return c;
}
#ifdef BOXEDWINE_RECORDER
static SDL_Texture* screenCopyTexture;

void sdlPushWindowSurface() {
    SDL_Surface* src = SDL_GetWindowSurface(sdlWindow);
    SDL_Rect r;
    U32 depth;

    if (bits_per_pixel==15)
        depth = 15;
    else if (bits_per_pixel==16)
        depth = 16;
    else
        depth = 32;
    r.x = 0;
    r.y = 0;
    r.w = src->w;
    r.h = src->h;

    screenCopyTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, src->w, src->h);
    U32 len = src->w * src->h * src->format->BytesPerPixel;
    U8* pixels = new unsigned char[len];

    if (!SDL_RenderReadPixels(sdlRenderer, &src->clip_rect, src->format->format, pixels, src->w * src->format->BytesPerPixel)) {
        SDL_UpdateTexture(screenCopyTexture, NULL, pixels, src->w * src->format->BytesPerPixel);
    }
    delete[] pixels;
}

void sdlPopWindowSurface() {
    SDL_Surface* dst = SDL_GetWindowSurface(sdlWindow);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = dst->w;
    rect.h = dst->h;    
        
    SDL_RenderCopy(sdlRenderer, screenCopyTexture, NULL, &rect);	
    SDL_RenderPresent(sdlRenderer);

    SDL_DestroyTexture(screenCopyTexture);
    screenCopyTexture = NULL;
}

void sdlDrawRectOnPushedSurfaceAndDisplay(U32 x, U32 y, U32 w, U32 h, U8 r, U8 g, U8 b, U8 a) {
    SDL_Surface* dst = SDL_GetWindowSurface(sdlWindow);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = dst->w;
    rect.h = dst->h;    
        
    SDL_RenderCopy(sdlRenderer, screenCopyTexture, NULL, &rect);	

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(sdlRenderer, &rect);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);

    SDL_RenderPresent(sdlRenderer);
}
#endif
bool sdlInternalScreenShot(std::string filepath, SDL_Rect* r, U32* crc) {       
#ifdef BOXEDWINE_RECORDER
     if (!recorderBuffer) {
        if (filepath.length()) {
            klog("failed to save screenshot, %s, because recorderBuffer was NULL", filepath.c_str());
        }
        return false;
    }
    U8* pixels = NULL;
    SDL_Surface* s = NULL;
    U32 rMask=0;
    U32 gMask=0;
    U32 bMask=0;
    int bpp = bits_per_pixel==8?32:bits_per_pixel;

    if (bpp==32) {
        rMask = 0x00FF0000;
        gMask = 0x0000FF00;
        bMask = 0x000000FF;
    } else if (bpp==16) {
        rMask = 0xF800;
        gMask = 0x07E0;
        bMask = 0x001F;
    } else {
        kpanic("Unhandled bpp for screen shot: %d", bpp);
    }
    if (r) {
        int inPitch = (screenCx*((bpp+7)/8)+3) & ~3;
        int outPitch = (r->w*((bpp+7)/8)+3) & ~3;        
        U32 bytesPerPixel = (bpp+7)/8;

        pixels = new unsigned char[outPitch*r->h];

        for (int y=0;y<r->h;y++) {
            memcpy(pixels+y*outPitch, recorderBuffer+(y+r->y)*inPitch+(r->x*bytesPerPixel), outPitch);
        }
        s = SDL_CreateRGBSurfaceFrom(pixels, r->w, r->h, bpp, outPitch, rMask, gMask, bMask, 0);
        if (crc) {
            U32 len = outPitch*r->h;
            *crc = crc32b(pixels, len);
        }
    } else {               
        int pitch = (screenCx*((bpp+7)/8)+3) & ~3;
        s = SDL_CreateRGBSurfaceFrom(recorderBuffer, screenCx, screenCy, bpp, pitch, rMask, gMask, bMask, 0);
        if (crc) {
            U32 len = pitch*screenCy;
            *crc = crc32b(recorderBuffer, len);
        }
    }

    if (!s) {
        klog("sdlScreenshot: %s", SDL_GetError());
        delete[] pixels;
        return false;
    }
    if (filepath.length()) {
        SDL_SaveBMP(s, filepath.c_str());
    }    
    if (s) {
        SDL_FreeSurface(s);
    }
    if (pixels) {
        delete[] pixels;
    }
    return true;
#else
    return false;
#endif
}

bool sdlPartialScreenShot(std::string filepath, U32 x, U32 y, U32 w, U32 h, U32* crc) {
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return sdlInternalScreenShot(filepath, &r, crc);
}

bool sdlScreenShot(std::string filepath, U32* crc) {
    return sdlInternalScreenShot(filepath, NULL, crc);

}

#ifdef SDL2
#define SDLK_NUMLOCK SDL_SCANCODE_NUMLOCKCLEAR
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#endif
U32 translate(U32 key) {
    switch (key) {
        case SDLK_ESCAPE:
            return K_KEY_ESC;
        case SDLK_1:
            return K_KEY_1;
        case SDLK_2:
            return K_KEY_2;
        case SDLK_3:
            return K_KEY_3;
        case SDLK_4:
            return K_KEY_4;
        case SDLK_5:
            return K_KEY_5;
        case SDLK_6:
            return K_KEY_6;
        case SDLK_7:
            return K_KEY_7;
        case SDLK_8:
            return K_KEY_8;
        case SDLK_9:
            return K_KEY_9;
        case SDLK_0:
            return K_KEY_0;
        case SDLK_MINUS:
            return K_KEY_MINUS;
        case SDLK_EQUALS:
            return K_KEY_EQUAL;
        case SDLK_BACKSPACE:
            return K_KEY_BACKSPACE;
        case SDLK_TAB:
            return K_KEY_TAB;
        case SDLK_q:
            return K_KEY_Q;
        case SDLK_w:
            return K_KEY_W;
        case SDLK_e:
            return K_KEY_E;
        case SDLK_r:
            return K_KEY_R;
        case SDLK_t:
            return K_KEY_T;
        case SDLK_y:
            return K_KEY_Y;
        case SDLK_u:
            return K_KEY_U;
        case SDLK_i:
            return K_KEY_I;
        case SDLK_o:
            return K_KEY_O;
        case SDLK_p:
            return K_KEY_P;
        case SDLK_LEFTBRACKET:
            return K_KEY_LEFTBRACE;
        case SDLK_RIGHTBRACKET:
            return K_KEY_RIGHTBRACE;
        case SDLK_RETURN:
            return K_KEY_ENTER;
        case SDLK_LCTRL:
            return K_KEY_LEFTCTRL;
        case SDLK_RCTRL:
            return K_KEY_RIGHTCTRL;
        case SDLK_a:
            return K_KEY_A;
        case SDLK_s:
            return K_KEY_S;
        case SDLK_d:
            return K_KEY_D;
        case SDLK_f:
            return K_KEY_F;
        case SDLK_g:
            return K_KEY_G;
        case SDLK_h:
            return K_KEY_H;
        case SDLK_j:
            return K_KEY_J;
        case SDLK_k:
            return K_KEY_K;
        case SDLK_l:
            return K_KEY_L;
        case SDLK_SEMICOLON:
            return K_KEY_SEMICOLON;
        case SDLK_QUOTE:
            return K_KEY_APOSTROPHE;
        case SDLK_BACKQUOTE:
            return K_KEY_GRAVE;
        case SDLK_LSHIFT:
            return K_KEY_LEFTSHIFT;
        case SDLK_RSHIFT:
            return K_KEY_RIGHTSHIFT;
        case SDLK_BACKSLASH:
            return K_KEY_BACKSLASH;
        case SDLK_z:
            return K_KEY_Z;
        case SDLK_x:
            return K_KEY_X;
        case SDLK_c:
            return K_KEY_C;
        case SDLK_v:
            return K_KEY_V;
        case SDLK_b:
            return K_KEY_B;
        case SDLK_n:
            return K_KEY_N;
        case SDLK_m:
            return K_KEY_M;
        case SDLK_COMMA:
            return K_KEY_COMMA;
        case SDLK_PERIOD:
            return K_KEY_DOT;
        case SDLK_SLASH:
            return K_KEY_SLASH;
        case SDLK_LALT:
             return K_KEY_LEFTALT;
        case SDLK_RALT:
            return K_KEY_RIGHTALT;
        case SDLK_SPACE:
            return K_KEY_SPACE;
        case SDLK_CAPSLOCK:
            return K_KEY_CAPSLOCK;
        case SDLK_F1:
            return K_KEY_F1;
        case SDLK_F2:
            return K_KEY_F2;
        case SDLK_F3:
            return K_KEY_F3;
        case SDLK_F4:
            return K_KEY_F4;
        case SDLK_F5:
            return K_KEY_F5;
        case SDLK_F6:
            return K_KEY_F6;
        case SDLK_F7:
            return K_KEY_F7;
        case SDLK_F8:
            return K_KEY_F8;
        case SDLK_F9:
            return K_KEY_F9;
        case SDLK_F10:
            return K_KEY_F10;
        case SDLK_NUMLOCK:
            return K_KEY_NUMLOCK;
        case SDLK_SCROLLOCK:
            return K_KEY_SCROLLLOCK;
        case SDLK_F11:
            return K_KEY_F11;
        case SDLK_F12:
            return K_KEY_F12;
        case SDLK_HOME:
            return K_KEY_HOME;
        case SDLK_UP:
            return K_KEY_UP;
        case SDLK_PAGEUP:
            return K_KEY_PAGEUP;
        case SDLK_LEFT:
            return K_KEY_LEFT;
        case SDLK_RIGHT:
            return K_KEY_RIGHT;
        case SDLK_END:
            return K_KEY_END;
        case SDLK_DOWN:
            return K_KEY_DOWN;
        case SDLK_PAGEDOWN:
            return K_KEY_PAGEDOWN;
        case SDLK_INSERT:
            return K_KEY_INSERT;
        case SDLK_DELETE:
            return K_KEY_DELETE;
        case SDLK_PAUSE:
            return K_KEY_PAUSE;
        default:
            kwarn("Unhandled key: %d", key);
            return 0;
    }
}

bool handlSdlEvent(void* p) {
    SDL_Event* e=(SDL_Event*)p;
#ifdef BOXEDWINE_RECORDER
    if (Player::instance) {
        if (e->type == SDL_QUIT) {
            return false;
        }
        return true;
    }
#endif
    if (e->type == SDL_QUIT) {
        return false;
    } else if (e->type == SDL_MOUSEMOTION) { 
        BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(e);
        if (relativeMouse) {
            if (!sdlMouseMouse((e->motion.x-screenCx/2)*rel_mouse_sensitivity/100, (e->motion.y-screenCy/2)*rel_mouse_sensitivity/100, true)) {
                onMouseMove((e->motion.x-screenCx/2)*rel_mouse_sensitivity/100, (e->motion.y-screenCy/2)*rel_mouse_sensitivity/100, true);                
            }      
#ifdef SDL2
            SDL_WarpMouseInWindow(sdlWindow, screenCx/2, screenCy/2);
#else
            SDL_WarpMouse((U16)(screenCx/2), (U16)(screenCy/2));
#endif
        } else {
            if (!sdlMouseMouse(e->motion.x, e->motion.y, false)) {
                onMouseMove(e->motion.x, e->motion.y, false);
            }
        }
        
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {
        BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(e);
        if (e->button.button==SDL_BUTTON_LEFT) {
            if (!sdlMouseButton(1, 0, (relativeMouse?0:e->motion.x), (relativeMouse?0:e->motion.y)))
                onMouseButtonDown(0);
        } else if (e->button.button == SDL_BUTTON_MIDDLE) {
            if (!sdlMouseButton(1, 2, (relativeMouse?0:e->motion.x), (relativeMouse?0:e->motion.y)))
                onMouseButtonDown(2);
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            if (!sdlMouseButton(1, 1, (relativeMouse?0:e->motion.x), (relativeMouse?0:e->motion.y)))
                onMouseButtonDown(1);
        }
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(e);
        if (e->button.button==SDL_BUTTON_LEFT) {
            if (!sdlMouseButton(0, 0, (relativeMouse?0:e->motion.x), (relativeMouse?0:e->motion.y)))
                onMouseButtonUp(0);
        } else if (e->button.button == SDL_BUTTON_MIDDLE) {
            if (!sdlMouseButton(0, 2, (relativeMouse?0:e->motion.x), (relativeMouse?0:e->motion.y)))
                onMouseButtonUp(2);
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            if (!sdlMouseButton(0, 1, (relativeMouse?0:e->motion.x), (relativeMouse?0:e->motion.y)))
                onMouseButtonUp(1);
        }
#ifdef SDL2
    } else if (e->type == SDL_MOUSEWHEEL) {
        // Handle up/down mouse wheel movements
        int x, y;
        SDL_GetMouseState(&x, &y);
        if (!sdlMouseWheel(e->wheel.y*80, (relativeMouse?0:x), (relativeMouse?0:y))) {
            onMouseWheel(e->wheel.y);
        }
#endif
    } else if (e->type == SDL_KEYDOWN) {
        if (!BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(e)) {
            if (e->key.keysym.sym==SDLK_SCROLLOCK) {
                KSystem::printStacks();
            }
            else if (!sdlKey(e->key.keysym.sym, 1)) {
                onKeyDown(translate(e->key.keysym.sym));
            }
/*
            // I'm leaving this relative mouse code in for now since it might be useful later
            // Currently Wine will automatically handle it
            if (e->key.keysym.sym==SDLK_RETURN && (SDL_GetModState() & KMOD_RALT)) {                
                if (relativeMouse) {
                    SDL_SetWindowGrab(sdlWindow, SDL_FALSE);
                    relativeMouse = false;
                } else {
                    SDL_SetCursor(NULL);
                    SDL_SetWindowGrab(sdlWindow, SDL_FALSE);
                    SDL_WarpMouseInWindow(sdlWindow, screenCx/2, screenCy/2);
                    relativeMouse = true;
                }                
            }
*/
        }
    } else if (e->type == SDL_KEYUP) {
        if (!BOXEDWINE_RECORDER_HANDLE_KEY_UP(e)) {
            if (!sdlKey(e->key.keysym.sym, 0)) {
                onKeyUp(translate(e->key.keysym.sym));
            }
        }
    }
#ifdef SDL2
    else if (e->type == SDL_WINDOWEVENT) {
        if (!isBoxedWineDriverActive())
            flipFBNoCheck();
    }
#endif
    return true;
}
