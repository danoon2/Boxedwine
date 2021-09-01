
#include "egl.h"
#include "../gl/loader.h"
#include "../glx/hardext.h"

#ifndef AliasExport
#define AliasExport(name)   __attribute__((alias(name))) __attribute__((visibility("default")))
#endif

static EGLint egl_context_attrib_es2[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

static EGLint egl_context_attrib[] = {
    EGL_NONE
};

EGLint gl4es_eglGetError(void) {
    LOAD_EGL(eglGetError);
    return egl_eglGetError();
}

EGLDisplay gl4es_eglGetDisplay(EGLNativeDisplayType display_id) {
    LOAD_EGL(eglGetDisplay);
    return egl_eglGetDisplay(display_id);
}

EGLBoolean gl4es_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) {
    LOAD_EGL(eglInitialize);
    return egl_eglInitialize(dpy, major, minor);
}

EGLBoolean gl4es_eglTerminate(EGLDisplay dpy) {
    LOAD_EGL(eglTerminate);
    return egl_eglTerminate(dpy);
}

const char * gl4es_eglQueryString(EGLDisplay dpy, EGLint name) {
    LOAD_EGL(eglQueryString);
    return egl_eglQueryString(dpy, name);
}

EGLBoolean gl4es_eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    LOAD_EGL(eglGetConfigs);
    return egl_eglGetConfigs(dpy, configs, config_size, num_config);
}

EGLBoolean gl4es_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    LOAD_EGL(eglChooseConfig);
    return egl_eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean gl4es_eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
    EGLint old_value = 0;
    LOAD_EGL(eglGetConfigAttrib);
    EGLBoolean ret = egl_eglGetConfigAttrib(dpy, config, attribute, &old_value);
    switch (attribute) {
        case EGL_RENDERABLE_TYPE:
            *value = old_value | EGL_OPENGL_BIT;
        break;
        default:
            *value = old_value;
    }
    return ret;
}

EGLSurface gl4es_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) {
    LOAD_EGL(eglCreateWindowSurface);
    return egl_eglCreateWindowSurface(dpy, config, win, attrib_list);
}

EGLSurface gl4es_eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
    LOAD_EGL(eglCreatePbufferSurface);
    return egl_eglCreatePbufferSurface(dpy, config, attrib_list);
}

EGLSurface gl4es_eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) {
    LOAD_EGL(eglCreatePixmapSurface);
    return egl_eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

EGLBoolean gl4es_eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    LOAD_EGL(eglDestroySurface);
    return egl_eglDestroySurface(dpy, surface);
}

EGLBoolean gl4es_eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) {
    LOAD_EGL(eglQuerySurface);
    return egl_eglQuerySurface(dpy, surface, attribute, value);
}

EGLBoolean gl4es_eglBindAPI(EGLenum api) {
    LOAD_EGL(eglBindAPI);
    return egl_eglBindAPI(EGL_OPENGL_ES_API);
}

EGLenum gl4es_eglQueryAPI(void) {
    return EGL_OPENGL_API;
}

EGLBoolean gl4es_eglWaitClient(void) {
    LOAD_EGL(eglWaitClient);
    return egl_eglWaitClient();
}

EGLBoolean gl4es_eglReleaseThread(void) {
    LOAD_EGL(eglReleaseThread);
    return egl_eglReleaseThread();
}

EGLSurface gl4es_eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) {
    LOAD_EGL(eglCreatePbufferFromClientBuffer);
    return egl_eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean gl4es_eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    LOAD_EGL(eglSurfaceAttrib);
    return egl_eglSurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean gl4es_eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    LOAD_EGL(eglBindTexImage);
    return egl_eglBindTexImage(dpy, surface, buffer);
}

EGLBoolean gl4es_eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    LOAD_EGL(eglReleaseTexImage);
    return egl_eglReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean gl4es_eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    LOAD_EGL(eglSwapInterval);
    return egl_eglSwapInterval(dpy, interval);
}

EGLContext gl4es_eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
    LOAD_EGL(eglCreateContext);
    return egl_eglCreateContext(dpy, config, share_context, (hardext.esversion == 1) ? egl_context_attrib : egl_context_attrib_es2);
}

EGLBoolean gl4es_eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    LOAD_EGL(eglDestroyContext);
    return egl_eglDestroyContext(dpy, ctx);
}

EGLBoolean gl4es_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    LOAD_EGL(eglMakeCurrent);
    return egl_eglMakeCurrent(dpy, draw, read, ctx);
}

EGLContext gl4es_eglGetCurrentContext(void) {
    LOAD_EGL(eglGetCurrentContext);
    return egl_eglGetCurrentContext();
}

EGLSurface gl4es_eglGetCurrentSurface(EGLint readdraw) {
    LOAD_EGL(eglGetCurrentSurface);
    return egl_eglGetCurrentSurface(readdraw);
}

EGLDisplay gl4es_eglGetCurrentDisplay(void) {
    LOAD_EGL(eglGetCurrentDisplay);
    return egl_eglGetCurrentDisplay();
}

EGLBoolean gl4es_eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) {
    LOAD_EGL(eglQueryContext);
    return egl_eglQueryContext(dpy, ctx, attribute, value);
}

EGLBoolean gl4es_eglWaitGL(void) {
    LOAD_EGL(eglWaitGL);
    return egl_eglWaitGL();
}

EGLBoolean gl4es_eglWaitNative(EGLint engine) {
    LOAD_EGL(eglWaitNative);
    return egl_eglWaitNative(engine);
}

EGLBoolean gl4es_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    LOAD_EGL(eglSwapBuffers);
    return egl_eglSwapBuffers(dpy, surface);
}

EGLBoolean gl4es_eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
    LOAD_EGL(eglCopyBuffers);
    return egl_eglCopyBuffers(dpy, surface, target);
}

EGLint eglGetError(void) AliasExport("gl4es_eglGetError");
EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) AliasExport("gl4es_eglGetDisplay");
EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor) AliasExport("gl4es_eglInitialize");
EGLBoolean eglTerminate(EGLDisplay dpy) AliasExport("gl4es_eglTerminate");
const char * eglQueryString(EGLDisplay dpy, EGLint name) AliasExport("gl4es_eglQueryString");
EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) AliasExport("gl4es_eglGetConfigs");
EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) AliasExport("gl4es_eglChooseConfig");
EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) AliasExport("gl4es_eglGetConfigAttrib");
EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) AliasExport("gl4es_eglCreateWindowSurface");
EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) AliasExport("gl4es_eglCreatePbufferSurface");
EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) AliasExport("gl4es_eglCreatePixmapSurface");
EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) AliasExport("gl4es_eglDestroySurface");
EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) AliasExport("gl4es_eglQuerySurface");
EGLBoolean eglBindAPI(EGLenum api) AliasExport("gl4es_eglBindAPI");
EGLenum eglQueryAPI(void) AliasExport("gl4es_eglQueryAPI");
EGLBoolean eglWaitClient(void) AliasExport("gl4es_eglWaitClient");
EGLBoolean eglReleaseThread(void) AliasExport("gl4es_eglReleaseThread");
EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) AliasExport("gl4es_eglCreatePbufferFromClientBuffer");
EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) AliasExport("gl4es_eglSurfaceAttrib");
EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) AliasExport("gl4es_eglBindTexImage");
EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) AliasExport("gl4es_eglReleaseTexImage");
EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) AliasExport("gl4es_eglSwapInterval");
EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) AliasExport("gl4es_eglCreateContext");
EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) AliasExport("gl4es_eglDestroyContext");
EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) AliasExport("gl4es_eglMakeCurrent");
EGLContext eglGetCurrentContext(void) AliasExport("gl4es_eglGetCurrentContext");
EGLSurface eglGetCurrentSurface(EGLint readdraw) AliasExport("gl4es_eglGetCurrentSurface");
EGLDisplay eglGetCurrentDisplay(void) AliasExport("gl4es_eglGetCurrentDisplay");
EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) AliasExport("gl4es_eglQueryContext");
EGLBoolean eglWaitGL(void) AliasExport("gl4es_eglWaitGL");
EGLBoolean eglWaitNative(EGLint engine) AliasExport("gl4es_eglWaitNative");
EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) AliasExport("gl4es_eglSwapBuffers");
EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) AliasExport("gl4es_eglCopyBuffers");

