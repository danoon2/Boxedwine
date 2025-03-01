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

#ifdef BOXEDWINE_OPENGL
#include GLH
#include "knativesystem.h"
#include "glcommon.h"

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)
#undef GL_FUNCTION_FMT
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG)
#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)
#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) gl##func##_func ext_gl##func;

#include "glfunctions.h"

#include "glMarshal.h"

static BString glExt;

float fARG(CPU* cpu, U32 arg) {
    struct int2Float i;
    i.i = arg;
    return i.f;
}

double dARG(CPU* cpu, int address) {
    struct long2Double i;
    i.l = cpu->memory->readq(address);
    return i.d;
}

#ifndef DISABLE_GL_EXTENSIONS
static const char* extentions[] = {
#include "glfunctions_ext_def.h"
};
#endif

// const GLubyte *glGetStringi(GLenum name, GLuint index);
void glcommon_glGetString(CPU* cpu);
void glcommon_glGetStringi(CPU* cpu) { 
    GLenum pname = ARG1;
    if (pname == GL_EXTENSIONS) {
        GLuint index = ARG2;
        if (!cpu->thread->process->glStringsiExtensions) {
            glcommon_glGetString(cpu);
        }
        if (index < cpu->thread->process->glStringsiExtensionsOffset.size()) {
            EAX = cpu->thread->process->glStringsiExtensions + cpu->thread->process->glStringsiExtensionsOffset[index];
        } else {
            EAX = 0;
        }
    } else {
        glcommon_glGetString(cpu);
    }
}

extern int sdlScaleX;
extern int sdlScaleY;

void glcommon_glViewport(CPU* cpu) {
    GLint x = ARG1;
    GLint y = ARG2;
    GLsizei width = ARG3;
    GLsizei height = ARG4;
    GL_FUNC(pglViewport)(x, y, width, height);
}

static GLfloat* feedbackBuffer;
static GLsizei feedbackBufferSize;
static GLsizei feedbackSize;
static U32 feedbackBufferAddress;

void glcommon_glFeedbackBuffer(CPU* cpu) {
    GLsizei size = ARG1;
    GLenum type = ARG2;

    if (size > feedbackBufferSize) {
        if (feedbackBuffer) {
            delete[] feedbackBuffer;
        }
        feedbackBuffer = new GLfloat[size];
        feedbackBufferSize = size;
    }
    cpu->memory->memcpy(feedbackBuffer, ARG3, size * sizeof(GLfloat));
    GL_FUNC(pglFeedbackBuffer)(size, type, feedbackBuffer);
    feedbackBufferAddress = ARG3;
    feedbackSize = size;
}

static GLuint* selectBuffer;
static GLsizei selectBufferSize;
static GLsizei selectSize;
static U32 selectBufferAddress;

void glcommon_glSelectBuffer(CPU* cpu) {
    GLsizei size = ARG1;

    if (size > selectBufferSize) {
        if (selectBuffer) {
            delete[] selectBuffer;
        }
        selectBuffer = new GLuint[size];
        selectBufferSize = size;
    }
    cpu->memory->memcpy(selectBuffer, ARG2, size*sizeof(GLuint));
    GL_FUNC(pglSelectBuffer)(size, selectBuffer);
    selectBufferAddress = ARG2;
    selectSize = size;
}

// changed this to an invalid value to fix motorhead under windows.  I will need to reevaluate why it was necessary for ma
static const char* addedExt[] = { "WGL_ARB_create_context_Invalid" };

void glcommon_glGetIntegerv(CPU* cpu) {
    GLenum pname = ARG1;
    if (pname == GL_NUM_EXTENSIONS) {
        cpu->memory->writed(ARG2, cpu->thread->process->numberOfExtensions);
    } else {
        MarshalReadWrite<GLint> buffer(cpu, ARG2, getSize(ARG1));
        GL_FUNC(pglGetIntegerv)(pname, buffer.getPtr());
    }
}

void glcommon_glRenderMode(CPU* cpu) {
    GLenum mode = ARG1;
    GLint current;
    GL_FUNC(pglGetIntegerv)(GL_RENDER_MODE, &current);
    EAX = GL_FUNC(pglRenderMode)(mode);
    // could be -1
    if (mode == GL_RENDER) {
        if (current == GL_FEEDBACK && EAX < (U32)feedbackBufferSize) {
            marshalBackArray<GLfloat>(cpu, feedbackBuffer, feedbackBufferAddress, feedbackSize);
        } if (current == GL_SELECT && EAX < (U32)selectBufferSize) {
            marshalBackArray<GLuint>(cpu, selectBuffer, selectBufferAddress, selectSize);
        }
    }
}

void glcommon_glDisableClientState(CPU* cpu) {
    GLenum cap = ARG1;
    GL_FUNC(pglDisableClientState)(cap);
    if (cap == GL_COLOR_ARRAY) {
        cpu->thread->glColorPointer.refreshEachCall = 0;
    } else if (cap == GL_EDGE_FLAG_ARRAY) {
        cpu->thread->glEdgeFlagPointer.refreshEachCall = 0;
    } else if (cap == GL_FOG_COORD_ARRAY) {
        cpu->thread->glFogPointer.refreshEachCall = 0;
    } else if (cap == GL_INDEX_ARRAY) {
        cpu->thread->glIndexPointer.refreshEachCall = 0;
    } else if (cap == GL_NORMAL_ARRAY) {
        cpu->thread->glNormalPointer.refreshEachCall = 0;
    } else if (cap == GL_SECONDARY_COLOR_ARRAY) {
        cpu->thread->glSecondaryColorPointer.refreshEachCall = 0;
    } else if (cap == GL_TEXTURE_COORD_ARRAY) {
        cpu->thread->glTexCoordPointer.refreshEachCall = 0;
    } else if (cap == GL_VERTEX_ARRAY) {
        cpu->thread->glVertextPointer.refreshEachCall = 0;
    }
}

template <typename T>
U32 getLargestValue(T* p, U32 count) {
    U32 result = 0;
    for (U32 i = 0; i < count; i++) {
        if (p[i] > result) {
            result = p[i];
        }
    }
    return result;
}

U32 getLargestIndexInType(GLenum type, GLsizei count, GLvoid* p) {
    switch (type) {
    case GL_UNSIGNED_BYTE: return getLargestValue<GLubyte>((GLubyte*)p, count);
    case GL_UNSIGNED_SHORT: return getLargestValue<GLushort>((GLushort*)p, count);
    case GL_UNSIGNED_INT: return getLargestValue<GLuint>((GLuint*)p, count);
    default:
        kpanic_fmt("marshalType unknown type: %d", type);
        return 0;
    }
}

void glcommon_glDrawElements(CPU* cpu) {
    GLenum mode = ARG1;
    GLsizei count = ARG2;
    GLenum type = ARG3;
    U32 indices = ARG4;
    GLvoid* p = nullptr;

    if (ELEMENT_ARRAY_BUFFER()) {
        p = (GLvoid*)pARG4;
        // :TODO: is this correct to use this count?
        updateVertexPointers(cpu, count);
        GL_LOG("glDrawElements mode=%x count=%d type=%x", mode, count, type);
    } else {
        p = marshalType(cpu, type, count, indices);
        U32 lastIndex = getLargestIndexInType(type, count, p);
        updateVertexPointers(cpu, lastIndex+1);
        GL_LOG("glDrawElements mode=%x count=%d type=%x lastIndex=%d", mode, count, type, lastIndex);
    }    
    GL_FUNC(pglDrawElements)(mode, count, type, p);    
}

void printOpenGLInfo() {
    klog_fmt("GL Vendor: %s", (const char*)GL_FUNC(pglGetString)(GL_VENDOR));
    klog_fmt("GL Renderer: %s", (const char*)GL_FUNC(pglGetString)(GL_RENDERER));
    klog_fmt("GL Version: %s", (const char*)GL_FUNC(pglGetString)(GL_VERSION));
}

// GLAPI const GLubyte* APIENTRY glGetString( GLenum name ) {
void glcommon_glGetString(CPU* cpu) {
    U32 name = ARG1;
    const char* result = (const char*)GL_FUNC(pglGetString)(name);
    KProcess* process = cpu->thread->process.get();

    if (name == GL_EXTENSIONS) {
#ifdef DISABLE_GL_EXTENSIONS
        result = "GL_EXT_texture3D";
#else
        static char* ext;
        if (!ext) {
            U32 len = (U32)strlen(result)+1;
            for (U32 i = 0; i < sizeof(addedExt) / sizeof(*addedExt); i++) {
                len += (U32)strlen(addedExt[i])+1;
            }
            ext = new char[len];
            memset(ext, 0, len);
        }
        if (ext[0]==0) {
            std::vector<BString> hardwareExt;
            std::vector<BString> supportedExt;
            B(result).split(' ', hardwareExt);
            for (U32 i=0;i<sizeof(extentions)/sizeof(char*);i++) {
                supportedExt.push_back(BString::copy(extentions[i]));
            }
            process->numberOfExtensions = 0;
            for (U32 i=0;i<hardwareExt.size();i++) {
                if (std::find(supportedExt.begin(), supportedExt.end(), hardwareExt[i]) == supportedExt.end()) {
                    continue;
                }

                if (!glExt.length() || strstr(glExt.c_str(), hardwareExt[i].c_str())) {
                    if (ext[0]!=0)
                        strcat(ext, " ");
                    process->numberOfExtensions++;
                    strcat(ext, hardwareExt[i].c_str());
                }
            }
            for (U32 i = 0; i < sizeof(addedExt) / sizeof(*addedExt); i++) {
                if (ext[0] != 0)
                    strcat(ext, " ");
                process->numberOfExtensions++;
                strcat(ext, addedExt[i]);
            }

        }
        result = ext;
#endif
        GL_LOG("glGetString GLenum name=GL_EXTENSIONS ret=%s", result);
    } else {
    }

    if (name == GL_EXTENSIONS && !cpu->thread->process->glStringsiExtensions) {
        U32 len = (U32)strlen(result)+1;
        U32 address = cpu->memory->mmap(cpu->thread, 0, len, K_PROT_WRITE|K_PROT_READ, K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        cpu->memory->memcpy(address, (void*)result, len);
        cpu->thread->process->glStringsiExtensions = address;
        cpu->thread->process->glStringsiExtensionsOffset.push_back(0);

        for (int i = 0; i < (int)len; i++) {
            char c = result[i];
            if (c == ' ') {
                cpu->memory->writeb(address+i, 0);
                cpu->thread->process->glStringsiExtensionsOffset.push_back(i + 1);
            }
        }
    }
    U32 previousAddress = 0;
    if (result && process->glStrings.get(name, previousAddress)) {
        if (process->memory->memcmp(previousAddress, result, (U32)strlen(result)+1) == 0) {
            EAX = previousAddress;
            return;
        }
    }
    if (!result) {
        EAX = 0;
    } else {
        U32 len = (U32)strlen(result) + 1;
        U32 address = cpu->memory->mmap(cpu->thread, 0, len, K_PROT_WRITE | K_PROT_READ, K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        cpu->memory->memcpy(address, (void*)result, len);

        process->glStrings.set(name, address);
        EAX = address;
    }
}

// GLAPI void APIENTRY glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ) {
void glcommon_glGetTexImage(CPU* cpu) {
    GLenum target = ARG1;
    GLint level = ARG2;
    GLsizei width=0;
    GLsizei height=0;
    GLenum format = ARG3;
    GLenum type = ARG4;

    GL_FUNC(pglGetTexLevelParameteriv)(target, level, GL_TEXTURE_WIDTH, &width);
    GL_FUNC(pglGetTexLevelParameteriv)(target, level, GL_TEXTURE_HEIGHT, &height);
    MarshalReadWritePackedPixels pixels(cpu, target == GL_TEXTURE_3D, width, height, 1, format, type, ARG5);
    GL_FUNC(pglGetTexImage)(target, level, format, type, pixels.getPtr());
}

U32 isMap2(GLenum target) {
    switch (target) {
    case GL_MAP2_INDEX:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_VERTEX_3: 
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_VERTEX_4:
    case GL_MAP2_COLOR_4:
    case GL_MAP2_TEXTURE_COORD_4:
        return 1;
    }
    return 2;
}

// GLAPI void APIENTRY glGetMapdv( GLenum target, GLenum query, GLdouble *v ) {
void glcommon_glGetMapdv(CPU* cpu) {
    GLenum target = ARG1;
    GLenum query = ARG2;
    
    GL_LOG("glGetMapdv GLenum target=%d, GLenum query=%d, GLdouble *v=%.08x", ARG1, ARG2, ARG3);

    switch (query) {
    case GL_COEFF: {
        GLint order[2] = {};
        int count=0;

        GL_FUNC(pglGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        MarshalReadWrite<GLdouble> buffer(cpu, ARG3, count);
        GL_FUNC(pglGetMapdv)(target, query, buffer.getPtr());
        break;
    }
    case GL_ORDER: {
        MarshalReadWrite<GLdouble> buffer(cpu, ARG3, isMap2(target) ? 2 : 1);
        GL_FUNC(pglGetMapdv)(target, query, buffer.getPtr());
        break;
    }
    case GL_DOMAIN: {
        MarshalReadWrite<GLdouble> buffer(cpu, ARG3, isMap2(target) ? 4 : 2);
        GL_FUNC(pglGetMapdv)(target, query, buffer.getPtr());
        break;
    }
    default:
        kpanic_fmt("glGetMapdv unknown query: %d", query);
    }	
}

// GLAPI void APIENTRY glGetMapfv( GLenum target, GLenum query, GLfloat *v ) {
void glcommon_glGetMapfv(CPU* cpu) {
    GLenum target = ARG1;
    GLenum query = ARG2;
    
    GL_LOG("glGetMapfv GLenum target=%d, GLenum query=%d, GLfloat *v=%.08x", ARG1, ARG2, ARG3);
    switch (query) {
    case GL_COEFF: {
        GLint order[2] = {};
        int count = 0;

        GL_FUNC(pglGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        MarshalReadWrite<GLfloat> buffer(cpu, ARG3, count);
        GL_FUNC(pglGetMapfv)(target, query, buffer.getPtr());
        break;
    }
    case GL_ORDER: {
        MarshalReadWrite<GLfloat> buffer(cpu, ARG3, isMap2(target) ? 2 : 1);
        GL_FUNC(pglGetMapfv)(target, query, buffer.getPtr());
        break;
    }
    case GL_DOMAIN: {
        MarshalReadWrite<GLfloat> buffer(cpu, ARG3, isMap2(target) ? 4 : 2);
        GL_FUNC(pglGetMapfv)(target, query, buffer.getPtr());
        break;
    }
    default:
        kpanic_fmt("glGetMapfv unknown query: %d", query);
    }	
}

// GLAPI void APIENTRY glGetMapiv( GLenum target, GLenum query, GLint *v ) {
void glcommon_glGetMapiv(CPU* cpu) {
    GLenum target = ARG1;
    GLenum query = ARG2;
    
    GL_LOG("glGetMapiv GLenum target=%d, GLenum query=%d, GLint *v=%.08x", ARG1, ARG2, ARG3);
    switch (query) {
    case GL_COEFF: {
        GLint order[2] = {};
        int count=0;

        GL_FUNC(pglGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        MarshalReadWrite<GLint> buffer(cpu, ARG3, count);
        GL_FUNC(pglGetMapiv)(target, query, buffer.getPtr());
        break;
    }
    case GL_ORDER: {
        MarshalReadWrite<GLint> buffer(cpu, ARG3, isMap2(target) ? 2 : 1);
        GL_FUNC(pglGetMapiv)(target, query, buffer.getPtr());
        break;
    }
    case GL_DOMAIN: {
        MarshalReadWrite<GLint> buffer(cpu, ARG3, isMap2(target) ? 4 : 2);
        GL_FUNC(pglGetMapiv)(target, query, buffer.getPtr());
        break;
    }
    default:
        kpanic_fmt("glGetMapfv unknown query: %d", query);
    }	
}

void glcommon_glTexSubImage2D(CPU* cpu) {
    GLenum target = ARG1;
    GLint level = ARG2;
    GLint xoffset = ARG3;
    GLint yoffset = ARG4;
    GLsizei width = ARG5;
    GLsizei height = ARG6;
    GLenum format = ARG7;
    GLenum type = ARG8;
    const GLvoid* pixels = PIXEL_UNPACK_BUFFER() ? (GLvoid*)pARG9 : marshalPixels(cpu, target == GL_TEXTURE_3D, width, height, 1, format, type, ARG9, xoffset, yoffset, level);

    GL_LOG("glTexSubImage2D GLenum target=%x, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%x, GLenum type=%x, const GLvoid* pixels=%x", ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_FUNC(pglTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

// GLAPI void APIENTRY glGetPointerv( GLenum pname, GLvoid **params ) {
void glcommon_glGetPointerv(CPU* cpu) {
    GL_LOG("glGetPointerv GLenum pname=%d, GLvoid **params=%.08x", ARG1, ARG2);
    switch (ARG1) {
    case GL_COLOR_ARRAY_POINTER: cpu->memory->writed(cpu->memory->readd(ARG2), cpu->thread->glColorPointer.ptr); break;
    case GL_EDGE_FLAG_ARRAY_POINTER: cpu->memory->writed(cpu->memory->readd(ARG2), cpu->thread->glEdgeFlagPointer.ptr); break;
    case GL_INDEX_ARRAY_POINTER: cpu->memory->writed(cpu->memory->readd(ARG2), cpu->thread->glIndexPointer.ptr); break;
    case GL_NORMAL_ARRAY_POINTER: cpu->memory->writed(cpu->memory->readd(ARG2), cpu->thread->glNormalPointer.ptr); break;
    case GL_TEXTURE_COORD_ARRAY_POINTER: cpu->memory->writed(cpu->memory->readd(ARG2), cpu->thread->glTexCoordPointer.ptr); break;
    case GL_VERTEX_ARRAY_POINTER: cpu->memory->writed(cpu->memory->readd(ARG2), cpu->thread->glVertextPointer.ptr); break;
    default: cpu->memory->writed(cpu->memory->readd(ARG2), 0);
    }
}

// GLAPI void APIENTRY glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ) {
void glcommon_glInterleavedArrays(CPU* cpu) {
    GLenum format = ARG1;
    GLsizei stride = ARG2;
    U32 address = ARG3;
    GL_FUNC(pglInterleavedArrays)(format, stride, marshalInterleavedPointer(cpu, format, stride, address));
}

// GLAPI void APIENTRY glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) {
void glcommon_glReadPixels(CPU* cpu) {
    GLsizei width = ARG3;
    GLsizei height = ARG4;
    GLenum format = ARG5;
    GLenum type = ARG6;

    GL_LOG("glReadPixels GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, GLvoid *pixels=%.08x", ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);

    MarshalReadWritePackedPixels pixels(cpu, 0, width, height, 1, format, type, ARG7);
    GL_FUNC(pglReadPixels)(ARG1, ARG2, width, height, format, type, pixels.getPtr());
}

void OPENGL_CALL_TYPE debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    klog_fmt("%s", message);
}

void glcommon_glSamplePass(CPU* cpu) {
    if (!ext_glSamplePass)
        kpanic("ext_glSamplePass is NULL");
    {
    GL_FUNC(ext_glSamplePass)(ARG1);
    GL_LOG ("SamplePass GLenum pass=%d", ARG1);
    }
}

void glcommon_glGetError(CPU* cpu) {
    if (cpu->thread->glLastError) {
        EAX = cpu->thread->glLastError;
        cpu->thread->glLastError = 0;
    } else {
        EAX = GL_FUNC(pglGetError)();
    }
}

#define GLX_USE_GL                        1
#define GLX_BUFFER_SIZE                   2
#define GLX_LEVEL                         3
#define GLX_RGBA                          4
#define GLX_DOUBLEBUFFER                  5
#define GLX_STEREO                        6
#define GLX_AUX_BUFFERS                   7
#define GLX_RED_SIZE                      8
#define GLX_GREEN_SIZE                    9
#define GLX_BLUE_SIZE                     10
#define GLX_ALPHA_SIZE                    11
#define GLX_DEPTH_SIZE                    12
#define GLX_STENCIL_SIZE                  13
#define GLX_ACCUM_RED_SIZE                14
#define GLX_ACCUM_GREEN_SIZE              15
#define GLX_ACCUM_BLUE_SIZE               16
#define GLX_ACCUM_ALPHA_SIZE              17

#define GLX_BAD_SCREEN                    1
#define GLX_BAD_ATTRIBUTE                 2
#define GLX_NO_EXTENSION                  3
#define GLX_BAD_VISUAL                    4
#define GLX_BAD_CONTEXT                   5
#define GLX_BAD_VALUE                     6
#define GLX_BAD_ENUM                      7

#include "../x11/x11.h"

/*
 * Tokens for glXChooseVisual and glXGetConfig:
 */
#define GLX_USE_GL		1
#define GLX_BUFFER_SIZE		2
#define GLX_LEVEL		3
#define GLX_RGBA		4
#define GLX_DOUBLEBUFFER	5
#define GLX_STEREO		6
#define GLX_AUX_BUFFERS		7
#define GLX_RED_SIZE		8
#define GLX_GREEN_SIZE		9
#define GLX_BLUE_SIZE		10
#define GLX_ALPHA_SIZE		11
#define GLX_DEPTH_SIZE		12
#define GLX_STENCIL_SIZE	13
#define GLX_ACCUM_RED_SIZE	14
#define GLX_ACCUM_GREEN_SIZE	15
#define GLX_ACCUM_BLUE_SIZE	16
#define GLX_ACCUM_ALPHA_SIZE	17

#define GLX_DONT_CARE                     0xFFFFFFFF

 /*
  * Error codes returned by glXGetConfig:
  */
#define GLX_BAD_SCREEN		1
#define GLX_BAD_ATTRIBUTE	2
#define GLX_NO_EXTENSION	3
#define GLX_BAD_VISUAL		4
#define GLX_BAD_CONTEXT		5
#define GLX_BAD_VALUE       	6
#define GLX_BAD_ENUM		7


  /*
   * GLX 1.1 and later:
   */
#define GLX_VENDOR		1
#define GLX_VERSION		2
#define GLX_EXTENSIONS 		3


   /*
    * GLX 1.3 and later:
    */
#define GLX_CONFIG_CAVEAT		0x20
#define GLX_DONT_CARE			0xFFFFFFFF
#define GLX_X_VISUAL_TYPE		0x22
#define GLX_TRANSPARENT_TYPE		0x23
#define GLX_TRANSPARENT_INDEX_VALUE	0x24
#define GLX_TRANSPARENT_RED_VALUE	0x25
#define GLX_TRANSPARENT_GREEN_VALUE	0x26
#define GLX_TRANSPARENT_BLUE_VALUE	0x27
#define GLX_TRANSPARENT_ALPHA_VALUE	0x28
#define GLX_WINDOW_BIT			0x00000001
#define GLX_PIXMAP_BIT			0x00000002
#define GLX_PBUFFER_BIT			0x00000004
#define GLX_AUX_BUFFERS_BIT		0x00000010
#define GLX_FRONT_LEFT_BUFFER_BIT	0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT	0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT	0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT	0x00000008
#define GLX_DEPTH_BUFFER_BIT		0x00000020
#define GLX_STENCIL_BUFFER_BIT		0x00000040
#define GLX_ACCUM_BUFFER_BIT		0x00000080
#define GLX_NONE			0x8000
#define GLX_SLOW_CONFIG			0x8001
#define GLX_TRUE_COLOR			0x8002
#define GLX_DIRECT_COLOR		0x8003
#define GLX_PSEUDO_COLOR		0x8004
#define GLX_STATIC_COLOR		0x8005
#define GLX_GRAY_SCALE			0x8006
#define GLX_STATIC_GRAY			0x8007
#define GLX_TRANSPARENT_RGB		0x8008
#define GLX_TRANSPARENT_INDEX		0x8009
#define GLX_VISUAL_ID			0x800B
#define GLX_SCREEN			0x800C
#define GLX_NON_CONFORMANT_CONFIG	0x800D
#define GLX_DRAWABLE_TYPE		0x8010
#define GLX_RENDER_TYPE			0x8011
#define GLX_X_RENDERABLE		0x8012
#define GLX_FBCONFIG_ID			0x8013
#define GLX_RGBA_TYPE			0x8014
#define GLX_COLOR_INDEX_TYPE		0x8015
#define GLX_MAX_PBUFFER_WIDTH		0x8016
#define GLX_MAX_PBUFFER_HEIGHT		0x8017
#define GLX_MAX_PBUFFER_PIXELS		0x8018
#define GLX_PRESERVED_CONTENTS		0x801B
#define GLX_LARGEST_PBUFFER		0x801C
#define GLX_WIDTH			0x801D
#define GLX_HEIGHT			0x801E
#define GLX_EVENT_MASK			0x801F
#define GLX_DAMAGED			0x8020
#define GLX_SAVED			0x8021
#define GLX_WINDOW			0x8022
#define GLX_PBUFFER			0x8023
#define GLX_PBUFFER_HEIGHT              0x8040
#define GLX_PBUFFER_WIDTH               0x8041
#define GLX_RGBA_BIT			0x00000001
#define GLX_COLOR_INDEX_BIT		0x00000002
#define GLX_PBUFFER_CLOBBER_MASK	0x08000000

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_FLAGS_ARB 0x2094
#define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
#define GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT  0x20B2
#define GLX_FLOAT_COMPONENTS_NV           0x20B0

    /*
     * GLX 1.4 and later:
     */
#define GLX_SAMPLE_BUFFERS              0x186a0 /*100000*/
#define GLX_SAMPLES                     0x186a1 /*100001*/

// XVisualInfo* pglXChooseVisual(Display* dpy, int screen, int* attribList) {
void gl_common_XChooseVisual(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    U32 bpp = KNativeSystem::getScreen()->screenBpp();
    U32 address = ARG3;
    U32 value;

    U32 colorType = PF_COLOR_TYPE_NOTSET;
    //U32 cRedBits = 0;
    //U32 cGreenBits = 0;
    //U32 cBlueBits = 0;
    //U32 cAlphaBits = 0;
    //U32 cAccumBits = 0;
    //U32 cDepthBits = 0;
    //U32 cStencilBits = 0;
    bool doubleBuffer = false;

    while ((value = cpu->memory->readd(address))) {
        if (value == GLX_RGBA) {
            colorType = K_PFD_TYPE_RGBA;
        } else if (value == GLX_DOUBLEBUFFER) {
            doubleBuffer = true;
        } 
        else {
            kpanic_fmt("gl_common_XChooseVisual unhandled attribute %d", value);
        }
        address += 4;
    }    

    CLXFBConfigPtr foundCfg = server->getFbConfig(1);
    S32 foundPoints = 0;

    server->iterateFbConfigs([doubleBuffer, colorType, bpp, &foundCfg, &foundPoints](const CLXFBConfigPtr& cfg) {
        S32 points = 0;

        if (cfg->glPixelFormat->pf.dwFlags & K_PFD_SWAP_COPY) {
            return true;
        }
        if (cfg->glPixelFormat->pf.cRedBits > 8) {
            return true;
        }        
        if (!(cfg->glPixelFormat->pf.dwFlags & K_PFD_DRAW_TO_WINDOW)) {
            return true;
        }
        if (cfg->depth != bpp) {
            return true;
        }
        if ((cfg->glPixelFormat->id & PIXEL_FORMAT_NATIVE_INDEX_MASK)) {
            points++;
        }
        bool isDoubleBuffer = (cfg->glPixelFormat->pf.dwFlags & K_PFD_DOUBLEBUFFER) != 0;
        if (isDoubleBuffer != doubleBuffer) {
            return true;
        }
        if (colorType != PF_COLOR_TYPE_NOTSET && colorType != cfg->glPixelFormat->pf.iPixelType) {
            return true;
        }        
        if (cfg->glPixelFormat->pf.cAuxBuffers) {
            points++;
        }
        if (cfg->glPixelFormat->pf.cAccumBits) {
            points++;
        }
        if (cfg->glPixelFormat->pf.cStencilBits) {
            points--;
        }
        if (cfg->glPixelFormat->pf.cAlphaBits) {
            points--;
        }
        if (points > foundPoints) {
            foundPoints = points;
            foundCfg = cfg;
        }
        return true;
        });

    U32 visualId = 0;
    
    if (foundCfg) {
        visualId = foundCfg->visualId;
    }
    if (!visualId) {
        EAX = 0;
        return;
    }
    U32 result = thread->process->alloc(thread, sizeof(XVisualInfo));
    bool found = false;
    Display::iterateVisuals(thread, ARG1, [&memory, result, &found, visualId](S32 screenIndex, U32 visualAddress, Depth* depth, Visual* visual) {
        if (visual->visualid == visualId) {
            XVisualInfo visualInfo;
            visualInfo.set(screenIndex, visualAddress, depth->depth, visual);
            visualInfo.write(memory, result);
            found = true;
            return false;
        }
        return true;
        });

    if (found) {
        EAX = result;
    } else {
        thread->process->free(result);
        EAX = 0;
    }
}

// GLXContext glXCreateContext(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct) {
void gl_common_XCreateContext(CPU* cpu) {    
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 shareList = ARG3;
    XVisualInfo info;
    info.read(memory, ARG2);
    U32 pixelFormatId = memory->readd(info.visual); // pixel format index is in visual.ext_data (first member)
    KOpenGLPtr gl = KNativeSystem::getOpenGL();

    EAX = gl->glCreateContext(thread, gl->getFormat(pixelFormatId), 0, 0, 0, 0, shareList);
}

// void glXDestroyContext(Display* dpy, GLXContext ctx) {
void gl_common_XDestroyContext(CPU* cpu) {      
    KNativeSystem::getOpenGL()->glDestroyContext(cpu->thread, ARG2);
}

// Bool glXMakeCurrent(Display* dpy, GLXDrawable drawable, GLXContext ctx) {
void gl_common_XMakeCurrent(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XDrawablePtr d = server->getDrawable(ARG2);
    U32 ctx = ARG3;

    if (ctx && !d) {
        EAX = False;
        return;
    }    
    EAX = KNativeSystem::getOpenGL()->glMakeCurrent(thread, d, ctx) ? True : False;
    thread->currentContext = ctx;
}

// void pglXCopyContext(Display* dpy, GLXContext src, GLXContext dst, unsigned long mask)
void gl_common_XCopyContext(CPU* cpu) {
    kpanic("glXCopyContext");
}

// Bool glXQueryVersion(Display* dpy, int* maj, int* min)
void gl_common_XQueryVersion(CPU* cpu) {
    kpanic("glXQueryVersion");
}

// Bool glXIsDirect(Display* dpy, GLXContext ctx)
void gl_common_XIsDirect(CPU* cpu) {
    EAX = True;
}

// GLXContext glXGetCurrentContext(void)
void gl_common_XGetCurrentContext(CPU* cpu) {
    kpanic("glXGetCurrentContext");
}

// GLXDrawable glXGetCurrentDrawable(void)
void gl_common_XGetCurrentDrawable(CPU* cpu) {
    kpanic("glXGetCurrentDrawable");
}

// const char* glXQueryExtensionsString(Display* dpy, int screen)
void gl_common_XQueryExtensionsString(CPU* cpu) {
    KThread* thread = cpu->thread;
    if (thread->process->glxStringExtensions) {
        EAX = thread->process->glxStringExtensions;
    }
    const char* s = "GLX_ARB_multisample GLX_SGIX_fbconfig GLX_ARB_create_context WGL_ARB_create_context_profile GLX_ARB_create_context_no_error";
    thread->process->glxStringExtensions = thread->process->alloc(thread, (U32)strlen(s) + 1);
    thread->memory->memcpy(thread->process->glxStringExtensions, s, (U32)strlen(s) + 1);
    EAX = thread->process->glxStringExtensions;
}

// const char* glXQueryServerString(Display* dpy, int screen, int name)
void gl_common_XQueryServerString(CPU* cpu) {
    kpanic("glXQueryServerString");
}

// const char* glXGetClientString(Display* dpy, int name)
void gl_common_XGetClientString(CPU* cpu) {
    kpanic("glXGetClientString");
}

// GLXFBConfig* glXChooseFBConfig(Display* dpy, int screen, const int* attribList, int* nitems)
void gl_common_XChooseFBConfig(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();

    //U32 count = 0;
    //U32 screen = ARG2;

    //U32 colorType = PF_COLOR_TYPE_NOTSET;
    U32 bufferSize = 0;
    U32 cAlphaBits = 0;
    U32 cDepthBits = 0;
    U32 cStencilBits = 0;
    U32 cAuxBits = 0;
    U32 doubleBuffer = GLX_DONT_CARE;
    U32 sampleBuffers = 0;
    U32 samples = 0;
    U32 drawableType = 0;
    U32 renderType = 0;
    U32 address = ARG3;
    U32 value;

    while ((value = cpu->memory->readd(address))) {
        if (value == GLX_BUFFER_SIZE) {
            bufferSize = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == GLX_ALPHA_SIZE) {
            cAlphaBits = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == GLX_DEPTH_SIZE) {
            cDepthBits = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == GLX_STENCIL_SIZE) {
            cStencilBits = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == GLX_AUX_BUFFERS) {
            cAuxBits = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == GLX_DOUBLEBUFFER) {
            doubleBuffer = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == 100000/*GLX_SAMPLE_BUFFERS_ARB*/) {
            sampleBuffers = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == 100001/*GLX_SAMPLES_ARB*/) {
            samples = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == 0x8010/*GLX_DRAWABLE_TYPE*/) {
            drawableType = cpu->memory->readd(address + 4);
            address += 4;
        } else if (value == 0x8011/*GLX_RENDER_TYPE*/) {
            renderType = cpu->memory->readd(address + 4);
            address += 4;
        } else {
            kpanic_fmt("gl_common_XChooseFBConfig unhandled attribute %d", value);
        }
        address += 4;
    }

    std::vector<CLXFBConfigPtr> foundCfgs;
    server->iterateFbConfigs([=, &foundCfgs](const CLXFBConfigPtr& cfg) {
        if (samples && samples > cfg->glPixelFormat->samples) {
            return true;
        }
        if (cAuxBits && cAuxBits > cfg->glPixelFormat->pf.cAuxBuffers) {
            return true;
        }
        if (doubleBuffer != GLX_DONT_CARE) {
            bool hasDoubleBuffer = (cfg->glPixelFormat->pf.dwFlags & K_PFD_DOUBLEBUFFER) != 0;
            bool wantsDoubleBuffer = doubleBuffer != 0;
            if (hasDoubleBuffer != wantsDoubleBuffer) {
                return true;
            }
        }
        if (cStencilBits && cStencilBits > cfg->glPixelFormat->pf.cStencilBits) {
            return true;
        }
        if (cDepthBits && cDepthBits > cfg->glPixelFormat->pf.cDepthBits) {
            return true;
        }
        if (cAlphaBits && cAlphaBits > cfg->glPixelFormat->pf.cAlphaBits) {
            return true;
        }
        if (bufferSize && bufferSize > cfg->glPixelFormat->depth) {
            return true;
        }
        foundCfgs.push_back(cfg);
        return true;
        });
    std::sort(foundCfgs.begin(), foundCfgs.end(), [=](const CLXFBConfigPtr& a, const CLXFBConfigPtr& b) {
        if (a->glPixelFormat->depth != b->glPixelFormat->depth) {
            return a->glPixelFormat->depth < b->glPixelFormat->depth;
        }
        if (doubleBuffer == GLX_DONT_CARE) {
            bool doubleA = (a->glPixelFormat->pf.dwFlags & K_PFD_DOUBLEBUFFER) != 0;
            bool doubleB = (b->glPixelFormat->pf.dwFlags & K_PFD_DOUBLEBUFFER) != 0;

            if (doubleA != doubleB) {
                // single buffer should come first
                return !doubleA;
            }
        }
        if (a->glPixelFormat->pf.cAuxBuffers != b->glPixelFormat->pf.cAuxBuffers) {
            return a->glPixelFormat->pf.cAuxBuffers < b->glPixelFormat->pf.cAuxBuffers;
        }
        if (a->glPixelFormat->pf.cDepthBits != b->glPixelFormat->pf.cDepthBits) {
            return a->glPixelFormat->pf.cDepthBits > b->glPixelFormat->pf.cDepthBits;
        }
        if (a->glPixelFormat->pf.cStencilBits != b->glPixelFormat->pf.cStencilBits) {
            return a->glPixelFormat->pf.cStencilBits < b->glPixelFormat->pf.cStencilBits;
        }
        return a->fbId < b->fbId;
        });
    memory->writed(ARG4, (U32)foundCfgs.size());

    U32 resultAddress = thread->process->alloc(thread, (U32)foundCfgs.size() * 4); // list of ids
    EAX = resultAddress;

    for (auto& cfg : foundCfgs) {
        memory->writed(resultAddress, cfg->fbId);
        resultAddress += 4;
    }
}

// int glXGetFBConfigAttrib(Display* dpy, GLXFBConfig config, int attribute, int* value)
void gl_common_XGetFBConfigAttrib(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    U32 attribute = ARG3;
    CLXFBConfigPtr cfg = server->getFbConfig(ARG2);

    if (!cfg) {
        EAX = BadValue;
        return;
    }
    switch (attribute) {
    case GLX_DOUBLEBUFFER:
        memory->writed(ARG4, (cfg->glPixelFormat->pf.dwFlags & K_PFD_DOUBLEBUFFER) ? True : False);
        break;
    case GLX_STEREO:
        memory->writed(ARG4, (cfg->glPixelFormat->pf.dwFlags & K_PFD_STEREO) ? True : False);
        break;
    case GLX_FBCONFIG_ID:
        memory->writed(ARG4, cfg->fbId);
        break;
    case GLX_VISUAL_ID:
        memory->writed(ARG4, cfg->visualId);
        break;
    case GLX_DRAWABLE_TYPE:
    {
        U32 result = 0;
        if (cfg->glPixelFormat->pf.dwFlags & K_PFD_DRAW_TO_WINDOW) {
            result |= GLX_WINDOW_BIT;
        }
        if (cfg->glPixelFormat->pf.dwFlags & K_PFD_DRAW_TO_BITMAP) {
            result |= GLX_PIXMAP_BIT;
        }
        if (cfg->glPixelFormat->pbuffer) {
            result |= GLX_PBUFFER_BIT;
        }
        memory->writed(ARG4, result);
        break;
    }
    case GLX_RENDER_TYPE:
        if (cfg->glPixelFormat->pf.iPixelType == K_PFD_TYPE_RGBA) {
            memory->writed(ARG4, GLX_RGBA_BIT);
        } else if (cfg->glPixelFormat->pf.iPixelType == K_PFD_TYPE_COLORINDEX) {
            memory->writed(ARG4, GLX_COLOR_INDEX_BIT);
        } else {
            kpanic_fmt("gl_common_XGetFBConfigAttrib unhandled GLX_RENDER_TYPE %x", cfg->glPixelFormat->pf.iPixelType);
        }
        break;
    case GLX_BUFFER_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cColorBits);
        break;
    case GLX_RED_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cRedBits);
        break;
    case GLX_GREEN_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cGreenBits);
        break;
    case GLX_BLUE_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cBlueBits);
        break;
    case GLX_ALPHA_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cAlphaBits);
        break;
    case GLX_ACCUM_RED_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cAccumRedBits);
        break;
    case GLX_ACCUM_BLUE_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cAccumBlueBits);
        break;
    case GLX_ACCUM_GREEN_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cAccumGreenBits);
        break;
    case GLX_ACCUM_ALPHA_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cAccumAlphaBits);
        break;
    case GLX_AUX_BUFFERS:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cAuxBuffers);
        break;
    case GLX_DEPTH_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cDepthBits);
        break;
    case GLX_STENCIL_SIZE:
        memory->writed(ARG4, cfg->glPixelFormat->pf.cStencilBits);
        break;
    case GLX_TRANSPARENT_TYPE:
        memory->writed(ARG4, GLX_NONE);
        break;
    case GLX_MAX_PBUFFER_PIXELS:
        memory->writed(ARG4, cfg->glPixelFormat->pbufferMaxPixels);
        break;
    case GLX_MAX_PBUFFER_WIDTH:
        memory->writed(ARG4, cfg->glPixelFormat->pbufferMaxWidth);
        break;
    case GLX_MAX_PBUFFER_HEIGHT:
        memory->writed(ARG4, cfg->glPixelFormat->pbufferMaxHeight);
        break;
    case GLX_SAMPLE_BUFFERS:
        memory->writed(ARG4, cfg->glPixelFormat->sampleBuffers);
        break;
    case GLX_SAMPLES:
        memory->writed(ARG4, cfg->glPixelFormat->samples);
        break;
    case GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT:
    case GLX_TRANSPARENT_INDEX_VALUE:
    case GLX_TRANSPARENT_RED_VALUE:
    case GLX_TRANSPARENT_GREEN_VALUE:
    case GLX_TRANSPARENT_BLUE_VALUE:
    case GLX_TRANSPARENT_ALPHA_VALUE:
    case GLX_FLOAT_COMPONENTS_NV:
        EAX = 1;
        return;
    default:
        kpanic_fmt("gl_common_XGetFBConfigAttrib attribute not handled: %x", attribute);
    }
    EAX = Success;

}

// GLXFBConfig* glXGetFBConfigs(Display* dpy, int screen, int* nelements)
void gl_common_XGetFBConfigs(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();

    U32 resultAddress = thread->process->alloc(thread, server->getFbConfigCount() * 4); // list of ids
    EAX = resultAddress;
    
    server->iterateFbConfigs([memory, &resultAddress](const CLXFBConfigPtr& cfg) {
        memory->writed(resultAddress, cfg->fbId);
        resultAddress += 4;
        return true;
        });
    memory->writed(ARG3, server->getFbConfigCount());
}

// XVisualInfo* glXGetVisualFromFBConfig(Display* dpy, GLXFBConfig config)
void gl_common_XGetVisualFromFBConfig(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    CLXFBConfigPtr cfg = server->getFbConfig(ARG2);
    if (!cfg) {
        EAX = 0;
        return;
    }
    U32 visualInfoAddress = thread->process->alloc(thread, sizeof(XVisualInfo));
    VisualPtr vis = server->getVisual(cfg->visualId);
    XVisualInfo info;
    U32 visAddress = 0;

    Display::iterateVisuals(thread, ARG1, [&cfg, &visAddress](S32 screenIndex, U32 visualAddress, Depth* depth, Visual* visual) {
        if (visual->visualid == cfg->visualId) {
            visAddress = visualAddress;
        }
        return true;
        });
    if (!visAddress) {
        EAX = 0;
        return;
    }
    info.set(0, visAddress, cfg->depth, vis.get());
    info.write(memory, visualInfoAddress);
    EAX = visualInfoAddress;    
}

// GLXPbuffer glXCreatePbuffer(Display* dpy, GLXFBConfig config, const int* attribList)
void gl_common_XCreatePbuffer(CPU* cpu) {
    kpanic("glXCreatePbuffer");
}

// void glXDestroyPbuffer(Display* dpy, GLXPbuffer pbuf)
void gl_common_XDestroyPbuffer(CPU* cpu) {
    kpanic("glXDestroyPbuffer");
}

// void glXQueryDrawable(Display* dpy, GLXDrawable draw, int attribute, unsigned int* value)
void gl_common_XQueryDrawable(CPU* cpu) {
    kpanic("glXQueryDrawable");
}

// GLXContext glXCreateNewContext(Display* dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct)
void gl_common_XCreateNewContext(CPU* cpu) {
    kpanic("glXCreateNewContext");
}

// Bool glXMakeContextCurrent(Display* dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx)
void gl_common_XMakeContextCurrent(CPU* cpu) {
    U32 draw = ARG2;
    U32 read = ARG3;
    U32 ctx = ARG4;
    if (read && read != draw) {
        kpanic("glXMakeContextCurrent");
    }

    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XDrawablePtr d = server->getDrawable(draw);

    if (ctx && !d) {
        EAX = False;
        return;
    }
    thread->currentContext = ctx;
    EAX = KNativeSystem::getOpenGL()->glMakeCurrent(thread, d, ctx) ? True : False;
}

// GLXPixmap glXCreatePixmap(Display* dpy, GLXFBConfig config, Pixmap pixmap, const int* attrib_list)
void gl_common_XCreatePixmap(CPU* cpu) {
    XServer* server = XServer::getServer();
    CLXFBConfigPtr cfg = server->getFbConfig(ARG2);
    XPixmapPtr pixmap = server->getPixmap(ARG3);
    if (!cfg || !pixmap) {
        EAX = 0;
        return;
    }
    if (ARG4) {
        // not set by wine
        kpanic("gl_common_XCreatePixmap attrib_list not implemented");
    }
    pixmap->isOpenGL = true;
    EAX = ARG3; // the return must be a drawable, if we need config, maybe store on the actual pixmap
}

// void glXDestroyPixmap(Display* dpy, GLXPixmap pixmap)
void gl_common_XDestroyPixmap(CPU* cpu) {
    // no memory was allocated in gl_common_XCreatePixmap
    XServer* server = XServer::getServer();
    XPixmapPtr pixmap = server->getPixmap(ARG2);
    if (pixmap) {
        pixmap->isOpenGL = false;
    }
}

// GLXWindow glXCreateWindow(Display* dpy, GLXFBConfig config, Window win, const int* attrib_list)
void gl_common_XCreateWindow(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    CLXFBConfigPtr cfg = server->getFbConfig(ARG2);
    XWindowPtr win = server->getWindow(ARG3);
    if (!cfg || !win) {
        EAX = 0;
        return;
    }
    if (ARG4) {
        // not set by wine
        kpanic("gl_common_XCreateWindow attrib_list not implemented");
    }
    win->isOpenGL = true;
    KNativeSystem::getOpenGL()->glCreateWindow(thread, win, cfg);    
    EAX = win->id;
}

// void glXDestroyWindow(Display* dpy, GLXWindow win)
void gl_common_XDestroyWindow(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XWindowPtr win = server->getWindow(ARG2);
    if (win) {
        KNativeSystem::getOpenGL()->glDestroyWindow(thread, win);
        win->isOpenGL = false;
    }
}

// GLXContext glXCreateContextAttribsARB(Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list)
void gl_common_XCreateContextAttribsARB(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    CLXFBConfigPtr cfg = server->getFbConfig(ARG2);
    U32 share_context = ARG3;
    //U32 direct = ARG4;
    U32 attribList = ARG5;
    U32 major = 0;
    U32 minor = 0;
    U32 flags = 0;
    U32 profile = 0;

    while (true) {
        U32 attrib = memory->readd(attribList);
        attribList += 4;
        if (!attrib) {
            break;
        }
        switch (attrib) {
        case GLX_CONTEXT_MAJOR_VERSION_ARB:
            major = memory->readd(attribList);
            attribList += 4;
            break;
        case GLX_CONTEXT_MINOR_VERSION_ARB:
            minor = memory->readd(attribList);
            attribList += 4;
            break;
        case GLX_CONTEXT_FLAGS_ARB:
            flags = memory->readd(attribList);
            attribList += 4;
            break;
        case GLX_CONTEXT_PROFILE_MASK_ARB:
            profile = memory->readd(attribList);
            attribList += 4;
            break;
        default:
            kpanic_fmt("gl_common_XCreateContextAttribsARB unhandled attribute %x", attrib);
        }
    }
    EAX = KNativeSystem::getOpenGL()->glCreateContext(thread, cfg->glPixelFormat, major, minor, profile, flags, share_context);
}

// void glXSwapIntervalEXT(Display* dpy, GLXDrawable drawable, int interval)
void gl_common_XSwapIntervalEXT(CPU* cpu) {
    kpanic("glXSwapIntervalEXT");
}

// void glXSwapBuffers(Display* dpy, GLXDrawable drawable) 
void gl_common_XSwapBuffers(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XDrawablePtr d = server->getDrawable(ARG2);
    if (!d) {
        EAX = BadDrawable;
        return;
    }
    KNativeSystem::getOpenGL()->glSwapBuffers(thread, d);
}

// create variables to hold standard opengl calls like glClear
#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) gl##func##_func pgl##func;

#undef GL_FUNCTION_FMT
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG) gl##func##_func pgl##func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) gl##func##_func pgl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS)

#include "glfunctions.h"

// create the functions that will make the OpenGL call, these will be assigned into gl_callback
#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) void glcommon_gl##func(CPU* cpu) { PRE GL_FUNC(pgl##func)ARGS; POST; GL_LOG_NO_FMT LOG;} 

#undef GL_FUNCTION_FMT
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG) void glcommon_gl##func(CPU* cpu) { PRE GL_FUNC(pgl##func)ARGS; POST; GL_LOG LOG;} 

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) void glcommon_gl##func(CPU* cpu);

#include "glfunctions.h"

static Int99Callback gl_callback[GL_FUNC_COUNT];

Int99Callback* int99Callback;
U32 int99CallbackSize;
U32 lastGlCallTime;

void gl_init(BString allowExtensions) {
    int99Callback=gl_callback;
    int99CallbackSize=GL_FUNC_COUNT;
    glExt = allowExtensions;

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) gl_callback[func] = glcommon_gl##func;

#undef GL_FUNCTION_FMT
#define GL_FUNCTION_FMT(func, RET, PARAMS, ARGS, PRE, POST, LOG) gl_callback[func] = glcommon_gl##func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) gl_callback[func] = glcommon_gl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) gl_callback[func] = glcommon_gl##func;

#include "glfunctions.h"      
    
    gl_callback[kXCreateContext] = gl_common_XCreateContext;
    gl_callback[kXDestroyContext] = gl_common_XDestroyContext;
    gl_callback[kXMakeCurrent] = gl_common_XMakeCurrent;

    gl_callback[kXChooseVisual] = gl_common_XChooseVisual;
    gl_callback[kXCopyContext] = gl_common_XCopyContext;
    gl_callback[kXQueryVersion] = gl_common_XQueryVersion;
    gl_callback[kXIsDirect] = gl_common_XIsDirect;
    gl_callback[kXGetCurrentContext] = gl_common_XGetCurrentContext;
    gl_callback[kXGetCurrentDrawable] = gl_common_XGetCurrentDrawable;
    gl_callback[kXQueryExtensionsString] = gl_common_XQueryExtensionsString;
    gl_callback[kXQueryServerString] = gl_common_XQueryServerString;
    gl_callback[kXGetClientString] = gl_common_XGetClientString;
    gl_callback[kXChooseFBConfig] = gl_common_XChooseFBConfig;
    gl_callback[kXGetFBConfigAttrib] = gl_common_XGetFBConfigAttrib;
    gl_callback[kXGetFBConfigs] = gl_common_XGetFBConfigs;
    gl_callback[kXGetVisualFromFBConfig] = gl_common_XGetVisualFromFBConfig;
    gl_callback[kXCreatePbuffer] = gl_common_XCreatePbuffer;
    gl_callback[kXDestroyPbuffer] = gl_common_XDestroyPbuffer;
    gl_callback[kXQueryDrawable] = gl_common_XQueryDrawable;
    gl_callback[kXCreateNewContext] = gl_common_XCreateNewContext;
    gl_callback[kXMakeContextCurrent] = gl_common_XMakeContextCurrent;
    gl_callback[kXCreatePixmap] = gl_common_XCreatePixmap;
    gl_callback[kXDestroyPixmap] = gl_common_XDestroyPixmap;
    gl_callback[kXCreateWindow] = gl_common_XCreateWindow;
    gl_callback[kXDestroyWindow] = gl_common_XDestroyWindow;
    gl_callback[kXCreateContextAttribsARB] = gl_common_XCreateContextAttribsARB;
    gl_callback[kXSwapIntervalEXT] = gl_common_XSwapIntervalEXT;
    gl_callback[kXSwapBuffers] = gl_common_XSwapBuffers;
}

#else
Int99Callback* int99Callback;
U32 int99CallbackSize;
U32 lastGlCallTime;
void gl_init() {
    int99CallbackSize=0;
}
#endif

void callOpenGL(CPU* cpu, U32 index) {
#ifdef BOXEDWINE_OPENGL
    //KNativeWindow::getNativeWindow()->preOpenGLCall(index);
    if (index < int99CallbackSize && int99Callback[index]) {
        cpu->thread->marshalIndex = 0;
        lastGlCallTime = KSystem::getMilliesSinceStart();
        int99Callback[index](cpu);
    } else 
#endif
{
        kpanic_fmt("Uknown int 99 call: %d", index);
    }
}
