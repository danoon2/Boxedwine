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

#ifdef BOXEDWINE_OPENGL_OSMESA
#include <SDL_opengl.h>
#include <SDL.h>
#include "../glcommon.h"

#include <stdio.h>
#include "knativesystem.h"
#include "osmesa.h"
#include "../../x11/x11.h"
#include "../../../lib/mesa/include/osmesa.h"

#ifdef BOXEDWINE_MSVC
#define LIBRARY_NAME "osmesa.dll"
#elif defined (__APPLE__)
#define LIBRARY_NAME "libOSMesa.8.dylib"
#else
#define LIBRARY_NAME Name needs to be set for this platform
#endif

class MesaOpenGlContext {
public:
    MesaOpenGlContext(OSMesaContext context) : context(context) {}
    ~MesaOpenGlContext() {
        delete[] buffer;
    }
    XDrawablePtr drawable;
    U32 pitch = 0;
    U32 width = 0;
    U32 height = 0;
    OSMesaContext context;
    U8* buffer = nullptr;
    U32 bufferSize = 0;
};

typedef std::shared_ptr<MesaOpenGlContext> MesaOpenGlContextPtr;

class KOpenGLMesa : public KOpenGL {
public:
    U32 glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContext) override;
    void glDestroyContext(KThread* thread, U32 contextId) override;
    bool glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) override;
    void glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) override;
    void glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) override;
    void glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) override;
    void glResizeWindow(const std::shared_ptr<XWindow>& wnd) override;
    bool isActive() override;

    GLPixelFormatPtr getFormat(U32 pixelFormatId) override;

    static BOXEDWINE_MUTEX contextMutex;
    static U32 nextId;
    static BHashTable<U32, MesaOpenGlContextPtr> contextsById;
};

BOXEDWINE_MUTEX KOpenGLMesa::contextMutex;
U32 KOpenGLMesa::nextId = 1;
BHashTable<U32, MesaOpenGlContextPtr> KOpenGLMesa::contextsById;

typedef std::shared_ptr<KOpenGLMesa> KOpenGLMesaPtr;

//typedef OSMesaContext (GLAPIENTRY* pOSMesaCreateContext)(GLenum format, OSMesaContext sharelist);
//typedef OSMesaContext (GLAPIENTRY* fn_OSMesaCreateContextExt)(GLenum format, GLint depthBits, GLint stencilBits, GLint accumBits, OSMesaContext sharelist);
typedef OSMesaContext (GLAPIENTRY* fn_OSMesaCreateContextAttribs)(const int* attribList, OSMesaContext sharelist);
typedef void (GLAPIENTRY* fn_OSMesaDestroyContext)(OSMesaContext ctx);
typedef GLboolean (GLAPIENTRY* fn_OSMesaMakeCurrent)(OSMesaContext ctx, void* buffer, GLenum type, GLsizei width, GLsizei height);
typedef OSMESAproc (GLAPIENTRY* fn_OSMesaGetProcAddress)(const char* funcName);
typedef void (GLAPIENTRY* fn_OSMesaPixelStore)(GLint pname, GLint value);

typedef void(OPENGL_CALL_TYPE* fn_glFinish)();
typedef void(OPENGL_CALL_TYPE* fn_glFlush)();

static fn_glFinish pglFinish;
static fn_glFlush pglFlush;
static fn_OSMesaMakeCurrent pOSMesaMakeCurrent;
static fn_OSMesaCreateContextAttribs pOSMesaCreateContextAttribs;
static fn_OSMesaDestroyContext pOSMesaDestroyContext;
static fn_OSMesaPixelStore pOSMesaPixelStore;
static fn_OSMesaGetProcAddress pOSMesaGetProcAddress;

#include "../../sdl/startupArgs.h"

bool KOpenGLMesa::glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) {
    MesaOpenGlContextPtr context;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
        context = contextsById.get(contextId);
    }
    if (!context) {
        return false;
    }
    context->drawable = d;
    if (!d) {
        return true;
    }
    U32 len = d->getBytesPerLine() * d->height();
    if (context->bufferSize != len) {
        delete[] context->buffer;
        context->buffer = new U8[len];
        context->bufferSize = len;
    }
    context->width = d->width();
    context->height = d->height();
    if (pOSMesaMakeCurrent(context->context, context->buffer, d->getBitsPerPixel()==16 ? GL_UNSIGNED_SHORT_5_6_5 : GL_UNSIGNED_BYTE, d->width(), d->height())) {
        pOSMesaPixelStore(OSMESA_Y_UP, 0);

        return true;
    }

    return false;
}

void KOpenGLMesa::glDestroyContext(KThread* thread, U32 contextId) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    MesaOpenGlContextPtr context = contextsById.get(contextId);

    if (context) {
        pOSMesaDestroyContext(context->context);
    }
    contextsById.remove(contextId);
}

bool KOpenGLMesa::isActive() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    return contextsById.size() > 0;
}

void KOpenGLMesa::glResizeWindow(const std::shared_ptr<XWindow>& wnd) {
    // makeCurrent handle buffer size    
}

U32 KOpenGLMesa::glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContext) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    int attribs[16] = { 0 };
    int n = 0;
    MesaOpenGlContextPtr shared;

    if (sharedContext) {
        shared = contextsById.get(sharedContext);
    }
    attribs[n++] = OSMESA_FORMAT;
    attribs[n++] = pixelFormat->nativeId;
    attribs[n++] = OSMESA_DEPTH_BITS;
    attribs[n++] = pixelFormat->pf.cDepthBits;
    attribs[n++] = OSMESA_STENCIL_BITS;
    attribs[n++] = pixelFormat->pf.cStencilBits;
    attribs[n++] = OSMESA_ACCUM_BITS;
    attribs[n++] = pixelFormat->pf.cAccumRedBits;
    if (profile) {
        attribs[n++] = OSMESA_PROFILE;
        attribs[n++] = OSMESA_CORE_PROFILE;
    }
    if (major) {
        attribs[n++] = OSMESA_CONTEXT_MAJOR_VERSION;
        attribs[n++] = major;
        attribs[n++] = OSMESA_CONTEXT_MINOR_VERSION;
        attribs[n++] = minor;
    }
    attribs[n++] = 0;    

    OSMesaContext context = pOSMesaCreateContextAttribs(attribs, (shared?shared->context:nullptr));
    if (!context) {
        return 0;
    }
    MesaOpenGlContextPtr c = std::make_shared<MesaOpenGlContext>(context);

    U32 result = nextId++;
    contextsById.set(result, c);
    return result;
}

void KOpenGLMesa::glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) {
    pglFlush();
    KOpenGLMesaPtr gl = std::dynamic_pointer_cast<KOpenGLMesa>(KNativeSystem::getOpenGL());
    MesaOpenGlContextPtr context;

    if (gl) {        
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gl->contextMutex);
        context = gl->contextsById.get(thread->currentContext);
    }
    if (context) {
        if (context->width != d->width() || context->height != d->height()) {
            glMakeCurrent(thread, d, thread->currentContext);
        } else {
            d->lockData();
            if (d->getDataSize() >= context->bufferSize) {
                memcpy(d->getData(), context->buffer, context->bufferSize);
            }
            d->unlockData();
            d->setDirty();
        }
    }    
}

void KOpenGLMesa::glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) {
}

void KOpenGLMesa::glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) {
}

static BHashTable<U32, GLPixelFormatPtr> formatsById;
static std::vector<GLPixelFormatPtr> formats;

#define OSMESA_COLOR_INDEX	GL_COLOR_INDEX
#define OSMESA_RGBA		GL_RGBA
#define OSMESA_BGRA		0x1
#define OSMESA_ARGB		0x2
#define OSMESA_RGB		GL_RGB
#define OSMESA_BGR		0x4
#define OSMESA_RGB_565		0x5

void addDoubleBuffer(U32 nativeId, U32 rBits, U32 rShift, U32 gBits, U32 gShift, U32 bBits, U32 bShift, U32 aBits, U32 aShift, U32 depth, U32 bitsPerPixel, U32 depthBuffers, U32 stencil, U32 accum, bool doubleBuffer) {
    GLPixelFormatPtr format = std::make_shared<GLPixelFormat>();
    format->id = formats.size() | PIXEL_FORMAT_NATIVE_INDEX_MASK;
    format->nativeId = nativeId;
    format->depth = depth;
    format->bitsPerPixel = bitsPerPixel;
    format->pf.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    format->pf.nVersion = 1;
    format->pf.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP;

    if (doubleBuffer) {
        format->pf.dwFlags |= PFD_DOUBLEBUFFER;
    }

    if (bitsPerPixel > 8) {
        format->pf.iPixelType = PFD_TYPE_RGBA;
    } else  {
        format->pf.iPixelType = PFD_TYPE_COLORINDEX;
    }
    format->pf.cColorBits = bitsPerPixel;
    format->pf.cRedBits = rBits;
    format->pf.cRedShift = rShift;
    format->pf.cGreenBits = gBits;
    format->pf.cGreenShift = gShift;
    format->pf.cBlueBits = bBits;
    format->pf.cBlueShift = bShift;
    format->pf.cAlphaBits = aBits;
    format->pf.cAlphaShift = aShift;
    format->pf.cAccumBits = accum * 3 + (aBits ? accum : 0);
    format->pf.cAccumRedBits = accum;
    format->pf.cAccumGreenBits = accum;
    format->pf.cAccumBlueBits = accum;
    format->pf.cAccumAlphaBits = aBits ? accum : 0;
    format->pf.cDepthBits = depthBuffers;
    format->pf.cStencilBits = stencil;
    format->pf.cAuxBuffers = 0;
    format->pf.iLayerType = PFD_MAIN_PLANE;
    format->pf.bReserved = 0;
    format->pf.dwLayerMask = 0;
    format->pf.dwVisibleMask = 0;
    format->pf.dwDamageMask = 0;

    formatsById.set(format->id, format);
    formats.push_back(format);
}

void addAccum(U32 nativeId, U32 rBits, U32 rShift, U32 gBits, U32 gShift, U32 bBits, U32 bShift, U32 aBits, U32 aShift, U32 depth, U32 bitsPerPixel, U32 depthBuffers, U32 stencil, U32 accum) {
    addDoubleBuffer(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, depthBuffers, stencil, accum, true);
    addDoubleBuffer(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, depthBuffers, stencil, accum, false);
}

void addStencil(U32 nativeId, U32 rBits, U32 rShift, U32 gBits, U32 gShift, U32 bBits, U32 bShift, U32 aBits, U32 aShift, U32 depth, U32 bitsPerPixel, U32 depthBuffers, U32 stencil) {
    addAccum(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, depthBuffers, stencil, 0);
    addAccum(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, depthBuffers, stencil, 16);
}

void addDepth(U32 nativeId, U32 rBits, U32 rShift, U32 gBits, U32 gShift, U32 bBits, U32 bShift, U32 aBits, U32 aShift, U32 depth, U32 bitsPerPixel, U32 depthBuffers) {
    addStencil(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, depthBuffers, 0);
    addStencil(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, depthBuffers, 8);
}

void addFormat(U32 nativeId, U32 rBits, U32 rShift, U32 gBits, U32 gShift, U32 bBits, U32 bShift, U32 aBits, U32 aShift, U32 depth, U32 bitsPerPixel) {
    addDepth(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, 0);
    addDepth(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, 16);
    addDepth(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, 24);
    addDepth(nativeId, rBits, rShift, gBits, gShift, bBits, bShift, aBits, aShift, depth, bitsPerPixel, 32);
}

#define OSMESA_COLOR_INDEX	GL_COLOR_INDEX
#define OSMESA_RGBA		GL_RGBA
#define OSMESA_BGRA		0x1
#define OSMESA_ARGB		0x2
#define OSMESA_RGB		GL_RGB
#define OSMESA_BGR		0x4
#define OSMESA_RGB_565		0x5

void addFormats() {
    //addFormat(OSMESA_RGBA, 8, 0, 8, 8, 8, 16, 8, 24, 32, 32);
    addFormat(OSMESA_BGRA, 8, 16, 8, 8, 8, 0, 8, 24, 32, 32);
    //addFormat(OSMESA_ARGB, 8, 8, 8, 16, 8, 24, 8, 0, 32, 32);
    addFormat(OSMESA_RGB, 8, 16, 8, 8, 8, 0, 0, 0, 24, 24);
    //addFormat(OSMESA_BGR, 8, 0, 8, 8, 8, 16, 0, 0, 24, 24);
    addFormat(OSMESA_RGB_565, 5, 11, 6, 5, 5, 0, 0, 0, 16, 16);
    addFormat(OSMESA_COLOR_INDEX, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8);
}

void OsMesaGL::iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback) {
    if (formats.size() == 0) {
        addFormats();
    }
    for (auto& format : formats) {
        callback(format);
    }
}

GLPixelFormatPtr KOpenGLMesa::getFormat(U32 pixelFormatId) {
    return formatsById.get(pixelFormatId);
}

static bool found = false;
static bool isAvailableInitialized = false;

bool OsMesaGL::isAvailable() {
    if (!isAvailableInitialized) {
        isAvailableInitialized = true;
        BString libPath = KSystem::exePath + LIBRARY_NAME;
        void* dll = SDL_LoadObject(libPath.c_str());
        if (dll) {
            found = true;
            SDL_UnloadObject(dll);
        }
    }
    return found;
}

// GLAPI void APIENTRY glFinish( void ) {
static void osmesa_glFinish(CPU* cpu) {
    pglFinish();
}

// GLAPI void APIENTRY glFlush( void ) {
static void osmesa_glFlush(CPU* cpu) {
    KThread* thread = cpu->thread;
    KOpenGLMesaPtr gl = std::dynamic_pointer_cast<KOpenGLMesa>(KNativeSystem::getOpenGL());
    if (gl) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gl->contextMutex);
        MesaOpenGlContextPtr context = gl->contextsById.get(thread->currentContext);

        if (context) {
            gl->glSwapBuffers(thread, context->drawable);
            return;
        }
    }
    // hope for the best, this shouldn't be possible
    pglFlush();
}

static void* pDLL;

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) pgl##func = (gl##func##_func)pOSMesaGetProcAddress("gl" #func);

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) pgl##func = (gl##func##_func)pOSMesaGetProcAddress("gl" #func);

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) ext_gl##func = (gl##func##_func)pOSMesaGetProcAddress("gl" #func);

static void initMesaOpenGL() {    
    if (!pDLL) {
        BString libPath = KSystem::exePath + LIBRARY_NAME;
        pDLL = SDL_LoadObject(libPath.c_str());
        if (!pDLL) {
            klog("Failed to load %s %s", libPath.c_str(), SDL_GetError());
            return;
        }
    }
    pOSMesaGetProcAddress = (fn_OSMesaGetProcAddress)SDL_LoadFunction(pDLL, "OSMesaGetProcAddress");
    if (!pOSMesaGetProcAddress) {
        return;
    }
    pOSMesaMakeCurrent = (fn_OSMesaMakeCurrent)SDL_LoadFunction(pDLL, "OSMesaMakeCurrent");
    pOSMesaCreateContextAttribs = (fn_OSMesaCreateContextAttribs)SDL_LoadFunction(pDLL, "OSMesaCreateContextAttribs");
    pOSMesaDestroyContext = (fn_OSMesaDestroyContext)SDL_LoadFunction(pDLL, "OSMesaDestroyContext");
    pOSMesaPixelStore = (fn_OSMesaPixelStore)SDL_LoadFunction(pDLL, "OSMesaPixelStore");

    pglFinish = pOSMesaGetProcAddress("glFinish");
    pglFlush = pOSMesaGetProcAddress("glFlush");
#include "../glfunctions.h"

    int99Callback[Finish] = osmesa_glFinish;
    int99Callback[Flush] = osmesa_glFlush;
}

KOpenGLPtr OsMesaGL::create() {
    initMesaOpenGL();
    return std::make_shared<KOpenGLMesa>();
};

#endif
