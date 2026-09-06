#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

static int str_len(const char* s);
static void fail(const char* message);

static void check_initial_vertex_arrays(void) {
    GLint count = 0;
    GLint i;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &count);
    if (count < 8) fail("GLES maximum vertex attributes");
    for (i = 0; i < count; ++i) {
        GLint enabled = -1;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        if (enabled != GL_FALSE) fail("fresh GLES context has enabled vertex arrays");
    }
    if (glGetError() != GL_NO_ERROR) fail("GLES initial vertex array state");
}

static unsigned int compile_shader(unsigned int type, const char* source) {
    const char* sources[1] = { source };
    int lengths[1] = { str_len(source) };
    int compileStatus = 0;

    unsigned int shader = glCreateShader(type);
    if (!shader) {
        fail("glCreateShader");
    }
    glShaderSource(shader, 1, sources, lengths);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (glGetError() != GL_NO_ERROR) {
        fail("GLES shader compile glGetError");
    }
    if (compileStatus != GL_TRUE) {
        fail("GLES shader compile status");
    }
    return shader;
}

static void test_shader_compile(void) {
    const char* source = "attribute vec4 a_pos;\nvoid main() { gl_Position = a_pos; }\n";
    unsigned int shader = compile_shader(GL_VERTEX_SHADER, source);
    glDeleteShader(shader);
}

static void test_program_vbo_draw(void) {
    const char* vertexSource =
        "attribute vec2 a_pos;\n"
        "void main() { gl_Position = vec4(a_pos, 0.0, 1.0); }\n";
    const char* fragmentSource =
        "precision mediump float;\n"
        "void main() { gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); }\n";
    const float vertices[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f
    };
    unsigned char pixel[4] = {};
    unsigned int buffer = 0;
    int linkStatus = 0;

    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSource);
    unsigned int program = glCreateProgram();
    if (!program) {
        fail("glCreateProgram");
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glBindAttribLocation(program, 0, "a_pos");
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (glGetError() != GL_NO_ERROR) {
        fail("GLES program link glGetError");
    }
    if (linkStatus != GL_TRUE) {
        fail("GLES program link status");
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, (int)sizeof(vertices), vertices, GL_STATIC_DRAW);
    if (!buffer || glGetError() != GL_NO_ERROR) fail("GLES VBO upload");
    glUseProgram(program);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * (int)sizeof(float), (const void*)0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glReadPixels(2, 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glDeleteBuffers(1, &buffer);
    glDeleteProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (glGetError() != GL_NO_ERROR) {
        fail("GLES VBO draw glGetError");
    }
    if (pixel[0] > 20 || pixel[1] < 220 || pixel[2] > 20 || pixel[3] < 245) {
        fail("GLES VBO draw readback color");
    }
}

static void test_texture_sample_draw(void) {
    const char* vertexSource =
        "attribute vec2 a_pos;\n"
        "attribute vec2 a_uv;\n"
        "varying vec2 v_uv;\n"
        "void main() { v_uv = a_uv; gl_Position = vec4(a_pos, 0.0, 1.0); }\n";
    const char* fragmentSource =
        "precision mediump float;\n"
        "varying vec2 v_uv;\n"
        "uniform sampler2D u_tex;\n"
        "void main() { gl_FragColor = texture2D(u_tex, v_uv); }\n";
    const float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         3.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  3.0f, 0.0f, 1.0f
    };
    const unsigned char texel[4] = { 210, 40, 160, 255 };
    unsigned char pixel[4] = {};
    unsigned int buffer = 0;
    unsigned int texture = 0;
    int linkStatus = 0;

    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSource);
    unsigned int program = glCreateProgram();
    if (!program) {
        fail("glCreateProgram texture");
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glBindAttribLocation(program, 0, "a_pos");
    glBindAttribLocation(program, 1, "a_uv");
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (glGetError() != GL_NO_ERROR) {
        fail("GLES texture program link glGetError");
    }
    if (linkStatus != GL_TRUE) {
        fail("GLES texture program link status");
    }

    int sampler = glGetUniformLocation(program, "u_tex");
    if (sampler < 0) {
        fail("glGetUniformLocation texture sampler");
    }

    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, (int)sizeof(vertices), vertices, GL_STATIC_DRAW);
    glUseProgram(program);
    glUniform1i(sampler, 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * (int)sizeof(float), (const void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * (int)sizeof(float), (const void*)(2 * sizeof(float)));
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glReadPixels(2, 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDeleteBuffers(1, &buffer);
    glDeleteTextures(1, &texture);
    glDeleteProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (glGetError() != GL_NO_ERROR) {
        fail("GLES texture draw glGetError");
    }
    if (pixel[0] < 190 || pixel[0] > 230 ||
        pixel[1] < 20 || pixel[1] > 60 ||
        pixel[2] < 140 || pixel[2] > 180 ||
        pixel[3] < 245) {
        fail("GLES texture draw readback color");
    }
}

static int str_len(const char* s) {
    int n = 0;
    while (s[n]) {
        ++n;
    }
    return n;
}

static void write_str(const char* s) {
    int n = str_len(s);
    __asm__ volatile("int $0x80" : : "a"(4), "b"(1), "c"(s), "d"(n) : "memory");
}

static void write_hex(unsigned int value) {
    char text[9];
    int i;
    for (i = 0; i < 8; ++i) {
        text[i] = "0123456789abcdef"[(value >> (28 - i * 4)) & 15];
    }
    text[8] = 0;
    write_str(text);
}

static void exit_code(int code) {
    __asm__ volatile("int $0x80" : : "a"(1), "b"(code));
    for (;;) {
    }
}

static void fail(const char* message) {
    write_str("FAIL ");
    write_str(message);
    write_str("\n");
    exit_code(1);
}

static void pass(const char* message) {
    write_str("PASS ");
    write_str(message);
    write_str("\n");
    exit_code(0);
}

void _start(void) {
    // Wine fills its GL dispatch table through EGL before creating a context.
    PFNGLCLEARCOLORPROC clearColor = (PFNGLCLEARCOLORPROC)eglGetProcAddress("glClearColor");
    PFNGLGETSTRINGPROC getString = (PFNGLGETSTRINGPROC)eglGetProcAddress("glGetString");
    PFNGLFINISHPROC finish = (PFNGLFINISHPROC)eglGetProcAddress("glFinish");
    PFNGLFLUSHPROC flush = (PFNGLFLUSHPROC)eglGetProcAddress("glFlush");
    if (!clearColor || !getString || !finish || !flush ||
        eglGetProcAddress("glDefinitelyNotARealBoxedWineFunction")) {
        fail("eglGetProcAddress before context creation");
    }

    EGLint eglMajor = 0;
    EGLint eglMinor = 0;
    EGLint configCount = 0;
    EGLint contextVersion = 0;
    EGLConfig config = 0;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!display) {
        fail("eglGetDisplay");
    }
    if (!eglInitialize(display, &eglMajor, &eglMinor)) {
        fail("eglInitialize");
    }
    if (eglMajor != 1 || eglMinor < 4) {
        fail("eglInitialize version output");
    }
    if (!eglQueryString(display, EGL_VERSION)) {
        fail("eglQueryString");
    }
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        fail("eglBindAPI(EGL_OPENGL_ES_API)");
    }
    if (eglQueryAPI() != EGL_OPENGL_ES_API || eglGetError() != EGL_SUCCESS) {
        fail("EGL zero-argument return values");
    }

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_NONE
    };
    if (!eglChooseConfig(display, configAttribs, &config, 1, &configCount) || configCount < 1 || !config) {
        fail("eglChooseConfig");
    }

    EGLint surfaceAttribs[] = {
        EGL_WIDTH, 4,
        EGL_HEIGHT, 4,
        EGL_NONE
    };
    EGLSurface surface = eglCreatePbufferSurface(display, config, surfaceAttribs);
    if (!surface) {
        fail("eglCreatePbufferSurface");
    }
    EGLint width = 0;
    EGLint height = 0;
    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) || width != 4 ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height) || height != 4) {
        fail("eglQuerySurface output arguments");
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (!context) {
        fail("eglCreateContext");
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        fail("eglMakeCurrent");
    }
    if (eglGetCurrentContext() != context) {
        fail("eglGetCurrentContext");
    }
    if (!eglQueryContext(display, context, EGL_CONTEXT_CLIENT_VERSION, &contextVersion) || contextVersion < 2) {
        fail("eglQueryContext(EGL_CONTEXT_CLIENT_VERSION)");
    }
    check_initial_vertex_arrays();

    const unsigned char* version = getString(GL_VERSION);
    if (!version || !version[0]) {
        fail("glGetString(GL_VERSION)");
    }
    write_str("GL_VERSION: ");
    write_str((const char*)version);
    write_str("\n");
    int i;

    PFNEGLGETCURRENTCONTEXTPROC getContext = (PFNEGLGETCURRENTCONTEXTPROC)eglGetProcAddress("eglGetCurrentContext");
    if (!clearColor || !getContext || getContext() != context ||
        eglGetProcAddress("glDefinitelyNotARealBoxedWineFunction")) {
        fail("eglGetProcAddress");
    }

    // Cross the JIT warmup threshold and exercise callbacks that write back to
    // the caller's stack, as well as a GL function returned through libEGL.
    for (i = 0; i < 512; ++i) {
        width = 0;
        if (eglGetCurrentContext() != context ||
            !eglQuerySurface(display, surface, EGL_WIDTH, &width) || width != 4) {
            fail("repeated EGL calls");
        }
        clearColor(0.25f, 0.5f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (glGetError() != GL_NO_ERROR) {
            fail("repeated GLES calls");
        }
    }

    unsigned char pixel[4] = {};
    glViewport(0, 0, 4, 4);
    clearColor(0.25f, 0.5f, 0.75f, 1.0f);
    GLfloat clearState[4] = {-1, -1, -1, -1};
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearState);
    if (clearState[0] != 0.25f || clearState[1] != 0.5f ||
        clearState[2] != 0.75f || clearState[3] != 1.0f) {
        fail("GLES clear color state after warmup");
    }
    EGLContext secondary = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (!secondary || eglGetCurrentContext() != context || !eglDestroyContext(display, secondary)) {
        fail("EGL secondary context creation preserves current context");
    }
    clearState[0] = -1.0f;
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearState);
    if (clearState[0] != 0.25f || clearState[1] != 0.5f ||
        clearState[2] != 0.75f || clearState[3] != 1.0f) {
        fail("EGL secondary context creation preserves GL state");
    }
    glClear(GL_COLOR_BUFFER_BIT);
    flush();
    finish();
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    if (glGetError() != GL_NO_ERROR) {
        fail("GLES clear/readpixels glGetError");
    }
    if (pixel[0] < 55 || pixel[0] > 75 ||
        pixel[1] < 120 || pixel[1] > 140 ||
        pixel[2] < 180 || pixel[2] > 200 ||
        pixel[3] < 245) {
        write_str("RGBA: ");
        for (i = 0; i < 4; ++i) {
            write_hex(pixel[i]);
            write_str(" ");
        }
        write_str("\n");
        fail("GLES clear/readpixels color");
    }
    test_shader_compile();
    test_program_vbo_draw();
    test_texture_sample_draw();

    if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ||
        eglGetCurrentContext() != EGL_NO_CONTEXT ||
        !eglDestroyContext(display, context) || !eglDestroySurface(display, surface) ||
        !eglTerminate(display)) {
        fail("EGL context and surface cleanup");
    }
    pass("real ES pbuffer context, VBO draw, and texture sample");
}
