#include "../opengl/gldef.h"

typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef int intptr_t;
typedef unsigned int uintptr_t;

extern char* getenv(const char* name) __attribute__((weak));
extern void* dlopen(const char* filename, int flags);
extern void* dlsym(void* handle, const char* symbol);

#define RTLD_NOW 2
#define RTLD_GLOBAL 0x100

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wint-conversion"
#endif

#ifndef EGLAPI
#define EGLAPI
#endif

#ifndef EGLAPIENTRY
// Wrappers that adapt an extension's parameters must call a separate ABI stub.
#define EGLAPIENTRY __attribute__((noinline))
#endif

typedef int32_t EGLint;
typedef uint32_t EGLBoolean;
typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLSurface;
typedef void* EGLContext;
typedef void* EGLSync;
typedef void* EGLImage;
typedef void* EGLClientBuffer;
typedef void* EGLNativeDisplayType;
typedef void* EGLNativeWindowType;
typedef void* EGLNativePixmapType;
typedef intptr_t EGLAttrib;
typedef intptr_t EGLAttribKHR;
typedef uint64_t EGLTime;
typedef uint64_t EGLTimeKHR;
typedef uint64_t EGLTimeNV;
typedef uint64_t EGLuint64KHR;
typedef uint64_t EGLuint64NV;
typedef uint32_t EGLenum;
typedef void (*__eglMustCastToProperFunctionPointerType)(void);

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char* procname);

#define EGL_TRUE 1
#define EGL_FALSE 0

// Match libGL: arguments stay in the original i386 call frame and the callback
// index follows the interrupt. EAX is returned unchanged by the optimized stub.
#define EGL_CALL(index) __asm__ volatile("int $0x99\n\t.long %c0" :: "i"(index) : "memory", "cc")
#define CALL_0_R(index) EGL_CALL(index)
#define CALL_1_R(index, ...) EGL_CALL(index)
#define CALL_2_R(index, ...) EGL_CALL(index)
#define CALL_3_R(index, ...) EGL_CALL(index)
#define CALL_4_R(index, ...) EGL_CALL(index)
#define CALL_5_R(index, ...) EGL_CALL(index)

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id) {
    CALL_1_R(kEglGetDisplay, display_id);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLint platform, void* native_display, const EGLint* attrib_list) {
    return eglGetDisplay(native_display);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplay(EGLint platform, void* native_display, const EGLAttrib* attrib_list) {
    CALL_3_R(kEglGetPlatformDisplay, platform, native_display, attrib_list);
}

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor) {
    CALL_3_R(kEglInitialize, dpy, major, minor);
}

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy) {
    CALL_1_R(kEglTerminate, dpy);
}

EGLAPI const char* EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name) {
    CALL_2_R(kEglQueryString, dpy, name);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig* configs, EGLint config_size, EGLint* num_config) {
    CALL_4_R(kEglGetConfigs, dpy, configs, config_size, num_config);
}

EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config) {
    CALL_5_R(kEglChooseConfig, dpy, attrib_list, configs, config_size, num_config);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value) {
    CALL_4_R(kEglGetConfigAttrib, dpy, config, attribute, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLint api) {
    CALL_1_R(kEglBindAPI, api);
}

EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list) {
    CALL_4_R(kEglCreateContext, dpy, config, share_context, attrib_list);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    CALL_2_R(kEglDestroyContext, dpy, ctx);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list) {
    CALL_4_R(kEglCreateWindowSurface, dpy, config, win, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLAttrib* attrib_list) {
    CALL_4_R(kEglCreatePlatformWindowSurface, dpy, config, native_window, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLint* attrib_list) {
    CALL_4_R(kEglCreateWindowSurface, dpy, config, native_window, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list) {
    CALL_3_R(kEglCreatePbufferSurface, dpy, config, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint* attrib_list) {
    CALL_5_R(kEglCreatePbufferFromClientBuffer, dpy, buftype, buffer, config, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint* attrib_list) {
    CALL_4_R(kEglCreatePixmapSurface, dpy, config, pixmap, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void* native_pixmap, const EGLAttrib* attrib_list) {
    CALL_4_R(kEglCreatePlatformPixmapSurface, dpy, config, native_pixmap, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT(EGLDisplay dpy, EGLConfig config, void* native_pixmap, const EGLint* attrib_list) {
    return (EGLSurface)native_pixmap;
}

EGLAPI EGLImage EGLAPIENTRY eglCreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib* attrib_list) {
    CALL_5_R(kEglCreateImage, dpy, ctx, target, buffer, attrib_list);
}

EGLAPI EGLImage EGLAPIENTRY eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint* attrib_list) {
    CALL_5_R(kEglCreateImage, dpy, ctx, target, buffer, attrib_list);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImage(EGLDisplay dpy, EGLImage image) {
    CALL_2_R(kEglDestroyImage, dpy, image);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImageKHR(EGLDisplay dpy, EGLImage image) {
    CALL_2_R(kEglDestroyImage, dpy, image);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    CALL_2_R(kEglDestroySurface, dpy, surface);
}

EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    CALL_4_R(kEglMakeCurrent, dpy, draw, read, ctx);
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    CALL_2_R(kEglSwapBuffers, dpy, surface);
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    CALL_2_R(kEglSwapInterval, dpy, interval);
}

EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void) {
    CALL_0_R(kEglGetCurrentContext);
}

EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw) {
    CALL_1_R(kEglGetCurrentSurface, readdraw);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void) {
    CALL_0_R(kEglGetCurrentDisplay);
}

EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint* value) {
    CALL_4_R(kEglQuerySurface, dpy, surface, attribute, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint* value) {
    CALL_4_R(kEglQueryContext, dpy, ctx, attribute, value);
}

EGLAPI EGLint EGLAPIENTRY eglQueryAPI(void) {
    CALL_0_R(kEglQueryAPI);
}

EGLAPI EGLint EGLAPIENTRY eglGetError(void) {
    CALL_0_R(kEglGetError);
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseThread(void) {
    CALL_0_R(kEglReleaseThread);
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void) {
    CALL_0_R(kEglWaitGL);
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void) {
    CALL_0_R(kEglWaitClient);
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine) {
    CALL_1_R(kEglWaitNative, engine);
}

EGLAPI EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) {
    CALL_3_R(kEglCopyBuffers, dpy, surface, target);
}

EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    CALL_4_R(kEglSurfaceAttrib, dpy, surface, attribute, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    CALL_3_R(kEglBindTexImage, dpy, surface, buffer);
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    CALL_3_R(kEglReleaseTexImage, dpy, surface, buffer);
}

EGLAPI EGLSync EGLAPIENTRY eglCreateSync(EGLDisplay dpy, EGLint type, const EGLint* attrib_list) {
    CALL_3_R(kEglCreateSync, dpy, type, attrib_list);
}

EGLAPI EGLSync EGLAPIENTRY eglCreateSyncKHR(EGLDisplay dpy, EGLint type, const EGLint* attrib_list) {
    CALL_3_R(kEglCreateSync, dpy, type, attrib_list);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySync(EGLDisplay dpy, EGLSync sync) {
    CALL_2_R(kEglDestroySync, dpy, sync);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySyncKHR(EGLDisplay dpy, EGLSync sync) {
    CALL_2_R(kEglDestroySync, dpy, sync);
}

EGLAPI EGLint EGLAPIENTRY eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout) {
    CALL_5_R(kEglClientWaitSync, dpy, sync, flags, (uint32_t)timeout, (uint32_t)(timeout >> 32));
}

EGLAPI EGLint EGLAPIENTRY eglClientWaitSyncKHR(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout) {
    CALL_5_R(kEglClientWaitSync, dpy, sync, flags, (uint32_t)timeout, (uint32_t)(timeout >> 32));
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLint* value) {
    CALL_4_R(kEglGetSyncAttrib, dpy, sync, attribute, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttribKHR(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLint* value) {
    CALL_4_R(kEglGetSyncAttrib, dpy, sync, attribute, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags) {
    CALL_3_R(kEglWaitSync, dpy, sync, flags);
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitSyncKHR(EGLDisplay dpy, EGLSync sync, EGLint flags) {
    CALL_3_R(kEglWaitSync, dpy, sync, flags);
}

EGLAPI EGLint EGLAPIENTRY eglClientWaitSyncNV(EGLSync sync, EGLint flags, EGLTimeNV timeout) {
    return eglClientWaitSync(0, sync, flags, timeout);
}

EGLAPI EGLSync EGLAPIENTRY eglCreateFenceSyncNV(EGLDisplay dpy, EGLenum condition, const EGLint* attrib_list) {
    CALL_3_R(kEglCreateSync, dpy, condition, attrib_list);
}

EGLAPI EGLSync EGLAPIENTRY eglCreateSync64KHR(EGLDisplay dpy, EGLint type, const EGLAttribKHR* attrib_list) {
    CALL_3_R(kEglCreateSync, dpy, type, attrib_list);
}

EGLAPI EGLSync EGLAPIENTRY eglCreateStreamSyncNV(EGLDisplay dpy, EGLint type, const EGLint* attrib_list) {
    return 0;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySyncNV(EGLSync sync) {
    return eglDestroySync(0, sync);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttribNV(EGLSync sync, EGLint attribute, EGLint* value) {
    return eglGetSyncAttrib(0, sync, attribute, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglFenceNV(EGLSync sync) { return EGL_TRUE; }
EGLAPI EGLBoolean EGLAPIENTRY eglSignalSyncKHR(EGLDisplay dpy, EGLSync sync, EGLenum mode) { return EGL_TRUE; }
EGLAPI EGLBoolean EGLAPIENTRY eglSignalSyncNV(EGLSync sync, EGLenum mode) { return EGL_TRUE; }

#define EGL_STUB_BOOL(name, args) EGLAPI EGLBoolean EGLAPIENTRY name args { return EGL_FALSE; }
#define EGL_STUB_INT(name, args) EGLAPI EGLint EGLAPIENTRY name args { return 0; }
#define EGL_STUB_U64(name, args) EGLAPI EGLuint64KHR EGLAPIENTRY name args { return 0; }
#define EGL_STUB_PTR(ret, name, args) EGLAPI ret EGLAPIENTRY name args { return 0; }
#define EGL_STUB_VOID(name, args) EGLAPI void EGLAPIENTRY name args { }

EGL_STUB_BOOL(eglBindWaylandDisplayWL, (EGLDisplay dpy, void* display))
EGL_STUB_BOOL(eglClientSignalSyncEXT, (EGLDisplay dpy, EGLSync sync, const EGLAttrib* attrib_list))
EGL_STUB_BOOL(eglCompositorBindTexWindowEXT, (EGLint external_win_id))
EGL_STUB_BOOL(eglCompositorSetContextAttributesEXT, (EGLint external_ref_id, const EGLint* context_attributes, EGLint num_entries))
EGL_STUB_BOOL(eglCompositorSetContextListEXT, (const EGLint* external_ref_ids, EGLint num_entries))
EGL_STUB_BOOL(eglCompositorSetSizeEXT, (EGLint external_win_id, EGLint width, EGLint height))
EGL_STUB_BOOL(eglCompositorSetWindowAttributesEXT, (EGLint external_win_id, const EGLint* window_attributes, EGLint num_entries))
EGL_STUB_BOOL(eglCompositorSetWindowListEXT, (EGLint external_ref_id, const EGLint* external_win_ids, EGLint num_entries))
EGL_STUB_BOOL(eglCompositorSwapPolicyEXT, (EGLint external_win_id, EGLint policy))
EGL_STUB_PTR(EGLImage, eglCreateDRMImageMESA, (EGLDisplay dpy, const EGLint* attrib_list))
EGL_STUB_PTR(EGLSync, eglCreateStreamAttribKHR, (EGLDisplay dpy, const EGLAttrib* attrib_list))
EGL_STUB_PTR(EGLSync, eglCreateStreamFromFileDescriptorKHR, (EGLDisplay dpy, int file_descriptor))
EGL_STUB_PTR(EGLSync, eglCreateStreamKHR, (EGLDisplay dpy, const EGLint* attrib_list))
EGL_STUB_PTR(EGLSurface, eglCreateStreamProducerSurfaceKHR, (EGLDisplay dpy, EGLConfig config, EGLSync stream, const EGLint* attrib_list))
EGL_STUB_PTR(EGLClientBuffer, eglCreateWaylandBufferFromImageWL, (EGLDisplay dpy, EGLImage image))
EGL_STUB_INT(eglDebugMessageControlKHR, (void* callback, const EGLAttrib* attrib_list))
EGL_STUB_BOOL(eglDestroyDisplayEXT, (EGLDisplay dpy))
EGL_STUB_BOOL(eglDestroyStreamKHR, (EGLDisplay dpy, EGLSync stream))
EGL_STUB_BOOL(eglExportDMABUFImageMESA, (EGLDisplay dpy, EGLImage image, int* fds, EGLint* strides, EGLint* offsets))
EGL_STUB_BOOL(eglExportDMABUFImageQueryMESA, (EGLDisplay dpy, EGLImage image, int* fourcc, int* num_planes, uint64_t* modifiers))
EGL_STUB_BOOL(eglExportDRMImageMESA, (EGLDisplay dpy, EGLImage image, EGLint* name, EGLint* handle, EGLint* stride))
EGL_STUB_PTR(const char*, eglGetDisplayDriverConfig, (EGLDisplay dpy))
EGL_STUB_PTR(const char*, eglGetDisplayDriverName, (EGLDisplay dpy))
EGL_STUB_BOOL(eglGetOutputLayersEXT, (EGLDisplay dpy, const EGLAttrib* attrib_list, void** layers, EGLint max_layers, EGLint* num_layers))
EGL_STUB_BOOL(eglGetOutputPortsEXT, (EGLDisplay dpy, const EGLAttrib* attrib_list, void** ports, EGLint max_ports, EGLint* num_ports))
EGL_STUB_INT(eglGetStreamFileDescriptorKHR, (EGLDisplay dpy, EGLSync stream))
EGL_STUB_U64(eglGetSystemTimeFrequencyNV, (void))
EGL_STUB_U64(eglGetSystemTimeNV, (void))
EGL_STUB_BOOL(eglLabelObjectKHR, (EGLDisplay dpy, EGLenum objectType, void* object, void* label))
EGL_STUB_BOOL(eglLockSurfaceKHR, (EGLDisplay dpy, EGLSurface surface, const EGLint* attrib_list))
EGL_STUB_BOOL(eglOutputLayerAttribEXT, (EGLDisplay dpy, void* layer, EGLint attribute, EGLAttrib value))
EGL_STUB_BOOL(eglOutputPortAttribEXT, (EGLDisplay dpy, void* port, EGLint attribute, EGLAttrib value))
EGL_STUB_BOOL(eglPostSubBufferNV, (EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height))
EGL_STUB_BOOL(eglQueryDebugKHR, (EGLint attribute, EGLAttrib* value))
EGL_STUB_BOOL(eglQueryDeviceAttribEXT, (void* device, EGLint attribute, EGLAttrib* value))
EGL_STUB_BOOL(eglQueryDeviceBinaryEXT, (void* device, EGLint name, EGLint max_size, void* value, EGLint* size))
EGL_STUB_PTR(const char*, eglQueryDeviceStringEXT, (void* device, EGLint name))
EGL_STUB_BOOL(eglQueryDevicesEXT, (EGLint max_devices, void** devices, EGLint* num_devices))
EGL_STUB_BOOL(eglQueryDisplayAttribEXT, (EGLDisplay dpy, EGLint attribute, EGLAttrib* value))
EGL_STUB_BOOL(eglQueryDisplayAttribKHR, (EGLDisplay dpy, EGLint attribute, EGLAttrib* value))
EGL_STUB_BOOL(eglQueryDisplayAttribNV, (EGLDisplay dpy, EGLint attribute, EGLAttrib* value))
EGL_STUB_BOOL(eglQueryDmaBufFormatsEXT, (EGLDisplay dpy, EGLint max_formats, EGLint* formats, EGLint* num_formats))
EGL_STUB_BOOL(eglQueryDmaBufModifiersEXT, (EGLDisplay dpy, EGLint format, EGLint max_modifiers, uint64_t* modifiers, EGLBoolean* external_only, EGLint* num_modifiers))
EGL_STUB_BOOL(eglQueryNativeDisplayNV, (EGLDisplay dpy, EGLNativeDisplayType* display_id))
EGL_STUB_BOOL(eglQueryNativePixmapNV, (EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType* pixmap))
EGL_STUB_BOOL(eglQueryNativeWindowNV, (EGLDisplay dpy, EGLSurface surface, EGLNativeWindowType* window))
EGL_STUB_BOOL(eglQueryOutputLayerAttribEXT, (EGLDisplay dpy, void* layer, EGLint attribute, EGLAttrib* value))
EGL_STUB_PTR(const char*, eglQueryOutputLayerStringEXT, (EGLDisplay dpy, void* layer, EGLint name))
EGL_STUB_BOOL(eglQueryOutputPortAttribEXT, (EGLDisplay dpy, void* port, EGLint attribute, EGLAttrib* value))
EGL_STUB_PTR(const char*, eglQueryOutputPortStringEXT, (EGLDisplay dpy, void* port, EGLint name))
EGL_STUB_BOOL(eglQueryStreamAttribKHR, (EGLDisplay dpy, EGLSync stream, EGLenum attribute, EGLAttrib* value))
EGL_STUB_BOOL(eglQueryStreamConsumerEventNV, (EGLDisplay dpy, EGLSync stream, EGLTime timeout, EGLenum* event, EGLAttrib* aux))
EGL_STUB_BOOL(eglQueryStreamKHR, (EGLDisplay dpy, EGLSync stream, EGLenum attribute, EGLint* value))
EGL_STUB_BOOL(eglQueryStreamMetadataNV, (EGLDisplay dpy, EGLSync stream, EGLenum name, EGLint n, EGLint offset, EGLint size, void* data))
EGL_STUB_BOOL(eglQueryStreamTimeKHR, (EGLDisplay dpy, EGLSync stream, EGLenum attribute, EGLTimeKHR* value))
EGL_STUB_BOOL(eglQueryStreamu64KHR, (EGLDisplay dpy, EGLSync stream, EGLenum attribute, EGLuint64KHR* value))
EGL_STUB_BOOL(eglQuerySupportedCompressionRatesEXT, (EGLDisplay dpy, EGLConfig config, const EGLAttrib* attrib_list, EGLint* rates, EGLint rate_size, EGLint* num_rates))
EGL_STUB_BOOL(eglQuerySurface64KHR, (EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLAttribKHR* value))
EGL_STUB_BOOL(eglQueryWaylandBufferWL, (EGLDisplay dpy, EGLClientBuffer buffer, EGLint attribute, EGLint* value))
EGL_STUB_BOOL(eglResetStreamNV, (EGLDisplay dpy, EGLSync stream))
EGL_STUB_BOOL(eglSetDamageRegionKHR, (EGLDisplay dpy, EGLSurface surface, EGLint* rects, EGLint n_rects))
EGL_STUB_BOOL(eglSetStreamAttribKHR, (EGLDisplay dpy, EGLSync stream, EGLenum attribute, EGLAttrib value))
EGL_STUB_BOOL(eglSetStreamMetadataNV, (EGLDisplay dpy, EGLSync stream, EGLint n, EGLint offset, EGLint size, const void* data))
EGL_STUB_BOOL(eglStreamAcquireImageNV, (EGLDisplay dpy, EGLSync stream, EGLImage* image, EGLSync sync))
EGL_STUB_BOOL(eglStreamAttribKHR, (EGLDisplay dpy, EGLSync stream, EGLenum attribute, EGLAttrib value))
EGL_STUB_BOOL(eglStreamConsumerAcquireAttribKHR, (EGLDisplay dpy, EGLSync stream, const EGLAttrib* attrib_list))
EGL_STUB_BOOL(eglStreamConsumerAcquireKHR, (EGLDisplay dpy, EGLSync stream))
EGL_STUB_BOOL(eglStreamConsumerGLTextureExternalAttribsNV, (EGLDisplay dpy, EGLSync stream, const EGLAttrib* attrib_list))
EGL_STUB_BOOL(eglStreamConsumerGLTextureExternalKHR, (EGLDisplay dpy, EGLSync stream))
EGL_STUB_BOOL(eglStreamConsumerOutputEXT, (EGLDisplay dpy, EGLSync stream, void* layer))
EGL_STUB_BOOL(eglStreamConsumerReleaseAttribKHR, (EGLDisplay dpy, EGLSync stream, const EGLAttrib* attrib_list))
EGL_STUB_BOOL(eglStreamConsumerReleaseKHR, (EGLDisplay dpy, EGLSync stream))
EGL_STUB_BOOL(eglStreamFlushNV, (EGLDisplay dpy, EGLSync stream))
EGL_STUB_BOOL(eglStreamImageConsumerConnectNV, (EGLDisplay dpy, EGLSync stream, EGLint num_modifiers, const EGLuint64KHR* modifiers, const EGLAttrib* attrib_list))
EGL_STUB_BOOL(eglStreamReleaseImageNV, (EGLDisplay dpy, EGLSync stream, EGLImage image, EGLSync sync))
EGL_STUB_BOOL(eglSwapBuffersWithDamageEXT, (EGLDisplay dpy, EGLSurface surface, EGLint* rects, EGLint n_rects))
EGL_STUB_BOOL(eglSwapBuffersWithDamageKHR, (EGLDisplay dpy, EGLSurface surface, EGLint* rects, EGLint n_rects))
EGL_STUB_BOOL(eglUnbindWaylandDisplayWL, (EGLDisplay dpy, void* display))
EGL_STUB_BOOL(eglUnlockSurfaceKHR, (EGLDisplay dpy, EGLSurface surface))
EGL_STUB_BOOL(eglUnsignalSyncEXT, (EGLDisplay dpy, EGLSync sync, const EGLAttrib* attrib_list))

typedef struct {
    const char* name;
    void* proc;
} EglProc;

#define EGL_PROC(name) { #name, (void*)name }

static const EglProc localEglProcs[] = {
    EGL_PROC(eglBindAPI),
    EGL_PROC(eglBindTexImage),
    EGL_PROC(eglChooseConfig),
    EGL_PROC(eglClientWaitSync),
    EGL_PROC(eglCopyBuffers),
    EGL_PROC(eglCreateContext),
    EGL_PROC(eglCreateImage),
    EGL_PROC(eglCreatePbufferFromClientBuffer),
    EGL_PROC(eglCreatePbufferSurface),
    EGL_PROC(eglCreatePixmapSurface),
    EGL_PROC(eglCreatePlatformPixmapSurface),
    EGL_PROC(eglCreatePlatformWindowSurface),
    EGL_PROC(eglCreateSync),
    EGL_PROC(eglCreateWindowSurface),
    EGL_PROC(eglDestroyContext),
    EGL_PROC(eglDestroyImage),
    EGL_PROC(eglDestroySurface),
    EGL_PROC(eglDestroySync),
    EGL_PROC(eglGetConfigAttrib),
    EGL_PROC(eglGetConfigs),
    EGL_PROC(eglGetCurrentContext),
    EGL_PROC(eglGetCurrentDisplay),
    EGL_PROC(eglGetCurrentSurface),
    EGL_PROC(eglGetDisplay),
    EGL_PROC(eglGetError),
    EGL_PROC(eglGetPlatformDisplay),
    EGL_PROC(eglGetProcAddress),
    EGL_PROC(eglGetSyncAttrib),
    EGL_PROC(eglInitialize),
    EGL_PROC(eglMakeCurrent),
    EGL_PROC(eglQueryAPI),
    EGL_PROC(eglQueryContext),
    EGL_PROC(eglQueryString),
    EGL_PROC(eglQuerySurface),
    EGL_PROC(eglReleaseTexImage),
    EGL_PROC(eglReleaseThread),
    EGL_PROC(eglSurfaceAttrib),
    EGL_PROC(eglSwapBuffers),
    EGL_PROC(eglSwapInterval),
    EGL_PROC(eglTerminate),
    EGL_PROC(eglWaitClient),
    EGL_PROC(eglWaitGL),
    EGL_PROC(eglWaitNative),
    EGL_PROC(eglWaitSync),
    EGL_PROC(eglBindWaylandDisplayWL),
    EGL_PROC(eglClientSignalSyncEXT),
    EGL_PROC(eglClientWaitSyncKHR),
    EGL_PROC(eglClientWaitSyncNV),
    EGL_PROC(eglCompositorBindTexWindowEXT),
    EGL_PROC(eglCompositorSetContextAttributesEXT),
    EGL_PROC(eglCompositorSetContextListEXT),
    EGL_PROC(eglCompositorSetSizeEXT),
    EGL_PROC(eglCompositorSetWindowAttributesEXT),
    EGL_PROC(eglCompositorSetWindowListEXT),
    EGL_PROC(eglCompositorSwapPolicyEXT),
    EGL_PROC(eglCreateDRMImageMESA),
    EGL_PROC(eglCreateFenceSyncNV),
    EGL_PROC(eglCreateImageKHR),
    EGL_PROC(eglCreatePlatformPixmapSurfaceEXT),
    EGL_PROC(eglCreatePlatformWindowSurfaceEXT),
    EGL_PROC(eglCreateStreamAttribKHR),
    EGL_PROC(eglCreateStreamFromFileDescriptorKHR),
    EGL_PROC(eglCreateStreamKHR),
    EGL_PROC(eglCreateStreamProducerSurfaceKHR),
    EGL_PROC(eglCreateStreamSyncNV),
    EGL_PROC(eglCreateSync64KHR),
    EGL_PROC(eglCreateSyncKHR),
    EGL_PROC(eglCreateWaylandBufferFromImageWL),
    EGL_PROC(eglDebugMessageControlKHR),
    EGL_PROC(eglDestroyDisplayEXT),
    EGL_PROC(eglDestroyImageKHR),
    EGL_PROC(eglDestroyStreamKHR),
    EGL_PROC(eglDestroySyncKHR),
    EGL_PROC(eglDestroySyncNV),
    EGL_PROC(eglExportDMABUFImageMESA),
    EGL_PROC(eglExportDMABUFImageQueryMESA),
    EGL_PROC(eglExportDRMImageMESA),
    EGL_PROC(eglFenceNV),
    EGL_PROC(eglGetDisplayDriverConfig),
    EGL_PROC(eglGetDisplayDriverName),
    EGL_PROC(eglGetOutputLayersEXT),
    EGL_PROC(eglGetOutputPortsEXT),
    EGL_PROC(eglGetPlatformDisplayEXT),
    EGL_PROC(eglGetStreamFileDescriptorKHR),
    EGL_PROC(eglGetSyncAttribKHR),
    EGL_PROC(eglGetSyncAttribNV),
    EGL_PROC(eglGetSystemTimeFrequencyNV),
    EGL_PROC(eglGetSystemTimeNV),
    EGL_PROC(eglLabelObjectKHR),
    EGL_PROC(eglLockSurfaceKHR),
    EGL_PROC(eglOutputLayerAttribEXT),
    EGL_PROC(eglOutputPortAttribEXT),
    EGL_PROC(eglPostSubBufferNV),
    EGL_PROC(eglQueryDebugKHR),
    EGL_PROC(eglQueryDeviceAttribEXT),
    EGL_PROC(eglQueryDeviceBinaryEXT),
    EGL_PROC(eglQueryDeviceStringEXT),
    EGL_PROC(eglQueryDevicesEXT),
    EGL_PROC(eglQueryDisplayAttribEXT),
    EGL_PROC(eglQueryDisplayAttribKHR),
    EGL_PROC(eglQueryDisplayAttribNV),
    EGL_PROC(eglQueryDmaBufFormatsEXT),
    EGL_PROC(eglQueryDmaBufModifiersEXT),
    EGL_PROC(eglQueryNativeDisplayNV),
    EGL_PROC(eglQueryNativePixmapNV),
    EGL_PROC(eglQueryNativeWindowNV),
    EGL_PROC(eglQueryOutputLayerAttribEXT),
    EGL_PROC(eglQueryOutputLayerStringEXT),
    EGL_PROC(eglQueryOutputPortAttribEXT),
    EGL_PROC(eglQueryOutputPortStringEXT),
    EGL_PROC(eglQueryStreamAttribKHR),
    EGL_PROC(eglQueryStreamConsumerEventNV),
    EGL_PROC(eglQueryStreamKHR),
    EGL_PROC(eglQueryStreamMetadataNV),
    EGL_PROC(eglQueryStreamTimeKHR),
    EGL_PROC(eglQueryStreamu64KHR),
    EGL_PROC(eglQuerySupportedCompressionRatesEXT),
    EGL_PROC(eglQuerySurface64KHR),
    EGL_PROC(eglQueryWaylandBufferWL),
    EGL_PROC(eglResetStreamNV),
    EGL_PROC(eglSetDamageRegionKHR),
    EGL_PROC(eglSetStreamAttribKHR),
    EGL_PROC(eglSetStreamMetadataNV),
    EGL_PROC(eglSignalSyncKHR),
    EGL_PROC(eglSignalSyncNV),
    EGL_PROC(eglStreamAcquireImageNV),
    EGL_PROC(eglStreamAttribKHR),
    EGL_PROC(eglStreamConsumerAcquireAttribKHR),
    EGL_PROC(eglStreamConsumerAcquireKHR),
    EGL_PROC(eglStreamConsumerGLTextureExternalAttribsNV),
    EGL_PROC(eglStreamConsumerGLTextureExternalKHR),
    EGL_PROC(eglStreamConsumerOutputEXT),
    EGL_PROC(eglStreamConsumerReleaseAttribKHR),
    EGL_PROC(eglStreamConsumerReleaseKHR),
    EGL_PROC(eglStreamFlushNV),
    EGL_PROC(eglStreamImageConsumerConnectNV),
    EGL_PROC(eglStreamReleaseImageNV),
    EGL_PROC(eglSwapBuffersWithDamageEXT),
    EGL_PROC(eglSwapBuffersWithDamageKHR),
    EGL_PROC(eglUnbindWaylandDisplayWL),
    EGL_PROC(eglUnlockSurfaceKHR),
    EGL_PROC(eglUnsignalSyncEXT),
    EGL_PROC(eglWaitSyncKHR),
};

static int eglLogProcAddress(void) {
    return getenv && getenv("BOXEDWINE_EGL_LOG");
}

static int eglStrcmp(const char* a, const char* b) {
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static int eglIsGlProcAddressAvailable(const char* procname) {
    uintptr_t result;
    if (!procname || procname[0] != 'g' || procname[1] != 'l') {
        return 0;
    }
    if (eglStrcmp(procname, "glBeginQueryEXT") == 0
            || eglStrcmp(procname, "glDeleteQueriesEXT") == 0
            || eglStrcmp(procname, "glEndQueryEXT") == 0
            || eglStrcmp(procname, "glGenQueriesEXT") == 0
            || eglStrcmp(procname, "glGetQueryivEXT") == 0
            || eglStrcmp(procname, "glGetQueryObjecti64vEXT") == 0
            || eglStrcmp(procname, "glGetQueryObjectivEXT") == 0
            || eglStrcmp(procname, "glGetQueryObjectui64vEXT") == 0
            || eglStrcmp(procname, "glGetQueryObjectuivEXT") == 0
            || eglStrcmp(procname, "glIsQueryEXT") == 0
            || eglStrcmp(procname, "glQueryCounterEXT") == 0) {
        return 0;
    }
    // This helper has its own C frame, so provide an argument and a dummy
    // return-address slot. The callback index belongs after the interrupt.
    __asm__ volatile("push %2\n\tpush $0\n\tint $0x99\n\t.long %c1\n\taddl $8, %%esp":"=a"(result):"i"(kGlProcAddressAvailable), "g"(procname):"memory", "cc");
    return result != 0;
}

static void eglWriteString(const char* s) {
    unsigned int len = 0;
    while (s && s[len]) {
        ++len;
    }
    if (len) {
        __asm__ volatile("int $0x80" : : "a"(4), "b"(2), "c"(s), "d"(len) : "memory");
    }
}

static void eglLogProcAddressResult(const char* procname, const char* source) {
    eglWriteString("boxedwine libEGL: eglGetProcAddress(");
    eglWriteString(procname ? procname : "<null>");
    eglWriteString(") -> ");
    eglWriteString(source);
    eglWriteString("\n");
}

static void* eglGetGlProcAddress(const char* procname, const char** source) {
    static void* libgl;

    if (!procname || procname[0] != 'g' || procname[1] != 'l') {
        return 0;
    }
    if (!libgl) {
        libgl = dlopen("libGL.so.1", RTLD_NOW | RTLD_GLOBAL);
        if (!libgl) {
            libgl = dlopen("/lib/libGL.so.1", RTLD_NOW | RTLD_GLOBAL);
        }
    }
    if (!libgl) {
        return 0;
    }
    if (!eglIsGlProcAddressAvailable(procname)) {
        *source = "unsupported";
        return 0;
    }
    *source = "libGL.so.1";
    return dlsym(libgl, procname);
}

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char* procname) {
    const char* source = "local";
    void* result = 0;
    unsigned int i;
    for (i = 0; i < sizeof(localEglProcs) / sizeof(localEglProcs[0]); ++i) {
        if (eglStrcmp(procname, localEglProcs[i].name) == 0) {
            result = localEglProcs[i].proc;
            break;
        }
    }
    if (result) {
        if (eglLogProcAddress()) {
            eglLogProcAddressResult(procname, source);
        }
        return (__eglMustCastToProperFunctionPointerType)result;
    }

    result = eglGetGlProcAddress(procname, &source);
    if (result) {
        if (eglLogProcAddress()) {
            eglLogProcAddressResult(procname, source);
        }
        return (__eglMustCastToProperFunctionPointerType)result;
    }

    if (eglLogProcAddress()) {
        eglLogProcAddressResult(procname, "missing");
    }
    return (__eglMustCastToProperFunctionPointerType)result;
}
