/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __EMSCRIPTEN_GL_PROC_ADDRESS_H__
#define __EMSCRIPTEN_GL_PROC_ADDRESS_H__

#include <string.h>

inline bool boxedwineIsUnsupportedEmscriptenGLProcAddress(const char* name) {
    if (!name) {
        return true;
    }

    if (strstr(name, "Convolution")
            || strstr(name, "Histogram")
            || strstr(name, "Minmax")
            || strstr(name, "SeparableFilter")
            || strstr(name, "ColorTable")) {
        return true;
    }

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

    // Emscripten's GL proc-address shim strips vendor suffixes before lookup.
    // Fixed-point OES names can therefore match desktop/legacy entry points
    // with incompatible wasm signatures.
    size_t len = strlen(name);
    if (len < 6 || strcmp(name + len - 3, "OES") != 0) {
        return false;
    }
    return strstr(name, "xOES") || strstr(name, "xvOES");
}

#endif
