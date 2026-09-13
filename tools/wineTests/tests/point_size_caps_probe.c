/* Compare Direct3D's public point-size capability with the actual WebGL limit.
 * Compile once with TEST_D3D8 and once without it. The Windows control checks
 * valid capabilities; exact GL/D3D equality applies only to BoxedWine WebGL.
 */
#define COBJMACROS
#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef TEST_D3D8
#include <d3d8.h>
typedef IDirect3D8 TestD3D;
typedef D3DCAPS8 TestCaps;
#define TEST_VERSION 8
#define CREATE_D3D() Direct3DCreate8(D3D_SDK_VERSION)
#define GET_CAPS(d,c) IDirect3D8_GetDeviceCaps(d,0,D3DDEVTYPE_HAL,c)
#define RELEASE_D3D(d) IDirect3D8_Release(d)
#else
#include <d3d9.h>
typedef IDirect3D9 TestD3D;
typedef D3DCAPS9 TestCaps;
#define TEST_VERSION 9
#define CREATE_D3D() Direct3DCreate9(D3D_SDK_VERSION)
#define GET_CAPS(d,c) IDirect3D9_GetDeviceCaps(d,0,D3DDEVTYPE_HAL,c)
#define RELEASE_D3D(d) IDirect3D9_Release(d)
#endif
#ifndef GL_ALIASED_POINT_SIZE_RANGE
#define GL_ALIASED_POINT_SIZE_RANGE 0x846d
#endif

static unsigned int tests, failures;
static int check(int condition, unsigned int line, const char *message)
{
    ++tests;
    if (condition) return 1;
    ++failures;
    printf("point_size_caps_probe.c:%u: Test failed: D3D%u %s.\n", line, TEST_VERSION, message);
    return 0;
}
#define REQUIRE(c,m) do { if (!check(!!(c),__LINE__,m)) goto done; } while(0)
#define CHECK(c,m) check(!!(c),__LINE__,m)

int main(void)
{
    HWND window=NULL;
    HDC dc=NULL;
    HGLRC context=NULL;
    TestD3D *d3d=NULL;
    TestCaps caps;
    PIXELFORMATDESCRIPTOR pfd;
    GLfloat range[2]={-1.0f,-1.0f};
    const char *vendor, *renderer;
    int format, webgl;
    unsigned int iteration;
    GLenum error;

    window=CreateWindowA("static","Point size capabilities",WS_OVERLAPPEDWINDOW,
            0,0,96,96,NULL,NULL,NULL,NULL);
    REQUIRE(window,"CreateWindow");
    dc=GetDC(window);
    REQUIRE(dc,"GetDC");
    ZeroMemory(&pfd,sizeof(pfd));
    pfd.nSize=sizeof(pfd);pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType=PFD_TYPE_RGBA;pfd.cColorBits=32;pfd.cDepthBits=24;pfd.cStencilBits=8;
    format=ChoosePixelFormat(dc,&pfd);
    REQUIRE(format,"ChoosePixelFormat");
    REQUIRE(SetPixelFormat(dc,format,&pfd),"SetPixelFormat");
    context=wglCreateContext(dc);
    REQUIRE(context,"wglCreateContext");
    REQUIRE(wglMakeCurrent(dc,context),"wglMakeCurrent");
    vendor=(const char *)glGetString(GL_VENDOR);
    renderer=(const char *)glGetString(GL_RENDERER);
    webgl=vendor&&renderer&&strstr(vendor,"BoxedWine")&&strstr(renderer,"WebGL");
    for(iteration=0;iteration<32&&glGetError()!=GL_NO_ERROR;++iteration) {}
    REQUIRE(iteration<32,"clear initial GL errors");
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE,range);
    error=glGetError();
    printf("POINT_GL_RANGE D3D%u webgl=%u min=%.9g max=%.9g error=%#x\n",
            TEST_VERSION,webgl,range[0],range[1],error);
    REQUIRE(error==GL_NO_ERROR,"supported aliased point-size query");
    REQUIRE(isfinite(range[0])&&isfinite(range[1])&&range[0]>0.0f&&range[1]>=range[0],"valid GL range");
    for(iteration=0;iteration<3;++iteration)
    {
        d3d=CREATE_D3D();
        REQUIRE(d3d,"create Direct3D adapter");
        ZeroMemory(&caps,sizeof(caps));
        REQUIRE(SUCCEEDED(GET_CAPS(d3d,&caps)),"GetDeviceCaps");
        printf("POINT_D3D_CAP D3D%u iteration=%u max=%.9g expected_webgl=%.9g\n",
                TEST_VERSION,iteration,caps.MaxPointSize,range[1]);
        CHECK(isfinite(caps.MaxPointSize)&&caps.MaxPointSize>=1.0f,"finite, usable maximum point size");
        if(webgl) CHECK(fabsf(caps.MaxPointSize-range[1])<0.001f,"capability matches actual WebGL range");
        RELEASE_D3D(d3d);d3d=NULL;
    }
done:
    if(d3d) RELEASE_D3D(d3d);
    if(context) { wglMakeCurrent(NULL,NULL);wglDeleteContext(context); }
    if(dc) ReleaseDC(window,dc);
    if(window) DestroyWindow(window);
    printf("0000:pointsize: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures?1:0;
}
