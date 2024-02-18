#include "boxedwine.h"

#ifndef DISABLE_GL_EXTENSIONS
#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"
#include "glMarshal.h"

void glcommon_glGetUniformfv(CPU* cpu) {
    if (!ext_glGetUniformfv)
        kpanic("ext_glGetUniformfv is NULL");
    {
    GL_FUNC(ext_glGetUniformfv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformfv GLuint program=%d, GLint location=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformfvARB(CPU* cpu) {
    if (!ext_glGetUniformfvARB)
        kpanic("ext_glGetUniformfvARB is NULL");
    {
    GL_FUNC(ext_glGetUniformfvARB)(INDEX_TO_HANDLE(hARG1), ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformfvARB GLhandleARB programObj=%d, GLint location=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformi64vARB(CPU* cpu) {
    if (!ext_glGetUniformi64vARB)
        kpanic("ext_glGetUniformi64vARB is NULL");
    {
    GL_FUNC(ext_glGetUniformi64vARB)(ARG1, ARG2, (GLint64*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformi64vARB GLuint program=%d, GLint location=%d, GLint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformi64vNV(CPU* cpu) {
    if (!ext_glGetUniformi64vNV)
        kpanic("ext_glGetUniformi64vNV is NULL");
    {
    GL_FUNC(ext_glGetUniformi64vNV)(ARG1, ARG2, (GLint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformi64vNV GLuint program=%d, GLint location=%d, GLint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformiv(CPU* cpu) {
    if (!ext_glGetUniformiv)
        kpanic("ext_glGetUniformiv is NULL");
    {
    GL_FUNC(ext_glGetUniformiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformiv GLuint program=%d, GLint location=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformivARB(CPU* cpu) {
    if (!ext_glGetUniformivARB)
        kpanic("ext_glGetUniformivARB is NULL");
    {
    GL_FUNC(ext_glGetUniformivARB)(INDEX_TO_HANDLE(hARG1), ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformivARB GLhandleARB programObj=%d, GLint location=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformui64vARB(CPU* cpu) {
    if (!ext_glGetUniformui64vARB)
        kpanic("ext_glGetUniformui64vARB is NULL");
    {
    GL_FUNC(ext_glGetUniformui64vARB)(ARG1, ARG2, (GLuint64*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformui64vARB GLuint program=%d, GLint location=%d, GLuint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformui64vNV(CPU* cpu) {
    if (!ext_glGetUniformui64vNV)
        kpanic("ext_glGetUniformui64vNV is NULL");
    {
    GL_FUNC(ext_glGetUniformui64vNV)(ARG1, ARG2, (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformui64vNV GLuint program=%d, GLint location=%d, GLuint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformuiv(CPU* cpu) {
    if (!ext_glGetUniformuiv)
        kpanic("ext_glGetUniformuiv is NULL");
    {
    GL_FUNC(ext_glGetUniformuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformuiv GLuint program=%d, GLint location=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformuivEXT(CPU* cpu) {
    if (!ext_glGetUniformuivEXT)
        kpanic("ext_glGetUniformuivEXT is NULL");
    {
    GL_FUNC(ext_glGetUniformuivEXT)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformuivEXT GLuint program=%d, GLint location=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVariantArrayObjectfvATI(CPU* cpu) {
    if (!ext_glGetVariantArrayObjectfvATI)
        kpanic("ext_glGetVariantArrayObjectfvATI is NULL");
    {
    GL_FUNC(ext_glGetVariantArrayObjectfvATI)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVariantArrayObjectfvATI GLuint id=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVariantArrayObjectivATI(CPU* cpu) {
    if (!ext_glGetVariantArrayObjectivATI)
        kpanic("ext_glGetVariantArrayObjectivATI is NULL");
    {
    GL_FUNC(ext_glGetVariantArrayObjectivATI)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVariantArrayObjectivATI GLuint id=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVariantBooleanvEXT(CPU* cpu) {
    if (!ext_glGetVariantBooleanvEXT)
        kpanic("ext_glGetVariantBooleanvEXT is NULL");
    {
    GL_FUNC(ext_glGetVariantBooleanvEXT)(ARG1, ARG2, (GLboolean*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVariantBooleanvEXT GLuint id=%d, GLenum value=%d, GLboolean* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVariantFloatvEXT(CPU* cpu) {
    if (!ext_glGetVariantFloatvEXT)
        kpanic("ext_glGetVariantFloatvEXT is NULL");
    {
    GL_FUNC(ext_glGetVariantFloatvEXT)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVariantFloatvEXT GLuint id=%d, GLenum value=%d, GLfloat* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVariantIntegervEXT(CPU* cpu) {
    if (!ext_glGetVariantIntegervEXT)
        kpanic("ext_glGetVariantIntegervEXT is NULL");
    {
    GL_FUNC(ext_glGetVariantIntegervEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVariantIntegervEXT GLuint id=%d, GLenum value=%d, GLint* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVariantPointervEXT(CPU* cpu) {
    if (!ext_glGetVariantPointervEXT)
        kpanic("ext_glGetVariantPointervEXT is NULL");
    {
    void* p;GL_FUNC(ext_glGetVariantPointervEXT)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetVariantPointervEXT GLuint id=%d, GLenum value=%d, void** data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVaryingLocationNV(CPU* cpu) {
    if (!ext_glGetVaryingLocationNV)
        kpanic("ext_glGetVaryingLocationNV is NULL");
    {
    EAX=GL_FUNC(ext_glGetVaryingLocationNV)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glGetVaryingLocationNV GLuint program=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetVertexArrayIndexed64iv(CPU* cpu) {
    if (!ext_glGetVertexArrayIndexed64iv)
        kpanic("ext_glGetVertexArrayIndexed64iv is NULL");
    {
    GL_FUNC(ext_glGetVertexArrayIndexed64iv)(ARG1, ARG2, ARG3, (GLint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetVertexArrayIndexed64iv GLuint vaobj=%d, GLuint index=%d, GLenum pname=%d, GLint64* param=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetVertexArrayIndexediv(CPU* cpu) {
    if (!ext_glGetVertexArrayIndexediv)
        kpanic("ext_glGetVertexArrayIndexediv is NULL");
    {
    GL_FUNC(ext_glGetVertexArrayIndexediv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetVertexArrayIndexediv GLuint vaobj=%d, GLuint index=%d, GLenum pname=%d, GLint* param=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetVertexArrayIntegeri_vEXT(CPU* cpu) {
    if (!ext_glGetVertexArrayIntegeri_vEXT)
        kpanic("ext_glGetVertexArrayIntegeri_vEXT is NULL");
    {
    GL_FUNC(ext_glGetVertexArrayIntegeri_vEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetVertexArrayIntegeri_vEXT GLuint vaobj=%d, GLuint index=%d, GLenum pname=%d, GLint* param=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetVertexArrayIntegervEXT(CPU* cpu) {
    if (!ext_glGetVertexArrayIntegervEXT)
        kpanic("ext_glGetVertexArrayIntegervEXT is NULL");
    {
    GL_FUNC(ext_glGetVertexArrayIntegervEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexArrayIntegervEXT GLuint vaobj=%d, GLenum pname=%d, GLint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexArrayPointeri_vEXT(CPU* cpu) {
    if (!ext_glGetVertexArrayPointeri_vEXT)
        kpanic("ext_glGetVertexArrayPointeri_vEXT is NULL");
    {
    void* p;GL_FUNC(ext_glGetVertexArrayPointeri_vEXT)(ARG1, ARG2, ARG3, &p);
    cpu->memory->writed(ARG4, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetVertexArrayPointeri_vEXT GLuint vaobj=%d, GLuint index=%d, GLenum pname=%d, void** param=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetVertexArrayPointervEXT(CPU* cpu) {
    if (!ext_glGetVertexArrayPointervEXT)
        kpanic("ext_glGetVertexArrayPointervEXT is NULL");
    {
    void* p;GL_FUNC(ext_glGetVertexArrayPointervEXT)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetVertexArrayPointervEXT GLuint vaobj=%d, GLenum pname=%d, void** param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexArrayiv(CPU* cpu) {
    if (!ext_glGetVertexArrayiv)
        kpanic("ext_glGetVertexArrayiv is NULL");
    {
    GL_FUNC(ext_glGetVertexArrayiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexArrayiv GLuint vaobj=%d, GLenum pname=%d, GLint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribArrayObjectfvATI(CPU* cpu) {
    if (!ext_glGetVertexAttribArrayObjectfvATI)
        kpanic("ext_glGetVertexAttribArrayObjectfvATI is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribArrayObjectfvATI)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribArrayObjectfvATI GLuint index=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribArrayObjectivATI(CPU* cpu) {
    if (!ext_glGetVertexAttribArrayObjectivATI)
        kpanic("ext_glGetVertexAttribArrayObjectivATI is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribArrayObjectivATI)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribArrayObjectivATI GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribIiv(CPU* cpu) {
    if (!ext_glGetVertexAttribIiv)
        kpanic("ext_glGetVertexAttribIiv is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribIiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribIiv GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribIivEXT(CPU* cpu) {
    if (!ext_glGetVertexAttribIivEXT)
        kpanic("ext_glGetVertexAttribIivEXT is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribIivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribIivEXT GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribIuiv(CPU* cpu) {
    if (!ext_glGetVertexAttribIuiv)
        kpanic("ext_glGetVertexAttribIuiv is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribIuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribIuiv GLuint index=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribIuivEXT(CPU* cpu) {
    if (!ext_glGetVertexAttribIuivEXT)
        kpanic("ext_glGetVertexAttribIuivEXT is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribIuivEXT)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribIuivEXT GLuint index=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribLdv(CPU* cpu) {
    if (!ext_glGetVertexAttribLdv)
        kpanic("ext_glGetVertexAttribLdv is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribLdv)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribLdv GLuint index=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribLdvEXT(CPU* cpu) {
    if (!ext_glGetVertexAttribLdvEXT)
        kpanic("ext_glGetVertexAttribLdvEXT is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribLdvEXT)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribLdvEXT GLuint index=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribLi64vNV(CPU* cpu) {
    if (!ext_glGetVertexAttribLi64vNV)
        kpanic("ext_glGetVertexAttribLi64vNV is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribLi64vNV)(ARG1, ARG2, (GLint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribLi64vNV GLuint index=%d, GLenum pname=%d, GLint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribLui64vARB(CPU* cpu) {
    if (!ext_glGetVertexAttribLui64vARB)
        kpanic("ext_glGetVertexAttribLui64vARB is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribLui64vARB)(ARG1, ARG2, (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribLui64vARB GLuint index=%d, GLenum pname=%d, GLuint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribLui64vNV(CPU* cpu) {
    if (!ext_glGetVertexAttribLui64vNV)
        kpanic("ext_glGetVertexAttribLui64vNV is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribLui64vNV)(ARG1, ARG2, (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribLui64vNV GLuint index=%d, GLenum pname=%d, GLuint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribPointerv(CPU* cpu) {
    if (!ext_glGetVertexAttribPointerv)
        kpanic("ext_glGetVertexAttribPointerv is NULL");
    {
    void* p;GL_FUNC(ext_glGetVertexAttribPointerv)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetVertexAttribPointerv GLuint index=%d, GLenum pname=%d, void** pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribPointervARB(CPU* cpu) {
    if (!ext_glGetVertexAttribPointervARB)
        kpanic("ext_glGetVertexAttribPointervARB is NULL");
    {
    void* p;GL_FUNC(ext_glGetVertexAttribPointervARB)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetVertexAttribPointervARB GLuint index=%d, GLenum pname=%d, void** pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribPointervNV(CPU* cpu) {
    if (!ext_glGetVertexAttribPointervNV)
        kpanic("ext_glGetVertexAttribPointervNV is NULL");
    {
    void* p;GL_FUNC(ext_glGetVertexAttribPointervNV)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetVertexAttribPointervNV GLuint index=%d, GLenum pname=%d, void** pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribdv(CPU* cpu) {
    if (!ext_glGetVertexAttribdv)
        kpanic("ext_glGetVertexAttribdv is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribdv)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribdv GLuint index=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribdvARB(CPU* cpu) {
    if (!ext_glGetVertexAttribdvARB)
        kpanic("ext_glGetVertexAttribdvARB is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribdvARB)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribdvARB GLuint index=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribdvNV(CPU* cpu) {
    if (!ext_glGetVertexAttribdvNV)
        kpanic("ext_glGetVertexAttribdvNV is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribdvNV)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribdvNV GLuint index=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribfv(CPU* cpu) {
    if (!ext_glGetVertexAttribfv)
        kpanic("ext_glGetVertexAttribfv is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribfv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribfv GLuint index=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribfvARB(CPU* cpu) {
    if (!ext_glGetVertexAttribfvARB)
        kpanic("ext_glGetVertexAttribfvARB is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribfvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribfvARB GLuint index=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribfvNV(CPU* cpu) {
    if (!ext_glGetVertexAttribfvNV)
        kpanic("ext_glGetVertexAttribfvNV is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribfvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribfvNV GLuint index=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribiv(CPU* cpu) {
    if (!ext_glGetVertexAttribiv)
        kpanic("ext_glGetVertexAttribiv is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribiv GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribivARB(CPU* cpu) {
    if (!ext_glGetVertexAttribivARB)
        kpanic("ext_glGetVertexAttribivARB is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribivARB)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribivARB GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVertexAttribivNV(CPU* cpu) {
    if (!ext_glGetVertexAttribivNV)
        kpanic("ext_glGetVertexAttribivNV is NULL");
    {
    GL_FUNC(ext_glGetVertexAttribivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVertexAttribivNV GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVideoCaptureStreamdvNV(CPU* cpu) {
    if (!ext_glGetVideoCaptureStreamdvNV)
        kpanic("ext_glGetVideoCaptureStreamdvNV is NULL");
    {
    GL_FUNC(ext_glGetVideoCaptureStreamdvNV)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetVideoCaptureStreamdvNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetVideoCaptureStreamfvNV(CPU* cpu) {
    if (!ext_glGetVideoCaptureStreamfvNV)
        kpanic("ext_glGetVideoCaptureStreamfvNV is NULL");
    {
    GL_FUNC(ext_glGetVideoCaptureStreamfvNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetVideoCaptureStreamfvNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetVideoCaptureStreamivNV(CPU* cpu) {
    if (!ext_glGetVideoCaptureStreamivNV)
        kpanic("ext_glGetVideoCaptureStreamivNV is NULL");
    {
    GL_FUNC(ext_glGetVideoCaptureStreamivNV)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetVideoCaptureStreamivNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetVideoCaptureivNV(CPU* cpu) {
    if (!ext_glGetVideoCaptureivNV)
        kpanic("ext_glGetVideoCaptureivNV is NULL");
    {
    GL_FUNC(ext_glGetVideoCaptureivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVideoCaptureivNV GLuint video_capture_slot=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVideoi64vNV(CPU* cpu) {
    if (!ext_glGetVideoi64vNV)
        kpanic("ext_glGetVideoi64vNV is NULL");
    {
    GL_FUNC(ext_glGetVideoi64vNV)(ARG1, ARG2, (GLint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVideoi64vNV GLuint video_slot=%d, GLenum pname=%d, GLint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVideoivNV(CPU* cpu) {
    if (!ext_glGetVideoivNV)
        kpanic("ext_glGetVideoivNV is NULL");
    {
    GL_FUNC(ext_glGetVideoivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVideoivNV GLuint video_slot=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVideoui64vNV(CPU* cpu) {
    if (!ext_glGetVideoui64vNV)
        kpanic("ext_glGetVideoui64vNV is NULL");
    {
    GL_FUNC(ext_glGetVideoui64vNV)(ARG1, ARG2, (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVideoui64vNV GLuint video_slot=%d, GLenum pname=%d, GLuint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetVideouivNV(CPU* cpu) {
    if (!ext_glGetVideouivNV)
        kpanic("ext_glGetVideouivNV is NULL");
    {
    GL_FUNC(ext_glGetVideouivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetVideouivNV GLuint video_slot=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetnColorTable(CPU* cpu) {
    if (!ext_glGetnColorTable)
        kpanic("ext_glGetnColorTable is NULL");
    {
    GL_FUNC(ext_glGetnColorTable)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetnColorTable GLenum target=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* table=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetnColorTableARB(CPU* cpu) {
    if (!ext_glGetnColorTableARB)
        kpanic("ext_glGetnColorTableARB is NULL");
    {
    GL_FUNC(ext_glGetnColorTableARB)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetnColorTableARB GLenum target=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* table=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetnCompressedTexImage(CPU* cpu) {
    if (!ext_glGetnCompressedTexImage)
        kpanic("ext_glGetnCompressedTexImage is NULL");
    {
    GL_FUNC(ext_glGetnCompressedTexImage)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnCompressedTexImage GLenum target=%d, GLint lod=%d, GLsizei bufSize=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnCompressedTexImageARB(CPU* cpu) {
    if (!ext_glGetnCompressedTexImageARB)
        kpanic("ext_glGetnCompressedTexImageARB is NULL");
    {
    GL_FUNC(ext_glGetnCompressedTexImageARB)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnCompressedTexImageARB GLenum target=%d, GLint lod=%d, GLsizei bufSize=%d, void* img=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnConvolutionFilter(CPU* cpu) {
    if (!ext_glGetnConvolutionFilter)
        kpanic("ext_glGetnConvolutionFilter is NULL");
    {
    GL_FUNC(ext_glGetnConvolutionFilter)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetnConvolutionFilter GLenum target=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* image=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetnConvolutionFilterARB(CPU* cpu) {
    if (!ext_glGetnConvolutionFilterARB)
        kpanic("ext_glGetnConvolutionFilterARB is NULL");
    {
    GL_FUNC(ext_glGetnConvolutionFilterARB)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetnConvolutionFilterARB GLenum target=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* image=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetnHistogram(CPU* cpu) {
    if (!ext_glGetnHistogram)
        kpanic("ext_glGetnHistogram is NULL");
    {
    GL_FUNC(ext_glGetnHistogram)(ARG1, bARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetnHistogram GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetnHistogramARB(CPU* cpu) {
    if (!ext_glGetnHistogramARB)
        kpanic("ext_glGetnHistogramARB is NULL");
    {
    GL_FUNC(ext_glGetnHistogramARB)(ARG1, bARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetnHistogramARB GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetnMapdv(CPU* cpu) {
    if (!ext_glGetnMapdv)
        kpanic("ext_glGetnMapdv is NULL");
    {
    GL_FUNC(ext_glGetnMapdv)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnMapdv GLenum target=%d, GLenum query=%d, GLsizei bufSize=%d, GLdouble* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnMapdvARB(CPU* cpu) {
    if (!ext_glGetnMapdvARB)
        kpanic("ext_glGetnMapdvARB is NULL");
    {
    GL_FUNC(ext_glGetnMapdvARB)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnMapdvARB GLenum target=%d, GLenum query=%d, GLsizei bufSize=%d, GLdouble* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnMapfv(CPU* cpu) {
    if (!ext_glGetnMapfv)
        kpanic("ext_glGetnMapfv is NULL");
    {
    GL_FUNC(ext_glGetnMapfv)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnMapfv GLenum target=%d, GLenum query=%d, GLsizei bufSize=%d, GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnMapfvARB(CPU* cpu) {
    if (!ext_glGetnMapfvARB)
        kpanic("ext_glGetnMapfvARB is NULL");
    {
    GL_FUNC(ext_glGetnMapfvARB)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnMapfvARB GLenum target=%d, GLenum query=%d, GLsizei bufSize=%d, GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnMapiv(CPU* cpu) {
    if (!ext_glGetnMapiv)
        kpanic("ext_glGetnMapiv is NULL");
    {
    GL_FUNC(ext_glGetnMapiv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnMapiv GLenum target=%d, GLenum query=%d, GLsizei bufSize=%d, GLint* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnMapivARB(CPU* cpu) {
    if (!ext_glGetnMapivARB)
        kpanic("ext_glGetnMapivARB is NULL");
    {
    GL_FUNC(ext_glGetnMapivARB)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnMapivARB GLenum target=%d, GLenum query=%d, GLsizei bufSize=%d, GLint* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnMinmax(CPU* cpu) {
    if (!ext_glGetnMinmax)
        kpanic("ext_glGetnMinmax is NULL");
    {
    GL_FUNC(ext_glGetnMinmax)(ARG1, bARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetnMinmax GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetnMinmaxARB(CPU* cpu) {
    if (!ext_glGetnMinmaxARB)
        kpanic("ext_glGetnMinmaxARB is NULL");
    {
    GL_FUNC(ext_glGetnMinmaxARB)(ARG1, bARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetnMinmaxARB GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetnPixelMapfv(CPU* cpu) {
    if (!ext_glGetnPixelMapfv)
        kpanic("ext_glGetnPixelMapfv is NULL");
    {
    GL_FUNC(ext_glGetnPixelMapfv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetnPixelMapfv GLenum map=%d, GLsizei bufSize=%d, GLfloat* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetnPixelMapfvARB(CPU* cpu) {
    if (!ext_glGetnPixelMapfvARB)
        kpanic("ext_glGetnPixelMapfvARB is NULL");
    {
    GL_FUNC(ext_glGetnPixelMapfvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetnPixelMapfvARB GLenum map=%d, GLsizei bufSize=%d, GLfloat* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetnPixelMapuiv(CPU* cpu) {
    if (!ext_glGetnPixelMapuiv)
        kpanic("ext_glGetnPixelMapuiv is NULL");
    {
    GL_FUNC(ext_glGetnPixelMapuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetnPixelMapuiv GLenum map=%d, GLsizei bufSize=%d, GLuint* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetnPixelMapuivARB(CPU* cpu) {
    if (!ext_glGetnPixelMapuivARB)
        kpanic("ext_glGetnPixelMapuivARB is NULL");
    {
    GL_FUNC(ext_glGetnPixelMapuivARB)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetnPixelMapuivARB GLenum map=%d, GLsizei bufSize=%d, GLuint* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetnPixelMapusv(CPU* cpu) {
    if (!ext_glGetnPixelMapusv)
        kpanic("ext_glGetnPixelMapusv is NULL");
    {
    GL_FUNC(ext_glGetnPixelMapusv)(ARG1, ARG2, (GLushort*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetnPixelMapusv GLenum map=%d, GLsizei bufSize=%d, GLushort* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetnPixelMapusvARB(CPU* cpu) {
    if (!ext_glGetnPixelMapusvARB)
        kpanic("ext_glGetnPixelMapusvARB is NULL");
    {
    GL_FUNC(ext_glGetnPixelMapusvARB)(ARG1, ARG2, (GLushort*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetnPixelMapusvARB GLenum map=%d, GLsizei bufSize=%d, GLushort* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetnPolygonStipple(CPU* cpu) {
    if (!ext_glGetnPolygonStipple)
        kpanic("ext_glGetnPolygonStipple is NULL");
    {
    GL_FUNC(ext_glGetnPolygonStipple)(ARG1, (GLubyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetnPolygonStipple GLsizei bufSize=%d, GLubyte* pattern=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetnPolygonStippleARB(CPU* cpu) {
    if (!ext_glGetnPolygonStippleARB)
        kpanic("ext_glGetnPolygonStippleARB is NULL");
    {
    GL_FUNC(ext_glGetnPolygonStippleARB)(ARG1, (GLubyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetnPolygonStippleARB GLsizei bufSize=%d, GLubyte* pattern=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetnSeparableFilter(CPU* cpu) {
    if (!ext_glGetnSeparableFilter)
        kpanic("ext_glGetnSeparableFilter is NULL");
    {
    GL_FUNC(ext_glGetnSeparableFilter)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0), ARG6, (void*)marshalp(cpu, 0, ARG7, 0), (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glGetnSeparableFilter GLenum target=%d, GLenum format=%d, GLenum type=%d, GLsizei rowBufSize=%d, void* row=%.08x, GLsizei columnBufSize=%d, void* column=%.08x, void* span=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glGetnSeparableFilterARB(CPU* cpu) {
    if (!ext_glGetnSeparableFilterARB)
        kpanic("ext_glGetnSeparableFilterARB is NULL");
    {
    GL_FUNC(ext_glGetnSeparableFilterARB)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0), ARG6, (void*)marshalp(cpu, 0, ARG7, 0), (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glGetnSeparableFilterARB GLenum target=%d, GLenum format=%d, GLenum type=%d, GLsizei rowBufSize=%d, void* row=%.08x, GLsizei columnBufSize=%d, void* column=%.08x, void* span=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glGetnTexImage(CPU* cpu) {
    if (!ext_glGetnTexImage)
        kpanic("ext_glGetnTexImage is NULL");
    {
    GL_FUNC(ext_glGetnTexImage)(ARG1, ARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetnTexImage GLenum target=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetnTexImageARB(CPU* cpu) {
    if (!ext_glGetnTexImageARB)
        kpanic("ext_glGetnTexImageARB is NULL");
    {
    GL_FUNC(ext_glGetnTexImageARB)(ARG1, ARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetnTexImageARB GLenum target=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* img=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetnUniformdv(CPU* cpu) {
    if (!ext_glGetnUniformdv)
        kpanic("ext_glGetnUniformdv is NULL");
    {
    GL_FUNC(ext_glGetnUniformdv)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformdv GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformdvARB(CPU* cpu) {
    if (!ext_glGetnUniformdvARB)
        kpanic("ext_glGetnUniformdvARB is NULL");
    {
    GL_FUNC(ext_glGetnUniformdvARB)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformdvARB GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformfv(CPU* cpu) {
    if (!ext_glGetnUniformfv)
        kpanic("ext_glGetnUniformfv is NULL");
    {
    GL_FUNC(ext_glGetnUniformfv)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformfv GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformfvARB(CPU* cpu) {
    if (!ext_glGetnUniformfvARB)
        kpanic("ext_glGetnUniformfvARB is NULL");
    {
    GL_FUNC(ext_glGetnUniformfvARB)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformfvARB GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformi64vARB(CPU* cpu) {
    if (!ext_glGetnUniformi64vARB)
        kpanic("ext_glGetnUniformi64vARB is NULL");
    {
    GL_FUNC(ext_glGetnUniformi64vARB)(ARG1, ARG2, ARG3, (GLint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformi64vARB GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLint64* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformiv(CPU* cpu) {
    if (!ext_glGetnUniformiv)
        kpanic("ext_glGetnUniformiv is NULL");
    {
    GL_FUNC(ext_glGetnUniformiv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformiv GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformivARB(CPU* cpu) {
    if (!ext_glGetnUniformivARB)
        kpanic("ext_glGetnUniformivARB is NULL");
    {
    GL_FUNC(ext_glGetnUniformivARB)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformivARB GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformui64vARB(CPU* cpu) {
    if (!ext_glGetnUniformui64vARB)
        kpanic("ext_glGetnUniformui64vARB is NULL");
    {
    GL_FUNC(ext_glGetnUniformui64vARB)(ARG1, ARG2, ARG3, (GLuint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformui64vARB GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLuint64* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformuiv(CPU* cpu) {
    if (!ext_glGetnUniformuiv)
        kpanic("ext_glGetnUniformuiv is NULL");
    {
    GL_FUNC(ext_glGetnUniformuiv)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformuiv GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetnUniformuivARB(CPU* cpu) {
    if (!ext_glGetnUniformuivARB)
        kpanic("ext_glGetnUniformuivARB is NULL");
    {
    GL_FUNC(ext_glGetnUniformuivARB)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetnUniformuivARB GLuint program=%d, GLint location=%d, GLsizei bufSize=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGlobalAlphaFactorbSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactorbSUN)
        kpanic("ext_glGlobalAlphaFactorbSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactorbSUN)(bARG1);
    GL_LOG ("glGlobalAlphaFactorbSUN GLbyte factor=%d",bARG1);
    }
}
void glcommon_glGlobalAlphaFactordSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactordSUN)
        kpanic("ext_glGlobalAlphaFactordSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactordSUN)(dARG1);
    GL_LOG ("glGlobalAlphaFactordSUN GLdouble factor=%f",dARG1);
    }
}
void glcommon_glGlobalAlphaFactorfSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactorfSUN)
        kpanic("ext_glGlobalAlphaFactorfSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactorfSUN)(fARG1);
    GL_LOG ("glGlobalAlphaFactorfSUN GLfloat factor=%f",fARG1);
    }
}
void glcommon_glGlobalAlphaFactoriSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactoriSUN)
        kpanic("ext_glGlobalAlphaFactoriSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactoriSUN)(ARG1);
    GL_LOG ("glGlobalAlphaFactoriSUN GLint factor=%d",ARG1);
    }
}
void glcommon_glGlobalAlphaFactorsSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactorsSUN)
        kpanic("ext_glGlobalAlphaFactorsSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactorsSUN)(sARG1);
    GL_LOG ("glGlobalAlphaFactorsSUN GLshort factor=%d",sARG1);
    }
}
void glcommon_glGlobalAlphaFactorubSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactorubSUN)
        kpanic("ext_glGlobalAlphaFactorubSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactorubSUN)(bARG1);
    GL_LOG ("glGlobalAlphaFactorubSUN GLubyte factor=%d",bARG1);
    }
}
void glcommon_glGlobalAlphaFactoruiSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactoruiSUN)
        kpanic("ext_glGlobalAlphaFactoruiSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactoruiSUN)(ARG1);
    GL_LOG ("glGlobalAlphaFactoruiSUN GLuint factor=%d",ARG1);
    }
}
void glcommon_glGlobalAlphaFactorusSUN(CPU* cpu) {
    if (!ext_glGlobalAlphaFactorusSUN)
        kpanic("ext_glGlobalAlphaFactorusSUN is NULL");
    {
    GL_FUNC(ext_glGlobalAlphaFactorusSUN)(sARG1);
    GL_LOG ("glGlobalAlphaFactorusSUN GLushort factor=%d",sARG1);
    }
}
void glcommon_glHintPGI(CPU* cpu) {
    if (!ext_glHintPGI)
        kpanic("ext_glHintPGI is NULL");
    {
    GL_FUNC(ext_glHintPGI)(ARG1, ARG2);
    GL_LOG ("glHintPGI GLenum target=%d, GLint mode=%d",ARG1,ARG2);
    }
}
void glcommon_glHistogram(CPU* cpu) {
    if (!ext_glHistogram)
        kpanic("ext_glHistogram is NULL");
    {
    GL_FUNC(ext_glHistogram)(ARG1, ARG2, ARG3, bARG4);
    GL_LOG ("glHistogram GLenum target=%d, GLsizei width=%d, GLenum internalformat=%d, GLboolean sink=%d",ARG1,ARG2,ARG3,bARG4);
    }
}
void glcommon_glHistogramEXT(CPU* cpu) {
    if (!ext_glHistogramEXT)
        kpanic("ext_glHistogramEXT is NULL");
    {
    GL_FUNC(ext_glHistogramEXT)(ARG1, ARG2, ARG3, bARG4);
    GL_LOG ("glHistogramEXT GLenum target=%d, GLsizei width=%d, GLenum internalformat=%d, GLboolean sink=%d",ARG1,ARG2,ARG3,bARG4);
    }
}
void glcommon_glIglooInterfaceSGIX(CPU* cpu) {
    if (!ext_glIglooInterfaceSGIX)
        kpanic("ext_glIglooInterfaceSGIX is NULL");
    {
    GL_FUNC(ext_glIglooInterfaceSGIX)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glIglooInterfaceSGIX GLenum pname=%d, const void* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glImageTransformParameterfHP(CPU* cpu) {
    if (!ext_glImageTransformParameterfHP)
        kpanic("ext_glImageTransformParameterfHP is NULL");
    {
    GL_FUNC(ext_glImageTransformParameterfHP)(ARG1, ARG2, fARG3);
    GL_LOG ("glImageTransformParameterfHP GLenum target=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glImageTransformParameterfvHP(CPU* cpu) {
    if (!ext_glImageTransformParameterfvHP)
        kpanic("ext_glImageTransformParameterfvHP is NULL");
    {
    GL_FUNC(ext_glImageTransformParameterfvHP)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glImageTransformParameterfvHP GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glImageTransformParameteriHP(CPU* cpu) {
    if (!ext_glImageTransformParameteriHP)
        kpanic("ext_glImageTransformParameteriHP is NULL");
    {
    GL_FUNC(ext_glImageTransformParameteriHP)(ARG1, ARG2, ARG3);
    GL_LOG ("glImageTransformParameteriHP GLenum target=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glImageTransformParameterivHP(CPU* cpu) {
    if (!ext_glImageTransformParameterivHP)
        kpanic("ext_glImageTransformParameterivHP is NULL");
    {
    GL_FUNC(ext_glImageTransformParameterivHP)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glImageTransformParameterivHP GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glImportSyncEXT(CPU* cpu) {
    if (!ext_glImportSyncEXT)
        kpanic("ext_glImportSyncEXT is NULL");
    {
    GLsync ret=GL_FUNC(ext_glImportSyncEXT)(ARG1, ARG2, ARG3);
    EAX=marshalBackSync(cpu, ret);
    GL_LOG ("glImportSyncEXT GLenum external_sync_type=%d, GLintptr external_sync=%d, GLbitfield flags=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glIndexFormatNV(CPU* cpu) {
    if (!ext_glIndexFormatNV)
        kpanic("ext_glIndexFormatNV is NULL");
    {
    GL_FUNC(ext_glIndexFormatNV)(ARG1, ARG2);
    GL_LOG ("glIndexFormatNV GLenum type=%d, GLsizei stride=%d",ARG1,ARG2);
    }
}
void glcommon_glIndexFuncEXT(CPU* cpu) {
    if (!ext_glIndexFuncEXT)
        kpanic("ext_glIndexFuncEXT is NULL");
    {
    GL_FUNC(ext_glIndexFuncEXT)(ARG1, fARG2);
    GL_LOG ("glIndexFuncEXT GLenum func=%d, GLclampf ref=%f",ARG1,fARG2);
    }
}
void glcommon_glIndexMaterialEXT(CPU* cpu) {
    if (!ext_glIndexMaterialEXT)
        kpanic("ext_glIndexMaterialEXT is NULL");
    {
    GL_FUNC(ext_glIndexMaterialEXT)(ARG1, ARG2);
    GL_LOG ("glIndexMaterialEXT GLenum face=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glIndexPointerEXT(CPU* cpu) {
    if (!ext_glIndexPointerEXT)
        kpanic("ext_glIndexPointerEXT is NULL");
    {
    GL_FUNC(ext_glIndexPointerEXT)(ARG1, ARG2, ARG3, (const void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glIndexPointerEXT GLenum type=%d, GLsizei stride=%d, GLsizei count=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glIndexPointerListIBM(CPU* cpu) {
    if (!ext_glIndexPointerListIBM)
        kpanic("ext_glIndexPointerListIBM is NULL");
    {
    GL_FUNC(ext_glIndexPointerListIBM)(ARG1, ARG2, (const void**)marshalunhandled("glIndexPointerListIBM", "pointer", cpu, ARG3), ARG4);
    GL_LOG ("glIndexPointerListIBM GLenum type=%d, GLint stride=%d, const void** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glIndexxOES(CPU* cpu) {
    if (!ext_glIndexxOES)
        kpanic("ext_glIndexxOES is NULL");
    {
    GL_FUNC(ext_glIndexxOES)(ARG1);
    GL_LOG ("glIndexxOES GLfixed component=%d",ARG1);
    }
}
void glcommon_glIndexxvOES(CPU* cpu) {
    if (!ext_glIndexxvOES)
        kpanic("ext_glIndexxvOES is NULL");
    {
    GL_FUNC(ext_glIndexxvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glIndexxvOES const GLfixed* component=%.08x",ARG1);
    }
}
void glcommon_glInsertComponentEXT(CPU* cpu) {
    if (!ext_glInsertComponentEXT)
        kpanic("ext_glInsertComponentEXT is NULL");
    {
    GL_FUNC(ext_glInsertComponentEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glInsertComponentEXT GLuint res=%d, GLuint src=%d, GLuint num=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glInsertEventMarkerEXT(CPU* cpu) {
    if (!ext_glInsertEventMarkerEXT)
        kpanic("ext_glInsertEventMarkerEXT is NULL");
    {
    GL_FUNC(ext_glInsertEventMarkerEXT)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glInsertEventMarkerEXT GLsizei length=%d, const GLchar* marker=%.08x",ARG1,ARG2);
    }
}
void glcommon_glInstrumentsBufferSGIX(CPU* cpu) {
    if (!ext_glInstrumentsBufferSGIX)
        kpanic("ext_glInstrumentsBufferSGIX is NULL");
    {
    GL_FUNC(ext_glInstrumentsBufferSGIX)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glInstrumentsBufferSGIX GLsizei size=%d, GLint* buffer=%.08x",ARG1,ARG2);
    }
}
void glcommon_glInterpolatePathsNV(CPU* cpu) {
    if (!ext_glInterpolatePathsNV)
        kpanic("ext_glInterpolatePathsNV is NULL");
    {
    GL_FUNC(ext_glInterpolatePathsNV)(ARG1, ARG2, ARG3, fARG4);
    GL_LOG ("glInterpolatePathsNV GLuint resultPath=%d, GLuint pathA=%d, GLuint pathB=%d, GLfloat weight=%f",ARG1,ARG2,ARG3,fARG4);
    }
}
void glcommon_glInvalidateBufferData(CPU* cpu) {
    if (!ext_glInvalidateBufferData)
        kpanic("ext_glInvalidateBufferData is NULL");
    {
    GL_FUNC(ext_glInvalidateBufferData)(ARG1);
    GL_LOG ("glInvalidateBufferData GLuint buffer=%d",ARG1);
    }
}
void glcommon_glInvalidateBufferSubData(CPU* cpu) {
    if (!ext_glInvalidateBufferSubData)
        kpanic("ext_glInvalidateBufferSubData is NULL");
    {
    GL_FUNC(ext_glInvalidateBufferSubData)(ARG1, ARG2, ARG3);
    GL_LOG ("glInvalidateBufferSubData GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr length=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glInvalidateFramebuffer(CPU* cpu) {
    if (!ext_glInvalidateFramebuffer)
        kpanic("ext_glInvalidateFramebuffer is NULL");
    {
    GL_FUNC(ext_glInvalidateFramebuffer)(ARG1, ARG2, (GLenum*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glInvalidateFramebuffer GLenum target=%d, GLsizei numAttachments=%d, const GLenum* attachments=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glInvalidateNamedFramebufferData(CPU* cpu) {
    if (!ext_glInvalidateNamedFramebufferData)
        kpanic("ext_glInvalidateNamedFramebufferData is NULL");
    {
    GL_FUNC(ext_glInvalidateNamedFramebufferData)(ARG1, ARG2, (GLenum*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glInvalidateNamedFramebufferData GLuint framebuffer=%d, GLsizei numAttachments=%d, const GLenum* attachments=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glInvalidateNamedFramebufferSubData(CPU* cpu) {
    if (!ext_glInvalidateNamedFramebufferSubData)
        kpanic("ext_glInvalidateNamedFramebufferSubData is NULL");
    {
    GL_FUNC(ext_glInvalidateNamedFramebufferSubData)(ARG1, ARG2, (GLenum*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glInvalidateNamedFramebufferSubData GLuint framebuffer=%d, GLsizei numAttachments=%d, const GLenum* attachments=%.08x, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glInvalidateSubFramebuffer(CPU* cpu) {
    if (!ext_glInvalidateSubFramebuffer)
        kpanic("ext_glInvalidateSubFramebuffer is NULL");
    {
    GL_FUNC(ext_glInvalidateSubFramebuffer)(ARG1, ARG2, (GLenum*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glInvalidateSubFramebuffer GLenum target=%d, GLsizei numAttachments=%d, const GLenum* attachments=%.08x, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glInvalidateTexImage(CPU* cpu) {
    if (!ext_glInvalidateTexImage)
        kpanic("ext_glInvalidateTexImage is NULL");
    {
    GL_FUNC(ext_glInvalidateTexImage)(ARG1, ARG2);
    GL_LOG ("glInvalidateTexImage GLuint texture=%d, GLint level=%d",ARG1,ARG2);
    }
}
void glcommon_glInvalidateTexSubImage(CPU* cpu) {
    if (!ext_glInvalidateTexSubImage)
        kpanic("ext_glInvalidateTexSubImage is NULL");
    {
    GL_FUNC(ext_glInvalidateTexSubImage)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glInvalidateTexSubImage GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glIsAsyncMarkerSGIX(CPU* cpu) {
    if (!ext_glIsAsyncMarkerSGIX)
        kpanic("ext_glIsAsyncMarkerSGIX is NULL");
    {
    EAX=GL_FUNC(ext_glIsAsyncMarkerSGIX)(ARG1);
    GL_LOG ("glIsAsyncMarkerSGIX GLuint marker=%d",ARG1);
    }
}
void glcommon_glIsBuffer(CPU* cpu) {
    if (!ext_glIsBuffer)
        kpanic("ext_glIsBuffer is NULL");
    {
    EAX=GL_FUNC(ext_glIsBuffer)(ARG1);
    GL_LOG ("glIsBuffer GLuint buffer=%d",ARG1);
    }
}
void glcommon_glIsBufferARB(CPU* cpu) {
    if (!ext_glIsBufferARB)
        kpanic("ext_glIsBufferARB is NULL");
    {
    EAX=GL_FUNC(ext_glIsBufferARB)(ARG1);
    GL_LOG ("glIsBufferARB GLuint buffer=%d",ARG1);
    }
}
void glcommon_glIsBufferResidentNV(CPU* cpu) {
    if (!ext_glIsBufferResidentNV)
        kpanic("ext_glIsBufferResidentNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsBufferResidentNV)(ARG1);
    GL_LOG ("glIsBufferResidentNV GLenum target=%d",ARG1);
    }
}
void glcommon_glIsCommandListNV(CPU* cpu) {
    if (!ext_glIsCommandListNV)
        kpanic("ext_glIsCommandListNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsCommandListNV)(ARG1);
    GL_LOG ("glIsCommandListNV GLuint list=%d",ARG1);
    }
}
void glcommon_glIsEnabledIndexedEXT(CPU* cpu) {
    if (!ext_glIsEnabledIndexedEXT)
        kpanic("ext_glIsEnabledIndexedEXT is NULL");
    {
    EAX=GL_FUNC(ext_glIsEnabledIndexedEXT)(ARG1, ARG2);
    GL_LOG ("glIsEnabledIndexedEXT GLenum target=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glIsEnabledi(CPU* cpu) {
    if (!ext_glIsEnabledi)
        kpanic("ext_glIsEnabledi is NULL");
    {
    EAX=GL_FUNC(ext_glIsEnabledi)(ARG1, ARG2);
    GL_LOG ("glIsEnabledi GLenum target=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glIsFenceAPPLE(CPU* cpu) {
    if (!ext_glIsFenceAPPLE)
        kpanic("ext_glIsFenceAPPLE is NULL");
    {
    EAX=GL_FUNC(ext_glIsFenceAPPLE)(ARG1);
    GL_LOG ("glIsFenceAPPLE GLuint fence=%d",ARG1);
    }
}
void glcommon_glIsFenceNV(CPU* cpu) {
    if (!ext_glIsFenceNV)
        kpanic("ext_glIsFenceNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsFenceNV)(ARG1);
    GL_LOG ("glIsFenceNV GLuint fence=%d",ARG1);
    }
}
void glcommon_glIsFramebuffer(CPU* cpu) {
    if (!ext_glIsFramebuffer)
        kpanic("ext_glIsFramebuffer is NULL");
    {
    EAX=GL_FUNC(ext_glIsFramebuffer)(ARG1);
    GL_LOG ("glIsFramebuffer GLuint framebuffer=%d",ARG1);
    }
}
void glcommon_glIsFramebufferEXT(CPU* cpu) {
    if (!ext_glIsFramebufferEXT)
        kpanic("ext_glIsFramebufferEXT is NULL");
    {
    EAX=GL_FUNC(ext_glIsFramebufferEXT)(ARG1);
    GL_LOG ("glIsFramebufferEXT GLuint framebuffer=%d",ARG1);
    }
}
void glcommon_glIsImageHandleResidentARB(CPU* cpu) {
    if (!ext_glIsImageHandleResidentARB)
        kpanic("ext_glIsImageHandleResidentARB is NULL");
    {
    EAX=GL_FUNC(ext_glIsImageHandleResidentARB)(llARG1);
    GL_LOG ("glIsImageHandleResidentARB GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glIsImageHandleResidentNV(CPU* cpu) {
    if (!ext_glIsImageHandleResidentNV)
        kpanic("ext_glIsImageHandleResidentNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsImageHandleResidentNV)(llARG1);
    GL_LOG ("glIsImageHandleResidentNV GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glIsNameAMD(CPU* cpu) {
    if (!ext_glIsNameAMD)
        kpanic("ext_glIsNameAMD is NULL");
    {
    EAX=GL_FUNC(ext_glIsNameAMD)(ARG1, ARG2);
    GL_LOG ("glIsNameAMD GLenum identifier=%d, GLuint name=%d",ARG1,ARG2);
    }
}
void glcommon_glIsNamedBufferResidentNV(CPU* cpu) {
    if (!ext_glIsNamedBufferResidentNV)
        kpanic("ext_glIsNamedBufferResidentNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsNamedBufferResidentNV)(ARG1);
    GL_LOG ("glIsNamedBufferResidentNV GLuint buffer=%d",ARG1);
    }
}
void glcommon_glIsNamedStringARB(CPU* cpu) {
    if (!ext_glIsNamedStringARB)
        kpanic("ext_glIsNamedStringARB is NULL");
    {
    EAX=GL_FUNC(ext_glIsNamedStringARB)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glIsNamedStringARB GLint namelen=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glIsObjectBufferATI(CPU* cpu) {
    if (!ext_glIsObjectBufferATI)
        kpanic("ext_glIsObjectBufferATI is NULL");
    {
    EAX=GL_FUNC(ext_glIsObjectBufferATI)(ARG1);
    GL_LOG ("glIsObjectBufferATI GLuint buffer=%d",ARG1);
    }
}
void glcommon_glIsOcclusionQueryNV(CPU* cpu) {
    if (!ext_glIsOcclusionQueryNV)
        kpanic("ext_glIsOcclusionQueryNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsOcclusionQueryNV)(ARG1);
    GL_LOG ("glIsOcclusionQueryNV GLuint id=%d",ARG1);
    }
}
void glcommon_glIsPathNV(CPU* cpu) {
    if (!ext_glIsPathNV)
        kpanic("ext_glIsPathNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsPathNV)(ARG1);
    GL_LOG ("glIsPathNV GLuint path=%d",ARG1);
    }
}
void glcommon_glIsPointInFillPathNV(CPU* cpu) {
    if (!ext_glIsPointInFillPathNV)
        kpanic("ext_glIsPointInFillPathNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsPointInFillPathNV)(ARG1, ARG2, fARG3, fARG4);
    GL_LOG ("glIsPointInFillPathNV GLuint path=%d, GLuint mask=%d, GLfloat x=%f, GLfloat y=%f",ARG1,ARG2,fARG3,fARG4);
    }
}
void glcommon_glIsPointInStrokePathNV(CPU* cpu) {
    if (!ext_glIsPointInStrokePathNV)
        kpanic("ext_glIsPointInStrokePathNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsPointInStrokePathNV)(ARG1, fARG2, fARG3);
    GL_LOG ("glIsPointInStrokePathNV GLuint path=%d, GLfloat x=%f, GLfloat y=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glIsProgram(CPU* cpu) {
    if (!ext_glIsProgram)
        kpanic("ext_glIsProgram is NULL");
    {
    EAX=GL_FUNC(ext_glIsProgram)(ARG1);
    GL_LOG ("glIsProgram GLuint program=%d",ARG1);
    }
}
void glcommon_glIsProgramARB(CPU* cpu) {
    if (!ext_glIsProgramARB)
        kpanic("ext_glIsProgramARB is NULL");
    {
    EAX=GL_FUNC(ext_glIsProgramARB)(ARG1);
    GL_LOG ("glIsProgramARB GLuint program=%d",ARG1);
    }
}
void glcommon_glIsProgramNV(CPU* cpu) {
    if (!ext_glIsProgramNV)
        kpanic("ext_glIsProgramNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsProgramNV)(ARG1);
    GL_LOG ("glIsProgramNV GLuint id=%d",ARG1);
    }
}
void glcommon_glIsProgramPipeline(CPU* cpu) {
    if (!ext_glIsProgramPipeline)
        kpanic("ext_glIsProgramPipeline is NULL");
    {
    EAX=GL_FUNC(ext_glIsProgramPipeline)(ARG1);
    GL_LOG ("glIsProgramPipeline GLuint pipeline=%d",ARG1);
    }
}
void glcommon_glIsQuery(CPU* cpu) {
    if (!ext_glIsQuery)
        kpanic("ext_glIsQuery is NULL");
    {
    EAX=GL_FUNC(ext_glIsQuery)(ARG1);
    GL_LOG ("glIsQuery GLuint id=%d",ARG1);
    }
}
void glcommon_glIsQueryARB(CPU* cpu) {
    if (!ext_glIsQueryARB)
        kpanic("ext_glIsQueryARB is NULL");
    {
    EAX=GL_FUNC(ext_glIsQueryARB)(ARG1);
    GL_LOG ("glIsQueryARB GLuint id=%d",ARG1);
    }
}
void glcommon_glIsRenderbuffer(CPU* cpu) {
    if (!ext_glIsRenderbuffer)
        kpanic("ext_glIsRenderbuffer is NULL");
    {
    EAX=GL_FUNC(ext_glIsRenderbuffer)(ARG1);
    GL_LOG ("glIsRenderbuffer GLuint renderbuffer=%d",ARG1);
    }
}
void glcommon_glIsRenderbufferEXT(CPU* cpu) {
    if (!ext_glIsRenderbufferEXT)
        kpanic("ext_glIsRenderbufferEXT is NULL");
    {
    EAX=GL_FUNC(ext_glIsRenderbufferEXT)(ARG1);
    GL_LOG ("glIsRenderbufferEXT GLuint renderbuffer=%d",ARG1);
    }
}
void glcommon_glIsSampler(CPU* cpu) {
    if (!ext_glIsSampler)
        kpanic("ext_glIsSampler is NULL");
    {
    EAX=GL_FUNC(ext_glIsSampler)(ARG1);
    GL_LOG ("glIsSampler GLuint sampler=%d",ARG1);
    }
}
void glcommon_glIsShader(CPU* cpu) {
    if (!ext_glIsShader)
        kpanic("ext_glIsShader is NULL");
    {
    EAX=GL_FUNC(ext_glIsShader)(ARG1);
    GL_LOG ("glIsShader GLuint shader=%d",ARG1);
    }
}
void glcommon_glIsStateNV(CPU* cpu) {
    if (!ext_glIsStateNV)
        kpanic("ext_glIsStateNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsStateNV)(ARG1);
    GL_LOG ("glIsStateNV GLuint state=%d",ARG1);
    }
}
void glcommon_glIsSync(CPU* cpu) {
    if (!ext_glIsSync)
        kpanic("ext_glIsSync is NULL");
    {
    EAX=GL_FUNC(ext_glIsSync)(marshalSync(cpu, ARG1));
    GL_LOG ("glIsSync GLsync sync=%d",ARG1);
    }
}
void glcommon_glIsTextureEXT(CPU* cpu) {
    if (!ext_glIsTextureEXT)
        kpanic("ext_glIsTextureEXT is NULL");
    {
    EAX=GL_FUNC(ext_glIsTextureEXT)(ARG1);
    GL_LOG ("glIsTextureEXT GLuint texture=%d",ARG1);
    }
}
void glcommon_glIsTextureHandleResidentARB(CPU* cpu) {
    if (!ext_glIsTextureHandleResidentARB)
        kpanic("ext_glIsTextureHandleResidentARB is NULL");
    {
    EAX=GL_FUNC(ext_glIsTextureHandleResidentARB)(llARG1);
    GL_LOG ("glIsTextureHandleResidentARB GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glIsTextureHandleResidentNV(CPU* cpu) {
    if (!ext_glIsTextureHandleResidentNV)
        kpanic("ext_glIsTextureHandleResidentNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsTextureHandleResidentNV)(llARG1);
    GL_LOG ("glIsTextureHandleResidentNV GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glIsTransformFeedback(CPU* cpu) {
    if (!ext_glIsTransformFeedback)
        kpanic("ext_glIsTransformFeedback is NULL");
    {
    EAX=GL_FUNC(ext_glIsTransformFeedback)(ARG1);
    GL_LOG ("glIsTransformFeedback GLuint id=%d",ARG1);
    }
}
void glcommon_glIsTransformFeedbackNV(CPU* cpu) {
    if (!ext_glIsTransformFeedbackNV)
        kpanic("ext_glIsTransformFeedbackNV is NULL");
    {
    EAX=GL_FUNC(ext_glIsTransformFeedbackNV)(ARG1);
    GL_LOG ("glIsTransformFeedbackNV GLuint id=%d",ARG1);
    }
}
void glcommon_glIsVariantEnabledEXT(CPU* cpu) {
    if (!ext_glIsVariantEnabledEXT)
        kpanic("ext_glIsVariantEnabledEXT is NULL");
    {
    EAX=GL_FUNC(ext_glIsVariantEnabledEXT)(ARG1, ARG2);
    GL_LOG ("glIsVariantEnabledEXT GLuint id=%d, GLenum cap=%d",ARG1,ARG2);
    }
}
void glcommon_glIsVertexArray(CPU* cpu) {
    if (!ext_glIsVertexArray)
        kpanic("ext_glIsVertexArray is NULL");
    {
    EAX=GL_FUNC(ext_glIsVertexArray)(ARG1);
    GL_LOG ("glIsVertexArray GLuint array=%d",ARG1);
    }
}
void glcommon_glIsVertexArrayAPPLE(CPU* cpu) {
    if (!ext_glIsVertexArrayAPPLE)
        kpanic("ext_glIsVertexArrayAPPLE is NULL");
    {
    EAX=GL_FUNC(ext_glIsVertexArrayAPPLE)(ARG1);
    GL_LOG ("glIsVertexArrayAPPLE GLuint array=%d",ARG1);
    }
}
void glcommon_glIsVertexAttribEnabledAPPLE(CPU* cpu) {
    if (!ext_glIsVertexAttribEnabledAPPLE)
        kpanic("ext_glIsVertexAttribEnabledAPPLE is NULL");
    {
    EAX=GL_FUNC(ext_glIsVertexAttribEnabledAPPLE)(ARG1, ARG2);
    GL_LOG ("glIsVertexAttribEnabledAPPLE GLuint index=%d, GLenum pname=%d",ARG1,ARG2);
    }
}
void glcommon_glLabelObjectEXT(CPU* cpu) {
    if (!ext_glLabelObjectEXT)
        kpanic("ext_glLabelObjectEXT is NULL");
    {
    GL_FUNC(ext_glLabelObjectEXT)(ARG1, ARG2, ARG3, marshalsz(cpu, ARG4));
    GL_LOG ("glLabelObjectEXT GLenum type=%d, GLuint object=%d, GLsizei length=%d, const GLchar* label=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glLightEnviSGIX(CPU* cpu) {
    if (!ext_glLightEnviSGIX)
        kpanic("ext_glLightEnviSGIX is NULL");
    {
    GL_FUNC(ext_glLightEnviSGIX)(ARG1, ARG2);
    GL_LOG ("glLightEnviSGIX GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glLightModelxOES(CPU* cpu) {
    if (!ext_glLightModelxOES)
        kpanic("ext_glLightModelxOES is NULL");
    {
    GL_FUNC(ext_glLightModelxOES)(ARG1, ARG2);
    GL_LOG ("glLightModelxOES GLenum pname=%d, GLfixed param=%d",ARG1,ARG2);
    }
}
void glcommon_glLightModelxvOES(CPU* cpu) {
    if (!ext_glLightModelxvOES)
        kpanic("ext_glLightModelxvOES is NULL");
    {
    GL_FUNC(ext_glLightModelxvOES)(ARG1, (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glLightModelxvOES GLenum pname=%d, const GLfixed* param=%.08x",ARG1,ARG2);
    }
}
void glcommon_glLightxOES(CPU* cpu) {
    if (!ext_glLightxOES)
        kpanic("ext_glLightxOES is NULL");
    {
    GL_FUNC(ext_glLightxOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glLightxOES GLenum light=%d, GLenum pname=%d, GLfixed param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glLightxvOES(CPU* cpu) {
    if (!ext_glLightxvOES)
        kpanic("ext_glLightxvOES is NULL");
    {
    GL_FUNC(ext_glLightxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glLightxvOES GLenum light=%d, GLenum pname=%d, const GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glLineWidthxOES(CPU* cpu) {
    if (!ext_glLineWidthxOES)
        kpanic("ext_glLineWidthxOES is NULL");
    {
    GL_FUNC(ext_glLineWidthxOES)(ARG1);
    GL_LOG ("glLineWidthxOES GLfixed width=%d",ARG1);
    }
}
void glcommon_glLinkProgram(CPU* cpu) {
    if (!ext_glLinkProgram)
        kpanic("ext_glLinkProgram is NULL");
    {
    GL_FUNC(ext_glLinkProgram)(ARG1);
    GL_LOG ("glLinkProgram GLuint program=%d",ARG1);
    }
}
void glcommon_glLinkProgramARB(CPU* cpu) {
    if (!ext_glLinkProgramARB)
        kpanic("ext_glLinkProgramARB is NULL");
    {
    GL_FUNC(ext_glLinkProgramARB)(INDEX_TO_HANDLE(hARG1));
    GL_LOG ("glLinkProgramARB GLhandleARB programObj=%d",ARG1);
    }
}
void glcommon_glListDrawCommandsStatesClientNV(CPU* cpu) {
    if (!ext_glListDrawCommandsStatesClientNV)
        kpanic("ext_glListDrawCommandsStatesClientNV is NULL");
    {
    GL_FUNC(ext_glListDrawCommandsStatesClientNV)(ARG1, ARG2, (const void**)marshalpp(cpu, ARG3, ARG7, ARG4, 1, 0), (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLuint*)marshalp(cpu, 0, ARG5, 0), (GLuint*)marshalp(cpu, 0, ARG6, 0), ARG7);
    GL_LOG ("glListDrawCommandsStatesClientNV GLuint list=%d, GLuint segment=%d, const void** indirects=%.08x, const GLsizei* sizes=%.08x, const GLuint* states=%.08x, const GLuint* fbos=%.08x, GLuint count=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glListParameterfSGIX(CPU* cpu) {
    if (!ext_glListParameterfSGIX)
        kpanic("ext_glListParameterfSGIX is NULL");
    {
    GL_FUNC(ext_glListParameterfSGIX)(ARG1, ARG2, fARG3);
    GL_LOG ("glListParameterfSGIX GLuint list=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glListParameterfvSGIX(CPU* cpu) {
    if (!ext_glListParameterfvSGIX)
        kpanic("ext_glListParameterfvSGIX is NULL");
    {
    GL_FUNC(ext_glListParameterfvSGIX)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glListParameterfvSGIX GLuint list=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glListParameteriSGIX(CPU* cpu) {
    if (!ext_glListParameteriSGIX)
        kpanic("ext_glListParameteriSGIX is NULL");
    {
    GL_FUNC(ext_glListParameteriSGIX)(ARG1, ARG2, ARG3);
    GL_LOG ("glListParameteriSGIX GLuint list=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glListParameterivSGIX(CPU* cpu) {
    if (!ext_glListParameterivSGIX)
        kpanic("ext_glListParameterivSGIX is NULL");
    {
    GL_FUNC(ext_glListParameterivSGIX)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glListParameterivSGIX GLuint list=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glLoadIdentityDeformationMapSGIX(CPU* cpu) {
    if (!ext_glLoadIdentityDeformationMapSGIX)
        kpanic("ext_glLoadIdentityDeformationMapSGIX is NULL");
    {
    GL_FUNC(ext_glLoadIdentityDeformationMapSGIX)(ARG1);
    GL_LOG ("glLoadIdentityDeformationMapSGIX GLbitfield mask=%d",ARG1);
    }
}
void glcommon_glLoadMatrixxOES(CPU* cpu) {
    if (!ext_glLoadMatrixxOES)
        kpanic("ext_glLoadMatrixxOES is NULL");
    {
    GL_FUNC(ext_glLoadMatrixxOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glLoadMatrixxOES const GLfixed* m=%.08x",ARG1);
    }
}
void glcommon_glLoadProgramNV(CPU* cpu) {
    if (!ext_glLoadProgramNV)
        kpanic("ext_glLoadProgramNV is NULL");
    {
    GL_FUNC(ext_glLoadProgramNV)(ARG1, ARG2, ARG3, (GLubyte*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glLoadProgramNV GLenum target=%d, GLuint id=%d, GLsizei len=%d, const GLubyte* program=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glLoadTransposeMatrixd(CPU* cpu) {
    if (!ext_glLoadTransposeMatrixd)
        kpanic("ext_glLoadTransposeMatrixd is NULL");
    {
    GL_FUNC(ext_glLoadTransposeMatrixd)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glLoadTransposeMatrixd const GLdouble* m=%.08x",ARG1);
    }
}
void glcommon_glLoadTransposeMatrixdARB(CPU* cpu) {
    if (!ext_glLoadTransposeMatrixdARB)
        kpanic("ext_glLoadTransposeMatrixdARB is NULL");
    {
    GL_FUNC(ext_glLoadTransposeMatrixdARB)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glLoadTransposeMatrixdARB const GLdouble* m=%.08x",ARG1);
    }
}
void glcommon_glLoadTransposeMatrixf(CPU* cpu) {
    if (!ext_glLoadTransposeMatrixf)
        kpanic("ext_glLoadTransposeMatrixf is NULL");
    {
    GL_FUNC(ext_glLoadTransposeMatrixf)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glLoadTransposeMatrixf const GLfloat* m=%.08x",ARG1);
    }
}
void glcommon_glLoadTransposeMatrixfARB(CPU* cpu) {
    if (!ext_glLoadTransposeMatrixfARB)
        kpanic("ext_glLoadTransposeMatrixfARB is NULL");
    {
    GL_FUNC(ext_glLoadTransposeMatrixfARB)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glLoadTransposeMatrixfARB const GLfloat* m=%.08x",ARG1);
    }
}
void glcommon_glLoadTransposeMatrixxOES(CPU* cpu) {
    if (!ext_glLoadTransposeMatrixxOES)
        kpanic("ext_glLoadTransposeMatrixxOES is NULL");
    {
    GL_FUNC(ext_glLoadTransposeMatrixxOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glLoadTransposeMatrixxOES const GLfixed* m=%.08x",ARG1);
    }
}
void glcommon_glLockArraysEXT(CPU* cpu) {
    if (!ext_glLockArraysEXT)
        kpanic("ext_glLockArraysEXT is NULL");
    {
    GL_FUNC(ext_glLockArraysEXT)(ARG1, ARG2);
    GL_LOG ("glLockArraysEXT GLint first=%d, GLsizei count=%d",ARG1,ARG2);
    }
}
void glcommon_glMTexCoord2fSGIS(CPU* cpu) {
    if (!ext_glMTexCoord2fSGIS)
        kpanic("ext_glMTexCoord2fSGIS is NULL");
    {
    GL_FUNC(ext_glMTexCoord2fSGIS)(ARG1, fARG2, fARG3);
    GL_LOG ("glMTexCoord2fSGIS GLenum target=%d, GLfloat s=%f, GLfloat t=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glMTexCoord2fvSGIS(CPU* cpu) {
    if (!ext_glMTexCoord2fvSGIS)
        kpanic("ext_glMTexCoord2fvSGIS is NULL");
    {
    GL_FUNC(ext_glMTexCoord2fvSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMTexCoord2fvSGIS GLenum target=%d, GLfloat * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMakeBufferNonResidentNV(CPU* cpu) {
    if (!ext_glMakeBufferNonResidentNV)
        kpanic("ext_glMakeBufferNonResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeBufferNonResidentNV)(ARG1);
    GL_LOG ("glMakeBufferNonResidentNV GLenum target=%d",ARG1);
    }
}
void glcommon_glMakeBufferResidentNV(CPU* cpu) {
    if (!ext_glMakeBufferResidentNV)
        kpanic("ext_glMakeBufferResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeBufferResidentNV)(ARG1, ARG2);
    GL_LOG ("glMakeBufferResidentNV GLenum target=%d, GLenum access=%d",ARG1,ARG2);
    }
}
void glcommon_glMakeImageHandleNonResidentARB(CPU* cpu) {
    if (!ext_glMakeImageHandleNonResidentARB)
        kpanic("ext_glMakeImageHandleNonResidentARB is NULL");
    {
    GL_FUNC(ext_glMakeImageHandleNonResidentARB)(llARG1);
    GL_LOG ("glMakeImageHandleNonResidentARB GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glMakeImageHandleNonResidentNV(CPU* cpu) {
    if (!ext_glMakeImageHandleNonResidentNV)
        kpanic("ext_glMakeImageHandleNonResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeImageHandleNonResidentNV)(llARG1);
    GL_LOG ("glMakeImageHandleNonResidentNV GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glMakeImageHandleResidentARB(CPU* cpu) {
    if (!ext_glMakeImageHandleResidentARB)
        kpanic("ext_glMakeImageHandleResidentARB is NULL");
    {
    GL_FUNC(ext_glMakeImageHandleResidentARB)(llARG1, ARG2);
    GL_LOG ("glMakeImageHandleResidentARB GLuint64 handle=" PRIu64 ", GLenum access=%d",llARG1,ARG2);
    }
}
void glcommon_glMakeImageHandleResidentNV(CPU* cpu) {
    if (!ext_glMakeImageHandleResidentNV)
        kpanic("ext_glMakeImageHandleResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeImageHandleResidentNV)(llARG1, ARG2);
    GL_LOG ("glMakeImageHandleResidentNV GLuint64 handle=" PRIu64 ", GLenum access=%d",llARG1,ARG2);
    }
}
void glcommon_glMakeNamedBufferNonResidentNV(CPU* cpu) {
    if (!ext_glMakeNamedBufferNonResidentNV)
        kpanic("ext_glMakeNamedBufferNonResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeNamedBufferNonResidentNV)(ARG1);
    GL_LOG ("glMakeNamedBufferNonResidentNV GLuint buffer=%d",ARG1);
    }
}
void glcommon_glMakeNamedBufferResidentNV(CPU* cpu) {
    if (!ext_glMakeNamedBufferResidentNV)
        kpanic("ext_glMakeNamedBufferResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeNamedBufferResidentNV)(ARG1, ARG2);
    GL_LOG ("glMakeNamedBufferResidentNV GLuint buffer=%d, GLenum access=%d",ARG1,ARG2);
    }
}
void glcommon_glMakeTextureHandleNonResidentARB(CPU* cpu) {
    if (!ext_glMakeTextureHandleNonResidentARB)
        kpanic("ext_glMakeTextureHandleNonResidentARB is NULL");
    {
    GL_FUNC(ext_glMakeTextureHandleNonResidentARB)(llARG1);
    GL_LOG ("glMakeTextureHandleNonResidentARB GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glMakeTextureHandleNonResidentNV(CPU* cpu) {
    if (!ext_glMakeTextureHandleNonResidentNV)
        kpanic("ext_glMakeTextureHandleNonResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeTextureHandleNonResidentNV)(llARG1);
    GL_LOG ("glMakeTextureHandleNonResidentNV GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glMakeTextureHandleResidentARB(CPU* cpu) {
    if (!ext_glMakeTextureHandleResidentARB)
        kpanic("ext_glMakeTextureHandleResidentARB is NULL");
    {
    GL_FUNC(ext_glMakeTextureHandleResidentARB)(llARG1);
    GL_LOG ("glMakeTextureHandleResidentARB GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glMakeTextureHandleResidentNV(CPU* cpu) {
    if (!ext_glMakeTextureHandleResidentNV)
        kpanic("ext_glMakeTextureHandleResidentNV is NULL");
    {
    GL_FUNC(ext_glMakeTextureHandleResidentNV)(llARG1);
    GL_LOG ("glMakeTextureHandleResidentNV GLuint64 handle=" PRIu64 "",llARG1);
    }
}
void glcommon_glMap1xOES(CPU* cpu) {
    if (!ext_glMap1xOES)
        kpanic("ext_glMap1xOES is NULL");
    {
    GL_FUNC(ext_glMap1xOES)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glMap1xOES GLenum target=%d, GLfixed u1=%d, GLfixed u2=%d, GLint stride=%d, GLint order=%d, GLfixed points=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glMap2xOES(CPU* cpu) {
    if (!ext_glMap2xOES)
        kpanic("ext_glMap2xOES is NULL");
    {
    GL_FUNC(ext_glMap2xOES)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10);
    GL_LOG ("glMap2xOES GLenum target=%d, GLfixed u1=%d, GLfixed u2=%d, GLint ustride=%d, GLint uorder=%d, GLfixed v1=%d, GLfixed v2=%d, GLint vstride=%d, GLint vorder=%d, GLfixed points=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glMapBuffer(CPU* cpu) {
    if (!ext_glMapBuffer)
        kpanic("ext_glMapBuffer is NULL");
    {
    GLint size=0;void* ret=GL_FUNC(ext_glMapBuffer)(ARG1, ARG2);
    ext_glGetBufferParameteriv(ARG1, GL_BUFFER_SIZE, &size); 
    EAX = mapBufferRange(cpu, ARG1, ret, 0, size);
    GL_LOG ("glMapBuffer GLenum target=%d, GLenum access=%d",ARG1,ARG2);
    }
}
void glcommon_glMapBufferARB(CPU* cpu) {
    if (!ext_glMapBufferARB)
        kpanic("ext_glMapBufferARB is NULL");
    {
    GLint size=0;void* ret=GL_FUNC(ext_glMapBufferARB)(ARG1, ARG2);
    ext_glGetBufferParameterivARB(ARG1, GL_BUFFER_SIZE, &size);
    EAX = mapBufferRange(cpu, ARG1, ret, 0, size); 
    GL_LOG ("glMapBufferARB GLenum target=%d, GLenum access=%d",ARG1,ARG2);
    }
}
void glcommon_glMapBufferRange(CPU* cpu) {
    if (!ext_glMapBufferRange)
        kpanic("ext_glMapBufferRange is NULL");
    {
    void* ret=GL_FUNC(ext_glMapBufferRange)(ARG1, ARG2, ARG3, ARG4);

    if (ARG4 & GL_MAP_COHERENT_BIT) {
        static int shown;
        if (shown<100) {
            klog("glMapBufferRange with GL_MAP_COHERENT_BIT not supported");
            shown++;
        }
    }
    EAX=mapBufferRange(cpu, ARG1, ret, ARG2, ARG3);

    GL_LOG ("glMapBufferRange GLenum target=%d, GLintptr offset=%d, GLsizeiptr length=%d, GLbitfield access=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMapControlPointsNV(CPU* cpu) {
    if (!ext_glMapControlPointsNV)
        kpanic("ext_glMapControlPointsNV is NULL");
    {
    GL_FUNC(ext_glMapControlPointsNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, bARG8, (void*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glMapControlPointsNV GLenum target=%d, GLuint index=%d, GLenum type=%d, GLsizei ustride=%d, GLsizei vstride=%d, GLint uorder=%d, GLint vorder=%d, GLboolean packed=%d, const void* points=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,bARG8,ARG9);
    }
}
void glcommon_glMapGrid1xOES(CPU* cpu) {
    if (!ext_glMapGrid1xOES)
        kpanic("ext_glMapGrid1xOES is NULL");
    {
    GL_FUNC(ext_glMapGrid1xOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glMapGrid1xOES GLint n=%d, GLfixed u1=%d, GLfixed u2=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMapGrid2xOES(CPU* cpu) {
    if (!ext_glMapGrid2xOES)
        kpanic("ext_glMapGrid2xOES is NULL");
    {
    GL_FUNC(ext_glMapGrid2xOES)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glMapGrid2xOES GLint n=%d, GLfixed u1=%d, GLfixed u2=%d, GLfixed v1=%d, GLfixed v2=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMapNamedBuffer(CPU* cpu) {
    if (!ext_glMapNamedBuffer)
        kpanic("ext_glMapNamedBuffer is NULL");
    {
    GLint size=0;void* ret=GL_FUNC(ext_glMapNamedBuffer)(ARG1, ARG2);
    ext_glGetNamedBufferParameteriv(ARG1, GL_BUFFER_SIZE, &size); EAX=marshalBackp(cpu, ret, size);
    GL_LOG ("glMapNamedBuffer GLuint buffer=%d, GLenum access=%d",ARG1,ARG2);
    }
}
void glcommon_glMapNamedBufferEXT(CPU* cpu) {
    if (!ext_glMapNamedBufferEXT)
        kpanic("ext_glMapNamedBufferEXT is NULL");
    {
    GLint size=0;void* ret=GL_FUNC(ext_glMapNamedBufferEXT)(ARG1, ARG2);
    ext_glGetNamedBufferParameterivEXT(ARG1, GL_BUFFER_SIZE, &size); EAX=marshalBackp(cpu, ret, size);
    GL_LOG ("glMapNamedBufferEXT GLuint buffer=%d, GLenum access=%d",ARG1,ARG2);
    }
}
void glcommon_glMapNamedBufferRange(CPU* cpu) {
    if (!ext_glMapNamedBufferRange)
        kpanic("ext_glMapNamedBufferRange is NULL");
    {
    void* ret=GL_FUNC(ext_glMapNamedBufferRange)(ARG1, ARG2, ARG3, ARG4);
    EAX=marshalBackp(cpu, ret, ARG3);
    GL_LOG ("glMapNamedBufferRange GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr length=%d, GLbitfield access=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMapNamedBufferRangeEXT(CPU* cpu) {
    if (!ext_glMapNamedBufferRangeEXT)
        kpanic("ext_glMapNamedBufferRangeEXT is NULL");
    {
    void* ret=GL_FUNC(ext_glMapNamedBufferRangeEXT)(ARG1, ARG2, ARG3, ARG4);
    EAX=marshalBackp(cpu, ret, ARG3);
    GL_LOG ("glMapNamedBufferRangeEXT GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr length=%d, GLbitfield access=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMapObjectBufferATI(CPU* cpu) {
    if (!ext_glMapObjectBufferATI)
        kpanic("ext_glMapObjectBufferATI is NULL");
    {
    GL_FUNC(ext_glMapObjectBufferATI)(ARG1);
    GL_LOG ("glMapObjectBufferATI GLuint buffer=%d",ARG1);
    }
}
void glcommon_glMapParameterfvNV(CPU* cpu) {
    if (!ext_glMapParameterfvNV)
        kpanic("ext_glMapParameterfvNV is NULL");
    {
    GL_FUNC(ext_glMapParameterfvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glMapParameterfvNV GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMapParameterivNV(CPU* cpu) {
    if (!ext_glMapParameterivNV)
        kpanic("ext_glMapParameterivNV is NULL");
    {
    GL_FUNC(ext_glMapParameterivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glMapParameterivNV GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMapTexture2DINTEL(CPU* cpu) {
    if (!ext_glMapTexture2DINTEL)
        kpanic("ext_glMapTexture2DINTEL is NULL");
    {
    GL_FUNC(ext_glMapTexture2DINTEL)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0), (GLenum*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glMapTexture2DINTEL GLuint texture=%d, GLint level=%d, GLbitfield access=%d, GLint* stride=%.08x, GLenum* layout=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMapVertexAttrib1dAPPLE(CPU* cpu) {
    if (!ext_glMapVertexAttrib1dAPPLE)
        kpanic("ext_glMapVertexAttrib1dAPPLE is NULL");
    {
    GL_FUNC(ext_glMapVertexAttrib1dAPPLE)(ARG1, ARG2, dARG3, dARG4, ARG5, ARG6, (GLdouble*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glMapVertexAttrib1dAPPLE GLuint index=%d, GLuint size=%d, GLdouble u1=%f, GLdouble u2=%f, GLint stride=%d, GLint order=%d, const GLdouble* points=%.08x",ARG1,ARG2,dARG3,dARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glMapVertexAttrib1fAPPLE(CPU* cpu) {
    if (!ext_glMapVertexAttrib1fAPPLE)
        kpanic("ext_glMapVertexAttrib1fAPPLE is NULL");
    {
    GL_FUNC(ext_glMapVertexAttrib1fAPPLE)(ARG1, ARG2, fARG3, fARG4, ARG5, ARG6, (GLfloat*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glMapVertexAttrib1fAPPLE GLuint index=%d, GLuint size=%d, GLfloat u1=%f, GLfloat u2=%f, GLint stride=%d, GLint order=%d, const GLfloat* points=%.08x",ARG1,ARG2,fARG3,fARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glMapVertexAttrib2dAPPLE(CPU* cpu) {
    if (!ext_glMapVertexAttrib2dAPPLE)
        kpanic("ext_glMapVertexAttrib2dAPPLE is NULL");
    {
    GL_FUNC(ext_glMapVertexAttrib2dAPPLE)(ARG1, ARG2, dARG3, dARG4, ARG5, ARG6, dARG7, dARG8, ARG9, ARG10, (GLdouble*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glMapVertexAttrib2dAPPLE GLuint index=%d, GLuint size=%d, GLdouble u1=%f, GLdouble u2=%f, GLint ustride=%d, GLint uorder=%d, GLdouble v1=%f, GLdouble v2=%f, GLint vstride=%d, GLint vorder=%d, const GLdouble* points=%.08x",ARG1,ARG2,dARG3,dARG4,ARG5,ARG6,dARG7,dARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glMapVertexAttrib2fAPPLE(CPU* cpu) {
    if (!ext_glMapVertexAttrib2fAPPLE)
        kpanic("ext_glMapVertexAttrib2fAPPLE is NULL");
    {
    GL_FUNC(ext_glMapVertexAttrib2fAPPLE)(ARG1, ARG2, fARG3, fARG4, ARG5, ARG6, fARG7, fARG8, ARG9, ARG10, (GLfloat*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glMapVertexAttrib2fAPPLE GLuint index=%d, GLuint size=%d, GLfloat u1=%f, GLfloat u2=%f, GLint ustride=%d, GLint uorder=%d, GLfloat v1=%f, GLfloat v2=%f, GLint vstride=%d, GLint vorder=%d, const GLfloat* points=%.08x",ARG1,ARG2,fARG3,fARG4,ARG5,ARG6,fARG7,fARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glMaterialxOES(CPU* cpu) {
    if (!ext_glMaterialxOES)
        kpanic("ext_glMaterialxOES is NULL");
    {
    GL_FUNC(ext_glMaterialxOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glMaterialxOES GLenum face=%d, GLenum pname=%d, GLfixed param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMaterialxvOES(CPU* cpu) {
    if (!ext_glMaterialxvOES)
        kpanic("ext_glMaterialxvOES is NULL");
    {
    GL_FUNC(ext_glMaterialxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glMaterialxvOES GLenum face=%d, GLenum pname=%d, const GLfixed* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMatrixFrustumEXT(CPU* cpu) {
    if (!ext_glMatrixFrustumEXT)
        kpanic("ext_glMatrixFrustumEXT is NULL");
    {
    GL_FUNC(ext_glMatrixFrustumEXT)(ARG1, dARG2, dARG3, dARG4, dARG5, dARG6, dARG7);
    GL_LOG ("glMatrixFrustumEXT GLenum mode=%d, GLdouble left=%f, GLdouble right=%f, GLdouble bottom=%f, GLdouble top=%f, GLdouble zNear=%f, GLdouble zFar=%f",ARG1,dARG2,dARG3,dARG4,dARG5,dARG6,dARG7);
    }
}
void glcommon_glMatrixIndexPointerARB(CPU* cpu) {
    if (!ext_glMatrixIndexPointerARB)
        kpanic("ext_glMatrixIndexPointerARB is NULL");
    {
    GL_FUNC(ext_glMatrixIndexPointerARB)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMatrixIndexPointerARB GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMatrixIndexubvARB(CPU* cpu) {
    if (!ext_glMatrixIndexubvARB)
        kpanic("ext_glMatrixIndexubvARB is NULL");
    {
    GL_FUNC(ext_glMatrixIndexubvARB)(ARG1, (GLubyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixIndexubvARB GLint size=%d, const GLubyte* indices=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixIndexuivARB(CPU* cpu) {
    if (!ext_glMatrixIndexuivARB)
        kpanic("ext_glMatrixIndexuivARB is NULL");
    {
    GL_FUNC(ext_glMatrixIndexuivARB)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixIndexuivARB GLint size=%d, const GLuint* indices=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixIndexusvARB(CPU* cpu) {
    if (!ext_glMatrixIndexusvARB)
        kpanic("ext_glMatrixIndexusvARB is NULL");
    {
    GL_FUNC(ext_glMatrixIndexusvARB)(ARG1, (GLushort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixIndexusvARB GLint size=%d, const GLushort* indices=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixLoad3x2fNV(CPU* cpu) {
    if (!ext_glMatrixLoad3x2fNV)
        kpanic("ext_glMatrixLoad3x2fNV is NULL");
    {
    GL_FUNC(ext_glMatrixLoad3x2fNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixLoad3x2fNV GLenum matrixMode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixLoad3x3fNV(CPU* cpu) {
    if (!ext_glMatrixLoad3x3fNV)
        kpanic("ext_glMatrixLoad3x3fNV is NULL");
    {
    GL_FUNC(ext_glMatrixLoad3x3fNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixLoad3x3fNV GLenum matrixMode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixLoadIdentityEXT(CPU* cpu) {
    if (!ext_glMatrixLoadIdentityEXT)
        kpanic("ext_glMatrixLoadIdentityEXT is NULL");
    {
    GL_FUNC(ext_glMatrixLoadIdentityEXT)(ARG1);
    GL_LOG ("glMatrixLoadIdentityEXT GLenum mode=%d",ARG1);
    }
}
void glcommon_glMatrixLoadTranspose3x3fNV(CPU* cpu) {
    if (!ext_glMatrixLoadTranspose3x3fNV)
        kpanic("ext_glMatrixLoadTranspose3x3fNV is NULL");
    {
    GL_FUNC(ext_glMatrixLoadTranspose3x3fNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixLoadTranspose3x3fNV GLenum matrixMode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixLoadTransposedEXT(CPU* cpu) {
    if (!ext_glMatrixLoadTransposedEXT)
        kpanic("ext_glMatrixLoadTransposedEXT is NULL");
    {
    GL_FUNC(ext_glMatrixLoadTransposedEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixLoadTransposedEXT GLenum mode=%d, const GLdouble* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixLoadTransposefEXT(CPU* cpu) {
    if (!ext_glMatrixLoadTransposefEXT)
        kpanic("ext_glMatrixLoadTransposefEXT is NULL");
    {
    GL_FUNC(ext_glMatrixLoadTransposefEXT)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixLoadTransposefEXT GLenum mode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixLoaddEXT(CPU* cpu) {
    if (!ext_glMatrixLoaddEXT)
        kpanic("ext_glMatrixLoaddEXT is NULL");
    {
    GL_FUNC(ext_glMatrixLoaddEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixLoaddEXT GLenum mode=%d, const GLdouble* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixLoadfEXT(CPU* cpu) {
    if (!ext_glMatrixLoadfEXT)
        kpanic("ext_glMatrixLoadfEXT is NULL");
    {
    GL_FUNC(ext_glMatrixLoadfEXT)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixLoadfEXT GLenum mode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixMult3x2fNV(CPU* cpu) {
    if (!ext_glMatrixMult3x2fNV)
        kpanic("ext_glMatrixMult3x2fNV is NULL");
    {
    GL_FUNC(ext_glMatrixMult3x2fNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixMult3x2fNV GLenum matrixMode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixMult3x3fNV(CPU* cpu) {
    if (!ext_glMatrixMult3x3fNV)
        kpanic("ext_glMatrixMult3x3fNV is NULL");
    {
    GL_FUNC(ext_glMatrixMult3x3fNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixMult3x3fNV GLenum matrixMode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixMultTranspose3x3fNV(CPU* cpu) {
    if (!ext_glMatrixMultTranspose3x3fNV)
        kpanic("ext_glMatrixMultTranspose3x3fNV is NULL");
    {
    GL_FUNC(ext_glMatrixMultTranspose3x3fNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixMultTranspose3x3fNV GLenum matrixMode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixMultTransposedEXT(CPU* cpu) {
    if (!ext_glMatrixMultTransposedEXT)
        kpanic("ext_glMatrixMultTransposedEXT is NULL");
    {
    GL_FUNC(ext_glMatrixMultTransposedEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixMultTransposedEXT GLenum mode=%d, const GLdouble* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixMultTransposefEXT(CPU* cpu) {
    if (!ext_glMatrixMultTransposefEXT)
        kpanic("ext_glMatrixMultTransposefEXT is NULL");
    {
    GL_FUNC(ext_glMatrixMultTransposefEXT)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixMultTransposefEXT GLenum mode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixMultdEXT(CPU* cpu) {
    if (!ext_glMatrixMultdEXT)
        kpanic("ext_glMatrixMultdEXT is NULL");
    {
    GL_FUNC(ext_glMatrixMultdEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixMultdEXT GLenum mode=%d, const GLdouble* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixMultfEXT(CPU* cpu) {
    if (!ext_glMatrixMultfEXT)
        kpanic("ext_glMatrixMultfEXT is NULL");
    {
    GL_FUNC(ext_glMatrixMultfEXT)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMatrixMultfEXT GLenum mode=%d, const GLfloat* m=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMatrixOrthoEXT(CPU* cpu) {
    if (!ext_glMatrixOrthoEXT)
        kpanic("ext_glMatrixOrthoEXT is NULL");
    {
    GL_FUNC(ext_glMatrixOrthoEXT)(ARG1, dARG2, dARG3, dARG4, dARG5, dARG6, dARG7);
    GL_LOG ("glMatrixOrthoEXT GLenum mode=%d, GLdouble left=%f, GLdouble right=%f, GLdouble bottom=%f, GLdouble top=%f, GLdouble zNear=%f, GLdouble zFar=%f",ARG1,dARG2,dARG3,dARG4,dARG5,dARG6,dARG7);
    }
}
void glcommon_glMatrixPopEXT(CPU* cpu) {
    if (!ext_glMatrixPopEXT)
        kpanic("ext_glMatrixPopEXT is NULL");
    {
    GL_FUNC(ext_glMatrixPopEXT)(ARG1);
    GL_LOG ("glMatrixPopEXT GLenum mode=%d",ARG1);
    }
}
void glcommon_glMatrixPushEXT(CPU* cpu) {
    if (!ext_glMatrixPushEXT)
        kpanic("ext_glMatrixPushEXT is NULL");
    {
    GL_FUNC(ext_glMatrixPushEXT)(ARG1);
    GL_LOG ("glMatrixPushEXT GLenum mode=%d",ARG1);
    }
}
void glcommon_glMatrixRotatedEXT(CPU* cpu) {
    if (!ext_glMatrixRotatedEXT)
        kpanic("ext_glMatrixRotatedEXT is NULL");
    {
    GL_FUNC(ext_glMatrixRotatedEXT)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glMatrixRotatedEXT GLenum mode=%d, GLdouble angle=%f, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glMatrixRotatefEXT(CPU* cpu) {
    if (!ext_glMatrixRotatefEXT)
        kpanic("ext_glMatrixRotatefEXT is NULL");
    {
    GL_FUNC(ext_glMatrixRotatefEXT)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glMatrixRotatefEXT GLenum mode=%d, GLfloat angle=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glMatrixScaledEXT(CPU* cpu) {
    if (!ext_glMatrixScaledEXT)
        kpanic("ext_glMatrixScaledEXT is NULL");
    {
    GL_FUNC(ext_glMatrixScaledEXT)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glMatrixScaledEXT GLenum mode=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glMatrixScalefEXT(CPU* cpu) {
    if (!ext_glMatrixScalefEXT)
        kpanic("ext_glMatrixScalefEXT is NULL");
    {
    GL_FUNC(ext_glMatrixScalefEXT)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glMatrixScalefEXT GLenum mode=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glMatrixTranslatedEXT(CPU* cpu) {
    if (!ext_glMatrixTranslatedEXT)
        kpanic("ext_glMatrixTranslatedEXT is NULL");
    {
    GL_FUNC(ext_glMatrixTranslatedEXT)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glMatrixTranslatedEXT GLenum mode=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glMatrixTranslatefEXT(CPU* cpu) {
    if (!ext_glMatrixTranslatefEXT)
        kpanic("ext_glMatrixTranslatefEXT is NULL");
    {
    GL_FUNC(ext_glMatrixTranslatefEXT)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glMatrixTranslatefEXT GLenum mode=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glMaxShaderCompilerThreadsARB(CPU* cpu) {
    if (!ext_glMaxShaderCompilerThreadsARB)
        kpanic("ext_glMaxShaderCompilerThreadsARB is NULL");
    {
    GL_FUNC(ext_glMaxShaderCompilerThreadsARB)(ARG1);
    GL_LOG ("glMaxShaderCompilerThreadsARB GLuint count=%d",ARG1);
    }
}
void glcommon_glMemoryBarrier(CPU* cpu) {
    if (!ext_glMemoryBarrier)
        kpanic("ext_glMemoryBarrier is NULL");
    {
    GL_FUNC(ext_glMemoryBarrier)(ARG1);
    GL_LOG ("glMemoryBarrier GLbitfield barriers=%d",ARG1);
    }
}
void glcommon_glMemoryBarrierByRegion(CPU* cpu) {
    if (!ext_glMemoryBarrierByRegion)
        kpanic("ext_glMemoryBarrierByRegion is NULL");
    {
    GL_FUNC(ext_glMemoryBarrierByRegion)(ARG1);
    GL_LOG ("glMemoryBarrierByRegion GLbitfield barriers=%d",ARG1);
    }
}
void glcommon_glMemoryBarrierEXT(CPU* cpu) {
    if (!ext_glMemoryBarrierEXT)
        kpanic("ext_glMemoryBarrierEXT is NULL");
    {
    GL_FUNC(ext_glMemoryBarrierEXT)(ARG1);
    GL_LOG ("glMemoryBarrierEXT GLbitfield barriers=%d",ARG1);
    }
}
void glcommon_glMinSampleShading(CPU* cpu) {
    if (!ext_glMinSampleShading)
        kpanic("ext_glMinSampleShading is NULL");
    {
    GL_FUNC(ext_glMinSampleShading)(fARG1);
    GL_LOG ("glMinSampleShading GLfloat value=%f",fARG1);
    }
}
void glcommon_glMinSampleShadingARB(CPU* cpu) {
    if (!ext_glMinSampleShadingARB)
        kpanic("ext_glMinSampleShadingARB is NULL");
    {
    GL_FUNC(ext_glMinSampleShadingARB)(fARG1);
    GL_LOG ("glMinSampleShadingARB GLfloat value=%f",fARG1);
    }
}
void glcommon_glMinmax(CPU* cpu) {
    if (!ext_glMinmax)
        kpanic("ext_glMinmax is NULL");
    {
    GL_FUNC(ext_glMinmax)(ARG1, ARG2, bARG3);
    GL_LOG ("glMinmax GLenum target=%d, GLenum internalformat=%d, GLboolean sink=%d",ARG1,ARG2,bARG3);
    }
}
void glcommon_glMinmaxEXT(CPU* cpu) {
    if (!ext_glMinmaxEXT)
        kpanic("ext_glMinmaxEXT is NULL");
    {
    GL_FUNC(ext_glMinmaxEXT)(ARG1, ARG2, bARG3);
    GL_LOG ("glMinmaxEXT GLenum target=%d, GLenum internalformat=%d, GLboolean sink=%d",ARG1,ARG2,bARG3);
    }
}
void glcommon_glMultMatrixxOES(CPU* cpu) {
    if (!ext_glMultMatrixxOES)
        kpanic("ext_glMultMatrixxOES is NULL");
    {
    GL_FUNC(ext_glMultMatrixxOES)(marshalArray<GLint>(cpu, ARG1, 16));
    GL_LOG ("glMultMatrixxOES const GLfixed* m=%.08x",ARG1);
    }
}
void glcommon_glMultTransposeMatrixd(CPU* cpu) {
    if (!ext_glMultTransposeMatrixd)
        kpanic("ext_glMultTransposeMatrixd is NULL");
    {
    GL_FUNC(ext_glMultTransposeMatrixd)(marshalArray<GLdouble>(cpu, ARG1, 16));
    GL_LOG ("glMultTransposeMatrixd const GLdouble* m=%.08x",ARG1);
    }
}
void glcommon_glMultTransposeMatrixdARB(CPU* cpu) {
    if (!ext_glMultTransposeMatrixdARB)
        kpanic("ext_glMultTransposeMatrixdARB is NULL");
    {
    GL_FUNC(ext_glMultTransposeMatrixdARB)(marshalArray<GLdouble>(cpu, ARG1, 16));
    GL_LOG ("glMultTransposeMatrixdARB const GLdouble* m=%.08x",ARG1);
    }
}
void glcommon_glMultTransposeMatrixf(CPU* cpu) {
    if (!ext_glMultTransposeMatrixf)
        kpanic("ext_glMultTransposeMatrixf is NULL");
    {
    GL_FUNC(ext_glMultTransposeMatrixf)(marshalArray<GLfloat>(cpu, ARG1, 16));
    GL_LOG ("glMultTransposeMatrixf const GLfloat* m=%.08x",ARG1);
    }
}
void glcommon_glMultTransposeMatrixfARB(CPU* cpu) {
    if (!ext_glMultTransposeMatrixfARB)
        kpanic("ext_glMultTransposeMatrixfARB is NULL");
    {
    GL_FUNC(ext_glMultTransposeMatrixfARB)(marshalArray<GLfloat>(cpu, ARG1, 16));
    GL_LOG ("glMultTransposeMatrixfARB const GLfloat* m=%.08x",ARG1);
    }
}
void glcommon_glMultTransposeMatrixxOES(CPU* cpu) {
    if (!ext_glMultTransposeMatrixxOES)
        kpanic("ext_glMultTransposeMatrixxOES is NULL");
    {
    GL_FUNC(ext_glMultTransposeMatrixxOES)(marshalArray<GLint>(cpu, ARG1, 16));
    GL_LOG ("glMultTransposeMatrixxOES const GLfixed* m=%.08x",ARG1);
    }
}
void glcommon_glMultiDrawArrays(CPU* cpu) {
    if (!ext_glMultiDrawArrays)
        kpanic("ext_glMultiDrawArrays is NULL");
    {
    GL_FUNC(ext_glMultiDrawArrays)(ARG1, marshalArray<GLint>(cpu, ARG2, ARG4), marshalArray<GLint>(cpu, ARG3, ARG4), ARG4);
    GL_LOG ("glMultiDrawArrays GLenum mode=%d, const GLint* first=%.08x, const GLsizei* count=%.08x, GLsizei drawcount=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiDrawArraysEXT(CPU* cpu) {
    if (!ext_glMultiDrawArraysEXT)
        kpanic("ext_glMultiDrawArraysEXT is NULL");
    {
    GL_FUNC(ext_glMultiDrawArraysEXT)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0), (GLsizei*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glMultiDrawArraysEXT GLenum mode=%d, const GLint* first=%.08x, const GLsizei* count=%.08x, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiDrawArraysIndirect(CPU* cpu) {
    if (!ext_glMultiDrawArraysIndirect)
        kpanic("ext_glMultiDrawArraysIndirect is NULL");
    {
    GL_FUNC(ext_glMultiDrawArraysIndirect)(ARG1, marshalArray<GLuint>(cpu, ARG2, ((ARG4?ARG4:16)*ARG3)/4), ARG3, ARG4);
    GL_LOG ("glMultiDrawArraysIndirect GLenum mode=%d, const void* indirect=%.08x, GLsizei drawcount=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiDrawArraysIndirectAMD(CPU* cpu) {
    if (!ext_glMultiDrawArraysIndirectAMD)
        kpanic("ext_glMultiDrawArraysIndirectAMD is NULL");
    {
    GL_FUNC(ext_glMultiDrawArraysIndirectAMD)(ARG1, marshalArray<GLuint>(cpu, ARG2, ((ARG4?ARG4:16)*ARG3)/4), ARG3, ARG4);
    GL_LOG ("glMultiDrawArraysIndirectAMD GLenum mode=%d, const void* indirect=%.08x, GLsizei primcount=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiDrawArraysIndirectBindlessCountNV(CPU* cpu) {
    if (!ext_glMultiDrawArraysIndirectBindlessCountNV)
        kpanic("ext_glMultiDrawArraysIndirectBindlessCountNV is NULL");
    {
    GL_FUNC(ext_glMultiDrawArraysIndirectBindlessCountNV)(ARG1, marshalDrawArraysIndirectBindlessCommandNV(cpu, ARG2, ARG4, ARG5, ARG6), ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glMultiDrawArraysIndirectBindlessCountNV GLenum mode=%d, const void* indirect=%.08x, GLsizei drawCount=%d, GLsizei maxDrawCount=%d, GLsizei stride=%d, GLint vertexBufferCount=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glMultiDrawArraysIndirectBindlessNV(CPU* cpu) {
    if (!ext_glMultiDrawArraysIndirectBindlessNV)
        kpanic("ext_glMultiDrawArraysIndirectBindlessNV is NULL");
    {
    GL_FUNC(ext_glMultiDrawArraysIndirectBindlessNV)(ARG1, marshalDrawArraysIndirectBindlessCommandNV(cpu, ARG2, ARG3, ARG4, ARG5), ARG3, ARG4, ARG5);
    GL_LOG ("glMultiDrawArraysIndirectBindlessNV GLenum mode=%d, const void* indirect=%.08x, GLsizei drawCount=%d, GLsizei stride=%d, GLint vertexBufferCount=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiDrawArraysIndirectCountARB(CPU* cpu) {
    if (!ext_glMultiDrawArraysIndirectCountARB)
        kpanic("ext_glMultiDrawArraysIndirectCountARB is NULL");
    {
    GL_FUNC(ext_glMultiDrawArraysIndirectCountARB)(ARG1, (GLintptr)marshalArray<GLuint>(cpu, ARG2, ((ARG5?ARG5:16)*ARG4)/4), ARG3, ARG4, ARG5);
    GL_LOG ("glMultiDrawArraysIndirectCountARB GLenum mode=%d, GLintptr indirect=%d, GLintptr drawcount=%d, GLsizei maxdrawcount=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiDrawElementArrayAPPLE(CPU* cpu) {
    if (!ext_glMultiDrawElementArrayAPPLE)
        kpanic("ext_glMultiDrawElementArrayAPPLE is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementArrayAPPLE)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0), (GLsizei*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glMultiDrawElementArrayAPPLE GLenum mode=%d, const GLint* first=%.08x, const GLsizei* count=%.08x, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiDrawElements(CPU* cpu) {
    if (!ext_glMultiDrawElements)
        kpanic("ext_glMultiDrawElements is NULL");
    {
    GL_FUNC(ext_glMultiDrawElements)(ARG1, (GLsizei*)marshalp(cpu, 0, ARG2, 0), ARG3, (void*const*)marshalpp(cpu, ARG4, ARG5, ARG2, getDataSize(ARG3), 0), ARG5);
    GL_LOG ("glMultiDrawElements GLenum mode=%d, const GLsizei* count=%.08x, GLenum type=%d, const void*const* indices=%.08x, GLsizei drawcount=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiDrawElementsBaseVertex(CPU* cpu) {
    if (!ext_glMultiDrawElementsBaseVertex)
        kpanic("ext_glMultiDrawElementsBaseVertex is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementsBaseVertex)(ARG1, (GLsizei*)marshalp(cpu, 0, ARG2, 0), ARG3, (void*const*)marshalpp(cpu, ARG4, ARG5, ARG2, getDataSize(ARG3), 0), ARG5, (GLint*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glMultiDrawElementsBaseVertex GLenum mode=%d, const GLsizei* count=%.08x, GLenum type=%d, const void*const* indices=%.08x, GLsizei drawcount=%d, const GLint* basevertex=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glMultiDrawElementsEXT(CPU* cpu) {
    if (!ext_glMultiDrawElementsEXT)
        kpanic("ext_glMultiDrawElementsEXT is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementsEXT)(ARG1, (GLsizei*)marshalp(cpu, 0, ARG2, 0), ARG3, (void*const*)marshalpp(cpu, ARG4, ARG5, ARG2, getDataSize(ARG3), 0), ARG5);
    GL_LOG ("glMultiDrawElementsEXT GLenum mode=%d, const GLsizei* count=%.08x, GLenum type=%d, const void*const* indices=%.08x, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiDrawElementsIndirect(CPU* cpu) {
    if (!ext_glMultiDrawElementsIndirect)
        kpanic("ext_glMultiDrawElementsIndirect is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementsIndirect)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5);
    GL_LOG ("glMultiDrawElementsIndirect GLenum mode=%d, GLenum type=%d, const void* indirect=%.08x, GLsizei drawcount=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiDrawElementsIndirectAMD(CPU* cpu) {
    if (!ext_glMultiDrawElementsIndirectAMD)
        kpanic("ext_glMultiDrawElementsIndirectAMD is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementsIndirectAMD)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5);
    GL_LOG ("glMultiDrawElementsIndirectAMD GLenum mode=%d, GLenum type=%d, const void* indirect=%.08x, GLsizei primcount=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiDrawElementsIndirectBindlessCountNV(CPU* cpu) {
    if (!ext_glMultiDrawElementsIndirectBindlessCountNV)
        kpanic("ext_glMultiDrawElementsIndirectBindlessCountNV is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementsIndirectBindlessCountNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glMultiDrawElementsIndirectBindlessCountNV GLenum mode=%d, GLenum type=%d, const void* indirect=%.08x, GLsizei drawCount=%d, GLsizei maxDrawCount=%d, GLsizei stride=%d, GLint vertexBufferCount=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glMultiDrawElementsIndirectBindlessNV(CPU* cpu) {
    if (!ext_glMultiDrawElementsIndirectBindlessNV)
        kpanic("ext_glMultiDrawElementsIndirectBindlessNV is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementsIndirectBindlessNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6);
    GL_LOG ("glMultiDrawElementsIndirectBindlessNV GLenum mode=%d, GLenum type=%d, const void* indirect=%.08x, GLsizei drawCount=%d, GLsizei stride=%d, GLint vertexBufferCount=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glMultiDrawElementsIndirectCountARB(CPU* cpu) {
    if (!ext_glMultiDrawElementsIndirectCountARB)
        kpanic("ext_glMultiDrawElementsIndirectCountARB is NULL");
    {
    GL_FUNC(ext_glMultiDrawElementsIndirectCountARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glMultiDrawElementsIndirectCountARB GLenum mode=%d, GLenum type=%d, GLintptr indirect=%d, GLintptr drawcount=%d, GLsizei maxdrawcount=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glMultiDrawRangeElementArrayAPPLE(CPU* cpu) {
    if (!ext_glMultiDrawRangeElementArrayAPPLE)
        kpanic("ext_glMultiDrawRangeElementArrayAPPLE is NULL");
    {
    GL_FUNC(ext_glMultiDrawRangeElementArrayAPPLE)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0), (GLsizei*)marshalp(cpu, 0, ARG5, 0), ARG6);
    GL_LOG ("glMultiDrawRangeElementArrayAPPLE GLenum mode=%d, GLuint start=%d, GLuint end=%d, const GLint* first=%.08x, const GLsizei* count=%.08x, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glMultiModeDrawArraysIBM(CPU* cpu) {
    if (!ext_glMultiModeDrawArraysIBM)
        kpanic("ext_glMultiModeDrawArraysIBM is NULL");
    {
    GL_FUNC(ext_glMultiModeDrawArraysIBM)((GLenum*)marshalp(cpu, 0, ARG1, 0), (GLint*)marshalp(cpu, 0, ARG2, 0), (GLsizei*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5);
    GL_LOG ("glMultiModeDrawArraysIBM const GLenum* mode=%.08x, const GLint* first=%.08x, const GLsizei* count=%.08x, GLsizei primcount=%d, GLint modestride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiModeDrawElementsIBM(CPU* cpu) {
    if (!ext_glMultiModeDrawElementsIBM)
        kpanic("ext_glMultiModeDrawElementsIBM is NULL");
    {
    GL_FUNC(ext_glMultiModeDrawElementsIBM)((GLenum*)marshalp(cpu, 0, ARG1, 0), (GLsizei*)marshalp(cpu, 0, ARG2, 0), ARG3, (void*const*)marshalpp(cpu, ARG4, ARG5, ARG2, getDataSize(ARG3), 0), ARG5, ARG6);
    GL_LOG ("glMultiModeDrawElementsIBM const GLenum* mode=%.08x, const GLsizei* count=%.08x, GLenum type=%d, const void*const* indices=%.08x, GLsizei primcount=%d, GLint modestride=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glMultiTexBufferEXT(CPU* cpu) {
    if (!ext_glMultiTexBufferEXT)
        kpanic("ext_glMultiTexBufferEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexBufferEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexBufferEXT GLenum texunit=%d, GLenum target=%d, GLenum internalformat=%d, GLuint buffer=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexCoord1bOES(CPU* cpu) {
    if (!ext_glMultiTexCoord1bOES)
        kpanic("ext_glMultiTexCoord1bOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1bOES)(ARG1, bARG2);
    GL_LOG ("glMultiTexCoord1bOES GLenum texture=%d, GLbyte s=%d",ARG1,bARG2);
    }
}
void glcommon_glMultiTexCoord1bvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord1bvOES)
        kpanic("ext_glMultiTexCoord1bvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1bvOES)(ARG1, (GLbyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1bvOES GLenum texture=%d, const GLbyte* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1d(CPU* cpu) {
    if (!ext_glMultiTexCoord1d)
        kpanic("ext_glMultiTexCoord1d is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1d)(ARG1, dARG2);
    GL_LOG ("glMultiTexCoord1d GLenum target=%d, GLdouble s=%f",ARG1,dARG2);
    }
}
void glcommon_glMultiTexCoord1dARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1dARB)
        kpanic("ext_glMultiTexCoord1dARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1dARB)(ARG1, dARG2);
    GL_LOG ("glMultiTexCoord1dARB GLenum target=%d, GLdouble s=%f",ARG1,dARG2);
    }
}
void glcommon_glMultiTexCoord1dSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1dSGIS)
        kpanic("ext_glMultiTexCoord1dSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1dSGIS)(ARG1, dARG2);
    GL_LOG ("glMultiTexCoord1dSGIS GLenum target=%d, GLdouble s=%f",ARG1,dARG2);
    }
}
void glcommon_glMultiTexCoord1dv(CPU* cpu) {
    if (!ext_glMultiTexCoord1dv)
        kpanic("ext_glMultiTexCoord1dv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1dv GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1dvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1dvARB)
        kpanic("ext_glMultiTexCoord1dvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1dvARB)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1dvARB GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1dvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1dvSGIS)
        kpanic("ext_glMultiTexCoord1dvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1dvSGIS)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1dvSGIS GLenum target=%d, GLdouble * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1f(CPU* cpu) {
    if (!ext_glMultiTexCoord1f)
        kpanic("ext_glMultiTexCoord1f is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1f)(ARG1, fARG2);
    GL_LOG ("glMultiTexCoord1f GLenum target=%d, GLfloat s=%f",ARG1,fARG2);
    }
}
void glcommon_glMultiTexCoord1fARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1fARB)
        kpanic("ext_glMultiTexCoord1fARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1fARB)(ARG1, fARG2);
    GL_LOG ("glMultiTexCoord1fARB GLenum target=%d, GLfloat s=%f",ARG1,fARG2);
    }
}
void glcommon_glMultiTexCoord1fSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1fSGIS)
        kpanic("ext_glMultiTexCoord1fSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1fSGIS)(ARG1, fARG2);
    GL_LOG ("glMultiTexCoord1fSGIS GLenum target=%d, GLfloat s=%f",ARG1,fARG2);
    }
}
void glcommon_glMultiTexCoord1fv(CPU* cpu) {
    if (!ext_glMultiTexCoord1fv)
        kpanic("ext_glMultiTexCoord1fv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1fv)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1fv GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1fvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1fvARB)
        kpanic("ext_glMultiTexCoord1fvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1fvARB)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1fvARB GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1fvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1fvSGIS)
        kpanic("ext_glMultiTexCoord1fvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1fvSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1fvSGIS GLenum target=%d, const GLfloat * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1hNV(CPU* cpu) {
    if (!ext_glMultiTexCoord1hNV)
        kpanic("ext_glMultiTexCoord1hNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1hNV)(ARG1, sARG2);
    GL_LOG ("glMultiTexCoord1hNV GLenum target=%d, GLhalfNV s=%d",ARG1,sARG2);
    }
}
void glcommon_glMultiTexCoord1hvNV(CPU* cpu) {
    if (!ext_glMultiTexCoord1hvNV)
        kpanic("ext_glMultiTexCoord1hvNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1hvNV)(ARG1, (GLhalfNV*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1hvNV GLenum target=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1i(CPU* cpu) {
    if (!ext_glMultiTexCoord1i)
        kpanic("ext_glMultiTexCoord1i is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1i)(ARG1, ARG2);
    GL_LOG ("glMultiTexCoord1i GLenum target=%d, GLint s=%d",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1iARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1iARB)
        kpanic("ext_glMultiTexCoord1iARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1iARB)(ARG1, ARG2);
    GL_LOG ("glMultiTexCoord1iARB GLenum target=%d, GLint s=%d",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1iSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1iSGIS)
        kpanic("ext_glMultiTexCoord1iSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1iSGIS)(ARG1, ARG2);
    GL_LOG ("glMultiTexCoord1iSGIS GLenum target=%d, GLint s=%d",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1iv(CPU* cpu) {
    if (!ext_glMultiTexCoord1iv)
        kpanic("ext_glMultiTexCoord1iv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1iv)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1iv GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1ivARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1ivARB)
        kpanic("ext_glMultiTexCoord1ivARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1ivARB)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1ivARB GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1ivSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1ivSGIS)
        kpanic("ext_glMultiTexCoord1ivSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1ivSGIS)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1ivSGIS GLenum target=%d, GLint * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1s(CPU* cpu) {
    if (!ext_glMultiTexCoord1s)
        kpanic("ext_glMultiTexCoord1s is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1s)(ARG1, sARG2);
    GL_LOG ("glMultiTexCoord1s GLenum target=%d, GLshort s=%d",ARG1,sARG2);
    }
}
void glcommon_glMultiTexCoord1sARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1sARB)
        kpanic("ext_glMultiTexCoord1sARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1sARB)(ARG1, sARG2);
    GL_LOG ("glMultiTexCoord1sARB GLenum target=%d, GLshort s=%d",ARG1,sARG2);
    }
}
void glcommon_glMultiTexCoord1sSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1sSGIS)
        kpanic("ext_glMultiTexCoord1sSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1sSGIS)(ARG1, sARG2);
    GL_LOG ("glMultiTexCoord1sSGIS GLenum target=%d, GLshort s=%d",ARG1,sARG2);
    }
}
void glcommon_glMultiTexCoord1sv(CPU* cpu) {
    if (!ext_glMultiTexCoord1sv)
        kpanic("ext_glMultiTexCoord1sv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1sv)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1sv GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1svARB(CPU* cpu) {
    if (!ext_glMultiTexCoord1svARB)
        kpanic("ext_glMultiTexCoord1svARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1svARB)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1svARB GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1svSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord1svSGIS)
        kpanic("ext_glMultiTexCoord1svSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1svSGIS)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1svSGIS GLenum target=%d, GLshort * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1xOES(CPU* cpu) {
    if (!ext_glMultiTexCoord1xOES)
        kpanic("ext_glMultiTexCoord1xOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1xOES)(ARG1, ARG2);
    GL_LOG ("glMultiTexCoord1xOES GLenum texture=%d, GLfixed s=%d",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord1xvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord1xvOES)
        kpanic("ext_glMultiTexCoord1xvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord1xvOES)(ARG1, (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord1xvOES GLenum texture=%d, const GLfixed* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2bOES(CPU* cpu) {
    if (!ext_glMultiTexCoord2bOES)
        kpanic("ext_glMultiTexCoord2bOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2bOES)(ARG1, bARG2, bARG3);
    GL_LOG ("glMultiTexCoord2bOES GLenum texture=%d, GLbyte s=%d, GLbyte t=%d",ARG1,bARG2,bARG3);
    }
}
void glcommon_glMultiTexCoord2bvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord2bvOES)
        kpanic("ext_glMultiTexCoord2bvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2bvOES)(ARG1, (GLbyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2bvOES GLenum texture=%d, const GLbyte* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2d(CPU* cpu) {
    if (!ext_glMultiTexCoord2d)
        kpanic("ext_glMultiTexCoord2d is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2d)(ARG1, dARG2, dARG3);
    GL_LOG ("glMultiTexCoord2d GLenum target=%d, GLdouble s=%f, GLdouble t=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glMultiTexCoord2dARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2dARB)
        kpanic("ext_glMultiTexCoord2dARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2dARB)(ARG1, dARG2, dARG3);
    GL_LOG ("glMultiTexCoord2dARB GLenum target=%d, GLdouble s=%f, GLdouble t=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glMultiTexCoord2dSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2dSGIS)
        kpanic("ext_glMultiTexCoord2dSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2dSGIS)(ARG1, dARG2, dARG3);
    GL_LOG ("glMultiTexCoord2dSGIS GLenum target=%d, GLdouble s=%f, GLdouble t=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glMultiTexCoord2dv(CPU* cpu) {
    if (!ext_glMultiTexCoord2dv)
        kpanic("ext_glMultiTexCoord2dv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2dv GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2dvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2dvARB)
        kpanic("ext_glMultiTexCoord2dvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2dvARB)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2dvARB GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2dvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2dvSGIS)
        kpanic("ext_glMultiTexCoord2dvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2dvSGIS)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2dvSGIS GLenum target=%d, GLdouble * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2f(CPU* cpu) {
    if (!ext_glMultiTexCoord2f)
        kpanic("ext_glMultiTexCoord2f is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2f)(ARG1, fARG2, fARG3);
    GL_LOG ("glMultiTexCoord2f GLenum target=%d, GLfloat s=%f, GLfloat t=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glMultiTexCoord2fARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2fARB)
        kpanic("ext_glMultiTexCoord2fARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2fARB)(ARG1, fARG2, fARG3);
    GL_LOG ("glMultiTexCoord2fARB GLenum target=%d, GLfloat s=%f, GLfloat t=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glMultiTexCoord2fSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2fSGIS)
        kpanic("ext_glMultiTexCoord2fSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2fSGIS)(ARG1, fARG2, fARG3);
    GL_LOG ("glMultiTexCoord2fSGIS GLenum target=%d, GLfloat s=%f, GLfloat t=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glMultiTexCoord2fv(CPU* cpu) {
    if (!ext_glMultiTexCoord2fv)
        kpanic("ext_glMultiTexCoord2fv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2fv)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2fv GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2fvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2fvARB)
        kpanic("ext_glMultiTexCoord2fvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2fvARB)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2fvARB GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2fvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2fvSGIS)
        kpanic("ext_glMultiTexCoord2fvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2fvSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2fvSGIS GLenum target=%d, GLfloat * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2hNV(CPU* cpu) {
    if (!ext_glMultiTexCoord2hNV)
        kpanic("ext_glMultiTexCoord2hNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2hNV)(ARG1, sARG2, sARG3);
    GL_LOG ("glMultiTexCoord2hNV GLenum target=%d, GLhalfNV s=%d, GLhalfNV t=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glMultiTexCoord2hvNV(CPU* cpu) {
    if (!ext_glMultiTexCoord2hvNV)
        kpanic("ext_glMultiTexCoord2hvNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2hvNV)(ARG1, (GLhalfNV*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2hvNV GLenum target=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2i(CPU* cpu) {
    if (!ext_glMultiTexCoord2i)
        kpanic("ext_glMultiTexCoord2i is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2i)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoord2i GLenum target=%d, GLint s=%d, GLint t=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoord2iARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2iARB)
        kpanic("ext_glMultiTexCoord2iARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2iARB)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoord2iARB GLenum target=%d, GLint s=%d, GLint t=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoord2iSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2iSGIS)
        kpanic("ext_glMultiTexCoord2iSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2iSGIS)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoord2iSGIS GLenum target=%d, GLint s=%d, GLint t=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoord2iv(CPU* cpu) {
    if (!ext_glMultiTexCoord2iv)
        kpanic("ext_glMultiTexCoord2iv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2iv)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2iv GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2ivARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2ivARB)
        kpanic("ext_glMultiTexCoord2ivARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2ivARB)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2ivARB GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2ivSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2ivSGIS)
        kpanic("ext_glMultiTexCoord2ivSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2ivSGIS)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2ivSGIS GLenum target=%d, GLint * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2s(CPU* cpu) {
    if (!ext_glMultiTexCoord2s)
        kpanic("ext_glMultiTexCoord2s is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2s)(ARG1, sARG2, sARG3);
    GL_LOG ("glMultiTexCoord2s GLenum target=%d, GLshort s=%d, GLshort t=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glMultiTexCoord2sARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2sARB)
        kpanic("ext_glMultiTexCoord2sARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2sARB)(ARG1, sARG2, sARG3);
    GL_LOG ("glMultiTexCoord2sARB GLenum target=%d, GLshort s=%d, GLshort t=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glMultiTexCoord2sSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2sSGIS)
        kpanic("ext_glMultiTexCoord2sSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2sSGIS)(ARG1, sARG2, sARG3);
    GL_LOG ("glMultiTexCoord2sSGIS GLenum target=%d, GLshort s=%d, GLshort t=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glMultiTexCoord2sv(CPU* cpu) {
    if (!ext_glMultiTexCoord2sv)
        kpanic("ext_glMultiTexCoord2sv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2sv)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2sv GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2svARB(CPU* cpu) {
    if (!ext_glMultiTexCoord2svARB)
        kpanic("ext_glMultiTexCoord2svARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2svARB)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2svARB GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2svSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord2svSGIS)
        kpanic("ext_glMultiTexCoord2svSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2svSGIS)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2svSGIS GLenum target=%d, GLshort * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord2xOES(CPU* cpu) {
    if (!ext_glMultiTexCoord2xOES)
        kpanic("ext_glMultiTexCoord2xOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2xOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoord2xOES GLenum texture=%d, GLfixed s=%d, GLfixed t=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoord2xvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord2xvOES)
        kpanic("ext_glMultiTexCoord2xvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord2xvOES)(ARG1, (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord2xvOES GLenum texture=%d, const GLfixed* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3bOES(CPU* cpu) {
    if (!ext_glMultiTexCoord3bOES)
        kpanic("ext_glMultiTexCoord3bOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3bOES)(ARG1, bARG2, bARG3, bARG4);
    GL_LOG ("glMultiTexCoord3bOES GLenum texture=%d, GLbyte s=%d, GLbyte t=%d, GLbyte r=%d",ARG1,bARG2,bARG3,bARG4);
    }
}
void glcommon_glMultiTexCoord3bvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord3bvOES)
        kpanic("ext_glMultiTexCoord3bvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3bvOES)(ARG1, (GLbyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3bvOES GLenum texture=%d, const GLbyte* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3d(CPU* cpu) {
    if (!ext_glMultiTexCoord3d)
        kpanic("ext_glMultiTexCoord3d is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3d)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glMultiTexCoord3d GLenum target=%d, GLdouble s=%f, GLdouble t=%f, GLdouble r=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glMultiTexCoord3dARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3dARB)
        kpanic("ext_glMultiTexCoord3dARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3dARB)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glMultiTexCoord3dARB GLenum target=%d, GLdouble s=%f, GLdouble t=%f, GLdouble r=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glMultiTexCoord3dSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3dSGIS)
        kpanic("ext_glMultiTexCoord3dSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3dSGIS)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glMultiTexCoord3dSGIS GLenum target=%d, GLdouble s=%f, GLdouble t=%f, GLdouble r=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glMultiTexCoord3dv(CPU* cpu) {
    if (!ext_glMultiTexCoord3dv)
        kpanic("ext_glMultiTexCoord3dv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3dv GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3dvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3dvARB)
        kpanic("ext_glMultiTexCoord3dvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3dvARB)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3dvARB GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3dvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3dvSGIS)
        kpanic("ext_glMultiTexCoord3dvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3dvSGIS)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3dvSGIS GLenum target=%d, GLdouble * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3f(CPU* cpu) {
    if (!ext_glMultiTexCoord3f)
        kpanic("ext_glMultiTexCoord3f is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3f)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glMultiTexCoord3f GLenum target=%d, GLfloat s=%f, GLfloat t=%f, GLfloat r=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glMultiTexCoord3fARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3fARB)
        kpanic("ext_glMultiTexCoord3fARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3fARB)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glMultiTexCoord3fARB GLenum target=%d, GLfloat s=%f, GLfloat t=%f, GLfloat r=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glMultiTexCoord3fSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3fSGIS)
        kpanic("ext_glMultiTexCoord3fSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3fSGIS)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glMultiTexCoord3fSGIS GLenum target=%d, GLfloat s=%f, GLfloat t=%f, GLfloat r=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glMultiTexCoord3fv(CPU* cpu) {
    if (!ext_glMultiTexCoord3fv)
        kpanic("ext_glMultiTexCoord3fv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3fv)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3fv GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3fvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3fvARB)
        kpanic("ext_glMultiTexCoord3fvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3fvARB)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3fvARB GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3fvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3fvSGIS)
        kpanic("ext_glMultiTexCoord3fvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3fvSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3fvSGIS GLenum target=%d, GLfloat * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3hNV(CPU* cpu) {
    if (!ext_glMultiTexCoord3hNV)
        kpanic("ext_glMultiTexCoord3hNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3hNV)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glMultiTexCoord3hNV GLenum target=%d, GLhalfNV s=%d, GLhalfNV t=%d, GLhalfNV r=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glMultiTexCoord3hvNV(CPU* cpu) {
    if (!ext_glMultiTexCoord3hvNV)
        kpanic("ext_glMultiTexCoord3hvNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3hvNV)(ARG1, (GLhalfNV*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3hvNV GLenum target=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3i(CPU* cpu) {
    if (!ext_glMultiTexCoord3i)
        kpanic("ext_glMultiTexCoord3i is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3i)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexCoord3i GLenum target=%d, GLint s=%d, GLint t=%d, GLint r=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexCoord3iARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3iARB)
        kpanic("ext_glMultiTexCoord3iARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3iARB)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexCoord3iARB GLenum target=%d, GLint s=%d, GLint t=%d, GLint r=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexCoord3iSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3iSGIS)
        kpanic("ext_glMultiTexCoord3iSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3iSGIS)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexCoord3iSGIS GLenum target=%d, GLint s=%d, GLint t=%d, GLint r=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexCoord3iv(CPU* cpu) {
    if (!ext_glMultiTexCoord3iv)
        kpanic("ext_glMultiTexCoord3iv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3iv)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3iv GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3ivARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3ivARB)
        kpanic("ext_glMultiTexCoord3ivARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3ivARB)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3ivARB GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3ivSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3ivSGIS)
        kpanic("ext_glMultiTexCoord3ivSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3ivSGIS)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3ivSGIS GLenum target=%d, GLint * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3s(CPU* cpu) {
    if (!ext_glMultiTexCoord3s)
        kpanic("ext_glMultiTexCoord3s is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3s)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glMultiTexCoord3s GLenum target=%d, GLshort s=%d, GLshort t=%d, GLshort r=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glMultiTexCoord3sARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3sARB)
        kpanic("ext_glMultiTexCoord3sARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3sARB)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glMultiTexCoord3sARB GLenum target=%d, GLshort s=%d, GLshort t=%d, GLshort r=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glMultiTexCoord3sSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3sSGIS)
        kpanic("ext_glMultiTexCoord3sSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3sSGIS)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glMultiTexCoord3sSGIS GLenum target=%d, GLshort s=%d, GLshort t=%d, GLshort r=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glMultiTexCoord3sv(CPU* cpu) {
    if (!ext_glMultiTexCoord3sv)
        kpanic("ext_glMultiTexCoord3sv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3sv)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3sv GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3svARB(CPU* cpu) {
    if (!ext_glMultiTexCoord3svARB)
        kpanic("ext_glMultiTexCoord3svARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3svARB)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3svARB GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3svSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord3svSGIS)
        kpanic("ext_glMultiTexCoord3svSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3svSGIS)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3svSGIS GLenum target=%d, GLshort * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord3xOES(CPU* cpu) {
    if (!ext_glMultiTexCoord3xOES)
        kpanic("ext_glMultiTexCoord3xOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3xOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexCoord3xOES GLenum texture=%d, GLfixed s=%d, GLfixed t=%d, GLfixed r=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexCoord3xvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord3xvOES)
        kpanic("ext_glMultiTexCoord3xvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord3xvOES)(ARG1, (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord3xvOES GLenum texture=%d, const GLfixed* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4bOES(CPU* cpu) {
    if (!ext_glMultiTexCoord4bOES)
        kpanic("ext_glMultiTexCoord4bOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4bOES)(ARG1, bARG2, bARG3, bARG4, bARG5);
    GL_LOG ("glMultiTexCoord4bOES GLenum texture=%d, GLbyte s=%d, GLbyte t=%d, GLbyte r=%d, GLbyte q=%d",ARG1,bARG2,bARG3,bARG4,bARG5);
    }
}
void glcommon_glMultiTexCoord4bvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord4bvOES)
        kpanic("ext_glMultiTexCoord4bvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4bvOES)(ARG1, (GLbyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4bvOES GLenum texture=%d, const GLbyte* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4d(CPU* cpu) {
    if (!ext_glMultiTexCoord4d)
        kpanic("ext_glMultiTexCoord4d is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4d)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glMultiTexCoord4d GLenum target=%d, GLdouble s=%f, GLdouble t=%f, GLdouble r=%f, GLdouble q=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glMultiTexCoord4dARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4dARB)
        kpanic("ext_glMultiTexCoord4dARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4dARB)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glMultiTexCoord4dARB GLenum target=%d, GLdouble s=%f, GLdouble t=%f, GLdouble r=%f, GLdouble q=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glMultiTexCoord4dSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4dSGIS)
        kpanic("ext_glMultiTexCoord4dSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4dSGIS)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glMultiTexCoord4dSGIS GLenum target=%d, GLdouble s=%f, GLdouble t=%f, GLdouble r=%f, GLdouble q=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glMultiTexCoord4dv(CPU* cpu) {
    if (!ext_glMultiTexCoord4dv)
        kpanic("ext_glMultiTexCoord4dv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4dv GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4dvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4dvARB)
        kpanic("ext_glMultiTexCoord4dvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4dvARB)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4dvARB GLenum target=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4dvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4dvSGIS)
        kpanic("ext_glMultiTexCoord4dvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4dvSGIS)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4dvSGIS GLenum target=%d, GLdouble * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4f(CPU* cpu) {
    if (!ext_glMultiTexCoord4f)
        kpanic("ext_glMultiTexCoord4f is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4f)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glMultiTexCoord4f GLenum target=%d, GLfloat s=%f, GLfloat t=%f, GLfloat r=%f, GLfloat q=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glMultiTexCoord4fARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4fARB)
        kpanic("ext_glMultiTexCoord4fARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4fARB)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glMultiTexCoord4fARB GLenum target=%d, GLfloat s=%f, GLfloat t=%f, GLfloat r=%f, GLfloat q=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glMultiTexCoord4fSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4fSGIS)
        kpanic("ext_glMultiTexCoord4fSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4fSGIS)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glMultiTexCoord4fSGIS GLenum target=%d, GLfloat s=%f, GLfloat t=%f, GLfloat r=%f, GLfloat q=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glMultiTexCoord4fv(CPU* cpu) {
    if (!ext_glMultiTexCoord4fv)
        kpanic("ext_glMultiTexCoord4fv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4fv)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4fv GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4fvARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4fvARB)
        kpanic("ext_glMultiTexCoord4fvARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4fvARB)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4fvARB GLenum target=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4fvSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4fvSGIS)
        kpanic("ext_glMultiTexCoord4fvSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4fvSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4fvSGIS GLenum target=%d, GLfloat * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4hNV(CPU* cpu) {
    if (!ext_glMultiTexCoord4hNV)
        kpanic("ext_glMultiTexCoord4hNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4hNV)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glMultiTexCoord4hNV GLenum target=%d, GLhalfNV s=%d, GLhalfNV t=%d, GLhalfNV r=%d, GLhalfNV q=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glMultiTexCoord4hvNV(CPU* cpu) {
    if (!ext_glMultiTexCoord4hvNV)
        kpanic("ext_glMultiTexCoord4hvNV is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4hvNV)(ARG1, (GLhalfNV*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4hvNV GLenum target=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4i(CPU* cpu) {
    if (!ext_glMultiTexCoord4i)
        kpanic("ext_glMultiTexCoord4i is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4i)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glMultiTexCoord4i GLenum target=%d, GLint s=%d, GLint t=%d, GLint r=%d, GLint q=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiTexCoord4iARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4iARB)
        kpanic("ext_glMultiTexCoord4iARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4iARB)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glMultiTexCoord4iARB GLenum target=%d, GLint s=%d, GLint t=%d, GLint r=%d, GLint q=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiTexCoord4iSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4iSGIS)
        kpanic("ext_glMultiTexCoord4iSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4iSGIS)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glMultiTexCoord4iSGIS GLenum target=%d, GLint s=%d, GLint t=%d, GLint r=%d, GLint q=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiTexCoord4iv(CPU* cpu) {
    if (!ext_glMultiTexCoord4iv)
        kpanic("ext_glMultiTexCoord4iv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4iv)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4iv GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4ivARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4ivARB)
        kpanic("ext_glMultiTexCoord4ivARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4ivARB)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4ivARB GLenum target=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4ivSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4ivSGIS)
        kpanic("ext_glMultiTexCoord4ivSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4ivSGIS)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4ivSGIS GLenum target=%d, GLint * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4s(CPU* cpu) {
    if (!ext_glMultiTexCoord4s)
        kpanic("ext_glMultiTexCoord4s is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4s)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glMultiTexCoord4s GLenum target=%d, GLshort s=%d, GLshort t=%d, GLshort r=%d, GLshort q=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glMultiTexCoord4sARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4sARB)
        kpanic("ext_glMultiTexCoord4sARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4sARB)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glMultiTexCoord4sARB GLenum target=%d, GLshort s=%d, GLshort t=%d, GLshort r=%d, GLshort q=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glMultiTexCoord4sSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4sSGIS)
        kpanic("ext_glMultiTexCoord4sSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4sSGIS)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glMultiTexCoord4sSGIS GLenum target=%d, GLshort s=%d, GLshort t=%d, GLshort r=%d, GLshort q=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glMultiTexCoord4sv(CPU* cpu) {
    if (!ext_glMultiTexCoord4sv)
        kpanic("ext_glMultiTexCoord4sv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4sv)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4sv GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4svARB(CPU* cpu) {
    if (!ext_glMultiTexCoord4svARB)
        kpanic("ext_glMultiTexCoord4svARB is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4svARB)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4svARB GLenum target=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4svSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoord4svSGIS)
        kpanic("ext_glMultiTexCoord4svSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4svSGIS)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4svSGIS GLenum target=%d, GLshort * v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoord4xOES(CPU* cpu) {
    if (!ext_glMultiTexCoord4xOES)
        kpanic("ext_glMultiTexCoord4xOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4xOES)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glMultiTexCoord4xOES GLenum texture=%d, GLfixed s=%d, GLfixed t=%d, GLfixed r=%d, GLfixed q=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiTexCoord4xvOES(CPU* cpu) {
    if (!ext_glMultiTexCoord4xvOES)
        kpanic("ext_glMultiTexCoord4xvOES is NULL");
    {
    GL_FUNC(ext_glMultiTexCoord4xvOES)(ARG1, (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glMultiTexCoord4xvOES GLenum texture=%d, const GLfixed* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glMultiTexCoordP1ui(CPU* cpu) {
    if (!ext_glMultiTexCoordP1ui)
        kpanic("ext_glMultiTexCoordP1ui is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP1ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoordP1ui GLenum texture=%d, GLenum type=%d, GLuint coords=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordP1uiv(CPU* cpu) {
    if (!ext_glMultiTexCoordP1uiv)
        kpanic("ext_glMultiTexCoordP1uiv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP1uiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glMultiTexCoordP1uiv GLenum texture=%d, GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordP2ui(CPU* cpu) {
    if (!ext_glMultiTexCoordP2ui)
        kpanic("ext_glMultiTexCoordP2ui is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP2ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoordP2ui GLenum texture=%d, GLenum type=%d, GLuint coords=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordP2uiv(CPU* cpu) {
    if (!ext_glMultiTexCoordP2uiv)
        kpanic("ext_glMultiTexCoordP2uiv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP2uiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glMultiTexCoordP2uiv GLenum texture=%d, GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordP3ui(CPU* cpu) {
    if (!ext_glMultiTexCoordP3ui)
        kpanic("ext_glMultiTexCoordP3ui is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP3ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoordP3ui GLenum texture=%d, GLenum type=%d, GLuint coords=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordP3uiv(CPU* cpu) {
    if (!ext_glMultiTexCoordP3uiv)
        kpanic("ext_glMultiTexCoordP3uiv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP3uiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glMultiTexCoordP3uiv GLenum texture=%d, GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordP4ui(CPU* cpu) {
    if (!ext_glMultiTexCoordP4ui)
        kpanic("ext_glMultiTexCoordP4ui is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP4ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexCoordP4ui GLenum texture=%d, GLenum type=%d, GLuint coords=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordP4uiv(CPU* cpu) {
    if (!ext_glMultiTexCoordP4uiv)
        kpanic("ext_glMultiTexCoordP4uiv is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordP4uiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glMultiTexCoordP4uiv GLenum texture=%d, GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexCoordPointerEXT(CPU* cpu) {
    if (!ext_glMultiTexCoordPointerEXT)
        kpanic("ext_glMultiTexCoordPointerEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordPointerEXT)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glMultiTexCoordPointerEXT GLenum texunit=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiTexCoordPointerSGIS(CPU* cpu) {
    if (!ext_glMultiTexCoordPointerSGIS)
        kpanic("ext_glMultiTexCoordPointerSGIS is NULL");
    {
    GL_FUNC(ext_glMultiTexCoordPointerSGIS)(ARG1, ARG2, ARG3, ARG4, (GLvoid*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glMultiTexCoordPointerSGIS GLenum target=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLvoid * pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glMultiTexEnvfEXT(CPU* cpu) {
    if (!ext_glMultiTexEnvfEXT)
        kpanic("ext_glMultiTexEnvfEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexEnvfEXT)(ARG1, ARG2, ARG3, fARG4);
    GL_LOG ("glMultiTexEnvfEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,ARG3,fARG4);
    }
}
void glcommon_glMultiTexEnvfvEXT(CPU* cpu) {
    if (!ext_glMultiTexEnvfvEXT)
        kpanic("ext_glMultiTexEnvfvEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexEnvfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexEnvfvEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexEnviEXT(CPU* cpu) {
    if (!ext_glMultiTexEnviEXT)
        kpanic("ext_glMultiTexEnviEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexEnviEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexEnviEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexEnvivEXT(CPU* cpu) {
    if (!ext_glMultiTexEnvivEXT)
        kpanic("ext_glMultiTexEnvivEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexEnvivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexEnvivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexGendEXT(CPU* cpu) {
    if (!ext_glMultiTexGendEXT)
        kpanic("ext_glMultiTexGendEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexGendEXT)(ARG1, ARG2, ARG3, dARG4);
    GL_LOG ("glMultiTexGendEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, GLdouble param=%f",ARG1,ARG2,ARG3,dARG4);
    }
}
void glcommon_glMultiTexGendvEXT(CPU* cpu) {
    if (!ext_glMultiTexGendvEXT)
        kpanic("ext_glMultiTexGendvEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexGendvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexGendvEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, const GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexGenfEXT(CPU* cpu) {
    if (!ext_glMultiTexGenfEXT)
        kpanic("ext_glMultiTexGenfEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexGenfEXT)(ARG1, ARG2, ARG3, fARG4);
    GL_LOG ("glMultiTexGenfEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,ARG3,fARG4);
    }
}
void glcommon_glMultiTexGenfvEXT(CPU* cpu) {
    if (!ext_glMultiTexGenfvEXT)
        kpanic("ext_glMultiTexGenfvEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexGenfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexGenfvEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexGeniEXT(CPU* cpu) {
    if (!ext_glMultiTexGeniEXT)
        kpanic("ext_glMultiTexGeniEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexGeniEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexGeniEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexGenivEXT(CPU* cpu) {
    if (!ext_glMultiTexGenivEXT)
        kpanic("ext_glMultiTexGenivEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexGenivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexGenivEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexImage1DEXT(CPU* cpu) {
    if (!ext_glMultiTexImage1DEXT)
        kpanic("ext_glMultiTexImage1DEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, (void*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glMultiTexImage1DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint internalformat=%d, GLsizei width=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glMultiTexImage2DEXT(CPU* cpu) {
    if (!ext_glMultiTexImage2DEXT)
        kpanic("ext_glMultiTexImage2DEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, (void*)marshalp(cpu, 0, ARG10, 0));
    GL_LOG ("glMultiTexImage2DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glMultiTexImage3DEXT(CPU* cpu) {
    if (!ext_glMultiTexImage3DEXT)
        kpanic("ext_glMultiTexImage3DEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, (void*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glMultiTexImage3DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glMultiTexParameterIivEXT(CPU* cpu) {
    if (!ext_glMultiTexParameterIivEXT)
        kpanic("ext_glMultiTexParameterIivEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexParameterIivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexParameterIivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexParameterIuivEXT(CPU* cpu) {
    if (!ext_glMultiTexParameterIuivEXT)
        kpanic("ext_glMultiTexParameterIuivEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexParameterIuivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexParameterIuivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexParameterfEXT(CPU* cpu) {
    if (!ext_glMultiTexParameterfEXT)
        kpanic("ext_glMultiTexParameterfEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexParameterfEXT)(ARG1, ARG2, ARG3, fARG4);
    GL_LOG ("glMultiTexParameterfEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,ARG3,fARG4);
    }
}
void glcommon_glMultiTexParameterfvEXT(CPU* cpu) {
    if (!ext_glMultiTexParameterfvEXT)
        kpanic("ext_glMultiTexParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexParameterfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexParameterfvEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexParameteriEXT(CPU* cpu) {
    if (!ext_glMultiTexParameteriEXT)
        kpanic("ext_glMultiTexParameteriEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexParameteriEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glMultiTexParameteriEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexParameterivEXT(CPU* cpu) {
    if (!ext_glMultiTexParameterivEXT)
        kpanic("ext_glMultiTexParameterivEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexParameterivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glMultiTexParameterivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glMultiTexRenderbufferEXT(CPU* cpu) {
    if (!ext_glMultiTexRenderbufferEXT)
        kpanic("ext_glMultiTexRenderbufferEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexRenderbufferEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glMultiTexRenderbufferEXT GLenum texunit=%d, GLenum target=%d, GLuint renderbuffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glMultiTexSubImage1DEXT(CPU* cpu) {
    if (!ext_glMultiTexSubImage1DEXT)
        kpanic("ext_glMultiTexSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glMultiTexSubImage1DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glMultiTexSubImage2DEXT(CPU* cpu) {
    if (!ext_glMultiTexSubImage2DEXT)
        kpanic("ext_glMultiTexSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, (void*)marshalp(cpu, 0, ARG10, 0));
    GL_LOG ("glMultiTexSubImage2DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glMultiTexSubImage3DEXT(CPU* cpu) {
    if (!ext_glMultiTexSubImage3DEXT)
        kpanic("ext_glMultiTexSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glMultiTexSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, (void*)marshalp(cpu, 0, ARG12, 0));
    GL_LOG ("glMultiTexSubImage3DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12);
    }
}
void glcommon_glNamedBufferData(CPU* cpu) {
    if (!ext_glNamedBufferData)
        kpanic("ext_glNamedBufferData is NULL");
    {
    GL_FUNC(ext_glNamedBufferData)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glNamedBufferData GLuint buffer=%d, GLsizeiptr size=%d, const void* data=%.08x, GLenum usage=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedBufferDataEXT(CPU* cpu) {
    if (!ext_glNamedBufferDataEXT)
        kpanic("ext_glNamedBufferDataEXT is NULL");
    {
    GL_FUNC(ext_glNamedBufferDataEXT)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glNamedBufferDataEXT GLuint buffer=%d, GLsizeiptr size=%d, const void* data=%.08x, GLenum usage=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedBufferPageCommitmentARB(CPU* cpu) {
    if (!ext_glNamedBufferPageCommitmentARB)
        kpanic("ext_glNamedBufferPageCommitmentARB is NULL");
    {
    GL_FUNC(ext_glNamedBufferPageCommitmentARB)(ARG1, ARG2, ARG3, bARG4);
    GL_LOG ("glNamedBufferPageCommitmentARB GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d, GLboolean commit=%d",ARG1,ARG2,ARG3,bARG4);
    }
}
void glcommon_glNamedBufferPageCommitmentEXT(CPU* cpu) {
    if (!ext_glNamedBufferPageCommitmentEXT)
        kpanic("ext_glNamedBufferPageCommitmentEXT is NULL");
    {
    GL_FUNC(ext_glNamedBufferPageCommitmentEXT)(ARG1, ARG2, ARG3, bARG4);
    GL_LOG ("glNamedBufferPageCommitmentEXT GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d, GLboolean commit=%d",ARG1,ARG2,ARG3,bARG4);
    }
}
void glcommon_glNamedBufferStorage(CPU* cpu) {
    if (!ext_glNamedBufferStorage)
        kpanic("ext_glNamedBufferStorage is NULL");
    {
    GL_FUNC(ext_glNamedBufferStorage)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glNamedBufferStorage GLuint buffer=%d, GLsizeiptr size=%d, const void* data=%.08x, GLbitfield flags=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedBufferStorageEXT(CPU* cpu) {
    if (!ext_glNamedBufferStorageEXT)
        kpanic("ext_glNamedBufferStorageEXT is NULL");
    {
    GL_FUNC(ext_glNamedBufferStorageEXT)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glNamedBufferStorageEXT GLuint buffer=%d, GLsizeiptr size=%d, const void* data=%.08x, GLbitfield flags=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedBufferSubData(CPU* cpu) {
    if (!ext_glNamedBufferSubData)
        kpanic("ext_glNamedBufferSubData is NULL");
    {
    GL_FUNC(ext_glNamedBufferSubData)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedBufferSubData GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedBufferSubDataEXT(CPU* cpu) {
    if (!ext_glNamedBufferSubDataEXT)
        kpanic("ext_glNamedBufferSubDataEXT is NULL");
    {
    GL_FUNC(ext_glNamedBufferSubDataEXT)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedBufferSubDataEXT GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedCopyBufferSubDataEXT(CPU* cpu) {
    if (!ext_glNamedCopyBufferSubDataEXT)
        kpanic("ext_glNamedCopyBufferSubDataEXT is NULL");
    {
    GL_FUNC(ext_glNamedCopyBufferSubDataEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedCopyBufferSubDataEXT GLuint readBuffer=%d, GLuint writeBuffer=%d, GLintptr readOffset=%d, GLintptr writeOffset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedFramebufferDrawBuffer(CPU* cpu) {
    if (!ext_glNamedFramebufferDrawBuffer)
        kpanic("ext_glNamedFramebufferDrawBuffer is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferDrawBuffer)(ARG1, ARG2);
    GL_LOG ("glNamedFramebufferDrawBuffer GLuint framebuffer=%d, GLenum buf=%d",ARG1,ARG2);
    }
}
void glcommon_glNamedFramebufferDrawBuffers(CPU* cpu) {
    if (!ext_glNamedFramebufferDrawBuffers)
        kpanic("ext_glNamedFramebufferDrawBuffers is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferDrawBuffers)(ARG1, ARG2, (GLenum*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glNamedFramebufferDrawBuffers GLuint framebuffer=%d, GLsizei n=%d, const GLenum* bufs=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glNamedFramebufferParameteri(CPU* cpu) {
    if (!ext_glNamedFramebufferParameteri)
        kpanic("ext_glNamedFramebufferParameteri is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferParameteri)(ARG1, ARG2, ARG3);
    GL_LOG ("glNamedFramebufferParameteri GLuint framebuffer=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glNamedFramebufferParameteriEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferParameteriEXT)
        kpanic("ext_glNamedFramebufferParameteriEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferParameteriEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glNamedFramebufferParameteriEXT GLuint framebuffer=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glNamedFramebufferReadBuffer(CPU* cpu) {
    if (!ext_glNamedFramebufferReadBuffer)
        kpanic("ext_glNamedFramebufferReadBuffer is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferReadBuffer)(ARG1, ARG2);
    GL_LOG ("glNamedFramebufferReadBuffer GLuint framebuffer=%d, GLenum src=%d",ARG1,ARG2);
    }
}
void glcommon_glNamedFramebufferRenderbuffer(CPU* cpu) {
    if (!ext_glNamedFramebufferRenderbuffer)
        kpanic("ext_glNamedFramebufferRenderbuffer is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferRenderbuffer)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glNamedFramebufferRenderbuffer GLuint framebuffer=%d, GLenum attachment=%d, GLenum renderbuffertarget=%d, GLuint renderbuffer=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedFramebufferRenderbufferEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferRenderbufferEXT)
        kpanic("ext_glNamedFramebufferRenderbufferEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferRenderbufferEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glNamedFramebufferRenderbufferEXT GLuint framebuffer=%d, GLenum attachment=%d, GLenum renderbuffertarget=%d, GLuint renderbuffer=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedFramebufferSampleLocationsfvARB(CPU* cpu) {
    if (!ext_glNamedFramebufferSampleLocationsfvARB)
        kpanic("ext_glNamedFramebufferSampleLocationsfvARB is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferSampleLocationsfvARB)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedFramebufferSampleLocationsfvARB GLuint framebuffer=%d, GLuint start=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedFramebufferSampleLocationsfvNV(CPU* cpu) {
    if (!ext_glNamedFramebufferSampleLocationsfvNV)
        kpanic("ext_glNamedFramebufferSampleLocationsfvNV is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferSampleLocationsfvNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedFramebufferSampleLocationsfvNV GLuint framebuffer=%d, GLuint start=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedFramebufferTexture(CPU* cpu) {
    if (!ext_glNamedFramebufferTexture)
        kpanic("ext_glNamedFramebufferTexture is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTexture)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glNamedFramebufferTexture GLuint framebuffer=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedFramebufferTexture1DEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferTexture1DEXT)
        kpanic("ext_glNamedFramebufferTexture1DEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTexture1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedFramebufferTexture1DEXT GLuint framebuffer=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedFramebufferTexture2DEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferTexture2DEXT)
        kpanic("ext_glNamedFramebufferTexture2DEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTexture2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedFramebufferTexture2DEXT GLuint framebuffer=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedFramebufferTexture3DEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferTexture3DEXT)
        kpanic("ext_glNamedFramebufferTexture3DEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTexture3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glNamedFramebufferTexture3DEXT GLuint framebuffer=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d, GLint zoffset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glNamedFramebufferTextureEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferTextureEXT)
        kpanic("ext_glNamedFramebufferTextureEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTextureEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glNamedFramebufferTextureEXT GLuint framebuffer=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedFramebufferTextureFaceEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferTextureFaceEXT)
        kpanic("ext_glNamedFramebufferTextureFaceEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTextureFaceEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedFramebufferTextureFaceEXT GLuint framebuffer=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLenum face=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedFramebufferTextureLayer(CPU* cpu) {
    if (!ext_glNamedFramebufferTextureLayer)
        kpanic("ext_glNamedFramebufferTextureLayer is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTextureLayer)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedFramebufferTextureLayer GLuint framebuffer=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLint layer=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedFramebufferTextureLayerEXT(CPU* cpu) {
    if (!ext_glNamedFramebufferTextureLayerEXT)
        kpanic("ext_glNamedFramebufferTextureLayerEXT is NULL");
    {
    GL_FUNC(ext_glNamedFramebufferTextureLayerEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedFramebufferTextureLayerEXT GLuint framebuffer=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLint layer=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedProgramLocalParameter4dEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameter4dEXT)
        kpanic("ext_glNamedProgramLocalParameter4dEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameter4dEXT)(ARG1, ARG2, ARG3, dARG4, dARG5, dARG6, dARG7);
    GL_LOG ("glNamedProgramLocalParameter4dEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,ARG2,ARG3,dARG4,dARG5,dARG6,dARG7);
    }
}
void glcommon_glNamedProgramLocalParameter4dvEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameter4dvEXT)
        kpanic("ext_glNamedProgramLocalParameter4dvEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameter4dvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedProgramLocalParameter4dvEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, const GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedProgramLocalParameter4fEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameter4fEXT)
        kpanic("ext_glNamedProgramLocalParameter4fEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameter4fEXT)(ARG1, ARG2, ARG3, fARG4, fARG5, fARG6, fARG7);
    GL_LOG ("glNamedProgramLocalParameter4fEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,ARG2,ARG3,fARG4,fARG5,fARG6,fARG7);
    }
}
void glcommon_glNamedProgramLocalParameter4fvEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameter4fvEXT)
        kpanic("ext_glNamedProgramLocalParameter4fvEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameter4fvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedProgramLocalParameter4fvEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedProgramLocalParameterI4iEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameterI4iEXT)
        kpanic("ext_glNamedProgramLocalParameterI4iEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameterI4iEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glNamedProgramLocalParameterI4iEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLint x=%d, GLint y=%d, GLint z=%d, GLint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glNamedProgramLocalParameterI4ivEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameterI4ivEXT)
        kpanic("ext_glNamedProgramLocalParameterI4ivEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameterI4ivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedProgramLocalParameterI4ivEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedProgramLocalParameterI4uiEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameterI4uiEXT)
        kpanic("ext_glNamedProgramLocalParameterI4uiEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameterI4uiEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glNamedProgramLocalParameterI4uiEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLuint x=%d, GLuint y=%d, GLuint z=%d, GLuint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glNamedProgramLocalParameterI4uivEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameterI4uivEXT)
        kpanic("ext_glNamedProgramLocalParameterI4uivEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameterI4uivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNamedProgramLocalParameterI4uivEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedProgramLocalParameters4fvEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParameters4fvEXT)
        kpanic("ext_glNamedProgramLocalParameters4fvEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParameters4fvEXT)(ARG1, ARG2, ARG3, ARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glNamedProgramLocalParameters4fvEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedProgramLocalParametersI4ivEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParametersI4ivEXT)
        kpanic("ext_glNamedProgramLocalParametersI4ivEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParametersI4ivEXT)(ARG1, ARG2, ARG3, ARG4, (GLint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glNamedProgramLocalParametersI4ivEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedProgramLocalParametersI4uivEXT(CPU* cpu) {
    if (!ext_glNamedProgramLocalParametersI4uivEXT)
        kpanic("ext_glNamedProgramLocalParametersI4uivEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramLocalParametersI4uivEXT)(ARG1, ARG2, ARG3, ARG4, (GLuint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glNamedProgramLocalParametersI4uivEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedProgramStringEXT(CPU* cpu) {
    if (!ext_glNamedProgramStringEXT)
        kpanic("ext_glNamedProgramStringEXT is NULL");
    {
    GL_FUNC(ext_glNamedProgramStringEXT)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glNamedProgramStringEXT GLuint program=%d, GLenum target=%d, GLenum format=%d, GLsizei len=%d, const void* string=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedRenderbufferStorage(CPU* cpu) {
    if (!ext_glNamedRenderbufferStorage)
        kpanic("ext_glNamedRenderbufferStorage is NULL");
    {
    GL_FUNC(ext_glNamedRenderbufferStorage)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glNamedRenderbufferStorage GLuint renderbuffer=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedRenderbufferStorageEXT(CPU* cpu) {
    if (!ext_glNamedRenderbufferStorageEXT)
        kpanic("ext_glNamedRenderbufferStorageEXT is NULL");
    {
    GL_FUNC(ext_glNamedRenderbufferStorageEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glNamedRenderbufferStorageEXT GLuint renderbuffer=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNamedRenderbufferStorageMultisample(CPU* cpu) {
    if (!ext_glNamedRenderbufferStorageMultisample)
        kpanic("ext_glNamedRenderbufferStorageMultisample is NULL");
    {
    GL_FUNC(ext_glNamedRenderbufferStorageMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedRenderbufferStorageMultisample GLuint renderbuffer=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedRenderbufferStorageMultisampleCoverageEXT(CPU* cpu) {
    if (!ext_glNamedRenderbufferStorageMultisampleCoverageEXT)
        kpanic("ext_glNamedRenderbufferStorageMultisampleCoverageEXT is NULL");
    {
    GL_FUNC(ext_glNamedRenderbufferStorageMultisampleCoverageEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glNamedRenderbufferStorageMultisampleCoverageEXT GLuint renderbuffer=%d, GLsizei coverageSamples=%d, GLsizei colorSamples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glNamedRenderbufferStorageMultisampleEXT(CPU* cpu) {
    if (!ext_glNamedRenderbufferStorageMultisampleEXT)
        kpanic("ext_glNamedRenderbufferStorageMultisampleEXT is NULL");
    {
    GL_FUNC(ext_glNamedRenderbufferStorageMultisampleEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glNamedRenderbufferStorageMultisampleEXT GLuint renderbuffer=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNamedStringARB(CPU* cpu) {
    if (!ext_glNamedStringARB)
        kpanic("ext_glNamedStringARB is NULL");
    {
    GL_FUNC(ext_glNamedStringARB)(ARG1, ARG2, marshalsz(cpu, ARG3), ARG4, marshalsz(cpu, ARG5));
    GL_LOG ("glNamedStringARB GLenum type=%d, GLint namelen=%d, const GLchar* name=%.08x, GLint stringlen=%d, const GLchar* string=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glNewBufferRegion(CPU* cpu) {
    if (!ext_glNewBufferRegion)
        kpanic("ext_glNewBufferRegion is NULL");
    {
    EAX=GL_FUNC(ext_glNewBufferRegion)(ARG1);
    GL_LOG ("glNewBufferRegion GLenum type=%d",ARG1);
    }
}
void glcommon_glNewObjectBufferATI(CPU* cpu) {
    if (!ext_glNewObjectBufferATI)
        kpanic("ext_glNewObjectBufferATI is NULL");
    {
    EAX=GL_FUNC(ext_glNewObjectBufferATI)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0), ARG3);
    GL_LOG ("glNewObjectBufferATI GLsizei size=%d, const void* pointer=%.08x, GLenum usage=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glNormal3fVertex3fSUN)
        kpanic("ext_glNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glNormal3fVertex3fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glNormal3fVertex3fSUN GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glNormal3fVertex3fvSUN)
        kpanic("ext_glNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glNormal3fVertex3fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glNormal3fVertex3fvSUN const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glNormal3hNV(CPU* cpu) {
    if (!ext_glNormal3hNV)
        kpanic("ext_glNormal3hNV is NULL");
    {
    GL_FUNC(ext_glNormal3hNV)(sARG1, sARG2, sARG3);
    GL_LOG ("glNormal3hNV GLhalfNV nx=%d, GLhalfNV ny=%d, GLhalfNV nz=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glNormal3hvNV(CPU* cpu) {
    if (!ext_glNormal3hvNV)
        kpanic("ext_glNormal3hvNV is NULL");
    {
    GL_FUNC(ext_glNormal3hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glNormal3hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glNormal3xOES(CPU* cpu) {
    if (!ext_glNormal3xOES)
        kpanic("ext_glNormal3xOES is NULL");
    {
    GL_FUNC(ext_glNormal3xOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glNormal3xOES GLfixed nx=%d, GLfixed ny=%d, GLfixed nz=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glNormal3xvOES(CPU* cpu) {
    if (!ext_glNormal3xvOES)
        kpanic("ext_glNormal3xvOES is NULL");
    {
    GL_FUNC(ext_glNormal3xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glNormal3xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glNormalFormatNV(CPU* cpu) {
    if (!ext_glNormalFormatNV)
        kpanic("ext_glNormalFormatNV is NULL");
    {
    GL_FUNC(ext_glNormalFormatNV)(ARG1, ARG2);
    GL_LOG ("glNormalFormatNV GLenum type=%d, GLsizei stride=%d",ARG1,ARG2);
    }
}
void glcommon_glNormalP3ui(CPU* cpu) {
    if (!ext_glNormalP3ui)
        kpanic("ext_glNormalP3ui is NULL");
    {
    GL_FUNC(ext_glNormalP3ui)(ARG1, ARG2);
    GL_LOG ("glNormalP3ui GLenum type=%d, GLuint coords=%d",ARG1,ARG2);
    }
}
void glcommon_glNormalP3uiv(CPU* cpu) {
    if (!ext_glNormalP3uiv)
        kpanic("ext_glNormalP3uiv is NULL");
    {
    GL_FUNC(ext_glNormalP3uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glNormalP3uiv GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glNormalPointerEXT(CPU* cpu) {
    if (!ext_glNormalPointerEXT)
        kpanic("ext_glNormalPointerEXT is NULL");
    {
    GL_FUNC(ext_glNormalPointerEXT)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glNormalPointerEXT GLenum type=%d, GLsizei stride=%d, GLsizei count=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNormalPointerListIBM(CPU* cpu) {
    if (!ext_glNormalPointerListIBM)
        kpanic("ext_glNormalPointerListIBM is NULL");
    {
    GL_FUNC(ext_glNormalPointerListIBM)(ARG1, ARG2, (const void**)marshalunhandled("glNormalPointerListIBM", "pointer", cpu, ARG3), ARG4);
    GL_LOG ("glNormalPointerListIBM GLenum type=%d, GLint stride=%d, const void** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNormalPointervINTEL(CPU* cpu) {
    if (!ext_glNormalPointervINTEL)
        kpanic("ext_glNormalPointervINTEL is NULL");
    {
    GL_FUNC(ext_glNormalPointervINTEL)(ARG1, (const void**)marshalunhandled("glNormalPointervINTEL", "pointer", cpu, ARG2));
    GL_LOG ("glNormalPointervINTEL GLenum type=%d, const void** pointer=%.08x",ARG1,ARG2);
    }
}
void glcommon_glNormalStream3bATI(CPU* cpu) {
    if (!ext_glNormalStream3bATI)
        kpanic("ext_glNormalStream3bATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3bATI)(ARG1, bARG2, bARG3, bARG4);
    GL_LOG ("glNormalStream3bATI GLenum stream=%d, GLbyte nx=%d, GLbyte ny=%d, GLbyte nz=%d",ARG1,bARG2,bARG3,bARG4);
    }
}
void glcommon_glNormalStream3bvATI(CPU* cpu) {
    if (!ext_glNormalStream3bvATI)
        kpanic("ext_glNormalStream3bvATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3bvATI)(ARG1, (GLbyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glNormalStream3bvATI GLenum stream=%d, const GLbyte* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glNormalStream3dATI(CPU* cpu) {
    if (!ext_glNormalStream3dATI)
        kpanic("ext_glNormalStream3dATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3dATI)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glNormalStream3dATI GLenum stream=%d, GLdouble nx=%f, GLdouble ny=%f, GLdouble nz=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glNormalStream3dvATI(CPU* cpu) {
    if (!ext_glNormalStream3dvATI)
        kpanic("ext_glNormalStream3dvATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3dvATI)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glNormalStream3dvATI GLenum stream=%d, const GLdouble* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glNormalStream3fATI(CPU* cpu) {
    if (!ext_glNormalStream3fATI)
        kpanic("ext_glNormalStream3fATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3fATI)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glNormalStream3fATI GLenum stream=%d, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glNormalStream3fvATI(CPU* cpu) {
    if (!ext_glNormalStream3fvATI)
        kpanic("ext_glNormalStream3fvATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3fvATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glNormalStream3fvATI GLenum stream=%d, const GLfloat* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glNormalStream3iATI(CPU* cpu) {
    if (!ext_glNormalStream3iATI)
        kpanic("ext_glNormalStream3iATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3iATI)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glNormalStream3iATI GLenum stream=%d, GLint nx=%d, GLint ny=%d, GLint nz=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glNormalStream3ivATI(CPU* cpu) {
    if (!ext_glNormalStream3ivATI)
        kpanic("ext_glNormalStream3ivATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3ivATI)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glNormalStream3ivATI GLenum stream=%d, const GLint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glNormalStream3sATI(CPU* cpu) {
    if (!ext_glNormalStream3sATI)
        kpanic("ext_glNormalStream3sATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3sATI)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glNormalStream3sATI GLenum stream=%d, GLshort nx=%d, GLshort ny=%d, GLshort nz=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glNormalStream3svATI(CPU* cpu) {
    if (!ext_glNormalStream3svATI)
        kpanic("ext_glNormalStream3svATI is NULL");
    {
    GL_FUNC(ext_glNormalStream3svATI)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glNormalStream3svATI GLenum stream=%d, const GLshort* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glObjectLabel(CPU* cpu) {
    if (!ext_glObjectLabel)
        kpanic("ext_glObjectLabel is NULL");
    {
    GL_FUNC(ext_glObjectLabel)(ARG1, ARG2, ARG3, marshalsz(cpu, ARG4));
    GL_LOG ("glObjectLabel GLenum identifier=%d, GLuint name=%d, GLsizei length=%d, const GLchar* label=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glObjectPtrLabel(CPU* cpu) {
    if (!ext_glObjectPtrLabel)
        kpanic("ext_glObjectPtrLabel is NULL");
    {
    GL_FUNC(ext_glObjectPtrLabel)((void*)marshalp(cpu, 0, ARG1, 0), ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glObjectPtrLabel const void* ptr=%.08x, GLsizei length=%d, const GLchar* label=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glObjectPurgeableAPPLE(CPU* cpu) {
    if (!ext_glObjectPurgeableAPPLE)
        kpanic("ext_glObjectPurgeableAPPLE is NULL");
    {
    EAX=GL_FUNC(ext_glObjectPurgeableAPPLE)(ARG1, ARG2, ARG3);
    GL_LOG ("glObjectPurgeableAPPLE GLenum objectType=%d, GLuint name=%d, GLenum option=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glObjectUnpurgeableAPPLE(CPU* cpu) {
    if (!ext_glObjectUnpurgeableAPPLE)
        kpanic("ext_glObjectUnpurgeableAPPLE is NULL");
    {
    EAX=GL_FUNC(ext_glObjectUnpurgeableAPPLE)(ARG1, ARG2, ARG3);
    GL_LOG ("glObjectUnpurgeableAPPLE GLenum objectType=%d, GLuint name=%d, GLenum option=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glOrthofOES(CPU* cpu) {
    if (!ext_glOrthofOES)
        kpanic("ext_glOrthofOES is NULL");
    {
    GL_FUNC(ext_glOrthofOES)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glOrthofOES GLfloat l=%f, GLfloat r=%f, GLfloat b=%f, GLfloat t=%f, GLfloat n=%f, GLfloat f=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glOrthoxOES(CPU* cpu) {
    if (!ext_glOrthoxOES)
        kpanic("ext_glOrthoxOES is NULL");
    {
    GL_FUNC(ext_glOrthoxOES)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glOrthoxOES GLfixed l=%d, GLfixed r=%d, GLfixed b=%d, GLfixed t=%d, GLfixed n=%d, GLfixed f=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glPNTrianglesfATI(CPU* cpu) {
    if (!ext_glPNTrianglesfATI)
        kpanic("ext_glPNTrianglesfATI is NULL");
    {
    GL_FUNC(ext_glPNTrianglesfATI)(ARG1, fARG2);
    GL_LOG ("glPNTrianglesfATI GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glPNTrianglesiATI(CPU* cpu) {
    if (!ext_glPNTrianglesiATI)
        kpanic("ext_glPNTrianglesiATI is NULL");
    {
    GL_FUNC(ext_glPNTrianglesiATI)(ARG1, ARG2);
    GL_LOG ("glPNTrianglesiATI GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glPassTexCoordATI(CPU* cpu) {
    if (!ext_glPassTexCoordATI)
        kpanic("ext_glPassTexCoordATI is NULL");
    {
    GL_FUNC(ext_glPassTexCoordATI)(ARG1, ARG2, ARG3);
    GL_LOG ("glPassTexCoordATI GLuint dst=%d, GLuint coord=%d, GLenum swizzle=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPassThroughxOES(CPU* cpu) {
    if (!ext_glPassThroughxOES)
        kpanic("ext_glPassThroughxOES is NULL");
    {
    GL_FUNC(ext_glPassThroughxOES)(ARG1);
    GL_LOG ("glPassThroughxOES GLfixed token=%d",ARG1);
    }
}
void glcommon_glPatchParameterfv(CPU* cpu) {
    if (!ext_glPatchParameterfv)
        kpanic("ext_glPatchParameterfv is NULL");
    {
    GL_FUNC(ext_glPatchParameterfv)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glPatchParameterfv GLenum pname=%d, const GLfloat* values=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPatchParameteri(CPU* cpu) {
    if (!ext_glPatchParameteri)
        kpanic("ext_glPatchParameteri is NULL");
    {
    GL_FUNC(ext_glPatchParameteri)(ARG1, ARG2);
    GL_LOG ("glPatchParameteri GLenum pname=%d, GLint value=%d",ARG1,ARG2);
    }
}
void glcommon_glPathColorGenNV(CPU* cpu) {
    if (!ext_glPathColorGenNV)
        kpanic("ext_glPathColorGenNV is NULL");
    {
    GL_FUNC(ext_glPathColorGenNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glPathColorGenNV GLenum color=%d, GLenum genMode=%d, GLenum colorFormat=%d, const GLfloat* coeffs=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glPathCommandsNV(CPU* cpu) {
    if (!ext_glPathCommandsNV)
        kpanic("ext_glPathCommandsNV is NULL");
    {
    GL_FUNC(ext_glPathCommandsNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glPathCommandsNV GLuint path=%d, GLsizei numCommands=%d, const GLubyte* commands=%.08x, GLsizei numCoords=%d, GLenum coordType=%d, const void* coords=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glPathCoordsNV(CPU* cpu) {
    if (!ext_glPathCoordsNV)
        kpanic("ext_glPathCoordsNV is NULL");
    {
    GL_FUNC(ext_glPathCoordsNV)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glPathCoordsNV GLuint path=%d, GLsizei numCoords=%d, GLenum coordType=%d, const void* coords=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glPathCoverDepthFuncNV(CPU* cpu) {
    if (!ext_glPathCoverDepthFuncNV)
        kpanic("ext_glPathCoverDepthFuncNV is NULL");
    {
    GL_FUNC(ext_glPathCoverDepthFuncNV)(ARG1);
    GL_LOG ("glPathCoverDepthFuncNV GLenum func=%d",ARG1);
    }
}
void glcommon_glPathDashArrayNV(CPU* cpu) {
    if (!ext_glPathDashArrayNV)
        kpanic("ext_glPathDashArrayNV is NULL");
    {
    GL_FUNC(ext_glPathDashArrayNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPathDashArrayNV GLuint path=%d, GLsizei dashCount=%d, const GLfloat* dashArray=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPathFogGenNV(CPU* cpu) {
    if (!ext_glPathFogGenNV)
        kpanic("ext_glPathFogGenNV is NULL");
    {
    GL_FUNC(ext_glPathFogGenNV)(ARG1);
    GL_LOG ("glPathFogGenNV GLenum genMode=%d",ARG1);
    }
}
void glcommon_glPathGlyphIndexArrayNV(CPU* cpu) {
    if (!ext_glPathGlyphIndexArrayNV)
        kpanic("ext_glPathGlyphIndexArrayNV is NULL");
    {
    EAX=GL_FUNC(ext_glPathGlyphIndexArrayNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7, fARG8);
    GL_LOG ("glPathGlyphIndexArrayNV GLuint firstPathName=%d, GLenum fontTarget=%d, const void* fontName=%.08x, GLbitfield fontStyle=%d, GLuint firstGlyphIndex=%d, GLsizei numGlyphs=%d, GLuint pathParameterTemplate=%d, GLfloat emScale=%f",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,fARG8);
    }
}
void glcommon_glPathGlyphIndexRangeNV(CPU* cpu) {
    if (!ext_glPathGlyphIndexRangeNV)
        kpanic("ext_glPathGlyphIndexRangeNV is NULL");
    {
    EAX=GL_FUNC(ext_glPathGlyphIndexRangeNV)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0), ARG3, ARG4, fARG5, (GLuint*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glPathGlyphIndexRangeNV GLenum fontTarget=%d, const void* fontName=%.08x, GLbitfield fontStyle=%d, GLuint pathParameterTemplate=%d, GLfloat emScale=%f, GLuint baseAndCount[2]=%d",ARG1,ARG2,ARG3,ARG4,fARG5,ARG6);
    }
}
void glcommon_glPathGlyphRangeNV(CPU* cpu) {
    if (!ext_glPathGlyphRangeNV)
        kpanic("ext_glPathGlyphRangeNV is NULL");
    {
    GL_FUNC(ext_glPathGlyphRangeNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7, ARG8, fARG9);
    GL_LOG ("glPathGlyphRangeNV GLuint firstPathName=%d, GLenum fontTarget=%d, const void* fontName=%.08x, GLbitfield fontStyle=%d, GLuint firstGlyph=%d, GLsizei numGlyphs=%d, GLenum handleMissingGlyphs=%d, GLuint pathParameterTemplate=%d, GLfloat emScale=%f",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,fARG9);
    }
}
void glcommon_glPathGlyphsNV(CPU* cpu) {
    if (!ext_glPathGlyphsNV)
        kpanic("ext_glPathGlyphsNV is NULL");
    {
    GL_FUNC(ext_glPathGlyphsNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, (void*)marshalp(cpu, 0, ARG7, 0), ARG8, ARG9, fARG10);
    GL_LOG ("glPathGlyphsNV GLuint firstPathName=%d, GLenum fontTarget=%d, const void* fontName=%.08x, GLbitfield fontStyle=%d, GLsizei numGlyphs=%d, GLenum type=%d, const void* charcodes=%.08x, GLenum handleMissingGlyphs=%d, GLuint pathParameterTemplate=%d, GLfloat emScale=%f",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,fARG10);
    }
}
void glcommon_glPathMemoryGlyphIndexArrayNV(CPU* cpu) {
    if (!ext_glPathMemoryGlyphIndexArrayNV)
        kpanic("ext_glPathMemoryGlyphIndexArrayNV is NULL");
    {
    EAX=GL_FUNC(ext_glPathMemoryGlyphIndexArrayNV)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0), ARG5, ARG6, ARG7, ARG8, fARG9);
    GL_LOG ("glPathMemoryGlyphIndexArrayNV GLuint firstPathName=%d, GLenum fontTarget=%d, GLsizeiptr fontSize=%d, const void* fontData=%.08x, GLsizei faceIndex=%d, GLuint firstGlyphIndex=%d, GLsizei numGlyphs=%d, GLuint pathParameterTemplate=%d, GLfloat emScale=%f",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,fARG9);
    }
}
void glcommon_glPathParameterfNV(CPU* cpu) {
    if (!ext_glPathParameterfNV)
        kpanic("ext_glPathParameterfNV is NULL");
    {
    GL_FUNC(ext_glPathParameterfNV)(ARG1, ARG2, fARG3);
    GL_LOG ("glPathParameterfNV GLuint path=%d, GLenum pname=%d, GLfloat value=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glPathParameterfvNV(CPU* cpu) {
    if (!ext_glPathParameterfvNV)
        kpanic("ext_glPathParameterfvNV is NULL");
    {
    GL_FUNC(ext_glPathParameterfvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPathParameterfvNV GLuint path=%d, GLenum pname=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPathParameteriNV(CPU* cpu) {
    if (!ext_glPathParameteriNV)
        kpanic("ext_glPathParameteriNV is NULL");
    {
    GL_FUNC(ext_glPathParameteriNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glPathParameteriNV GLuint path=%d, GLenum pname=%d, GLint value=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPathParameterivNV(CPU* cpu) {
    if (!ext_glPathParameterivNV)
        kpanic("ext_glPathParameterivNV is NULL");
    {
    GL_FUNC(ext_glPathParameterivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPathParameterivNV GLuint path=%d, GLenum pname=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPathStencilDepthOffsetNV(CPU* cpu) {
    if (!ext_glPathStencilDepthOffsetNV)
        kpanic("ext_glPathStencilDepthOffsetNV is NULL");
    {
    GL_FUNC(ext_glPathStencilDepthOffsetNV)(fARG1, fARG2);
    GL_LOG ("glPathStencilDepthOffsetNV GLfloat factor=%f, GLfloat units=%f",fARG1,fARG2);
    }
}
void glcommon_glPathStencilFuncNV(CPU* cpu) {
    if (!ext_glPathStencilFuncNV)
        kpanic("ext_glPathStencilFuncNV is NULL");
    {
    GL_FUNC(ext_glPathStencilFuncNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glPathStencilFuncNV GLenum func=%d, GLint ref=%d, GLuint mask=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPathStringNV(CPU* cpu) {
    if (!ext_glPathStringNV)
        kpanic("ext_glPathStringNV is NULL");
    {
    GL_FUNC(ext_glPathStringNV)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glPathStringNV GLuint path=%d, GLenum format=%d, GLsizei length=%d, const void* pathString=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glPathSubCommandsNV(CPU* cpu) {
    if (!ext_glPathSubCommandsNV)
        kpanic("ext_glPathSubCommandsNV is NULL");
    {
    GL_FUNC(ext_glPathSubCommandsNV)(ARG1, ARG2, ARG3, ARG4, (GLubyte*)marshalp(cpu, 0, ARG5, 0), ARG6, ARG7, (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glPathSubCommandsNV GLuint path=%d, GLsizei commandStart=%d, GLsizei commandsToDelete=%d, GLsizei numCommands=%d, const GLubyte* commands=%.08x, GLsizei numCoords=%d, GLenum coordType=%d, const void* coords=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glPathSubCoordsNV(CPU* cpu) {
    if (!ext_glPathSubCoordsNV)
        kpanic("ext_glPathSubCoordsNV is NULL");
    {
    GL_FUNC(ext_glPathSubCoordsNV)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glPathSubCoordsNV GLuint path=%d, GLsizei coordStart=%d, GLsizei numCoords=%d, GLenum coordType=%d, const void* coords=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glPathTexGenNV(CPU* cpu) {
    if (!ext_glPathTexGenNV)
        kpanic("ext_glPathTexGenNV is NULL");
    {
    GL_FUNC(ext_glPathTexGenNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glPathTexGenNV GLenum texCoordSet=%d, GLenum genMode=%d, GLint components=%d, const GLfloat* coeffs=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glPauseTransformFeedback(CPU* cpu) {
    if (!ext_glPauseTransformFeedback)
        kpanic("ext_glPauseTransformFeedback is NULL");
    {
    GL_FUNC(ext_glPauseTransformFeedback)();
    GL_LOG ("glPauseTransformFeedback");
    }
}
void glcommon_glPauseTransformFeedbackNV(CPU* cpu) {
    if (!ext_glPauseTransformFeedbackNV)
        kpanic("ext_glPauseTransformFeedbackNV is NULL");
    {
    GL_FUNC(ext_glPauseTransformFeedbackNV)();
    GL_LOG ("glPauseTransformFeedbackNV");
    }
}
void glcommon_glPixelDataRangeNV(CPU* cpu) {
    if (!ext_glPixelDataRangeNV)
        kpanic("ext_glPixelDataRangeNV is NULL");
    {
    GL_FUNC(ext_glPixelDataRangeNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPixelDataRangeNV GLenum target=%d, GLsizei length=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPixelMapx(CPU* cpu) {
    if (!ext_glPixelMapx)
        kpanic("ext_glPixelMapx is NULL");
    {
    GL_FUNC(ext_glPixelMapx)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPixelMapx GLenum map=%d, GLint size=%d, const GLfixed* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPixelStorex(CPU* cpu) {
    if (!ext_glPixelStorex)
        kpanic("ext_glPixelStorex is NULL");
    {
    GL_FUNC(ext_glPixelStorex)(ARG1, ARG2);
    GL_LOG ("glPixelStorex GLenum pname=%d, GLfixed param=%d",ARG1,ARG2);
    }
}
void glcommon_glPixelTexGenParameterfSGIS(CPU* cpu) {
    if (!ext_glPixelTexGenParameterfSGIS)
        kpanic("ext_glPixelTexGenParameterfSGIS is NULL");
    {
    GL_FUNC(ext_glPixelTexGenParameterfSGIS)(ARG1, fARG2);
    GL_LOG ("glPixelTexGenParameterfSGIS GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glPixelTexGenParameterfvSGIS(CPU* cpu) {
    if (!ext_glPixelTexGenParameterfvSGIS)
        kpanic("ext_glPixelTexGenParameterfvSGIS is NULL");
    {
    GL_FUNC(ext_glPixelTexGenParameterfvSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glPixelTexGenParameterfvSGIS GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPixelTexGenParameteriSGIS(CPU* cpu) {
    if (!ext_glPixelTexGenParameteriSGIS)
        kpanic("ext_glPixelTexGenParameteriSGIS is NULL");
    {
    GL_FUNC(ext_glPixelTexGenParameteriSGIS)(ARG1, ARG2);
    GL_LOG ("glPixelTexGenParameteriSGIS GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glPixelTexGenParameterivSGIS(CPU* cpu) {
    if (!ext_glPixelTexGenParameterivSGIS)
        kpanic("ext_glPixelTexGenParameterivSGIS is NULL");
    {
    GL_FUNC(ext_glPixelTexGenParameterivSGIS)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glPixelTexGenParameterivSGIS GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPixelTexGenSGIX(CPU* cpu) {
    if (!ext_glPixelTexGenSGIX)
        kpanic("ext_glPixelTexGenSGIX is NULL");
    {
    GL_FUNC(ext_glPixelTexGenSGIX)(ARG1);
    GL_LOG ("glPixelTexGenSGIX GLenum mode=%d",ARG1);
    }
}
void glcommon_glPixelTransferxOES(CPU* cpu) {
    if (!ext_glPixelTransferxOES)
        kpanic("ext_glPixelTransferxOES is NULL");
    {
    GL_FUNC(ext_glPixelTransferxOES)(ARG1, ARG2);
    GL_LOG ("glPixelTransferxOES GLenum pname=%d, GLfixed param=%d",ARG1,ARG2);
    }
}
void glcommon_glPixelTransformParameterfEXT(CPU* cpu) {
    if (!ext_glPixelTransformParameterfEXT)
        kpanic("ext_glPixelTransformParameterfEXT is NULL");
    {
    GL_FUNC(ext_glPixelTransformParameterfEXT)(ARG1, ARG2, fARG3);
    GL_LOG ("glPixelTransformParameterfEXT GLenum target=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glPixelTransformParameterfvEXT(CPU* cpu) {
    if (!ext_glPixelTransformParameterfvEXT)
        kpanic("ext_glPixelTransformParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glPixelTransformParameterfvEXT)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPixelTransformParameterfvEXT GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPixelTransformParameteriEXT(CPU* cpu) {
    if (!ext_glPixelTransformParameteriEXT)
        kpanic("ext_glPixelTransformParameteriEXT is NULL");
    {
    GL_FUNC(ext_glPixelTransformParameteriEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glPixelTransformParameteriEXT GLenum target=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPixelTransformParameterivEXT(CPU* cpu) {
    if (!ext_glPixelTransformParameterivEXT)
        kpanic("ext_glPixelTransformParameterivEXT is NULL");
    {
    GL_FUNC(ext_glPixelTransformParameterivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPixelTransformParameterivEXT GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPixelZoomxOES(CPU* cpu) {
    if (!ext_glPixelZoomxOES)
        kpanic("ext_glPixelZoomxOES is NULL");
    {
    GL_FUNC(ext_glPixelZoomxOES)(ARG1, ARG2);
    GL_LOG ("glPixelZoomxOES GLfixed xfactor=%d, GLfixed yfactor=%d",ARG1,ARG2);
    }
}
void glcommon_glPointAlongPathNV(CPU* cpu) {
    if (!ext_glPointAlongPathNV)
        kpanic("ext_glPointAlongPathNV is NULL");
    {
    EAX=GL_FUNC(ext_glPointAlongPathNV)(ARG1, ARG2, ARG3, fARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0), (GLfloat*)marshalp(cpu, 0, ARG6, 0), (GLfloat*)marshalp(cpu, 0, ARG7, 0), (GLfloat*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glPointAlongPathNV GLuint path=%d, GLsizei startSegment=%d, GLsizei numSegments=%d, GLfloat distance=%f, GLfloat* x=%.08x, GLfloat* y=%.08x, GLfloat* tangentX=%.08x, GLfloat* tangentY=%.08x",ARG1,ARG2,ARG3,fARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glPointParameterf(CPU* cpu) {
    if (!ext_glPointParameterf)
        kpanic("ext_glPointParameterf is NULL");
    {
    GL_FUNC(ext_glPointParameterf)(ARG1, fARG2);
    GL_LOG ("glPointParameterf GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glPointParameterfARB(CPU* cpu) {
    if (!ext_glPointParameterfARB)
        kpanic("ext_glPointParameterfARB is NULL");
    {
    GL_FUNC(ext_glPointParameterfARB)(ARG1, fARG2);
    GL_LOG ("glPointParameterfARB GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glPointParameterfEXT(CPU* cpu) {
    if (!ext_glPointParameterfEXT)
        kpanic("ext_glPointParameterfEXT is NULL");
    {
    GL_FUNC(ext_glPointParameterfEXT)(ARG1, fARG2);
    GL_LOG ("glPointParameterfEXT GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glPointParameterfSGIS(CPU* cpu) {
    if (!ext_glPointParameterfSGIS)
        kpanic("ext_glPointParameterfSGIS is NULL");
    {
    GL_FUNC(ext_glPointParameterfSGIS)(ARG1, fARG2);
    GL_LOG ("glPointParameterfSGIS GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glPointParameterfv(CPU* cpu) {
    if (!ext_glPointParameterfv)
        kpanic("ext_glPointParameterfv is NULL");
    {
    GL_FUNC(ext_glPointParameterfv)(ARG1, marshalArray<GLfloat>(cpu, ARG2, ARG1==GL_POINT_DISTANCE_ATTENUATION?3:1));
    GL_LOG ("glPointParameterfv GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPointParameterfvARB(CPU* cpu) {
    if (!ext_glPointParameterfvARB)
        kpanic("ext_glPointParameterfvARB is NULL");
    {
    GL_FUNC(ext_glPointParameterfvARB)(ARG1, marshalArray<GLfloat>(cpu, ARG2, ARG1==GL_POINT_DISTANCE_ATTENUATION?3:1));
    GL_LOG ("glPointParameterfvARB GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPointParameterfvEXT(CPU* cpu) {
    if (!ext_glPointParameterfvEXT)
        kpanic("ext_glPointParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glPointParameterfvEXT)(ARG1, marshalArray<GLfloat>(cpu, ARG2, ARG1==GL_POINT_DISTANCE_ATTENUATION?3:1));
    GL_LOG ("glPointParameterfvEXT GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPointParameterfvSGIS(CPU* cpu) {
    if (!ext_glPointParameterfvSGIS)
        kpanic("ext_glPointParameterfvSGIS is NULL");
    {
    GL_FUNC(ext_glPointParameterfvSGIS)(ARG1, marshalArray<GLfloat>(cpu, ARG2, ARG1==GL_POINT_DISTANCE_ATTENUATION?3:1));
    GL_LOG ("glPointParameterfvSGIS GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPointParameteri(CPU* cpu) {
    if (!ext_glPointParameteri)
        kpanic("ext_glPointParameteri is NULL");
    {
    GL_FUNC(ext_glPointParameteri)(ARG1, ARG2);
    GL_LOG ("glPointParameteri GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glPointParameteriNV(CPU* cpu) {
    if (!ext_glPointParameteriNV)
        kpanic("ext_glPointParameteriNV is NULL");
    {
    GL_FUNC(ext_glPointParameteriNV)(ARG1, ARG2);
    GL_LOG ("glPointParameteriNV GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glPointParameteriv(CPU* cpu) {
    if (!ext_glPointParameteriv)
        kpanic("ext_glPointParameteriv is NULL");
    {
    GL_FUNC(ext_glPointParameteriv)(ARG1, marshalArray<GLint>(cpu, ARG2, ARG1==GL_POINT_DISTANCE_ATTENUATION?3:1));
    GL_LOG ("glPointParameteriv GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPointParameterivNV(CPU* cpu) {
    if (!ext_glPointParameterivNV)
        kpanic("ext_glPointParameterivNV is NULL");
    {
    GL_FUNC(ext_glPointParameterivNV)(ARG1, marshalArray<GLint>(cpu, ARG2, ARG1==GL_POINT_DISTANCE_ATTENUATION?3:1));
    GL_LOG ("glPointParameterivNV GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPointParameterxvOES(CPU* cpu) {
    if (!ext_glPointParameterxvOES)
        kpanic("ext_glPointParameterxvOES is NULL");
    {
    GL_FUNC(ext_glPointParameterxvOES)(ARG1, (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glPointParameterxvOES GLenum pname=%d, const GLfixed* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glPointSizexOES(CPU* cpu) {
    if (!ext_glPointSizexOES)
        kpanic("ext_glPointSizexOES is NULL");
    {
    GL_FUNC(ext_glPointSizexOES)(ARG1);
    GL_LOG ("glPointSizexOES GLfixed size=%d",ARG1);
    }
}
void glcommon_glPollAsyncSGIX(CPU* cpu) {
    if (!ext_glPollAsyncSGIX)
        kpanic("ext_glPollAsyncSGIX is NULL");
    {
    EAX=GL_FUNC(ext_glPollAsyncSGIX)((GLuint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glPollAsyncSGIX GLuint* markerp=%.08x",ARG1);
    }
}
void glcommon_glPollInstrumentsSGIX(CPU* cpu) {
    if (!ext_glPollInstrumentsSGIX)
        kpanic("ext_glPollInstrumentsSGIX is NULL");
    {
    EAX=GL_FUNC(ext_glPollInstrumentsSGIX)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glPollInstrumentsSGIX GLint* marker_p=%.08x",ARG1);
    }
}
void glcommon_glPolygonOffsetClampEXT(CPU* cpu) {
    if (!ext_glPolygonOffsetClampEXT)
        kpanic("ext_glPolygonOffsetClampEXT is NULL");
    {
    GL_FUNC(ext_glPolygonOffsetClampEXT)(fARG1, fARG2, fARG3);
    GL_LOG ("glPolygonOffsetClampEXT GLfloat factor=%f, GLfloat units=%f, GLfloat clamp=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glPolygonOffsetEXT(CPU* cpu) {
    if (!ext_glPolygonOffsetEXT)
        kpanic("ext_glPolygonOffsetEXT is NULL");
    {
    GL_FUNC(ext_glPolygonOffsetEXT)(fARG1, fARG2);
    GL_LOG ("glPolygonOffsetEXT GLfloat factor=%f, GLfloat bias=%f",fARG1,fARG2);
    }
}
void glcommon_glPolygonOffsetxOES(CPU* cpu) {
    if (!ext_glPolygonOffsetxOES)
        kpanic("ext_glPolygonOffsetxOES is NULL");
    {
    GL_FUNC(ext_glPolygonOffsetxOES)(ARG1, ARG2);
    GL_LOG ("glPolygonOffsetxOES GLfixed factor=%d, GLfixed units=%d",ARG1,ARG2);
    }
}
void glcommon_glPopDebugGroup(CPU* cpu) {
    if (!ext_glPopDebugGroup)
        kpanic("ext_glPopDebugGroup is NULL");
    {
    GL_FUNC(ext_glPopDebugGroup)();
    GL_LOG ("glPopDebugGroup");
    }
}
void glcommon_glPopGroupMarkerEXT(CPU* cpu) {
    if (!ext_glPopGroupMarkerEXT)
        kpanic("ext_glPopGroupMarkerEXT is NULL");
    {
    GL_FUNC(ext_glPopGroupMarkerEXT)();
    GL_LOG ("glPopGroupMarkerEXT");
    }
}
void glcommon_glPresentFrameDualFillNV(CPU* cpu) {
    if (!ext_glPresentFrameDualFillNV)
        kpanic("ext_glPresentFrameDualFillNV is NULL");
    {
    GL_FUNC(ext_glPresentFrameDualFillNV)(ARG1, llARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13);
    GL_LOG ("glPresentFrameDualFillNV GLuint video_slot=%d, GLuint64EXT minPresentTime=" PRIu64 ", GLuint beginPresentTimeId=%d, GLuint presentDurationId=%d, GLenum type=%d, GLenum target0=%d, GLuint fill0=%d, GLenum target1=%d, GLuint fill1=%d, GLenum target2=%d, GLuint fill2=%d, GLenum target3=%d, GLuint fill3=%d",ARG1,llARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12,ARG13);
    }
}
void glcommon_glPresentFrameKeyedNV(CPU* cpu) {
    if (!ext_glPresentFrameKeyedNV)
        kpanic("ext_glPresentFrameKeyedNV is NULL");
    {
    GL_FUNC(ext_glPresentFrameKeyedNV)(ARG1, llARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11);
    GL_LOG ("glPresentFrameKeyedNV GLuint video_slot=%d, GLuint64EXT minPresentTime=" PRIu64 ", GLuint beginPresentTimeId=%d, GLuint presentDurationId=%d, GLenum type=%d, GLenum target0=%d, GLuint fill0=%d, GLuint key0=%d, GLenum target1=%d, GLuint fill1=%d, GLuint key1=%d",ARG1,llARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glPrimitiveBoundingBoxARB(CPU* cpu) {
    if (!ext_glPrimitiveBoundingBoxARB)
        kpanic("ext_glPrimitiveBoundingBoxARB is NULL");
    {
    GL_FUNC(ext_glPrimitiveBoundingBoxARB)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8);
    GL_LOG ("glPrimitiveBoundingBoxARB GLfloat minX=%f, GLfloat minY=%f, GLfloat minZ=%f, GLfloat minW=%f, GLfloat maxX=%f, GLfloat maxY=%f, GLfloat maxZ=%f, GLfloat maxW=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8);
    }
}
void glcommon_glPrimitiveRestartIndex(CPU* cpu) {
    if (!ext_glPrimitiveRestartIndex)
        kpanic("ext_glPrimitiveRestartIndex is NULL");
    {
    GL_FUNC(ext_glPrimitiveRestartIndex)(ARG1);
    GL_LOG ("glPrimitiveRestartIndex GLuint index=%d",ARG1);
    }
}
void glcommon_glPrimitiveRestartIndexNV(CPU* cpu) {
    if (!ext_glPrimitiveRestartIndexNV)
        kpanic("ext_glPrimitiveRestartIndexNV is NULL");
    {
    GL_FUNC(ext_glPrimitiveRestartIndexNV)(ARG1);
    GL_LOG ("glPrimitiveRestartIndexNV GLuint index=%d",ARG1);
    }
}
void glcommon_glPrimitiveRestartNV(CPU* cpu) {
    if (!ext_glPrimitiveRestartNV)
        kpanic("ext_glPrimitiveRestartNV is NULL");
    {
    GL_FUNC(ext_glPrimitiveRestartNV)();
    GL_LOG ("glPrimitiveRestartNV");
    }
}
void glcommon_glPrioritizeTexturesEXT(CPU* cpu) {
    if (!ext_glPrioritizeTexturesEXT)
        kpanic("ext_glPrioritizeTexturesEXT is NULL");
    {
    GL_FUNC(ext_glPrioritizeTexturesEXT)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0), (GLclampf*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPrioritizeTexturesEXT GLsizei n=%d, const GLuint* textures=%.08x, const GLclampf* priorities=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glPrioritizeTexturesxOES(CPU* cpu) {
    if (!ext_glPrioritizeTexturesxOES)
        kpanic("ext_glPrioritizeTexturesxOES is NULL");
    {
    GL_FUNC(ext_glPrioritizeTexturesxOES)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0), (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glPrioritizeTexturesxOES GLsizei n=%d, const GLuint* textures=%.08x, const GLfixed* priorities=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramBinary(CPU* cpu) {
    if (!ext_glProgramBinary)
        kpanic("ext_glProgramBinary is NULL");
    {
    GL_FUNC(ext_glProgramBinary)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glProgramBinary GLuint program=%d, GLenum binaryFormat=%d, const void* binary=%.08x, GLsizei length=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramBufferParametersIivNV(CPU* cpu) {
    if (!ext_glProgramBufferParametersIivNV)
        kpanic("ext_glProgramBufferParametersIivNV is NULL");
    {
    GL_FUNC(ext_glProgramBufferParametersIivNV)(ARG1, ARG2, ARG3, ARG4, (GLint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramBufferParametersIivNV GLenum target=%d, GLuint bindingIndex=%d, GLuint wordIndex=%d, GLsizei count=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramBufferParametersIuivNV(CPU* cpu) {
    if (!ext_glProgramBufferParametersIuivNV)
        kpanic("ext_glProgramBufferParametersIuivNV is NULL");
    {
    GL_FUNC(ext_glProgramBufferParametersIuivNV)(ARG1, ARG2, ARG3, ARG4, (GLuint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramBufferParametersIuivNV GLenum target=%d, GLuint bindingIndex=%d, GLuint wordIndex=%d, GLsizei count=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramBufferParametersfvNV(CPU* cpu) {
    if (!ext_glProgramBufferParametersfvNV)
        kpanic("ext_glProgramBufferParametersfvNV is NULL");
    {
    GL_FUNC(ext_glProgramBufferParametersfvNV)(ARG1, ARG2, ARG3, ARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramBufferParametersfvNV GLenum target=%d, GLuint bindingIndex=%d, GLuint wordIndex=%d, GLsizei count=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramEnvParameter4dARB(CPU* cpu) {
    if (!ext_glProgramEnvParameter4dARB)
        kpanic("ext_glProgramEnvParameter4dARB is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameter4dARB)(ARG1, ARG2, dARG3, dARG4, dARG5, dARG6);
    GL_LOG ("glProgramEnvParameter4dARB GLenum target=%d, GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,ARG2,dARG3,dARG4,dARG5,dARG6);
    }
}
void glcommon_glProgramEnvParameter4dvARB(CPU* cpu) {
    if (!ext_glProgramEnvParameter4dvARB)
        kpanic("ext_glProgramEnvParameter4dvARB is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameter4dvARB)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramEnvParameter4dvARB GLenum target=%d, GLuint index=%d, const GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramEnvParameter4fARB(CPU* cpu) {
    if (!ext_glProgramEnvParameter4fARB)
        kpanic("ext_glProgramEnvParameter4fARB is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameter4fARB)(ARG1, ARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glProgramEnvParameter4fARB GLenum target=%d, GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,ARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glProgramEnvParameter4fvARB(CPU* cpu) {
    if (!ext_glProgramEnvParameter4fvARB)
        kpanic("ext_glProgramEnvParameter4fvARB is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameter4fvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramEnvParameter4fvARB GLenum target=%d, GLuint index=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramEnvParameterI4iNV(CPU* cpu) {
    if (!ext_glProgramEnvParameterI4iNV)
        kpanic("ext_glProgramEnvParameterI4iNV is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameterI4iNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramEnvParameterI4iNV GLenum target=%d, GLuint index=%d, GLint x=%d, GLint y=%d, GLint z=%d, GLint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramEnvParameterI4ivNV(CPU* cpu) {
    if (!ext_glProgramEnvParameterI4ivNV)
        kpanic("ext_glProgramEnvParameterI4ivNV is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameterI4ivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramEnvParameterI4ivNV GLenum target=%d, GLuint index=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramEnvParameterI4uiNV(CPU* cpu) {
    if (!ext_glProgramEnvParameterI4uiNV)
        kpanic("ext_glProgramEnvParameterI4uiNV is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameterI4uiNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramEnvParameterI4uiNV GLenum target=%d, GLuint index=%d, GLuint x=%d, GLuint y=%d, GLuint z=%d, GLuint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramEnvParameterI4uivNV(CPU* cpu) {
    if (!ext_glProgramEnvParameterI4uivNV)
        kpanic("ext_glProgramEnvParameterI4uivNV is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameterI4uivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramEnvParameterI4uivNV GLenum target=%d, GLuint index=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramEnvParameters4fvEXT(CPU* cpu) {
    if (!ext_glProgramEnvParameters4fvEXT)
        kpanic("ext_glProgramEnvParameters4fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramEnvParameters4fvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramEnvParameters4fvEXT GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramEnvParametersI4ivNV(CPU* cpu) {
    if (!ext_glProgramEnvParametersI4ivNV)
        kpanic("ext_glProgramEnvParametersI4ivNV is NULL");
    {
    GL_FUNC(ext_glProgramEnvParametersI4ivNV)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramEnvParametersI4ivNV GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramEnvParametersI4uivNV(CPU* cpu) {
    if (!ext_glProgramEnvParametersI4uivNV)
        kpanic("ext_glProgramEnvParametersI4uivNV is NULL");
    {
    GL_FUNC(ext_glProgramEnvParametersI4uivNV)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramEnvParametersI4uivNV GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramLocalParameter4dARB(CPU* cpu) {
    if (!ext_glProgramLocalParameter4dARB)
        kpanic("ext_glProgramLocalParameter4dARB is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameter4dARB)(ARG1, ARG2, dARG3, dARG4, dARG5, dARG6);
    GL_LOG ("glProgramLocalParameter4dARB GLenum target=%d, GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,ARG2,dARG3,dARG4,dARG5,dARG6);
    }
}
void glcommon_glProgramLocalParameter4dvARB(CPU* cpu) {
    if (!ext_glProgramLocalParameter4dvARB)
        kpanic("ext_glProgramLocalParameter4dvARB is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameter4dvARB)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramLocalParameter4dvARB GLenum target=%d, GLuint index=%d, const GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramLocalParameter4fARB(CPU* cpu) {
    if (!ext_glProgramLocalParameter4fARB)
        kpanic("ext_glProgramLocalParameter4fARB is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameter4fARB)(ARG1, ARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glProgramLocalParameter4fARB GLenum target=%d, GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,ARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glProgramLocalParameter4fvARB(CPU* cpu) {
    if (!ext_glProgramLocalParameter4fvARB)
        kpanic("ext_glProgramLocalParameter4fvARB is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameter4fvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramLocalParameter4fvARB GLenum target=%d, GLuint index=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramLocalParameterI4iNV(CPU* cpu) {
    if (!ext_glProgramLocalParameterI4iNV)
        kpanic("ext_glProgramLocalParameterI4iNV is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameterI4iNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramLocalParameterI4iNV GLenum target=%d, GLuint index=%d, GLint x=%d, GLint y=%d, GLint z=%d, GLint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramLocalParameterI4ivNV(CPU* cpu) {
    if (!ext_glProgramLocalParameterI4ivNV)
        kpanic("ext_glProgramLocalParameterI4ivNV is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameterI4ivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramLocalParameterI4ivNV GLenum target=%d, GLuint index=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramLocalParameterI4uiNV(CPU* cpu) {
    if (!ext_glProgramLocalParameterI4uiNV)
        kpanic("ext_glProgramLocalParameterI4uiNV is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameterI4uiNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramLocalParameterI4uiNV GLenum target=%d, GLuint index=%d, GLuint x=%d, GLuint y=%d, GLuint z=%d, GLuint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramLocalParameterI4uivNV(CPU* cpu) {
    if (!ext_glProgramLocalParameterI4uivNV)
        kpanic("ext_glProgramLocalParameterI4uivNV is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameterI4uivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramLocalParameterI4uivNV GLenum target=%d, GLuint index=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramLocalParameters4fvEXT(CPU* cpu) {
    if (!ext_glProgramLocalParameters4fvEXT)
        kpanic("ext_glProgramLocalParameters4fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramLocalParameters4fvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramLocalParameters4fvEXT GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramLocalParametersI4ivNV(CPU* cpu) {
    if (!ext_glProgramLocalParametersI4ivNV)
        kpanic("ext_glProgramLocalParametersI4ivNV is NULL");
    {
    GL_FUNC(ext_glProgramLocalParametersI4ivNV)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramLocalParametersI4ivNV GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramLocalParametersI4uivNV(CPU* cpu) {
    if (!ext_glProgramLocalParametersI4uivNV)
        kpanic("ext_glProgramLocalParametersI4uivNV is NULL");
    {
    GL_FUNC(ext_glProgramLocalParametersI4uivNV)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramLocalParametersI4uivNV GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramNamedParameter4dNV(CPU* cpu) {
    if (!ext_glProgramNamedParameter4dNV)
        kpanic("ext_glProgramNamedParameter4dNV is NULL");
    {
    GL_FUNC(ext_glProgramNamedParameter4dNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0), dARG4, dARG5, dARG6, dARG7);
    GL_LOG ("glProgramNamedParameter4dNV GLuint id=%d, GLsizei len=%d, const GLubyte* name=%.08x, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,ARG2,ARG3,dARG4,dARG5,dARG6,dARG7);
    }
}
void glcommon_glProgramNamedParameter4dvNV(CPU* cpu) {
    if (!ext_glProgramNamedParameter4dvNV)
        kpanic("ext_glProgramNamedParameter4dvNV is NULL");
    {
    GL_FUNC(ext_glProgramNamedParameter4dvNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0), (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramNamedParameter4dvNV GLuint id=%d, GLsizei len=%d, const GLubyte* name=%.08x, const GLdouble* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramNamedParameter4fNV(CPU* cpu) {
    if (!ext_glProgramNamedParameter4fNV)
        kpanic("ext_glProgramNamedParameter4fNV is NULL");
    {
    GL_FUNC(ext_glProgramNamedParameter4fNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0), fARG4, fARG5, fARG6, fARG7);
    GL_LOG ("glProgramNamedParameter4fNV GLuint id=%d, GLsizei len=%d, const GLubyte* name=%.08x, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,ARG2,ARG3,fARG4,fARG5,fARG6,fARG7);
    }
}
void glcommon_glProgramNamedParameter4fvNV(CPU* cpu) {
    if (!ext_glProgramNamedParameter4fvNV)
        kpanic("ext_glProgramNamedParameter4fvNV is NULL");
    {
    GL_FUNC(ext_glProgramNamedParameter4fvNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramNamedParameter4fvNV GLuint id=%d, GLsizei len=%d, const GLubyte* name=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramParameter4dNV(CPU* cpu) {
    if (!ext_glProgramParameter4dNV)
        kpanic("ext_glProgramParameter4dNV is NULL");
    {
    GL_FUNC(ext_glProgramParameter4dNV)(ARG1, ARG2, dARG3, dARG4, dARG5, dARG6);
    GL_LOG ("glProgramParameter4dNV GLenum target=%d, GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,ARG2,dARG3,dARG4,dARG5,dARG6);
    }
}
void glcommon_glProgramParameter4dvNV(CPU* cpu) {
    if (!ext_glProgramParameter4dvNV)
        kpanic("ext_glProgramParameter4dvNV is NULL");
    {
    GL_FUNC(ext_glProgramParameter4dvNV)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramParameter4dvNV GLenum target=%d, GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramParameter4fNV(CPU* cpu) {
    if (!ext_glProgramParameter4fNV)
        kpanic("ext_glProgramParameter4fNV is NULL");
    {
    GL_FUNC(ext_glProgramParameter4fNV)(ARG1, ARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glProgramParameter4fNV GLenum target=%d, GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,ARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glProgramParameter4fvNV(CPU* cpu) {
    if (!ext_glProgramParameter4fvNV)
        kpanic("ext_glProgramParameter4fvNV is NULL");
    {
    GL_FUNC(ext_glProgramParameter4fvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramParameter4fvNV GLenum target=%d, GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramParameteri(CPU* cpu) {
    if (!ext_glProgramParameteri)
        kpanic("ext_glProgramParameteri is NULL");
    {
    GL_FUNC(ext_glProgramParameteri)(ARG1, ARG2, ARG3);
    GL_LOG ("glProgramParameteri GLuint program=%d, GLenum pname=%d, GLint value=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramParameteriARB(CPU* cpu) {
    if (!ext_glProgramParameteriARB)
        kpanic("ext_glProgramParameteriARB is NULL");
    {
    GL_FUNC(ext_glProgramParameteriARB)(ARG1, ARG2, ARG3);
    GL_LOG ("glProgramParameteriARB GLuint program=%d, GLenum pname=%d, GLint value=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramParameteriEXT(CPU* cpu) {
    if (!ext_glProgramParameteriEXT)
        kpanic("ext_glProgramParameteriEXT is NULL");
    {
    GL_FUNC(ext_glProgramParameteriEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glProgramParameteriEXT GLuint program=%d, GLenum pname=%d, GLint value=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramParameters4dvNV(CPU* cpu) {
    if (!ext_glProgramParameters4dvNV)
        kpanic("ext_glProgramParameters4dvNV is NULL");
    {
    GL_FUNC(ext_glProgramParameters4dvNV)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramParameters4dvNV GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLdouble* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramParameters4fvNV(CPU* cpu) {
    if (!ext_glProgramParameters4fvNV)
        kpanic("ext_glProgramParameters4fvNV is NULL");
    {
    GL_FUNC(ext_glProgramParameters4fvNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramParameters4fvNV GLenum target=%d, GLuint index=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramPathFragmentInputGenNV(CPU* cpu) {
    if (!ext_glProgramPathFragmentInputGenNV)
        kpanic("ext_glProgramPathFragmentInputGenNV is NULL");
    {
    GL_FUNC(ext_glProgramPathFragmentInputGenNV)(ARG1, ARG2, ARG3, ARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramPathFragmentInputGenNV GLuint program=%d, GLint location=%d, GLenum genMode=%d, GLint components=%d, const GLfloat* coeffs=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramStringARB(CPU* cpu) {
    if (!ext_glProgramStringARB)
        kpanic("ext_glProgramStringARB is NULL");
    {
    GL_FUNC(ext_glProgramStringARB)(ARG1, ARG2, ARG3, marshalArray<GLubyte>(cpu, ARG4, ARG3));
    GL_LOG ("glProgramStringARB GLenum target=%d, GLenum format=%d, GLsizei len=%d, const void* string=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramSubroutineParametersuivNV(CPU* cpu) {
    if (!ext_glProgramSubroutineParametersuivNV)
        kpanic("ext_glProgramSubroutineParametersuivNV is NULL");
    {
    GL_FUNC(ext_glProgramSubroutineParametersuivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glProgramSubroutineParametersuivNV GLenum target=%d, GLsizei count=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramUniform1d(CPU* cpu) {
    if (!ext_glProgramUniform1d)
        kpanic("ext_glProgramUniform1d is NULL");
    {
    GL_FUNC(ext_glProgramUniform1d)(ARG1, ARG2, dARG3);
    GL_LOG ("glProgramUniform1d GLuint program=%d, GLint location=%d, GLdouble v0=%f",ARG1,ARG2,dARG3);
    }
}
void glcommon_glProgramUniform1dEXT(CPU* cpu) {
    if (!ext_glProgramUniform1dEXT)
        kpanic("ext_glProgramUniform1dEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1dEXT)(ARG1, ARG2, dARG3);
    GL_LOG ("glProgramUniform1dEXT GLuint program=%d, GLint location=%d, GLdouble x=%f",ARG1,ARG2,dARG3);
    }
}
void glcommon_glProgramUniform1dv(CPU* cpu) {
    if (!ext_glProgramUniform1dv)
        kpanic("ext_glProgramUniform1dv is NULL");
    {
    GL_FUNC(ext_glProgramUniform1dv)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1dv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1dvEXT(CPU* cpu) {
    if (!ext_glProgramUniform1dvEXT)
        kpanic("ext_glProgramUniform1dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1dvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1f(CPU* cpu) {
    if (!ext_glProgramUniform1f)
        kpanic("ext_glProgramUniform1f is NULL");
    {
    GL_FUNC(ext_glProgramUniform1f)(ARG1, ARG2, fARG3);
    GL_LOG ("glProgramUniform1f GLuint program=%d, GLint location=%d, GLfloat v0=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glProgramUniform1fEXT(CPU* cpu) {
    if (!ext_glProgramUniform1fEXT)
        kpanic("ext_glProgramUniform1fEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1fEXT)(ARG1, ARG2, fARG3);
    GL_LOG ("glProgramUniform1fEXT GLuint program=%d, GLint location=%d, GLfloat v0=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glProgramUniform1fv(CPU* cpu) {
    if (!ext_glProgramUniform1fv)
        kpanic("ext_glProgramUniform1fv is NULL");
    {
    GL_FUNC(ext_glProgramUniform1fv)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1fv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1fvEXT(CPU* cpu) {
    if (!ext_glProgramUniform1fvEXT)
        kpanic("ext_glProgramUniform1fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1fvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1i(CPU* cpu) {
    if (!ext_glProgramUniform1i)
        kpanic("ext_glProgramUniform1i is NULL");
    {
    GL_FUNC(ext_glProgramUniform1i)(ARG1, ARG2, ARG3);
    GL_LOG ("glProgramUniform1i GLuint program=%d, GLint location=%d, GLint v0=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramUniform1i64ARB(CPU* cpu) {
    if (!ext_glProgramUniform1i64ARB)
        kpanic("ext_glProgramUniform1i64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform1i64ARB)(ARG1, ARG2, llARG3);
    GL_LOG ("glProgramUniform1i64ARB GLuint program=%d, GLint location=%d, GLint64 x=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glProgramUniform1i64NV(CPU* cpu) {
    if (!ext_glProgramUniform1i64NV)
        kpanic("ext_glProgramUniform1i64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform1i64NV)(ARG1, ARG2, llARG3);
    GL_LOG ("glProgramUniform1i64NV GLuint program=%d, GLint location=%d, GLint64EXT x=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glProgramUniform1i64vARB(CPU* cpu) {
    if (!ext_glProgramUniform1i64vARB)
        kpanic("ext_glProgramUniform1i64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform1i64vARB)(ARG1, ARG2, ARG3, (GLint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1i64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1i64vNV(CPU* cpu) {
    if (!ext_glProgramUniform1i64vNV)
        kpanic("ext_glProgramUniform1i64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform1i64vNV)(ARG1, ARG2, ARG3, (GLint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1i64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1iEXT(CPU* cpu) {
    if (!ext_glProgramUniform1iEXT)
        kpanic("ext_glProgramUniform1iEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1iEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glProgramUniform1iEXT GLuint program=%d, GLint location=%d, GLint v0=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramUniform1iv(CPU* cpu) {
    if (!ext_glProgramUniform1iv)
        kpanic("ext_glProgramUniform1iv is NULL");
    {
    GL_FUNC(ext_glProgramUniform1iv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1iv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1ivEXT(CPU* cpu) {
    if (!ext_glProgramUniform1ivEXT)
        kpanic("ext_glProgramUniform1ivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1ivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1ivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1ui(CPU* cpu) {
    if (!ext_glProgramUniform1ui)
        kpanic("ext_glProgramUniform1ui is NULL");
    {
    GL_FUNC(ext_glProgramUniform1ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glProgramUniform1ui GLuint program=%d, GLint location=%d, GLuint v0=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramUniform1ui64ARB(CPU* cpu) {
    if (!ext_glProgramUniform1ui64ARB)
        kpanic("ext_glProgramUniform1ui64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform1ui64ARB)(ARG1, ARG2, llARG3);
    GL_LOG ("glProgramUniform1ui64ARB GLuint program=%d, GLint location=%d, GLuint64 x=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glProgramUniform1ui64NV(CPU* cpu) {
    if (!ext_glProgramUniform1ui64NV)
        kpanic("ext_glProgramUniform1ui64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform1ui64NV)(ARG1, ARG2, llARG3);
    GL_LOG ("glProgramUniform1ui64NV GLuint program=%d, GLint location=%d, GLuint64EXT x=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glProgramUniform1ui64vARB(CPU* cpu) {
    if (!ext_glProgramUniform1ui64vARB)
        kpanic("ext_glProgramUniform1ui64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform1ui64vARB)(ARG1, ARG2, ARG3, (GLuint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1ui64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1ui64vNV(CPU* cpu) {
    if (!ext_glProgramUniform1ui64vNV)
        kpanic("ext_glProgramUniform1ui64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform1ui64vNV)(ARG1, ARG2, ARG3, (GLuint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1ui64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1uiEXT(CPU* cpu) {
    if (!ext_glProgramUniform1uiEXT)
        kpanic("ext_glProgramUniform1uiEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1uiEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glProgramUniform1uiEXT GLuint program=%d, GLint location=%d, GLuint v0=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glProgramUniform1uiv(CPU* cpu) {
    if (!ext_glProgramUniform1uiv)
        kpanic("ext_glProgramUniform1uiv is NULL");
    {
    GL_FUNC(ext_glProgramUniform1uiv)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1uiv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform1uivEXT(CPU* cpu) {
    if (!ext_glProgramUniform1uivEXT)
        kpanic("ext_glProgramUniform1uivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform1uivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform1uivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2d(CPU* cpu) {
    if (!ext_glProgramUniform2d)
        kpanic("ext_glProgramUniform2d is NULL");
    {
    GL_FUNC(ext_glProgramUniform2d)(ARG1, ARG2, dARG3, dARG4);
    GL_LOG ("glProgramUniform2d GLuint program=%d, GLint location=%d, GLdouble v0=%f, GLdouble v1=%f",ARG1,ARG2,dARG3,dARG4);
    }
}
void glcommon_glProgramUniform2dEXT(CPU* cpu) {
    if (!ext_glProgramUniform2dEXT)
        kpanic("ext_glProgramUniform2dEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2dEXT)(ARG1, ARG2, dARG3, dARG4);
    GL_LOG ("glProgramUniform2dEXT GLuint program=%d, GLint location=%d, GLdouble x=%f, GLdouble y=%f",ARG1,ARG2,dARG3,dARG4);
    }
}
void glcommon_glProgramUniform2dv(CPU* cpu) {
    if (!ext_glProgramUniform2dv)
        kpanic("ext_glProgramUniform2dv is NULL");
    {
    GL_FUNC(ext_glProgramUniform2dv)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2dv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2dvEXT(CPU* cpu) {
    if (!ext_glProgramUniform2dvEXT)
        kpanic("ext_glProgramUniform2dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2dvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2f(CPU* cpu) {
    if (!ext_glProgramUniform2f)
        kpanic("ext_glProgramUniform2f is NULL");
    {
    GL_FUNC(ext_glProgramUniform2f)(ARG1, ARG2, fARG3, fARG4);
    GL_LOG ("glProgramUniform2f GLuint program=%d, GLint location=%d, GLfloat v0=%f, GLfloat v1=%f",ARG1,ARG2,fARG3,fARG4);
    }
}
void glcommon_glProgramUniform2fEXT(CPU* cpu) {
    if (!ext_glProgramUniform2fEXT)
        kpanic("ext_glProgramUniform2fEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2fEXT)(ARG1, ARG2, fARG3, fARG4);
    GL_LOG ("glProgramUniform2fEXT GLuint program=%d, GLint location=%d, GLfloat v0=%f, GLfloat v1=%f",ARG1,ARG2,fARG3,fARG4);
    }
}
void glcommon_glProgramUniform2fv(CPU* cpu) {
    if (!ext_glProgramUniform2fv)
        kpanic("ext_glProgramUniform2fv is NULL");
    {
    GL_FUNC(ext_glProgramUniform2fv)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2fv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2fvEXT(CPU* cpu) {
    if (!ext_glProgramUniform2fvEXT)
        kpanic("ext_glProgramUniform2fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2fvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2i(CPU* cpu) {
    if (!ext_glProgramUniform2i)
        kpanic("ext_glProgramUniform2i is NULL");
    {
    GL_FUNC(ext_glProgramUniform2i)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glProgramUniform2i GLuint program=%d, GLint location=%d, GLint v0=%d, GLint v1=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2i64ARB(CPU* cpu) {
    if (!ext_glProgramUniform2i64ARB)
        kpanic("ext_glProgramUniform2i64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform2i64ARB)(ARG1, ARG2, llARG3, llARG4);
    GL_LOG ("glProgramUniform2i64ARB GLuint program=%d, GLint location=%d, GLint64 x=" PRIu64 ", GLint64 y=" PRIu64 "",ARG1,ARG2,llARG3,llARG4);
    }
}
void glcommon_glProgramUniform2i64NV(CPU* cpu) {
    if (!ext_glProgramUniform2i64NV)
        kpanic("ext_glProgramUniform2i64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform2i64NV)(ARG1, ARG2, llARG3, llARG4);
    GL_LOG ("glProgramUniform2i64NV GLuint program=%d, GLint location=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 "",ARG1,ARG2,llARG3,llARG4);
    }
}
void glcommon_glProgramUniform2i64vARB(CPU* cpu) {
    if (!ext_glProgramUniform2i64vARB)
        kpanic("ext_glProgramUniform2i64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform2i64vARB)(ARG1, ARG2, ARG3, (GLint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2i64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2i64vNV(CPU* cpu) {
    if (!ext_glProgramUniform2i64vNV)
        kpanic("ext_glProgramUniform2i64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform2i64vNV)(ARG1, ARG2, ARG3, (GLint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2i64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2iEXT(CPU* cpu) {
    if (!ext_glProgramUniform2iEXT)
        kpanic("ext_glProgramUniform2iEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2iEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glProgramUniform2iEXT GLuint program=%d, GLint location=%d, GLint v0=%d, GLint v1=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2iv(CPU* cpu) {
    if (!ext_glProgramUniform2iv)
        kpanic("ext_glProgramUniform2iv is NULL");
    {
    GL_FUNC(ext_glProgramUniform2iv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2iv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2ivEXT(CPU* cpu) {
    if (!ext_glProgramUniform2ivEXT)
        kpanic("ext_glProgramUniform2ivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2ivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2ivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2ui(CPU* cpu) {
    if (!ext_glProgramUniform2ui)
        kpanic("ext_glProgramUniform2ui is NULL");
    {
    GL_FUNC(ext_glProgramUniform2ui)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glProgramUniform2ui GLuint program=%d, GLint location=%d, GLuint v0=%d, GLuint v1=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2ui64ARB(CPU* cpu) {
    if (!ext_glProgramUniform2ui64ARB)
        kpanic("ext_glProgramUniform2ui64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform2ui64ARB)(ARG1, ARG2, llARG3, llARG4);
    GL_LOG ("glProgramUniform2ui64ARB GLuint program=%d, GLint location=%d, GLuint64 x=" PRIu64 ", GLuint64 y=" PRIu64 "",ARG1,ARG2,llARG3,llARG4);
    }
}
void glcommon_glProgramUniform2ui64NV(CPU* cpu) {
    if (!ext_glProgramUniform2ui64NV)
        kpanic("ext_glProgramUniform2ui64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform2ui64NV)(ARG1, ARG2, llARG3, llARG4);
    GL_LOG ("glProgramUniform2ui64NV GLuint program=%d, GLint location=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 "",ARG1,ARG2,llARG3,llARG4);
    }
}
void glcommon_glProgramUniform2ui64vARB(CPU* cpu) {
    if (!ext_glProgramUniform2ui64vARB)
        kpanic("ext_glProgramUniform2ui64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform2ui64vARB)(ARG1, ARG2, ARG3, (GLuint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2ui64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2ui64vNV(CPU* cpu) {
    if (!ext_glProgramUniform2ui64vNV)
        kpanic("ext_glProgramUniform2ui64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform2ui64vNV)(ARG1, ARG2, ARG3, (GLuint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2ui64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2uiEXT(CPU* cpu) {
    if (!ext_glProgramUniform2uiEXT)
        kpanic("ext_glProgramUniform2uiEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2uiEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glProgramUniform2uiEXT GLuint program=%d, GLint location=%d, GLuint v0=%d, GLuint v1=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2uiv(CPU* cpu) {
    if (!ext_glProgramUniform2uiv)
        kpanic("ext_glProgramUniform2uiv is NULL");
    {
    GL_FUNC(ext_glProgramUniform2uiv)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2uiv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform2uivEXT(CPU* cpu) {
    if (!ext_glProgramUniform2uivEXT)
        kpanic("ext_glProgramUniform2uivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform2uivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform2uivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3d(CPU* cpu) {
    if (!ext_glProgramUniform3d)
        kpanic("ext_glProgramUniform3d is NULL");
    {
    GL_FUNC(ext_glProgramUniform3d)(ARG1, ARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glProgramUniform3d GLuint program=%d, GLint location=%d, GLdouble v0=%f, GLdouble v1=%f, GLdouble v2=%f",ARG1,ARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glProgramUniform3dEXT(CPU* cpu) {
    if (!ext_glProgramUniform3dEXT)
        kpanic("ext_glProgramUniform3dEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3dEXT)(ARG1, ARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glProgramUniform3dEXT GLuint program=%d, GLint location=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,ARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glProgramUniform3dv(CPU* cpu) {
    if (!ext_glProgramUniform3dv)
        kpanic("ext_glProgramUniform3dv is NULL");
    {
    GL_FUNC(ext_glProgramUniform3dv)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3dv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3dvEXT(CPU* cpu) {
    if (!ext_glProgramUniform3dvEXT)
        kpanic("ext_glProgramUniform3dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3dvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3f(CPU* cpu) {
    if (!ext_glProgramUniform3f)
        kpanic("ext_glProgramUniform3f is NULL");
    {
    GL_FUNC(ext_glProgramUniform3f)(ARG1, ARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glProgramUniform3f GLuint program=%d, GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f",ARG1,ARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glProgramUniform3fEXT(CPU* cpu) {
    if (!ext_glProgramUniform3fEXT)
        kpanic("ext_glProgramUniform3fEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3fEXT)(ARG1, ARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glProgramUniform3fEXT GLuint program=%d, GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f",ARG1,ARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glProgramUniform3fv(CPU* cpu) {
    if (!ext_glProgramUniform3fv)
        kpanic("ext_glProgramUniform3fv is NULL");
    {
    GL_FUNC(ext_glProgramUniform3fv)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3fv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3fvEXT(CPU* cpu) {
    if (!ext_glProgramUniform3fvEXT)
        kpanic("ext_glProgramUniform3fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3fvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3i(CPU* cpu) {
    if (!ext_glProgramUniform3i)
        kpanic("ext_glProgramUniform3i is NULL");
    {
    GL_FUNC(ext_glProgramUniform3i)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glProgramUniform3i GLuint program=%d, GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramUniform3i64ARB(CPU* cpu) {
    if (!ext_glProgramUniform3i64ARB)
        kpanic("ext_glProgramUniform3i64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform3i64ARB)(ARG1, ARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glProgramUniform3i64ARB GLuint program=%d, GLint location=%d, GLint64 x=" PRIu64 ", GLint64 y=" PRIu64 ", GLint64 z=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glProgramUniform3i64NV(CPU* cpu) {
    if (!ext_glProgramUniform3i64NV)
        kpanic("ext_glProgramUniform3i64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform3i64NV)(ARG1, ARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glProgramUniform3i64NV GLuint program=%d, GLint location=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 ", GLint64EXT z=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glProgramUniform3i64vARB(CPU* cpu) {
    if (!ext_glProgramUniform3i64vARB)
        kpanic("ext_glProgramUniform3i64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform3i64vARB)(ARG1, ARG2, ARG3, (GLint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3i64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3i64vNV(CPU* cpu) {
    if (!ext_glProgramUniform3i64vNV)
        kpanic("ext_glProgramUniform3i64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform3i64vNV)(ARG1, ARG2, ARG3, (GLint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3i64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3iEXT(CPU* cpu) {
    if (!ext_glProgramUniform3iEXT)
        kpanic("ext_glProgramUniform3iEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3iEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glProgramUniform3iEXT GLuint program=%d, GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramUniform3iv(CPU* cpu) {
    if (!ext_glProgramUniform3iv)
        kpanic("ext_glProgramUniform3iv is NULL");
    {
    GL_FUNC(ext_glProgramUniform3iv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3iv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3ivEXT(CPU* cpu) {
    if (!ext_glProgramUniform3ivEXT)
        kpanic("ext_glProgramUniform3ivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3ivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3ivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3ui(CPU* cpu) {
    if (!ext_glProgramUniform3ui)
        kpanic("ext_glProgramUniform3ui is NULL");
    {
    GL_FUNC(ext_glProgramUniform3ui)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glProgramUniform3ui GLuint program=%d, GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramUniform3ui64ARB(CPU* cpu) {
    if (!ext_glProgramUniform3ui64ARB)
        kpanic("ext_glProgramUniform3ui64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform3ui64ARB)(ARG1, ARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glProgramUniform3ui64ARB GLuint program=%d, GLint location=%d, GLuint64 x=" PRIu64 ", GLuint64 y=" PRIu64 ", GLuint64 z=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glProgramUniform3ui64NV(CPU* cpu) {
    if (!ext_glProgramUniform3ui64NV)
        kpanic("ext_glProgramUniform3ui64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform3ui64NV)(ARG1, ARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glProgramUniform3ui64NV GLuint program=%d, GLint location=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 ", GLuint64EXT z=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glProgramUniform3ui64vARB(CPU* cpu) {
    if (!ext_glProgramUniform3ui64vARB)
        kpanic("ext_glProgramUniform3ui64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform3ui64vARB)(ARG1, ARG2, ARG3, (GLuint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3ui64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3ui64vNV(CPU* cpu) {
    if (!ext_glProgramUniform3ui64vNV)
        kpanic("ext_glProgramUniform3ui64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform3ui64vNV)(ARG1, ARG2, ARG3, (GLuint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3ui64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3uiEXT(CPU* cpu) {
    if (!ext_glProgramUniform3uiEXT)
        kpanic("ext_glProgramUniform3uiEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3uiEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glProgramUniform3uiEXT GLuint program=%d, GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glProgramUniform3uiv(CPU* cpu) {
    if (!ext_glProgramUniform3uiv)
        kpanic("ext_glProgramUniform3uiv is NULL");
    {
    GL_FUNC(ext_glProgramUniform3uiv)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3uiv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform3uivEXT(CPU* cpu) {
    if (!ext_glProgramUniform3uivEXT)
        kpanic("ext_glProgramUniform3uivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform3uivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform3uivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4d(CPU* cpu) {
    if (!ext_glProgramUniform4d)
        kpanic("ext_glProgramUniform4d is NULL");
    {
    GL_FUNC(ext_glProgramUniform4d)(ARG1, ARG2, dARG3, dARG4, dARG5, dARG6);
    GL_LOG ("glProgramUniform4d GLuint program=%d, GLint location=%d, GLdouble v0=%f, GLdouble v1=%f, GLdouble v2=%f, GLdouble v3=%f",ARG1,ARG2,dARG3,dARG4,dARG5,dARG6);
    }
}
void glcommon_glProgramUniform4dEXT(CPU* cpu) {
    if (!ext_glProgramUniform4dEXT)
        kpanic("ext_glProgramUniform4dEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4dEXT)(ARG1, ARG2, dARG3, dARG4, dARG5, dARG6);
    GL_LOG ("glProgramUniform4dEXT GLuint program=%d, GLint location=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,ARG2,dARG3,dARG4,dARG5,dARG6);
    }
}
void glcommon_glProgramUniform4dv(CPU* cpu) {
    if (!ext_glProgramUniform4dv)
        kpanic("ext_glProgramUniform4dv is NULL");
    {
    GL_FUNC(ext_glProgramUniform4dv)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4dv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4dvEXT(CPU* cpu) {
    if (!ext_glProgramUniform4dvEXT)
        kpanic("ext_glProgramUniform4dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4dvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4f(CPU* cpu) {
    if (!ext_glProgramUniform4f)
        kpanic("ext_glProgramUniform4f is NULL");
    {
    GL_FUNC(ext_glProgramUniform4f)(ARG1, ARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glProgramUniform4f GLuint program=%d, GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f, GLfloat v3=%f",ARG1,ARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glProgramUniform4fEXT(CPU* cpu) {
    if (!ext_glProgramUniform4fEXT)
        kpanic("ext_glProgramUniform4fEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4fEXT)(ARG1, ARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glProgramUniform4fEXT GLuint program=%d, GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f, GLfloat v3=%f",ARG1,ARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glProgramUniform4fv(CPU* cpu) {
    if (!ext_glProgramUniform4fv)
        kpanic("ext_glProgramUniform4fv is NULL");
    {
    GL_FUNC(ext_glProgramUniform4fv)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4fv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4fvEXT(CPU* cpu) {
    if (!ext_glProgramUniform4fvEXT)
        kpanic("ext_glProgramUniform4fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4fvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4i(CPU* cpu) {
    if (!ext_glProgramUniform4i)
        kpanic("ext_glProgramUniform4i is NULL");
    {
    GL_FUNC(ext_glProgramUniform4i)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramUniform4i GLuint program=%d, GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d, GLint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramUniform4i64ARB(CPU* cpu) {
    if (!ext_glProgramUniform4i64ARB)
        kpanic("ext_glProgramUniform4i64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform4i64ARB)(ARG1, ARG2, llARG3, llARG4, llARG5, llARG6);
    GL_LOG ("glProgramUniform4i64ARB GLuint program=%d, GLint location=%d, GLint64 x=" PRIu64 ", GLint64 y=" PRIu64 ", GLint64 z=" PRIu64 ", GLint64 w=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5,llARG6);
    }
}
void glcommon_glProgramUniform4i64NV(CPU* cpu) {
    if (!ext_glProgramUniform4i64NV)
        kpanic("ext_glProgramUniform4i64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform4i64NV)(ARG1, ARG2, llARG3, llARG4, llARG5, llARG6);
    GL_LOG ("glProgramUniform4i64NV GLuint program=%d, GLint location=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 ", GLint64EXT z=" PRIu64 ", GLint64EXT w=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5,llARG6);
    }
}
void glcommon_glProgramUniform4i64vARB(CPU* cpu) {
    if (!ext_glProgramUniform4i64vARB)
        kpanic("ext_glProgramUniform4i64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform4i64vARB)(ARG1, ARG2, ARG3, (GLint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4i64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4i64vNV(CPU* cpu) {
    if (!ext_glProgramUniform4i64vNV)
        kpanic("ext_glProgramUniform4i64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform4i64vNV)(ARG1, ARG2, ARG3, (GLint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4i64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4iEXT(CPU* cpu) {
    if (!ext_glProgramUniform4iEXT)
        kpanic("ext_glProgramUniform4iEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4iEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramUniform4iEXT GLuint program=%d, GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d, GLint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramUniform4iv(CPU* cpu) {
    if (!ext_glProgramUniform4iv)
        kpanic("ext_glProgramUniform4iv is NULL");
    {
    GL_FUNC(ext_glProgramUniform4iv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4iv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4ivEXT(CPU* cpu) {
    if (!ext_glProgramUniform4ivEXT)
        kpanic("ext_glProgramUniform4ivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4ivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4ivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4ui(CPU* cpu) {
    if (!ext_glProgramUniform4ui)
        kpanic("ext_glProgramUniform4ui is NULL");
    {
    GL_FUNC(ext_glProgramUniform4ui)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramUniform4ui GLuint program=%d, GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d, GLuint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramUniform4ui64ARB(CPU* cpu) {
    if (!ext_glProgramUniform4ui64ARB)
        kpanic("ext_glProgramUniform4ui64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform4ui64ARB)(ARG1, ARG2, llARG3, llARG4, llARG5, llARG6);
    GL_LOG ("glProgramUniform4ui64ARB GLuint program=%d, GLint location=%d, GLuint64 x=" PRIu64 ", GLuint64 y=" PRIu64 ", GLuint64 z=" PRIu64 ", GLuint64 w=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5,llARG6);
    }
}
void glcommon_glProgramUniform4ui64NV(CPU* cpu) {
    if (!ext_glProgramUniform4ui64NV)
        kpanic("ext_glProgramUniform4ui64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniform4ui64NV)(ARG1, ARG2, llARG3, llARG4, llARG5, llARG6);
    GL_LOG ("glProgramUniform4ui64NV GLuint program=%d, GLint location=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 ", GLuint64EXT z=" PRIu64 ", GLuint64EXT w=" PRIu64 "",ARG1,ARG2,llARG3,llARG4,llARG5,llARG6);
    }
}
void glcommon_glProgramUniform4ui64vARB(CPU* cpu) {
    if (!ext_glProgramUniform4ui64vARB)
        kpanic("ext_glProgramUniform4ui64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniform4ui64vARB)(ARG1, ARG2, ARG3, (GLuint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4ui64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4ui64vNV(CPU* cpu) {
    if (!ext_glProgramUniform4ui64vNV)
        kpanic("ext_glProgramUniform4ui64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniform4ui64vNV)(ARG1, ARG2, ARG3, (GLuint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4ui64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4uiEXT(CPU* cpu) {
    if (!ext_glProgramUniform4uiEXT)
        kpanic("ext_glProgramUniform4uiEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4uiEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glProgramUniform4uiEXT GLuint program=%d, GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d, GLuint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glProgramUniform4uiv(CPU* cpu) {
    if (!ext_glProgramUniform4uiv)
        kpanic("ext_glProgramUniform4uiv is NULL");
    {
    GL_FUNC(ext_glProgramUniform4uiv)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4uiv GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniform4uivEXT(CPU* cpu) {
    if (!ext_glProgramUniform4uivEXT)
        kpanic("ext_glProgramUniform4uivEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniform4uivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniform4uivEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniformHandleui64ARB(CPU* cpu) {
    if (!ext_glProgramUniformHandleui64ARB)
        kpanic("ext_glProgramUniformHandleui64ARB is NULL");
    {
    GL_FUNC(ext_glProgramUniformHandleui64ARB)(ARG1, ARG2, llARG3);
    GL_LOG ("glProgramUniformHandleui64ARB GLuint program=%d, GLint location=%d, GLuint64 value=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glProgramUniformHandleui64NV(CPU* cpu) {
    if (!ext_glProgramUniformHandleui64NV)
        kpanic("ext_glProgramUniformHandleui64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniformHandleui64NV)(ARG1, ARG2, llARG3);
    GL_LOG ("glProgramUniformHandleui64NV GLuint program=%d, GLint location=%d, GLuint64 value=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glProgramUniformHandleui64vARB(CPU* cpu) {
    if (!ext_glProgramUniformHandleui64vARB)
        kpanic("ext_glProgramUniformHandleui64vARB is NULL");
    {
    GL_FUNC(ext_glProgramUniformHandleui64vARB)(ARG1, ARG2, ARG3, (GLuint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniformHandleui64vARB GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64* values=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniformHandleui64vNV(CPU* cpu) {
    if (!ext_glProgramUniformHandleui64vNV)
        kpanic("ext_glProgramUniformHandleui64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniformHandleui64vNV)(ARG1, ARG2, ARG3, (GLuint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniformHandleui64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64* values=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramUniformMatrix2dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2dv)
        kpanic("ext_glProgramUniformMatrix2dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2dvEXT)
        kpanic("ext_glProgramUniformMatrix2dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2fv)
        kpanic("ext_glProgramUniformMatrix2fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2fvEXT)
        kpanic("ext_glProgramUniformMatrix2fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x3dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x3dv)
        kpanic("ext_glProgramUniformMatrix2x3dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x3dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x3dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x3dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x3dvEXT)
        kpanic("ext_glProgramUniformMatrix2x3dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x3dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x3dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x3fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x3fv)
        kpanic("ext_glProgramUniformMatrix2x3fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x3fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x3fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x3fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x3fvEXT)
        kpanic("ext_glProgramUniformMatrix2x3fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x3fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x3fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x4dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x4dv)
        kpanic("ext_glProgramUniformMatrix2x4dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x4dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x4dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x4dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x4dvEXT)
        kpanic("ext_glProgramUniformMatrix2x4dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x4dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x4dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x4fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x4fv)
        kpanic("ext_glProgramUniformMatrix2x4fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x4fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x4fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix2x4fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix2x4fvEXT)
        kpanic("ext_glProgramUniformMatrix2x4fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix2x4fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix2x4fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3dv)
        kpanic("ext_glProgramUniformMatrix3dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3dvEXT)
        kpanic("ext_glProgramUniformMatrix3dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3fv)
        kpanic("ext_glProgramUniformMatrix3fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3fvEXT)
        kpanic("ext_glProgramUniformMatrix3fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x2dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x2dv)
        kpanic("ext_glProgramUniformMatrix3x2dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x2dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x2dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x2dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x2dvEXT)
        kpanic("ext_glProgramUniformMatrix3x2dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x2dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x2dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x2fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x2fv)
        kpanic("ext_glProgramUniformMatrix3x2fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x2fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x2fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x2fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x2fvEXT)
        kpanic("ext_glProgramUniformMatrix3x2fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x2fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x2fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x4dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x4dv)
        kpanic("ext_glProgramUniformMatrix3x4dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x4dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x4dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x4dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x4dvEXT)
        kpanic("ext_glProgramUniformMatrix3x4dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x4dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x4dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x4fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x4fv)
        kpanic("ext_glProgramUniformMatrix3x4fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x4fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x4fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix3x4fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix3x4fvEXT)
        kpanic("ext_glProgramUniformMatrix3x4fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix3x4fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix3x4fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4dv)
        kpanic("ext_glProgramUniformMatrix4dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4dvEXT)
        kpanic("ext_glProgramUniformMatrix4dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4fv)
        kpanic("ext_glProgramUniformMatrix4fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4fvEXT)
        kpanic("ext_glProgramUniformMatrix4fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x2dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x2dv)
        kpanic("ext_glProgramUniformMatrix4x2dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x2dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x2dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x2dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x2dvEXT)
        kpanic("ext_glProgramUniformMatrix4x2dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x2dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x2dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x2fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x2fv)
        kpanic("ext_glProgramUniformMatrix4x2fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x2fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x2fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x2fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x2fvEXT)
        kpanic("ext_glProgramUniformMatrix4x2fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x2fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x2fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x3dv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x3dv)
        kpanic("ext_glProgramUniformMatrix4x3dv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x3dv)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x3dv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x3dvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x3dvEXT)
        kpanic("ext_glProgramUniformMatrix4x3dvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x3dvEXT)(ARG1, ARG2, ARG3, bARG4, (GLdouble*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x3dvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x3fv(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x3fv)
        kpanic("ext_glProgramUniformMatrix4x3fv is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x3fv)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x3fv GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformMatrix4x3fvEXT(CPU* cpu) {
    if (!ext_glProgramUniformMatrix4x3fvEXT)
        kpanic("ext_glProgramUniformMatrix4x3fvEXT is NULL");
    {
    GL_FUNC(ext_glProgramUniformMatrix4x3fvEXT)(ARG1, ARG2, ARG3, bARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glProgramUniformMatrix4x3fvEXT GLuint program=%d, GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glProgramUniformui64NV(CPU* cpu) {
    if (!ext_glProgramUniformui64NV)
        kpanic("ext_glProgramUniformui64NV is NULL");
    {
    GL_FUNC(ext_glProgramUniformui64NV)(ARG1, ARG2, llARG3);
    GL_LOG ("glProgramUniformui64NV GLuint program=%d, GLint location=%d, GLuint64EXT value=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glProgramUniformui64vNV(CPU* cpu) {
    if (!ext_glProgramUniformui64vNV)
        kpanic("ext_glProgramUniformui64vNV is NULL");
    {
    GL_FUNC(ext_glProgramUniformui64vNV)(ARG1, ARG2, ARG3, (GLuint64EXT*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glProgramUniformui64vNV GLuint program=%d, GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glProgramVertexLimitNV(CPU* cpu) {
    if (!ext_glProgramVertexLimitNV)
        kpanic("ext_glProgramVertexLimitNV is NULL");
    {
    GL_FUNC(ext_glProgramVertexLimitNV)(ARG1, ARG2);
    GL_LOG ("glProgramVertexLimitNV GLenum target=%d, GLint limit=%d",ARG1,ARG2);
    }
}
void glcommon_glProvokingVertex(CPU* cpu) {
    if (!ext_glProvokingVertex)
        kpanic("ext_glProvokingVertex is NULL");
    {
    GL_FUNC(ext_glProvokingVertex)(ARG1);
    GL_LOG ("glProvokingVertex GLenum mode=%d",ARG1);
    }
}
void glcommon_glProvokingVertexEXT(CPU* cpu) {
    if (!ext_glProvokingVertexEXT)
        kpanic("ext_glProvokingVertexEXT is NULL");
    {
    GL_FUNC(ext_glProvokingVertexEXT)(ARG1);
    GL_LOG ("glProvokingVertexEXT GLenum mode=%d",ARG1);
    }
}
void glcommon_glPushClientAttribDefaultEXT(CPU* cpu) {
    if (!ext_glPushClientAttribDefaultEXT)
        kpanic("ext_glPushClientAttribDefaultEXT is NULL");
    {
    GL_FUNC(ext_glPushClientAttribDefaultEXT)(ARG1);
    GL_LOG ("glPushClientAttribDefaultEXT GLbitfield mask=%d",ARG1);
    }
}
void glcommon_glPushDebugGroup(CPU* cpu) {
    if (!ext_glPushDebugGroup)
        kpanic("ext_glPushDebugGroup is NULL");
    {
    GL_FUNC(ext_glPushDebugGroup)(ARG1, ARG2, ARG3, marshalsz(cpu, ARG4));
    GL_LOG ("glPushDebugGroup GLenum source=%d, GLuint id=%d, GLsizei length=%d, const GLchar* message=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glPushGroupMarkerEXT(CPU* cpu) {
    if (!ext_glPushGroupMarkerEXT)
        kpanic("ext_glPushGroupMarkerEXT is NULL");
    {
    GL_FUNC(ext_glPushGroupMarkerEXT)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glPushGroupMarkerEXT GLsizei length=%d, const GLchar* marker=%.08x",ARG1,ARG2);
    }
}
void glcommon_glQueryCounter(CPU* cpu) {
    if (!ext_glQueryCounter)
        kpanic("ext_glQueryCounter is NULL");
    {
    GL_FUNC(ext_glQueryCounter)(ARG1, ARG2);
    GL_LOG ("glQueryCounter GLuint id=%d, GLenum target=%d",ARG1,ARG2);
    }
}
void glcommon_glQueryMatrixxOES(CPU* cpu) {
    if (!ext_glQueryMatrixxOES)
        kpanic("ext_glQueryMatrixxOES is NULL");
    {
    EAX=GL_FUNC(ext_glQueryMatrixxOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0), (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glQueryMatrixxOES GLfixed* mantissa=%.08x, GLint* exponent=%.08x",ARG1,ARG2);
    }
}
void glcommon_glQueryObjectParameteruiAMD(CPU* cpu) {
    if (!ext_glQueryObjectParameteruiAMD)
        kpanic("ext_glQueryObjectParameteruiAMD is NULL");
    {
    GL_FUNC(ext_glQueryObjectParameteruiAMD)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glQueryObjectParameteruiAMD GLenum target=%d, GLuint id=%d, GLenum pname=%d, GLuint param=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glRasterPos2xOES(CPU* cpu) {
    if (!ext_glRasterPos2xOES)
        kpanic("ext_glRasterPos2xOES is NULL");
    {
    GL_FUNC(ext_glRasterPos2xOES)(ARG1, ARG2);
    GL_LOG ("glRasterPos2xOES GLfixed x=%d, GLfixed y=%d",ARG1,ARG2);
    }
}
void glcommon_glRasterPos2xvOES(CPU* cpu) {
    if (!ext_glRasterPos2xvOES)
        kpanic("ext_glRasterPos2xvOES is NULL");
    {
    GL_FUNC(ext_glRasterPos2xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glRasterPos2xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glRasterPos3xOES(CPU* cpu) {
    if (!ext_glRasterPos3xOES)
        kpanic("ext_glRasterPos3xOES is NULL");
    {
    GL_FUNC(ext_glRasterPos3xOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glRasterPos3xOES GLfixed x=%d, GLfixed y=%d, GLfixed z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glRasterPos3xvOES(CPU* cpu) {
    if (!ext_glRasterPos3xvOES)
        kpanic("ext_glRasterPos3xvOES is NULL");
    {
    GL_FUNC(ext_glRasterPos3xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glRasterPos3xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glRasterPos4xOES(CPU* cpu) {
    if (!ext_glRasterPos4xOES)
        kpanic("ext_glRasterPos4xOES is NULL");
    {
    GL_FUNC(ext_glRasterPos4xOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glRasterPos4xOES GLfixed x=%d, GLfixed y=%d, GLfixed z=%d, GLfixed w=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glRasterPos4xvOES(CPU* cpu) {
    if (!ext_glRasterPos4xvOES)
        kpanic("ext_glRasterPos4xvOES is NULL");
    {
    GL_FUNC(ext_glRasterPos4xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glRasterPos4xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glRasterSamplesEXT(CPU* cpu) {
    if (!ext_glRasterSamplesEXT)
        kpanic("ext_glRasterSamplesEXT is NULL");
    {
    GL_FUNC(ext_glRasterSamplesEXT)(ARG1, bARG2);
    GL_LOG ("glRasterSamplesEXT GLuint samples=%d, GLboolean fixedsamplelocations=%d",ARG1,bARG2);
    }
}
void glcommon_glReadBufferRegion(CPU* cpu) {
    if (!ext_glReadBufferRegion)
        kpanic("ext_glReadBufferRegion is NULL");
    {
    GL_FUNC(ext_glReadBufferRegion)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glReadBufferRegion GLenum region=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glReadInstrumentsSGIX(CPU* cpu) {
    if (!ext_glReadInstrumentsSGIX)
        kpanic("ext_glReadInstrumentsSGIX is NULL");
    {
    GL_FUNC(ext_glReadInstrumentsSGIX)(ARG1);
    GL_LOG ("glReadInstrumentsSGIX GLint marker=%d",ARG1);
    }
}
void glcommon_glReadnPixels(CPU* cpu) {
    if (!ext_glReadnPixels)
        kpanic("ext_glReadnPixels is NULL");
    {
    GL_FUNC(ext_glReadnPixels)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glReadnPixels GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glReadnPixelsARB(CPU* cpu) {
    if (!ext_glReadnPixelsARB)
        kpanic("ext_glReadnPixelsARB is NULL");
    {
    GL_FUNC(ext_glReadnPixelsARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glReadnPixelsARB GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glRectxOES(CPU* cpu) {
    if (!ext_glRectxOES)
        kpanic("ext_glRectxOES is NULL");
    {
    GL_FUNC(ext_glRectxOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glRectxOES GLfixed x1=%d, GLfixed y1=%d, GLfixed x2=%d, GLfixed y2=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glRectxvOES(CPU* cpu) {
    if (!ext_glRectxvOES)
        kpanic("ext_glRectxvOES is NULL");
    {
    GL_FUNC(ext_glRectxvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0), (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glRectxvOES const GLfixed* v1=%.08x, const GLfixed* v2=%.08x",ARG1,ARG2);
    }
}
void glcommon_glReferencePlaneSGIX(CPU* cpu) {
    if (!ext_glReferencePlaneSGIX)
        kpanic("ext_glReferencePlaneSGIX is NULL");
    {
    GL_FUNC(ext_glReferencePlaneSGIX)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glReferencePlaneSGIX const GLdouble* equation=%.08x",ARG1);
    }
}
void glcommon_glReleaseShaderCompiler(CPU* cpu) {
    if (!ext_glReleaseShaderCompiler)
        kpanic("ext_glReleaseShaderCompiler is NULL");
    {
    GL_FUNC(ext_glReleaseShaderCompiler)();
    GL_LOG ("glReleaseShaderCompiler");
    }
}
void glcommon_glRenderbufferStorage(CPU* cpu) {
    if (!ext_glRenderbufferStorage)
        kpanic("ext_glRenderbufferStorage is NULL");
    {
    GL_FUNC(ext_glRenderbufferStorage)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glRenderbufferStorage GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glRenderbufferStorageEXT(CPU* cpu) {
    if (!ext_glRenderbufferStorageEXT)
        kpanic("ext_glRenderbufferStorageEXT is NULL");
    {
    GL_FUNC(ext_glRenderbufferStorageEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glRenderbufferStorageEXT GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glRenderbufferStorageMultisample(CPU* cpu) {
    if (!ext_glRenderbufferStorageMultisample)
        kpanic("ext_glRenderbufferStorageMultisample is NULL");
    {
    GL_FUNC(ext_glRenderbufferStorageMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glRenderbufferStorageMultisample GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glRenderbufferStorageMultisampleCoverageNV(CPU* cpu) {
    if (!ext_glRenderbufferStorageMultisampleCoverageNV)
        kpanic("ext_glRenderbufferStorageMultisampleCoverageNV is NULL");
    {
    GL_FUNC(ext_glRenderbufferStorageMultisampleCoverageNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glRenderbufferStorageMultisampleCoverageNV GLenum target=%d, GLsizei coverageSamples=%d, GLsizei colorSamples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glRenderbufferStorageMultisampleEXT(CPU* cpu) {
    if (!ext_glRenderbufferStorageMultisampleEXT)
        kpanic("ext_glRenderbufferStorageMultisampleEXT is NULL");
    {
    GL_FUNC(ext_glRenderbufferStorageMultisampleEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glRenderbufferStorageMultisampleEXT GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glReplacementCodePointerSUN(CPU* cpu) {
    if (!ext_glReplacementCodePointerSUN)
        kpanic("ext_glReplacementCodePointerSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodePointerSUN)(ARG1, ARG2, (const void**)marshalunhandled("glReplacementCodePointerSUN", "pointer", cpu, ARG3));
    GL_LOG ("glReplacementCodePointerSUN GLenum type=%d, GLsizei stride=%d, const void** pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glReplacementCodeubSUN(CPU* cpu) {
    if (!ext_glReplacementCodeubSUN)
        kpanic("ext_glReplacementCodeubSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeubSUN)(bARG1);
    GL_LOG ("glReplacementCodeubSUN GLubyte code=%d",bARG1);
    }
}
void glcommon_glReplacementCodeubvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeubvSUN)
        kpanic("ext_glReplacementCodeubvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeubvSUN)((GLubyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glReplacementCodeubvSUN const GLubyte* code=%.08x",ARG1);
    }
}
void glcommon_glReplacementCodeuiColor3fVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiColor3fVertex3fSUN)
        kpanic("ext_glReplacementCodeuiColor3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiColor3fVertex3fSUN)(ARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7);
    GL_LOG ("glReplacementCodeuiColor3fVertex3fSUN GLuint rc=%d, GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7);
    }
}
void glcommon_glReplacementCodeuiColor3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiColor3fVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiColor3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiColor3fVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glReplacementCodeuiColor3fVertex3fvSUN const GLuint* rc=%.08x, const GLfloat* c=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glReplacementCodeuiColor4fNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiColor4fNormal3fVertex3fSUN)
        kpanic("ext_glReplacementCodeuiColor4fNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiColor4fNormal3fVertex3fSUN)(ARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8, fARG9, fARG10, fARG11);
    GL_LOG ("glReplacementCodeuiColor4fNormal3fVertex3fSUN GLuint rc=%d, GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat a=%f, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8,fARG9,fARG10,fARG11);
    }
}
void glcommon_glReplacementCodeuiColor4fNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiColor4fNormal3fVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiColor4fNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiColor4fNormal3fVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glReplacementCodeuiColor4fNormal3fVertex3fvSUN const GLuint* rc=%.08x, const GLfloat* c=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glReplacementCodeuiColor4ubVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiColor4ubVertex3fSUN)
        kpanic("ext_glReplacementCodeuiColor4ubVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiColor4ubVertex3fSUN)(ARG1, bARG2, bARG3, bARG4, bARG5, fARG6, fARG7, fARG8);
    GL_LOG ("glReplacementCodeuiColor4ubVertex3fSUN GLuint rc=%d, GLubyte r=%d, GLubyte g=%d, GLubyte b=%d, GLubyte a=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,bARG2,bARG3,bARG4,bARG5,fARG6,fARG7,fARG8);
    }
}
void glcommon_glReplacementCodeuiColor4ubVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiColor4ubVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiColor4ubVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiColor4ubVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLubyte*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glReplacementCodeuiColor4ubVertex3fvSUN const GLuint* rc=%.08x, const GLubyte* c=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glReplacementCodeuiNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiNormal3fVertex3fSUN)
        kpanic("ext_glReplacementCodeuiNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiNormal3fVertex3fSUN)(ARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7);
    GL_LOG ("glReplacementCodeuiNormal3fVertex3fSUN GLuint rc=%d, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7);
    }
}
void glcommon_glReplacementCodeuiNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiNormal3fVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiNormal3fVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glReplacementCodeuiNormal3fVertex3fvSUN const GLuint* rc=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glReplacementCodeuiSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiSUN)
        kpanic("ext_glReplacementCodeuiSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiSUN)(ARG1);
    GL_LOG ("glReplacementCodeuiSUN GLuint code=%d",ARG1);
    }
}
void glcommon_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN)
        kpanic("ext_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN)(ARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8, fARG9, fARG10, fARG11, fARG12, fARG13);
    GL_LOG ("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN GLuint rc=%d, GLfloat s=%f, GLfloat t=%f, GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat a=%f, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8,fARG9,fARG10,fARG11,fARG12,fARG13);
    }
}
void glcommon_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0), (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN const GLuint* rc=%.08x, const GLfloat* tc=%.08x, const GLfloat* c=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN)
        kpanic("ext_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN)(ARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8, fARG9);
    GL_LOG ("glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN GLuint rc=%d, GLfloat s=%f, GLfloat t=%f, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8,fARG9);
    }
}
void glcommon_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN const GLuint* rc=%.08x, const GLfloat* tc=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glReplacementCodeuiTexCoord2fVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiTexCoord2fVertex3fSUN)
        kpanic("ext_glReplacementCodeuiTexCoord2fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiTexCoord2fVertex3fSUN)(ARG1, fARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glReplacementCodeuiTexCoord2fVertex3fSUN GLuint rc=%d, GLfloat s=%f, GLfloat t=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glReplacementCodeuiTexCoord2fVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiTexCoord2fVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiTexCoord2fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiTexCoord2fVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glReplacementCodeuiTexCoord2fVertex3fvSUN const GLuint* rc=%.08x, const GLfloat* tc=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glReplacementCodeuiVertex3fSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiVertex3fSUN)
        kpanic("ext_glReplacementCodeuiVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiVertex3fSUN)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glReplacementCodeuiVertex3fSUN GLuint rc=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glReplacementCodeuiVertex3fvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuiVertex3fvSUN)
        kpanic("ext_glReplacementCodeuiVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuiVertex3fvSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glReplacementCodeuiVertex3fvSUN const GLuint* rc=%.08x, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glReplacementCodeuivSUN(CPU* cpu) {
    if (!ext_glReplacementCodeuivSUN)
        kpanic("ext_glReplacementCodeuivSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeuivSUN)((GLuint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glReplacementCodeuivSUN const GLuint* code=%.08x",ARG1);
    }
}
void glcommon_glReplacementCodeusSUN(CPU* cpu) {
    if (!ext_glReplacementCodeusSUN)
        kpanic("ext_glReplacementCodeusSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeusSUN)(sARG1);
    GL_LOG ("glReplacementCodeusSUN GLushort code=%d",sARG1);
    }
}
void glcommon_glReplacementCodeusvSUN(CPU* cpu) {
    if (!ext_glReplacementCodeusvSUN)
        kpanic("ext_glReplacementCodeusvSUN is NULL");
    {
    GL_FUNC(ext_glReplacementCodeusvSUN)((GLushort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glReplacementCodeusvSUN const GLushort* code=%.08x",ARG1);
    }
}
void glcommon_glRequestResidentProgramsNV(CPU* cpu) {
    if (!ext_glRequestResidentProgramsNV)
        kpanic("ext_glRequestResidentProgramsNV is NULL");
    {
    GL_FUNC(ext_glRequestResidentProgramsNV)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glRequestResidentProgramsNV GLsizei n=%d, const GLuint* programs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glResetHistogram(CPU* cpu) {
    if (!ext_glResetHistogram)
        kpanic("ext_glResetHistogram is NULL");
    {
    GL_FUNC(ext_glResetHistogram)(ARG1);
    GL_LOG ("glResetHistogram GLenum target=%d",ARG1);
    }
}
void glcommon_glResetHistogramEXT(CPU* cpu) {
    if (!ext_glResetHistogramEXT)
        kpanic("ext_glResetHistogramEXT is NULL");
    {
    GL_FUNC(ext_glResetHistogramEXT)(ARG1);
    GL_LOG ("glResetHistogramEXT GLenum target=%d",ARG1);
    }
}
void glcommon_glResetMinmax(CPU* cpu) {
    if (!ext_glResetMinmax)
        kpanic("ext_glResetMinmax is NULL");
    {
    GL_FUNC(ext_glResetMinmax)(ARG1);
    GL_LOG ("glResetMinmax GLenum target=%d",ARG1);
    }
}
void glcommon_glResetMinmaxEXT(CPU* cpu) {
    if (!ext_glResetMinmaxEXT)
        kpanic("ext_glResetMinmaxEXT is NULL");
    {
    GL_FUNC(ext_glResetMinmaxEXT)(ARG1);
    GL_LOG ("glResetMinmaxEXT GLenum target=%d",ARG1);
    }
}
void glcommon_glResizeBuffersMESA(CPU* cpu) {
    if (!ext_glResizeBuffersMESA)
        kpanic("ext_glResizeBuffersMESA is NULL");
    {
    GL_FUNC(ext_glResizeBuffersMESA)();
    GL_LOG ("glResizeBuffersMESA");
    }
}
void glcommon_glResolveDepthValuesNV(CPU* cpu) {
    if (!ext_glResolveDepthValuesNV)
        kpanic("ext_glResolveDepthValuesNV is NULL");
    {
    GL_FUNC(ext_glResolveDepthValuesNV)();
    GL_LOG ("glResolveDepthValuesNV");
    }
}
void glcommon_glResumeTransformFeedback(CPU* cpu) {
    if (!ext_glResumeTransformFeedback)
        kpanic("ext_glResumeTransformFeedback is NULL");
    {
    GL_FUNC(ext_glResumeTransformFeedback)();
    GL_LOG ("glResumeTransformFeedback");
    }
}
void glcommon_glResumeTransformFeedbackNV(CPU* cpu) {
    if (!ext_glResumeTransformFeedbackNV)
        kpanic("ext_glResumeTransformFeedbackNV is NULL");
    {
    GL_FUNC(ext_glResumeTransformFeedbackNV)();
    GL_LOG ("glResumeTransformFeedbackNV");
    }
}
void glcommon_glRotatexOES(CPU* cpu) {
    if (!ext_glRotatexOES)
        kpanic("ext_glRotatexOES is NULL");
    {
    GL_FUNC(ext_glRotatexOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glRotatexOES GLfixed angle=%d, GLfixed x=%d, GLfixed y=%d, GLfixed z=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glSampleCoverage(CPU* cpu) {
    if (!ext_glSampleCoverage)
        kpanic("ext_glSampleCoverage is NULL");
    {
    GL_FUNC(ext_glSampleCoverage)(fARG1, bARG2);
    GL_LOG ("glSampleCoverage GLfloat value=%f, GLboolean invert=%d",fARG1,bARG2);
    }
}
void glcommon_glSampleCoverageARB(CPU* cpu) {
    if (!ext_glSampleCoverageARB)
        kpanic("ext_glSampleCoverageARB is NULL");
    {
    GL_FUNC(ext_glSampleCoverageARB)(fARG1, bARG2);
    GL_LOG ("glSampleCoverageARB GLfloat value=%f, GLboolean invert=%d",fARG1,bARG2);
    }
}
void glcommon_glSampleMapATI(CPU* cpu) {
    if (!ext_glSampleMapATI)
        kpanic("ext_glSampleMapATI is NULL");
    {
    GL_FUNC(ext_glSampleMapATI)(ARG1, ARG2, ARG3);
    GL_LOG ("glSampleMapATI GLuint dst=%d, GLuint interp=%d, GLenum swizzle=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSampleMaskEXT(CPU* cpu) {
    if (!ext_glSampleMaskEXT)
        kpanic("ext_glSampleMaskEXT is NULL");
    {
    GL_FUNC(ext_glSampleMaskEXT)(fARG1, bARG2);
    GL_LOG ("glSampleMaskEXT GLclampf value=%f, GLboolean invert=%d",fARG1,bARG2);
    }
}
void glcommon_glSampleMaskIndexedNV(CPU* cpu) {
    if (!ext_glSampleMaskIndexedNV)
        kpanic("ext_glSampleMaskIndexedNV is NULL");
    {
    GL_FUNC(ext_glSampleMaskIndexedNV)(ARG1, ARG2);
    GL_LOG ("glSampleMaskIndexedNV GLuint index=%d, GLbitfield mask=%d",ARG1,ARG2);
    }
}
void glcommon_glSampleMaskSGIS(CPU* cpu) {
    if (!ext_glSampleMaskSGIS)
        kpanic("ext_glSampleMaskSGIS is NULL");
    {
    GL_FUNC(ext_glSampleMaskSGIS)(fARG1, bARG2);
    GL_LOG ("glSampleMaskSGIS GLclampf value=%f, GLboolean invert=%d",fARG1,bARG2);
    }
}
void glcommon_glSampleMaski(CPU* cpu) {
    if (!ext_glSampleMaski)
        kpanic("ext_glSampleMaski is NULL");
    {
    GL_FUNC(ext_glSampleMaski)(ARG1, ARG2);
    GL_LOG ("glSampleMaski GLuint maskNumber=%d, GLbitfield mask=%d",ARG1,ARG2);
    }
}
void glcommon_glSamplePatternEXT(CPU* cpu) {
    if (!ext_glSamplePatternEXT)
        kpanic("ext_glSamplePatternEXT is NULL");
    {
    GL_FUNC(ext_glSamplePatternEXT)(ARG1);
    GL_LOG ("glSamplePatternEXT GLenum pattern=%d",ARG1);
    }
}
void glcommon_glSamplePatternSGIS(CPU* cpu) {
    if (!ext_glSamplePatternSGIS)
        kpanic("ext_glSamplePatternSGIS is NULL");
    {
    GL_FUNC(ext_glSamplePatternSGIS)(ARG1);
    GL_LOG ("glSamplePatternSGIS GLenum pattern=%d",ARG1);
    }
}
void glcommon_glSamplerParameterIiv(CPU* cpu) {
    if (!ext_glSamplerParameterIiv)
        kpanic("ext_glSamplerParameterIiv is NULL");
    {
    GL_FUNC(ext_glSamplerParameterIiv)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, (ARG2==GL_TEXTURE_BORDER_COLOR)?4:1));
    GL_LOG ("glSamplerParameterIiv GLuint sampler=%d, GLenum pname=%d, const GLint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSamplerParameterIuiv(CPU* cpu) {
    if (!ext_glSamplerParameterIuiv)
        kpanic("ext_glSamplerParameterIuiv is NULL");
    {
    GL_FUNC(ext_glSamplerParameterIuiv)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, (ARG2==GL_TEXTURE_BORDER_COLOR)?4:1));
    GL_LOG ("glSamplerParameterIuiv GLuint sampler=%d, GLenum pname=%d, const GLuint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSamplerParameterf(CPU* cpu) {
    if (!ext_glSamplerParameterf)
        kpanic("ext_glSamplerParameterf is NULL");
    {
    GL_FUNC(ext_glSamplerParameterf)(ARG1, ARG2, fARG3);
    GL_LOG ("glSamplerParameterf GLuint sampler=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glSamplerParameterfv(CPU* cpu) {
    if (!ext_glSamplerParameterfv)
        kpanic("ext_glSamplerParameterfv is NULL");
    {
    GL_FUNC(ext_glSamplerParameterfv)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, (ARG2==GL_TEXTURE_BORDER_COLOR)?4:1));
    GL_LOG ("glSamplerParameterfv GLuint sampler=%d, GLenum pname=%d, const GLfloat* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSamplerParameteri(CPU* cpu) {
    if (!ext_glSamplerParameteri)
        kpanic("ext_glSamplerParameteri is NULL");
    {
    GL_FUNC(ext_glSamplerParameteri)(ARG1, ARG2, ARG3);
    GL_LOG ("glSamplerParameteri GLuint sampler=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSamplerParameteriv(CPU* cpu) {
    if (!ext_glSamplerParameteriv)
        kpanic("ext_glSamplerParameteriv is NULL");
    {
    GL_FUNC(ext_glSamplerParameteriv)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, (ARG2==GL_TEXTURE_BORDER_COLOR)?4:1));
    GL_LOG ("glSamplerParameteriv GLuint sampler=%d, GLenum pname=%d, const GLint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glScalexOES(CPU* cpu) {
    if (!ext_glScalexOES)
        kpanic("ext_glScalexOES is NULL");
    {
    GL_FUNC(ext_glScalexOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glScalexOES GLfixed x=%d, GLfixed y=%d, GLfixed z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glScissorArrayv(CPU* cpu) {
    if (!ext_glScissorArrayv)
        kpanic("ext_glScissorArrayv is NULL");
    {
    GL_FUNC(ext_glScissorArrayv)(ARG1, ARG2, (GLint*)marshalArray<GLint>(cpu, ARG3, ARG2*4));
    GL_LOG ("glScissorArrayv GLuint first=%d, GLsizei count=%d, const GLint* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glScissorIndexed(CPU* cpu) {
    if (!ext_glScissorIndexed)
        kpanic("ext_glScissorIndexed is NULL");
    {
    GL_FUNC(ext_glScissorIndexed)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glScissorIndexed GLuint index=%d, GLint left=%d, GLint bottom=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glScissorIndexedv(CPU* cpu) {
    if (!ext_glScissorIndexedv)
        kpanic("ext_glScissorIndexedv is NULL");
    {
    GL_FUNC(ext_glScissorIndexedv)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glScissorIndexedv GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glSecondaryColor3b(CPU* cpu) {
    if (!ext_glSecondaryColor3b)
        kpanic("ext_glSecondaryColor3b is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3b)(bARG1, bARG2, bARG3);
    GL_LOG ("glSecondaryColor3b GLbyte red=%d, GLbyte green=%d, GLbyte blue=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glSecondaryColor3bEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3bEXT)
        kpanic("ext_glSecondaryColor3bEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3bEXT)(bARG1, bARG2, bARG3);
    GL_LOG ("glSecondaryColor3bEXT GLbyte red=%d, GLbyte green=%d, GLbyte blue=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glSecondaryColor3bv(CPU* cpu) {
    if (!ext_glSecondaryColor3bv)
        kpanic("ext_glSecondaryColor3bv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3bv)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3bv const GLbyte* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3bvEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3bvEXT)
        kpanic("ext_glSecondaryColor3bvEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3bvEXT)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3bvEXT const GLbyte* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3d(CPU* cpu) {
    if (!ext_glSecondaryColor3d)
        kpanic("ext_glSecondaryColor3d is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3d)(dARG1, dARG2, dARG3);
    GL_LOG ("glSecondaryColor3d GLdouble red=%f, GLdouble green=%f, GLdouble blue=%f",dARG1,dARG2,dARG3);
    }
}
void glcommon_glSecondaryColor3dEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3dEXT)
        kpanic("ext_glSecondaryColor3dEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3dEXT)(dARG1, dARG2, dARG3);
    GL_LOG ("glSecondaryColor3dEXT GLdouble red=%f, GLdouble green=%f, GLdouble blue=%f",dARG1,dARG2,dARG3);
    }
}
void glcommon_glSecondaryColor3dv(CPU* cpu) {
    if (!ext_glSecondaryColor3dv)
        kpanic("ext_glSecondaryColor3dv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3dv)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3dv const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3dvEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3dvEXT)
        kpanic("ext_glSecondaryColor3dvEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3dvEXT)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3dvEXT const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3f(CPU* cpu) {
    if (!ext_glSecondaryColor3f)
        kpanic("ext_glSecondaryColor3f is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3f)(fARG1, fARG2, fARG3);
    GL_LOG ("glSecondaryColor3f GLfloat red=%f, GLfloat green=%f, GLfloat blue=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glSecondaryColor3fEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3fEXT)
        kpanic("ext_glSecondaryColor3fEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3fEXT)(fARG1, fARG2, fARG3);
    GL_LOG ("glSecondaryColor3fEXT GLfloat red=%f, GLfloat green=%f, GLfloat blue=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glSecondaryColor3fv(CPU* cpu) {
    if (!ext_glSecondaryColor3fv)
        kpanic("ext_glSecondaryColor3fv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3fv)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3fv const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3fvEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3fvEXT)
        kpanic("ext_glSecondaryColor3fvEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3fvEXT)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3fvEXT const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3hNV(CPU* cpu) {
    if (!ext_glSecondaryColor3hNV)
        kpanic("ext_glSecondaryColor3hNV is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3hNV)(sARG1, sARG2, sARG3);
    GL_LOG ("glSecondaryColor3hNV GLhalfNV red=%d, GLhalfNV green=%d, GLhalfNV blue=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glSecondaryColor3hvNV(CPU* cpu) {
    if (!ext_glSecondaryColor3hvNV)
        kpanic("ext_glSecondaryColor3hvNV is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3i(CPU* cpu) {
    if (!ext_glSecondaryColor3i)
        kpanic("ext_glSecondaryColor3i is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3i)(ARG1, ARG2, ARG3);
    GL_LOG ("glSecondaryColor3i GLint red=%d, GLint green=%d, GLint blue=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSecondaryColor3iEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3iEXT)
        kpanic("ext_glSecondaryColor3iEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3iEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glSecondaryColor3iEXT GLint red=%d, GLint green=%d, GLint blue=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSecondaryColor3iv(CPU* cpu) {
    if (!ext_glSecondaryColor3iv)
        kpanic("ext_glSecondaryColor3iv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3iv)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3iv const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3ivEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3ivEXT)
        kpanic("ext_glSecondaryColor3ivEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3ivEXT)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3ivEXT const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3s(CPU* cpu) {
    if (!ext_glSecondaryColor3s)
        kpanic("ext_glSecondaryColor3s is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3s)(sARG1, sARG2, sARG3);
    GL_LOG ("glSecondaryColor3s GLshort red=%d, GLshort green=%d, GLshort blue=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glSecondaryColor3sEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3sEXT)
        kpanic("ext_glSecondaryColor3sEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3sEXT)(sARG1, sARG2, sARG3);
    GL_LOG ("glSecondaryColor3sEXT GLshort red=%d, GLshort green=%d, GLshort blue=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glSecondaryColor3sv(CPU* cpu) {
    if (!ext_glSecondaryColor3sv)
        kpanic("ext_glSecondaryColor3sv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3sv)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3sv const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3svEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3svEXT)
        kpanic("ext_glSecondaryColor3svEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3svEXT)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3svEXT const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3ub(CPU* cpu) {
    if (!ext_glSecondaryColor3ub)
        kpanic("ext_glSecondaryColor3ub is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3ub)(bARG1, bARG2, bARG3);
    GL_LOG ("glSecondaryColor3ub GLubyte red=%d, GLubyte green=%d, GLubyte blue=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glSecondaryColor3ubEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3ubEXT)
        kpanic("ext_glSecondaryColor3ubEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3ubEXT)(bARG1, bARG2, bARG3);
    GL_LOG ("glSecondaryColor3ubEXT GLubyte red=%d, GLubyte green=%d, GLubyte blue=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glSecondaryColor3ubv(CPU* cpu) {
    if (!ext_glSecondaryColor3ubv)
        kpanic("ext_glSecondaryColor3ubv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3ubv)((GLubyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3ubv const GLubyte* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3ubvEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3ubvEXT)
        kpanic("ext_glSecondaryColor3ubvEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3ubvEXT)((GLubyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3ubvEXT const GLubyte* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3ui(CPU* cpu) {
    if (!ext_glSecondaryColor3ui)
        kpanic("ext_glSecondaryColor3ui is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glSecondaryColor3ui GLuint red=%d, GLuint green=%d, GLuint blue=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSecondaryColor3uiEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3uiEXT)
        kpanic("ext_glSecondaryColor3uiEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3uiEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glSecondaryColor3uiEXT GLuint red=%d, GLuint green=%d, GLuint blue=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSecondaryColor3uiv(CPU* cpu) {
    if (!ext_glSecondaryColor3uiv)
        kpanic("ext_glSecondaryColor3uiv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3uiv)((GLuint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3uiv const GLuint* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3uivEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3uivEXT)
        kpanic("ext_glSecondaryColor3uivEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3uivEXT)((GLuint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3uivEXT const GLuint* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3us(CPU* cpu) {
    if (!ext_glSecondaryColor3us)
        kpanic("ext_glSecondaryColor3us is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3us)(sARG1, sARG2, sARG3);
    GL_LOG ("glSecondaryColor3us GLushort red=%d, GLushort green=%d, GLushort blue=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glSecondaryColor3usEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3usEXT)
        kpanic("ext_glSecondaryColor3usEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3usEXT)(sARG1, sARG2, sARG3);
    GL_LOG ("glSecondaryColor3usEXT GLushort red=%d, GLushort green=%d, GLushort blue=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glSecondaryColor3usv(CPU* cpu) {
    if (!ext_glSecondaryColor3usv)
        kpanic("ext_glSecondaryColor3usv is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3usv)((GLushort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3usv const GLushort* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColor3usvEXT(CPU* cpu) {
    if (!ext_glSecondaryColor3usvEXT)
        kpanic("ext_glSecondaryColor3usvEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColor3usvEXT)((GLushort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glSecondaryColor3usvEXT const GLushort* v=%.08x",ARG1);
    }
}
void glcommon_glSecondaryColorFormatNV(CPU* cpu) {
    if (!ext_glSecondaryColorFormatNV)
        kpanic("ext_glSecondaryColorFormatNV is NULL");
    {
    GL_FUNC(ext_glSecondaryColorFormatNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glSecondaryColorFormatNV GLint size=%d, GLenum type=%d, GLsizei stride=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSecondaryColorP3ui(CPU* cpu) {
    if (!ext_glSecondaryColorP3ui)
        kpanic("ext_glSecondaryColorP3ui is NULL");
    {
    GL_FUNC(ext_glSecondaryColorP3ui)(ARG1, ARG2);
    GL_LOG ("glSecondaryColorP3ui GLenum type=%d, GLuint color=%d",ARG1,ARG2);
    }
}
void glcommon_glSecondaryColorP3uiv(CPU* cpu) {
    if (!ext_glSecondaryColorP3uiv)
        kpanic("ext_glSecondaryColorP3uiv is NULL");
    {
    GL_FUNC(ext_glSecondaryColorP3uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glSecondaryColorP3uiv GLenum type=%d, const GLuint* color=%.08x",ARG1,ARG2);
    }
}
void glcommon_glSecondaryColorPointer(CPU* cpu) {
    if (!ext_glSecondaryColorPointer)
        kpanic("ext_glSecondaryColorPointer is NULL");
    {
    GL_FUNC(ext_glSecondaryColorPointer)(ARG1, ARG2, ARG3, marshalSecondaryColorPointer(cpu, ARG1, ARG2, ARG3, ARG4));
    GL_LOG ("glSecondaryColorPointer GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glSecondaryColorPointerEXT(CPU* cpu) {
    if (!ext_glSecondaryColorPointerEXT)
        kpanic("ext_glSecondaryColorPointerEXT is NULL");
    {
    GL_FUNC(ext_glSecondaryColorPointerEXT)(ARG1, ARG2, ARG3, marshalSecondaryColorPointerEXT(cpu, ARG1, ARG2, ARG3, ARG4));
    GL_LOG ("glSecondaryColorPointerEXT GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glSecondaryColorPointerListIBM(CPU* cpu) {
    if (!ext_glSecondaryColorPointerListIBM)
        kpanic("ext_glSecondaryColorPointerListIBM is NULL");
    {
    GL_FUNC(ext_glSecondaryColorPointerListIBM)(ARG1, ARG2, ARG3, (const void**)marshalunhandled("glSecondaryColorPointerListIBM", "pointer", cpu, ARG4), ARG5);
    GL_LOG ("glSecondaryColorPointerListIBM GLint size=%d, GLenum type=%d, GLint stride=%d, const void** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glSelectPerfMonitorCountersAMD(CPU* cpu) {
    if (!ext_glSelectPerfMonitorCountersAMD)
        kpanic("ext_glSelectPerfMonitorCountersAMD is NULL");
    {
    GL_FUNC(ext_glSelectPerfMonitorCountersAMD)(ARG1, bARG2, ARG3, ARG4, (GLuint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glSelectPerfMonitorCountersAMD GLuint monitor=%d, GLboolean enable=%d, GLuint group=%d, GLint numCounters=%d, GLuint* counterList=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glSelectTextureCoordSetSGIS(CPU* cpu) {
    if (!ext_glSelectTextureCoordSetSGIS)
        kpanic("ext_glSelectTextureCoordSetSGIS is NULL");
    {
    GL_FUNC(ext_glSelectTextureCoordSetSGIS)(ARG1);
    GL_LOG ("glSelectTextureCoordSetSGIS GLenum target=%d",ARG1);
    }
}
void glcommon_glSelectTextureSGIS(CPU* cpu) {
    if (!ext_glSelectTextureSGIS)
        kpanic("ext_glSelectTextureSGIS is NULL");
    {
    GL_FUNC(ext_glSelectTextureSGIS)(ARG1);
    GL_LOG ("glSelectTextureSGIS GLenum target=%d",ARG1);
    }
}
void glcommon_glSeparableFilter2D(CPU* cpu) {
    if (!ext_glSeparableFilter2D)
        kpanic("ext_glSeparableFilter2D is NULL");
    {
    GL_FUNC(ext_glSeparableFilter2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, (void*)marshalp(cpu, 0, ARG7, 0), (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glSeparableFilter2D GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* row=%.08x, const void* column=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glSeparableFilter2DEXT(CPU* cpu) {
    if (!ext_glSeparableFilter2DEXT)
        kpanic("ext_glSeparableFilter2DEXT is NULL");
    {
    GL_FUNC(ext_glSeparableFilter2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, (void*)marshalp(cpu, 0, ARG7, 0), (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glSeparableFilter2DEXT GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* row=%.08x, const void* column=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glSetFenceAPPLE(CPU* cpu) {
    if (!ext_glSetFenceAPPLE)
        kpanic("ext_glSetFenceAPPLE is NULL");
    {
    GL_FUNC(ext_glSetFenceAPPLE)(ARG1);
    GL_LOG ("glSetFenceAPPLE GLuint fence=%d",ARG1);
    }
}
#endif
#endif
