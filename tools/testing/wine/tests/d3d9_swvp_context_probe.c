/* WineD3D GL cleanup regression. Run inside Boxedwine with
 * WINE_D3D_CONFIG=csmt=0 so D3D and the unrelated WGL context use one thread.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror
 * d3d9_swvp_context_probe.c -ld3d9 -lopengl32 -lgdi32 -o swvp-context.exe
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <GL/gl.h>
#include <stddef.h>
#include <stdio.h>

typedef void (APIENTRY *bind_buffer_fn)(GLenum, GLuint);
typedef void (APIENTRY *buffer_data_fn)(GLenum, ptrdiff_t, const void *, GLenum);
typedef GLboolean (APIENTRY *is_buffer_fn)(GLuint);
typedef void (APIENTRY *delete_buffers_fn)(GLsizei, const GLuint *);

static unsigned checks, failures;
#define CHECK(c, message) do { ++checks; if (!(c)) { ++failures; puts("FAIL: " message); } } while (0)
#define HR(call) CHECK(SUCCEEDED(call), #call)

int main(int argc, char **argv)
{
    const DWORD vs[] = {0xfffe0200, 0x0200001f, 0x80000000, 0x900f0000,
            0x02000001, 0xc00f0000, 0x90e40000, /* mov oPos, v0 */
            0x02000001, 0xd00f0000, 0xa0e40000, /* mov oD0, c0 */
            0x0000ffff};
    const float color[4] = {1, 0, 0, 1};
    const float quad[4][3] = {{-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0}};
    IDirect3D9 *d3d;
    IDirect3DDevice9 *device = NULL;
    IDirect3DVertexShader9 *shader = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    PIXELFORMATDESCRIPTOR pfd = {0};
    HWND window, gl_window;
    HDC dc = NULL;
    HGLRC rc = NULL;
    GLint swvp_buffer = 0;
    GLuint protected_buffer;
    bind_buffer_fn bind;
    buffer_data_fn data;
    is_buffer_fn is;
    delete_buffers_fn del;

    if (argc > 1 && !freopen(argv[1], "w", stdout)) return 2;
    window = CreateWindowA("STATIC", "D3D cleanup", WS_OVERLAPPEDWINDOW,
            20,20,128,128,NULL,NULL,GetModuleHandleA(NULL),NULL);
    gl_window = CreateWindowA("STATIC", "Unrelated GL context", WS_OVERLAPPEDWINDOW,
            20,20,128,128,NULL,NULL,GetModuleHandleA(NULL),NULL);
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    CHECK(d3d != NULL, "Direct3DCreate9");
    if (!d3d) goto done;
    pp.Windowed = TRUE; pp.hDeviceWindow = window; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferWidth = pp.BackBufferHeight = 64; pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_MIXED_VERTEXPROCESSING, &pp, &device));
    if (!device) goto done;
    HR(IDirect3DDevice9_CreateVertexShader(device, vs, &shader));
    if (!shader) goto done;
    HR(IDirect3DDevice9_SetVertexShader(device, shader));
    HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 0, color, 1));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ));
    HR(IDirect3DDevice9_BeginScene(device));
    HR(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(quad[0])));
    HR(IDirect3DDevice9_EndScene(device));
    IDirect3DVertexShader9_Release(shader); shader = NULL;
    CHECK(wglGetCurrentContext() != NULL, "Wine GL context current (requires csmt=0)");
    if (!wglGetCurrentContext()) goto done;
    glGetIntegerv(0x8a28 /* GL_UNIFORM_BUFFER_BINDING */, &swvp_buffer);
    CHECK(swvp_buffer > 0, "SWVP constant buffer created");
    if (swvp_buffer <= 0) goto done;

    dc = GetDC(gl_window);
    pfd.nSize = sizeof(pfd); pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA; pfd.cColorBits = 24;
    CHECK(SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd), "external pixel format");
    rc = wglCreateContext(dc);
    CHECK(rc != NULL && wglMakeCurrent(dc, rc), "external context current");
    if (!rc) goto done;
    bind = (bind_buffer_fn)(void *)wglGetProcAddress("glBindBuffer");
    data = (buffer_data_fn)(void *)wglGetProcAddress("glBufferData");
    is = (is_buffer_fn)(void *)wglGetProcAddress("glIsBuffer");
    del = (delete_buffers_fn)(void *)wglGetProcAddress("glDeleteBuffers");
    CHECK(bind && data && is && del, "GL buffer entry points");
    if (!(bind && data && is && del)) goto done;
    /* A legacy WGL context allows binding an unused name to create a buffer.
     * Protect the same name in a different share group, without depending on
     * the driver's allocation order or on Wine's other GL resources. */
    protected_buffer = swvp_buffer;
    bind(0x8892 /* GL_ARRAY_BUFFER */, protected_buffer);
    data(0x8892, sizeof(color), color, 0x88e4 /* GL_STATIC_DRAW */);
    bind(0x8892, 0);
    CHECK(is(protected_buffer) && glGetError() == GL_NO_ERROR, "unrelated buffer created");
    IDirect3DDevice9_Release(device); device = NULL;
    CHECK(wglMakeCurrent(dc, rc), "external context restored");
    CHECK(is(protected_buffer), "D3D cleanup preserves unrelated GL buffer");
    del(1, &protected_buffer);
done:
    if (shader) IDirect3DVertexShader9_Release(shader);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (rc) { wglMakeCurrent(NULL, NULL); wglDeleteContext(rc); }
    if (dc) ReleaseDC(gl_window, dc);
    DestroyWindow(gl_window); DestroyWindow(window);
    printf("%u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
