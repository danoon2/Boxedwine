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
        U32 elementSize = p->stride ? p->stride : p->size * getDataSize(p->type);
        U32 datasize = count * elementSize;
        U32 available = K_PAGE_SIZE - (p->ptr & K_PAGE_MASK);

        if (!datasize) {
            U8* ramPtr = cpu->memory->getRamPtr(p->ptr, 1, false);
            p->marshal_size = 0;
            if (ramPtr != p->marshal || p->lastMarshalledPtr != p->ptr) {
                p->marshal = ramPtr;
                p->lastMarshalledPtr = p->ptr;
                return 1;
            }
            return 0;
        }

        if (p->marshal_size < datasize || p->lastMarshalledPtr != p->ptr) {
            U32 pageCount = 1;
            if (datasize > available) {
                pageCount += (datasize - available + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT;
            }
            U32 continuousSize = cpu->memory->ensureContinuousNative_unsafe(p->ptr >> K_PAGE_SHIFT, pageCount);
            U32 pageOffset = p->ptr & K_PAGE_MASK;
            p->marshal_size = continuousSize > pageOffset ? continuousSize - pageOffset : 0;
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

static void clearInterleavedArrayReplay(CPU* cpu) {
    cpu->thread->glInterleavedArray.refreshEachCall = 0;
}

static OpenGLVetexPointer* getClientPointerByKey(BHashTable<U32, OpenGLVetexPointerPtr>& pointers, U32 key) {
    OpenGLVetexPointerPtr found = pointers.get(key);
    if (!found) {
        found = std::make_shared<OpenGLVetexPointer>();
        pointers.set(key, found);
    }
    return found.get();
}

OpenGLVetexPointer* getCurrentTexCoordPointer(CPU* cpu) {
    return getClientPointerByKey(cpu->thread->glTexCoordPointersByTexture,
        cpu->thread->glClientActiveTexture);
}

void syncClientActiveTextureFromHost(CPU* cpu) {
    GLint texture = GL_TEXTURE0;
    GL_FUNC(pglGetIntegerv)(GL_CLIENT_ACTIVE_TEXTURE, &texture);
    cpu->thread->glClientActiveTexture = (U32)texture;
}

static bool selectClientActiveTextureForReplay(GLenum texture) {
#ifndef DISABLE_GL_EXTENSIONS
    if (ext_glClientActiveTexture) {
        GL_FUNC(ext_glClientActiveTexture)(texture);
        return true;
    }
    if (ext_glClientActiveTextureARB) {
        GL_FUNC(ext_glClientActiveTextureARB)(texture);
        return true;
    }
#endif
    return texture == GL_TEXTURE0;
}

void updateVertexPointers(CPU* cpu, U32 count) {
    if (cpu->thread->glVertextPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glVertextPointer, count)) {
            if (cpu->thread->glVertextPointer.isVertexAttrib && ext_glVertexAttribPointer) {
                GL_FUNC(ext_glVertexAttribPointer)(0, cpu->thread->glVertextPointer.size, cpu->thread->glVertextPointer.type, cpu->thread->glVertextPointer.normalized ? GL_TRUE : GL_FALSE, cpu->thread->glVertextPointer.stride, cpu->thread->glVertextPointer.marshal);
            } else {
                GL_FUNC(pglVertexPointer)(cpu->thread->glVertextPointer.size, cpu->thread->glVertextPointer.type, cpu->thread->glVertextPointer.stride, cpu->thread->glVertextPointer.marshal);
            }
        }
    }
    if (cpu->thread->glInterleavedArray.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glInterleavedArray, count) &&
            selectClientActiveTextureForReplay(cpu->thread->glInterleavedArrayTexture)) {
            GL_FUNC(pglInterleavedArrays)(cpu->thread->glInterleavedArray.type,
                cpu->thread->glInterleavedArray.stride,
                cpu->thread->glInterleavedArray.marshal);
            if (cpu->thread->glInterleavedArrayTexture != cpu->thread->glClientActiveTexture) {
                selectClientActiveTextureForReplay(cpu->thread->glClientActiveTexture);
            }
        }
    }
    for (auto& vp : cpu->thread->glVertextPointersByIndex) {
        if (vp.value->refreshEachCall) {
            if (updateVertexPointer(cpu, vp.value.get(), count))
                GL_FUNC(ext_glVertexAttribPointer)(vp.key, vp.value->size, vp.value->type, vp.value->normalized ? GL_TRUE : GL_FALSE, vp.value->stride, vp.value->marshal);
        }
    }
    for (auto& vp : cpu->thread->glVertexAttribPointerNVByIndex) {
        if (vp.value->refreshEachCall) {
            if (updateVertexPointer(cpu, vp.value.get(), count) && ext_glVertexAttribPointerNV)
                ext_glVertexAttribPointerNV(vp.key, vp.value->size, vp.value->type, vp.value->stride, vp.value->marshal);
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

    if (cpu->thread->glTangentPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glTangentPointerEXT, count)) {
            if (ext_glTangentPointerEXT)
                ext_glTangentPointerEXT(cpu->thread->glTangentPointerEXT.type, cpu->thread->glTangentPointerEXT.stride, cpu->thread->glTangentPointerEXT.marshal);
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
    
    U32 replayTexture = cpu->thread->glClientActiveTexture;
    for (auto& vp : cpu->thread->glTexCoordPointersByTexture) {
        if (vp.value->refreshEachCall && updateVertexPointer(cpu, vp.value.get(), count)) {
            if (selectClientActiveTextureForReplay(vp.key)) {
                GL_FUNC(pglTexCoordPointer)(vp.value->size, vp.value->type,
                    vp.value->stride, vp.value->marshal);
                replayTexture = vp.key;
            }
        }
    }
    if (replayTexture != cpu->thread->glClientActiveTexture) {
        selectClientActiveTextureForReplay(cpu->thread->glClientActiveTexture);
    }

#ifndef DISABLE_GL_EXTENSIONS
    for (auto& vp : cpu->thread->glMultiTexCoordPointerEXTByTexunit) {
        if (vp.value->refreshEachCall) {
            if (updateVertexPointer(cpu, vp.value.get(), count) && ext_glMultiTexCoordPointerEXT)
                ext_glMultiTexCoordPointerEXT(vp.key, vp.value->size, vp.value->type, vp.value->stride, vp.value->marshal);
        }
    }

    for (auto& vp : cpu->thread->glMultiTexCoordPointerSGISByTarget) {
        if (vp.value->refreshEachCall) {
            if (updateVertexPointer(cpu, vp.value.get(), count) && ext_glMultiTexCoordPointerSGIS)
                ext_glMultiTexCoordPointerSGIS(vp.key, vp.value->size, vp.value->type, vp.value->stride, vp.value->marshal);
        }
    }

    for (auto& vp : cpu->thread->glVariantPointerEXTById) {
        if (vp.value->refreshEachCall) {
            if (updateVertexPointer(cpu, vp.value.get(), count) && ext_glVariantPointerEXT)
                ext_glVariantPointerEXT(vp.key, vp.value->type, vp.value->stride, vp.value->marshal);
        }
    }

    if (cpu->thread->glMatrixIndexPointerARB.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glMatrixIndexPointerARB, count)) {
            if (ext_glMatrixIndexPointerARB)
                ext_glMatrixIndexPointerARB(cpu->thread->glMatrixIndexPointerARB.size, cpu->thread->glMatrixIndexPointerARB.type, cpu->thread->glMatrixIndexPointerARB.stride, cpu->thread->glMatrixIndexPointerARB.marshal);
        }
    }

    if (cpu->thread->glVertexWeightPointerEXT.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glVertexWeightPointerEXT, count)) {
            if (ext_glVertexWeightPointerEXT)
                ext_glVertexWeightPointerEXT(cpu->thread->glVertexWeightPointerEXT.size, cpu->thread->glVertexWeightPointerEXT.type, cpu->thread->glVertexWeightPointerEXT.stride, cpu->thread->glVertexWeightPointerEXT.marshal);
        }
    }

    if (cpu->thread->glWeightPointerARB.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glWeightPointerARB, count)) {
            if (ext_glWeightPointerARB)
                ext_glWeightPointerARB(cpu->thread->glWeightPointerARB.size, cpu->thread->glWeightPointerARB.type, cpu->thread->glWeightPointerARB.stride, cpu->thread->glWeightPointerARB.marshal);
        }
    }
#endif

    if (cpu->thread->glEdgeFlagPointer.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glEdgeFlagPointer, count))
            GL_FUNC(pglEdgeFlagPointer)(cpu->thread->glEdgeFlagPointer.stride, cpu->thread->glEdgeFlagPointer.marshal);
    }
}

static U32 getArrayElementAddress(const OpenGLVetexPointer* p, GLint i) {
    U32 elementSize = p->stride ? p->stride : p->size * getDataSize(p->type);
    return p->ptr + (U32)i * elementSize;
}

static bool usesGuestArray(const OpenGLVetexPointer& p) {
    return p.enabled && p.ptr && !p.isArrayBuffer;
}

static bool usesHostArray(const OpenGLVetexPointer& p) {
    return p.enabled && p.isArrayBuffer;
}

static void emitVertexArrayElement(CPU* cpu, OpenGLVetexPointer* p, GLint i) {
    U32 address = getArrayElementAddress(p, i);
    switch (p->type) {
    case GL_SHORT: {
        const GLshort* v = marshalArray<GLshort>(cpu, address, p->size);
        if (p->size == 2) GL_FUNC(pglVertex2sv)(v);
        else if (p->size == 3) GL_FUNC(pglVertex3sv)(v);
        else GL_FUNC(pglVertex4sv)(v);
        break;
    }
    case GL_INT: {
        const GLint* v = marshalArray<GLint>(cpu, address, p->size);
        if (p->size == 2) GL_FUNC(pglVertex2iv)(v);
        else if (p->size == 3) GL_FUNC(pglVertex3iv)(v);
        else GL_FUNC(pglVertex4iv)(v);
        break;
    }
    case GL_FLOAT: {
        const GLfloat* v = marshalArray<GLfloat>(cpu, address, p->size);
        if (p->size == 2) GL_FUNC(pglVertex2fv)(v);
        else if (p->size == 3) GL_FUNC(pglVertex3fv)(v);
        else GL_FUNC(pglVertex4fv)(v);
        break;
    }
    case GL_DOUBLE: {
        const GLdouble* v = marshalArray<GLdouble>(cpu, address, p->size);
        if (p->size == 2) GL_FUNC(pglVertex2dv)(v);
        else if (p->size == 3) GL_FUNC(pglVertex3dv)(v);
        else GL_FUNC(pglVertex4dv)(v);
        break;
    }
    default:
        GL_FUNC(pglArrayElement)(i);
        break;
    }
}

static void emitNormalArrayElement(CPU* cpu, OpenGLVetexPointer* p, GLint i) {
    U32 address = getArrayElementAddress(p, i);
    switch (p->type) {
    case GL_BYTE: GL_FUNC(pglNormal3bv)(marshalArray<GLbyte>(cpu, address, 3)); break;
    case GL_SHORT: GL_FUNC(pglNormal3sv)(marshalArray<GLshort>(cpu, address, 3)); break;
    case GL_INT: GL_FUNC(pglNormal3iv)(marshalArray<GLint>(cpu, address, 3)); break;
    case GL_FLOAT: GL_FUNC(pglNormal3fv)(marshalArray<GLfloat>(cpu, address, 3)); break;
    case GL_DOUBLE: GL_FUNC(pglNormal3dv)(marshalArray<GLdouble>(cpu, address, 3)); break;
    default: break;
    }
}

static void emitColorArrayElement(CPU* cpu, OpenGLVetexPointer* p, GLint i) {
    U32 address = getArrayElementAddress(p, i);
    switch (p->type) {
    case GL_BYTE:
        if (p->size == 3) GL_FUNC(pglColor3bv)(marshalArray<GLbyte>(cpu, address, 3));
        else GL_FUNC(pglColor4bv)(marshalArray<GLbyte>(cpu, address, 4));
        break;
    case GL_UNSIGNED_BYTE:
        if (p->size == 3) GL_FUNC(pglColor3ubv)(marshalArray<GLubyte>(cpu, address, 3));
        else GL_FUNC(pglColor4ubv)(marshalArray<GLubyte>(cpu, address, 4));
        break;
    case GL_SHORT:
        if (p->size == 3) GL_FUNC(pglColor3sv)(marshalArray<GLshort>(cpu, address, 3));
        else GL_FUNC(pglColor4sv)(marshalArray<GLshort>(cpu, address, 4));
        break;
    case GL_UNSIGNED_SHORT:
        if (p->size == 3) GL_FUNC(pglColor3usv)(marshalArray<GLushort>(cpu, address, 3));
        else GL_FUNC(pglColor4usv)(marshalArray<GLushort>(cpu, address, 4));
        break;
    case GL_INT:
        if (p->size == 3) GL_FUNC(pglColor3iv)(marshalArray<GLint>(cpu, address, 3));
        else GL_FUNC(pglColor4iv)(marshalArray<GLint>(cpu, address, 4));
        break;
    case GL_UNSIGNED_INT:
        if (p->size == 3) GL_FUNC(pglColor3uiv)(marshalArray<GLuint>(cpu, address, 3));
        else GL_FUNC(pglColor4uiv)(marshalArray<GLuint>(cpu, address, 4));
        break;
    case GL_FLOAT:
        if (p->size == 3) GL_FUNC(pglColor3fv)(marshalArray<GLfloat>(cpu, address, 3));
        else GL_FUNC(pglColor4fv)(marshalArray<GLfloat>(cpu, address, 4));
        break;
    case GL_DOUBLE:
        if (p->size == 3) GL_FUNC(pglColor3dv)(marshalArray<GLdouble>(cpu, address, 3));
        else GL_FUNC(pglColor4dv)(marshalArray<GLdouble>(cpu, address, 4));
        break;
    default:
        break;
    }
}

static void emitIndexArrayElement(CPU* cpu, OpenGLVetexPointer* p, GLint i) {
    U32 address = getArrayElementAddress(p, i);
    switch (p->type) {
    case GL_UNSIGNED_BYTE: GL_FUNC(pglIndexubv)(marshalArray<GLubyte>(cpu, address, 1)); break;
    case GL_SHORT: GL_FUNC(pglIndexsv)(marshalArray<GLshort>(cpu, address, 1)); break;
    case GL_INT: GL_FUNC(pglIndexiv)(marshalArray<GLint>(cpu, address, 1)); break;
    case GL_FLOAT: GL_FUNC(pglIndexfv)(marshalArray<GLfloat>(cpu, address, 1)); break;
    case GL_DOUBLE: GL_FUNC(pglIndexdv)(marshalArray<GLdouble>(cpu, address, 1)); break;
    default: break;
    }
}

#ifndef DISABLE_GL_EXTENSIONS
#define EMIT_MULTI_TEX_COORD_OR_UNIT0(count, suffix, texture, value) do { \
    if (ext_glMultiTexCoord##count##suffix) { \
        GL_FUNC(ext_glMultiTexCoord##count##suffix)(texture, value); \
    } else if (ext_glMultiTexCoord##count##suffix##ARB) { \
        GL_FUNC(ext_glMultiTexCoord##count##suffix##ARB)(texture, value); \
    } else if (texture == GL_TEXTURE0) { \
        GL_FUNC(pglTexCoord##count##suffix)(value); \
    } \
} while (0)
#else
#define EMIT_MULTI_TEX_COORD_OR_UNIT0(count, suffix, texture, value) do { \
    if (texture == GL_TEXTURE0) { \
        GL_FUNC(pglTexCoord##count##suffix)(value); \
    } \
} while (0)
#endif

static void emitTexCoordArrayElement(CPU* cpu, GLenum texture,
    OpenGLVetexPointer* p, GLint i) {
    U32 address = getArrayElementAddress(p, i);
    switch (p->type) {
    case GL_SHORT: {
        const GLshort* v = marshalArray<GLshort>(cpu, address, p->size);
        if (p->size == 1) EMIT_MULTI_TEX_COORD_OR_UNIT0(1, sv, texture, v);
        else if (p->size == 2) EMIT_MULTI_TEX_COORD_OR_UNIT0(2, sv, texture, v);
        else if (p->size == 3) EMIT_MULTI_TEX_COORD_OR_UNIT0(3, sv, texture, v);
        else EMIT_MULTI_TEX_COORD_OR_UNIT0(4, sv, texture, v);
        break;
    }
    case GL_INT: {
        const GLint* v = marshalArray<GLint>(cpu, address, p->size);
        if (p->size == 1) EMIT_MULTI_TEX_COORD_OR_UNIT0(1, iv, texture, v);
        else if (p->size == 2) EMIT_MULTI_TEX_COORD_OR_UNIT0(2, iv, texture, v);
        else if (p->size == 3) EMIT_MULTI_TEX_COORD_OR_UNIT0(3, iv, texture, v);
        else EMIT_MULTI_TEX_COORD_OR_UNIT0(4, iv, texture, v);
        break;
    }
    case GL_FLOAT: {
        const GLfloat* v = marshalArray<GLfloat>(cpu, address, p->size);
        if (p->size == 1) EMIT_MULTI_TEX_COORD_OR_UNIT0(1, fv, texture, v);
        else if (p->size == 2) EMIT_MULTI_TEX_COORD_OR_UNIT0(2, fv, texture, v);
        else if (p->size == 3) EMIT_MULTI_TEX_COORD_OR_UNIT0(3, fv, texture, v);
        else EMIT_MULTI_TEX_COORD_OR_UNIT0(4, fv, texture, v);
        break;
    }
    case GL_DOUBLE: {
        const GLdouble* v = marshalArray<GLdouble>(cpu, address, p->size);
        if (p->size == 1) EMIT_MULTI_TEX_COORD_OR_UNIT0(1, dv, texture, v);
        else if (p->size == 2) EMIT_MULTI_TEX_COORD_OR_UNIT0(2, dv, texture, v);
        else if (p->size == 3) EMIT_MULTI_TEX_COORD_OR_UNIT0(3, dv, texture, v);
        else EMIT_MULTI_TEX_COORD_OR_UNIT0(4, dv, texture, v);
        break;
    }
    default:
        break;
    }
}

#undef EMIT_MULTI_TEX_COORD_OR_UNIT0

static void emitEdgeFlagArrayElement(CPU* cpu, OpenGLVetexPointer* p, GLint i) {
    GLboolean flag = cpu->memory->readb(getArrayElementAddress(p, i)) ? GL_TRUE : GL_FALSE;
    GL_FUNC(pglEdgeFlagv)(&flag);
}

void marshalArrayElement(CPU* cpu, GLint i) {
    if (i < 0) {
        GL_FUNC(pglArrayElement)(i);
        return;
    }

    KThread* thread = cpu->thread;
    bool hostArray = usesHostArray(thread->glVertextPointer) ||
        usesHostArray(thread->glNormalPointer) ||
        usesHostArray(thread->glColorPointer) ||
        usesHostArray(thread->glIndexPointer) ||
        usesHostArray(thread->glEdgeFlagPointer);
    bool guestTexCoordArray = false;
    for (auto& vp : thread->glTexCoordPointersByTexture) {
        hostArray = hostArray || usesHostArray(*vp.value);
        guestTexCoordArray = guestTexCoordArray || usesGuestArray(*vp.value);
    }
    if (hostArray) {
        GL_FUNC(pglArrayElement)(i);
        return;
    }

    bool guestArray = usesGuestArray(thread->glVertextPointer) ||
        usesGuestArray(thread->glNormalPointer) ||
        usesGuestArray(thread->glColorPointer) ||
        usesGuestArray(thread->glIndexPointer) ||
        guestTexCoordArray ||
        usesGuestArray(thread->glEdgeFlagPointer);
    if (!guestArray) {
        GL_FUNC(pglArrayElement)(i);
        return;
    }

    if (usesGuestArray(thread->glColorPointer))
        emitColorArrayElement(cpu, &thread->glColorPointer, i);
    if (usesGuestArray(thread->glIndexPointer))
        emitIndexArrayElement(cpu, &thread->glIndexPointer, i);
    if (usesGuestArray(thread->glNormalPointer))
        emitNormalArrayElement(cpu, &thread->glNormalPointer, i);
    for (auto& vp : thread->glTexCoordPointersByTexture) {
        if (usesGuestArray(*vp.value)) {
            emitTexCoordArrayElement(cpu, vp.key, vp.value.get(), i);
        }
    }
    if (usesGuestArray(thread->glEdgeFlagPointer))
        emitEdgeFlagArrayElement(cpu, &thread->glEdgeFlagPointer, i);
    if (usesGuestArray(thread->glVertextPointer))
        emitVertexArrayElement(cpu, &thread->glVertextPointer, i);
}

static OpenGLVetexPointer* getVertexAttribPointerNV(CPU* cpu, GLuint index) {
    OpenGLVetexPointerPtr found = cpu->thread->glVertexAttribPointerNVByIndex.get(index);
    if (!found) {
        found = std::make_shared<OpenGLVetexPointer>();
        cpu->thread->glVertexAttribPointerNVByIndex.set(index, found);
    }
    return found.get();
}

GLvoid* marshalVetextPointer(CPU* cpu, GLuint index, GLboolean normalized, GLint size, GLenum type, GLsizei stride, U32 ptr, bool isVertexAttrib) {
    clearInterleavedArrayReplay(cpu);
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
        p->isVertexAttrib = isVertexAttrib;
        updateVertexPointer(cpu, p, 0);
        return ptr ? p->marshal : 0;
    }
}

GLvoid* marshalVertexAttribPointerNV(CPU* cpu, GLuint index, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    OpenGLVetexPointer* p = getVertexAttribPointerNV(cpu, index);

    p->isArrayBuffer = ARRAY_BUFFER();
    if (p->isArrayBuffer) {
        p->refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        p->size = size;
        p->type = type;
        p->stride = stride;
        p->ptr = ptr;
        p->refreshEachCall = 1;
        updateVertexPointer(cpu, p, 0);
        return ptr ? p->marshal : 0;
    }
}

U32 marshalBackVertexAttribPointer(CPU* cpu, GLuint index, GLvoid* ptr, bool nv) {
    if (!ptr) {
        return 0;
    }

    OpenGLVetexPointer* p = nullptr;
    if (nv) {
        OpenGLVetexPointerPtr found = cpu->thread->glVertexAttribPointerNVByIndex.get(index);
        p = found.get();
    } else if (index == 0) {
        p = &cpu->thread->glVertextPointer;
    } else {
        OpenGLVetexPointerPtr found = cpu->thread->glVertextPointersByIndex.get(index);
        p = found.get();
    }

    if (p) {
        if (p->isArrayBuffer) {
            return (U32)(uintptr_t)ptr;
        }
        if (p->marshal == ptr) {
            return p->ptr;
        }
    }
    return (U32)(uintptr_t)ptr;
}

GLvoid* marshalNormalPointer(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    clearInterleavedArrayReplay(cpu);
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
    clearInterleavedArrayReplay(cpu);
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
    clearInterleavedArrayReplay(cpu);
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

GLvoid* marshalTangentPointerEXT(CPU* cpu, GLenum type, GLsizei stride, U32 ptr) {
    clearInterleavedArrayReplay(cpu);
    cpu->thread->glTangentPointerEXT.isArrayBuffer = ARRAY_BUFFER();
    if (cpu->thread->glTangentPointerEXT.isArrayBuffer) {
        cpu->thread->glTangentPointerEXT.refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        cpu->thread->glTangentPointerEXT.size = 3;
        cpu->thread->glTangentPointerEXT.type = type;
        cpu->thread->glTangentPointerEXT.stride = stride;
        cpu->thread->glTangentPointerEXT.ptr = ptr;
        cpu->thread->glTangentPointerEXT.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glTangentPointerEXT, 0);
        return ptr ? cpu->thread->glTangentPointerEXT.marshal : 0;
    }
}

GLvoid* marshalColorPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    clearInterleavedArrayReplay(cpu);
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
    clearInterleavedArrayReplay(cpu);
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
    clearInterleavedArrayReplay(cpu);
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
    clearInterleavedArrayReplay(cpu);
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
    bool validType = type == GL_SHORT || type == GL_INT ||
        type == GL_FLOAT || type == GL_DOUBLE;
    if (size < 1 || size > 4 || !validType || stride < 0) {
        return (GLvoid*)(uintptr_t)ptr;
    }
    clearInterleavedArrayReplay(cpu);
    OpenGLVetexPointer* p = getCurrentTexCoordPointer(cpu);
    p->size = size;
    p->type = type;
    p->stride = stride;
    p->ptr = ptr;
    p->isArrayBuffer = ARRAY_BUFFER();
    if (p->isArrayBuffer) {
        p->refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    }
    p->refreshEachCall = 1;
    updateVertexPointer(cpu, p, 0);
    return ptr ? p->marshal : 0;
}

static GLvoid* marshalClientPointer(CPU* cpu, OpenGLVetexPointer* p, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    clearInterleavedArrayReplay(cpu);
    p->isArrayBuffer = ARRAY_BUFFER();
    if (p->isArrayBuffer) {
        p->refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        p->size = size;
        p->type = type;
        p->stride = stride;
        p->ptr = ptr;
        p->refreshEachCall = 1;
        updateVertexPointer(cpu, p, 0);
        return ptr ? p->marshal : 0;
    }
}

GLvoid* marshalMultiTexCoordPointerEXT(CPU* cpu, GLenum texunit, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    return marshalClientPointer(cpu, getClientPointerByKey(cpu->thread->glMultiTexCoordPointerEXTByTexunit, texunit), size, type, stride, ptr);
}

GLvoid* marshalMultiTexCoordPointerSGIS(CPU* cpu, GLenum target, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    return marshalClientPointer(cpu, getClientPointerByKey(cpu->thread->glMultiTexCoordPointerSGISByTarget, target), size, type, stride, ptr);
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

static GLvoid* marshalElementPointer(CPU* cpu, OpenGLVetexPointer* p, GLenum type, U32 ptr) {
    p->isArrayBuffer = ELEMENT_ARRAY_BUFFER();
    if (p->isArrayBuffer) {
        p->refreshEachCall = 0;
        return (GLvoid*)(uintptr_t)ptr;
    } else {
        p->size = 1;
        p->type = type;
        p->stride = 0;
        p->ptr = ptr;
        p->refreshEachCall = 1;
        updateVertexPointer(cpu, p, 0);
        return ptr ? p->marshal : 0;
    }
}

GLvoid* marshalElementPointerAPPLE(CPU* cpu, GLenum type, U32 ptr) {
    return marshalElementPointer(cpu, &cpu->thread->glElementPointerAPPLE, type, ptr);
}

GLvoid* marshalElementPointerATI(CPU* cpu, GLenum type, U32 ptr) {
    return marshalElementPointer(cpu, &cpu->thread->glElementPointerATI, type, ptr);
}

GLvoid* marshalVariantPointerEXT(CPU* cpu, GLuint id, GLenum type, GLuint stride, U32 ptr) {
    return marshalClientPointer(cpu, getClientPointerByKey(cpu->thread->glVariantPointerEXTById, id), 1, type, stride, ptr);
}

GLvoid* marshalMatrixIndexPointerARB(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    return marshalClientPointer(cpu, &cpu->thread->glMatrixIndexPointerARB, size, type, stride, ptr);
}

GLvoid* marshalVertexWeightPointerEXT(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    return marshalClientPointer(cpu, &cpu->thread->glVertexWeightPointerEXT, size, type, stride, ptr);
}

GLvoid* marshalWeightPointerARB(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr) {
    return marshalClientPointer(cpu, &cpu->thread->glWeightPointerARB, size, type, stride, ptr);
}

void updateElementPointerAPPLE(CPU* cpu, U32 count) {
    if (cpu->thread->glElementPointerAPPLE.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glElementPointerAPPLE, count) && ext_glElementPointerAPPLE) {
            ext_glElementPointerAPPLE(cpu->thread->glElementPointerAPPLE.type, cpu->thread->glElementPointerAPPLE.marshal);
        }
    }
}

void updateElementPointerATI(CPU* cpu, U32 count) {
    if (cpu->thread->glElementPointerATI.refreshEachCall) {
        if (updateVertexPointer(cpu, &cpu->thread->glElementPointerATI, count) && ext_glElementPointerATI) {
            ext_glElementPointerATI(cpu->thread->glElementPointerATI.type, cpu->thread->glElementPointerATI.marshal);
        }
    }
}

struct InterleavedArrayComponent {
    U32 size = 0;
    GLenum type = 0;
    U32 offset = 0;
};

struct InterleavedArrayFormat {
    InterleavedArrayComponent vertex;
    InterleavedArrayComponent color;
    InterleavedArrayComponent normal;
    InterleavedArrayComponent texture;
};

static bool decodeInterleavedArrayFormat(GLenum format, InterleavedArrayFormat& decoded) {
    decoded = {};
    switch (format) {
    case GL_V2F:
        decoded.vertex = { 2, GL_FLOAT, 0 };
        return true;
    case GL_V3F:
        decoded.vertex = { 3, GL_FLOAT, 0 };
        return true;
    case GL_C4UB_V2F:
        decoded.color = { 4, GL_UNSIGNED_BYTE, 0 };
        decoded.vertex = { 2, GL_FLOAT, 4 };
        return true;
    case GL_C4UB_V3F:
        decoded.color = { 4, GL_UNSIGNED_BYTE, 0 };
        decoded.vertex = { 3, GL_FLOAT, 4 };
        return true;
    case GL_C3F_V3F:
        decoded.color = { 3, GL_FLOAT, 0 };
        decoded.vertex = { 3, GL_FLOAT, 12 };
        return true;
    case GL_N3F_V3F:
        decoded.normal = { 3, GL_FLOAT, 0 };
        decoded.vertex = { 3, GL_FLOAT, 12 };
        return true;
    case GL_C4F_N3F_V3F:
        decoded.color = { 4, GL_FLOAT, 0 };
        decoded.normal = { 3, GL_FLOAT, 16 };
        decoded.vertex = { 3, GL_FLOAT, 28 };
        return true;
    case GL_T2F_V3F:
        decoded.texture = { 2, GL_FLOAT, 0 };
        decoded.vertex = { 3, GL_FLOAT, 8 };
        return true;
    case GL_T4F_V4F:
        decoded.texture = { 4, GL_FLOAT, 0 };
        decoded.vertex = { 4, GL_FLOAT, 16 };
        return true;
    case GL_T2F_C4UB_V3F:
        decoded.texture = { 2, GL_FLOAT, 0 };
        decoded.color = { 4, GL_UNSIGNED_BYTE, 8 };
        decoded.vertex = { 3, GL_FLOAT, 12 };
        return true;
    case GL_T2F_C3F_V3F:
        decoded.texture = { 2, GL_FLOAT, 0 };
        decoded.color = { 3, GL_FLOAT, 8 };
        decoded.vertex = { 3, GL_FLOAT, 20 };
        return true;
    case GL_T2F_N3F_V3F:
        decoded.texture = { 2, GL_FLOAT, 0 };
        decoded.normal = { 3, GL_FLOAT, 8 };
        decoded.vertex = { 3, GL_FLOAT, 20 };
        return true;
    case GL_T2F_C4F_N3F_V3F:
        decoded.texture = { 2, GL_FLOAT, 0 };
        decoded.color = { 4, GL_FLOAT, 8 };
        decoded.normal = { 3, GL_FLOAT, 24 };
        decoded.vertex = { 3, GL_FLOAT, 36 };
        return true;
    case GL_T4F_C4F_N3F_V4F:
        decoded.texture = { 4, GL_FLOAT, 0 };
        decoded.color = { 4, GL_FLOAT, 16 };
        decoded.normal = { 3, GL_FLOAT, 32 };
        decoded.vertex = { 4, GL_FLOAT, 44 };
        return true;
    default:
        return false;
    }
}

static void syncInterleavedArrayComponent(OpenGLVetexPointer& pointer,
    const InterleavedArrayComponent& component, U32 stride, U32 ptr,
    bool isArrayBuffer) {
    pointer.enabled = component.size != 0;
    pointer.refreshEachCall = 0;
    if (pointer.enabled) {
        pointer.size = component.size;
        pointer.type = component.type;
        pointer.stride = stride;
        pointer.ptr = ptr + component.offset;
        pointer.isArrayBuffer = isArrayBuffer;
    }
}

const void* marshalInterleavedPointer(CPU* cpu, GLenum format, GLsizei stride, U32 ptr) {
    if (stride < 0) {
        return (const void*)(uintptr_t)ptr;
    }
    InterleavedArrayFormat decoded;
    if (!decodeInterleavedArrayFormat(format, decoded)) {
        return (const void*)(uintptr_t)ptr;
    }

    cpu->thread->glIndexPointer.enabled = false;
    cpu->thread->glIndexPointer.refreshEachCall = 0;
    cpu->thread->glEdgeFlagPointer.enabled = false;
    cpu->thread->glEdgeFlagPointer.refreshEachCall = 0;
    OpenGLVetexPointer* texCoordPointer = getCurrentTexCoordPointer(cpu);
    bool isArrayBuffer = ARRAY_BUFFER();
    U32 effectiveStride = stride ? stride : getDataSize(format);
    cpu->thread->glInterleavedArrayTexture = cpu->thread->glClientActiveTexture;
    syncInterleavedArrayComponent(cpu->thread->glVertextPointer,
        decoded.vertex, effectiveStride, ptr, isArrayBuffer);
    syncInterleavedArrayComponent(cpu->thread->glColorPointer,
        decoded.color, effectiveStride, ptr, isArrayBuffer);
    syncInterleavedArrayComponent(cpu->thread->glNormalPointer,
        decoded.normal, effectiveStride, ptr, isArrayBuffer);
    syncInterleavedArrayComponent(*texCoordPointer,
        decoded.texture, effectiveStride, ptr, isArrayBuffer);
    cpu->thread->glInterleavedArray.size = 1;
    cpu->thread->glInterleavedArray.type = format;
    cpu->thread->glInterleavedArray.stride = stride;
    cpu->thread->glInterleavedArray.ptr = ptr;
    cpu->thread->glInterleavedArray.isArrayBuffer = isArrayBuffer;
    if (isArrayBuffer) {
        cpu->thread->glInterleavedArray.refreshEachCall = 0;
        return (const GLboolean*)(uintptr_t)ptr;
    }
    else {
        cpu->thread->glInterleavedArray.refreshEachCall = 1;
        updateVertexPointer(cpu, &cpu->thread->glInterleavedArray, 0);
        return cpu->thread->glInterleavedArray.marshal;
    }
}

#endif
