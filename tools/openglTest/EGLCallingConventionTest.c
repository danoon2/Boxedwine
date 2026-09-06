/* Run natively on i386 Linux. Intercept int99 to inspect the real library call
 * frames without needing Boxedwine or a graphics driver. */
#define _GNU_SOURCE
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>
#include "../opengl/gldef.h"

extern void (*glXGetProcAddressARB(const unsigned char* name))(void);

static uint32_t expectedIndex;
static uint32_t expectedArgs[9];
static unsigned int expectedCount;
static volatile sig_atomic_t pending;
static unsigned int calls;
static const uint32_t returnValue = 0x13572468;

static void trapFail(const char* error, size_t length) {
    ssize_t written = write(2, error, length);
    (void)written;
    _exit(1);
}

static void trap(int signal, siginfo_t* info, void* context) {
    ucontext_t* cpu = context;
    const unsigned char* ip = (void*)cpu->uc_mcontext.gregs[REG_EIP];
    const uint32_t* stack = (void*)cpu->uc_mcontext.gregs[REG_ESP];
    unsigned int i;
    uint32_t index;
    (void)signal;
    (void)info;
    if (!pending || ip[0] != 0xcd || ip[1] != 0x99) {
        static const char error[] = "FAIL unexpected interrupt or fault\n";
        trapFail(error, sizeof(error) - 1);
    }
    memcpy(&index, ip + 2, sizeof(index));
    if (index != expectedIndex) {
        static const char error[] = "FAIL int99 callback index\n";
        trapFail(error, sizeof(error) - 1);
    }
    for (i = 0; i < expectedCount; ++i) {
        if (stack[i + 1] != expectedArgs[i]) {
            static const char error[] = "FAIL int99 argument frame\n";
            trapFail(error, sizeof(error) - 1);
        }
    }
    pending = 0;
    ++calls;
    cpu->uc_mcontext.gregs[REG_EAX] = returnValue;
    cpu->uc_mcontext.gregs[REG_EIP] += 6;
}

static void expect(uint32_t index, unsigned int count, ...) {
    va_list args;
    unsigned int i;
    if (pending || count > sizeof(expectedArgs) / sizeof(expectedArgs[0])) {
        fputs("FAIL missing callback\n", stderr);
        exit(1);
    }
    expectedIndex = index;
    expectedCount = count;
    va_start(args, count);
    for (i = 0; i < count; ++i) {
        expectedArgs[i] = va_arg(args, uint32_t);
    }
    va_end(args);
    pending = 1;
}

#define WORD(value) ((uint32_t)(uintptr_t)(value))
#define CHECK(expression) do { \
    if (WORD(expression) != returnValue || pending) { \
        fprintf(stderr, "FAIL return value: %s\n", #expression); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct sigaction action = {0};
    EGLDisplay display = (EGLDisplay)0x12345678;
    EGLSync sync = (EGLSync)0x23456789;
    EGLConfig config = (EGLConfig)0x3456789a;
    EGLSurface surface = (EGLSurface)0x456789ab;
    EGLint attribs[] = {EGL_WIDTH, 19, EGL_HEIGHT, 23, EGL_NONE};
    EGLint major = 0, minor = 0, count = 0;
    const EGLTime timeout = 0x1234567887654321ULL;
    const char* name = "glClearColor";
    PFNGLCLEARCOLORPROC clearColor;
    PFNEGLGETCURRENTCONTEXTPROC getContext;

    action.sa_sigaction = trap;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGSEGV, &action, NULL)) {
        perror("sigaction");
        return 1;
    }

    expect(kEglGetCurrentContext, 0);
    CHECK(eglGetCurrentContext());
    expect(kEglGetDisplay, 1, WORD(display));
    CHECK(eglGetDisplay((EGLNativeDisplayType)display));
    expect(kEglQueryString, 2, WORD(display), WORD(EGL_VERSION));
    CHECK(eglQueryString(display, EGL_VERSION));
    expect(kEglInitialize, 3, WORD(display), WORD(&major), WORD(&minor));
    CHECK(eglInitialize(display, &major, &minor));
    expect(kEglQuerySurface, 4, WORD(display), WORD(surface), WORD(EGL_WIDTH), WORD(&major));
    CHECK(eglQuerySurface(display, surface, EGL_WIDTH, &major));
    expect(kEglChooseConfig, 5, WORD(display), WORD(attribs), WORD(&config), WORD(1), WORD(&count));
    CHECK(eglChooseConfig(display, attribs, &config, 1, &count));

    // EXT drops the platform argument. NV sync entry points insert a display.
    expect(kEglGetDisplay, 1, WORD(display));
    CHECK(eglGetPlatformDisplayEXT(EGL_PLATFORM_X11_EXT, display, attribs));
    expect(kEglGetPlatformDisplay, 3, WORD(EGL_PLATFORM_X11_EXT), WORD(display), WORD(attribs));
    CHECK(eglGetPlatformDisplay(EGL_PLATFORM_X11_EXT, display, (const EGLAttrib*)attribs));
    expect(kEglClientWaitSync, 5, WORD(display), WORD(sync), WORD(7), WORD(timeout), WORD(timeout >> 32));
    CHECK(eglClientWaitSync(display, sync, 7, timeout));
    expect(kEglClientWaitSync, 5, WORD(display), WORD(sync), WORD(9), WORD(timeout), WORD(timeout >> 32));
    CHECK(eglClientWaitSyncKHR(display, sync, 9, timeout));
    expect(kEglClientWaitSync, 5, WORD(0), WORD(sync), WORD(11), WORD(timeout), WORD(timeout >> 32));
    CHECK(eglClientWaitSyncNV(sync, 11, timeout));
    expect(kEglDestroySync, 2, WORD(0), WORD(sync));
    CHECK(eglDestroySyncNV(sync));
    expect(kEglGetSyncAttrib, 4, WORD(0), WORD(sync), WORD(EGL_SYNC_STATUS), WORD(&major));
    CHECK(eglGetSyncAttribNV(sync, EGL_SYNC_STATUS, &major));

    // Verify lookup's nested callback frame, then call the returned ABI stub.
    expect(kGlProcAddressAvailable, 1, WORD(name));
    clearColor = (PFNGLCLEARCOLORPROC)eglGetProcAddress(name);
    if (!clearColor || pending) return 1;
    expect(ClearColor, 4, 0x3e800000u, 0x3f000000u, 0x3f400000u, 0x3f800000u);
    clearColor(0.25f, 0.5f, 0.75f, 1.0f);
    getContext = (PFNEGLGETCURRENTCONTEXTPROC)eglGetProcAddress("eglGetCurrentContext");
    if (!getContext) return 1;
    expect(kEglGetCurrentContext, 0);
    CHECK(getContext());
    expect(kGlProcAddressAvailable, 1, WORD(name));
    clearColor = (PFNGLCLEARCOLORPROC)glXGetProcAddressARB((const unsigned char*)name);
    if (!clearColor || pending) return 1;
    expect(ClearColor, 4, 0x3e800000u, 0x3f000000u, 0x3f400000u, 0x3f800000u);
    clearColor(0.25f, 0.5f, 0.75f, 1.0f);

    expect(ClearColor, 4, 0x3e800000u, 0x3f000000u, 0x3f400000u, 0x3f800000u);
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
    expect(TexImage2D, 9, WORD(GL_TEXTURE_2D), WORD(2), WORD(GL_RGBA), WORD(19), WORD(23), WORD(0), WORD(GL_RGBA), WORD(GL_UNSIGNED_BYTE), WORD(attribs));
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 19, 23, 0, GL_RGBA, GL_UNSIGNED_BYTE, attribs);
    if (pending) return 1;
    printf("PASS EGL/GLES calling convention (%u callbacks)\n", calls);
    return 0;
}
