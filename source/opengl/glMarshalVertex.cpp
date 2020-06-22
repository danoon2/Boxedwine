#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"
#include "glMarshal.h"

#ifndef BOXEDWINE_64BIT_MMU
U32 updateVertexPointer(CPU* cpu, OpenGLVetexPointer* p, U32 count) {
    if (ARRAY_BUFFER()) {
        klog("updateVertexPointer might have failed");
        return 0;
    }
    if (p->ptr) {        
        U32 datasize = count * p->size * (p->stride?p->stride:getDataSize(p->type));    
        U32 available = K_PAGE_SIZE - (p->ptr & K_PAGE_MASK) + (1 << K_PAGE_SHIFT);

#ifndef UNALIGNED_MEMORY
        if (count == 0 || available > datasize) {
            if (p->marshal_size) {
                free(p->marshal);
            }            
            p->marshal = getPhysicalAddress(p->ptr, (datasize?datasize:available));
            p->marshal_size = 0;
            
            if (p->marshal) {
                if (p->refreshEachCall)
                    return 1; 
                // the datasize is still < available so we don't need to marshal the pointer
                return 0;
            }
        }
#endif
        if (count == 0) {
            datasize = available; // :TODO: should this be capped at all?
        }
        if (p->marshal_size < datasize) {
            if (p->marshal_size) {
                delete[] p->marshal;
            }
            p->marshal = new unsigned char[datasize];
            p->marshal_size = datasize;
        }
        memcopyToNative(p->ptr, p->marshal, datasize);
    } else {
        if (p->marshal_size) {
            free(p->marshal);
            p->marshal_size = 0;
        }
        p->marshal = (U8*)(uintptr_t)p->ptr;
    }
    return 1;
}

void updateVertexPointers(CPU* cpu, U32 count) {    
    if (cpu->thread->glVertextPointer.refreshEachCall) {        
        if (updateVertexPointer(cpu, &cpu->thread->glVertextPointer, count))
            GL_FUNC(pglVertexPointer)(cpu->thread->glVertextPointer.size, cpu->thread->glVertextPointer.type, cpu->thread->glVertextPointer.stride, cpu->thread->glVertextPointer.marshal);
    }
    
    if (cpu->thread->gglNormalPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglNormalPointer, count))
            GL_FUNC(glNormalPointer)(cpu->thread->gglNormalPointer.type, cpu->thread->gglNormalPointer.stride, cpu->thread->gglNormalPointer.marshal);
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

    if (cpu->thread->gglSecondaryColorPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglSecondaryColorPointer, count)) {
            if (ext_glSecondaryColorPointer)
                ext_glSecondaryColorPointer(cpu->thread->gglSecondaryColorPointer.size, cpu->thread->gglSecondaryColorPointer.type, cpu->thread->gglSecondaryColorPointer.stride, cpu->thread->gglSecondaryColorPointer.marshal);
        }
    }

    if (cpu->thread->gglSecondaryColorPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglSecondaryColorPointerEXT, count)) {
            if (ext_glSecondaryColorPointerEXT)
                ext_glSecondaryColorPointerEXT(cpu->thread->gglSecondaryColorPointerEXT.size, cpu->thread->gglSecondaryColorPointerEXT.type, cpu->thread->gglSecondaryColorPointerEXT.stride, cpu->thread->gglSecondaryColorPointerEXT.marshal);
        }
    }

    if (cpu->thread->gglEdgeFlagPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglEdgeFlagPointerEXT, count)) {
            if (ext_glEdgeFlagPointerEXT)
                ext_glEdgeFlagPointerEXT(cpu->thread->gglEdgeFlagPointerEXT.stride, cpu->thread->gglEdgeFlagPointerEXT.count, cpu->thread->gglEdgeFlagPointerEXT.marshal);
        }
    }
#endif
    if (cpu->thread->gglColorPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglColorPointer, count))
            GL_FUNC(glColorPointer)(cpu->thread->gglColorPointer.size, cpu->thread->gglColorPointer.type, cpu->thread->gglColorPointer.stride, cpu->thread->gglColorPointer.marshal);
    }    
    
    if (cpu->thread->gglIndexPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglIndexPointer, count))
            GL_FUNC(glIndexPointer)(cpu->thread->gglIndexPointer.type, cpu->thread->gglIndexPointer.stride, cpu->thread->gglIndexPointer.marshal);
    }
    
    if (cpu->thread->gglTexCoordPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglTexCoordPointer, count))
            GL_FUNC(glTexCoordPointer)(cpu->thread->gglTexCoordPointer.size, cpu->thread->gglTexCoordPointer.type, cpu->thread->gglTexCoordPointer.stride, cpu->thread->gglTexCoordPointer.marshal);
    }
    
    if (cpu->thread->gglEdgeFlagPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->gglEdgeFlagPointer, count))
            GL_FUNC(glEdgeFlagPointer)(cpu->thread->gglEdgeFlagPointer.stride, cpu->thread->gglEdgeFlagPointer.marshal);
    }
}

GLvoid* marshalVetextPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glVertextPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glVertextPointer.size = size;
        cpu->thread->glVertextPointer.type = type;
        cpu->thread->glVertextPointer.stride = stride;
        cpu->thread->glVertextPointer.ptr = ptr;
        cpu->thread->glVertextPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glVertextPointer, 0);
        return cpu->thread->glVertextPointer.marshal;
    }
}

GLvoid* marshalNormalPointer(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglNormalPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->gglNormalPointer.size = 1;
        cpu->thread->gglNormalPointer.type = type;
        cpu->thread->gglNormalPointer.stride = stride;
        cpu->thread->gglNormalPointer.ptr = ptr;
        cpu->thread->gglNormalPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->gglNormalPointer, 0);
        return cpu->thread->gglNormalPointer.marshal;
    }
}

GLvoid* marshalFogPointer(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glFogPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glFogPointer.size = 1;
        cpu->thread->glFogPointer.type = type;
        cpu->thread->glFogPointer.stride = stride;
        cpu->thread->glFogPointer.ptr = ptr;
        cpu->thread->glFogPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glFogPointer, 0);
        return cpu->thread->glFogPointer.marshal;
    }
}

GLvoid* marshalFogPointerEXT(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->glFogPointerEXT.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glFogPointerEXT.size = 1;
        cpu->thread->glFogPointerEXT.type = type;
        cpu->thread->glFogPointerEXT.stride = stride;
        cpu->thread->glFogPointerEXT.ptr = ptr;
        cpu->thread->glFogPointerEXT.refreshEachCall = 0;
        updateVertexPointer(cpu, &cpu->thread->glFogPointerEXT, 0);
        return cpu->thread->glFogPointerEXT.marshal;
    }
}

GLvoid* marshalColorPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglColorPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->gglColorPointer.size = size;
        cpu->thread->gglColorPointer.type = type;
        cpu->thread->gglColorPointer.stride = stride;
        cpu->thread->gglColorPointer.ptr = ptr;
        cpu->thread->gglColorPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->gglColorPointer, 0);
        return cpu->thread->gglColorPointer.marshal;
    }
}

GLvoid* marshalSecondaryColorPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglSecondaryColorPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {        
        cpu->thread->gglSecondaryColorPointer.size = size;
        cpu->thread->gglSecondaryColorPointer.type = type;
        cpu->thread->gglSecondaryColorPointer.stride = stride;
        cpu->thread->gglSecondaryColorPointer.ptr = ptr;
        cpu->thread->gglSecondaryColorPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->gglSecondaryColorPointer, 0);
        return cpu->thread->gglSecondaryColorPointer.marshal;
    }
}

GLvoid* marshalSecondaryColorPointerEXT(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglSecondaryColorPointerEXT.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {        
        cpu->thread->gglSecondaryColorPointerEXT.size = size;
        cpu->thread->gglSecondaryColorPointerEXT.type = type;
        cpu->thread->gglSecondaryColorPointerEXT.stride = stride;
        cpu->thread->gglSecondaryColorPointerEXT.ptr = ptr;
        cpu->thread->gglSecondaryColorPointerEXT.refreshEachCall = 0;
        updateVertexPointer(cpu, &cpu->thread->gglSecondaryColorPointerEXT, 0);
        return cpu->thread->gglSecondaryColorPointerEXT.marshal;
    }
}

GLvoid* marshalIndexPointer(CPU* cpu,  GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglIndexPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->gglIndexPointer.size = 1;
        cpu->thread->gglIndexPointer.type = type;
        cpu->thread->gglIndexPointer.stride = stride;
        cpu->thread->gglIndexPointer.ptr = ptr;
        cpu->thread->gglIndexPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->gglIndexPointer, 0);
        return cpu->thread->gglIndexPointer.marshal;
    }
}

GLvoid* marshalTexCoordPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglTexCoordPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->gglTexCoordPointer.size = size;
        cpu->thread->gglTexCoordPointer.type = type;
        cpu->thread->gglTexCoordPointer.stride = stride;
        cpu->thread->gglTexCoordPointer.ptr = ptr;
        cpu->thread->gglTexCoordPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->gglTexCoordPointer, 0);
        return cpu->thread->gglTexCoordPointer.marshal;
    }
}

GLvoid* marshalEdgeFlagPointer(CPU* cpu, GLsizei stride, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglEdgeFlagPointer.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->gglEdgeFlagPointer.size = 1;
        cpu->thread->gglEdgeFlagPointer.type = GL_BYTE;
        cpu->thread->gglEdgeFlagPointer.stride = stride;
        cpu->thread->gglEdgeFlagPointer.ptr = ptr;
        cpu->thread->gglEdgeFlagPointer.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->gglEdgeFlagPointer, 0);
        return cpu->thread->gglEdgeFlagPointer.marshal;
    }
}

const GLboolean* marshalEdgeFlagPointerEXT(CPU* cpu, GLsizei stride, GLsizei count, U32 ptr) {
    if (ARRAY_BUFFER()) {        
        cpu->thread->gglEdgeFlagPointerEXT.refreshEachCall = 0;
        return (const GLboolean*)(uintptr_t)ptr;
    } else {
        cpu->thread->gglEdgeFlagPointerEXT.size = 1;
        cpu->thread->gglEdgeFlagPointerEXT.type = GL_BYTE;
        cpu->thread->gglEdgeFlagPointerEXT.stride = stride;
        cpu->thread->gglEdgeFlagPointerEXT.ptr = ptr;
        cpu->thread->gglEdgeFlagPointerEXT.refreshEachCall = 0;
        cpu->thread->gglEdgeFlagPointerEXT.count = count;
        updateVertexPointer(cpu, &cpu->thread->gglEdgeFlagPointerEXT, 0);
        return cpu->thread->gglEdgeFlagPointerEXT.marshal;
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
#endif
