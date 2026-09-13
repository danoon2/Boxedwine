#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>

struct context { HWND window; HDC dc; HGLRC gl; unsigned int tests, failures; };
static int check(struct context *c, int success, const char *message)
{
    ++c->tests;
    if (success) return 1;
    ++c->failures;
    printf("thread_binding.c: Test failed: %s.\n", message);
    return 0;
}
#define CHECK(c, value, message) check(c, !!(value), message)
static int initialize(struct context *c)
{
    PIXELFORMATDESCRIPTOR pfd = {0};
    int format;
    c->window = CreateWindowA("static", "GL thread binding", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    if (!CHECK(c, c->window, "create window")) return 0;
    c->dc = GetDC(c->window);
    if (!CHECK(c, c->dc, "get DC")) return 0;
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    format = ChoosePixelFormat(c->dc, &pfd);
    if (!CHECK(c, format, "choose format")) return 0;
    if (!CHECK(c, SetPixelFormat(c->dc, format, &pfd), "set format")) return 0;
    c->gl = wglCreateContext(c->dc);
    if (!CHECK(c, c->gl, "create context")) return 0;
    return CHECK(c, wglMakeCurrent(c->dc, c->gl), "bind context");
}
static void cleanup(struct context *c)
{
    if (c->gl)
    {
        CHECK(c, wglMakeCurrent(NULL, NULL), "unbind context");
        CHECK(c, wglDeleteContext(c->gl), "delete context");
    }
    if (c->dc) CHECK(c, ReleaseDC(c->window, c->dc), "release DC");
    if (c->window) CHECK(c, DestroyWindow(c->window), "destroy window");
}
static DWORD WINAPI worker(void *data)
{
    struct context *c = data;
    initialize(c);
    cleanup(c);
    return c->failures;
}
int main(int argc, char **argv)
{
    struct context main_context = {0}, other = {0};
    HANDLE thread = NULL;
    DWORD status = 1;
    GLenum error;
    int expect_error = argc >= 2 && !strcmp(argv[1], "--expect-restore-error");
    if (!initialize(&main_context)) goto done;
    CHECK(&main_context, glGetError() == GL_NO_ERROR, "initial GL state");
    thread = CreateThread(NULL, 0, worker, &other, 0, NULL);
    if (!CHECK(&main_context, thread, "create second thread")) goto done;
    if (!CHECK(&main_context, WaitForSingleObject(thread, 60000) == WAIT_OBJECT_0, "join second thread"))
        return 2; /* Do not free a stack context still in use by the worker. */
    CHECK(&main_context, GetExitCodeThread(thread, &status) && !status, "second thread succeeded");
    CHECK(&main_context, wglGetCurrentContext() == main_context.gl, "guest current context survived");
    /* The worker released the ST host binding; this call must restore ours.
     * The optional argument is for a browser-injected failed-restore control. */
    error = glGetError();
    printf("RESTORE_ERROR_RESULT actual=%#x expected=%#x\n", error, expect_error ? GL_INVALID_OPERATION : GL_NO_ERROR);
    CHECK(&main_context, error == (expect_error ? GL_INVALID_OPERATION : GL_NO_ERROR), "restoration error result");
    CHECK(&main_context, wglMakeCurrent(NULL, NULL), "explicitly unbind after restoration");
    CHECK(&main_context, wglMakeCurrent(main_context.dc, main_context.gl), "explicitly rebind after restoration");
    CHECK(&main_context, glGetError() == GL_NO_ERROR, "restoration error was consumed");
done:
    if (thread) CloseHandle(thread);
    cleanup(&main_context);
    printf("0000:binding: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",
            main_context.tests + other.tests, main_context.failures + other.failures);
    return main_context.failures || other.failures ? 1 : 0;
}
