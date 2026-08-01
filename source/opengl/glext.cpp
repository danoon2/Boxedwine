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

#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern BHashTable<BString, void*> glFunctionMap;

const char* glIsLoaded[GL_FUNC_COUNT];

#ifdef __EMSCRIPTEN__
EM_JS(void, boxedwineGetBufferSubDataJS,
    (int target, int offset, int size, int data), {
        if (!GLctx || typeof GLctx.getBufferSubData !== "function") {
            GL.recordError(0x0502); // GL_INVALID_OPERATION
            return;
        }
        GLctx.getBufferSubData(target, offset, HEAPU8, data, size);
    });

static void OPENGL_CALL_TYPE boxedwineGetBufferSubData(
        GLenum target, GLintptr offset, GLsizeiptr size, void* data) {
    boxedwineGetBufferSubDataJS((int)target, (int)offset, (int)size,
        (int)(uintptr_t)data);
}
#endif

void glExtensionsLoaded() {
#ifdef __EMSCRIPTEN__
    // WebGL 2 exposes getBufferSubData(), but GLES does not define the C API,
    // so Emscripten has no glGetBufferSubData symbol for SDL to resolve.
    // Supply the desktop GL entry points Wine expects through the WebGL call.
    ext_glGetBufferSubData = boxedwineGetBufferSubData;
    ext_glGetBufferSubDataARB = boxedwineGetBufferSubData;
#endif
}
#endif
