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
#include "knativewindow.h"
#include "glcommon.h"

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)
#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)
#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) gl##func##_func ext_gl##func;

#include "glfunctions.h"

#include "glMarshal.h"

thread_local static BString glExt;

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
        kpanic("marshalType unknown type: %d", type);
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
    klog("GL Vendor: %s", (const char*)GL_FUNC(pglGetString)(GL_VENDOR));
    klog("GL Renderer: %s", (const char*)GL_FUNC(pglGetString)(GL_RENDERER));
    klog("GL Version: %s", (const char*)GL_FUNC(pglGetString)(GL_VERSION));
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
    if (process->glStrings.get(name, previousAddress)) {
        if (process->memory->memcmp(previousAddress, result, (U32)strlen(result)+1) == 0) {
            EAX = previousAddress;
            return;
        }
    }
    U32 len = (U32)strlen(result) + 1;
    U32 address = cpu->memory->mmap(cpu->thread, 0, len, K_PROT_WRITE|K_PROT_READ, K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
    cpu->memory->memcpy(address, (void*)result, len);
    cpu->thread->process->glStringsiExtensions = address;

    process->glStrings.set(name, address);
    EAX = address;
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
        kpanic("glGetMapdv unknown query: %d", query);
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
        kpanic("glGetMapfv unknown query: %d", query);
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
        kpanic("glGetMapfv unknown query: %d", query);
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
    klog("%s", message);
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

// create variables to hold standard opengl calls like glClear
#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) gl##func##_func pgl##func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) gl##func##_func pgl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS)

#include "glfunctions.h"

// create the functions that will make the OpenGL call, these will be assigned into gl_callback
#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) void glcommon_gl##func(CPU* cpu) { PRE GL_FUNC(pgl##func)ARGS; POST; GL_LOG LOG;} 

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

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) gl_callback[func] = glcommon_gl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) gl_callback[func] = glcommon_gl##func;

#include "glfunctions.h"      
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
    KNativeWindow::getNativeWindow()->preOpenGLCall(index);
    if (index < int99CallbackSize && int99Callback[index]) {
        cpu->thread->marshalIndex = 0;
        lastGlCallTime = KSystem::getMilliesSinceStart();
        int99Callback[index](cpu);
    } else 
#endif
{
        kpanic("Uknown int 99 call: %d", index);
    }
}
