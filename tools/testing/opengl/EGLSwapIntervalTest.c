/* Native EGL/X11 regression: real swap pacing and per-surface interval state. */
#include <GL/gl.h>
#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void require(int condition, const char* message) {
    if (!condition) {
        printf("FAIL %s\n", message);
        exit(1);
    }
}

static double seconds(void) {
    struct timespec ts;
    require(clock_gettime(CLOCK_MONOTONIC, &ts) == 0, "monotonic clock");
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

static double measure(EGLDisplay display, EGLSurface surface, const char* label) {
    int i;
    double start, elapsed;
    require(eglSwapBuffers(display, surface), "warm-up swap");
    start = seconds();
    for (i = 0; i < 24; ++i) {
        glClearColor(i / 24.0f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        require(eglSwapBuffers(display, surface), "measured swap");
    }
    elapsed = seconds() - start;
    printf("%s: %.2f FPS, %.3f seconds\n", label, 24.0 / elapsed, elapsed);
    return elapsed;
}

static void expect_sync(double elapsed, double fps) {
    // Permit scheduling jitter and a slower host monitor, but reject swaps
    // returning early. Run on an otherwise idle machine for the upper bound.
    require(elapsed >= 24.0 / fps * 0.95 && elapsed < 24.0 / fps * 4,
            "synchronized swaps obey the configured frame limit");
}

void _start(void) {
    Display* xdisplay = XOpenDisplay(NULL);
    EGLDisplay display;
    EGLConfig config;
    EGLint count = 0, visual_id = 0, min_interval = -1, max_interval = -1;
    XVisualInfo visual_template = {0};
    int visual_count = 0, i;
    XVisualInfo* visual;
    XSetWindowAttributes attributes = {0};
    EGLSurface surfaces[2], pbuffer;
    EGLContext contexts[2];
    Window windows[2];
    double synced, unsynced;
    const char* expected_fps = getenv("EGL_TEST_VSYNC_FPS");
    double fps = expected_fps ? atof(expected_fps) : 60;
    const EGLint config_attributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_SAMPLE_BUFFERS, 0, EGL_NONE
    };
    const EGLint pbuffer_attributes[] = { EGL_WIDTH, 8, EGL_HEIGHT, 8, EGL_NONE };

    require(fps > 0, "positive expected FPS");
    require(xdisplay != NULL, "XOpenDisplay");
    display = eglGetDisplay((EGLNativeDisplayType)xdisplay);
    require(display != EGL_NO_DISPLAY && eglInitialize(display, NULL, NULL), "eglInitialize");
    require(eglBindAPI(EGL_OPENGL_API), "eglBindAPI desktop GL");
    require(!eglSwapInterval(display, 1) && eglGetError() == EGL_BAD_CONTEXT &&
            eglGetError() == EGL_SUCCESS, "no-context error is reported and cleared");
    require(eglChooseConfig(display, config_attributes, &config, 1, &count) && count > 0,
            "eglChooseConfig");
    require(eglGetConfigAttrib(display, config, EGL_MIN_SWAP_INTERVAL, &min_interval) &&
            eglGetConfigAttrib(display, config, EGL_MAX_SWAP_INTERVAL, &max_interval) &&
            min_interval == 0 && max_interval == 1, "advertised interval range");
    require(eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &visual_id), "EGL visual");
    visual_template.visualid = visual_id;
    visual = XGetVisualInfo(xdisplay, VisualIDMask, &visual_template, &visual_count);
    require(visual && visual_count > 0, "X11 visual");
    attributes.colormap = XCreateColormap(xdisplay, DefaultRootWindow(xdisplay), visual->visual, AllocNone);
    for (i = 0; i < 2; ++i) {
        windows[i] = XCreateWindow(xdisplay, DefaultRootWindow(xdisplay), i * 100, 0,
                80, 60, 0, visual->depth, InputOutput, visual->visual, CWColormap, &attributes);
        require(windows[i] != None, "XCreateWindow");
        XMapWindow(xdisplay, windows[i]);
        surfaces[i] = eglCreateWindowSurface(display, config, (EGLNativeWindowType)windows[i], NULL);
        contexts[i] = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
        require(surfaces[i] != EGL_NO_SURFACE && contexts[i] != EGL_NO_CONTEXT, "EGL window/context");
    }
    XSync(xdisplay, False);
    require(eglMakeCurrent(display, surfaces[0], surfaces[0], contexts[0]), "bind first surface");
    require(eglGetCurrentSurface(EGL_DRAW) == surfaces[0] &&
            eglGetCurrentSurface(EGL_READ) == surfaces[0], "current surface queries");
    synced = measure(display, surfaces[0], "default interval 1");
    expect_sync(synced, fps);
    require(eglSwapInterval(display, 0), "disable VSync");
    unsynced = measure(display, surfaces[0], "interval 0");
    require(unsynced < synced * 0.75, "disabling VSync removes frame limit");
    require(!eglSwapInterval(EGL_NO_DISPLAY, 1) && eglGetError() == EGL_BAD_DISPLAY,
            "invalid display is rejected");
    require(eglSwapInterval(display, -100), "negative interval clamps to zero");
    require(measure(display, surfaces[0], "clamped interval 0") < synced * 0.75,
            "negative intervals do not enable VSync");
    require(eglSwapInterval(display, 100), "large interval clamps to one");
    expect_sync(measure(display, surfaces[0], "clamped interval 1"), fps);

    require(eglMakeCurrent(display, surfaces[1], surfaces[1], contexts[0]), "same context, second surface");
    expect_sync(measure(display, surfaces[1], "second surface default"), fps);
    require(eglSwapInterval(display, 0), "disable second surface VSync");
    require(eglMakeCurrent(display, surfaces[0], surfaces[0], contexts[1]), "new context, first surface");
    expect_sync(measure(display, surfaces[0], "first surface retains interval 1"), fps);
    require(eglMakeCurrent(display, surfaces[1], surfaces[1], contexts[1]), "new context, second surface");
    require(measure(display, surfaces[1], "second surface retains interval 0") < synced * 0.75,
            "swap interval belongs to the surface across context switches");
    require(!eglMakeCurrent(display, (EGLSurface)0xdeadbeef, (EGLSurface)0xdeadbeef, contexts[0]) &&
            eglGetCurrentContext() == contexts[1] && eglGetCurrentSurface(EGL_DRAW) == surfaces[1],
            "failed bind preserves current state");

    pbuffer = eglCreatePbufferSurface(display, config, pbuffer_attributes);
    require(pbuffer != EGL_NO_SURFACE && eglMakeCurrent(display, pbuffer, pbuffer, contexts[0]) &&
            eglSwapInterval(display, 1), "pbuffer interval is accepted");
    require(measure(display, pbuffer, "pbuffer swaps") < synced * 0.75, "pbuffer swaps are not paced");
    require(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, contexts[0]) &&
            !eglSwapInterval(display, 1) && eglGetError() == EGL_BAD_SURFACE,
            "surfaceless context rejects swap interval");
    require(eglReleaseThread() && eglGetCurrentContext() == EGL_NO_CONTEXT &&
            eglGetCurrentSurface(EGL_DRAW) == EGL_NO_SURFACE &&
            eglGetCurrentSurface(EGL_READ) == EGL_NO_SURFACE, "release clears current bindings");
    require(eglDestroySurface(display, pbuffer), "destroy pbuffer");
    for (i = 0; i < 2; ++i) {
        require(eglDestroyContext(display, contexts[i]) && eglDestroySurface(display, surfaces[i]), "EGL cleanup");
        XDestroyWindow(xdisplay, windows[i]);
    }
    require(eglTerminate(display), "eglTerminate");
    XFreeColormap(xdisplay, attributes.colormap);
    XFree(visual);
    XCloseDisplay(xdisplay);
    printf("PASS EGL swap interval pacing, clamping, surface state, and errors\n");
    exit(0);
}
