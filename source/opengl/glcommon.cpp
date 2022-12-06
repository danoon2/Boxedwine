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

#ifdef BOXEDWINE_64BIT_MMU
#include "../emulation/hardmmu/hard_memory.h"
#endif

static std::string glExt;

float fARG(CPU* cpu, U32 arg) {
    struct int2Float i;
    i.i = arg;
    return i.f;
}

double dARG(CPU* cpu, int address) {
    struct long2Double i;
    i.l = readq(address);
    return i.d;
}

// GLintptr is 32-bit in the emulator, on the host it depends on void* size
GLintptr* bufferip;
U32 bufferip_len;

GLintptr* marshalip(CPU* cpu, U32 address, U32 count) {
    U32 i;

    if (!address)
        return NULL;
    if (bufferip && bufferip_len<count) {
        delete[] bufferip;
        bufferip=NULL;
    }
    if (!bufferip) {
        bufferip = new GLintptr[count];
        bufferip_len = count;
    }
    for (i=0;i<count;i++) {
        bufferip[i] = (GLintptr)readd(address);
        address+=4;
    }
    return bufferip;
}

GLintptr* buffer2ip;
U32 buffer2ip_len;

GLintptr* marshal2ip(CPU* cpu, U32 address, U32 count) {
    U32 i;

    if (!address)
        return NULL;
    if (buffer2ip && buffer2ip_len<count) {
        delete[] buffer2ip;
        buffer2ip=NULL;
    }
    if (!buffer2ip) {
        buffer2ip = new GLintptr[count];
        buffer2ip_len = count;
    }
    for (i=0;i<count;i++) {
        buffer2ip[i] = (GLintptr)readd(address);
        address+=4;
    }
    return buffer2ip;
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
static U32 feedbackBufferAddress;

void glcommon_glFeedbackBuffer(CPU* cpu) {
    GLsizei size = ARG1;
    GLenum type = ARG2;
#ifdef BOXEDWINE_64BIT_MMU
    U32 buffer = ARG3; // GLfloat*
    GL_FUNC(pglFeedbackBuffer)(size, type, (GLfloat*)getNativeAddress(cpu->thread->process->memory, buffer));
#else
    if (size > feedbackBufferSize) {
        if (feedbackBuffer) {
            delete[] feedbackBuffer;
        }
        feedbackBuffer = new GLfloat[size];
        feedbackBufferSize = size;
    }
    GL_FUNC(pglFeedbackBuffer)(size, type, feedbackBuffer);
    feedbackBufferAddress = ARG3;
#endif
}

// changed this to an invalid value to fix motorhead under windows.  I will need to reevaluate why it was necessary for ma
static const char* addedExt[] = { "WGL_ARB_create_context_Invalid" };

void glcommon_glGetIntegerv(CPU* cpu) {
    GLenum pname = ARG1;
    if (pname == GL_NUM_EXTENSIONS) {
        writed(ARG2, cpu->thread->process->numberOfExtensions);
    } else {
        GLint* buffer = marshali(cpu, ARG2, getSize(ARG1));
        GL_FUNC(pglGetIntegerv)(pname, buffer);
        marshalBacki(cpu, ARG2, buffer, getSize(ARG1));
    }
}

void glcommon_glRenderMode(CPU* cpu) {
    GLenum mode = ARG1;
    EAX = GL_FUNC(pglRenderMode)(mode);
#ifndef BOXEDWINE_64BIT_MMU
    // could be -1
    if (EAX < (U32)feedbackBufferSize) {
        marshalBackf(cpu, feedbackBufferAddress, feedbackBuffer, EAX);
    }
#endif
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
            std::vector<std::string> hardwareExt;
            std::vector<std::string> supportedExt;
            stringSplit(hardwareExt, result, ' ');
            for (U32 i=0;i<sizeof(extentions)/sizeof(char*);i++) {
                supportedExt.push_back(extentions[i]);
            }
            cpu->thread->process->numberOfExtensions = 0;
            for (U32 i=0;i<hardwareExt.size();i++) {
                if (std::find(supportedExt.begin(), supportedExt.end(), hardwareExt[i]) == supportedExt.end()) {
                    continue;
                }

                if (!glExt.length() || strstr(glExt.c_str(), hardwareExt[i].c_str())) {
                    if (ext[0]!=0)
                        strcat(ext, " ");
                    cpu->thread->process->numberOfExtensions++;
                    strcat(ext, hardwareExt[i].c_str());
                }
            }
            for (U32 i = 0; i < sizeof(addedExt) / sizeof(*addedExt); i++) {
                if (ext[0] != 0)
                    strcat(ext, " ");
                cpu->thread->process->numberOfExtensions++;
                strcat(ext, addedExt[i]);
            }

        }
        result = ext;
#endif
        GL_LOG("glGetString GLenum name=GL_EXTENSIONS ret=%s", result);
    } else {
    }
#ifdef BOXEDWINE_64BIT_MMU
    if (!cpu->thread->process->glStrings.count(name)) {
        U32 len = (U32)strlen(result);
        U32 address = cpu->thread->process->allocNative(len+1);
        char* nativeResult = (char*)getNativeAddress(cpu->thread->process->memory, address);
        strcpy(nativeResult, result);
        cpu->thread->process->glStrings[name] = address;

        if (name == GL_EXTENSIONS) {
            address = cpu->thread->process->allocNative(len + 1);
            cpu->thread->process->glStringsiExtensions = address;
            nativeResult = (char*)getNativeAddress(cpu->thread->process->memory, address);
            strcpy(nativeResult, result);
            cpu->thread->process->glStringsiExtensionsOffset.push_back(0);
            for (int i = 0; i < (int)len; i++) {
                char c = nativeResult[i];
                if (c == ' ') {
                    nativeResult[i] = 0;
                    cpu->thread->process->glStringsiExtensionsOffset.push_back(i + 1);
                }
            }
        }
    }
    EAX = cpu->thread->process->glStrings[name];
#else
    if (name == GL_EXTENSIONS && !cpu->thread->process->glStringsiExtensions) {
        int len = strlen(result);
        U32 pageCount = ((len + 1) + K_PAGE_MASK) >> K_PAGE_SHIFT;
        U32 page = 0;
        cpu->thread->memory->findFirstAvailablePage(ADDRESS_PROCESS_MMAP_START, pageCount, &page, false);
        cpu->thread->memory->allocPages(page, pageCount, PAGE_READ | PAGE_WRITE, 0, 0, nullptr);
        U32 address = page << K_PAGE_SHIFT;
        cpu->thread->process->glStringsiExtensions = address;
        memcopyFromNative(address, result, len + 1);
        cpu->thread->process->glStringsiExtensionsOffset.push_back(0);
        for (int i = 0; i < (int)len; i++) {
            char c = readb(address+i);
            if (c == ' ') {
                writeb(address+i, 0);
                cpu->thread->process->glStringsiExtensionsOffset.push_back(i + 1);
            }
        }
    }
    EAX = cpu->thread->memory->mapNativeMemory((void*)result, (U32)(strlen(result)+1));
#endif
}

// GLAPI void APIENTRY glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ) {
void glcommon_glGetTexImage(CPU* cpu) {
    GLenum target = ARG1;
    GLint level = ARG2;
    GLsizei width;
    GLsizei height;
    GLenum format = ARG3;
    GLenum type = ARG4;

    GLvoid* pixels;
    GLboolean b = PIXEL_PACK_BUFFER();

    //GL_LOG("glGetTexImage GLenum target=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, GLvoid *pixels=%.08x", ARG1, ARG2, ARG3, ARG4, ARG5);
    if (b) {
        pixels = (GLvoid*)pARG5;
    } else {
        GL_FUNC(pglGetTexLevelParameteriv)(target, level, GL_TEXTURE_WIDTH, &width);
        GL_FUNC(pglGetTexLevelParameteriv)(target, level, GL_TEXTURE_HEIGHT, &height);
        pixels = marshalPixels(cpu, target == GL_TEXTURE_3D, width, height, 1, format, type, ARG5);
    }
    GL_FUNC(pglGetTexImage)(target, level, format, type, pixels);
    if (!b)
        marshalBackPixels(cpu, target == GL_TEXTURE_3D, width, height, 1, format, type, ARG5, pixels);
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
        GLdouble* buffer;
        GLint order[2];
        int count;

        GL_FUNC(pglGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        buffer = marshald(cpu, ARG3, count);
        GL_FUNC(pglGetMapdv)(target, query, buffer);
        marshalBackd(cpu, ARG3, buffer, count);
        break;
    }
    case GL_ORDER: {
        GLdouble buffer[2];
        GL_FUNC(pglGetMapdv)(target, query, buffer);
        marshalBackd(cpu, ARG3, buffer, isMap2(target)?2:1);
    }
    case GL_DOMAIN: {
        GLdouble buffer[4];
        GL_FUNC(pglGetMapdv)(target, query, buffer);
        marshalBackd(cpu, ARG3, buffer, isMap2(target)?4:2);
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
        GLfloat* buffer;
        GLint order[2];
        int count;

        GL_FUNC(pglGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        buffer = marshalf(cpu, ARG3, count);
        GL_FUNC(pglGetMapfv)(target, query, buffer);
        marshalBackf(cpu, ARG3, buffer, count);
        break;
    }
    case GL_ORDER: {
        GLfloat buffer[2];
        GL_FUNC(pglGetMapfv)(target, query, buffer);
        marshalBackf(cpu, ARG3, buffer, isMap2(target)?2:1);
    }
    case GL_DOMAIN: {
        GLfloat buffer[4];
        GL_FUNC(pglGetMapfv)(target, query, buffer);
        marshalBackf(cpu, ARG3, buffer, isMap2(target)?4:2);
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
        GLint* buffer;
        GLint order[2];
        int count;

        GL_FUNC(pglGetMapiv)(target, GL_ORDER, order);
        if (isMap2(target)) {
            count = order[0]*order[1];
        } else {
            count = order[0];
        }
        buffer = marshali(cpu, ARG3, count);
        GL_FUNC(pglGetMapiv)(target, query, buffer);
        marshalBacki(cpu, ARG3, buffer, count);
        break;
    }
    case GL_ORDER: {
        GLint buffer[2];
        GL_FUNC(pglGetMapiv)(target, query, buffer);
        marshalBacki(cpu, ARG3, buffer, isMap2(target)?2:1);
    }
    case GL_DOMAIN: {
        GLint buffer[4];
        GL_FUNC(pglGetMapiv)(target, query, buffer);
        marshalBacki(cpu, ARG3, buffer, isMap2(target)?4:2);
        break;
    }
    default:
        kpanic("glGetMapfv unknown query: %d", query);
    }	
}

// GLAPI void APIENTRY glGetPointerv( GLenum pname, GLvoid **params ) {
void glcommon_glGetPointerv(CPU* cpu) {
    GL_LOG("glGetPointerv GLenum pname=%d, GLvoid **params=%.08x", ARG1, ARG2);
#ifdef BOXEDWINE_64BIT_MMU
    {
        GLvoid* params;
        GL_FUNC(pglGetPointerv)(ARG1, &params);
        if ((U64)params>0xFFFFFFFFl)
            kwarn("problem with glGetPointerv");
        writed(ARG2, (U32)(size_t)params);
    }
#else
    switch (ARG1) {
    case GL_COLOR_ARRAY_POINTER: writed(readd(ARG2), cpu->thread->glColorPointer.ptr); break;
    case GL_EDGE_FLAG_ARRAY_POINTER: writed(readd(ARG2), cpu->thread->glEdgeFlagPointer.ptr); break;
    case GL_INDEX_ARRAY_POINTER: writed(readd(ARG2), cpu->thread->glIndexPointer.ptr); break;
    case GL_NORMAL_ARRAY_POINTER: writed(readd(ARG2), cpu->thread->glNormalPointer.ptr); break;
    case GL_TEXTURE_COORD_ARRAY_POINTER: writed(readd(ARG2), cpu->thread->glTexCoordPointer.ptr); break;
    case GL_VERTEX_ARRAY_POINTER: writed(readd(ARG2), cpu->thread->glVertextPointer.ptr); break;
    default: writed(readd(ARG2), 0);
    }
#endif
}

// GLAPI void APIENTRY glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ) {
void glcommon_glInterleavedArrays(CPU* cpu) {
    GLenum format = ARG1;
    GLsizei stride = ARG2;
    U32 address = ARG3;
#ifdef BOXEDWINE_64BIT_MMU
    GL_FUNC(pglInterleavedArrays)(format, stride, getNativeAddress(cpu->thread->process->memory, address));
#else
    GL_FUNC(pglInterleavedArrays)(format, stride, marshalInterleavedPointer(cpu, format, stride, address));
#endif    
}

// GLAPI void APIENTRY glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) {
void glcommon_glReadPixels(CPU* cpu) {
    GLvoid* pixels;
    GLsizei width = ARG3;
    GLsizei height = ARG4;
    GLenum format = ARG5;
    GLenum type = ARG6;
    GLboolean b = PIXEL_PACK_BUFFER();

    GL_LOG("glReadPixels GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, GLvoid *pixels=%.08x", ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);

    if (b)
        pixels = (GLvoid*)pARG7;
    else
        pixels = marshalPixels(cpu, 0, width, height, 1, format, type, ARG7);
    GL_FUNC(pglReadPixels)(ARG1, ARG2, width, height, format, type, pixels);
    if (!b)
        marshalBackPixels(cpu, 0, width, height, 1, format, type, ARG7, pixels);
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

void gl_init(const std::string& allowExtensions) {    
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
        lastGlCallTime = KSystem::getMilliesSinceStart();
        int99Callback[index](cpu);
    } else 
#endif
{
        kpanic("Uknown int 99 call: %d", index);
    }
}
