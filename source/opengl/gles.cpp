/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL
#include GLH
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif
#include "glcommon.h"
#include "../x11/x11.h"
#include "knativesystem.h"

#define EGL_FALSE 0
#define EGL_TRUE 1
#define EGL_SUCCESS 0x3000
#define EGL_DONT_CARE 0xFFFFFFFF
#define EGL_VENDOR 0x3053
#define EGL_VERSION 0x3054
#define EGL_EXTENSIONS 0x3055
#define EGL_CLIENT_APIS 0x308D
#define EGL_OPENGL_ES_API 0x30A0
#define EGL_OPENGL_API 0x30A2
#define EGL_NONE 0x3038
#define EGL_WIDTH 0x3057
#define EGL_HEIGHT 0x3056
#define EGL_BUFFER_SIZE 0x3020
#define EGL_ALPHA_SIZE 0x3021
#define EGL_BLUE_SIZE 0x3022
#define EGL_GREEN_SIZE 0x3023
#define EGL_RED_SIZE 0x3024
#define EGL_DEPTH_SIZE 0x3025
#define EGL_STENCIL_SIZE 0x3026
#define EGL_CONFIG_CAVEAT 0x3027
#define EGL_CONFIG_ID 0x3028
#define EGL_LEVEL 0x3029
#define EGL_MAX_PBUFFER_HEIGHT 0x302A
#define EGL_MAX_PBUFFER_PIXELS 0x302B
#define EGL_MAX_PBUFFER_WIDTH 0x302C
#define EGL_NATIVE_RENDERABLE 0x302D
#define EGL_NATIVE_VISUAL_ID 0x302E
#define EGL_NATIVE_VISUAL_TYPE 0x302F
#define EGL_SAMPLES 0x3031
#define EGL_SAMPLE_BUFFERS 0x3032
#define EGL_SURFACE_TYPE 0x3033
#define EGL_TRANSPARENT_TYPE 0x3034
#define EGL_TRANSPARENT_BLUE_VALUE 0x3035
#define EGL_TRANSPARENT_GREEN_VALUE 0x3036
#define EGL_TRANSPARENT_RED_VALUE 0x3037
#define EGL_BIND_TO_TEXTURE_RGB 0x3039
#define EGL_BIND_TO_TEXTURE_RGBA 0x303A
#define EGL_MIN_SWAP_INTERVAL 0x303B
#define EGL_MAX_SWAP_INTERVAL 0x303C
#define EGL_LUMINANCE_SIZE 0x303D
#define EGL_ALPHA_MASK_SIZE 0x303E
#define EGL_COLOR_BUFFER_TYPE 0x303F
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_CONFORMANT 0x3042
#define EGL_WINDOW_BIT 0x0004
#define EGL_PBUFFER_BIT 0x0001
#define EGL_PIXMAP_BIT 0x0002
#define EGL_OPENGL_ES_BIT 0x0001
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_OPENGL_ES3_BIT 0x00000040
#define EGL_OPENGL_BIT 0x0008
#define EGL_RGB_BUFFER 0x308E
#define EGL_NO_TEXTURE 0x305C
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_CONTEXT_MAJOR_VERSION 0x3098
#define EGL_CONTEXT_MINOR_VERSION 0x30FB
#define EGL_NO_CONFIG_KHR 0
#define EGL_SYNC_STATUS 0x30F1
#define EGL_SYNC_TYPE 0x30F7
#define EGL_SYNC_CONDITION 0x30F8
#define EGL_SIGNALED 0x30F2
#define EGL_SYNC_FENCE 0x30F9
#define EGL_SYNC_PRIOR_COMMANDS_COMPLETE 0x30F0
#define EGL_CONDITION_SATISFIED 0x30F6

static U32 eglBoundApi = EGL_OPENGL_ES_API;
static U32 eglLastWindowSurfaceConfig;

static bool eglLog() {
    static int enabled = -1;
    if (enabled < 0) {
        enabled = getenv("BOXEDWINE_EGL_LOG") ? 1 : 0;
    }
    return enabled != 0;
}

static U32 eglString(CPU* cpu, const char* s) {
    U32 result = cpu->thread->process->alloc(cpu->thread, (U32)strlen(s) + 1);
    cpu->memory->memcpy(result, s, (U32)strlen(s) + 1);
    return result;
}

static const char* eglAttribName(U32 attribute) {
    switch (attribute) {
    case EGL_BUFFER_SIZE: return "EGL_BUFFER_SIZE";
    case EGL_ALPHA_SIZE: return "EGL_ALPHA_SIZE";
    case EGL_BLUE_SIZE: return "EGL_BLUE_SIZE";
    case EGL_GREEN_SIZE: return "EGL_GREEN_SIZE";
    case EGL_RED_SIZE: return "EGL_RED_SIZE";
    case EGL_DEPTH_SIZE: return "EGL_DEPTH_SIZE";
    case EGL_STENCIL_SIZE: return "EGL_STENCIL_SIZE";
    case EGL_CONFIG_CAVEAT: return "EGL_CONFIG_CAVEAT";
    case EGL_CONFIG_ID: return "EGL_CONFIG_ID";
    case EGL_LEVEL: return "EGL_LEVEL";
    case EGL_NATIVE_RENDERABLE: return "EGL_NATIVE_RENDERABLE";
    case EGL_NATIVE_VISUAL_ID: return "EGL_NATIVE_VISUAL_ID";
    case EGL_NATIVE_VISUAL_TYPE: return "EGL_NATIVE_VISUAL_TYPE";
    case EGL_SAMPLES: return "EGL_SAMPLES";
    case EGL_SAMPLE_BUFFERS: return "EGL_SAMPLE_BUFFERS";
    case EGL_SURFACE_TYPE: return "EGL_SURFACE_TYPE";
    case EGL_TRANSPARENT_TYPE: return "EGL_TRANSPARENT_TYPE";
    case EGL_TRANSPARENT_BLUE_VALUE: return "EGL_TRANSPARENT_BLUE_VALUE";
    case EGL_TRANSPARENT_GREEN_VALUE: return "EGL_TRANSPARENT_GREEN_VALUE";
    case EGL_TRANSPARENT_RED_VALUE: return "EGL_TRANSPARENT_RED_VALUE";
    case EGL_COLOR_BUFFER_TYPE: return "EGL_COLOR_BUFFER_TYPE";
    case EGL_RENDERABLE_TYPE: return "EGL_RENDERABLE_TYPE";
    case EGL_CONFORMANT: return "EGL_CONFORMANT";
    case EGL_MIN_SWAP_INTERVAL: return "EGL_MIN_SWAP_INTERVAL";
    case EGL_MAX_SWAP_INTERVAL: return "EGL_MAX_SWAP_INTERVAL";
    default: return "unknown";
    }
}

static bool eglConfigAttribValue(const CLXFBConfigPtr& cfg, U32 attribute, U32* value) {
    switch (attribute) {
    case EGL_BUFFER_SIZE: *value = cfg->glPixelFormat->pf.cColorBits; return true;
    case EGL_ALPHA_SIZE: *value = cfg->glPixelFormat->pf.cAlphaBits; return true;
    case EGL_BLUE_SIZE: *value = cfg->glPixelFormat->pf.cBlueBits; return true;
    case EGL_GREEN_SIZE: *value = cfg->glPixelFormat->pf.cGreenBits; return true;
    case EGL_RED_SIZE: *value = cfg->glPixelFormat->pf.cRedBits; return true;
    case EGL_DEPTH_SIZE: *value = cfg->glPixelFormat->pf.cDepthBits; return true;
    case EGL_STENCIL_SIZE: *value = cfg->glPixelFormat->pf.cStencilBits; return true;
    case EGL_CONFIG_ID: *value = cfg->fbId; return true;
    case EGL_NATIVE_VISUAL_ID: *value = cfg->visualId; return true;
    case EGL_MAX_PBUFFER_HEIGHT: *value = cfg->glPixelFormat->pbufferMaxHeight ? cfg->glPixelFormat->pbufferMaxHeight : 4096; return true;
    case EGL_MAX_PBUFFER_PIXELS: *value = cfg->glPixelFormat->pbufferMaxPixels ? cfg->glPixelFormat->pbufferMaxPixels : 4096 * 4096; return true;
    case EGL_MAX_PBUFFER_WIDTH: *value = cfg->glPixelFormat->pbufferMaxWidth ? cfg->glPixelFormat->pbufferMaxWidth : 4096; return true;
    case EGL_SAMPLES: *value = cfg->glPixelFormat->samples; return true;
    case EGL_SAMPLE_BUFFERS: *value = cfg->glPixelFormat->sampleBuffers ? EGL_TRUE : EGL_FALSE; return true;
    case EGL_SURFACE_TYPE: *value = EGL_WINDOW_BIT | EGL_PBUFFER_BIT | EGL_PIXMAP_BIT; return true;
    case EGL_RENDERABLE_TYPE:
    case EGL_CONFORMANT:
        *value = EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT | EGL_OPENGL_BIT;
        return true;
    case EGL_COLOR_BUFFER_TYPE: *value = EGL_RGB_BUFFER; return true;
    case EGL_CONFIG_CAVEAT:
    case EGL_NATIVE_VISUAL_TYPE:
    case EGL_TRANSPARENT_TYPE:
        *value = EGL_NONE;
        return true;
    case EGL_NATIVE_RENDERABLE: *value = EGL_TRUE; return true;
    case EGL_LEVEL:
    case EGL_TRANSPARENT_BLUE_VALUE:
    case EGL_TRANSPARENT_GREEN_VALUE:
    case EGL_TRANSPARENT_RED_VALUE:
    case EGL_BIND_TO_TEXTURE_RGB:
    case EGL_BIND_TO_TEXTURE_RGBA:
    case EGL_LUMINANCE_SIZE:
    case EGL_ALPHA_MASK_SIZE:
        *value = 0;
        return true;
    case EGL_MIN_SWAP_INTERVAL: *value = 0; return true;
    case EGL_MAX_SWAP_INTERVAL: *value = 1; return true;
    default:
        return false;
    }
}

static bool eglConfigMatches(CPU* cpu, const CLXFBConfigPtr& cfg, U32 attribList) {
    U32 originalAttribList = attribList;
    while (attribList) {
        U32 attribute = cpu->memory->readd(attribList);
        attribList += 4;
        if (attribute == EGL_NONE) {
            break;
        }
        U32 requested = cpu->memory->readd(attribList);
        attribList += 4;
        if (requested == EGL_DONT_CARE) {
            continue;
        }

        U32 actual = 0;
        if (!eglConfigAttribValue(cfg, attribute, &actual)) {
            if (eglLog()) {
                klog_fmt("boxedwine EGL: eglChooseConfig rejecting config %u unknown attr 0x%x", cfg->fbId, attribute);
            }
            return false;
        }

        switch (attribute) {
        case EGL_SURFACE_TYPE:
        case EGL_RENDERABLE_TYPE:
        case EGL_CONFORMANT:
            if ((actual & requested) != requested) {
                if (eglLog()) {
                    klog_fmt("boxedwine EGL: eglChooseConfig rejecting config %u attr %s(0x%x) actual=0x%x requested=0x%x list=0x%x", cfg->fbId, eglAttribName(attribute), attribute, actual, requested, originalAttribList);
                }
                return false;
            }
            break;
        case EGL_CONFIG_CAVEAT:
        case EGL_COLOR_BUFFER_TYPE:
        case EGL_NATIVE_RENDERABLE:
        case EGL_TRANSPARENT_TYPE:
            if (actual != requested) {
                if (eglLog()) {
                    klog_fmt("boxedwine EGL: eglChooseConfig rejecting config %u attr %s(0x%x) actual=0x%x requested=0x%x list=0x%x", cfg->fbId, eglAttribName(attribute), attribute, actual, requested, originalAttribList);
                }
                return false;
            }
            break;
        default:
            if (actual < requested) {
                if (eglLog()) {
                    klog_fmt("boxedwine EGL: eglChooseConfig rejecting config %u attr %s(0x%x) actual=0x%x requested=0x%x list=0x%x", cfg->fbId, eglAttribName(attribute), attribute, actual, requested, originalAttribList);
                }
                return false;
            }
            break;
        }
    }
    return true;
}

static CLXFBConfigPtr eglChooseDefaultConfig() {
    CLXFBConfigPtr result;
    S32 bestScore = -1;

    XServer::getServer()->iterateFbConfigs([&result, &bestScore](const CLXFBConfigPtr& cfg) {
        const PixelFormat& pfd = cfg->glPixelFormat->pf;
        if (!(pfd.dwFlags & K_PFD_DRAW_TO_WINDOW) || !(pfd.dwFlags & K_PFD_SUPPORT_OPENGL)) {
            return true;
        }

        S32 score = 0;
        if (pfd.iPixelType == K_PFD_TYPE_RGBA) score += 1000;
        if (pfd.dwFlags & K_PFD_DOUBLEBUFFER) score += 500;
        if (pfd.cRedBits == 8 && pfd.cGreenBits == 8 && pfd.cBlueBits == 8) score += 200;
        if (pfd.cAlphaBits == 8) score += 100;
        if (pfd.cDepthBits >= 24) score += 50;
        if (pfd.cStencilBits >= 8) score += 25;
        if (pfd.cRedBits > 8 || pfd.cGreenBits > 8 || pfd.cBlueBits > 8 || pfd.cAlphaBits > 8) score -= 100;
        if (pfd.dwFlags & K_PFD_SWAP_COPY) score -= 10;

        if (score > bestScore) {
            bestScore = score;
            result = cfg;
        }
        return true;
    });

    return result;
}

static XDrawablePtr eglGetSurfacelessDrawable() {
    static U32 drawableId;
    XServer* server = XServer::getServer();
    XDrawablePtr drawable;

    if (drawableId) {
        drawable = server->getDrawable(drawableId);
        if (drawable) {
            return drawable;
        }
    }

    CLXFBConfigPtr cfg = eglChooseDefaultConfig();
    if (!cfg) {
        return nullptr;
    }
    VisualPtr visual = server->getVisual(cfg->visualId);
    XPixmapPtr pixmap = server->createNewPixmap(1, 1, cfg->glPixelFormat->depth, visual);
    if (!pixmap) {
        return nullptr;
    }
    drawableId = pixmap->id;
    return pixmap;
}

static void eglWriteConfigList(CPU* cpu, U32 attribList, U32 configs, U32 configSize, U32 numConfig) {
    XServer* server = XServer::getServer();
    U32 count = 0;
    U32 written = 0;
    U32 firstConfigs[8] = {};
    server->iterateFbConfigs([cpu, attribList, configs, configSize, &count](const CLXFBConfigPtr& cfg) {
        if (attribList && !eglConfigMatches(cpu, cfg, attribList)) {
            return true;
        }
        if (configs && count < configSize) {
            cpu->memory->writed(configs + count * 4, cfg->fbId);
        }
        ++count;
        return true;
        });
    if (configs) {
        U32 readBackCount = count < configSize ? count : configSize;
        written = readBackCount;
        for (U32 i = 0; i < readBackCount && i < 8; ++i) {
            firstConfigs[i] = cpu->memory->readd(configs + i * 4);
        }
    }
    if (numConfig) {
        cpu->memory->writed(numConfig, count);
    }
    if (eglLog()) {
        klog_fmt("boxedwine EGL: config query attribList=0x%x configs=0x%x configSize=%u numConfig=0x%x count=%u written=%u first=[0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x]",
            attribList, configs, configSize, numConfig, count, written,
            firstConfigs[0], firstConfigs[1], firstConfigs[2], firstConfigs[3],
            firstConfigs[4], firstConfigs[5], firstConfigs[6], firstConfigs[7]);
    }
}

void gl_common_EglGetDisplay(CPU* cpu) {
    EAX = 1;
}

void gl_common_EglInitialize(CPU* cpu) {
    if (ARG2) {
        cpu->memory->writed(ARG2, 1);
    }
    if (ARG3) {
        cpu->memory->writed(ARG3, 5);
    }
    EAX = EGL_TRUE;
}

void gl_common_EglTerminate(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglQueryString(CPU* cpu) {
    switch (ARG2) {
    case EGL_VENDOR:
        EAX = eglString(cpu, "BoxedWine EGL");
        break;
    case EGL_VERSION:
        EAX = eglString(cpu, "1.5");
        break;
    case EGL_EXTENSIONS:
        EAX = eglString(cpu, "EGL_EXT_platform_base EGL_EXT_platform_x11 EGL_KHR_platform_x11 EGL_KHR_client_get_all_proc_addresses EGL_KHR_create_context EGL_KHR_create_context_no_error EGL_KHR_fence_sync EGL_KHR_no_config_context");
        break;
    case EGL_CLIENT_APIS:
        EAX = eglString(cpu, "OpenGL_ES OpenGL");
        break;
    default:
        EAX = 0;
        break;
    }
}

void gl_common_EglGetConfigs(CPU* cpu) {
    if (eglLog()) {
        klog_fmt("boxedwine EGL: eglGetConfigs dpy=0x%x configs=0x%x configSize=%u numConfig=0x%x", ARG1, ARG2, ARG3, ARG4);
    }
    eglWriteConfigList(cpu, 0, ARG2, ARG3, ARG4);
    EAX = EGL_TRUE;
}

void gl_common_EglChooseConfig(CPU* cpu) {
    if (eglLog()) {
        klog_fmt("boxedwine EGL: eglChooseConfig dpy=0x%x attribList=0x%x configs=0x%x configSize=%u numConfig=0x%x", ARG1, ARG2, ARG3, ARG4, ARG5);
    }
    eglWriteConfigList(cpu, ARG2, ARG3, ARG4, ARG5);
    EAX = EGL_TRUE;
}

void gl_common_EglGetConfigAttrib(CPU* cpu) {
    XServer* server = XServer::getServer();
    CLXFBConfigPtr cfg = server->getFbConfig(ARG2);
    if (!cfg || !ARG4) {
        EAX = EGL_FALSE;
        return;
    }
    U32 value = 0;
    if (!eglConfigAttribValue(cfg, ARG3, &value)) {
        if (eglLog()) {
            klog_fmt("boxedwine EGL: eglGetConfigAttrib config=%u attr=0x%x unsupported", ARG2, ARG3);
        }
        EAX = EGL_FALSE;
        return;
    }
    cpu->memory->writed(ARG4, value);
    if (eglLog()) {
        klog_fmt("boxedwine EGL: eglGetConfigAttrib config=%u attr=%s(0x%x) value=0x%x", ARG2, eglAttribName(ARG3), ARG3, value);
    }
    EAX = EGL_TRUE;
}

void gl_common_EglBindAPI(CPU* cpu) {
    if (ARG1 == EGL_OPENGL_ES_API || ARG1 == EGL_OPENGL_API) {
        eglBoundApi = ARG1;
        EAX = EGL_TRUE;
    } else {
        EAX = EGL_FALSE;
    }
}

void gl_common_EglCreateContext(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    U32 config = ARG2;
    if (config == EGL_NO_CONFIG_KHR && eglLastWindowSurfaceConfig) {
        config = eglLastWindowSurfaceConfig;
    }
    CLXFBConfigPtr cfg = config == EGL_NO_CONFIG_KHR ? eglChooseDefaultConfig() : server->getFbConfig(config);
    if (!cfg) {
        if (eglLog()) {
            klog_fmt("boxedwine EGL: eglCreateContext config=0x%x failed no cfg", ARG2);
        }
        EAX = 0;
        return;
    }
    int major = 2;
    int minor = 0;
    U32 attribList = ARG4;
    while (attribList) {
        U32 attrib = cpu->memory->readd(attribList);
        attribList += 4;
        if (attrib == EGL_NONE) {
            break;
        }
        U32 value = cpu->memory->readd(attribList);
        attribList += 4;
        if (attrib == EGL_CONTEXT_CLIENT_VERSION || attrib == EGL_CONTEXT_MAJOR_VERSION) {
            major = value;
        } else if (attrib == EGL_CONTEXT_MINOR_VERSION) {
            minor = value;
        }
    }
    U32 profile = eglBoundApi == EGL_OPENGL_ES_API ? BOXEDWINE_GL_PROFILE_ES : 0;
    EAX = KNativeSystem::getOpenGL()->glCreateContext(thread, cfg->glPixelFormat, major, minor, profile, 0, ARG3);
    if (eglLog()) {
#ifdef __EMSCRIPTEN__
        klog_fmt("boxedwine EGL: eglCreateContext config=0x%x resolved=0x%x version=%d.%d profile=0x%x share=%u -> %u webgl=%d",
                ARG2, config, major, minor, profile, ARG3, EAX, emscripten_webgl_get_current_context());
#else
        klog_fmt("boxedwine EGL: eglCreateContext config=0x%x resolved=0x%x fb=%u version=%d.%d profile=0x%x share=%u -> %u", ARG2, config, cfg->fbId, major, minor, profile, ARG3, EAX);
#endif
    }
}

void gl_common_EglDestroyContext(CPU* cpu) {
    KNativeSystem::getOpenGL()->glDestroyContext(cpu->thread, ARG2);
    EAX = EGL_TRUE;
}

static void eglCreateWindowSurface(CPU* cpu, U32 config, U32 nativeWindow) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    CLXFBConfigPtr cfg = server->getFbConfig(config);
    XWindowPtr win = server->getWindow(nativeWindow);
    if (!cfg || !win) {
        EAX = 0;
        return;
    }
    eglLastWindowSurfaceConfig = config;
    win->isOpenGL = true;
    KNativeSystem::getOpenGL()->glCreateWindow(thread, win, cfg);
    EAX = win->id;
}

void gl_common_EglCreateWindowSurface(CPU* cpu) {
    eglCreateWindowSurface(cpu, ARG2, ARG3);
}

void gl_common_EglCreatePbufferSurface(CPU* cpu) {
    XServer* server = XServer::getServer();
    CLXFBConfigPtr cfg = server->getFbConfig(ARG2);
    if (!cfg) {
        EAX = 0;
        return;
    }
    U32 width = 1;
    U32 height = 1;
    U32 attribList = ARG3;
    while (attribList) {
        U32 attrib = cpu->memory->readd(attribList);
        attribList += 4;
        if (attrib == EGL_NONE) {
            break;
        }
        U32 value = cpu->memory->readd(attribList);
        attribList += 4;
        if (attrib == EGL_WIDTH) {
            width = value;
        } else if (attrib == EGL_HEIGHT) {
            height = value;
        }
    }
    VisualPtr visual = server->getVisual(cfg->visualId);
    XPixmapPtr pixmap = server->createNewPixmap(width, height, cfg->glPixelFormat->depth, visual);
    EAX = pixmap ? pixmap->id : 0;
}

void gl_common_EglDestroySurface(CPU* cpu) {
    XServer* server = XServer::getServer();
    XWindowPtr win = server->getWindow(ARG2);
    if (win) {
        KNativeSystem::getOpenGL()->glDestroyWindow(cpu->thread, win);
        win->isOpenGL = false;
    } else {
        server->removePixmap(ARG2);
    }
    EAX = EGL_TRUE;
}

void gl_common_EglMakeCurrent(CPU* cpu) {
    U32 draw = ARG2;
    U32 read = ARG3;
    U32 ctx = ARG4;
    if (read && read != draw) {
        EAX = EGL_FALSE;
        return;
    }

    KThread* thread = cpu->thread;
    XDrawablePtr d;
    if (draw) {
        d = XServer::getServer()->getDrawable(draw);
        if (!d) {
            EAX = EGL_FALSE;
            return;
        }
    } else if (ctx) {
        d = eglGetSurfacelessDrawable();
        if (!d) {
            EAX = EGL_FALSE;
            return;
        }
    }
    thread->currentContext = ctx;
    EAX = KNativeSystem::getOpenGL()->glMakeCurrent(thread, d, ctx) ? EGL_TRUE : EGL_FALSE;
    if (eglLog()) {
#ifdef __EMSCRIPTEN__
        klog_fmt("boxedwine EGL: eglMakeCurrent draw=%u read=%u ctx=%u drawable=%u -> %u webgl=%d",
                draw, read, ctx, d ? d->id : 0, EAX, emscripten_webgl_get_current_context());
#else
        klog_fmt("boxedwine EGL: eglMakeCurrent draw=%u read=%u ctx=%u drawable=%u -> %u", draw, read, ctx, d ? d->id : 0, EAX);
#endif
    }
}

void gl_common_EglSwapBuffers(CPU* cpu) {
    XDrawablePtr d = XServer::getServer()->getDrawable(ARG2);
    if (!d) {
        EAX = EGL_FALSE;
        return;
    }
    KNativeSystem::getOpenGL()->glSwapBuffers(cpu->thread, d);
    EAX = EGL_TRUE;
}

void gl_common_EglSwapInterval(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglGetCurrentContext(CPU* cpu) {
    EAX = cpu->thread->currentContext;
}

void gl_common_EglGetCurrentSurface(CPU* cpu) {
    EAX = 0;
}

void gl_common_EglGetCurrentDisplay(CPU* cpu) {
    EAX = cpu->thread->currentContext ? 1 : 0;
}

void gl_common_EglQuerySurface(CPU* cpu) {
    XDrawablePtr d = XServer::getServer()->getDrawable(ARG2);
    if (!d || !ARG4) {
        EAX = EGL_FALSE;
        return;
    }
    switch (ARG3) {
    case EGL_WIDTH:
        cpu->memory->writed(ARG4, d->width());
        break;
    case EGL_HEIGHT:
        cpu->memory->writed(ARG4, d->height());
        break;
    default:
        cpu->memory->writed(ARG4, 0);
        break;
    }
    EAX = EGL_TRUE;
}

void gl_common_EglGetError(CPU* cpu) {
    EAX = EGL_SUCCESS;
}

void gl_common_EglReleaseThread(CPU* cpu) {
    EAX = KNativeSystem::getOpenGL()->glMakeCurrent(cpu->thread, nullptr, 0) ? EGL_TRUE : EGL_FALSE;
    cpu->thread->currentContext = 0;
}

void gl_common_EglWaitGL(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglWaitNative(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglCopyBuffers(CPU* cpu) {
    EAX = EGL_FALSE;
}

void gl_common_EglSurfaceAttrib(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglBindTexImage(CPU* cpu) {
    EAX = EGL_FALSE;
}

void gl_common_EglReleaseTexImage(CPU* cpu) {
    EAX = EGL_FALSE;
}

void gl_common_EglCreateSync(CPU* cpu) {
    EAX = 1;
}

void gl_common_EglDestroySync(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglClientWaitSync(CPU* cpu) {
    EAX = EGL_CONDITION_SATISFIED;
}

void gl_common_EglGetSyncAttrib(CPU* cpu) {
    if (!ARG4) {
        EAX = EGL_FALSE;
        return;
    }
    U32 value = 0;
    switch (ARG3) {
    case EGL_SYNC_STATUS:
        value = EGL_SIGNALED;
        break;
    case EGL_SYNC_TYPE:
        value = EGL_SYNC_FENCE;
        break;
    case EGL_SYNC_CONDITION:
        value = EGL_SYNC_PRIOR_COMMANDS_COMPLETE;
        break;
    default:
        EAX = EGL_FALSE;
        return;
    }
    cpu->memory->writed(ARG4, value);
    EAX = EGL_TRUE;
}

void gl_common_EglWaitSync(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglCreateImage(CPU* cpu) {
    EAX = 0;
}

void gl_common_EglDestroyImage(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_common_EglCreatePbufferFromClientBuffer(CPU* cpu) {
    EAX = 0;
}

void gl_common_EglCreatePixmapSurface(CPU* cpu) {
    EAX = ARG3;
}

void gl_common_EglCreatePlatformPixmapSurface(CPU* cpu) {
    EAX = ARG3;
}

void gl_common_EglCreatePlatformWindowSurface(CPU* cpu) {
    eglCreateWindowSurface(cpu, ARG2, ARG3);
}

void gl_common_EglGetPlatformDisplay(CPU* cpu) {
    EAX = 1;
}

void gl_common_EglQueryAPI(CPU* cpu) {
    EAX = eglBoundApi;
}

void gl_common_EglQueryContext(CPU* cpu) {
    if (!ARG4) {
        EAX = EGL_FALSE;
        return;
    }
    switch (ARG3) {
    case EGL_CONTEXT_MAJOR_VERSION:
        cpu->memory->writed(ARG4, 2);
        break;
    case EGL_CONTEXT_MINOR_VERSION:
        cpu->memory->writed(ARG4, 0);
        break;
    default:
        cpu->memory->writed(ARG4, 0);
        break;
    }
    EAX = EGL_TRUE;
}

void gl_common_EglWaitClient(CPU* cpu) {
    EAX = EGL_TRUE;
}

void gl_init_egl_callbacks(Int99Callback* gl_callback) {
    gl_callback[kEglGetDisplay] = gl_common_EglGetDisplay;
    gl_callback[kEglInitialize] = gl_common_EglInitialize;
    gl_callback[kEglTerminate] = gl_common_EglTerminate;
    gl_callback[kEglQueryString] = gl_common_EglQueryString;
    gl_callback[kEglGetConfigs] = gl_common_EglGetConfigs;
    gl_callback[kEglChooseConfig] = gl_common_EglChooseConfig;
    gl_callback[kEglGetConfigAttrib] = gl_common_EglGetConfigAttrib;
    gl_callback[kEglBindAPI] = gl_common_EglBindAPI;
    gl_callback[kEglCreateContext] = gl_common_EglCreateContext;
    gl_callback[kEglDestroyContext] = gl_common_EglDestroyContext;
    gl_callback[kEglCreateWindowSurface] = gl_common_EglCreateWindowSurface;
    gl_callback[kEglCreatePbufferSurface] = gl_common_EglCreatePbufferSurface;
    gl_callback[kEglDestroySurface] = gl_common_EglDestroySurface;
    gl_callback[kEglMakeCurrent] = gl_common_EglMakeCurrent;
    gl_callback[kEglSwapBuffers] = gl_common_EglSwapBuffers;
    gl_callback[kEglSwapInterval] = gl_common_EglSwapInterval;
    gl_callback[kEglGetCurrentContext] = gl_common_EglGetCurrentContext;
    gl_callback[kEglGetCurrentSurface] = gl_common_EglGetCurrentSurface;
    gl_callback[kEglGetCurrentDisplay] = gl_common_EglGetCurrentDisplay;
    gl_callback[kEglQuerySurface] = gl_common_EglQuerySurface;
    gl_callback[kEglGetError] = gl_common_EglGetError;
    gl_callback[kEglReleaseThread] = gl_common_EglReleaseThread;
    gl_callback[kEglWaitGL] = gl_common_EglWaitGL;
    gl_callback[kEglWaitNative] = gl_common_EglWaitNative;
    gl_callback[kEglCopyBuffers] = gl_common_EglCopyBuffers;
    gl_callback[kEglSurfaceAttrib] = gl_common_EglSurfaceAttrib;
    gl_callback[kEglBindTexImage] = gl_common_EglBindTexImage;
    gl_callback[kEglReleaseTexImage] = gl_common_EglReleaseTexImage;
    gl_callback[kEglCreateSync] = gl_common_EglCreateSync;
    gl_callback[kEglDestroySync] = gl_common_EglDestroySync;
    gl_callback[kEglClientWaitSync] = gl_common_EglClientWaitSync;
    gl_callback[kEglGetSyncAttrib] = gl_common_EglGetSyncAttrib;
    gl_callback[kEglWaitSync] = gl_common_EglWaitSync;
    gl_callback[kEglCreateImage] = gl_common_EglCreateImage;
    gl_callback[kEglDestroyImage] = gl_common_EglDestroyImage;
    gl_callback[kEglCreatePbufferFromClientBuffer] = gl_common_EglCreatePbufferFromClientBuffer;
    gl_callback[kEglCreatePixmapSurface] = gl_common_EglCreatePixmapSurface;
    gl_callback[kEglCreatePlatformPixmapSurface] = gl_common_EglCreatePlatformPixmapSurface;
    gl_callback[kEglCreatePlatformWindowSurface] = gl_common_EglCreatePlatformWindowSurface;
    gl_callback[kEglGetPlatformDisplay] = gl_common_EglGetPlatformDisplay;
    gl_callback[kEglQueryAPI] = gl_common_EglQueryAPI;
    gl_callback[kEglQueryContext] = gl_common_EglQueryContext;
    gl_callback[kEglWaitClient] = gl_common_EglWaitClient;
}
#endif
