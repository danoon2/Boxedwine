/* Focused regression for client packing and pixel-pack-buffer readback. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define GL_FRAMEBUFFER 0x8d40
#define GL_COLOR_ATTACHMENT0 0x8ce0
#define GL_FRAMEBUFFER_COMPLETE 0x8cd5
#define GL_RGBA8 0x8058
#define GL_RGBA32F 0x8814
#define GL_RGBA32I 0x8d82
#define GL_RGBA32UI 0x8d70
#define GL_RGBA_INTEGER 0x8d99
#define GL_PIXEL_PACK_BUFFER 0x88eb
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ed
#define GL_STREAM_READ 0x88e1
#ifndef GL_PACK_ROW_LENGTH
#define GL_PACK_ROW_LENGTH 0x0d02
#endif
#ifndef GL_PACK_SKIP_ROWS
#define GL_PACK_SKIP_ROWS 0x0d03
#endif
#ifndef GL_PACK_SKIP_PIXELS
#define GL_PACK_SKIP_PIXELS 0x0d04
#endif
typedef void (APIENTRY *gen_fn)(GLsizei, GLuint *);
typedef void (APIENTRY *bind_fn)(GLenum, GLuint);
typedef void (APIENTRY *attach_fn)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (APIENTRY *status_fn)(GLenum);
typedef void (APIENTRY *delete_fn)(GLsizei, const GLuint *);
typedef void (APIENTRY *data_fn)(GLenum, ptrdiff_t, const void *, GLenum);
typedef void (APIENTRY *getdata_fn)(GLenum, ptrdiff_t, ptrdiff_t, void *);
static unsigned int checks, failures;
static void check(int success, const char *format, ...)
{
    va_list args;
    ++checks;
    if (success) return;
    ++failures; printf("FAIL "); va_start(args, format); vprintf(format, args); va_end(args);
    printf("\n"); fflush(stdout);
}

int main(void)
{
    const GLubyte colors[] = {255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,0,255};
    struct pack_case { const char *name; int width, row_length, skip_rows, skip_pixels, alignment, pbo, offset; };
    const struct pack_case cases[] = {
        {"tight client", 2,0,0,0,4,0,0},
        {"strided client", 2,6,1,1,8,0,0},
        {"aligned client", 1,0,0,0,8,0,0},
        {"tight PBO offset", 2,0,0,0,4,1,8},
        {"strided PBO offset", 2,6,1,1,8,1,8},
    };
    WNDCLASSA wc = {0}; PIXELFORMATDESCRIPTOR pfd = {0};
    HWND window = NULL; HDC dc = NULL; HGLRC context = NULL;
    GLuint texture=0, framebuffer=0, pbo=0;
    gen_fn gen_frames=NULL, gen_buffers=NULL;
    bind_fn bind_frame=NULL, bind_buffer=NULL;
    delete_fn delete_frames=NULL, delete_buffers=NULL;
    attach_fn attach=NULL; status_fn status=NULL;
    data_fn buffer_data=NULL; getdata_fn get_data=NULL;
    unsigned char *pages=NULL, *pixels;
    unsigned int n;
    int pf;
    wc.style=CS_OWNDC; wc.lpfnWndProc=DefWindowProcA;
    wc.hInstance=GetModuleHandleA(NULL); wc.lpszClassName="BWReadbackPackProbe";
    check(RegisterClassA(&wc)!=0,"register class");
    window=CreateWindowA(wc.lpszClassName,"Readback pack probe",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        0,0,320,240,NULL,NULL,wc.hInstance,NULL);
    check(window!=NULL,"create window"); if (!window) goto done;
    dc=GetDC(window); pfd.nSize=sizeof(pfd); pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType=PFD_TYPE_RGBA; pfd.cColorBits=32;
    pf=ChoosePixelFormat(dc,&pfd);
    check(pf && SetPixelFormat(dc,pf,&pfd),"set pixel format");
    context=wglCreateContext(dc);
    check(context && wglMakeCurrent(dc,context),"create/select context");
    if (!context || wglGetCurrentContext()!=context) goto done;
#define LOAD(variable,type,name) variable=(type)wglGetProcAddress(name)
    LOAD(gen_frames,gen_fn,"glGenFramebuffers"); LOAD(bind_frame,bind_fn,"glBindFramebuffer");
    LOAD(attach,attach_fn,"glFramebufferTexture2D"); LOAD(status,status_fn,"glCheckFramebufferStatus");
    LOAD(delete_frames,delete_fn,"glDeleteFramebuffers");
    LOAD(gen_buffers,gen_fn,"glGenBuffers"); LOAD(bind_buffer,bind_fn,"glBindBuffer");
    LOAD(buffer_data,data_fn,"glBufferData"); LOAD(get_data,getdata_fn,"glGetBufferSubData");
    LOAD(delete_buffers,delete_fn,"glDeleteBuffers");
    check(gen_frames && bind_frame && attach && status && delete_frames && gen_buffers
        && bind_buffer && buffer_data && get_data && delete_buffers,"required GL entry points");
    if (!gen_frames || !bind_frame || !attach || !status || !delete_frames || !gen_buffers
        || !bind_buffer || !buffer_data || !get_data || !delete_buffers) goto done;
    pages=VirtualAlloc(NULL,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    check(pages!=NULL,"allocate page-crossing client buffer"); if (!pages) goto done;
    pixels=pages+4093;
    glGenTextures(1,&texture); glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,2,2,0,GL_RGBA,GL_UNSIGNED_BYTE,colors);
    gen_frames(1,&framebuffer); bind_frame(GL_FRAMEBUFFER,framebuffer);
    attach(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    glReadBuffer(GL_COLOR_ATTACHMENT0); glDrawBuffer(GL_COLOR_ATTACHMENT0);
    check(status(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"complete source framebuffer");
    check(glGetError()==GL_NO_ERROR,"setup GL error");
    gen_buffers(1,&pbo);
    for (n=0;n<sizeof(cases)/sizeof(cases[0]);++n)
    {
        const struct pack_case *c=&cases[n];
        unsigned char expected[128], initial[128];
        int row, stride=(c->row_length?c->row_length:c->width)*4;
        unsigned int i;
        GLint binding=-1, row_length=-1, skip_rows=-1, skip_pixels=-1, alignment=-1;
        stride=(stride+c->alignment-1)/c->alignment*c->alignment;
        memset(initial,0xa7,sizeof(initial)); memcpy(expected,initial,sizeof(expected));
        for (row=0;row<2;++row)
            memcpy(expected+c->offset+(c->skip_rows+row)*stride+c->skip_pixels*4,
                colors+row*8,c->width*4);
        memcpy(pixels,initial,sizeof(initial));
        glPixelStorei(GL_PACK_ROW_LENGTH,c->row_length); glPixelStorei(GL_PACK_SKIP_ROWS,c->skip_rows);
        glPixelStorei(GL_PACK_SKIP_PIXELS,c->skip_pixels); glPixelStorei(GL_PACK_ALIGNMENT,c->alignment);
        bind_buffer(GL_PIXEL_PACK_BUFFER,c->pbo?pbo:0);
        if (c->pbo) buffer_data(GL_PIXEL_PACK_BUFFER,sizeof(initial),initial,GL_STREAM_READ);
        check(glGetError()==GL_NO_ERROR,"%s pre-read error",c->name);
        glReadPixels(0,0,c->width,2,GL_RGBA,GL_UNSIGNED_BYTE,
            c->pbo?(void *)(ptrdiff_t)c->offset:pixels);
        check(glGetError()==GL_NO_ERROR,"%s read error",c->name);
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&binding);
        glGetIntegerv(GL_PACK_ROW_LENGTH,&row_length); glGetIntegerv(GL_PACK_SKIP_ROWS,&skip_rows);
        glGetIntegerv(GL_PACK_SKIP_PIXELS,&skip_pixels); glGetIntegerv(GL_PACK_ALIGNMENT,&alignment);
        check(binding==(GLint)(c->pbo?pbo:0) && row_length==c->row_length && skip_rows==c->skip_rows
            && skip_pixels==c->skip_pixels && alignment==c->alignment,"%s retained packing/binding",c->name);
        if (c->pbo) get_data(GL_PIXEL_PACK_BUFFER,0,128,pixels);
        check(glGetError()==GL_NO_ERROR,"%s result inspection error",c->name);
        for (i=0;i<sizeof(expected);++i)
            check(pixels[i]==expected[i],"%s byte %u expected %u got %u",c->name,i,expected[i],pixels[i]);
    }
    bind_buffer(GL_PIXEL_PACK_BUFFER,0);
    glPixelStorei(GL_PACK_ROW_LENGTH,0); glPixelStorei(GL_PACK_SKIP_ROWS,0);
    glPixelStorei(GL_PACK_SKIP_PIXELS,0); glPixelStorei(GL_PACK_ALIGNMENT,4);
    glReadBuffer(GL_NONE); memset(pixels,0xa7,128);
    check(glGetError()==GL_NO_ERROR,"rejected read setup");
    glReadPixels(0,0,2,2,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    check(glGetError()==GL_INVALID_OPERATION,"read with no selected color buffer must fail");
    for (n=0;n<128;++n)
        check(pixels[n]==0xa7,"rejected read changed byte %u to %u",n,pixels[n]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    /* The client wrapper must choose a correctly typed view for non-byte
     * readback too. All values are exactly representable, including floats. */
    {
        const GLfloat floats[8]={0.25f,0.5f,0.75f,1.0f, -1.0f,-0.5f,2.0f,4.0f};
        const GLint signed_values[8]={-1,-2,-3,-4, 1,2,3,4};
        const GLuint unsigned_values[8]={1,2,3,4, 0x80000000u,0xffffffffu,128,256};
        const struct typed_case { const char *name; GLenum internal, format, type; const void *values; } typed[] = {
            {"float",GL_RGBA32F,GL_RGBA,GL_FLOAT,floats},
            {"signed integer",GL_RGBA32I,GL_RGBA_INTEGER,GL_INT,signed_values},
            {"unsigned integer",GL_RGBA32UI,GL_RGBA_INTEGER,GL_UNSIGNED_INT,unsigned_values},
        };
        pixels=pages+4092; /* Aligned to 32-bit elements, across guest pages. */
        for (n=0;n<sizeof(typed)/sizeof(typed[0]);++n)
        {
            const struct typed_case *t=&typed[n];
            int use_pbo;
            glTexImage2D(GL_TEXTURE_2D,0,t->internal,1,2,0,t->format,t->type,t->values);
            check(status(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"%s complete framebuffer",t->name);
            check(glGetError()==GL_NO_ERROR,"%s setup error",t->name);
            for (use_pbo=0;use_pbo<2;++use_pbo)
            {
                unsigned char expected[256], initial[256];
                unsigned int i;
                int row, offset=use_pbo?16:0;
                GLint binding=-1, row_length=-1, skip_rows=-1, skip_pixels=-1, alignment=-1;
                memset(initial,0xa7,sizeof(initial)); memcpy(expected,initial,sizeof(expected));
                for (row=0;row<2;++row)
                    memcpy(expected+offset+(1+row)*48+16,(const char *)t->values+row*16,16);
                memcpy(pixels,initial,sizeof(initial));
                glPixelStorei(GL_PACK_ROW_LENGTH,3); glPixelStorei(GL_PACK_SKIP_ROWS,1);
                glPixelStorei(GL_PACK_SKIP_PIXELS,1); glPixelStorei(GL_PACK_ALIGNMENT,8);
                bind_buffer(GL_PIXEL_PACK_BUFFER,use_pbo?pbo:0);
                if (use_pbo) buffer_data(GL_PIXEL_PACK_BUFFER,sizeof(initial),initial,GL_STREAM_READ);
                check(glGetError()==GL_NO_ERROR,"%s PBO=%d pre-read error",t->name,use_pbo);
                glReadPixels(0,0,1,2,t->format,t->type,use_pbo?(void *)(ptrdiff_t)offset:pixels);
                check(glGetError()==GL_NO_ERROR,"%s PBO=%d read error",t->name,use_pbo);
                glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&binding);
                glGetIntegerv(GL_PACK_ROW_LENGTH,&row_length); glGetIntegerv(GL_PACK_SKIP_ROWS,&skip_rows);
                glGetIntegerv(GL_PACK_SKIP_PIXELS,&skip_pixels); glGetIntegerv(GL_PACK_ALIGNMENT,&alignment);
                check(binding==(GLint)(use_pbo?pbo:0) && row_length==3 && skip_rows==1
                    && skip_pixels==1 && alignment==8,"%s PBO=%d retained packing/binding",t->name,use_pbo);
                if (use_pbo) get_data(GL_PIXEL_PACK_BUFFER,0,sizeof(initial),pixels);
                check(glGetError()==GL_NO_ERROR,"%s PBO=%d result inspection error",t->name,use_pbo);
                for (i=0;i<sizeof(expected);++i)
                    check(pixels[i]==expected[i],"%s PBO=%d byte %u expected %u got %u",
                        t->name,use_pbo,i,expected[i],pixels[i]);
            }
        }
    }
    bind_buffer(GL_PIXEL_PACK_BUFFER,0);
    glPixelStorei(GL_PACK_ROW_LENGTH,0); glPixelStorei(GL_PACK_SKIP_ROWS,0);
    glPixelStorei(GL_PACK_SKIP_PIXELS,0); glPixelStorei(GL_PACK_ALIGNMENT,4);
    delete_buffers(1,&pbo);
    bind_frame(GL_FRAMEBUFFER,0); delete_frames(1,&framebuffer); glDeleteTextures(1,&texture);
    check(glGetError()==GL_NO_ERROR,"resource cleanup error");
done:
    if (pages) VirtualFree(pages,0,MEM_RELEASE);
    if (context) { wglMakeCurrent(NULL,NULL); wglDeleteContext(context); }
    if (dc) ReleaseDC(window,dc);
    if (window) DestroyWindow(window);
    UnregisterClassA(wc.lpszClassName,wc.hInstance);
    if (!failures) printf("PASS pixel pack layout, PBO offsets, sentinels and state\n");
    printf("Summary: %u passed, %u failed, 0 skipped\n",checks-failures,failures);
    return failures?1:0;
}
