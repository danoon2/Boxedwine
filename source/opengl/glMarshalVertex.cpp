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
#include "glMarshal.h"

U32 updateVertexPointer(CPU* cpu, OpenGLVetexPointer* p, U32 count) {
    if (p->isArrayBuffer) {
        klog("updateVertexPointer might have failed");
        return 0;
    }

    if (p->ptr) {        
        U32 datasize = count * p->size * (p->stride?p->stride:getDataSize(p->type));
        U32 available = K_PAGE_SIZE - (p->ptr & K_PAGE_MASK);

        if (p->marshal_size < datasize || p->lastMarshalledPtr != p->ptr) {
            U32 pageCount = 1 + ((datasize - available + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT);
            p->marshal_size = cpu->memory->ensureContinuousNative_unsafe(p->ptr >> K_PAGE_SHIFT, pageCount);
            U8* ramPtr = cpu->memory->getRamPtr(p->ptr, 1, false);
            if (ramPtr != p->marshal) {
                p->marshal = ramPtr;
                p->lastMarshalledPtr = p->ptr;
                return 1; // hope for the best, could be illegal if between glBegin/glEnd
            }
        }        
    } else {
        p->marshal_size = 0;
        p->marshal = nullptr;
    }
    return 0;
}

void updateVertexPointers(CPU* cpu, U32 count) {
    if (cpu->thread->glVertextPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glVertextPointer, count))
            GL_FUNC(pglVertexPointer)(cpu->thread->glVertextPointer.size, cpu->thread->glVertextPointer.type, cpu->thread->glVertextPointer.stride, cpu->thread->glVertextPointer.marshal);
    }
    for (auto& vp : cpu->thread->glVertextPointersByIndex) {
        if (vp.value->refreshEachCall) {
            if (updateVertexPointer(cpu, vp.value.get(), count))
                GL_FUNC(ext_glVertexAttribPointer)(vp.key, vp.value->size, vp.value->type, vp.value->normalized ? GL_TRUE : GL_FALSE, vp.value->stride, vp.value->marshal);
        }
    }

    if (cpu->thread->glNormalPointer.refreshEachCall) {
        cpu->thread->glNormalPointer.size = cpu->thread->glVertextPointer.size;
        if (updateVertexPointer(cpu, &cpu->thread->glNormalPointer, count))
            GL_FUNC(pglNormalPointer)(cpu->thread->glNormalPointer.type, cpu->thread->glNormalPointer.stride, cpu->thread->glNormalPointer.marshal);
    }

#ifndef DISABLE_GL_EXTENSIONS
    if (cpu->thread->glFogPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glFogPointer, count)) {
            if (ext_glFogCoordPointer)
                ext_glFogCoordPointer(cpu->thread->glFogPointer.type, cpu->thread->glFogPointer.stride, cpu->thread->glFogPointer.marshal);
        }
    }

    if (cpu->thread->glFogPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glFogPointerEXT, count)) {
            if (ext_glFogCoordPointerEXT)
                ext_glFogCoordPointerEXT(cpu->thread->glFogPointerEXT.type, cpu->thread->glFogPointerEXT.stride, cpu->thread->glFogPointerEXT.marshal);
        }
    }

    if (cpu->thread->glSecondaryColorPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointer, count)) {
            if (ext_glSecondaryColorPointer)
                ext_glSecondaryColorPointer(cpu->thread->glSecondaryColorPointer.size, cpu->thread->glSecondaryColorPointer.type, cpu->thread->glSecondaryColorPointer.stride, cpu->thread->glSecondaryColorPointer.marshal);
        }
    }

    if (cpu->thread->glSecondaryColorPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointerEXT, count)) {
            if (ext_glSecondaryColorPointerEXT)
                ext_glSecondaryColorPointerEXT(cpu->thread->glSecondaryColorPointerEXT.size, cpu->thread->glSecondaryColorPointerEXT.type, cpu->thread->glSecondaryColorPointerEXT.stride, cpu->thread->glSecondaryColorPointerEXT.marshal);
        }
    }

    if (cpu->thread->glEdgeFlagPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointerEXT, cpu->thread->glEdgeFlagPointerEXT.count)) {
            if (ext_glEdgeFlagPointerEXT)
                ext_glEdgeFlagPointerEXT(cpu->thread->glEdgeFlagPointerEXT.stride, cpu->thread->glEdgeFlagPointerEXT.count, cpu->thread->glEdgeFlagPointerEXT.marshal);
        }
    }
#endif
    if (cpu->thread->glColorPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glColorPointer, count))
            GL_FUNC(pglColorPointer)(cpu->thread->glColorPointer.size, cpu->thread->glColorPointer.type, cpu->thread->glColorPointer.stride, cpu->thread->glColorPointer.marshal);
    }    
    
    if (cpu->thread->glIndexPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glIndexPointer, count))
            GL_FUNC(pglIndexPointer)(cpu->thread->glIndexPointer.type, cpu->thread->glIndexPointer.stride, cpu->thread->glIndexPointer.marshal);
    }
    
    if (cpu->thread->glTexCoordPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glTexCoordPointer, count))
            GL_FUNC(pglTexCoordPointer)(cpu->thread->glTexCoordPointer.size, cpu->thread->glTexCoordPointer.type, cpu->thread->glTexCoordPointer.stride, cpu->thread->glTexCoordPointer.marshal);
    }
    
    if (cpu->thread->glEdgeFlagPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointer, count))
            GL_FUNC(pglEdgeFlagPointer)(cpu->thread->glEdgeFlagPointer.stride, cpu->thread->glEdgeFlagPointer.marshal);
    }
}

GLvoid* marshalVetextPointer(CPU* cpu, GLuint index, GLboolean normalized, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glVertextPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glVertextPointer.isArrayBuffer) {        
        cpu->thread->glVertextPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        OpenGLVetexPointer* p;

        if (index == 0) {
            p = &cpu->thread->glVertextPointer;
        }
        else {
            OpenGLVetexPointerPtr found = cpu->thread->glVertextPointersByIndex.get(index);
            if (!found) {
                found = std::make_shared<OpenGLVetexPointer>();
                cpu->thread->glVertextPointersByIndex.set(index, found);
            }
            p = found.get();
        }
        p->size = size;
        p->type = type;
        p->stride = stride;
        p->ptr = ptr;
        p->refreshEachCall = 1;
        p->normalized = normalized != GL_FALSE;
        updateVertexPointer(cpu, p, 0);
        return ptr ? p->marshal : 0;
    }
}

GLvoid* marshalNormalPointer(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glNormalPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glNormalPointer.isArrayBuffer) {
        cpu->thread->glNormalPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glNormalPointer.size = 1;
        cpu->thread->glNormalPointer.type = type;
        cpu->thread->glNormalPointer.stride = stride;
        cpu->thread->glNormalPointer.ptr = ptr;
        cpu->thread->glNormalPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glNormalPointer, 0);
        return ptr ? cpu->thread->glNormalPointer.marshal : 0;
    }
}

GLvoid* marshalFogPointer(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glFogPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glFogPointer.isArrayBuffer) {
        cpu->thread->glFogPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glFogPointer.size = 1;
        cpu->thread->glFogPointer.type = type;
        cpu->thread->glFogPointer.stride = stride;
        cpu->thread->glFogPointer.ptr = ptr;
        cpu->thread->glFogPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glFogPointer, 0);
        return ptr ? cpu->thread->glFogPointer.marshal : 0;
    }
}

GLvoid* marshalFogPointerEXT(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glFogPointerEXT.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glFogPointerEXT.isArrayBuffer) {     
        cpu->thread->glFogPointerEXT.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glFogPointerEXT.size = 1;
        cpu->thread->glFogPointerEXT.type = type;
        cpu->thread->glFogPointerEXT.stride = stride;
        cpu->thread->glFogPointerEXT.ptr = ptr;
        cpu->thread->glFogPointerEXT.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glFogPointerEXT, 0);
        return ptr ? cpu->thread->glFogPointerEXT.marshal : 0;
    }
}

GLvoid* marshalColorPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glColorPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glColorPointer.isArrayBuffer) {
        cpu->thread->glColorPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glColorPointer.size = size;
        cpu->thread->glColorPointer.type = type;
        cpu->thread->glColorPointer.stride = stride;
        cpu->thread->glColorPointer.ptr = ptr;
        cpu->thread->glColorPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glColorPointer, 0);
        return ptr ? cpu->thread->glColorPointer.marshal : 0;
    }
}

GLvoid* marshalSecondaryColorPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glSecondaryColorPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glSecondaryColorPointer.isArrayBuffer) {
        cpu->thread->glSecondaryColorPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {        
        cpu->thread->glSecondaryColorPointer.size = size;
        cpu->thread->glSecondaryColorPointer.type = type;
        cpu->thread->glSecondaryColorPointer.stride = stride;
        cpu->thread->glSecondaryColorPointer.ptr = ptr;
        cpu->thread->glSecondaryColorPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointer, 0);
        return ptr ? cpu->thread->glSecondaryColorPointer.marshal : 0;
    }
}

GLvoid* marshalSecondaryColorPointerEXT(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glSecondaryColorPointerEXT.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glSecondaryColorPointerEXT.isArrayBuffer) {
        cpu->thread->glSecondaryColorPointerEXT.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {        
        cpu->thread->glSecondaryColorPointerEXT.size = size;
        cpu->thread->glSecondaryColorPointerEXT.type = type;
        cpu->thread->glSecondaryColorPointerEXT.stride = stride;
        cpu->thread->glSecondaryColorPointerEXT.ptr = ptr;
        cpu->thread->glSecondaryColorPointerEXT.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glSecondaryColorPointerEXT, 0);
        return ptr ? cpu->thread->glSecondaryColorPointerEXT.marshal : 0;
    }
}

GLvoid* marshalIndexPointer(CPU* cpu,  GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glIndexPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glIndexPointer.isArrayBuffer) {
        cpu->thread->glIndexPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glIndexPointer.size = 1;
        cpu->thread->glIndexPointer.type = type;
        cpu->thread->glIndexPointer.stride = stride;
        cpu->thread->glIndexPointer.ptr = ptr;
        cpu->thread->glIndexPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glIndexPointer, 0);
        return ptr ? cpu->thread->glIndexPointer.marshal : 0;
    }
}

GLvoid* marshalTexCoordPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    cpu->thread->glTexCoordPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glTexCoordPointer.isArrayBuffer) {
        cpu->thread->glTexCoordPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glTexCoordPointer.size = size;
        cpu->thread->glTexCoordPointer.type = type;
        cpu->thread->glTexCoordPointer.stride = stride;
        cpu->thread->glTexCoordPointer.ptr = ptr;
        cpu->thread->glTexCoordPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glTexCoordPointer, 0);
        return ptr ? cpu->thread->glTexCoordPointer.marshal : 0;
    }
}

GLvoid* marshalEdgeFlagPointer(CPU* cpu, GLsizei stride, U32 ptr) {
    cpu->thread->glEdgeFlagPointer.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glEdgeFlagPointer.isArrayBuffer) {
        cpu->thread->glEdgeFlagPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glEdgeFlagPointer.size = 1;
        cpu->thread->glEdgeFlagPointer.type = GL_BYTE;
        cpu->thread->glEdgeFlagPointer.stride = stride;
        cpu->thread->glEdgeFlagPointer.ptr = ptr;
        cpu->thread->glEdgeFlagPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointer, 0);
        return ptr ? cpu->thread->glEdgeFlagPointer.marshal : 0;
    }
}

const GLboolean* marshalEdgeFlagPointerEXT(CPU* cpu, GLsizei stride, GLsizei count, U32 ptr) {
    cpu->thread->glEdgeFlagPointerEXT.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glEdgeFlagPointerEXT.isArrayBuffer) {
        cpu->thread->glEdgeFlagPointerEXT.refreshEachCall = 0;
        return (const GLboolean*)(uintptr_t)ptr;
    } else {
        cpu->thread->glEdgeFlagPointerEXT.size = 1;
        cpu->thread->glEdgeFlagPointerEXT.type = GL_BYTE;
        cpu->thread->glEdgeFlagPointerEXT.stride = stride;
        cpu->thread->glEdgeFlagPointerEXT.ptr = ptr;
        cpu->thread->glEdgeFlagPointerEXT.refreshEachCall = 1;
        cpu->thread->glEdgeFlagPointerEXT.count = count;
        updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointerEXT, 0);
        return ptr ? cpu->thread->glEdgeFlagPointerEXT.marshal : 0;
    }
}

static bool interleavedHasColor(GLenum format) {
    switch (format) {
    case GL_C4UB_V2F:
    case GL_C4UB_V3F:
    case GL_C3F_V3F:
    case GL_C4F_N3F_V3F:
    case GL_T2F_C4UB_V3F:
    case GL_T2F_C3F_V3F:
    case GL_T2F_C4F_N3F_V3F:
    case GL_T4F_C4F_N3F_V4F:
        return true;
    default:
        return false;
    }
}

static bool interleavedHasTexture(GLenum format) {
    switch (format) {
    case GL_T2F_V3F:
    case GL_T4F_V4F:
    case GL_T2F_C4UB_V3F:
    case GL_T2F_C3F_V3F:
    case GL_T2F_N3F_V3F:
    case GL_T2F_C4F_N3F_V3F:
    case GL_T4F_C4F_N3F_V4F:
        return true;
    default:
        return false;
    }
}

static bool interleavedHasNormal(GLenum format) {
    switch (format) {
    case GL_N3F_V3F:
    case GL_C4F_N3F_V3F:
    case GL_T2F_N3F_V3F:
    case GL_T2F_C4F_N3F_V3F:
    case GL_T4F_C4F_N3F_V4F:
        return true;
    default:
        return false;
    }
}

const void* marshalInterleavedPointer(CPU* cpu, GLenum format, GLsizei stride, U32 ptr) {
    cpu->thread->glVertextPointer.refreshEachCall = 0;
    if (interleavedHasColor(format)) {
        cpu->thread->glColorPointer.refreshEachCall = 0;
    }
    if (interleavedHasNormal(format)) {
        cpu->thread->glNormalPointer.refreshEachCall = 0;
    }
    if (interleavedHasTexture(format)) {
        cpu->thread->glTexCoordPointer.refreshEachCall = 0;
    }
    if (ARRAY_BUFFER()) {
        cpu->thread->glInterleavedArray.refreshEachCall = 0;
        return (const GLboolean*)(uintptr_t)ptr;
    }
    else {
        cpu->thread->glInterleavedArray.size = 1;
        cpu->thread->glInterleavedArray.type = format;
        cpu->thread->glInterleavedArray.stride = stride;
        cpu->thread->glInterleavedArray.ptr = ptr;
        cpu->thread->glInterleavedArray.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glInterleavedArray, 0);
        return cpu->thread->glInterleavedArray.marshal;
    }
}

#endif
