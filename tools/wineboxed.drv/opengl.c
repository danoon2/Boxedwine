#if 0
MAKE_DEP_UNIX
#endif
#include "wineboxed.h"
#include "wine/wgl.h"
#include "wine/wgl_driver.h"
#include "wine/debug.h"

#include <dlfcn.h>

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

// 21 Apr 23, 2020 Wine 5.7 opengl32: Make wgl driver entry points WINAPI.
// 22 Nov 23, 2022 Wine 7.22 win32u: Don't use CDECL for __wine_get_wgl_driver. 
// 23 Dec 6, 2022 Wine 8.0-rc1 opengl32: Use default calling convention for WGL driver entry points. 
// 24 Apr 24, 2024 Wine 9.8 opengl32: Implement wglDescribePixelFormat using new driver API get_pixel_formats.

#if BOXED_WINE_VERSION >= 7150
#define SetLastError RtlSetLastWin32Error
#endif

#if BOXED_WINE_VERSION >= 7120
#define WindowFromDC NtUserWindowFromDC
#endif

#if WINE_WGL_DRIVER_VERSION >= 23
#define BGLAPI
#elif WINE_WGL_DRIVER_VERSION >= 21
#define BGLAPI WINAPI
#else
#define BGLAPI
#endif

static BOOL BGLAPI boxeddrv_wglCopyContext(struct wgl_context* src, struct wgl_context* dst, UINT mask) {
    int result;
    CALL_3(BOXED_GL_COPY_CONTEXT, src, dst, mask);
    TRACE("boxeddrv_wglCopyContext src=%p dst=%p mask=%X result=%d\n", src, dst, mask, result);
    return (BOOL)result;
}

static struct wgl_context* BGLAPI boxeddrv_wglCreateContext(HDC hdc) {
    struct wgl_context* result;
    CALL_5(BOXED_GL_CREATE_CONTEXT, WindowFromDC(hdc), 0, 0, 0, 0);
    TRACE("boxeddrv_wglCreateContext hdc=%X result=%p\n", (int)hdc, result);
    return result;
}

static BOOL BGLAPI boxeddrv_wglDeleteContext(struct wgl_context* context) {
    TRACE("boxeddrv_wglDeleteContext context=%p\n", context);
    CALL_NORETURN_1(BOXED_GL_DELETE_CONTEXT, context);
    return TRUE;
}

static int BGLAPI boxeddrv_wglDescribePixelFormat(HDC hdc, int fmt, UINT size, PIXELFORMATDESCRIPTOR* descr) {
    int result;
    CALL_4(BOXED_GL_DESCRIBE_PIXEL_FORMAT, hdc, fmt, size, descr);
    TRACE("boxeddrv_wglDescribePixelFormat hdc=%X fmt=%d size=%d descr=%p result=%d\n", (int)hdc, fmt, size, descr, result);
    return result;
}

static int BGLAPI boxeddrv_wglGetPixelFormat(HDC hdc) {
    int result;
    CALL_1(BOXED_GL_GET_PIXEL_FORMAT, WindowFromDC(hdc));
    TRACE("boxeddrv_wglGetPixelFormat hdc=%X result=%d\n", (int)hdc, result);
    return result;
}

static struct wgl_context* boxeddrv_wglCreateContextAttribsARB(HDC hdc, struct wgl_context* share_context, const int* attrib_list);
static void* glModule;

static PROC BGLAPI boxeddrv_wglGetProcAddress(const char* proc) {
    TRACE("boxeddrv_wglGetProcAddress %s\n", proc);
    if (!strcmp(proc, "wglCreateContextAttribsARB"))
        return (PROC)boxeddrv_wglCreateContextAttribsARB;

    if (!glModule) {
        glModule = dlopen("/lib/libGL.so.1", RTLD_LAZY);
    }
    if (glModule) {
        int result = 0;
        PROC pfn;

        CALL_1(BOXED_GL_GET_PROC_ADDRESS, proc);
        if (!result) {
            TRACE("    %s not found\n", proc);
            return 0;
        }
        pfn = dlsym(glModule, proc);
        TRACE("glModule=%p result=%p\n", glModule, pfn);
        return pfn;
    }
    TRACE("could not find /lib/libGL.so.1\n");
    return NULL;
}

static BOOL BGLAPI boxeddrv_wglMakeCurrent(HDC hdc, struct wgl_context* context) {
    int result;
    CALL_2(BOXED_GL_MAKE_CURRENT, WindowFromDC(hdc), context);
    TRACE("boxeddrv_wglMakeCurrent hdc=%X context=%p result=%d\n", (int)hdc, context, result);
    return (BOOL)result;
}

static BOOL BGLAPI boxeddrv_wglSetPixelFormat(HDC hdc, int fmt, const PIXELFORMATDESCRIPTOR* descr) {
    int result;
    CALL_3(BOXED_GL_SET_PIXEL_FORMAT, WindowFromDC(hdc), fmt, descr);
    TRACE("boxeddrv_wglSetPixelFormat hdc=%X fmt=%d descr=%p result=%d\n", (int)hdc, fmt, descr, result);
    return (BOOL)result;
}

static BOOL BGLAPI boxeddrv_wglShareLists(struct wgl_context* org, struct wgl_context* dest) {
    int result;
    CALL_2(BOXED_GL_SHARE_LISTS, org, dest);
    TRACE("boxeddrv_wglShareLists org=%p dest=%p result=%d\n", org, dest, result);
    return (BOOL)result;
}

static BOOL BGLAPI boxeddrv_wglSwapBuffers(HDC hdc) {
    int result;
    CALL_1(BOXED_GL_SWAP_BUFFERS, hdc);
    return (BOOL)result;
}

static struct wgl_context* boxeddrv_wglCreateContextAttribsARB(HDC hdc, struct wgl_context* share_context, const int* attrib_list)
{
    const int* iptr;
    int major = 1, minor = 0, profile = WGL_CONTEXT_CORE_PROFILE_BIT_ARB, flags = 0;
    struct wgl_context* result;

    TRACE("boxeddrv_wglCreateContextAttribsARB hdc=%p share_context=%p attrib_list=%p\n", hdc, share_context, attrib_list);

    for (iptr = attrib_list; iptr && *iptr; iptr += 2)
    {
        int attr = iptr[0];
        int value = iptr[1];

        TRACE("attribute %d.%d\n", attr, value);

        switch (attr)
        {
        case WGL_CONTEXT_MAJOR_VERSION_ARB:
            major = value;
            break;

        case WGL_CONTEXT_MINOR_VERSION_ARB:
            minor = value;
            break;

        case WGL_CONTEXT_LAYER_PLANE_ARB:
            WARN("WGL_CONTEXT_LAYER_PLANE_ARB attribute ignored\n");
            break;

        case WGL_CONTEXT_FLAGS_ARB:
            flags = value;
            if (flags & ~WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB)
                WARN("WGL_CONTEXT_FLAGS_ARB attributes %#x ignored\n", flags & ~WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB);
            break;

        case WGL_CONTEXT_PROFILE_MASK_ARB:
            if (value != WGL_CONTEXT_CORE_PROFILE_BIT_ARB &&
                value != WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB)
            {
                WARN("WGL_CONTEXT_PROFILE_MASK_ARB bits %#x invalid\n", value);
                SetLastError(ERROR_INVALID_PROFILE_ARB);
                return NULL;
            }
            profile = value;
            break;
        default:
            WARN("Unknown attribute %d.%d\n", attr, value);
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }
    }

    CALL_5(BOXED_GL_CREATE_CONTEXT, WindowFromDC(hdc), major, minor, profile, flags);
    return result;
}

static void BGLAPI boxeddrv_get_pixel_formats(struct wgl_pixel_format* formats, UINT max_formats, UINT* num_formats, UINT* num_onscreen_formats) {
    int result;
    CALL_NORETURN_4(BOXED_GL_PIXEL_FORMATS, formats, max_formats, num_formats, num_onscreen_formats);
    TRACE("boxeddrv_get_pixel_formats formats=%X max_formats=%d num_formats=%d num_onscreen_formats=%p\n", formats, max_formats, num_formats, num_onscreen_formats);
}

static struct opengl_funcs opengl_funcs =
{
    {
        boxeddrv_wglCopyContext,          /* p_wglCopyContext */
        boxeddrv_wglCreateContext,        /* p_wglCreateContext */
        boxeddrv_wglDeleteContext,        /* p_wglDeleteContext */
        boxeddrv_wglDescribePixelFormat,  /* p_wglDescribePixelFormat */
        boxeddrv_wglGetPixelFormat,       /* p_wglGetPixelFormat */
        boxeddrv_wglGetProcAddress,       /* p_wglGetProcAddress */
        boxeddrv_wglMakeCurrent,          /* p_wglMakeCurrent */
        boxeddrv_wglSetPixelFormat,       /* p_wglSetPixelFormat */
        boxeddrv_wglShareLists,           /* p_wglShareLists */
        boxeddrv_wglSwapBuffers,          /* p_wglSwapBuffers */
#if WINE_WGL_DRIVER_VERSION >= 24
        boxeddrv_get_pixel_formats
#endif
    }
};

#define USE_GL_FUNC(name) #name,
static const char* opengl_func_names[] = { ALL_WGL_FUNCS };
#undef USE_GL_FUNC

int initOpengl(void) {
    static int init_done;
    static void* opengl_handle;

    char buffer[200];
    unsigned int i;

    if (init_done) return (opengl_handle != NULL);
    init_done = 1;

    /* No need to load any other libraries as according to the ABI, libGL should be self-sufficient
       and include all dependencies */
    opengl_handle = dlopen("libGL.so.1", RTLD_NOW | RTLD_GLOBAL);
    if (opengl_handle == NULL)
    {
        ERR("Failed to load libGL: %s\n", buffer);
        ERR("OpenGL support is disabled.\n");
        return FALSE;
    }

    for (i = 0; i < sizeof(opengl_func_names) / sizeof(opengl_func_names[0]); i++)
    {
        if (!(((void**)&opengl_funcs.gl)[i] = dlsym(opengl_handle, opengl_func_names[i])))
        {
            ERR("%s not found in libGL, disabling OpenGL.\n", opengl_func_names[i]);
            goto failed;
        }
    }
    return TRUE;

failed:
    dlclose(opengl_handle);
    opengl_handle = NULL;
    return FALSE;
}

#if WINE_GDI_DRIVER_VERSION >= 75
#if WINE_WGL_DRIVER_VERSION >= 22
struct opengl_funcs* boxeddrv_wine_get_wgl_driver(UINT version)
#else
struct opengl_funcs* CDECL boxeddrv_wine_get_wgl_driver(UINT version)
#endif
#else
struct opengl_funcs* CDECL boxeddrv_wine_get_wgl_driver(PHYSDEV hdc, UINT version)
#endif
{
    if (version != WINE_WGL_DRIVER_VERSION)
    {
        ERR("version mismatch, opengl32 wants %u but boxeddrv has %u\n", version, WINE_WGL_DRIVER_VERSION);
        return NULL;
    }

    if (initOpengl())
        return &opengl_funcs;
    return NULL;
}