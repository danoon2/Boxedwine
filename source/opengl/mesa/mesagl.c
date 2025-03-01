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

#if defined(MESA) && defined(BOXEDWINE_GL)
#include "../../../mesa/include/GL/osmesa.h"
#include "../glcommon.h"
#include "kalloc.h"
#include "kprocess.h"
#include "log.h"

#include <stdio.h>
#include <SDL.h>

void flipFBNoCheck();

// GLAPI void APIENTRY glFinish( void ) {
void mesa_glFinish(struct CPU* cpu) {	
	glFinish();
	flipFBNoCheck();
#ifdef __EMSCRIPTEN__
        // we to return control to the browser in order for the screen to be updated
        threadDone(cpu);
#endif
}

// GLAPI void APIENTRY glFlush( void ) {
void mesa_glFlush(struct CPU* cpu) {	
	glFlush();	
	flipFBNoCheck();
#ifdef __EMSCRIPTEN__
        // we to return control to the browser in order for the screen to be updated
        threadDone(cpu);
#endif
}

// GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext share_list, Bool direct)
void mesa_glXCreateContext(struct CPU* cpu) {
	//U32 format = ARG5;
	OSMesaContext share_list = (OSMesaContext)ARG4;
	U32 accum = ARG3;
	U32 stencil = ARG2;
	U32 depth = ARG1;	

	EAX = (U32)OSMesaCreateContextExt( OSMESA_BGRA, depth, stencil, accum, share_list );	
	if (!EAX) {
		printf("OSMesaCreateContext failed!\n");
	}
}

// void glXDestroyContext(Display *dpy, GLXContext ctx)
void mesa_glXDestroyContext(struct CPU* cpu) {
	OSMesaContext ctx = (OSMesaContext)ARG2;

	OSMesaDestroyContext(ctx);
}

extern SDL_Surface* surface;
void fbSetupScreenForMesa(int width, int height, int depth);
void fbSetupScreen();

// Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx) 
void mesa_glXMakeCurrent(struct CPU* cpu) {
	//U32 isWindow = ARG5;
	//U32 depth = ARG4;
	U32 height = ARG3;
	U32 width = ARG2;
	OSMesaContext ctx = (OSMesaContext)ARG1;
	void* buffer = 0;

	if (width) {
		fbSetupScreenForMesa(width, height, 32);
		buffer = surface->pixels;
	} else {
		fbSetupScreen();
	}

	
	EAX = OSMesaMakeCurrent(ctx, buffer, GL_UNSIGNED_BYTE, width, height);
	OSMesaPixelStore(OSMESA_Y_UP, 0);
}

// void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
void mesa_glXSwapBuffers(struct CPU* cpu) {
	glFinish();
	flipFBNoCheck();
#ifdef __EMSCRIPTEN__
	// we to return control to the browser in order for the screen to be updated
	threadDone(cpu);
#endif
}

void mesa_init() {
	ext_glTexImage3D = (glTexImage3D_func)glTexImage3D;
	int99Callback[Finish] = mesa_glFinish;
	int99Callback[Flush] = mesa_glFlush;
	int99Callback[XCreateContext] = mesa_glXCreateContext;
	int99Callback[XMakeCurrent] = mesa_glXMakeCurrent;
	int99Callback[XDestroyContext] = mesa_glXDestroyContext;	
	int99Callback[XSwapBuffer] = mesa_glXSwapBuffers;
}
#endif
