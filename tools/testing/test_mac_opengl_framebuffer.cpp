/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include "platformtypes.h"
#include "macOpenGL.h"
#include <cassert>
#include <cstdio>
#include <initializer_list>

void* macOpenGLChoosePixelFormat(U32, int major) {
    CGLPixelFormatAttribute attributes[] = {
        kCGLPFAAccelerated,
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)(major >= 3 ? kCGLOGLPVersion_3_2_Core : kCGLOGLPVersion_Legacy),
        (CGLPixelFormatAttribute)0
    };
    CGLPixelFormatObj format = nullptr;
    GLint count = 0;
    assert(CGLChoosePixelFormat(attributes, &format, &count) == kCGLNoError && format);
    return format;
}

template<typename T> static T load(const char* name) {
    void* proc = macOpenGLGetProcAddressFallback(name);
    assert(proc);
    return reinterpret_cast<T>(proc);
}

int main() {
    // Wine loads these before there is a current context.
    auto createFramebuffers = load<void(*)(GLsizei, GLuint*)>("glCreateFramebuffers");
    auto createRenderbuffers = load<void(*)(GLsizei, GLuint*)>("glCreateRenderbuffers");
    auto attach = load<void(*)(GLuint, GLenum, GLenum, GLuint)>("glNamedFramebufferRenderbuffer");
    auto drawBuffer = load<void(*)(GLuint, GLenum)>("glNamedFramebufferDrawBuffer");
    auto readBuffer = load<void(*)(GLuint, GLenum)>("glNamedFramebufferReadBuffer");
    auto getAttachment = load<void(*)(GLuint, GLenum, GLenum, GLint*)>("glGetNamedFramebufferAttachmentParameteriv");
    auto status = load<GLenum(*)(GLuint, GLenum)>("glCheckNamedFramebufferStatus");
    auto storage = load<void(*)(GLuint, GLsizei, GLenum, GLsizei, GLsizei)>("glNamedRenderbufferStorageMultisample");
    assert(!macOpenGLGetProcAddressFallback(nullptr));
    assert(!macOpenGLGetProcAddressFallback("glDefinitelyNotARealFunction"));
    assert(!macOpenGLGetProcAddressFallback("glCreateTextures")); // Not full DSA support.

    for (int major : {0, 3}) {
        void* context = macOpenGLCreateContext(1, major, 2, 0, 0, nullptr);
        assert(context && macOpenGLMakeCurrent(context));
        GLuint framebuffers[3], renderbuffers[3];
        createFramebuffers(2, framebuffers);
        createRenderbuffers(2, renderbuffers);
        assert(glIsFramebuffer(framebuffers[0]) && glIsRenderbuffer(renderbuffers[0]));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers[0]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[1]);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[0]);
        auto checkBindings = [&]() {
            GLint read, draw, renderbuffer;
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
            glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
            assert(read == framebuffers[0] && draw == framebuffers[1] && renderbuffer == renderbuffers[0]);
        };
        createFramebuffers(1, framebuffers + 2);
        createRenderbuffers(1, renderbuffers + 2);
        checkBindings();
        storage(renderbuffers[2], 0, GL_RGBA8, 8, 8);
        attach(framebuffers[2], GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffers[2]);
        drawBuffer(framebuffers[2], GL_COLOR_ATTACHMENT0);
        readBuffer(framebuffers[2], GL_COLOR_ATTACHMENT0);
        GLint attachment = 0;
        getAttachment(framebuffers[2], GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attachment);
        assert(attachment == renderbuffers[2]);
        for (GLenum target : {GL_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER}) {
            assert(status(framebuffers[2], target) == GL_FRAMEBUFFER_COMPLETE);
            checkBindings();
        }
        assert(glGetError() == GL_NO_ERROR);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[2]);
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        unsigned char pixel[4] = {};
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        glFinish(); // A surfaceless context must not try to present a Cocoa surface.
        assert(pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0 && pixel[3] == 255);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers[0]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[1]);

        // Resize through Wine's multisample path as well as the zero-sample path.
        GLint maxSamples = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        assert(maxSamples >= 2);
        storage(renderbuffers[2], 2, GL_RGBA8, 16, 16);
        assert(status(framebuffers[2], GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        checkBindings();
        assert(glGetError() == GL_NO_ERROR);

        GLuint reservedFramebuffer, reservedRenderbuffer;
        glGenFramebuffers(1, &reservedFramebuffer);
        glGenRenderbuffers(1, &reservedRenderbuffer);
        attach(reservedFramebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffers[2]);
        assert(glGetError() == GL_INVALID_OPERATION && !glIsFramebuffer(reservedFramebuffer));
        storage(reservedRenderbuffer, 0, GL_RGBA8, 8, 8);
        assert(glGetError() == GL_INVALID_OPERATION && !glIsRenderbuffer(reservedRenderbuffer));
        attach(0, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
        assert(glGetError() == GL_INVALID_OPERATION);
        assert(status(framebuffers[2], GL_TEXTURE_2D) == 0 && glGetError() == GL_INVALID_ENUM);
        GLuint sentinel = 123;
        createFramebuffers(-1, &sentinel);
        assert(glGetError() == GL_INVALID_VALUE && sentinel == 123);
        createRenderbuffers(-1, &sentinel);
        assert(glGetError() == GL_INVALID_VALUE && sentinel == 123);
        glEnable(0xffffffff);
        getAttachment(framebuffers[2], GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attachment);
        assert(glGetError() == GL_INVALID_ENUM && attachment == renderbuffers[2]);
        checkBindings();
        assert(glGetError() == GL_NO_ERROR);

        glDeleteFramebuffers(1, &reservedFramebuffer);
        glDeleteRenderbuffers(1, &reservedRenderbuffer);
        glDeleteFramebuffers(3, framebuffers);
        glDeleteRenderbuffers(3, renderbuffers);
        macOpenGLClearCurrent();
        macOpenGLDestroyContext(context);
    }
    std::puts("Native framebuffer fallback checks passed (legacy and core profiles)");
}
