#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL
#include "platformOpenGL.h"
#include <Windows.h>
#include GLH
#include "../source/x11/x11.h"
#include "../source/opengl/glcommon.h"

BHashTable<U32, GLPixelFormatPtr> PlatformOpenGL::formatsById;
std::vector<GLPixelFormatPtr> PlatformOpenGL::formats;
bool PlatformOpenGL::initialized;
bool PlatformOpenGL::hardwareListLoaded;

static BHashTable<U32, HGLRC> contexts;
static std::atomic_int nextContextId = 1;
static BOXEDWINE_MUTEX contextMutex;

static BHashTable<U32, HWND> nativeWindowHandles;
static BOXEDWINE_MUTEX windowMutex;

LRESULT CALLBACK dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
    return DefWindowProc(hwnd, umsg, wp, lp);
}

#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_DRAW_TO_BITMAP_ARB 0x2002
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NEED_PALETTE_ARB 0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB 0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB 0x2006
#define WGL_SWAP_METHOD_ARB 0x2007
#define WGL_NUMBER_OVERLAYS_ARB 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB 0x2009
#define WGL_TRANSPARENT_ARB 0x200A
#define WGL_SHARE_DEPTH_ARB 0x200C
#define WGL_SHARE_STENCIL_ARB 0x200D
#define WGL_SHARE_ACCUM_ARB 0x200E
#define WGL_SUPPORT_GDI_ARB 0x200F
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_STEREO_ARB 0x2012
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201A
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_ALPHA_SHIFT_ARB 0x201C
#define WGL_ACCUM_BITS_ARB 0x201D
#define WGL_ACCUM_RED_BITS_ARB 0x201E
#define WGL_ACCUM_GREEN_BITS_ARB 0x201F
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_GENERIC_ACCELERATION_ARB 0x2026
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_SWAP_EXCHANGE_ARB 0x2028
#define WGL_SWAP_COPY_ARB 0x2029
#define WGL_SWAP_UNDEFINED_ARB 0x202A
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_TYPE_COLORINDEX_ARB 0x202C
#define WGL_TRANSPARENT_RED_VALUE_ARB 0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B

#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042

#define WGL_DRAW_TO_PBUFFER_ARB 0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB 0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB 0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB 0x2030

typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
typedef BOOL(WINAPI* PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int* piValues);
typedef BOOL(WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);

static PFNWGLCREATECONTEXTATTRIBSARBPROC pfnWglCreateContextAttribsARB;
static PFNWGLCHOOSEPIXELFORMATARBPROC pfnWglChoosePixelFormat;

bool queryOpenGL(BHashTable<U32, GLPixelFormatPtr>& formatsById, std::vector<GLPixelFormatPtr>& glformats) {
    WNDCLASS tmpClass;
    memset(&tmpClass, 0, sizeof(WNDCLASS));
    tmpClass.style = CS_OWNDC;
    tmpClass.hInstance = GetModuleHandle(NULL);
    tmpClass.lpfnWndProc = dummyWndProc;
    tmpClass.lpszClassName = "boxedwine";
    RegisterClass(&tmpClass);

    HWND hwnd = CreateWindow("boxedwine", "boxedwine", WS_POPUP | WS_CLIPCHILDREN, 0, 0, 32, 32, 0, 0, tmpClass.hInstance, 0);

    if (!hwnd) {
        kpanic("queryOpenGL failed to create opengl window");
    }

    HDC hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.cColorBits = 32;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;

    int format = ChoosePixelFormat(hdc, &pfd);
    if (format != 0) {
        SetPixelFormat(hdc, format, &pfd);
    }

    HGLRC hrc = wglCreateContext(hdc);
    if (hrc) {
        HGLRC oldrc = wglGetCurrentContext();
        HDC oldhdc = wglGetCurrentDC();

        wglMakeCurrent(hdc, hrc);

        pfnWglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        PFNWGLGETEXTENSIONSSTRINGARBPROC pfnWglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
        PFNWGLGETPIXELFORMATATTRIBIVARBPROC pfnWglGetPixelFormatAttribiv = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
        pfnWglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

        bool arbPixelFormat = false;
        bool arbMultisample = false;
        bool mHasHardwareGamma = false;
        bool arbPBuffer = false;
        // check for pixel format and multisampling support
        if (pfnWglGetExtensionsStringARB) {
            std::istringstream wglexts(pfnWglGetExtensionsStringARB(hdc));
            std::string ext;
            while (wglexts >> ext) {
                if (ext == "WGL_ARB_pixel_format") {
                    arbPixelFormat = true;
                } else if (ext == "WGL_ARB_multisample") {
                    arbMultisample = true;
                }  else if (ext == "WGL_ARB_pbuffer") {
                    arbPBuffer = true;
                }
            }
        }

        if (arbPixelFormat) {
            int formats[1024] = { 0 };
            unsigned int count = 0;            

            static const int attributes[] = {
                WGL_DRAW_TO_WINDOW_ARB,
                WGL_DRAW_TO_BITMAP_ARB,
                WGL_STEREO_ARB,
                WGL_PIXEL_TYPE_ARB,
                WGL_COLOR_BITS_ARB,
                WGL_RED_BITS_ARB,
                WGL_RED_SHIFT_ARB,
                WGL_GREEN_BITS_ARB,
                WGL_GREEN_SHIFT_ARB,
                WGL_BLUE_BITS_ARB,
                WGL_BLUE_SHIFT_ARB,
                WGL_ALPHA_BITS_ARB,
                WGL_ALPHA_SHIFT_ARB,
                WGL_ACCUM_BITS_ARB,
                WGL_ACCUM_RED_BITS_ARB,
                WGL_ACCUM_GREEN_BITS_ARB,
                WGL_ACCUM_BLUE_BITS_ARB,
                WGL_ACCUM_ALPHA_BITS_ARB,
                WGL_DEPTH_BITS_ARB,
                WGL_STENCIL_BITS_ARB,
                WGL_AUX_BUFFERS_ARB,
                WGL_DOUBLE_BUFFER_ARB,               
                0
            };
            const int attributesPBuffer[] = {
                WGL_DRAW_TO_PBUFFER_ARB,
                WGL_MAX_PBUFFER_WIDTH_ARB,
                WGL_MAX_PBUFFER_HEIGHT_ARB,
                WGL_MAX_PBUFFER_PIXELS_ARB
            };
            const int attributesSamples[] = {
                WGL_SAMPLE_BUFFERS_ARB,
                WGL_SAMPLES_ARB
            };

            int results[22];
            static const int openGlAttributes[] = {
                WGL_SUPPORT_OPENGL_ARB, true,
                WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                0
            };
            if (pfnWglChoosePixelFormat(hdc, openGlAttributes, 0, 1024, formats, &count)) {
                for (U32 i = 0; i < count; ++i) {
                    if (!pfnWglGetPixelFormatAttribiv(hdc, formats[i], 0, 22, attributes, results)) {
                        continue;
                    }

                    GLPixelFormatPtr format = std::make_shared<GLPixelFormat>();
                    format->id = i | PIXEL_FORMAT_NATIVE_INDEX_MASK;
                    format->depth = results[4];
                    format->nativeId = formats[i];
                    format->pf.nSize = sizeof(PIXELFORMATDESCRIPTOR);
                    format->pf.nVersion = 1;
                    format->pf.dwFlags = PFD_SUPPORT_OPENGL;
                    if (results[0]) {
                        format->pf.dwFlags |= PFD_DRAW_TO_WINDOW;
                    }
                    if (results[1]) {
                        format->pf.dwFlags |= PFD_DRAW_TO_BITMAP;
                    }
                    if (results[21]) {
                        format->pf.dwFlags |= PFD_DOUBLEBUFFER;
                    }
                    if (results[2]) {
                        format->pf.dwFlags |= PFD_STEREO;
                    }
                    if (results[3] == WGL_TYPE_RGBA_ARB) {
                        format->pf.iPixelType = PFD_TYPE_RGBA;
                    } else if (results[3] == WGL_TYPE_COLORINDEX_ARB) {
                        format->pf.iPixelType = PFD_TYPE_COLORINDEX;
                    } else {
                        continue;
                    }
                    format->pf.cColorBits = results[4];
                    format->pf.cRedBits = results[5];
                    format->pf.cRedShift = results[6];
                    format->pf.cGreenBits = results[7];
                    format->pf.cGreenShift = results[8];
                    format->pf.cBlueBits = results[9];
                    format->pf.cBlueShift = results[10];
                    format->pf.cAlphaBits = results[11];
                    format->pf.cAlphaShift = results[12];
                    format->pf.cAccumBits = results[13];
                    format->pf.cAccumRedBits = results[14];
                    format->pf.cAccumGreenBits = results[15];
                    format->pf.cAccumBlueBits = results[16];
                    format->pf.cAccumAlphaBits = results[17];
                    format->pf.cDepthBits = results[18];
                    format->pf.cStencilBits = results[19];
                    format->pf.cAuxBuffers = results[20];
                    format->pf.iLayerType = PFD_MAIN_PLANE;
                    format->pf.bReserved = 0;
                    format->pf.dwLayerMask = 0;
                    format->pf.dwVisibleMask = 0;
                    format->pf.dwDamageMask = 0;


                    if (arbPBuffer && pfnWglGetPixelFormatAttribiv(hdc, formats[i], 0, 3, attributesPBuffer, results)) {
                        format->pbuffer = results[0] ? true : false;
                        format->pbufferMaxWidth = results[1];
                        format->pbufferMaxHeight = results[2];
                        format->pbufferMaxPixels = results[3];
                    }
                    if (arbMultisample && pfnWglGetPixelFormatAttribiv(hdc, formats[i], 0, 2, attributesSamples, results)) {
                        format->sampleBuffers = results[0] ? true : false;
                        format->samples = results[1];
                    }

                    formatsById.set(format->id, format);
                    glformats.push_back(format);
                }
            }
        }

        wglMakeCurrent(oldhdc, oldrc);
        wglDeleteContext(hrc);
    }
    DestroyWindow(hwnd);
    UnregisterClass(tmpClass.lpszClassName, tmpClass.hInstance);
    return formatsById.size() > 0;
}

LRESULT CALLBACK glWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message) {
    case WM_PAINT:
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_SIZE:
        //glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_KEYDOWN:
        break;
    case WM_CLOSE:
        //wglMakeCurrent(window.deviceContext, NULL);
        //wglDeleteContext(window.renderContext);
        //ReleaseDC(hWnd, window.deviceContext);
        DestroyWindow(hWnd);
        /* stop event queue thread */
        PostQuitMessage(0);
        break;
    default:
        result = DefWindowProc(hWnd, message, wParam, lParam);
    }
    return result;
}

static WNDCLASS glClass;
static bool registered = false;

static HWND glCreateWindow(const GLPixelFormatPtr& format, int width, int height) {
    if (!registered) {        
        memset(&glClass, 0, sizeof(WNDCLASS));
        glClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        glClass.hInstance = GetModuleHandle(NULL);
        glClass.lpfnWndProc = glWndProc;
        glClass.lpszClassName = "boxedwineGL";
        RegisterClass(&glClass);
    }

    HWND hwnd = CreateWindow(glClass.lpszClassName, "boxedwine", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, glClass.hInstance, 0);

    if (!hwnd) {
        kpanic("glCreateWindow failed to create opengl window");
    }

    HDC hdc = GetDC(hwnd);

    if (format->nativeId & PIXEL_FORMAT_NATIVE_INDEX_MASK) {
        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        SetPixelFormat(hdc, (format->nativeId & ~PIXEL_FORMAT_NATIVE_INDEX_MASK), &pfd);
    } else {
        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        // format->pf has same flag flags
        pfd.dwFlags = format->pf.dwFlags;
        pfd.iPixelType = format->pf.iPixelType;
        pfd.cColorBits = format->pf.cColorBits;
        pfd.cRedBits = format->pf.cRedBits;
        pfd.cRedShift = format->pf.cRedShift;
        pfd.cGreenBits = format->pf.cGreenBits;
        pfd.cGreenShift = format->pf.cGreenShift;
        pfd.cBlueBits = format->pf.cBlueBits;
        pfd.cBlueShift = format->pf.cBlueShift;
        pfd.cAlphaBits = format->pf.cAlphaBits;
        pfd.cAlphaShift = format->pf.cAlphaShift;
        pfd.cAccumBits = format->pf.cAccumBits;
        pfd.cAccumRedBits = format->pf.cAccumRedBits;
        pfd.cAccumGreenBits = format->pf.cAccumGreenBits;
        pfd.cAccumBlueBits = format->pf.cAccumBlueBits;
        pfd.cAccumAlphaBits = format->pf.cAccumAlphaBits;
        pfd.cDepthBits = format->pf.cDepthBits;
        pfd.cStencilBits = format->pf.cStencilBits;
        pfd.cAuxBuffers = format->pf.cAuxBuffers;

        int pfIndex = ChoosePixelFormat(hdc, &pfd);
        if (format != 0) {
            SetPixelFormat(hdc, pfIndex, &pfd);
        }
    }
    return hwnd;
}

static void platform_glFinish(CPU* cpu) {
    glFinish();
}

// GLAPI void APIENTRY glFlush( void ) {
static void platform_glFlush(CPU* cpu) {
    glFlush();
}

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) pgl##func = gl##func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) pgl##func = gl##func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS)

#include "../tools/opengl/gldef.h"

static bool started = false;
static void initGL() {
    if (!started) {
#include "../source/opengl/glfunctions.h"
        int99Callback[Finish] = platform_glFinish;
        int99Callback[Flush] = platform_glFlush;
        started = true;
    }
}

#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

U32 PlatformOpenGL::createContext(const GLPixelFormatPtr& format, U32 shareContext, U32 major, U32 minor, U32 profile, U32 flags) {
    HWND hwnd = glCreateWindow(format, 32, 32);
    HGLRC result;
    HGLRC share = nullptr;
    int attribs[10];
    int index = 0;

    if (major) {
        attribs[index++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
        attribs[index++] = major;
    }
    if (minor) {
        attribs[index++] = WGL_CONTEXT_MINOR_VERSION_ARB;
        attribs[index++] = minor;
    }
    if (profile) {
        attribs[index++] = WGL_CONTEXT_PROFILE_MASK_ARB;
        attribs[index++] = profile; // glx and wgl use the same values
    }
    if (flags) {
        attribs[index++] = WGL_CONTEXT_FLAGS_ARB;
        attribs[index++] = flags; // glx and wgl use the same values
    }
    if (index) {
        attribs[index++] = 0;
    }

    initGL();
    if (shareContext) {
        share = (HGLRC)getNativeContext(shareContext);
    }
    HDC hdc = GetDC(hwnd);
    if (pfnWglCreateContextAttribsARB) {
        result = pfnWglCreateContextAttribsARB(hdc, share, index ? attribs : 0);
    } else {
        if (index) {
            klog("PlatformOpenGL::createContext wglCreateContextAttribsARB not found, falling back to wglCreateContext. attributes was supplied but not supported in wglCreateContext, this might not work as expected");
        }
        result = wglCreateContext(hdc);
        if (share) {
            wglShareLists(share, result);
        }
    }
    DestroyWindow(hwnd);
    if (result) {
        U32 id = nextContextId++;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
        contexts.set(id, result);
        return id;
    }
    return 0;
}

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG)

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS)

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) ext_gl##func = (gl##func##_func)wglGetProcAddress("gl" #func);

void glExtensionsLoaded();
static bool loaded = false;
static void loadExtensions() {
    if (!loaded) {
        loaded = true;
#include "../source/opengl/glfunctions.h"
        glExtensionsLoaded();
    }
}

void PlatformOpenGL::makeCurrent(const XWindowPtr& wnd, U32 contextId) {    
    if (!contextId) {
        wglMakeCurrent(0, 0);
    } else {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
        HWND hwnd = nullptr;
        HGLRC context = (HGLRC)getNativeContext(contextId);

        if (!nativeWindowHandles.get(wnd->id, hwnd)) {
            PlatformOpenGL::createWindow(wnd);
            if (!nativeWindowHandles.get(wnd->id, hwnd)) {
                kwarn("PlatformOpenGL::makeCurrent failed to create window");
                return;
            }
        }
        wglMakeCurrent(GetDC(hwnd), context);
        if (context) {
            loadExtensions();
        }
    }
}

void PlatformOpenGL::show(const std::shared_ptr<XWindow>& wnd, bool bShow) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
    HWND hwnd;
    if (nativeWindowHandles.get(wnd->id, hwnd)) {
        return;
    }
    if (bShow) {
        SetWindowPos(hwnd, 0, 0, 0, wnd->width(), wnd->height(), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
    }
    ShowWindow(hwnd, bShow ? SW_SHOW : SW_HIDE);
}

void PlatformOpenGL::resize(const std::shared_ptr<XWindow>& wnd) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
    HWND hwnd;
    if (nativeWindowHandles.get(wnd->id, hwnd)) {
        return;
    }
    SetWindowPos(hwnd, 0, 0, 0, wnd->width(), wnd->height(), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
}

void PlatformOpenGL::createWindow(const XWindowPtr& wnd) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
    HWND hwnd;
    if (nativeWindowHandles.get(wnd->id, hwnd)) {
        return;
    }
    GLPixelFormatPtr format = getFormat(wnd->getVisual()->ext_data);
    hwnd = glCreateWindow(format, wnd->width(), wnd->height());
    if (!hwnd) {
        return;
    }
    nativeWindowHandles.set(wnd->id, hwnd);
    if (wnd->isThisAndAncestorsMapped()) {
        ShowWindow(hwnd, SW_SHOW);
    }
}

void PlatformOpenGL::destroyContext(U32 id) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    HGLRC context = 0;
    if (contexts.get(id, context)) {
        wglDeleteContext(context);
        contexts.remove(id);
    }
}

void PlatformOpenGL::swapBuffer(const std::shared_ptr<XWindow>& wnd) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowMutex);
    HWND hwnd = nullptr;

    if (nativeWindowHandles.get(wnd->id, hwnd)) {
        wglSwapLayerBuffers(GetDC(hwnd), WGL_SWAP_MAIN_PLANE);
    }
    //pump();
}

void* PlatformOpenGL::getNativeContext(U32 id) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(contextMutex);
    HGLRC context = 0;
    if (contexts.get(id, context)) {
        return context;
    }
    return nullptr;
}

void PlatformOpenGL::init() {
	if (!initialized) {
		BOXEDWINE_CRITICAL_SECTION;

		if (!initialized) {
			initialized = true;            

			U32 count = KSystem::getPixelFormatCount();
			for (U32 i = 1; i < count; i++) {
				GLPixelFormatPtr format = std::make_shared<GLPixelFormat>();                
				format->id = i;
				format->pf = *KSystem::getPixelFormat(i);
                format->nativeId = i;
                format->depth = format->pf.cColorBits;
                format->bitsPerPixel = format->pf.cColorBits;
				formatsById.set(format->id, format);
                formats.push_back(format);
			}
            if (KSystem::videoEnabled) {
                hardwareListLoaded = queryOpenGL(formatsById, formats);
            }

		}
	}
}

void PlatformOpenGL::iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback) {
	for (auto& format : formats) {
		callback(format);
	}
}

GLPixelFormatPtr PlatformOpenGL::getFormat(U32 pixelFormatId) {
	return formatsById.get(pixelFormatId);
}
#endif