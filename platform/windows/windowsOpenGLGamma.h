/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#pragma once

// Private compatibility context: presentation never changes guest GL objects,
// state, framebuffer contents, or the host monitor's gamma tables.
class WindowsGammaPresenter {
public:
    HGLRC context = nullptr;
    ~WindowsGammaPresenter() { if (context && deleteContext) deleteContext(context); }

    bool create(HDC dc) {
        if (context) return true;
        auto createContext = reinterpret_cast<decltype(&::wglCreateContext)>(SDL_GL_GetProcAddress("wglCreateContext"));
        deleteContext = reinterpret_cast<decltype(&::wglDeleteContext)>(SDL_GL_GetProcAddress("wglDeleteContext"));
        return createContext && deleteContext && (context = createContext(dc));
    }

    bool draw(int width, int height, int hostWidth, int hostHeight,
            const OpenGLViewport& viewport, GLenum readBuffer, GLenum drawBuffer,
            const Uint16* ramp, Uint64 revision) {
        if (!initialized && !initialize()) return false;
        read(readBuffer);
        activeTexture(GL_TEXTURE0);
        bindTexture(GL_TEXTURE_2D, frame);
        if (width != frameWidth || height != frameHeight) {
            texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            frameWidth = width;
            frameHeight = height;
        }
        copyTexture(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
        activeTexture(GL_TEXTURE1);
        bindTexture(GL_TEXTURE_1D, lut);
        if (revision != uploadedRevision) {
            Uint16 rgb[256 * 3];
            for (int i = 0; i < 256; ++i)
                for (int c = 0; c < 3; ++c) rgb[i * 3 + c] = ramp[c * 256 + i];
            texImage1D(GL_TEXTURE_1D, 0, GL_RGB16, 256, 0, GL_RGB, GL_UNSIGNED_SHORT, rgb);
            uploadedRevision = revision;
        }
        drawBufferFn(drawBuffer);
        viewportFn(0, 0, hostWidth, hostHeight);
        clearColor(0, 0, 0, 1);
        clear(GL_COLOR_BUFFER_BIT);
        viewportFn(viewport.x, hostHeight - viewport.y - viewport.height, viewport.width, viewport.height);
        useProgram(program);
        begin(GL_TRIANGLE_STRIP);
        texCoord(0, 0); vertex(-1, -1);
        texCoord(1, 0); vertex( 1, -1);
        texCoord(0, 1); vertex(-1,  1);
        texCoord(1, 1); vertex( 1,  1);
        end();
        return true;
    }

private:
    decltype(&::wglDeleteContext) deleteContext = nullptr;
#define GL_MEMBER(type, member) type member = nullptr
    GL_MEMBER(decltype(&::glReadBuffer), read);
    GL_MEMBER(decltype(&::glDrawBuffer), drawBufferFn);
    GL_MEMBER(decltype(&::glViewport), viewportFn);
    GL_MEMBER(decltype(&::glClearColor), clearColor);
    GL_MEMBER(decltype(&::glClear), clear);
    GL_MEMBER(decltype(&::glGenTextures), genTextures);
    GL_MEMBER(decltype(&::glBindTexture), bindTexture);
    GL_MEMBER(decltype(&::glTexParameteri), texParameter);
    GL_MEMBER(decltype(&::glTexImage1D), texImage1D);
    GL_MEMBER(decltype(&::glTexImage2D), texImage2D);
    GL_MEMBER(decltype(&::glCopyTexSubImage2D), copyTexture);
    GL_MEMBER(decltype(&::glBegin), begin);
    GL_MEMBER(decltype(&::glEnd), end);
    GL_MEMBER(decltype(&::glTexCoord2f), texCoord);
    GL_MEMBER(decltype(&::glVertex2f), vertex);
    GL_MEMBER(PFNGLACTIVETEXTUREPROC, activeTexture);
    GL_MEMBER(PFNGLCREATESHADERPROC, createShader);
    GL_MEMBER(PFNGLSHADERSOURCEPROC, shaderSource);
    GL_MEMBER(PFNGLCOMPILESHADERPROC, compileShader);
    GL_MEMBER(PFNGLGETSHADERIVPROC, getShader);
    GL_MEMBER(PFNGLGETSHADERINFOLOGPROC, shaderLog);
    GL_MEMBER(PFNGLDELETESHADERPROC, deleteShader);
    GL_MEMBER(PFNGLCREATEPROGRAMPROC, createProgram);
    GL_MEMBER(PFNGLATTACHSHADERPROC, attachShader);
    GL_MEMBER(PFNGLLINKPROGRAMPROC, linkProgram);
    GL_MEMBER(PFNGLGETPROGRAMIVPROC, getProgram);
    GL_MEMBER(PFNGLUSEPROGRAMPROC, useProgram);
    GL_MEMBER(PFNGLGETUNIFORMLOCATIONPROC, uniformLocation);
    GL_MEMBER(PFNGLUNIFORM1IPROC, uniform1i);
#undef GL_MEMBER
    GLuint frame = 0, lut = 0, program = 0;
    int frameWidth = 0, frameHeight = 0;
    Uint64 uploadedRevision = 0;
    bool initialized = false;
    bool attempted = false;

    GLuint shader(GLenum type, const char* source) {
        GLuint result = createShader(type);
        shaderSource(result, 1, &source, nullptr);
        compileShader(result);
        GLint compiled = 0;
        getShader(result, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            char message[1024] = {};
            shaderLog(result, sizeof(message), nullptr, message);
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Gamma shader: %s", message);
            deleteShader(result);
            return 0;
        }
        return result;
    }

    bool initialize() {
        if (attempted) return false;
        attempted = true;
#define LOAD(member, name) member = reinterpret_cast<decltype(member)>(SDL_GL_GetProcAddress(name)); if (!member) { SDL_SetError("Gamma presentation requires %s", name); return false; }
        LOAD(read, "glReadBuffer"); LOAD(drawBufferFn, "glDrawBuffer");
        LOAD(viewportFn, "glViewport"); LOAD(clearColor, "glClearColor"); LOAD(clear, "glClear");
        LOAD(genTextures, "glGenTextures"); LOAD(bindTexture, "glBindTexture");
        LOAD(texParameter, "glTexParameteri"); LOAD(texImage1D, "glTexImage1D");
        LOAD(texImage2D, "glTexImage2D"); LOAD(copyTexture, "glCopyTexSubImage2D");
        LOAD(begin, "glBegin"); LOAD(end, "glEnd"); LOAD(texCoord, "glTexCoord2f"); LOAD(vertex, "glVertex2f");
        LOAD(activeTexture, "glActiveTexture"); LOAD(createShader, "glCreateShader");
        LOAD(shaderSource, "glShaderSource"); LOAD(compileShader, "glCompileShader");
        LOAD(getShader, "glGetShaderiv"); LOAD(shaderLog, "glGetShaderInfoLog"); LOAD(deleteShader, "glDeleteShader");
        LOAD(createProgram, "glCreateProgram"); LOAD(attachShader, "glAttachShader");
        LOAD(linkProgram, "glLinkProgram"); LOAD(getProgram, "glGetProgramiv"); LOAD(useProgram, "glUseProgram");
        LOAD(uniformLocation, "glGetUniformLocation"); LOAD(uniform1i, "glUniform1i");
#undef LOAD
        const char* vs = "#version 120\nvoid main(){gl_Position=gl_Vertex;gl_TexCoord[0]=gl_MultiTexCoord0;}";
        const char* fs = "#version 120\nuniform sampler2D frame;uniform sampler1D ramp;\n"
            "void main(){vec4 c=texture2D(frame,gl_TexCoord[0].xy);vec3 t=(c.rgb*255.0+0.5)/256.0;"
            "gl_FragColor=vec4(texture1D(ramp,t.r).r,texture1D(ramp,t.g).g,texture1D(ramp,t.b).b,c.a);}";
        GLuint v = shader(GL_VERTEX_SHADER, vs), f = shader(GL_FRAGMENT_SHADER, fs);
        if (!v || !f) { if (v) deleteShader(v); if (f) deleteShader(f); return false; }
        program = createProgram();
        attachShader(program, v); attachShader(program, f); linkProgram(program);
        deleteShader(v); deleteShader(f);
        GLint linked = 0; getProgram(program, GL_LINK_STATUS, &linked);
        if (!linked) { SDL_SetError("Could not link gamma presentation shader"); return false; }
        useProgram(program);
        uniform1i(uniformLocation(program, "frame"), 0);
        uniform1i(uniformLocation(program, "ramp"), 1);
        genTextures(1, &frame); genTextures(1, &lut);
        activeTexture(GL_TEXTURE0); bindTexture(GL_TEXTURE_2D, frame);
        texParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        texParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        texParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        activeTexture(GL_TEXTURE1); bindTexture(GL_TEXTURE_1D, lut);
        texParameter(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texParameter(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        texParameter(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        initialized = true;
        return true;
    }
};
