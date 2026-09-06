/* Native EGL/X11 regression: resize a GL window before its first presentation. */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

static void require(int condition, const char* message) {
    if (!condition) {
        printf("FAIL %s\n", message);
        exit(1);
    }
}

static void check_size(Display* xdisplay, Window window, EGLDisplay display,
        EGLSurface surface, unsigned int width, unsigned int height) {
    EGLint actual_width = 0, actual_height = 0;
    GLuint query = 0, samples = 0;
    GLint viewport[4] = {0};
    XWindowChanges changes = {0};

    changes.width = width;
    changes.height = height;
    XConfigureWindow(xdisplay, window, CWWidth | CWHeight, &changes);
    XSync(xdisplay, False);
    require(eglQuerySurface(display, surface, EGL_WIDTH, &actual_width) &&
            eglQuerySurface(display, surface, EGL_HEIGHT, &actual_height) &&
            actual_width == (EGLint)width && actual_height == (EGLint)height,
            "EGL surface reports resized dimensions");

    // Keep the viewport larger than every tested window. The sample count
    // measures the actual host framebuffer's bounds, independently of the
    // guest dimensions returned by eglQuerySurface.
    glGetIntegerv(GL_VIEWPORT, viewport);
    require(viewport[2] == 80 && viewport[3] == 60, "resize preserves GL viewport");
    glGenQueries(1, &query);
    require(query != 0, "occlusion query creation");
    glBeginQuery(GL_SAMPLES_PASSED, query);
    glBegin(GL_TRIANGLES);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(3.0f, -1.0f);
    glVertex2f(-1.0f, 3.0f);
    glEnd();
    glEndQuery(GL_SAMPLES_PASSED);
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &samples);
    glDeleteQueries(1, &query);
    require(glGetError() == GL_NO_ERROR, "resized framebuffer draw");
    printf("Framebuffer %ux%u: %u samples (expected %u)\n",
            width, height, samples, width * height);
    require(samples == width * height, "native framebuffer follows EGL window resize");
}

void _start(void) {
    Display* xdisplay = XOpenDisplay(NULL);
    EGLDisplay display;
    EGLConfig config;
    EGLint count = 0, visual_id = 0;
    XVisualInfo visual_template = {0};
    int visual_count = 0;
    XVisualInfo* visual;
    XSetWindowAttributes attributes = {0};
    EGLSurface surface;
    EGLContext context;
    Window window;
    GLint samples = 0;
    const EGLint config_attributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    require(xdisplay != NULL, "XOpenDisplay");
    display = eglGetDisplay((EGLNativeDisplayType)xdisplay);
    require(display != EGL_NO_DISPLAY && eglInitialize(display, NULL, NULL), "eglInitialize");
    require(eglBindAPI(EGL_OPENGL_API), "eglBindAPI desktop GL");
    require(eglChooseConfig(display, config_attributes, &config, 1, &count) && count > 0,
            "eglChooseConfig");
    require(eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &visual_id), "EGL visual");
    visual_template.visualid = visual_id;
    visual = XGetVisualInfo(xdisplay, VisualIDMask, &visual_template, &visual_count);
    require(visual && visual_count > 0, "X11 visual");
    attributes.colormap = XCreateColormap(xdisplay, DefaultRootWindow(xdisplay), visual->visual, AllocNone);
    window = XCreateWindow(xdisplay, DefaultRootWindow(xdisplay), 0, 0, 80, 60, 0,
            visual->depth, InputOutput, visual->visual, CWColormap, &attributes);
    require(window != None, "XCreateWindow");
    XMapWindow(xdisplay, window);
    XSync(xdisplay, False);
    surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)window, NULL);
    require(surface != EGL_NO_SURFACE, "eglCreateWindowSurface");
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
    require(context != EGL_NO_CONTEXT && eglMakeCurrent(display, surface, surface, context),
            "eglMakeCurrent");
    glGetIntegerv(GL_SAMPLES, &samples);
    require(samples <= 1, "single-sample framebuffer");
    glViewport(0, 0, 80, 60);

    // No swap or flush yet: the native GL window remains hidden.
    check_size(xdisplay, window, display, surface, 32, 24);
    check_size(xdisplay, window, display, surface, 64, 48);
    require(eglSwapBuffers(display, surface), "first presentation");
    check_size(xdisplay, window, display, surface, 40, 30);

    require(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) &&
            eglDestroyContext(display, context) && eglDestroySurface(display, surface) &&
            eglTerminate(display), "EGL cleanup");
    XDestroyWindow(xdisplay, window);
    XFreeColormap(xdisplay, attributes.colormap);
    XFree(visual);
    XCloseDisplay(xdisplay);
    printf("PASS EGL window resize before and after first presentation\n");
    exit(0);
}
