/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __MAC_OPENGL_H__
#define __MAC_OPENGL_H__

struct SDL_Window;

// Pixel-format enumeration lives in pixelformat.cpp so that context creation
// uses the exact CGL format that was advertised to the GLX client.
void* macOpenGLChoosePixelFormat(U32 nativeId, int major);
bool macOpenGLGetPixelFormatInfo(U32 nativeId, bool* supportsPbuffer, bool* sampleBuffers, U32* samples,
                                U32* maxPbufferWidth, U32* maxPbufferHeight, U32* maxPbufferPixels);

void* macOpenGLCreateContext(U32 nativeId, int major, int minor, int profile, int flags, void* shareContext);
void macOpenGLDestroyContext(void* context);
bool macOpenGLSetWindow(void* context, SDL_Window* window);
bool macOpenGLMakeCurrent(void* context);
bool macOpenGLMakeCurrentPbuffer(void* context, void* pbuffer);
void macOpenGLClearCurrent();
void macOpenGLSwapBuffers(void* context);
void macOpenGLUpdateContext(void* context);
void macOpenGLDetachContext(void* context);

void* macOpenGLCreatePbuffer(U32 width, U32 height);
void macOpenGLDestroyPbuffer(void* pbuffer);

#if defined(__cplusplus) && defined(__TEST)
extern "C" int macOpenGLPbufferSmokeTest();
#endif

#endif
