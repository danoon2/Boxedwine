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

int extLoaded = 0;

#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) ext_gl##func = (gl##func##_func)SDL_GL_GetProcAddress("gl" #func);

void glExtensionsLoaded();

void loadExtensions() {
    if (!extLoaded) {
        extLoaded = 1;
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
    glFlush();	
}

void fbSetupScreenForOpenGL(int width, int height, int depth);
void fbSetupScreen();

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
    U32 depth = ARG4;
    U32 height = ARG3;
    U32 width = ARG2;

    if (width) {
        loadExtensions();
        fbSetupScreenForOpenGL(width, height, depth);
    } else {
        fbSetupScreen();
    }
}

// void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
extern SDL_Window *sdlWindow;

void sdl_glXSwapBuffers(CPU* cpu) {
    SDL_GL_SwapWindow(sdlWindow);
}

void sdlgl_init() {	
    int99Callback[Finish] = sdl_glFinish;
    int99Callback[Flush] = sdl_glFlush;
    int99Callback[XCreateContext] = sdl_glXCreateContext;
    int99Callback[XMakeCurrent] = sdl_glXMakeCurrent;
    int99Callback[XDestroyContext] = sdl_glXDestroyContext;	
    int99Callback[XSwapBuffer] = sdl_glXSwapBuffers;
}

#endif
