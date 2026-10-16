/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */

#ifndef __MAC_OPENGL_FRAMEBUFFER_H__
#define __MAC_OPENGL_FRAMEBUFFER_H__

#include <cstring>

// Apple exposes framebuffer objects in both legacy and core contexts, but not
// the DSA calls Wine 11 uses for windows on a non-interactive desktop. Bind only
// the target we need and restore it without disturbing the other framebuffer.
namespace {
class MacFramebufferBinding {
public:
    MacFramebufferBinding(GLenum target, GLuint framebuffer) : target(target) {
        glGetIntegerv(target == GL_READ_FRAMEBUFFER ? GL_READ_FRAMEBUFFER_BINDING : GL_DRAW_FRAMEBUFFER_BINDING, &previous);
        glBindFramebuffer(target, framebuffer);
    }
    ~MacFramebufferBinding() { glBindFramebuffer(target, previous); }
private:
    GLenum target;
    GLint previous = 0;
};

class MacRenderbufferBinding {
public:
    MacRenderbufferBinding(GLuint renderbuffer) {
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &previous);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    }
    ~MacRenderbufferBinding() { glBindRenderbuffer(GL_RENDERBUFFER, previous); }
private:
    GLint previous = 0;
};

static void macFramebufferInvalidOperation() {
    // Attaching to the default framebuffer raises INVALID_OPERATION without
    // modifying an object or consuming a pre-existing GL error.
    MacFramebufferBinding binding(GL_DRAW_FRAMEBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
}

static bool macValidNamedFramebuffer(GLuint framebuffer, bool allowDefault) {
    if ((!framebuffer && allowDefault) || glIsFramebuffer(framebuffer)) return true;
    macFramebufferInvalidOperation();
    return false;
}

static void macCreateFramebuffers(GLsizei count, GLuint* framebuffers) {
    glGenFramebuffers(count, framebuffers);
    // Gen only reserves names; Create must instantiate the objects as well.
    for (GLsizei i = 0; i < count; ++i) {
        MacFramebufferBinding binding(GL_DRAW_FRAMEBUFFER, framebuffers[i]);
    }
}

static void macCreateRenderbuffers(GLsizei count, GLuint* renderbuffers) {
    glGenRenderbuffers(count, renderbuffers);
    for (GLsizei i = 0; i < count; ++i) {
        MacRenderbufferBinding binding(renderbuffers[i]);
    }
}

static void macNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum target, GLuint renderbuffer) {
    if (!macValidNamedFramebuffer(framebuffer, false)) return;
    MacFramebufferBinding binding(GL_DRAW_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachment, target, renderbuffer);
}

static void macNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buffer) {
    if (!macValidNamedFramebuffer(framebuffer, true)) return;
    MacFramebufferBinding binding(GL_DRAW_FRAMEBUFFER, framebuffer);
    glDrawBuffer(buffer);
}

static void macNamedFramebufferReadBuffer(GLuint framebuffer, GLenum buffer) {
    if (!macValidNamedFramebuffer(framebuffer, true)) return;
    MacFramebufferBinding binding(GL_READ_FRAMEBUFFER, framebuffer);
    glReadBuffer(buffer);
}

static void macGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params) {
    if (!macValidNamedFramebuffer(framebuffer, true)) return;
    MacFramebufferBinding binding(GL_DRAW_FRAMEBUFFER, framebuffer);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment, pname, params);
}

static GLenum macCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target) {
    if (target != GL_FRAMEBUFFER && target != GL_DRAW_FRAMEBUFFER && target != GL_READ_FRAMEBUFFER) {
        return glCheckFramebufferStatus(target); // Preserve INVALID_ENUM.
    }
    if (!macValidNamedFramebuffer(framebuffer, true)) return 0;
    // FRAMEBUFFER status is defined in terms of the draw framebuffer. Avoid
    // binding GL_FRAMEBUFFER, which would also change the read binding.
    MacFramebufferBinding binding(target == GL_READ_FRAMEBUFFER ? target : GL_DRAW_FRAMEBUFFER, framebuffer);
    return glCheckFramebufferStatus(target);
}

static void macNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum format, GLsizei width, GLsizei height) {
    if (!glIsRenderbuffer(renderbuffer)) {
        macFramebufferInvalidOperation();
        return;
    }
    MacRenderbufferBinding binding(renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);
}
}

void* macOpenGLGetProcAddressFallback(const char* name) {
    if (!name) return nullptr;
#define MAC_FRAMEBUFFER_PROC(name) if (!std::strcmp(nameString, "gl" #name)) return (void*)mac##name
    const char* nameString = name;
    MAC_FRAMEBUFFER_PROC(CreateFramebuffers);
    MAC_FRAMEBUFFER_PROC(CreateRenderbuffers);
    MAC_FRAMEBUFFER_PROC(NamedFramebufferRenderbuffer);
    MAC_FRAMEBUFFER_PROC(NamedFramebufferDrawBuffer);
    MAC_FRAMEBUFFER_PROC(NamedFramebufferReadBuffer);
    MAC_FRAMEBUFFER_PROC(GetNamedFramebufferAttachmentParameteriv);
    MAC_FRAMEBUFFER_PROC(CheckNamedFramebufferStatus);
    MAC_FRAMEBUFFER_PROC(NamedRenderbufferStorageMultisample);
#undef MAC_FRAMEBUFFER_PROC
    return nullptr;
}

#endif
