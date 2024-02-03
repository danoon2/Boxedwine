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
#include <SDL_opengl.h>
#include "../glcommon.h"

#include <stdio.h>
#include <SDL.h>
#include "knativewindow.h"
#include "../boxedwineGL.h"
#include "../../sdl/startupArgs.h"

class SdlBoxedwineGL : public BoxedwineGL {
public:
    // from BoxedwineGL
    void deleteContext(void* context) override;
    bool makeCurrent(void* context, void* window) override;
    BString getLastError() override;
    void* createContext(void* window, std::shared_ptr<Wnd> wnd, PixelFormat* pixelFormat, U32 width, U32 height, int major, int minor, int profile) override;
    void swapBuffer(void* window) override;
    void setSwapInterval(U32 vsync) override;
    bool shareList(const std::shared_ptr<KThreadGlContext>& src, const std::shared_ptr<KThreadGlContext>& dst, void* window) override;
};

void SdlBoxedwineGL::deleteContext(void* context) {
    SDL_GL_DeleteContext(context);
}

bool SdlBoxedwineGL::makeCurrent(void* context, void* window) {
    return SDL_GL_MakeCurrent((SDL_Window*)window, context) == 0;
}

BString SdlBoxedwineGL::getLastError() {
    return BString::copy(SDL_GetError());
}

void* SdlBoxedwineGL::createContext(void* window, std::shared_ptr<Wnd> wnd, PixelFormat* pixelFormat, U32 width, U32 height, int major, int minor, int profile) {
    return SDL_GL_CreateContext((SDL_Window*)window);
}

void SdlBoxedwineGL::swapBuffer(void* window) {
    SDL_GL_SwapWindow((SDL_Window*)window);
}

void SdlBoxedwineGL::setSwapInterval(U32 vsync) {
    if (vsync == VSYNC_ADAPTIVE) {
        if (SDL_GL_SetSwapInterval(-1) == -1) {
            SDL_GL_SetSwapInterval(1);
        }
    }
    else if (vsync == VSYNC_ENABLED) {
        SDL_GL_SetSwapInterval(1);
    }
    else {
        SDL_GL_SetSwapInterval(0);
    }
}

bool SdlBoxedwineGL::shareList(const std::shared_ptr<KThreadGlContext>& src, const std::shared_ptr<KThreadGlContext>& dst, void* window) {
    if (src && dst) {
        if (dst->hasBeenMadeCurrent) {
            klog("could not share display lists, the destination context has been current already");
            return 0;
        }
        else if (dst->sharing)
        {
            klog("could not share display lists because dest has already shared lists before\n");
            return 0;
        }
        SDL_GL_DeleteContext(dst->context);
        KThread* thread = KThread::currentThread();
        SDL_GLContext currentContext = (SDL_GLContext)thread->currentContext;
        bool changedContext = false;

        if (thread->currentContext != src->context) {
            changedContext = true;
            SDL_GL_MakeCurrent((SDL_Window*)window, (SDL_GLContext)src->context);
        }
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        dst->context = SDL_GL_CreateContext((SDL_Window*)window);
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);

        if (changedContext) {
            SDL_GL_MakeCurrent((SDL_Window*)window, currentContext);
        }
        dst->sharing = true;
        return true;
    }
    return false;
}

static SdlBoxedwineGL sdlBoxedwineGL;

static int sdlOpenExtensionsLoaded = false;

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) ext_gl##func = (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);

void glExtensionsLoaded();

void loadSdlExtensions() {
    if (!sdlOpenExtensionsLoaded) {
        sdlOpenExtensionsLoaded = true;
        #include "../glfunctions.h"
        glExtensionsLoaded();
    }
}

// GLAPI void APIENTRY glFinish( void ) {
void sdl_glFinish(CPU* cpu) {	
    glFinish();
}

// GLAPI void APIENTRY glFlush( void ) {
void sdl_glFlush(CPU* cpu) {
    KNativeWindow::getNativeWindow()->preOpenGLCall(Flush);
    glFlush();	
}

// GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext share_list, Bool direct)
void sdl_glXCreateContext(CPU* cpu) {
    U32 doubleBuffered = ARG6;
    U32 format = ARG5;
    //U32 share = ARG4;
    U32 accum = ARG3;
    U32 stencil = ARG2;
    U32 depth = ARG1;	

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depth );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, stencil);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, doubleBuffered?1:0 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, format==0x1907?24:32);

    //SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 ); 
    //SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    EAX = 0x1000;	
}

// void glXDestroyContext(Display *dpy, GLXContext ctx)
void sdl_glXDestroyContext(CPU* cpu) {

}

// Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx) 
void sdl_glXMakeCurrent(CPU* cpu) {
    //U32 isWindow = ARG5;
    //U32 depth = ARG4;
    //U32 height = ARG3;
    //U32 width = ARG2;

    if (ARG2) {
        loadSdlExtensions();
    }
}

// void glXSwapBuffers(Display *dpy, GLXDrawable drawable)

void sdl_glXSwapBuffers(CPU* cpu) {
    KNativeWindow::getNativeWindow()->glSwapBuffers(cpu->thread);
}

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) pgl##func = gl##func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) pgl##func = gl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS)

void initSdlOpenGL() {
    if (BoxedwineGL::current != &sdlBoxedwineGL) {
        BoxedwineGL::current = &sdlBoxedwineGL;
        sdlOpenExtensionsLoaded = false;
#include "../glfunctions.h"
    }

    int99Callback[Finish] = sdl_glFinish;
    int99Callback[Flush] = sdl_glFlush;
    int99Callback[XCreateContext] = sdl_glXCreateContext;
    int99Callback[XMakeCurrent] = sdl_glXMakeCurrent;
    int99Callback[XDestroyContext] = sdl_glXDestroyContext;
    int99Callback[XSwapBuffer] = sdl_glXSwapBuffers;
}

#endif
