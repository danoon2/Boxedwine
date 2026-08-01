/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_OPENGL_H__
#define __TEST_OPENGL_H__

#if defined(__EMSCRIPTEN__) && defined(BOXEDWINE_MULTI_THREADED) && defined(BOXEDWINE_OPENGL_SDL) && defined(BOXEDWINE_OPENGL_BOOTSTRAP_TEST_ONLY)
void testEmscriptenMtOpenGLProcAddressBootstrap();
#endif

#endif
