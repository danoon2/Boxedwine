#include "boxedwine.h"

#ifndef DISABLE_GL_EXTENSIONS
#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"
#include "glMarshal.h"

void glcommon_glAccumxOES(CPU* cpu) {
    if (!ext_glAccumxOES)
        kpanic("ext_glAccumxOES is NULL");
    {
    GL_FUNC(ext_glAccumxOES)(ARG1, ARG2);
    GL_LOG ("glAccumxOES GLenum op=%d, GLfixed value=%d",ARG1,ARG2);
    }
}
void glcommon_glActiveProgramEXT(CPU* cpu) {
    if (!ext_glActiveProgramEXT)
        kpanic("ext_glActiveProgramEXT is NULL");
    {
    GL_FUNC(ext_glActiveProgramEXT)(ARG1);
    GL_LOG ("glActiveProgramEXT GLuint program=%d",ARG1);
    }
}
void glcommon_glActiveShaderProgram(CPU* cpu) {
    if (!ext_glActiveShaderProgram)
        kpanic("ext_glActiveShaderProgram is NULL");
    {
    GL_FUNC(ext_glActiveShaderProgram)(ARG1, ARG2);
    GL_LOG ("glActiveShaderProgram GLuint pipeline=%d, GLuint program=%d",ARG1,ARG2);
    }
}
void glcommon_glActiveStencilFaceEXT(CPU* cpu) {
    if (!ext_glActiveStencilFaceEXT)
        kpanic("ext_glActiveStencilFaceEXT is NULL");
    {
    GL_FUNC(ext_glActiveStencilFaceEXT)(ARG1);
    GL_LOG ("glActiveStencilFaceEXT GLenum face=%d",ARG1);
    }
}
void glcommon_glActiveTexture(CPU* cpu) {
    if (!ext_glActiveTexture)
        kpanic("ext_glActiveTexture is NULL");
    {
    GL_FUNC(ext_glActiveTexture)(ARG1);
    GL_LOG ("glActiveTexture GLenum texture=%d",ARG1);
    }
}
void glcommon_glActiveTextureARB(CPU* cpu) {
    if (!ext_glActiveTextureARB)
        kpanic("ext_glActiveTextureARB is NULL");
    {
    GL_FUNC(ext_glActiveTextureARB)(ARG1);
    GL_LOG ("glActiveTextureARB GLenum texture=%d",ARG1);
    }
}
void glcommon_glActiveVaryingNV(CPU* cpu) {
    if (!ext_glActiveVaryingNV)
        kpanic("ext_glActiveVaryingNV is NULL");
    {
    GL_FUNC(ext_glActiveVaryingNV)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glActiveVaryingNV GLuint program=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glAlphaFragmentOp1ATI(CPU* cpu) {
    if (!ext_glAlphaFragmentOp1ATI)
        kpanic("ext_glAlphaFragmentOp1ATI is NULL");
    {
    GL_FUNC(ext_glAlphaFragmentOp1ATI)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glAlphaFragmentOp1ATI GLenum op=%d, GLuint dst=%d, GLuint dstMod=%d, GLuint arg1=%d, GLuint arg1Rep=%d, GLuint arg1Mod=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glAlphaFragmentOp2ATI(CPU* cpu) {
    if (!ext_glAlphaFragmentOp2ATI)
        kpanic("ext_glAlphaFragmentOp2ATI is NULL");
    {
    GL_FUNC(ext_glAlphaFragmentOp2ATI)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glAlphaFragmentOp2ATI GLenum op=%d, GLuint dst=%d, GLuint dstMod=%d, GLuint arg1=%d, GLuint arg1Rep=%d, GLuint arg1Mod=%d, GLuint arg2=%d, GLuint arg2Rep=%d, GLuint arg2Mod=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glAlphaFragmentOp3ATI(CPU* cpu) {
    if (!ext_glAlphaFragmentOp3ATI)
        kpanic("ext_glAlphaFragmentOp3ATI is NULL");
    {
    GL_FUNC(ext_glAlphaFragmentOp3ATI)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12);
    GL_LOG ("glAlphaFragmentOp3ATI GLenum op=%d, GLuint dst=%d, GLuint dstMod=%d, GLuint arg1=%d, GLuint arg1Rep=%d, GLuint arg1Mod=%d, GLuint arg2=%d, GLuint arg2Rep=%d, GLuint arg2Mod=%d, GLuint arg3=%d, GLuint arg3Rep=%d, GLuint arg3Mod=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12);
    }
}
void glcommon_glAlphaFuncxOES(CPU* cpu) {
    if (!ext_glAlphaFuncxOES)
        kpanic("ext_glAlphaFuncxOES is NULL");
    {
    GL_FUNC(ext_glAlphaFuncxOES)(ARG1, ARG2);
    GL_LOG ("glAlphaFuncxOES GLenum func=%d, GLfixed ref=%d",ARG1,ARG2);
    }
}
void glcommon_glApplyFramebufferAttachmentCMAAINTEL(CPU* cpu) {
    if (!ext_glApplyFramebufferAttachmentCMAAINTEL)
        kpanic("ext_glApplyFramebufferAttachmentCMAAINTEL is NULL");
    {
    GL_FUNC(ext_glApplyFramebufferAttachmentCMAAINTEL)();
    GL_LOG ("glApplyFramebufferAttachmentCMAAINTEL");
    }
}
void glcommon_glApplyTextureEXT(CPU* cpu) {
    if (!ext_glApplyTextureEXT)
        kpanic("ext_glApplyTextureEXT is NULL");
    {
    GL_FUNC(ext_glApplyTextureEXT)(ARG1);
    GL_LOG ("glApplyTextureEXT GLenum mode=%d",ARG1);
    }
}
void glcommon_glAreProgramsResidentNV(CPU* cpu) {
    if (!ext_glAreProgramsResidentNV)
        kpanic("ext_glAreProgramsResidentNV is NULL");
    {
        MarshalReadWrite<GLboolean> rw(cpu, ARG3, ARG1);
        EAX=GL_FUNC(ext_glAreProgramsResidentNV)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1), rw.getPtr());
        GL_LOG ("glAreProgramsResidentNV GLsizei n=%d, const GLuint* programs=%.08x, GLboolean* residences=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glAreTexturesResidentEXT(CPU* cpu) {
    if (!ext_glAreTexturesResidentEXT)
        kpanic("ext_glAreTexturesResidentEXT is NULL");
    {
        MarshalReadWrite<GLboolean> rw(cpu, ARG3, ARG1);
        EAX=GL_FUNC(ext_glAreTexturesResidentEXT)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1), rw.getPtr());
        GL_LOG ("glAreTexturesResidentEXT GLsizei n=%d, const GLuint* textures=%.08x, GLboolean* residences=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glArrayElementEXT(CPU* cpu) {
    if (!ext_glArrayElementEXT)
        kpanic("ext_glArrayElementEXT is NULL");
    {
    GL_FUNC(ext_glArrayElementEXT)(ARG1);
    GL_LOG ("glArrayElementEXT GLint i=%d",ARG1);
    }
}
void glcommon_glArrayObjectATI(CPU* cpu) {
    if (!ext_glArrayObjectATI)
        kpanic("ext_glArrayObjectATI is NULL");
    {
    GL_FUNC(ext_glArrayObjectATI)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glArrayObjectATI GLenum array=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLuint buffer=%d, GLuint offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glAsyncMarkerSGIX(CPU* cpu) {
    if (!ext_glAsyncMarkerSGIX)
        kpanic("ext_glAsyncMarkerSGIX is NULL");
    {
    GL_FUNC(ext_glAsyncMarkerSGIX)(ARG1);
    GL_LOG ("glAsyncMarkerSGIX GLuint marker=%d",ARG1);
    }
}
void glcommon_glAttachObjectARB(CPU* cpu) {
    if (!ext_glAttachObjectARB)
        kpanic("ext_glAttachObjectARB is NULL");
    {
    GL_FUNC(ext_glAttachObjectARB)(INDEX_TO_HANDLE(hARG1), INDEX_TO_HANDLE(hARG2));
    GL_LOG ("glAttachObjectARB GLhandleARB containerObj=%d, GLhandleARB obj=%d",ARG1,ARG2);
    }
}
void glcommon_glAttachShader(CPU* cpu) {
    if (!ext_glAttachShader)
        kpanic("ext_glAttachShader is NULL");
    {
    GL_FUNC(ext_glAttachShader)(ARG1, ARG2);
    GL_LOG ("glAttachShader GLuint program=%d, GLuint shader=%d",ARG1,ARG2);
    }
}
void glcommon_glBeginConditionalRender(CPU* cpu) {
    if (!ext_glBeginConditionalRender)
        kpanic("ext_glBeginConditionalRender is NULL");
    {
    GL_FUNC(ext_glBeginConditionalRender)(ARG1, ARG2);
    GL_LOG ("glBeginConditionalRender GLuint id=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glBeginConditionalRenderNV(CPU* cpu) {
    if (!ext_glBeginConditionalRenderNV)
        kpanic("ext_glBeginConditionalRenderNV is NULL");
    {
    GL_FUNC(ext_glBeginConditionalRenderNV)(ARG1, ARG2);
    GL_LOG ("glBeginConditionalRenderNV GLuint id=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glBeginConditionalRenderNVX(CPU* cpu) {
    if (!ext_glBeginConditionalRenderNVX)
        kpanic("ext_glBeginConditionalRenderNVX is NULL");
    {
    GL_FUNC(ext_glBeginConditionalRenderNVX)(ARG1);
    GL_LOG ("glBeginConditionalRenderNVX GLuint id=%d",ARG1);
    }
}
void glcommon_glBeginFragmentShaderATI(CPU* cpu) {
    if (!ext_glBeginFragmentShaderATI)
        kpanic("ext_glBeginFragmentShaderATI is NULL");
    {
    GL_FUNC(ext_glBeginFragmentShaderATI)();
    GL_LOG ("glBeginFragmentShaderATI");
    }
}
void glcommon_glBeginOcclusionQueryNV(CPU* cpu) {
    if (!ext_glBeginOcclusionQueryNV)
        kpanic("ext_glBeginOcclusionQueryNV is NULL");
    {
    GL_FUNC(ext_glBeginOcclusionQueryNV)(ARG1);
    GL_LOG ("glBeginOcclusionQueryNV GLuint id=%d",ARG1);
    }
}
void glcommon_glBeginPerfMonitorAMD(CPU* cpu) {
    if (!ext_glBeginPerfMonitorAMD)
        kpanic("ext_glBeginPerfMonitorAMD is NULL");
    {
    GL_FUNC(ext_glBeginPerfMonitorAMD)(ARG1);
    GL_LOG ("glBeginPerfMonitorAMD GLuint monitor=%d",ARG1);
    }
}
void glcommon_glBeginPerfQueryINTEL(CPU* cpu) {
    if (!ext_glBeginPerfQueryINTEL)
        kpanic("ext_glBeginPerfQueryINTEL is NULL");
    {
    GL_FUNC(ext_glBeginPerfQueryINTEL)(ARG1);
    GL_LOG ("glBeginPerfQueryINTEL GLuint queryHandle=%d",ARG1);
    }
}
void glcommon_glBeginQuery(CPU* cpu) {
    if (!ext_glBeginQuery)
        kpanic("ext_glBeginQuery is NULL");
    {
    GL_FUNC(ext_glBeginQuery)(ARG1, ARG2);
    GL_LOG ("glBeginQuery GLenum target=%d, GLuint id=%d",ARG1,ARG2);
    }
}
void glcommon_glBeginQueryARB(CPU* cpu) {
    if (!ext_glBeginQueryARB)
        kpanic("ext_glBeginQueryARB is NULL");
    {
    GL_FUNC(ext_glBeginQueryARB)(ARG1, ARG2);
    GL_LOG ("glBeginQueryARB GLenum target=%d, GLuint id=%d",ARG1,ARG2);
    }
}
void glcommon_glBeginQueryIndexed(CPU* cpu) {
    if (!ext_glBeginQueryIndexed)
        kpanic("ext_glBeginQueryIndexed is NULL");
    {
    GL_FUNC(ext_glBeginQueryIndexed)(ARG1, ARG2, ARG3);
    GL_LOG ("glBeginQueryIndexed GLenum target=%d, GLuint index=%d, GLuint id=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBeginTransformFeedback(CPU* cpu) {
    if (!ext_glBeginTransformFeedback)
        kpanic("ext_glBeginTransformFeedback is NULL");
    {
    GL_FUNC(ext_glBeginTransformFeedback)(ARG1);
    GL_LOG ("glBeginTransformFeedback GLenum primitiveMode=%d",ARG1);
    }
}
void glcommon_glBeginTransformFeedbackEXT(CPU* cpu) {
    if (!ext_glBeginTransformFeedbackEXT)
        kpanic("ext_glBeginTransformFeedbackEXT is NULL");
    {
    GL_FUNC(ext_glBeginTransformFeedbackEXT)(ARG1);
    GL_LOG ("glBeginTransformFeedbackEXT GLenum primitiveMode=%d",ARG1);
    }
}
void glcommon_glBeginTransformFeedbackNV(CPU* cpu) {
    if (!ext_glBeginTransformFeedbackNV)
        kpanic("ext_glBeginTransformFeedbackNV is NULL");
    {
    GL_FUNC(ext_glBeginTransformFeedbackNV)(ARG1);
    GL_LOG ("glBeginTransformFeedbackNV GLenum primitiveMode=%d",ARG1);
    }
}
void glcommon_glBeginVertexShaderEXT(CPU* cpu) {
    if (!ext_glBeginVertexShaderEXT)
        kpanic("ext_glBeginVertexShaderEXT is NULL");
    {
    GL_FUNC(ext_glBeginVertexShaderEXT)();
    GL_LOG ("glBeginVertexShaderEXT");
    }
}
void glcommon_glBeginVideoCaptureNV(CPU* cpu) {
    if (!ext_glBeginVideoCaptureNV)
        kpanic("ext_glBeginVideoCaptureNV is NULL");
    {
    GL_FUNC(ext_glBeginVideoCaptureNV)(ARG1);
    GL_LOG ("glBeginVideoCaptureNV GLuint video_capture_slot=%d",ARG1);
    }
}
void glcommon_glBindAttribLocation(CPU* cpu) {
    if (!ext_glBindAttribLocation)
        kpanic("ext_glBindAttribLocation is NULL");
    {
    GL_FUNC(ext_glBindAttribLocation)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glBindAttribLocation GLuint program=%d, GLuint index=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindAttribLocationARB(CPU* cpu) {
    if (!ext_glBindAttribLocationARB)
        kpanic("ext_glBindAttribLocationARB is NULL");
    {
    GL_FUNC(ext_glBindAttribLocationARB)(INDEX_TO_HANDLE(hARG1), ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glBindAttribLocationARB GLhandleARB programObj=%d, GLuint index=%d, const GLcharARB* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindBuffer(CPU* cpu) {
    if (!ext_glBindBuffer)
        kpanic("ext_glBindBuffer is NULL");
    {
    GL_FUNC(ext_glBindBuffer)(ARG1, ARG2);
    GL_LOG ("glBindBuffer GLenum target=%d, GLuint buffer=%d",ARG1,ARG2);
    }
}
void glcommon_glBindBufferARB(CPU* cpu) {
    if (!ext_glBindBufferARB)
        kpanic("ext_glBindBufferARB is NULL");
    {
    GL_FUNC(ext_glBindBufferARB)(ARG1, ARG2);
    GL_LOG ("glBindBufferARB GLenum target=%d, GLuint buffer=%d",ARG1,ARG2);
    }
}
void glcommon_glBindBufferBase(CPU* cpu) {
    if (!ext_glBindBufferBase)
        kpanic("ext_glBindBufferBase is NULL");
    {
    GL_FUNC(ext_glBindBufferBase)(ARG1, ARG2, ARG3);
    GL_LOG ("glBindBufferBase GLenum target=%d, GLuint index=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindBufferBaseEXT(CPU* cpu) {
    if (!ext_glBindBufferBaseEXT)
        kpanic("ext_glBindBufferBaseEXT is NULL");
    {
    GL_FUNC(ext_glBindBufferBaseEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glBindBufferBaseEXT GLenum target=%d, GLuint index=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindBufferBaseNV(CPU* cpu) {
    if (!ext_glBindBufferBaseNV)
        kpanic("ext_glBindBufferBaseNV is NULL");
    {
    GL_FUNC(ext_glBindBufferBaseNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glBindBufferBaseNV GLenum target=%d, GLuint index=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindBufferOffsetEXT(CPU* cpu) {
    if (!ext_glBindBufferOffsetEXT)
        kpanic("ext_glBindBufferOffsetEXT is NULL");
    {
    GL_FUNC(ext_glBindBufferOffsetEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBindBufferOffsetEXT GLenum target=%d, GLuint index=%d, GLuint buffer=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBindBufferOffsetNV(CPU* cpu) {
    if (!ext_glBindBufferOffsetNV)
        kpanic("ext_glBindBufferOffsetNV is NULL");
    {
    GL_FUNC(ext_glBindBufferOffsetNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBindBufferOffsetNV GLenum target=%d, GLuint index=%d, GLuint buffer=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBindBufferRange(CPU* cpu) {
    if (!ext_glBindBufferRange)
        kpanic("ext_glBindBufferRange is NULL");
    {
    GL_FUNC(ext_glBindBufferRange)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glBindBufferRange GLenum target=%d, GLuint index=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBindBufferRangeEXT(CPU* cpu) {
    if (!ext_glBindBufferRangeEXT)
        kpanic("ext_glBindBufferRangeEXT is NULL");
    {
    GL_FUNC(ext_glBindBufferRangeEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glBindBufferRangeEXT GLenum target=%d, GLuint index=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBindBufferRangeNV(CPU* cpu) {
    if (!ext_glBindBufferRangeNV)
        kpanic("ext_glBindBufferRangeNV is NULL");
    {
    GL_FUNC(ext_glBindBufferRangeNV)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glBindBufferRangeNV GLenum target=%d, GLuint index=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBindBuffersBase(CPU* cpu) {
    if (!ext_glBindBuffersBase)
        kpanic("ext_glBindBuffersBase is NULL");
    {
    GL_FUNC(ext_glBindBuffersBase)(ARG1, ARG2, ARG3, marshalArray<GLuint>(cpu, ARG4, ARG3));
    GL_LOG ("glBindBuffersBase GLenum target=%d, GLuint first=%d, GLsizei count=%d, const GLuint* buffers=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBindBuffersRange(CPU* cpu) {
    if (!ext_glBindBuffersRange)
        kpanic("ext_glBindBuffersRange is NULL");
    {
    GL_FUNC(ext_glBindBuffersRange)(ARG1, ARG2, ARG3, marshalArray<GLuint>(cpu, ARG4, ARG3), marshalip(cpu, ARG5, ARG3), marshalsip(cpu, ARG6, ARG3));
    GL_LOG ("glBindBuffersRange GLenum target=%d, GLuint first=%d, GLsizei count=%d, const GLuint* buffers=%.08x, const GLintptr* offsets=%.08x, const GLsizeiptr* sizes=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glBindFragDataLocation(CPU* cpu) {
    if (!ext_glBindFragDataLocation)
        kpanic("ext_glBindFragDataLocation is NULL");
    {
    GL_FUNC(ext_glBindFragDataLocation)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glBindFragDataLocation GLuint program=%d, GLuint color=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindFragDataLocationEXT(CPU* cpu) {
    if (!ext_glBindFragDataLocationEXT)
        kpanic("ext_glBindFragDataLocationEXT is NULL");
    {
    GL_FUNC(ext_glBindFragDataLocationEXT)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glBindFragDataLocationEXT GLuint program=%d, GLuint color=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindFragDataLocationIndexed(CPU* cpu) {
    if (!ext_glBindFragDataLocationIndexed)
        kpanic("ext_glBindFragDataLocationIndexed is NULL");
    {
    GL_FUNC(ext_glBindFragDataLocationIndexed)(ARG1, ARG2, ARG3, marshalsz(cpu, ARG4));
    GL_LOG ("glBindFragDataLocationIndexed GLuint program=%d, GLuint colorNumber=%d, GLuint index=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBindFragmentShaderATI(CPU* cpu) {
    if (!ext_glBindFragmentShaderATI)
        kpanic("ext_glBindFragmentShaderATI is NULL");
    {
    GL_FUNC(ext_glBindFragmentShaderATI)(ARG1);
    GL_LOG ("glBindFragmentShaderATI GLuint id=%d",ARG1);
    }
}
void glcommon_glBindFramebuffer(CPU* cpu) {
    if (!ext_glBindFramebuffer)
        kpanic("ext_glBindFramebuffer is NULL");
    {
    GL_FUNC(ext_glBindFramebuffer)(ARG1, ARG2);
    GL_LOG ("glBindFramebuffer GLenum target=%d, GLuint framebuffer=%d",ARG1,ARG2);
    }
}
void glcommon_glBindFramebufferEXT(CPU* cpu) {
    if (!ext_glBindFramebufferEXT)
        kpanic("ext_glBindFramebufferEXT is NULL");
    {
    GL_FUNC(ext_glBindFramebufferEXT)(ARG1, ARG2);
    GL_LOG ("glBindFramebufferEXT GLenum target=%d, GLuint framebuffer=%d",ARG1,ARG2);
    }
}
void glcommon_glBindImageTexture(CPU* cpu) {
    if (!ext_glBindImageTexture)
        kpanic("ext_glBindImageTexture is NULL");
    {
    GL_FUNC(ext_glBindImageTexture)(ARG1, ARG2, ARG3, bARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glBindImageTexture GLuint unit=%d, GLuint texture=%d, GLint level=%d, GLboolean layered=%d, GLint layer=%d, GLenum access=%d, GLenum format=%d",ARG1,ARG2,ARG3,bARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glBindImageTextureEXT(CPU* cpu) {
    if (!ext_glBindImageTextureEXT)
        kpanic("ext_glBindImageTextureEXT is NULL");
    {
    GL_FUNC(ext_glBindImageTextureEXT)(ARG1, ARG2, ARG3, bARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glBindImageTextureEXT GLuint index=%d, GLuint texture=%d, GLint level=%d, GLboolean layered=%d, GLint layer=%d, GLenum access=%d, GLint format=%d",ARG1,ARG2,ARG3,bARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glBindImageTextures(CPU* cpu) {
    if (!ext_glBindImageTextures)
        kpanic("ext_glBindImageTextures is NULL");
    {
    GL_FUNC(ext_glBindImageTextures)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2));
    GL_LOG ("glBindImageTextures GLuint first=%d, GLsizei count=%d, const GLuint* textures=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindLightParameterEXT(CPU* cpu) {
    if (!ext_glBindLightParameterEXT)
        kpanic("ext_glBindLightParameterEXT is NULL");
    {
    EAX=GL_FUNC(ext_glBindLightParameterEXT)(ARG1, ARG2);
    GL_LOG ("glBindLightParameterEXT GLenum light=%d, GLenum value=%d",ARG1,ARG2);
    }
}
void glcommon_glBindMaterialParameterEXT(CPU* cpu) {
    if (!ext_glBindMaterialParameterEXT)
        kpanic("ext_glBindMaterialParameterEXT is NULL");
    {
    EAX=GL_FUNC(ext_glBindMaterialParameterEXT)(ARG1, ARG2);
    GL_LOG ("glBindMaterialParameterEXT GLenum face=%d, GLenum value=%d",ARG1,ARG2);
    }
}
void glcommon_glBindMultiTextureEXT(CPU* cpu) {
    if (!ext_glBindMultiTextureEXT)
        kpanic("ext_glBindMultiTextureEXT is NULL");
    {
    GL_FUNC(ext_glBindMultiTextureEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glBindMultiTextureEXT GLenum texunit=%d, GLenum target=%d, GLuint texture=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindParameterEXT(CPU* cpu) {
    if (!ext_glBindParameterEXT)
        kpanic("ext_glBindParameterEXT is NULL");
    {
    EAX=GL_FUNC(ext_glBindParameterEXT)(ARG1);
    GL_LOG ("glBindParameterEXT GLenum value=%d",ARG1);
    }
}
void glcommon_glBindProgramARB(CPU* cpu) {
    if (!ext_glBindProgramARB)
        kpanic("ext_glBindProgramARB is NULL");
    {
    GL_FUNC(ext_glBindProgramARB)(ARG1, ARG2);
    GL_LOG ("glBindProgramARB GLenum target=%d, GLuint program=%d",ARG1,ARG2);
    }
}
void glcommon_glBindProgramNV(CPU* cpu) {
    if (!ext_glBindProgramNV)
        kpanic("ext_glBindProgramNV is NULL");
    {
    GL_FUNC(ext_glBindProgramNV)(ARG1, ARG2);
    GL_LOG ("glBindProgramNV GLenum target=%d, GLuint id=%d",ARG1,ARG2);
    }
}
void glcommon_glBindProgramPipeline(CPU* cpu) {
    if (!ext_glBindProgramPipeline)
        kpanic("ext_glBindProgramPipeline is NULL");
    {
    GL_FUNC(ext_glBindProgramPipeline)(ARG1);
    GL_LOG ("glBindProgramPipeline GLuint pipeline=%d",ARG1);
    }
}
void glcommon_glBindRenderbuffer(CPU* cpu) {
    if (!ext_glBindRenderbuffer)
        kpanic("ext_glBindRenderbuffer is NULL");
    {
    GL_FUNC(ext_glBindRenderbuffer)(ARG1, ARG2);
    GL_LOG ("glBindRenderbuffer GLenum target=%d, GLuint renderbuffer=%d",ARG1,ARG2);
    }
}
void glcommon_glBindRenderbufferEXT(CPU* cpu) {
    if (!ext_glBindRenderbufferEXT)
        kpanic("ext_glBindRenderbufferEXT is NULL");
    {
    GL_FUNC(ext_glBindRenderbufferEXT)(ARG1, ARG2);
    GL_LOG ("glBindRenderbufferEXT GLenum target=%d, GLuint renderbuffer=%d",ARG1,ARG2);
    }
}
void glcommon_glBindSampler(CPU* cpu) {
    if (!ext_glBindSampler)
        kpanic("ext_glBindSampler is NULL");
    {
    GL_FUNC(ext_glBindSampler)(ARG1, ARG2);
    GL_LOG ("glBindSampler GLuint unit=%d, GLuint sampler=%d",ARG1,ARG2);
    }
}
void glcommon_glBindSamplers(CPU* cpu) {
    if (!ext_glBindSamplers)
        kpanic("ext_glBindSamplers is NULL");
    {
    GL_FUNC(ext_glBindSamplers)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2));
    GL_LOG ("glBindSamplers GLuint first=%d, GLsizei count=%d, const GLuint* samplers=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindTexGenParameterEXT(CPU* cpu) {
    if (!ext_glBindTexGenParameterEXT)
        kpanic("ext_glBindTexGenParameterEXT is NULL");
    {
    EAX=GL_FUNC(ext_glBindTexGenParameterEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glBindTexGenParameterEXT GLenum unit=%d, GLenum coord=%d, GLenum value=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindTextureEXT(CPU* cpu) {
    if (!ext_glBindTextureEXT)
        kpanic("ext_glBindTextureEXT is NULL");
    {
    GL_FUNC(ext_glBindTextureEXT)(ARG1, ARG2);
    GL_LOG ("glBindTextureEXT GLenum target=%d, GLuint texture=%d",ARG1,ARG2);
    }
}
void glcommon_glBindTextureUnit(CPU* cpu) {
    if (!ext_glBindTextureUnit)
        kpanic("ext_glBindTextureUnit is NULL");
    {
    GL_FUNC(ext_glBindTextureUnit)(ARG1, ARG2);
    GL_LOG ("glBindTextureUnit GLuint unit=%d, GLuint texture=%d",ARG1,ARG2);
    }
}
void glcommon_glBindTextureUnitParameterEXT(CPU* cpu) {
    if (!ext_glBindTextureUnitParameterEXT)
        kpanic("ext_glBindTextureUnitParameterEXT is NULL");
    {
    EAX=GL_FUNC(ext_glBindTextureUnitParameterEXT)(ARG1, ARG2);
    GL_LOG ("glBindTextureUnitParameterEXT GLenum unit=%d, GLenum value=%d",ARG1,ARG2);
    }
}
void glcommon_glBindTextures(CPU* cpu) {
    if (!ext_glBindTextures)
        kpanic("ext_glBindTextures is NULL");
    {
    GL_FUNC(ext_glBindTextures)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2));
    GL_LOG ("glBindTextures GLuint first=%d, GLsizei count=%d, const GLuint* textures=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBindTransformFeedback(CPU* cpu) {
    if (!ext_glBindTransformFeedback)
        kpanic("ext_glBindTransformFeedback is NULL");
    {
    GL_FUNC(ext_glBindTransformFeedback)(ARG1, ARG2);
    GL_LOG ("glBindTransformFeedback GLenum target=%d, GLuint id=%d",ARG1,ARG2);
    }
}
void glcommon_glBindTransformFeedbackNV(CPU* cpu) {
    if (!ext_glBindTransformFeedbackNV)
        kpanic("ext_glBindTransformFeedbackNV is NULL");
    {
    GL_FUNC(ext_glBindTransformFeedbackNV)(ARG1, ARG2);
    GL_LOG ("glBindTransformFeedbackNV GLenum target=%d, GLuint id=%d",ARG1,ARG2);
    }
}
void glcommon_glBindVertexArray(CPU* cpu) {
    if (!ext_glBindVertexArray)
        kpanic("ext_glBindVertexArray is NULL");
    {
    GL_FUNC(ext_glBindVertexArray)(ARG1);
    GL_LOG ("glBindVertexArray GLuint array=%d",ARG1);
    }
}
void glcommon_glBindVertexArrayAPPLE(CPU* cpu) {
    if (!ext_glBindVertexArrayAPPLE)
        kpanic("ext_glBindVertexArrayAPPLE is NULL");
    {
    GL_FUNC(ext_glBindVertexArrayAPPLE)(ARG1);
    GL_LOG ("glBindVertexArrayAPPLE GLuint array=%d",ARG1);
    }
}
void glcommon_glBindVertexBuffer(CPU* cpu) {
    if (!ext_glBindVertexBuffer)
        kpanic("ext_glBindVertexBuffer is NULL");
    {
    GL_FUNC(ext_glBindVertexBuffer)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBindVertexBuffer GLuint bindingindex=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBindVertexBuffers(CPU* cpu) {
    if (!ext_glBindVertexBuffers)
        kpanic("ext_glBindVertexBuffers is NULL");
    {
    GL_FUNC(ext_glBindVertexBuffers)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2), marshalip(cpu, ARG4, ARG2), marshalArray<GLint>(cpu, ARG5, ARG2));
    GL_LOG ("glBindVertexBuffers GLuint first=%d, GLsizei count=%d, const GLuint* buffers=%.08x, const GLintptr* offsets=%.08x, const GLsizei* strides=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBindVertexShaderEXT(CPU* cpu) {
    if (!ext_glBindVertexShaderEXT)
        kpanic("ext_glBindVertexShaderEXT is NULL");
    {
    GL_FUNC(ext_glBindVertexShaderEXT)(ARG1);
    GL_LOG ("glBindVertexShaderEXT GLuint id=%d",ARG1);
    }
}
void glcommon_glBindVideoCaptureStreamBufferNV(CPU* cpu) {
    if (!ext_glBindVideoCaptureStreamBufferNV)
        kpanic("ext_glBindVideoCaptureStreamBufferNV is NULL");
    {
    GL_FUNC(ext_glBindVideoCaptureStreamBufferNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBindVideoCaptureStreamBufferNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum frame_region=%d, GLintptrARB offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBindVideoCaptureStreamTextureNV(CPU* cpu) {
    if (!ext_glBindVideoCaptureStreamTextureNV)
        kpanic("ext_glBindVideoCaptureStreamTextureNV is NULL");
    {
    GL_FUNC(ext_glBindVideoCaptureStreamTextureNV)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glBindVideoCaptureStreamTextureNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum frame_region=%d, GLenum target=%d, GLuint texture=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBinormal3bEXT(CPU* cpu) {
    if (!ext_glBinormal3bEXT)
        kpanic("ext_glBinormal3bEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3bEXT)(bARG1, bARG2, bARG3);
    GL_LOG ("glBinormal3bEXT GLbyte bx=%d, GLbyte by=%d, GLbyte bz=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glBinormal3bvEXT(CPU* cpu) {
    if (!ext_glBinormal3bvEXT)
        kpanic("ext_glBinormal3bvEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3bvEXT)(marshalArray<GLbyte>(cpu, ARG1, 3));
    GL_LOG ("glBinormal3bvEXT const GLbyte* v=%.08x",ARG1);
    }
}
void glcommon_glBinormal3dEXT(CPU* cpu) {
    if (!ext_glBinormal3dEXT)
        kpanic("ext_glBinormal3dEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3dEXT)(dARG1, dARG2, dARG3);
    GL_LOG ("glBinormal3dEXT GLdouble bx=%f, GLdouble by=%f, GLdouble bz=%f",dARG1,dARG2,dARG3);
    }
}
void glcommon_glBinormal3dvEXT(CPU* cpu) {
    if (!ext_glBinormal3dvEXT)
        kpanic("ext_glBinormal3dvEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3dvEXT)(marshalArray<GLdouble>(cpu, ARG1, 3));
    GL_LOG ("glBinormal3dvEXT const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glBinormal3fEXT(CPU* cpu) {
    if (!ext_glBinormal3fEXT)
        kpanic("ext_glBinormal3fEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3fEXT)(fARG1, fARG2, fARG3);
    GL_LOG ("glBinormal3fEXT GLfloat bx=%f, GLfloat by=%f, GLfloat bz=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glBinormal3fvEXT(CPU* cpu) {
    if (!ext_glBinormal3fvEXT)
        kpanic("ext_glBinormal3fvEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3fvEXT)(marshalArray<GLfloat>(cpu, ARG1, 3));
    GL_LOG ("glBinormal3fvEXT const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glBinormal3iEXT(CPU* cpu) {
    if (!ext_glBinormal3iEXT)
        kpanic("ext_glBinormal3iEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3iEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glBinormal3iEXT GLint bx=%d, GLint by=%d, GLint bz=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBinormal3ivEXT(CPU* cpu) {
    if (!ext_glBinormal3ivEXT)
        kpanic("ext_glBinormal3ivEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3ivEXT)(marshalArray<GLint>(cpu, ARG1, 3));
    GL_LOG ("glBinormal3ivEXT const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glBinormal3sEXT(CPU* cpu) {
    if (!ext_glBinormal3sEXT)
        kpanic("ext_glBinormal3sEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3sEXT)(sARG1, sARG2, sARG3);
    GL_LOG ("glBinormal3sEXT GLshort bx=%d, GLshort by=%d, GLshort bz=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glBinormal3svEXT(CPU* cpu) {
    if (!ext_glBinormal3svEXT)
        kpanic("ext_glBinormal3svEXT is NULL");
    {
    GL_FUNC(ext_glBinormal3svEXT)(marshalArray<GLshort>(cpu, ARG1, 3));
    GL_LOG ("glBinormal3svEXT const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glBinormalPointerEXT(CPU* cpu) {
    if (!ext_glBinormalPointerEXT)
        kpanic("ext_glBinormalPointerEXT is NULL");
    {
    GL_FUNC(ext_glBinormalPointerEXT)(ARG1, ARG2, marshalType(cpu, ARG1, ARG2, ARG3));
    GL_LOG ("glBinormalPointerEXT GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBitmapxOES(CPU* cpu) {
    if (!ext_glBitmapxOES)
        kpanic("ext_glBitmapxOES is NULL");
    {
    GL_FUNC(ext_glBitmapxOES)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, marshalArray<GLubyte>(cpu, ARG7, ARG1*ARG2));
    GL_LOG ("glBitmapxOES GLsizei width=%d, GLsizei height=%d, GLfixed xorig=%d, GLfixed yorig=%d, GLfixed xmove=%d, GLfixed ymove=%d, const GLubyte* bitmap=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glBlendBarrierKHR(CPU* cpu) {
    if (!ext_glBlendBarrierKHR)
        kpanic("ext_glBlendBarrierKHR is NULL");
    {
    GL_FUNC(ext_glBlendBarrierKHR)();
    GL_LOG ("glBlendBarrierKHR");
    }
}
void glcommon_glBlendBarrierNV(CPU* cpu) {
    if (!ext_glBlendBarrierNV)
        kpanic("ext_glBlendBarrierNV is NULL");
    {
    GL_FUNC(ext_glBlendBarrierNV)();
    GL_LOG ("glBlendBarrierNV");
    }
}
void glcommon_glBlendColor(CPU* cpu) {
    if (!ext_glBlendColor)
        kpanic("ext_glBlendColor is NULL");
    {
    GL_FUNC(ext_glBlendColor)(fARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glBlendColor GLfloat red=%f, GLfloat green=%f, GLfloat blue=%f, GLfloat alpha=%f",fARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glBlendColorEXT(CPU* cpu) {
    if (!ext_glBlendColorEXT)
        kpanic("ext_glBlendColorEXT is NULL");
    {
    GL_FUNC(ext_glBlendColorEXT)(fARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glBlendColorEXT GLfloat red=%f, GLfloat green=%f, GLfloat blue=%f, GLfloat alpha=%f",fARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glBlendColorxOES(CPU* cpu) {
    if (!ext_glBlendColorxOES)
        kpanic("ext_glBlendColorxOES is NULL");
    {
    GL_FUNC(ext_glBlendColorxOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBlendColorxOES GLfixed red=%d, GLfixed green=%d, GLfixed blue=%d, GLfixed alpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBlendEquation(CPU* cpu) {
    if (!ext_glBlendEquation)
        kpanic("ext_glBlendEquation is NULL");
    {
    GL_FUNC(ext_glBlendEquation)(ARG1);
    GL_LOG ("glBlendEquation GLenum mode=%d",ARG1);
    }
}
void glcommon_glBlendEquationEXT(CPU* cpu) {
    if (!ext_glBlendEquationEXT)
        kpanic("ext_glBlendEquationEXT is NULL");
    {
    GL_FUNC(ext_glBlendEquationEXT)(ARG1);
    GL_LOG ("glBlendEquationEXT GLenum mode=%d",ARG1);
    }
}
void glcommon_glBlendEquationIndexedAMD(CPU* cpu) {
    if (!ext_glBlendEquationIndexedAMD)
        kpanic("ext_glBlendEquationIndexedAMD is NULL");
    {
    GL_FUNC(ext_glBlendEquationIndexedAMD)(ARG1, ARG2);
    GL_LOG ("glBlendEquationIndexedAMD GLuint buf=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glBlendEquationSeparate(CPU* cpu) {
    if (!ext_glBlendEquationSeparate)
        kpanic("ext_glBlendEquationSeparate is NULL");
    {
    GL_FUNC(ext_glBlendEquationSeparate)(ARG1, ARG2);
    GL_LOG ("glBlendEquationSeparate GLenum modeRGB=%d, GLenum modeAlpha=%d",ARG1,ARG2);
    }
}
void glcommon_glBlendEquationSeparateEXT(CPU* cpu) {
    if (!ext_glBlendEquationSeparateEXT)
        kpanic("ext_glBlendEquationSeparateEXT is NULL");
    {
    GL_FUNC(ext_glBlendEquationSeparateEXT)(ARG1, ARG2);
    GL_LOG ("glBlendEquationSeparateEXT GLenum modeRGB=%d, GLenum modeAlpha=%d",ARG1,ARG2);
    }
}
void glcommon_glBlendEquationSeparateIndexedAMD(CPU* cpu) {
    if (!ext_glBlendEquationSeparateIndexedAMD)
        kpanic("ext_glBlendEquationSeparateIndexedAMD is NULL");
    {
    GL_FUNC(ext_glBlendEquationSeparateIndexedAMD)(ARG1, ARG2, ARG3);
    GL_LOG ("glBlendEquationSeparateIndexedAMD GLuint buf=%d, GLenum modeRGB=%d, GLenum modeAlpha=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBlendEquationSeparatei(CPU* cpu) {
    if (!ext_glBlendEquationSeparatei)
        kpanic("ext_glBlendEquationSeparatei is NULL");
    {
    GL_FUNC(ext_glBlendEquationSeparatei)(ARG1, ARG2, ARG3);
    GL_LOG ("glBlendEquationSeparatei GLuint buf=%d, GLenum modeRGB=%d, GLenum modeAlpha=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBlendEquationSeparateiARB(CPU* cpu) {
    if (!ext_glBlendEquationSeparateiARB)
        kpanic("ext_glBlendEquationSeparateiARB is NULL");
    {
    GL_FUNC(ext_glBlendEquationSeparateiARB)(ARG1, ARG2, ARG3);
    GL_LOG ("glBlendEquationSeparateiARB GLuint buf=%d, GLenum modeRGB=%d, GLenum modeAlpha=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBlendEquationi(CPU* cpu) {
    if (!ext_glBlendEquationi)
        kpanic("ext_glBlendEquationi is NULL");
    {
    GL_FUNC(ext_glBlendEquationi)(ARG1, ARG2);
    GL_LOG ("glBlendEquationi GLuint buf=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glBlendEquationiARB(CPU* cpu) {
    if (!ext_glBlendEquationiARB)
        kpanic("ext_glBlendEquationiARB is NULL");
    {
    GL_FUNC(ext_glBlendEquationiARB)(ARG1, ARG2);
    GL_LOG ("glBlendEquationiARB GLuint buf=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glBlendFuncIndexedAMD(CPU* cpu) {
    if (!ext_glBlendFuncIndexedAMD)
        kpanic("ext_glBlendFuncIndexedAMD is NULL");
    {
    GL_FUNC(ext_glBlendFuncIndexedAMD)(ARG1, ARG2, ARG3);
    GL_LOG ("glBlendFuncIndexedAMD GLuint buf=%d, GLenum src=%d, GLenum dst=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBlendFuncSeparate(CPU* cpu) {
    if (!ext_glBlendFuncSeparate)
        kpanic("ext_glBlendFuncSeparate is NULL");
    {
    GL_FUNC(ext_glBlendFuncSeparate)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBlendFuncSeparate GLenum sfactorRGB=%d, GLenum dfactorRGB=%d, GLenum sfactorAlpha=%d, GLenum dfactorAlpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBlendFuncSeparateEXT(CPU* cpu) {
    if (!ext_glBlendFuncSeparateEXT)
        kpanic("ext_glBlendFuncSeparateEXT is NULL");
    {
    GL_FUNC(ext_glBlendFuncSeparateEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBlendFuncSeparateEXT GLenum sfactorRGB=%d, GLenum dfactorRGB=%d, GLenum sfactorAlpha=%d, GLenum dfactorAlpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBlendFuncSeparateINGR(CPU* cpu) {
    if (!ext_glBlendFuncSeparateINGR)
        kpanic("ext_glBlendFuncSeparateINGR is NULL");
    {
    GL_FUNC(ext_glBlendFuncSeparateINGR)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glBlendFuncSeparateINGR GLenum sfactorRGB=%d, GLenum dfactorRGB=%d, GLenum sfactorAlpha=%d, GLenum dfactorAlpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBlendFuncSeparateIndexedAMD(CPU* cpu) {
    if (!ext_glBlendFuncSeparateIndexedAMD)
        kpanic("ext_glBlendFuncSeparateIndexedAMD is NULL");
    {
    GL_FUNC(ext_glBlendFuncSeparateIndexedAMD)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glBlendFuncSeparateIndexedAMD GLuint buf=%d, GLenum srcRGB=%d, GLenum dstRGB=%d, GLenum srcAlpha=%d, GLenum dstAlpha=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBlendFuncSeparatei(CPU* cpu) {
    if (!ext_glBlendFuncSeparatei)
        kpanic("ext_glBlendFuncSeparatei is NULL");
    {
    GL_FUNC(ext_glBlendFuncSeparatei)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glBlendFuncSeparatei GLuint buf=%d, GLenum srcRGB=%d, GLenum dstRGB=%d, GLenum srcAlpha=%d, GLenum dstAlpha=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBlendFuncSeparateiARB(CPU* cpu) {
    if (!ext_glBlendFuncSeparateiARB)
        kpanic("ext_glBlendFuncSeparateiARB is NULL");
    {
    GL_FUNC(ext_glBlendFuncSeparateiARB)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glBlendFuncSeparateiARB GLuint buf=%d, GLenum srcRGB=%d, GLenum dstRGB=%d, GLenum srcAlpha=%d, GLenum dstAlpha=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glBlendFunci(CPU* cpu) {
    if (!ext_glBlendFunci)
        kpanic("ext_glBlendFunci is NULL");
    {
    GL_FUNC(ext_glBlendFunci)(ARG1, ARG2, ARG3);
    GL_LOG ("glBlendFunci GLuint buf=%d, GLenum src=%d, GLenum dst=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBlendFunciARB(CPU* cpu) {
    if (!ext_glBlendFunciARB)
        kpanic("ext_glBlendFunciARB is NULL");
    {
    GL_FUNC(ext_glBlendFunciARB)(ARG1, ARG2, ARG3);
    GL_LOG ("glBlendFunciARB GLuint buf=%d, GLenum src=%d, GLenum dst=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBlendParameteriNV(CPU* cpu) {
    if (!ext_glBlendParameteriNV)
        kpanic("ext_glBlendParameteriNV is NULL");
    {
    GL_FUNC(ext_glBlendParameteriNV)(ARG1, ARG2);
    GL_LOG ("glBlendParameteriNV GLenum pname=%d, GLint value=%d",ARG1,ARG2);
    }
}
void glcommon_glBlitFramebuffer(CPU* cpu) {
    if (!ext_glBlitFramebuffer)
        kpanic("ext_glBlitFramebuffer is NULL");
    {
    GL_FUNC(ext_glBlitFramebuffer)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10);
    GL_LOG ("glBlitFramebuffer GLint srcX0=%d, GLint srcY0=%d, GLint srcX1=%d, GLint srcY1=%d, GLint dstX0=%d, GLint dstY0=%d, GLint dstX1=%d, GLint dstY1=%d, GLbitfield mask=%d, GLenum filter=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glBlitFramebufferEXT(CPU* cpu) {
    if (!ext_glBlitFramebufferEXT)
        kpanic("ext_glBlitFramebufferEXT is NULL");
    {
    GL_FUNC(ext_glBlitFramebufferEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10);
    GL_LOG ("glBlitFramebufferEXT GLint srcX0=%d, GLint srcY0=%d, GLint srcX1=%d, GLint srcY1=%d, GLint dstX0=%d, GLint dstY0=%d, GLint dstX1=%d, GLint dstY1=%d, GLbitfield mask=%d, GLenum filter=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glBlitNamedFramebuffer(CPU* cpu) {
    if (!ext_glBlitNamedFramebuffer)
        kpanic("ext_glBlitNamedFramebuffer is NULL");
    {
    GL_FUNC(ext_glBlitNamedFramebuffer)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12);
    GL_LOG ("glBlitNamedFramebuffer GLuint readFramebuffer=%d, GLuint drawFramebuffer=%d, GLint srcX0=%d, GLint srcY0=%d, GLint srcX1=%d, GLint srcY1=%d, GLint dstX0=%d, GLint dstY0=%d, GLint dstX1=%d, GLint dstY1=%d, GLbitfield mask=%d, GLenum filter=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12);
    }
}
void glcommon_glBufferAddressRangeNV(CPU* cpu) {
    if (!ext_glBufferAddressRangeNV)
        kpanic("ext_glBufferAddressRangeNV is NULL");
    {
    GL_FUNC(ext_glBufferAddressRangeNV)(ARG1, ARG2, llARG3, ARG4);
    GL_LOG ("glBufferAddressRangeNV GLenum pname=%d, GLuint index=%d, GLuint64EXT address=" PRIu64 ", GLsizeiptr length=%d",ARG1,ARG2,llARG3,ARG4);
    }
}
void glcommon_glBufferData(CPU* cpu) {
    if (!ext_glBufferData)
        kpanic("ext_glBufferData is NULL");
    {
    GL_FUNC(ext_glBufferData)(ARG1, ARG2, marshalArray<GLubyte>(cpu, ARG3, ARG2), ARG4);
    GL_LOG ("glBufferData GLenum target=%d, GLsizeiptr size=%d, const void* data=%.08x, GLenum usage=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBufferDataARB(CPU* cpu) {
    if (!ext_glBufferDataARB)
        kpanic("ext_glBufferDataARB is NULL");
    {
    GL_FUNC(ext_glBufferDataARB)(ARG1, ARG2, marshalArray<GLubyte>(cpu, ARG3, ARG2), ARG4);
    GL_LOG ("glBufferDataARB GLenum target=%d, GLsizeiptrARB size=%d, const void* data=%.08x, GLenum usage=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBufferPageCommitmentARB(CPU* cpu) {
    if (!ext_glBufferPageCommitmentARB)
        kpanic("ext_glBufferPageCommitmentARB is NULL");
    {
    GL_FUNC(ext_glBufferPageCommitmentARB)(ARG1, ARG2, ARG3, bARG4);
    GL_LOG ("glBufferPageCommitmentARB GLenum target=%d, GLintptr offset=%d, GLsizeiptr size=%d, GLboolean commit=%d",ARG1,ARG2,ARG3,bARG4);
    }
}
void glcommon_glBufferParameteriAPPLE(CPU* cpu) {
    if (!ext_glBufferParameteriAPPLE)
        kpanic("ext_glBufferParameteriAPPLE is NULL");
    {
    GL_FUNC(ext_glBufferParameteriAPPLE)(ARG1, ARG2, ARG3);
    GL_LOG ("glBufferParameteriAPPLE GLenum target=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glBufferRegionEnabled(CPU* cpu) {
    if (!ext_glBufferRegionEnabled)
        kpanic("ext_glBufferRegionEnabled is NULL");
    {
    EAX=GL_FUNC(ext_glBufferRegionEnabled)();
    GL_LOG ("glBufferRegionEnabled");
    }
}
void glcommon_glBufferStorage(CPU* cpu) {
    if (!ext_glBufferStorage)
        kpanic("ext_glBufferStorage is NULL");
    {
    GL_FUNC(ext_glBufferStorage)(ARG1, ARG2, marshalArray<GLubyte>(cpu, ARG3, ARG2), ARG4);
    GL_LOG ("glBufferStorage GLenum target=%d, GLsizeiptr size=%d, const void* data=%.08x, GLbitfield flags=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBufferSubData(CPU* cpu) {
    if (!ext_glBufferSubData)
        kpanic("ext_glBufferSubData is NULL");
    {
    GL_FUNC(ext_glBufferSubData)(ARG1, ARG2, ARG3, marshalArray<GLubyte>(cpu, ARG4, ARG3));
    GL_LOG ("glBufferSubData GLenum target=%d, GLintptr offset=%d, GLsizeiptr size=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glBufferSubDataARB(CPU* cpu) {
    if (!ext_glBufferSubDataARB)
        kpanic("ext_glBufferSubDataARB is NULL");
    {
    GL_FUNC(ext_glBufferSubDataARB)(ARG1, ARG2, ARG3, marshalArray<GLubyte>(cpu, ARG4, ARG3));
    GL_LOG ("glBufferSubDataARB GLenum target=%d, GLintptrARB offset=%d, GLsizeiptrARB size=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glCallCommandListNV(CPU* cpu) {
    if (!ext_glCallCommandListNV)
        kpanic("ext_glCallCommandListNV is NULL");
    {
    GL_FUNC(ext_glCallCommandListNV)(ARG1);
    GL_LOG ("glCallCommandListNV GLuint list=%d",ARG1);
    }
}
void glcommon_glCheckFramebufferStatus(CPU* cpu) {
    if (!ext_glCheckFramebufferStatus)
        kpanic("ext_glCheckFramebufferStatus is NULL");
    {
    EAX=GL_FUNC(ext_glCheckFramebufferStatus)(ARG1);
    GL_LOG ("glCheckFramebufferStatus GLenum target=%d",ARG1);
    }
}
void glcommon_glCheckFramebufferStatusEXT(CPU* cpu) {
    if (!ext_glCheckFramebufferStatusEXT)
        kpanic("ext_glCheckFramebufferStatusEXT is NULL");
    {
    EAX=GL_FUNC(ext_glCheckFramebufferStatusEXT)(ARG1);
    GL_LOG ("glCheckFramebufferStatusEXT GLenum target=%d",ARG1);
    }
}
void glcommon_glCheckNamedFramebufferStatus(CPU* cpu) {
    if (!ext_glCheckNamedFramebufferStatus)
        kpanic("ext_glCheckNamedFramebufferStatus is NULL");
    {
    EAX=GL_FUNC(ext_glCheckNamedFramebufferStatus)(ARG1, ARG2);
    GL_LOG ("glCheckNamedFramebufferStatus GLuint framebuffer=%d, GLenum target=%d",ARG1,ARG2);
    }
}
void glcommon_glCheckNamedFramebufferStatusEXT(CPU* cpu) {
    if (!ext_glCheckNamedFramebufferStatusEXT)
        kpanic("ext_glCheckNamedFramebufferStatusEXT is NULL");
    {
    EAX=GL_FUNC(ext_glCheckNamedFramebufferStatusEXT)(ARG1, ARG2);
    GL_LOG ("glCheckNamedFramebufferStatusEXT GLuint framebuffer=%d, GLenum target=%d",ARG1,ARG2);
    }
}
void glcommon_glClampColor(CPU* cpu) {
    if (!ext_glClampColor)
        kpanic("ext_glClampColor is NULL");
    {
    GL_FUNC(ext_glClampColor)(ARG1, ARG2);
    GL_LOG ("glClampColor GLenum target=%d, GLenum clamp=%d",ARG1,ARG2);
    }
}
void glcommon_glClampColorARB(CPU* cpu) {
    if (!ext_glClampColorARB)
        kpanic("ext_glClampColorARB is NULL");
    {
    GL_FUNC(ext_glClampColorARB)(ARG1, ARG2);
    GL_LOG ("glClampColorARB GLenum target=%d, GLenum clamp=%d",ARG1,ARG2);
    }
}
void glcommon_glClearAccumxOES(CPU* cpu) {
    if (!ext_glClearAccumxOES)
        kpanic("ext_glClearAccumxOES is NULL");
    {
    GL_FUNC(ext_glClearAccumxOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glClearAccumxOES GLfixed red=%d, GLfixed green=%d, GLfixed blue=%d, GLfixed alpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glClearBufferData(CPU* cpu) {
    if (!ext_glClearBufferData)
        kpanic("ext_glClearBufferData is NULL");
    {
    GL_FUNC(ext_glClearBufferData)(ARG1, ARG2, ARG3, ARG4, marshalPixel(cpu, ARG3, ARG4, ARG5));
    GL_LOG ("glClearBufferData GLenum target=%d, GLenum internalformat=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glClearBufferSubData(CPU* cpu) {
    if (!ext_glClearBufferSubData)
        kpanic("ext_glClearBufferSubData is NULL");
    {
    GL_FUNC(ext_glClearBufferSubData)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, marshalPixel(cpu, ARG5, ARG6, ARG7));
    GL_LOG ("glClearBufferSubData GLenum target=%d, GLenum internalformat=%d, GLintptr offset=%d, GLsizeiptr size=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glClearBufferfi(CPU* cpu) {
    if (!ext_glClearBufferfi)
        kpanic("ext_glClearBufferfi is NULL");
    {
    GL_FUNC(ext_glClearBufferfi)(ARG1, ARG2, fARG3, ARG4);
    GL_LOG ("glClearBufferfi GLenum buffer=%d, GLint drawbuffer=%d, GLfloat depth=%f, GLint stencil=%d",ARG1,ARG2,fARG3,ARG4);
    }
}
void glcommon_glClearBufferfv(CPU* cpu) {
    if (!ext_glClearBufferfv)
        kpanic("ext_glClearBufferfv is NULL");
    {
    GL_FUNC(ext_glClearBufferfv)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, 4));
    GL_LOG ("glClearBufferfv GLenum buffer=%d, GLint drawbuffer=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glClearBufferiv(CPU* cpu) {
    if (!ext_glClearBufferiv)
        kpanic("ext_glClearBufferiv is NULL");
    {
    GL_FUNC(ext_glClearBufferiv)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, 4));
    GL_LOG ("glClearBufferiv GLenum buffer=%d, GLint drawbuffer=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glClearBufferuiv(CPU* cpu) {
    if (!ext_glClearBufferuiv)
        kpanic("ext_glClearBufferuiv is NULL");
    {
    GL_FUNC(ext_glClearBufferuiv)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, 4));
    GL_LOG ("glClearBufferuiv GLenum buffer=%d, GLint drawbuffer=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glClearColorIiEXT(CPU* cpu) {
    if (!ext_glClearColorIiEXT)
        kpanic("ext_glClearColorIiEXT is NULL");
    {
    GL_FUNC(ext_glClearColorIiEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glClearColorIiEXT GLint red=%d, GLint green=%d, GLint blue=%d, GLint alpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glClearColorIuiEXT(CPU* cpu) {
    if (!ext_glClearColorIuiEXT)
        kpanic("ext_glClearColorIuiEXT is NULL");
    {
    GL_FUNC(ext_glClearColorIuiEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glClearColorIuiEXT GLuint red=%d, GLuint green=%d, GLuint blue=%d, GLuint alpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glClearColorxOES(CPU* cpu) {
    if (!ext_glClearColorxOES)
        kpanic("ext_glClearColorxOES is NULL");
    {
    GL_FUNC(ext_glClearColorxOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glClearColorxOES GLfixed red=%d, GLfixed green=%d, GLfixed blue=%d, GLfixed alpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glClearDepthdNV(CPU* cpu) {
    if (!ext_glClearDepthdNV)
        kpanic("ext_glClearDepthdNV is NULL");
    {
    GL_FUNC(ext_glClearDepthdNV)(dARG1);
    GL_LOG ("glClearDepthdNV GLdouble depth=%f",dARG1);
    }
}
void glcommon_glClearDepthf(CPU* cpu) {
    if (!ext_glClearDepthf)
        kpanic("ext_glClearDepthf is NULL");
    {
    GL_FUNC(ext_glClearDepthf)(fARG1);
    GL_LOG ("glClearDepthf GLfloat d=%f",fARG1);
    }
}
void glcommon_glClearDepthfOES(CPU* cpu) {
    if (!ext_glClearDepthfOES)
        kpanic("ext_glClearDepthfOES is NULL");
    {
    GL_FUNC(ext_glClearDepthfOES)(fARG1);
    GL_LOG ("glClearDepthfOES GLclampf depth=%f",fARG1);
    }
}
void glcommon_glClearDepthxOES(CPU* cpu) {
    if (!ext_glClearDepthxOES)
        kpanic("ext_glClearDepthxOES is NULL");
    {
    GL_FUNC(ext_glClearDepthxOES)(ARG1);
    GL_LOG ("glClearDepthxOES GLfixed depth=%d",ARG1);
    }
}
void glcommon_glClearNamedBufferData(CPU* cpu) {
    if (!ext_glClearNamedBufferData)
        kpanic("ext_glClearNamedBufferData is NULL");
    {
    GL_FUNC(ext_glClearNamedBufferData)(ARG1, ARG2, ARG3, ARG4, marshalPixel(cpu, ARG3, ARG4, ARG5));
    GL_LOG ("glClearNamedBufferData GLuint buffer=%d, GLenum internalformat=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glClearNamedBufferDataEXT(CPU* cpu) {
    if (!ext_glClearNamedBufferDataEXT)
        kpanic("ext_glClearNamedBufferDataEXT is NULL");
    {
    GL_FUNC(ext_glClearNamedBufferDataEXT)(ARG1, ARG2, ARG3, ARG4, marshalPixel(cpu, ARG3, ARG4, ARG5));
    GL_LOG ("glClearNamedBufferDataEXT GLuint buffer=%d, GLenum internalformat=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glClearNamedBufferSubData(CPU* cpu) {
    if (!ext_glClearNamedBufferSubData)
        kpanic("ext_glClearNamedBufferSubData is NULL");
    {
    GL_FUNC(ext_glClearNamedBufferSubData)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, marshalPixel(cpu, ARG5, ARG6, ARG7));
    GL_LOG ("glClearNamedBufferSubData GLuint buffer=%d, GLenum internalformat=%d, GLintptr offset=%d, GLsizeiptr size=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glClearNamedBufferSubDataEXT(CPU* cpu) {
    if (!ext_glClearNamedBufferSubDataEXT)
        kpanic("ext_glClearNamedBufferSubDataEXT is NULL");
    {
    GL_FUNC(ext_glClearNamedBufferSubDataEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, marshalPixel(cpu, ARG5, ARG6, ARG7));
    GL_LOG ("glClearNamedBufferSubDataEXT GLuint buffer=%d, GLenum internalformat=%d, GLsizeiptr offset=%d, GLsizeiptr size=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glClearNamedFramebufferfi(CPU* cpu) {
    if (!ext_glClearNamedFramebufferfi)
        kpanic("ext_glClearNamedFramebufferfi is NULL");
    {
    GL_FUNC(ext_glClearNamedFramebufferfi)(ARG1, ARG2, ARG3, fARG4, ARG5);
    GL_LOG ("glClearNamedFramebufferfi GLuint framebuffer=%d, GLenum buffer=%d, GLint drawbuffer=%d, GLfloat depth=%f, GLint stencil=%d",ARG1,ARG2,ARG3,fARG4,ARG5);
    }
}
void glcommon_glClearNamedFramebufferfv(CPU* cpu) {
    if (!ext_glClearNamedFramebufferfv)
        kpanic("ext_glClearNamedFramebufferfv is NULL");
    {
    GL_FUNC(ext_glClearNamedFramebufferfv)(ARG1, ARG2, ARG3, marshalArray<GLfloat>(cpu, ARG4, 4));
    GL_LOG ("glClearNamedFramebufferfv GLuint framebuffer=%d, GLenum buffer=%d, GLint drawbuffer=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glClearNamedFramebufferiv(CPU* cpu) {
    if (!ext_glClearNamedFramebufferiv)
        kpanic("ext_glClearNamedFramebufferiv is NULL");
    {
    GL_FUNC(ext_glClearNamedFramebufferiv)(ARG1, ARG2, ARG3, marshalArray<GLint>(cpu, ARG4, 4));
    GL_LOG ("glClearNamedFramebufferiv GLuint framebuffer=%d, GLenum buffer=%d, GLint drawbuffer=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glClearNamedFramebufferuiv(CPU* cpu) {
    if (!ext_glClearNamedFramebufferuiv)
        kpanic("ext_glClearNamedFramebufferuiv is NULL");
    {
    GL_FUNC(ext_glClearNamedFramebufferuiv)(ARG1, ARG2, ARG3, marshalArray<GLuint>(cpu, ARG4, 4));
    GL_LOG ("glClearNamedFramebufferuiv GLuint framebuffer=%d, GLenum buffer=%d, GLint drawbuffer=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glClearTexImage(CPU* cpu) {
    if (!ext_glClearTexImage)
        kpanic("ext_glClearTexImage is NULL");
    {
    GL_FUNC(ext_glClearTexImage)(ARG1, ARG2, ARG3, ARG4, marshalPixel(cpu, ARG3, ARG4, ARG5));
    GL_LOG ("glClearTexImage GLuint texture=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glClearTexSubImage(CPU* cpu) {
    if (!ext_glClearTexSubImage)
        kpanic("ext_glClearTexSubImage is NULL");
    {
    GL_FUNC(ext_glClearTexSubImage)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, marshalPixel(cpu, ARG9, ARG10, ARG11));
    GL_LOG ("glClearTexSubImage GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glClientActiveTexture(CPU* cpu) {
    if (!ext_glClientActiveTexture)
        kpanic("ext_glClientActiveTexture is NULL");
    {
    GL_FUNC(ext_glClientActiveTexture)(ARG1);
    GL_LOG ("glClientActiveTexture GLenum texture=%d",ARG1);
    }
}
void glcommon_glClientActiveTextureARB(CPU* cpu) {
    if (!ext_glClientActiveTextureARB)
        kpanic("ext_glClientActiveTextureARB is NULL");
    {
    GL_FUNC(ext_glClientActiveTextureARB)(ARG1);
    GL_LOG ("glClientActiveTextureARB GLenum texture=%d",ARG1);
    }
}
void glcommon_glClientActiveVertexStreamATI(CPU* cpu) {
    if (!ext_glClientActiveVertexStreamATI)
        kpanic("ext_glClientActiveVertexStreamATI is NULL");
    {
    GL_FUNC(ext_glClientActiveVertexStreamATI)(ARG1);
    GL_LOG ("glClientActiveVertexStreamATI GLenum stream=%d",ARG1);
    }
}
void glcommon_glClientAttribDefaultEXT(CPU* cpu) {
    if (!ext_glClientAttribDefaultEXT)
        kpanic("ext_glClientAttribDefaultEXT is NULL");
    {
    GL_FUNC(ext_glClientAttribDefaultEXT)(ARG1);
    GL_LOG ("glClientAttribDefaultEXT GLbitfield mask=%d",ARG1);
    }
}
void glcommon_glClientWaitSync(CPU* cpu) {
    if (!ext_glClientWaitSync)
        kpanic("ext_glClientWaitSync is NULL");
    {
    EAX=GL_FUNC(ext_glClientWaitSync)(marshalSync(cpu, ARG1), ARG2, llARG3);
    GL_LOG ("glClientWaitSync GLsync sync=%d, GLbitfield flags=%d, GLuint64 timeout=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glClipControl(CPU* cpu) {
    if (!ext_glClipControl)
        kpanic("ext_glClipControl is NULL");
    {
    GL_FUNC(ext_glClipControl)(ARG1, ARG2);
    GL_LOG ("glClipControl GLenum origin=%d, GLenum depth=%d",ARG1,ARG2);
    }
}
void glcommon_glClipPlanefOES(CPU* cpu) {
    if (!ext_glClipPlanefOES)
        kpanic("ext_glClipPlanefOES is NULL");
    {
    GL_FUNC(ext_glClipPlanefOES)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 4));
    GL_LOG ("glClipPlanefOES GLenum plane=%d, const GLfloat* equation=%.08x",ARG1,ARG2);
    }
}
void glcommon_glClipPlanexOES(CPU* cpu) {
    if (!ext_glClipPlanexOES)
        kpanic("ext_glClipPlanexOES is NULL");
    {
    GL_FUNC(ext_glClipPlanexOES)(ARG1, marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glClipPlanexOES GLenum plane=%d, const GLfixed* equation=%.08x",ARG1,ARG2);
    }
}
void glcommon_glColor3fVertex3fSUN(CPU* cpu) {
    if (!ext_glColor3fVertex3fSUN)
        kpanic("ext_glColor3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glColor3fVertex3fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glColor3fVertex3fSUN GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glColor3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glColor3fVertex3fvSUN)
        kpanic("ext_glColor3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glColor3fVertex3fvSUN)(marshalArray<GLfloat>(cpu, ARG1, 3), marshalArray<GLfloat>(cpu, ARG2, 3));
    GL_LOG ("glColor3fVertex3fvSUN const GLfloat* c=%.08x, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glColor3hNV(CPU* cpu) {
    if (!ext_glColor3hNV)
        kpanic("ext_glColor3hNV is NULL");
    {
    GL_FUNC(ext_glColor3hNV)(sARG1, sARG2, sARG3);
    GL_LOG ("glColor3hNV GLhalfNV red=%d, GLhalfNV green=%d, GLhalfNV blue=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glColor3hvNV(CPU* cpu) {
    if (!ext_glColor3hvNV)
        kpanic("ext_glColor3hvNV is NULL");
    {
    GL_FUNC(ext_glColor3hvNV)(marshalArray<GLushort>(cpu, ARG1, 3));
    GL_LOG ("glColor3hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glColor3xOES(CPU* cpu) {
    if (!ext_glColor3xOES)
        kpanic("ext_glColor3xOES is NULL");
    {
    GL_FUNC(ext_glColor3xOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glColor3xOES GLfixed red=%d, GLfixed green=%d, GLfixed blue=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColor3xvOES(CPU* cpu) {
    if (!ext_glColor3xvOES)
        kpanic("ext_glColor3xvOES is NULL");
    {
    GL_FUNC(ext_glColor3xvOES)(marshalArray<GLint>(cpu, ARG1, 3));
    GL_LOG ("glColor3xvOES const GLfixed* components=%.08x",ARG1);
    }
}
void glcommon_glColor4fNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glColor4fNormal3fVertex3fSUN)
        kpanic("ext_glColor4fNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glColor4fNormal3fVertex3fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8, fARG9, fARG10);
    GL_LOG ("glColor4fNormal3fVertex3fSUN GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat a=%f, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8,fARG9,fARG10);
    }
}
void glcommon_glColor4fNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glColor4fNormal3fVertex3fvSUN)
        kpanic("ext_glColor4fNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glColor4fNormal3fVertex3fvSUN)(marshalArray<GLfloat>(cpu, ARG1, 4), marshalArray<GLfloat>(cpu, ARG2, 3), marshalArray<GLfloat>(cpu, ARG3, 3));
    GL_LOG ("glColor4fNormal3fVertex3fvSUN const GLfloat* c=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColor4hNV(CPU* cpu) {
    if (!ext_glColor4hNV)
        kpanic("ext_glColor4hNV is NULL");
    {
    GL_FUNC(ext_glColor4hNV)(sARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glColor4hNV GLhalfNV red=%d, GLhalfNV green=%d, GLhalfNV blue=%d, GLhalfNV alpha=%d",sARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glColor4hvNV(CPU* cpu) {
    if (!ext_glColor4hvNV)
        kpanic("ext_glColor4hvNV is NULL");
    {
    GL_FUNC(ext_glColor4hvNV)(marshalArray<GLushort>(cpu, ARG1, 4));
    GL_LOG ("glColor4hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glColor4ubVertex2fSUN(CPU* cpu) {
    if (!ext_glColor4ubVertex2fSUN)
        kpanic("ext_glColor4ubVertex2fSUN is NULL");
    {
    GL_FUNC(ext_glColor4ubVertex2fSUN)(bARG1, bARG2, bARG3, bARG4, fARG5, fARG6);
    GL_LOG ("glColor4ubVertex2fSUN GLubyte r=%d, GLubyte g=%d, GLubyte b=%d, GLubyte a=%d, GLfloat x=%f, GLfloat y=%f",bARG1,bARG2,bARG3,bARG4,fARG5,fARG6);
    }
}
void glcommon_glColor4ubVertex2fvSUN(CPU* cpu) {
    if (!ext_glColor4ubVertex2fvSUN)
        kpanic("ext_glColor4ubVertex2fvSUN is NULL");
    {
    GL_FUNC(ext_glColor4ubVertex2fvSUN)(marshalArray<GLubyte>(cpu, ARG1, 4), marshalArray<GLfloat>(cpu, ARG2, 2));
    GL_LOG ("glColor4ubVertex2fvSUN const GLubyte* c=%.08x, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glColor4ubVertex3fSUN(CPU* cpu) {
    if (!ext_glColor4ubVertex3fSUN)
        kpanic("ext_glColor4ubVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glColor4ubVertex3fSUN)(bARG1, bARG2, bARG3, bARG4, fARG5, fARG6, fARG7);
    GL_LOG ("glColor4ubVertex3fSUN GLubyte r=%d, GLubyte g=%d, GLubyte b=%d, GLubyte a=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",bARG1,bARG2,bARG3,bARG4,fARG5,fARG6,fARG7);
    }
}
void glcommon_glColor4ubVertex3fvSUN(CPU* cpu) {
    if (!ext_glColor4ubVertex3fvSUN)
        kpanic("ext_glColor4ubVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glColor4ubVertex3fvSUN)(marshalArray<GLubyte>(cpu, ARG1, 4), marshalArray<GLfloat>(cpu, ARG2, 3));
    GL_LOG ("glColor4ubVertex3fvSUN const GLubyte* c=%.08x, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glColor4xOES(CPU* cpu) {
    if (!ext_glColor4xOES)
        kpanic("ext_glColor4xOES is NULL");
    {
    GL_FUNC(ext_glColor4xOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glColor4xOES GLfixed red=%d, GLfixed green=%d, GLfixed blue=%d, GLfixed alpha=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glColor4xvOES(CPU* cpu) {
    if (!ext_glColor4xvOES)
        kpanic("ext_glColor4xvOES is NULL");
    {
    GL_FUNC(ext_glColor4xvOES)(marshalArray<GLint>(cpu, ARG1, 4));
    GL_LOG ("glColor4xvOES const GLfixed* components=%.08x",ARG1);
    }
}
void glcommon_glColorFormatNV(CPU* cpu) {
    if (!ext_glColorFormatNV)
        kpanic("ext_glColorFormatNV is NULL");
    {
    GL_FUNC(ext_glColorFormatNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glColorFormatNV GLint size=%d, GLenum type=%d, GLsizei stride=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColorFragmentOp1ATI(CPU* cpu) {
    if (!ext_glColorFragmentOp1ATI)
        kpanic("ext_glColorFragmentOp1ATI is NULL");
    {
    GL_FUNC(ext_glColorFragmentOp1ATI)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glColorFragmentOp1ATI GLenum op=%d, GLuint dst=%d, GLuint dstMask=%d, GLuint dstMod=%d, GLuint arg1=%d, GLuint arg1Rep=%d, GLuint arg1Mod=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glColorFragmentOp2ATI(CPU* cpu) {
    if (!ext_glColorFragmentOp2ATI)
        kpanic("ext_glColorFragmentOp2ATI is NULL");
    {
    GL_FUNC(ext_glColorFragmentOp2ATI)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10);
    GL_LOG ("glColorFragmentOp2ATI GLenum op=%d, GLuint dst=%d, GLuint dstMask=%d, GLuint dstMod=%d, GLuint arg1=%d, GLuint arg1Rep=%d, GLuint arg1Mod=%d, GLuint arg2=%d, GLuint arg2Rep=%d, GLuint arg2Mod=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glColorFragmentOp3ATI(CPU* cpu) {
    if (!ext_glColorFragmentOp3ATI)
        kpanic("ext_glColorFragmentOp3ATI is NULL");
    {
    GL_FUNC(ext_glColorFragmentOp3ATI)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13);
    GL_LOG ("glColorFragmentOp3ATI GLenum op=%d, GLuint dst=%d, GLuint dstMask=%d, GLuint dstMod=%d, GLuint arg1=%d, GLuint arg1Rep=%d, GLuint arg1Mod=%d, GLuint arg2=%d, GLuint arg2Rep=%d, GLuint arg2Mod=%d, GLuint arg3=%d, GLuint arg3Rep=%d, GLuint arg3Mod=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12,ARG13);
    }
}
void glcommon_glColorMaskIndexedEXT(CPU* cpu) {
    if (!ext_glColorMaskIndexedEXT)
        kpanic("ext_glColorMaskIndexedEXT is NULL");
    {
    GL_FUNC(ext_glColorMaskIndexedEXT)(ARG1, bARG2, bARG3, bARG4, bARG5);
    GL_LOG ("glColorMaskIndexedEXT GLuint index=%d, GLboolean r=%d, GLboolean g=%d, GLboolean b=%d, GLboolean a=%d",ARG1,bARG2,bARG3,bARG4,bARG5);
    }
}
void glcommon_glColorMaski(CPU* cpu) {
    if (!ext_glColorMaski)
        kpanic("ext_glColorMaski is NULL");
    {
    GL_FUNC(ext_glColorMaski)(ARG1, bARG2, bARG3, bARG4, bARG5);
    GL_LOG ("glColorMaski GLuint index=%d, GLboolean r=%d, GLboolean g=%d, GLboolean b=%d, GLboolean a=%d",ARG1,bARG2,bARG3,bARG4,bARG5);
    }
}
void glcommon_glColorP3ui(CPU* cpu) {
    if (!ext_glColorP3ui)
        kpanic("ext_glColorP3ui is NULL");
    {
    GL_FUNC(ext_glColorP3ui)(ARG1, ARG2);
    GL_LOG ("glColorP3ui GLenum type=%d, GLuint color=%d",ARG1,ARG2);
    }
}
void glcommon_glColorP3uiv(CPU* cpu) {
    if (!ext_glColorP3uiv)
        kpanic("ext_glColorP3uiv is NULL");
    {
    GL_FUNC(ext_glColorP3uiv)(ARG1, marshalArray<GLuint>(cpu, ARG2, 1));
    GL_LOG ("glColorP3uiv GLenum type=%d, const GLuint* color=%.08x",ARG1,ARG2);
    }
}
void glcommon_glColorP4ui(CPU* cpu) {
    if (!ext_glColorP4ui)
        kpanic("ext_glColorP4ui is NULL");
    {
    GL_FUNC(ext_glColorP4ui)(ARG1, ARG2);
    GL_LOG ("glColorP4ui GLenum type=%d, GLuint color=%d",ARG1,ARG2);
    }
}
void glcommon_glColorP4uiv(CPU* cpu) {
    if (!ext_glColorP4uiv)
        kpanic("ext_glColorP4uiv is NULL");
    {
    GL_FUNC(ext_glColorP4uiv)(ARG1, marshalArray<GLuint>(cpu, ARG2, 1));
    GL_LOG ("glColorP4uiv GLenum type=%d, const GLuint* color=%.08x",ARG1,ARG2);
    }
}
void glcommon_glColorPointerEXT(CPU* cpu) {
    if (!ext_glColorPointerEXT)
        kpanic("ext_glColorPointerEXT is NULL");
    {
    GL_FUNC(ext_glColorPointerEXT)(ARG1, ARG2, ARG3, ARG4, marshalColorPointer(cpu, ARG1, ARG2, ARG3, ARG4));
    GL_LOG ("glColorPointerEXT GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLsizei count=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glColorPointerListIBM(CPU* cpu) {
    if (!ext_glColorPointerListIBM)
        kpanic("ext_glColorPointerListIBM is NULL");
    {
    GL_FUNC(ext_glColorPointerListIBM)(ARG1, ARG2, ARG3, (const void**)marshalunhandled("glColorPointerListIBM", "pointer", cpu, ARG4), ARG5);
    GL_LOG ("glColorPointerListIBM GLint size=%d, GLenum type=%d, GLint stride=%d, const void** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glColorPointervINTEL(CPU* cpu) {
    if (!ext_glColorPointervINTEL)
        kpanic("ext_glColorPointervINTEL is NULL");
    {
    GL_FUNC(ext_glColorPointervINTEL)(ARG1, ARG2, (const void**)marshalpp(cpu, ARG3, 4, 0, ARG1*getDataSize(ARG2), 0));
    GL_LOG ("glColorPointervINTEL GLint size=%d, GLenum type=%d, const void** pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColorSubTable(CPU* cpu) {
    if (!ext_glColorSubTable)
        kpanic("ext_glColorSubTable is NULL");
    {
    GL_FUNC(ext_glColorSubTable)(ARG1, ARG2, ARG3, ARG4, ARG5, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, components_in_format(ARG4)*ARG3, ARG6));
    GL_LOG ("glColorSubTable GLenum target=%d, GLsizei start=%d, GLsizei count=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glColorSubTableEXT(CPU* cpu) {
    if (!ext_glColorSubTableEXT)
        kpanic("ext_glColorSubTableEXT is NULL");
    {
    GL_FUNC(ext_glColorSubTableEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, marshalType(cpu, ARG5, components_in_format(ARG4)*ARG3, ARG6));
    GL_LOG ("glColorSubTableEXT GLenum target=%d, GLsizei start=%d, GLsizei count=%d, GLenum format=%d, GLenum type=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glColorTable(CPU* cpu) {
    if (!ext_glColorTable)
        kpanic("ext_glColorTable is NULL");
    {
    GL_FUNC(ext_glColorTable)(ARG1, ARG2, ARG3, ARG4, ARG5, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, components_in_format(ARG4)*ARG3, ARG6));
    GL_LOG ("glColorTable GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* table=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glColorTableEXT(CPU* cpu) {
    if (!ext_glColorTableEXT)
        kpanic("ext_glColorTableEXT is NULL");
    {
    GL_FUNC(ext_glColorTableEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, components_in_format(ARG4)*ARG3, ARG6));
    GL_LOG ("glColorTableEXT GLenum target=%d, GLenum internalFormat=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* table=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glColorTableParameterfv(CPU* cpu) {
    if (!ext_glColorTableParameterfv)
        kpanic("ext_glColorTableParameterfv is NULL");
    {
    GL_FUNC(ext_glColorTableParameterfv)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, 4));
    GL_LOG ("glColorTableParameterfv GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColorTableParameterfvSGI(CPU* cpu) {
    if (!ext_glColorTableParameterfvSGI)
        kpanic("ext_glColorTableParameterfvSGI is NULL");
    {
    GL_FUNC(ext_glColorTableParameterfvSGI)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, 4));
    GL_LOG ("glColorTableParameterfvSGI GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColorTableParameteriv(CPU* cpu) {
    if (!ext_glColorTableParameteriv)
        kpanic("ext_glColorTableParameteriv is NULL");
    {
    GL_FUNC(ext_glColorTableParameteriv)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, 4));
    GL_LOG ("glColorTableParameteriv GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColorTableParameterivSGI(CPU* cpu) {
    if (!ext_glColorTableParameterivSGI)
        kpanic("ext_glColorTableParameterivSGI is NULL");
    {
    GL_FUNC(ext_glColorTableParameterivSGI)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, 4));
    GL_LOG ("glColorTableParameterivSGI GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glColorTableSGI(CPU* cpu) {
    if (!ext_glColorTableSGI)
        kpanic("ext_glColorTableSGI is NULL");
    {
    GL_FUNC(ext_glColorTableSGI)(ARG1, ARG2, ARG3, ARG4, ARG5, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, components_in_format(ARG4)*ARG3, ARG6));
    GL_LOG ("glColorTableSGI GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* table=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glCombinerInputNV(CPU* cpu) {
    if (!ext_glCombinerInputNV)
        kpanic("ext_glCombinerInputNV is NULL");
    {
    GL_FUNC(ext_glCombinerInputNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glCombinerInputNV GLenum stage=%d, GLenum portion=%d, GLenum variable=%d, GLenum input=%d, GLenum mapping=%d, GLenum componentUsage=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glCombinerOutputNV(CPU* cpu) {
    if (!ext_glCombinerOutputNV)
        kpanic("ext_glCombinerOutputNV is NULL");
    {
    GL_FUNC(ext_glCombinerOutputNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, bARG8, bARG9, bARG10);
    GL_LOG ("glCombinerOutputNV GLenum stage=%d, GLenum portion=%d, GLenum abOutput=%d, GLenum cdOutput=%d, GLenum sumOutput=%d, GLenum scale=%d, GLenum bias=%d, GLboolean abDotProduct=%d, GLboolean cdDotProduct=%d, GLboolean muxSum=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,bARG8,bARG9,bARG10);
    }
}
void glcommon_glCombinerParameterfNV(CPU* cpu) {
    if (!ext_glCombinerParameterfNV)
        kpanic("ext_glCombinerParameterfNV is NULL");
    {
    GL_FUNC(ext_glCombinerParameterfNV)(ARG1, fARG2);
    GL_LOG ("glCombinerParameterfNV GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glCombinerParameterfvNV(CPU* cpu) {
    if (!ext_glCombinerParameterfvNV)
        kpanic("ext_glCombinerParameterfvNV is NULL");
    {
    GL_FUNC(ext_glCombinerParameterfvNV)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 4));
    GL_LOG ("glCombinerParameterfvNV GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCombinerParameteriNV(CPU* cpu) {
    if (!ext_glCombinerParameteriNV)
        kpanic("ext_glCombinerParameteriNV is NULL");
    {
    GL_FUNC(ext_glCombinerParameteriNV)(ARG1, ARG2);
    GL_LOG ("glCombinerParameteriNV GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glCombinerParameterivNV(CPU* cpu) {
    if (!ext_glCombinerParameterivNV)
        kpanic("ext_glCombinerParameterivNV is NULL");
    {
    GL_FUNC(ext_glCombinerParameterivNV)(ARG1, marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glCombinerParameterivNV GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCombinerStageParameterfvNV(CPU* cpu) {
    if (!ext_glCombinerStageParameterfvNV)
        kpanic("ext_glCombinerStageParameterfvNV is NULL");
    {
    GL_FUNC(ext_glCombinerStageParameterfvNV)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, 4));
    GL_LOG ("glCombinerStageParameterfvNV GLenum stage=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glCommandListSegmentsNV(CPU* cpu) {
    if (!ext_glCommandListSegmentsNV)
        kpanic("ext_glCommandListSegmentsNV is NULL");
    {
    GL_FUNC(ext_glCommandListSegmentsNV)(ARG1, ARG2);
    GL_LOG ("glCommandListSegmentsNV GLuint list=%d, GLuint segments=%d",ARG1,ARG2);
    }
}
void glcommon_glCompileCommandListNV(CPU* cpu) {
    if (!ext_glCompileCommandListNV)
        kpanic("ext_glCompileCommandListNV is NULL");
    {
    GL_FUNC(ext_glCompileCommandListNV)(ARG1);
    GL_LOG ("glCompileCommandListNV GLuint list=%d",ARG1);
    }
}
void glcommon_glCompileShader(CPU* cpu) {
    if (!ext_glCompileShader)
        kpanic("ext_glCompileShader is NULL");
    {
    GL_FUNC(ext_glCompileShader)(ARG1);
    GL_LOG ("glCompileShader GLuint shader=%d",ARG1);
    }
}
void glcommon_glCompileShaderARB(CPU* cpu) {
    if (!ext_glCompileShaderARB)
        kpanic("ext_glCompileShaderARB is NULL");
    {
    GL_FUNC(ext_glCompileShaderARB)(INDEX_TO_HANDLE(hARG1));
    GL_LOG ("glCompileShaderARB GLhandleARB shaderObj=%d",ARG1);
    }
}
void glcommon_glCompileShaderIncludeARB(CPU* cpu) {
    if (!ext_glCompileShaderIncludeARB)
        kpanic("ext_glCompileShaderIncludeARB is NULL");
    {
    GL_FUNC(ext_glCompileShaderIncludeARB)(ARG1, ARG2, (GLchar*const*)marshalpp(cpu, ARG3, ARG2, ARG4, -1, sizeof(GLchar)), marshalArray<GLint>(cpu, ARG4, ARG2));
    GL_LOG ("glCompileShaderIncludeARB GLuint shader=%d, GLsizei count=%d, const GLchar*const* path=%.08x, const GLint* length=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glCompressedMultiTexImage1DEXT(CPU* cpu) {
    if (!ext_glCompressedMultiTexImage1DEXT)
        kpanic("ext_glCompressedMultiTexImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedMultiTexImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG8:marshalArray<GLubyte>(cpu, ARG8, ARG7));
    GL_LOG ("glCompressedMultiTexImage1DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLint border=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCompressedMultiTexImage2DEXT(CPU* cpu) {
    if (!ext_glCompressedMultiTexImage2DEXT)
        kpanic("ext_glCompressedMultiTexImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedMultiTexImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedMultiTexImage2DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCompressedMultiTexImage3DEXT(CPU* cpu) {
    if (!ext_glCompressedMultiTexImage3DEXT)
        kpanic("ext_glCompressedMultiTexImage3DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedMultiTexImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG10:marshalArray<GLubyte>(cpu, ARG10, ARG9));
    GL_LOG ("glCompressedMultiTexImage3DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glCompressedMultiTexSubImage1DEXT(CPU* cpu) {
    if (!ext_glCompressedMultiTexSubImage1DEXT)
        kpanic("ext_glCompressedMultiTexSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedMultiTexSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG8:marshalArray<GLubyte>(cpu, ARG8, ARG7));
    GL_LOG ("glCompressedMultiTexSubImage1DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCompressedMultiTexSubImage2DEXT(CPU* cpu) {
    if (!ext_glCompressedMultiTexSubImage2DEXT)
        kpanic("ext_glCompressedMultiTexSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedMultiTexSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG10:marshalArray<GLubyte>(cpu, ARG10, ARG9));
    GL_LOG ("glCompressedMultiTexSubImage2DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glCompressedMultiTexSubImage3DEXT(CPU* cpu) {
    if (!ext_glCompressedMultiTexSubImage3DEXT)
        kpanic("ext_glCompressedMultiTexSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedMultiTexSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG12:marshalArray<GLubyte>(cpu, ARG12, ARG11));
    GL_LOG ("glCompressedMultiTexSubImage3DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12);
    }
}
void glcommon_glCompressedTexImage1D(CPU* cpu) {
    if (!ext_glCompressedTexImage1D)
        kpanic("ext_glCompressedTexImage1D is NULL");
    {
    GL_FUNC(ext_glCompressedTexImage1D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG7:marshalArray<GLubyte>(cpu, ARG7, ARG6));
    GL_LOG ("glCompressedTexImage1D GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLint border=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCompressedTexImage1DARB(CPU* cpu) {
    if (!ext_glCompressedTexImage1DARB)
        kpanic("ext_glCompressedTexImage1DARB is NULL");
    {
    GL_FUNC(ext_glCompressedTexImage1DARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG7:marshalArray<GLubyte>(cpu, ARG7, ARG6));
    GL_LOG ("glCompressedTexImage1DARB GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLint border=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCompressedTexImage2D(CPU* cpu) {
    if (!ext_glCompressedTexImage2D)
        kpanic("ext_glCompressedTexImage2D is NULL");
    {
    GL_FUNC(ext_glCompressedTexImage2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG8:marshalArray<GLubyte>(cpu, ARG8, ARG7));
    GL_LOG ("glCompressedTexImage2D GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d, GLsizei imageSize=%d, const void* data=%.08x bound=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,PIXEL_UNPACK_BUFFER());
    }
}
void glcommon_glCompressedTexImage2DARB(CPU* cpu) {
    if (!ext_glCompressedTexImage2DARB)
        kpanic("ext_glCompressedTexImage2DARB is NULL");
    {
    GL_FUNC(ext_glCompressedTexImage2DARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG8:marshalArray<GLubyte>(cpu, ARG8, ARG7));
    GL_LOG ("glCompressedTexImage2DARB GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCompressedTexImage3D(CPU* cpu) {
    if (!ext_glCompressedTexImage3D)
        kpanic("ext_glCompressedTexImage3D is NULL");
    {
    GL_FUNC(ext_glCompressedTexImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedTexImage3D GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCompressedTexImage3DARB(CPU* cpu) {
    if (!ext_glCompressedTexImage3DARB)
        kpanic("ext_glCompressedTexImage3DARB is NULL");
    {
    GL_FUNC(ext_glCompressedTexImage3DARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedTexImage3DARB GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCompressedTexSubImage1D(CPU* cpu) {
    if (!ext_glCompressedTexSubImage1D)
        kpanic("ext_glCompressedTexSubImage1D is NULL");
    {
    GL_FUNC(ext_glCompressedTexSubImage1D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedTexSubImage1D GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCompressedTexSubImage1DARB(CPU* cpu) {
    if (!ext_glCompressedTexSubImage1DARB)
        kpanic("ext_glCompressedTexSubImage1DARB is NULL");
    {
    GL_FUNC(ext_glCompressedTexSubImage1DARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG7:marshalArray<GLubyte>(cpu, ARG7, ARG6));
    GL_LOG ("glCompressedTexSubImage1DARB GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCompressedTexSubImage2D(CPU* cpu) {
    if (!ext_glCompressedTexSubImage2D)
        kpanic("ext_glCompressedTexSubImage2D is NULL");
    {
    GL_FUNC(ext_glCompressedTexSubImage2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedTexSubImage2D GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCompressedTexSubImage2DARB(CPU* cpu) {
    if (!ext_glCompressedTexSubImage2DARB)
        kpanic("ext_glCompressedTexSubImage2DARB is NULL");
    {
    GL_FUNC(ext_glCompressedTexSubImage2DARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedTexSubImage2DARB GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCompressedTexSubImage3D(CPU* cpu) {
    if (!ext_glCompressedTexSubImage3D)
        kpanic("ext_glCompressedTexSubImage3D is NULL");
    {
    GL_FUNC(ext_glCompressedTexSubImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG11:marshalArray<GLubyte>(cpu, ARG11, ARG10));
    GL_LOG ("glCompressedTexSubImage3D GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glCompressedTexSubImage3DARB(CPU* cpu) {
    if (!ext_glCompressedTexSubImage3DARB)
        kpanic("ext_glCompressedTexSubImage3DARB is NULL");
    {
    GL_FUNC(ext_glCompressedTexSubImage3DARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG11:marshalArray<GLubyte>(cpu, ARG11, ARG10));
    GL_LOG ("glCompressedTexSubImage3DARB GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glCompressedTextureImage1DEXT(CPU* cpu) {
    if (!ext_glCompressedTextureImage1DEXT)
        kpanic("ext_glCompressedTextureImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedTextureImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG8:marshalArray<GLubyte>(cpu, ARG8, ARG7));
    GL_LOG ("glCompressedTextureImage1DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLint border=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCompressedTextureImage2DEXT(CPU* cpu) {
    if (!ext_glCompressedTextureImage2DEXT)
        kpanic("ext_glCompressedTextureImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedTextureImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedTextureImage2DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCompressedTextureImage3DEXT(CPU* cpu) {
    if (!ext_glCompressedTextureImage3DEXT)
        kpanic("ext_glCompressedTextureImage3DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedTextureImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG10:marshalArray<GLubyte>(cpu, ARG10, ARG9));
    GL_LOG ("glCompressedTextureImage3DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glCompressedTextureSubImage1D(CPU* cpu) {
    if (!ext_glCompressedTextureSubImage1D)
        kpanic("ext_glCompressedTextureSubImage1D is NULL");
    {
    GL_FUNC(ext_glCompressedTextureSubImage1D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG6:marshalArray<GLubyte>(cpu, ARG7, ARG6));
    GL_LOG ("glCompressedTextureSubImage1D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCompressedTextureSubImage1DEXT(CPU* cpu) {
    if (!ext_glCompressedTextureSubImage1DEXT)
        kpanic("ext_glCompressedTextureSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedTextureSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG8:marshalArray<GLubyte>(cpu, ARG8, ARG7));
    GL_LOG ("glCompressedTextureSubImage1DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCompressedTextureSubImage2D(CPU* cpu) {
    if (!ext_glCompressedTextureSubImage2D)
        kpanic("ext_glCompressedTextureSubImage2D is NULL");
    {
    GL_FUNC(ext_glCompressedTextureSubImage2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG9:marshalArray<GLubyte>(cpu, ARG9, ARG8));
    GL_LOG ("glCompressedTextureSubImage2D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCompressedTextureSubImage2DEXT(CPU* cpu) {
    if (!ext_glCompressedTextureSubImage2DEXT)
        kpanic("ext_glCompressedTextureSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedTextureSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG10:marshalArray<GLubyte>(cpu, ARG10, ARG9));
    GL_LOG ("glCompressedTextureSubImage2DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glCompressedTextureSubImage3D(CPU* cpu) {
    if (!ext_glCompressedTextureSubImage3D)
        kpanic("ext_glCompressedTextureSubImage3D is NULL");
    {
    GL_FUNC(ext_glCompressedTextureSubImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG11:marshalArray<GLubyte>(cpu, ARG11, ARG10));
    GL_LOG ("glCompressedTextureSubImage3D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLsizei imageSize=%d, const void* data=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glCompressedTextureSubImage3DEXT(CPU* cpu) {
    if (!ext_glCompressedTextureSubImage3DEXT)
        kpanic("ext_glCompressedTextureSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glCompressedTextureSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG12:marshalArray<GLubyte>(cpu, ARG12, ARG11));
    GL_LOG ("glCompressedTextureSubImage3DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLsizei imageSize=%d, const void* bits=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12);
    }
}
void glcommon_glConservativeRasterParameterfNV(CPU* cpu) {
    if (!ext_glConservativeRasterParameterfNV)
        kpanic("ext_glConservativeRasterParameterfNV is NULL");
    {
    GL_FUNC(ext_glConservativeRasterParameterfNV)(ARG1, fARG2);
    GL_LOG ("glConservativeRasterParameterfNV GLenum pname=%d, GLfloat value=%f",ARG1,fARG2);
    }
}
void glcommon_glConvolutionFilter1D(CPU* cpu) {
    if (!ext_glConvolutionFilter1D)
        kpanic("ext_glConvolutionFilter1D is NULL");
    {
    GL_FUNC(ext_glConvolutionFilter1D)(ARG1, ARG2, ARG3, ARG4, ARG5, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, components_in_format(ARG4)*ARG3, ARG6));
    GL_LOG ("glConvolutionFilter1D GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* image=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glConvolutionFilter1DEXT(CPU* cpu) {
    if (!ext_glConvolutionFilter1DEXT)
        kpanic("ext_glConvolutionFilter1DEXT is NULL");
    {
    GL_FUNC(ext_glConvolutionFilter1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, components_in_format(ARG4)*ARG3, ARG6));
    GL_LOG ("glConvolutionFilter1DEXT GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* image=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glConvolutionFilter2D(CPU* cpu) {
    if (!ext_glConvolutionFilter2D)
        kpanic("ext_glConvolutionFilter2D is NULL");
    {
    GL_FUNC(ext_glConvolutionFilter2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG7:marshalType(cpu, ARG6, components_in_format(ARG5)*ARG4*ARG3, ARG7));
    GL_LOG ("glConvolutionFilter2D GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* image=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glConvolutionFilter2DEXT(CPU* cpu) {
    if (!ext_glConvolutionFilter2DEXT)
        kpanic("ext_glConvolutionFilter2DEXT is NULL");
    {
    GL_FUNC(ext_glConvolutionFilter2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG7:marshalType(cpu, ARG6, components_in_format(ARG5)*ARG4*ARG3, ARG7));
    GL_LOG ("glConvolutionFilter2DEXT GLenum target=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* image=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glConvolutionParameterf(CPU* cpu) {
    if (!ext_glConvolutionParameterf)
        kpanic("ext_glConvolutionParameterf is NULL");
    {
    GL_FUNC(ext_glConvolutionParameterf)(ARG1, ARG2, fARG3);
    GL_LOG ("glConvolutionParameterf GLenum target=%d, GLenum pname=%d, GLfloat params=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glConvolutionParameterfEXT(CPU* cpu) {
    if (!ext_glConvolutionParameterfEXT)
        kpanic("ext_glConvolutionParameterfEXT is NULL");
    {
    GL_FUNC(ext_glConvolutionParameterfEXT)(ARG1, ARG2, fARG3);
    GL_LOG ("glConvolutionParameterfEXT GLenum target=%d, GLenum pname=%d, GLfloat params=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glConvolutionParameterfv(CPU* cpu) {
    if (!ext_glConvolutionParameterfv)
        kpanic("ext_glConvolutionParameterfv is NULL");
    {
    GL_FUNC(ext_glConvolutionParameterfv)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_MODE)?1:4));
    GL_LOG ("glConvolutionParameterfv GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glConvolutionParameterfvEXT(CPU* cpu) {
    if (!ext_glConvolutionParameterfvEXT)
        kpanic("ext_glConvolutionParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glConvolutionParameterfvEXT)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_MODE_EXT)?1:4));
    GL_LOG ("glConvolutionParameterfvEXT GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glConvolutionParameteri(CPU* cpu) {
    if (!ext_glConvolutionParameteri)
        kpanic("ext_glConvolutionParameteri is NULL");
    {
    GL_FUNC(ext_glConvolutionParameteri)(ARG1, ARG2, ARG3);
    GL_LOG ("glConvolutionParameteri GLenum target=%d, GLenum pname=%d, GLint params=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glConvolutionParameteriEXT(CPU* cpu) {
    if (!ext_glConvolutionParameteriEXT)
        kpanic("ext_glConvolutionParameteriEXT is NULL");
    {
    GL_FUNC(ext_glConvolutionParameteriEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glConvolutionParameteriEXT GLenum target=%d, GLenum pname=%d, GLint params=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glConvolutionParameteriv(CPU* cpu) {
    if (!ext_glConvolutionParameteriv)
        kpanic("ext_glConvolutionParameteriv is NULL");
    {
    GL_FUNC(ext_glConvolutionParameteriv)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_MODE)?1:4));
    GL_LOG ("glConvolutionParameteriv GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glConvolutionParameterivEXT(CPU* cpu) {
    if (!ext_glConvolutionParameterivEXT)
        kpanic("ext_glConvolutionParameterivEXT is NULL");
    {
    GL_FUNC(ext_glConvolutionParameterivEXT)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_MODE_EXT)?1:4));
    GL_LOG ("glConvolutionParameterivEXT GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glConvolutionParameterxOES(CPU* cpu) {
    if (!ext_glConvolutionParameterxOES)
        kpanic("ext_glConvolutionParameterxOES is NULL");
    {
    GL_FUNC(ext_glConvolutionParameterxOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glConvolutionParameterxOES GLenum target=%d, GLenum pname=%d, GLfixed param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glConvolutionParameterxvOES(CPU* cpu) {
    if (!ext_glConvolutionParameterxvOES)
        kpanic("ext_glConvolutionParameterxvOES is NULL");
    {
    GL_FUNC(ext_glConvolutionParameterxvOES)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, 4));
    GL_LOG ("glConvolutionParameterxvOES GLenum target=%d, GLenum pname=%d, const GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glCopyBufferSubData(CPU* cpu) {
    if (!ext_glCopyBufferSubData)
        kpanic("ext_glCopyBufferSubData is NULL");
    {
    GL_FUNC(ext_glCopyBufferSubData)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyBufferSubData GLenum readTarget=%d, GLenum writeTarget=%d, GLintptr readOffset=%d, GLintptr writeOffset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyColorSubTable(CPU* cpu) {
    if (!ext_glCopyColorSubTable)
        kpanic("ext_glCopyColorSubTable is NULL");
    {
    GL_FUNC(ext_glCopyColorSubTable)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyColorSubTable GLenum target=%d, GLsizei start=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyColorSubTableEXT(CPU* cpu) {
    if (!ext_glCopyColorSubTableEXT)
        kpanic("ext_glCopyColorSubTableEXT is NULL");
    {
    GL_FUNC(ext_glCopyColorSubTableEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyColorSubTableEXT GLenum target=%d, GLsizei start=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyColorTable(CPU* cpu) {
    if (!ext_glCopyColorTable)
        kpanic("ext_glCopyColorTable is NULL");
    {
    GL_FUNC(ext_glCopyColorTable)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyColorTable GLenum target=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyColorTableSGI(CPU* cpu) {
    if (!ext_glCopyColorTableSGI)
        kpanic("ext_glCopyColorTableSGI is NULL");
    {
    GL_FUNC(ext_glCopyColorTableSGI)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyColorTableSGI GLenum target=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyConvolutionFilter1D(CPU* cpu) {
    if (!ext_glCopyConvolutionFilter1D)
        kpanic("ext_glCopyConvolutionFilter1D is NULL");
    {
    GL_FUNC(ext_glCopyConvolutionFilter1D)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyConvolutionFilter1D GLenum target=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyConvolutionFilter1DEXT(CPU* cpu) {
    if (!ext_glCopyConvolutionFilter1DEXT)
        kpanic("ext_glCopyConvolutionFilter1DEXT is NULL");
    {
    GL_FUNC(ext_glCopyConvolutionFilter1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyConvolutionFilter1DEXT GLenum target=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyConvolutionFilter2D(CPU* cpu) {
    if (!ext_glCopyConvolutionFilter2D)
        kpanic("ext_glCopyConvolutionFilter2D is NULL");
    {
    GL_FUNC(ext_glCopyConvolutionFilter2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glCopyConvolutionFilter2D GLenum target=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glCopyConvolutionFilter2DEXT(CPU* cpu) {
    if (!ext_glCopyConvolutionFilter2DEXT)
        kpanic("ext_glCopyConvolutionFilter2DEXT is NULL");
    {
    GL_FUNC(ext_glCopyConvolutionFilter2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glCopyConvolutionFilter2DEXT GLenum target=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glCopyImageSubData(CPU* cpu) {
    if (!ext_glCopyImageSubData)
        kpanic("ext_glCopyImageSubData is NULL");
    {
    GL_FUNC(ext_glCopyImageSubData)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15);
    GL_LOG ("glCopyImageSubData GLuint srcName=%d, GLenum srcTarget=%d, GLint srcLevel=%d, GLint srcX=%d, GLint srcY=%d, GLint srcZ=%d, GLuint dstName=%d, GLenum dstTarget=%d, GLint dstLevel=%d, GLint dstX=%d, GLint dstY=%d, GLint dstZ=%d, GLsizei srcWidth=%d, GLsizei srcHeight=%d, GLsizei srcDepth=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12,ARG13,ARG14,ARG15);
    }
}
void glcommon_glCopyImageSubDataNV(CPU* cpu) {
    if (!ext_glCopyImageSubDataNV)
        kpanic("ext_glCopyImageSubDataNV is NULL");
    {
    GL_FUNC(ext_glCopyImageSubDataNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15);
    GL_LOG ("glCopyImageSubDataNV GLuint srcName=%d, GLenum srcTarget=%d, GLint srcLevel=%d, GLint srcX=%d, GLint srcY=%d, GLint srcZ=%d, GLuint dstName=%d, GLenum dstTarget=%d, GLint dstLevel=%d, GLint dstX=%d, GLint dstY=%d, GLint dstZ=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12,ARG13,ARG14,ARG15);
    }
}
void glcommon_glCopyMultiTexImage1DEXT(CPU* cpu) {
    if (!ext_glCopyMultiTexImage1DEXT)
        kpanic("ext_glCopyMultiTexImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCopyMultiTexImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glCopyMultiTexImage1DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLint border=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCopyMultiTexImage2DEXT(CPU* cpu) {
    if (!ext_glCopyMultiTexImage2DEXT)
        kpanic("ext_glCopyMultiTexImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCopyMultiTexImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glCopyMultiTexImage2DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCopyMultiTexSubImage1DEXT(CPU* cpu) {
    if (!ext_glCopyMultiTexSubImage1DEXT)
        kpanic("ext_glCopyMultiTexSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCopyMultiTexSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glCopyMultiTexSubImage1DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCopyMultiTexSubImage2DEXT(CPU* cpu) {
    if (!ext_glCopyMultiTexSubImage2DEXT)
        kpanic("ext_glCopyMultiTexSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCopyMultiTexSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glCopyMultiTexSubImage2DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCopyMultiTexSubImage3DEXT(CPU* cpu) {
    if (!ext_glCopyMultiTexSubImage3DEXT)
        kpanic("ext_glCopyMultiTexSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glCopyMultiTexSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10);
    GL_LOG ("glCopyMultiTexSubImage3DEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glCopyNamedBufferSubData(CPU* cpu) {
    if (!ext_glCopyNamedBufferSubData)
        kpanic("ext_glCopyNamedBufferSubData is NULL");
    {
    GL_FUNC(ext_glCopyNamedBufferSubData)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glCopyNamedBufferSubData GLuint readBuffer=%d, GLuint writeBuffer=%d, GLintptr readOffset=%d, GLintptr writeOffset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glCopyPathNV(CPU* cpu) {
    if (!ext_glCopyPathNV)
        kpanic("ext_glCopyPathNV is NULL");
    {
    GL_FUNC(ext_glCopyPathNV)(ARG1, ARG2);
    GL_LOG ("glCopyPathNV GLuint resultPath=%d, GLuint srcPath=%d",ARG1,ARG2);
    }
}
void glcommon_glCopyTexImage1DEXT(CPU* cpu) {
    if (!ext_glCopyTexImage1DEXT)
        kpanic("ext_glCopyTexImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTexImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glCopyTexImage1DEXT GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLint border=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCopyTexImage2DEXT(CPU* cpu) {
    if (!ext_glCopyTexImage2DEXT)
        kpanic("ext_glCopyTexImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTexImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glCopyTexImage2DEXT GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCopyTexSubImage1DEXT(CPU* cpu) {
    if (!ext_glCopyTexSubImage1DEXT)
        kpanic("ext_glCopyTexSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTexSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glCopyTexSubImage1DEXT GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glCopyTexSubImage2DEXT(CPU* cpu) {
    if (!ext_glCopyTexSubImage2DEXT)
        kpanic("ext_glCopyTexSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTexSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glCopyTexSubImage2DEXT GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCopyTexSubImage3D(CPU* cpu) {
    if (!ext_glCopyTexSubImage3D)
        kpanic("ext_glCopyTexSubImage3D is NULL");
    {
    GL_FUNC(ext_glCopyTexSubImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glCopyTexSubImage3D GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCopyTexSubImage3DEXT(CPU* cpu) {
    if (!ext_glCopyTexSubImage3DEXT)
        kpanic("ext_glCopyTexSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTexSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glCopyTexSubImage3DEXT GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCopyTextureImage1DEXT(CPU* cpu) {
    if (!ext_glCopyTextureImage1DEXT)
        kpanic("ext_glCopyTextureImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTextureImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glCopyTextureImage1DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLint border=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCopyTextureImage2DEXT(CPU* cpu) {
    if (!ext_glCopyTextureImage2DEXT)
        kpanic("ext_glCopyTextureImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTextureImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glCopyTextureImage2DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCopyTextureSubImage1D(CPU* cpu) {
    if (!ext_glCopyTextureSubImage1D)
        kpanic("ext_glCopyTextureSubImage1D is NULL");
    {
    GL_FUNC(ext_glCopyTextureSubImage1D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glCopyTextureSubImage1D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glCopyTextureSubImage1DEXT(CPU* cpu) {
    if (!ext_glCopyTextureSubImage1DEXT)
        kpanic("ext_glCopyTextureSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTextureSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glCopyTextureSubImage1DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCopyTextureSubImage2D(CPU* cpu) {
    if (!ext_glCopyTextureSubImage2D)
        kpanic("ext_glCopyTextureSubImage2D is NULL");
    {
    GL_FUNC(ext_glCopyTextureSubImage2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glCopyTextureSubImage2D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glCopyTextureSubImage2DEXT(CPU* cpu) {
    if (!ext_glCopyTextureSubImage2DEXT)
        kpanic("ext_glCopyTextureSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTextureSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glCopyTextureSubImage2DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCopyTextureSubImage3D(CPU* cpu) {
    if (!ext_glCopyTextureSubImage3D)
        kpanic("ext_glCopyTextureSubImage3D is NULL");
    {
    GL_FUNC(ext_glCopyTextureSubImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    GL_LOG ("glCopyTextureSubImage3D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glCopyTextureSubImage3DEXT(CPU* cpu) {
    if (!ext_glCopyTextureSubImage3DEXT)
        kpanic("ext_glCopyTextureSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glCopyTextureSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10);
    GL_LOG ("glCopyTextureSubImage3DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glCoverFillPathInstancedNV(CPU* cpu) {
    if (!ext_glCoverFillPathInstancedNV)
        kpanic("ext_glCoverFillPathInstancedNV is NULL");
    {
    GL_FUNC(ext_glCoverFillPathInstancedNV)(ARG1, ARG2, marshalType(cpu, ARG2, ARG1, ARG3), ARG4, ARG5, ARG6, marshalArray<GLfloat>(cpu, ARG7, floatPerTransformList(ARG6)*ARG1));
    GL_LOG ("glCoverFillPathInstancedNV GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLenum coverMode=%d, GLenum transformType=%d, const GLfloat* transformValues=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCoverFillPathNV(CPU* cpu) {
    if (!ext_glCoverFillPathNV)
        kpanic("ext_glCoverFillPathNV is NULL");
    {
    GL_FUNC(ext_glCoverFillPathNV)(ARG1, ARG2);
    GL_LOG ("glCoverFillPathNV GLuint path=%d, GLenum coverMode=%d",ARG1,ARG2);
    }
}
void glcommon_glCoverStrokePathInstancedNV(CPU* cpu) {
    if (!ext_glCoverStrokePathInstancedNV)
        kpanic("ext_glCoverStrokePathInstancedNV is NULL");
    {
    GL_FUNC(ext_glCoverStrokePathInstancedNV)(ARG1, ARG2, marshalType(cpu, ARG2, ARG1, ARG3), ARG4, ARG5, ARG6, marshalArray<GLfloat>(cpu, ARG7, floatPerTransformList(ARG6)*ARG1));
    GL_LOG ("glCoverStrokePathInstancedNV GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLenum coverMode=%d, GLenum transformType=%d, const GLfloat* transformValues=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glCoverStrokePathNV(CPU* cpu) {
    if (!ext_glCoverStrokePathNV)
        kpanic("ext_glCoverStrokePathNV is NULL");
    {
    GL_FUNC(ext_glCoverStrokePathNV)(ARG1, ARG2);
    GL_LOG ("glCoverStrokePathNV GLuint path=%d, GLenum coverMode=%d",ARG1,ARG2);
    }
}
void glcommon_glCoverageModulationNV(CPU* cpu) {
    if (!ext_glCoverageModulationNV)
        kpanic("ext_glCoverageModulationNV is NULL");
    {
    GL_FUNC(ext_glCoverageModulationNV)(ARG1);
    GL_LOG ("glCoverageModulationNV GLenum components=%d",ARG1);
    }
}
void glcommon_glCoverageModulationTableNV(CPU* cpu) {
    if (!ext_glCoverageModulationTableNV)
        kpanic("ext_glCoverageModulationTableNV is NULL");
    {
    GL_FUNC(ext_glCoverageModulationTableNV)(ARG1, marshalArray<GLfloat>(cpu, ARG2, ARG1));
    GL_LOG ("glCoverageModulationTableNV GLsizei n=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateBuffers(CPU* cpu) {
    if (!ext_glCreateBuffers)
        kpanic("ext_glCreateBuffers is NULL");
    {
        MarshalReadWrite<GLuint> buffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateBuffers)(ARG1, buffers.getPtr());
        GL_LOG ("glCreateBuffers GLsizei n=%d, GLuint* buffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateCommandListsNV(CPU* cpu) {
    if (!ext_glCreateCommandListsNV)
        kpanic("ext_glCreateCommandListsNV is NULL");
    {
        MarshalReadWrite<GLuint> lists(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateCommandListsNV)(ARG1, lists.getPtr());
        GL_LOG ("glCreateCommandListsNV GLsizei n=%d, GLuint* lists=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateFramebuffers(CPU* cpu) {
    if (!ext_glCreateFramebuffers)
        kpanic("ext_glCreateFramebuffers is NULL");
    {
        MarshalReadWrite<GLuint> framebuffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateFramebuffers)(ARG1, framebuffers.getPtr());
        GL_LOG ("glCreateFramebuffers GLsizei n=%d, GLuint* framebuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreatePerfQueryINTEL(CPU* cpu) {
    if (!ext_glCreatePerfQueryINTEL)
        kpanic("ext_glCreatePerfQueryINTEL is NULL");
    {
        MarshalReadWrite<GLuint> queryHandle(cpu, ARG2, 1);
        GL_FUNC(ext_glCreatePerfQueryINTEL)(ARG1, queryHandle.getPtr());
        GL_LOG ("glCreatePerfQueryINTEL GLuint queryId=%d, GLuint* queryHandle=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateProgram(CPU* cpu) {
    if (!ext_glCreateProgram)
        kpanic("ext_glCreateProgram is NULL");
    {
    EAX=GL_FUNC(ext_glCreateProgram)();
    GL_LOG ("glCreateProgram");
    }
}
void glcommon_glCreateProgramObjectARB(CPU* cpu) {
    if (!ext_glCreateProgramObjectARB)
        kpanic("ext_glCreateProgramObjectARB is NULL");
    {
    EAX=HANDLE_TO_INDEX(GL_FUNC(ext_glCreateProgramObjectARB)());
    GL_LOG ("glCreateProgramObjectARB");
    }
}
void glcommon_glCreateProgramPipelines(CPU* cpu) {
    if (!ext_glCreateProgramPipelines)
        kpanic("ext_glCreateProgramPipelines is NULL");
    {
        MarshalReadWrite<GLuint> pipelines(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateProgramPipelines)(ARG1, pipelines.getPtr());
        GL_LOG ("glCreateProgramPipelines GLsizei n=%d, GLuint* pipelines=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateQueries(CPU* cpu) {
    if (!ext_glCreateQueries)
        kpanic("ext_glCreateQueries is NULL");
    {
        MarshalReadWrite<GLuint> ids(cpu, ARG3, ARG2);
        GL_FUNC(ext_glCreateQueries)(ARG1, ARG2, ids.getPtr());
        GL_LOG ("glCreateQueries GLenum target=%d, GLsizei n=%d, GLuint* ids=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glCreateRenderbuffers(CPU* cpu) {
    if (!ext_glCreateRenderbuffers)
        kpanic("ext_glCreateRenderbuffers is NULL");
    {
        MarshalReadWrite<GLuint> renderbuffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateRenderbuffers)(ARG1, renderbuffers.getPtr());
        GL_LOG ("glCreateRenderbuffers GLsizei n=%d, GLuint* renderbuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateSamplers(CPU* cpu) {
    if (!ext_glCreateSamplers)
        kpanic("ext_glCreateSamplers is NULL");
    {
        MarshalReadWrite<GLuint> samplers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateSamplers)(ARG1, samplers.getPtr());
        GL_LOG ("glCreateSamplers GLsizei n=%d, GLuint* samplers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateShader(CPU* cpu) {
    if (!ext_glCreateShader)
        kpanic("ext_glCreateShader is NULL");
    {
    EAX=GL_FUNC(ext_glCreateShader)(ARG1);
    GL_LOG ("glCreateShader GLenum type=%d",ARG1);
    }
}
void glcommon_glCreateShaderObjectARB(CPU* cpu) {
    if (!ext_glCreateShaderObjectARB)
        kpanic("ext_glCreateShaderObjectARB is NULL");
    {
    EAX=HANDLE_TO_INDEX(GL_FUNC(ext_glCreateShaderObjectARB)(ARG1));
    GL_LOG ("glCreateShaderObjectARB GLenum shaderType=%d",ARG1);
    }
}
void glcommon_glCreateShaderProgramEXT(CPU* cpu) {
    if (!ext_glCreateShaderProgramEXT)
        kpanic("ext_glCreateShaderProgramEXT is NULL");
    {
    EAX=GL_FUNC(ext_glCreateShaderProgramEXT)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glCreateShaderProgramEXT GLenum type=%d, const GLchar* string=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateShaderProgramv(CPU* cpu) {
    if (!ext_glCreateShaderProgramv)
        kpanic("ext_glCreateShaderProgramv is NULL");
    {
    EAX=GL_FUNC(ext_glCreateShaderProgramv)(ARG1, ARG2, (GLchar*const*)marshalpp(cpu, ARG3, ARG2, 0, -1, sizeof(GLchar)));
    GL_LOG ("glCreateShaderProgramv GLenum type=%d, GLsizei count=%d, const GLchar*const* strings=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glCreateStatesNV(CPU* cpu) {
    if (!ext_glCreateStatesNV)
        kpanic("ext_glCreateStatesNV is NULL");
    {
        MarshalReadWrite<GLuint> rw(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateStatesNV)(ARG1, rw.getPtr());
        GL_LOG ("glCreateStatesNV GLsizei n=%d, GLuint* states=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateSyncFromCLeventARB(CPU* cpu) {
    if (!ext_glCreateSyncFromCLeventARB)
        kpanic("ext_glCreateSyncFromCLeventARB is NULL");
    {
    GLsync ret=GL_FUNC(ext_glCreateSyncFromCLeventARB)((void*)marshalp(cpu, 0, ARG1, 0), (void*)marshalp(cpu, 0, ARG2, 0), ARG3);
    EAX=marshalBackSync(cpu, ret);
    GL_LOG ("glCreateSyncFromCLeventARB void* context=%.08x, void* event=%.08x, GLbitfield flags=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glCreateTextures(CPU* cpu) {
    if (!ext_glCreateTextures)
        kpanic("ext_glCreateTextures is NULL");
    {
        MarshalReadWrite<GLuint> textures(cpu, ARG3, ARG2);
        GL_FUNC(ext_glCreateTextures)(ARG1, ARG2, textures.getPtr());
        GL_LOG ("glCreateTextures GLenum target=%d, GLsizei n=%d, GLuint* textures=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glCreateTransformFeedbacks(CPU* cpu) {
    if (!ext_glCreateTransformFeedbacks)
        kpanic("ext_glCreateTransformFeedbacks is NULL");
    {
        MarshalReadWrite<GLuint> ids(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateTransformFeedbacks)(ARG1, ids.getPtr());
        GL_LOG ("glCreateTransformFeedbacks GLsizei n=%d, GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCreateVertexArrays(CPU* cpu) {
    if (!ext_glCreateVertexArrays)
        kpanic("ext_glCreateVertexArrays is NULL");
    {
        MarshalReadWrite<GLuint> arrays(cpu, ARG2, ARG1);
        GL_FUNC(ext_glCreateVertexArrays)(ARG1, arrays.getPtr());
        GL_LOG ("glCreateVertexArrays GLsizei n=%d, GLuint* arrays=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCullParameterdvEXT(CPU* cpu) {
    if (!ext_glCullParameterdvEXT)
        kpanic("ext_glCullParameterdvEXT is NULL");
    {
        MarshalReadWrite<GLdouble> params(cpu, ARG2, 4);
        GL_FUNC(ext_glCullParameterdvEXT)(ARG1, params.getPtr());
        GL_LOG ("glCullParameterdvEXT GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCullParameterfvEXT(CPU* cpu) {
    if (!ext_glCullParameterfvEXT)
        kpanic("ext_glCullParameterfvEXT is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG2, 4);
        GL_FUNC(ext_glCullParameterfvEXT)(ARG1, params.getPtr());
        GL_LOG ("glCullParameterfvEXT GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glCurrentPaletteMatrixARB(CPU* cpu) {
    if (!ext_glCurrentPaletteMatrixARB)
        kpanic("ext_glCurrentPaletteMatrixARB is NULL");
    {
    GL_FUNC(ext_glCurrentPaletteMatrixARB)(ARG1);
    GL_LOG ("glCurrentPaletteMatrixARB GLint index=%d",ARG1);
    }
}
void glcommon_glDebugMessageCallback(CPU* cpu) {
    if (!ext_glDebugMessageCallback)
        kpanic("ext_glDebugMessageCallback is NULL");
    {
    GL_FUNC(ext_glDebugMessageCallback)(debugMessageCallback, nullptr);
    GL_LOG ("glDebugMessageCallback void * callback=%.08x, const void* userParam=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDebugMessageCallbackAMD(CPU* cpu) {
    if (!ext_glDebugMessageCallbackAMD)
        kpanic("ext_glDebugMessageCallbackAMD is NULL");
    {
    GL_FUNC(ext_glDebugMessageCallbackAMD)(debugMessageCallback, nullptr);
    GL_LOG ("glDebugMessageCallbackAMD void * callback=%.08x, void* userParam=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDebugMessageCallbackARB(CPU* cpu) {
    if (!ext_glDebugMessageCallbackARB)
        kpanic("ext_glDebugMessageCallbackARB is NULL");
    {
    GL_FUNC(ext_glDebugMessageCallbackARB)(debugMessageCallback, nullptr);
    GL_LOG ("glDebugMessageCallbackARB void * callback=%.08x, const void* userParam=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDebugMessageControl(CPU* cpu) {
    if (!ext_glDebugMessageControl)
        kpanic("ext_glDebugMessageControl is NULL");
    {
    GL_FUNC(ext_glDebugMessageControl)(ARG1, ARG2, ARG3, ARG4, marshalArray<GLuint>(cpu, ARG5, ARG4), bARG6);
    GL_LOG ("glDebugMessageControl GLenum source=%d, GLenum type=%d, GLenum severity=%d, GLsizei count=%d, const GLuint* ids=%.08x, GLboolean enabled=%d",ARG1,ARG2,ARG3,ARG4,ARG5,bARG6);
    }
}
void glcommon_glDebugMessageControlARB(CPU* cpu) {
    if (!ext_glDebugMessageControlARB)
        kpanic("ext_glDebugMessageControlARB is NULL");
    {
    GL_FUNC(ext_glDebugMessageControlARB)(ARG1, ARG2, ARG3, ARG4, marshalArray<GLuint>(cpu, ARG5, ARG4), bARG6);
    GL_LOG ("glDebugMessageControlARB GLenum source=%d, GLenum type=%d, GLenum severity=%d, GLsizei count=%d, const GLuint* ids=%.08x, GLboolean enabled=%d",ARG1,ARG2,ARG3,ARG4,ARG5,bARG6);
    }
}
void glcommon_glDebugMessageEnableAMD(CPU* cpu) {
    if (!ext_glDebugMessageEnableAMD)
        kpanic("ext_glDebugMessageEnableAMD is NULL");
    {
    GL_FUNC(ext_glDebugMessageEnableAMD)(ARG1, ARG2, ARG3, marshalArray<GLuint>(cpu, ARG4, ARG3), bARG5);
    GL_LOG ("glDebugMessageEnableAMD GLenum category=%d, GLenum severity=%d, GLsizei count=%d, const GLuint* ids=%.08x, GLboolean enabled=%d",ARG1,ARG2,ARG3,ARG4,bARG5);
    }
}
void glcommon_glDebugMessageInsert(CPU* cpu) {
    if (!ext_glDebugMessageInsert)
        kpanic("ext_glDebugMessageInsert is NULL");
    {
    GL_FUNC(ext_glDebugMessageInsert)(ARG1, ARG2, ARG3, ARG4, ARG5, marshalsz(cpu, ARG6));
    GL_LOG ("glDebugMessageInsert GLenum source=%d, GLenum type=%d, GLuint id=%d, GLenum severity=%d, GLsizei length=%d, const GLchar* buf=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDebugMessageInsertAMD(CPU* cpu) {
    if (!ext_glDebugMessageInsertAMD)
        kpanic("ext_glDebugMessageInsertAMD is NULL");
    {
    GL_FUNC(ext_glDebugMessageInsertAMD)(ARG1, ARG2, ARG3, ARG4, marshalsz(cpu, ARG5));
    GL_LOG ("glDebugMessageInsertAMD GLenum category=%d, GLenum severity=%d, GLuint id=%d, GLsizei length=%d, const GLchar* buf=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDebugMessageInsertARB(CPU* cpu) {
    if (!ext_glDebugMessageInsertARB)
        kpanic("ext_glDebugMessageInsertARB is NULL");
    {
    GL_FUNC(ext_glDebugMessageInsertARB)(ARG1, ARG2, ARG3, ARG4, ARG5, marshalsz(cpu, ARG6));
    GL_LOG ("glDebugMessageInsertARB GLenum source=%d, GLenum type=%d, GLuint id=%d, GLenum severity=%d, GLsizei length=%d, const GLchar* buf=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDeformSGIX(CPU* cpu) {
    if (!ext_glDeformSGIX)
        kpanic("ext_glDeformSGIX is NULL");
    {
    GL_FUNC(ext_glDeformSGIX)(ARG1);
    GL_LOG ("glDeformSGIX GLbitfield mask=%d",ARG1);
    }
}
void glcommon_glDeformationMap3dSGIX(CPU* cpu) {
    if (!ext_glDeformationMap3dSGIX)
        kpanic("ext_glDeformationMap3dSGIX is NULL");
    {
    GL_FUNC(ext_glDeformationMap3dSGIX)(ARG1, dARG2, dARG3, ARG4, ARG5, dARG6, dARG7, ARG8, ARG9, dARG10, dARG11, ARG12, ARG13, (GLdouble*)marshalp(cpu, 0, ARG14, 0));
    GL_LOG ("glDeformationMap3dSGIX GLenum target=%d, GLdouble u1=%f, GLdouble u2=%f, GLint ustride=%d, GLint uorder=%d, GLdouble v1=%f, GLdouble v2=%f, GLint vstride=%d, GLint vorder=%d, GLdouble w1=%f, GLdouble w2=%f, GLint wstride=%d, GLint worder=%d, const GLdouble* points=%.08x",ARG1,dARG2,dARG3,ARG4,ARG5,dARG6,dARG7,ARG8,ARG9,dARG10,dARG11,ARG12,ARG13,ARG14);
    }
}
void glcommon_glDeformationMap3fSGIX(CPU* cpu) {
    if (!ext_glDeformationMap3fSGIX)
        kpanic("ext_glDeformationMap3fSGIX is NULL");
    {
    GL_FUNC(ext_glDeformationMap3fSGIX)(ARG1, fARG2, fARG3, ARG4, ARG5, fARG6, fARG7, ARG8, ARG9, fARG10, fARG11, ARG12, ARG13, (GLfloat*)marshalp(cpu, 0, ARG14, 0));
    GL_LOG ("glDeformationMap3fSGIX GLenum target=%d, GLfloat u1=%f, GLfloat u2=%f, GLint ustride=%d, GLint uorder=%d, GLfloat v1=%f, GLfloat v2=%f, GLint vstride=%d, GLint vorder=%d, GLfloat w1=%f, GLfloat w2=%f, GLint wstride=%d, GLint worder=%d, const GLfloat* points=%.08x",ARG1,fARG2,fARG3,ARG4,ARG5,fARG6,fARG7,ARG8,ARG9,fARG10,fARG11,ARG12,ARG13,ARG14);
    }
}
void glcommon_glDeleteAsyncMarkersSGIX(CPU* cpu) {
    if (!ext_glDeleteAsyncMarkersSGIX)
        kpanic("ext_glDeleteAsyncMarkersSGIX is NULL");
    {
    GL_FUNC(ext_glDeleteAsyncMarkersSGIX)(ARG1, ARG2);
    GL_LOG ("glDeleteAsyncMarkersSGIX GLuint marker=%d, GLsizei range=%d",ARG1,ARG2);
    }
}
void glcommon_glDeleteBufferRegion(CPU* cpu) {
    if (!ext_glDeleteBufferRegion)
        kpanic("ext_glDeleteBufferRegion is NULL");
    {
    GL_FUNC(ext_glDeleteBufferRegion)(ARG1);
    GL_LOG ("glDeleteBufferRegion GLenum region=%d",ARG1);
    }
}
void glcommon_glDeleteBuffers(CPU* cpu) {
    if (!ext_glDeleteBuffers)
        kpanic("ext_glDeleteBuffers is NULL");
    {
    GL_FUNC(ext_glDeleteBuffers)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteBuffers GLsizei n=%d, const GLuint* buffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteBuffersARB(CPU* cpu) {
    if (!ext_glDeleteBuffersARB)
        kpanic("ext_glDeleteBuffersARB is NULL");
    {
    GL_FUNC(ext_glDeleteBuffersARB)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteBuffersARB GLsizei n=%d, const GLuint* buffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteCommandListsNV(CPU* cpu) {
    if (!ext_glDeleteCommandListsNV)
        kpanic("ext_glDeleteCommandListsNV is NULL");
    {
    GL_FUNC(ext_glDeleteCommandListsNV)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteCommandListsNV GLsizei n=%d, const GLuint* lists=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteFencesAPPLE(CPU* cpu) {
    if (!ext_glDeleteFencesAPPLE)
        kpanic("ext_glDeleteFencesAPPLE is NULL");
    {
    GL_FUNC(ext_glDeleteFencesAPPLE)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteFencesAPPLE GLsizei n=%d, const GLuint* fences=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteFencesNV(CPU* cpu) {
    if (!ext_glDeleteFencesNV)
        kpanic("ext_glDeleteFencesNV is NULL");
    {
    GL_FUNC(ext_glDeleteFencesNV)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteFencesNV GLsizei n=%d, const GLuint* fences=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteFragmentShaderATI(CPU* cpu) {
    if (!ext_glDeleteFragmentShaderATI)
        kpanic("ext_glDeleteFragmentShaderATI is NULL");
    {
    GL_FUNC(ext_glDeleteFragmentShaderATI)(ARG1);
    GL_LOG ("glDeleteFragmentShaderATI GLuint id=%d",ARG1);
    }
}
void glcommon_glDeleteFramebuffers(CPU* cpu) {
    if (!ext_glDeleteFramebuffers)
        kpanic("ext_glDeleteFramebuffers is NULL");
    {
    GL_FUNC(ext_glDeleteFramebuffers)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteFramebuffers GLsizei n=%d, const GLuint* framebuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteFramebuffersEXT(CPU* cpu) {
    if (!ext_glDeleteFramebuffersEXT)
        kpanic("ext_glDeleteFramebuffersEXT is NULL");
    {
    GL_FUNC(ext_glDeleteFramebuffersEXT)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteFramebuffersEXT GLsizei n=%d, const GLuint* framebuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteNamedStringARB(CPU* cpu) {
    if (!ext_glDeleteNamedStringARB)
        kpanic("ext_glDeleteNamedStringARB is NULL");
    {
    GL_FUNC(ext_glDeleteNamedStringARB)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glDeleteNamedStringARB GLint namelen=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteNamesAMD(CPU* cpu) {
    if (!ext_glDeleteNamesAMD)
        kpanic("ext_glDeleteNamesAMD is NULL");
    {
    GL_FUNC(ext_glDeleteNamesAMD)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2));
    GL_LOG ("glDeleteNamesAMD GLenum identifier=%d, GLuint num=%d, const GLuint* names=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDeleteObjectARB(CPU* cpu) {
    if (!ext_glDeleteObjectARB)
        kpanic("ext_glDeleteObjectARB is NULL");
    {
    GL_FUNC(ext_glDeleteObjectARB)(INDEX_TO_HANDLE(hARG1));
    DELETE_HANDLE_INDEX(hARG1);
    GL_LOG ("glDeleteObjectARB GLhandleARB obj=%d",ARG1);
    }
}
void glcommon_glDeleteObjectBufferATI(CPU* cpu) {
    if (!ext_glDeleteObjectBufferATI)
        kpanic("ext_glDeleteObjectBufferATI is NULL");
    {
    GL_FUNC(ext_glDeleteObjectBufferATI)(ARG1);
    GL_LOG ("glDeleteObjectBufferATI GLuint buffer=%d",ARG1);
    }
}
void glcommon_glDeleteOcclusionQueriesNV(CPU* cpu) {
    if (!ext_glDeleteOcclusionQueriesNV)
        kpanic("ext_glDeleteOcclusionQueriesNV is NULL");
    {
    GL_FUNC(ext_glDeleteOcclusionQueriesNV)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteOcclusionQueriesNV GLsizei n=%d, const GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeletePathsNV(CPU* cpu) {
    if (!ext_glDeletePathsNV)
        kpanic("ext_glDeletePathsNV is NULL");
    {
    GL_FUNC(ext_glDeletePathsNV)(ARG1, ARG2);
    GL_LOG ("glDeletePathsNV GLuint path=%d, GLsizei range=%d",ARG1,ARG2);
    }
}
void glcommon_glDeletePerfMonitorsAMD(CPU* cpu) {
    if (!ext_glDeletePerfMonitorsAMD)
        kpanic("ext_glDeletePerfMonitorsAMD is NULL");
    {
        MarshalReadWrite<GLuint> monitors(cpu, ARG2, ARG1);
        GL_FUNC(ext_glDeletePerfMonitorsAMD)(ARG1, monitors.getPtr());
        GL_LOG ("glDeletePerfMonitorsAMD GLsizei n=%d, GLuint* monitors=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeletePerfQueryINTEL(CPU* cpu) {
    if (!ext_glDeletePerfQueryINTEL)
        kpanic("ext_glDeletePerfQueryINTEL is NULL");
    {
    GL_FUNC(ext_glDeletePerfQueryINTEL)(ARG1);
    GL_LOG ("glDeletePerfQueryINTEL GLuint queryHandle=%d",ARG1);
    }
}
void glcommon_glDeleteProgram(CPU* cpu) {
    if (!ext_glDeleteProgram)
        kpanic("ext_glDeleteProgram is NULL");
    {
    GL_FUNC(ext_glDeleteProgram)(ARG1);
    GL_LOG ("glDeleteProgram GLuint program=%d",ARG1);
    }
}
void glcommon_glDeleteProgramPipelines(CPU* cpu) {
    if (!ext_glDeleteProgramPipelines)
        kpanic("ext_glDeleteProgramPipelines is NULL");
    {
    GL_FUNC(ext_glDeleteProgramPipelines)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteProgramPipelines GLsizei n=%d, const GLuint* pipelines=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteProgramsARB(CPU* cpu) {
    if (!ext_glDeleteProgramsARB)
        kpanic("ext_glDeleteProgramsARB is NULL");
    {
    GL_FUNC(ext_glDeleteProgramsARB)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteProgramsARB GLsizei n=%d, const GLuint* programs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteProgramsNV(CPU* cpu) {
    if (!ext_glDeleteProgramsNV)
        kpanic("ext_glDeleteProgramsNV is NULL");
    {
    GL_FUNC(ext_glDeleteProgramsNV)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteProgramsNV GLsizei n=%d, const GLuint* programs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteQueries(CPU* cpu) {
    if (!ext_glDeleteQueries)
        kpanic("ext_glDeleteQueries is NULL");
    {
    GL_FUNC(ext_glDeleteQueries)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteQueries GLsizei n=%d, const GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteQueriesARB(CPU* cpu) {
    if (!ext_glDeleteQueriesARB)
        kpanic("ext_glDeleteQueriesARB is NULL");
    {
    GL_FUNC(ext_glDeleteQueriesARB)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteQueriesARB GLsizei n=%d, const GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteRenderbuffers(CPU* cpu) {
    if (!ext_glDeleteRenderbuffers)
        kpanic("ext_glDeleteRenderbuffers is NULL");
    {
    GL_FUNC(ext_glDeleteRenderbuffers)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteRenderbuffers GLsizei n=%d, const GLuint* renderbuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteRenderbuffersEXT(CPU* cpu) {
    if (!ext_glDeleteRenderbuffersEXT)
        kpanic("ext_glDeleteRenderbuffersEXT is NULL");
    {
    GL_FUNC(ext_glDeleteRenderbuffersEXT)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteRenderbuffersEXT GLsizei n=%d, const GLuint* renderbuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteSamplers(CPU* cpu) {
    if (!ext_glDeleteSamplers)
        kpanic("ext_glDeleteSamplers is NULL");
    {
    GL_FUNC(ext_glDeleteSamplers)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteSamplers GLsizei count=%d, const GLuint* samplers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteShader(CPU* cpu) {
    if (!ext_glDeleteShader)
        kpanic("ext_glDeleteShader is NULL");
    {
    GL_FUNC(ext_glDeleteShader)(ARG1);
    GL_LOG ("glDeleteShader GLuint shader=%d",ARG1);
    }
}
void glcommon_glDeleteStatesNV(CPU* cpu) {
    if (!ext_glDeleteStatesNV)
        kpanic("ext_glDeleteStatesNV is NULL");
    {
    GL_FUNC(ext_glDeleteStatesNV)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteStatesNV GLsizei n=%d, const GLuint* states=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteSync(CPU* cpu) {
    if (!ext_glDeleteSync)
        kpanic("ext_glDeleteSync is NULL");
    {
    GL_FUNC(ext_glDeleteSync)(marshalSync(cpu, ARG1));
    GL_LOG ("glDeleteSync GLsync sync=%d",ARG1);
    }
}
void glcommon_glDeleteTexturesEXT(CPU* cpu) {
    if (!ext_glDeleteTexturesEXT)
        kpanic("ext_glDeleteTexturesEXT is NULL");
    {
    GL_FUNC(ext_glDeleteTexturesEXT)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteTexturesEXT GLsizei n=%d, const GLuint* textures=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteTransformFeedbacks(CPU* cpu) {
    if (!ext_glDeleteTransformFeedbacks)
        kpanic("ext_glDeleteTransformFeedbacks is NULL");
    {
    GL_FUNC(ext_glDeleteTransformFeedbacks)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteTransformFeedbacks GLsizei n=%d, const GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteTransformFeedbacksNV(CPU* cpu) {
    if (!ext_glDeleteTransformFeedbacksNV)
        kpanic("ext_glDeleteTransformFeedbacksNV is NULL");
    {
    GL_FUNC(ext_glDeleteTransformFeedbacksNV)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteTransformFeedbacksNV GLsizei n=%d, const GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteVertexArrays(CPU* cpu) {
    if (!ext_glDeleteVertexArrays)
        kpanic("ext_glDeleteVertexArrays is NULL");
    {
    GL_FUNC(ext_glDeleteVertexArrays)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteVertexArrays GLsizei n=%d, const GLuint* arrays=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteVertexArraysAPPLE(CPU* cpu) {
    if (!ext_glDeleteVertexArraysAPPLE)
        kpanic("ext_glDeleteVertexArraysAPPLE is NULL");
    {
    GL_FUNC(ext_glDeleteVertexArraysAPPLE)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDeleteVertexArraysAPPLE GLsizei n=%d, const GLuint* arrays=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDeleteVertexShaderEXT(CPU* cpu) {
    if (!ext_glDeleteVertexShaderEXT)
        kpanic("ext_glDeleteVertexShaderEXT is NULL");
    {
    GL_FUNC(ext_glDeleteVertexShaderEXT)(ARG1);
    GL_LOG ("glDeleteVertexShaderEXT GLuint id=%d",ARG1);
    }
}
void glcommon_glDepthBoundsEXT(CPU* cpu) {
    if (!ext_glDepthBoundsEXT)
        kpanic("ext_glDepthBoundsEXT is NULL");
    {
    GL_FUNC(ext_glDepthBoundsEXT)(dARG1, dARG2);
    GL_LOG ("glDepthBoundsEXT GLclampd zmin=%f, GLclampd zmax=%f",dARG1,dARG2);
    }
}
void glcommon_glDepthBoundsdNV(CPU* cpu) {
    if (!ext_glDepthBoundsdNV)
        kpanic("ext_glDepthBoundsdNV is NULL");
    {
    GL_FUNC(ext_glDepthBoundsdNV)(dARG1, dARG2);
    GL_LOG ("glDepthBoundsdNV GLdouble zmin=%f, GLdouble zmax=%f",dARG1,dARG2);
    }
}
void glcommon_glDepthRangeArrayv(CPU* cpu) {
    if (!ext_glDepthRangeArrayv)
        kpanic("ext_glDepthRangeArrayv is NULL");
    {
    GL_FUNC(ext_glDepthRangeArrayv)(ARG1, ARG2, marshalArray<GLdouble>(cpu, ARG3, ARG2*2));
    GL_LOG ("glDepthRangeArrayv GLuint first=%d, GLsizei count=%d, const GLdouble* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDepthRangeIndexed(CPU* cpu) {
    if (!ext_glDepthRangeIndexed)
        kpanic("ext_glDepthRangeIndexed is NULL");
    {
    GL_FUNC(ext_glDepthRangeIndexed)(ARG1, dARG2, dARG3);
    GL_LOG ("glDepthRangeIndexed GLuint index=%d, GLdouble n=%f, GLdouble f=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glDepthRangedNV(CPU* cpu) {
    if (!ext_glDepthRangedNV)
        kpanic("ext_glDepthRangedNV is NULL");
    {
    GL_FUNC(ext_glDepthRangedNV)(dARG1, dARG2);
    GL_LOG ("glDepthRangedNV GLdouble zNear=%f, GLdouble zFar=%f",dARG1,dARG2);
    }
}
void glcommon_glDepthRangef(CPU* cpu) {
    if (!ext_glDepthRangef)
        kpanic("ext_glDepthRangef is NULL");
    {
    GL_FUNC(ext_glDepthRangef)(fARG1, fARG2);
    GL_LOG ("glDepthRangef GLfloat n=%f, GLfloat f=%f",fARG1,fARG2);
    }
}
void glcommon_glDepthRangefOES(CPU* cpu) {
    if (!ext_glDepthRangefOES)
        kpanic("ext_glDepthRangefOES is NULL");
    {
    GL_FUNC(ext_glDepthRangefOES)(fARG1, fARG2);
    GL_LOG ("glDepthRangefOES GLclampf n=%f, GLclampf f=%f",fARG1,fARG2);
    }
}
void glcommon_glDepthRangexOES(CPU* cpu) {
    if (!ext_glDepthRangexOES)
        kpanic("ext_glDepthRangexOES is NULL");
    {
    GL_FUNC(ext_glDepthRangexOES)(ARG1, ARG2);
    GL_LOG ("glDepthRangexOES GLfixed n=%d, GLfixed f=%d",ARG1,ARG2);
    }
}
void glcommon_glDetachObjectARB(CPU* cpu) {
    if (!ext_glDetachObjectARB)
        kpanic("ext_glDetachObjectARB is NULL");
    {
    GL_FUNC(ext_glDetachObjectARB)(INDEX_TO_HANDLE(hARG1), INDEX_TO_HANDLE(hARG2));
    GL_LOG ("glDetachObjectARB GLhandleARB containerObj=%d, GLhandleARB attachedObj=%d",ARG1,ARG2);
    }
}
void glcommon_glDetachShader(CPU* cpu) {
    if (!ext_glDetachShader)
        kpanic("ext_glDetachShader is NULL");
    {
    GL_FUNC(ext_glDetachShader)(ARG1, ARG2);
    GL_LOG ("glDetachShader GLuint program=%d, GLuint shader=%d",ARG1,ARG2);
    }
}
void glcommon_glDetailTexFuncSGIS(CPU* cpu) {
    if (!ext_glDetailTexFuncSGIS)
        kpanic("ext_glDetailTexFuncSGIS is NULL");
    {
    GL_FUNC(ext_glDetailTexFuncSGIS)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, ARG2*2));
    GL_LOG ("glDetailTexFuncSGIS GLenum target=%d, GLsizei n=%d, const GLfloat* points=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDisableClientStateIndexedEXT(CPU* cpu) {
    if (!ext_glDisableClientStateIndexedEXT)
        kpanic("ext_glDisableClientStateIndexedEXT is NULL");
    {
    GL_FUNC(ext_glDisableClientStateIndexedEXT)(ARG1, ARG2);
    GL_LOG ("glDisableClientStateIndexedEXT GLenum array=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glDisableClientStateiEXT(CPU* cpu) {
    if (!ext_glDisableClientStateiEXT)
        kpanic("ext_glDisableClientStateiEXT is NULL");
    {
    GL_FUNC(ext_glDisableClientStateiEXT)(ARG1, ARG2);
    GL_LOG ("glDisableClientStateiEXT GLenum array=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glDisableIndexedEXT(CPU* cpu) {
    if (!ext_glDisableIndexedEXT)
        kpanic("ext_glDisableIndexedEXT is NULL");
    {
    GL_FUNC(ext_glDisableIndexedEXT)(ARG1, ARG2);
    GL_LOG ("glDisableIndexedEXT GLenum target=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glDisableVariantClientStateEXT(CPU* cpu) {
    if (!ext_glDisableVariantClientStateEXT)
        kpanic("ext_glDisableVariantClientStateEXT is NULL");
    {
    GL_FUNC(ext_glDisableVariantClientStateEXT)(ARG1);
    GL_LOG ("glDisableVariantClientStateEXT GLuint id=%d",ARG1);
    }
}
void glcommon_glDisableVertexArrayAttrib(CPU* cpu) {
    if (!ext_glDisableVertexArrayAttrib)
        kpanic("ext_glDisableVertexArrayAttrib is NULL");
    {
    GL_FUNC(ext_glDisableVertexArrayAttrib)(ARG1, ARG2);
    GL_LOG ("glDisableVertexArrayAttrib GLuint vaobj=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glDisableVertexArrayAttribEXT(CPU* cpu) {
    if (!ext_glDisableVertexArrayAttribEXT)
        kpanic("ext_glDisableVertexArrayAttribEXT is NULL");
    {
    GL_FUNC(ext_glDisableVertexArrayAttribEXT)(ARG1, ARG2);
    GL_LOG ("glDisableVertexArrayAttribEXT GLuint vaobj=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glDisableVertexArrayEXT(CPU* cpu) {
    if (!ext_glDisableVertexArrayEXT)
        kpanic("ext_glDisableVertexArrayEXT is NULL");
    {
    GL_FUNC(ext_glDisableVertexArrayEXT)(ARG1, ARG2);
    GL_LOG ("glDisableVertexArrayEXT GLuint vaobj=%d, GLenum array=%d",ARG1,ARG2);
    }
}
void glcommon_glDisableVertexAttribAPPLE(CPU* cpu) {
    if (!ext_glDisableVertexAttribAPPLE)
        kpanic("ext_glDisableVertexAttribAPPLE is NULL");
    {
    GL_FUNC(ext_glDisableVertexAttribAPPLE)(ARG1, ARG2);
    GL_LOG ("glDisableVertexAttribAPPLE GLuint index=%d, GLenum pname=%d",ARG1,ARG2);
    }
}
void glcommon_glDisableVertexAttribArray(CPU* cpu) {
    if (!ext_glDisableVertexAttribArray)
        kpanic("ext_glDisableVertexAttribArray is NULL");
    {
    GL_FUNC(ext_glDisableVertexAttribArray)(ARG1);
    GL_LOG ("glDisableVertexAttribArray GLuint index=%d",ARG1);
    }
}
void glcommon_glDisableVertexAttribArrayARB(CPU* cpu) {
    if (!ext_glDisableVertexAttribArrayARB)
        kpanic("ext_glDisableVertexAttribArrayARB is NULL");
    {
    GL_FUNC(ext_glDisableVertexAttribArrayARB)(ARG1);
    GL_LOG ("glDisableVertexAttribArrayARB GLuint index=%d",ARG1);
    }
}
void glcommon_glDisablei(CPU* cpu) {
    if (!ext_glDisablei)
        kpanic("ext_glDisablei is NULL");
    {
    GL_FUNC(ext_glDisablei)(ARG1, ARG2);
    GL_LOG ("glDisablei GLenum target=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glDispatchCompute(CPU* cpu) {
    if (!ext_glDispatchCompute)
        kpanic("ext_glDispatchCompute is NULL");
    {
    GL_FUNC(ext_glDispatchCompute)(ARG1, ARG2, ARG3);
    GL_LOG ("glDispatchCompute GLuint num_groups_x=%d, GLuint num_groups_y=%d, GLuint num_groups_z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDispatchComputeGroupSizeARB(CPU* cpu) {
    if (!ext_glDispatchComputeGroupSizeARB)
        kpanic("ext_glDispatchComputeGroupSizeARB is NULL");
    {
    GL_FUNC(ext_glDispatchComputeGroupSizeARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glDispatchComputeGroupSizeARB GLuint num_groups_x=%d, GLuint num_groups_y=%d, GLuint num_groups_z=%d, GLuint group_size_x=%d, GLuint group_size_y=%d, GLuint group_size_z=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDispatchComputeIndirect(CPU* cpu) {
    if (!ext_glDispatchComputeIndirect)
        kpanic("ext_glDispatchComputeIndirect is NULL");
    {
    GL_FUNC(ext_glDispatchComputeIndirect)(ARG1);
    GL_LOG ("glDispatchComputeIndirect GLintptr indirect=%d",ARG1);
    }
}
void glcommon_glDrawArraysEXT(CPU* cpu) {
    if (!ext_glDrawArraysEXT)
        kpanic("ext_glDrawArraysEXT is NULL");
    {
    GL_FUNC(ext_glDrawArraysEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glDrawArraysEXT GLenum mode=%d, GLint first=%d, GLsizei count=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDrawArraysIndirect(CPU* cpu) {
    if (!ext_glDrawArraysIndirect)
        kpanic("ext_glDrawArraysIndirect is NULL");
    {
    GL_FUNC(ext_glDrawArraysIndirect)(ARG1, marshalArray<GLuint>(cpu, ARG2, 4));
    GL_LOG ("glDrawArraysIndirect GLenum mode=%d, const void* indirect=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDrawArraysInstanced(CPU* cpu) {
    if (!ext_glDrawArraysInstanced)
        kpanic("ext_glDrawArraysInstanced is NULL");
    {
    GL_FUNC(ext_glDrawArraysInstanced)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glDrawArraysInstanced GLenum mode=%d, GLint first=%d, GLsizei count=%d, GLsizei instancecount=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glDrawArraysInstancedARB(CPU* cpu) {
    if (!ext_glDrawArraysInstancedARB)
        kpanic("ext_glDrawArraysInstancedARB is NULL");
    {
    GL_FUNC(ext_glDrawArraysInstancedARB)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glDrawArraysInstancedARB GLenum mode=%d, GLint first=%d, GLsizei count=%d, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glDrawArraysInstancedBaseInstance(CPU* cpu) {
    if (!ext_glDrawArraysInstancedBaseInstance)
        kpanic("ext_glDrawArraysInstancedBaseInstance is NULL");
    {
    GL_FUNC(ext_glDrawArraysInstancedBaseInstance)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glDrawArraysInstancedBaseInstance GLenum mode=%d, GLint first=%d, GLsizei count=%d, GLsizei instancecount=%d, GLuint baseinstance=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawArraysInstancedEXT(CPU* cpu) {
    if (!ext_glDrawArraysInstancedEXT)
        kpanic("ext_glDrawArraysInstancedEXT is NULL");
    {
    GL_FUNC(ext_glDrawArraysInstancedEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glDrawArraysInstancedEXT GLenum mode=%d, GLint start=%d, GLsizei count=%d, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glDrawBufferRegion(CPU* cpu) {
    if (!ext_glDrawBufferRegion)
        kpanic("ext_glDrawBufferRegion is NULL");
    {
    GL_FUNC(ext_glDrawBufferRegion)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glDrawBufferRegion GLenum region=%d, GLint x=%d, GLint y=%d, GLsizei width=%d, GLsizei height=%d, GLint xDest=%d, GLint yDest=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glDrawBuffers(CPU* cpu) {
    if (!ext_glDrawBuffers)
        kpanic("ext_glDrawBuffers is NULL");
    {
    GL_FUNC(ext_glDrawBuffers)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDrawBuffers GLsizei n=%d, const GLenum* bufs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDrawBuffersARB(CPU* cpu) {
    if (!ext_glDrawBuffersARB)
        kpanic("ext_glDrawBuffersARB is NULL");
    {
    GL_FUNC(ext_glDrawBuffersARB)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDrawBuffersARB GLsizei n=%d, const GLenum* bufs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDrawBuffersATI(CPU* cpu) {
    if (!ext_glDrawBuffersATI)
        kpanic("ext_glDrawBuffersATI is NULL");
    {
    GL_FUNC(ext_glDrawBuffersATI)(ARG1, marshalArray<GLuint>(cpu, ARG2, ARG1));
    GL_LOG ("glDrawBuffersATI GLsizei n=%d, const GLenum* bufs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glDrawCommandsAddressNV(CPU* cpu) {
    if (!ext_glDrawCommandsAddressNV)
        kpanic("ext_glDrawCommandsAddressNV is NULL");
    {
    GL_FUNC(ext_glDrawCommandsAddressNV)(ARG1, marshalArray<GLuint64>(cpu, ARG2, ARG4), marshalArray<GLint>(cpu, ARG3, ARG4), ARG4);
    GL_LOG ("glDrawCommandsAddressNV GLenum primitiveMode=%d, const GLuint64* indirects=%.08x, const GLsizei* sizes=%.08x, GLuint count=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glDrawCommandsNV(CPU* cpu) {
    if (!ext_glDrawCommandsNV)
        kpanic("ext_glDrawCommandsNV is NULL");
    {
    GL_FUNC(ext_glDrawCommandsNV)(ARG1, ARG2, marshalip(cpu, ARG3, ARG5), marshalArray<GLint>(cpu, ARG4, ARG5), ARG5);
    GL_LOG ("glDrawCommandsNV GLenum primitiveMode=%d, GLuint buffer=%d, const GLintptr* indirects=%.08x, const GLsizei* sizes=%.08x, GLuint count=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawCommandsStatesAddressNV(CPU* cpu) {
    if (!ext_glDrawCommandsStatesAddressNV)
        kpanic("ext_glDrawCommandsStatesAddressNV is NULL");
    {
    GL_FUNC(ext_glDrawCommandsStatesAddressNV)(marshalArray<GLuint64>(cpu, ARG1, ARG5), marshalArray<GLint>(cpu, ARG2, ARG5), marshalArray<GLuint>(cpu, ARG3, ARG5), marshalArray<GLuint>(cpu, ARG4, ARG5), ARG5);
    GL_LOG ("glDrawCommandsStatesAddressNV const GLuint64* indirects=%.08x, const GLsizei* sizes=%.08x, const GLuint* states=%.08x, const GLuint* fbos=%.08x, GLuint count=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawCommandsStatesNV(CPU* cpu) {
    if (!ext_glDrawCommandsStatesNV)
        kpanic("ext_glDrawCommandsStatesNV is NULL");
    {
    GL_FUNC(ext_glDrawCommandsStatesNV)(ARG1, marshalip(cpu, ARG2, ARG6), marshalArray<GLint>(cpu, ARG3, ARG6), marshalArray<GLuint>(cpu, ARG4, ARG6), marshalArray<GLuint>(cpu, ARG5, ARG6), ARG6);
    GL_LOG ("glDrawCommandsStatesNV GLuint buffer=%d, const GLintptr* indirects=%.08x, const GLsizei* sizes=%.08x, const GLuint* states=%.08x, const GLuint* fbos=%.08x, GLuint count=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDrawElementArrayAPPLE(CPU* cpu) {
    if (!ext_glDrawElementArrayAPPLE)
        kpanic("ext_glDrawElementArrayAPPLE is NULL");
    {
    GL_FUNC(ext_glDrawElementArrayAPPLE)(ARG1, ARG2, ARG3);
    GL_LOG ("glDrawElementArrayAPPLE GLenum mode=%d, GLint first=%d, GLsizei count=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDrawElementArrayATI(CPU* cpu) {
    if (!ext_glDrawElementArrayATI)
        kpanic("ext_glDrawElementArrayATI is NULL");
    {
    GL_FUNC(ext_glDrawElementArrayATI)(ARG1, ARG2);
    GL_LOG ("glDrawElementArrayATI GLenum mode=%d, GLsizei count=%d",ARG1,ARG2);
    }
}
void glcommon_glDrawElementsBaseVertex(CPU* cpu) {
    if (!ext_glDrawElementsBaseVertex)
        kpanic("ext_glDrawElementsBaseVertex is NULL");
    {
    GL_FUNC(ext_glDrawElementsBaseVertex)(ARG1, ARG2, ARG3, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG4:marshalType(cpu, ARG3, ARG2, ARG4), ARG5);
    GL_LOG ("glDrawElementsBaseVertex GLenum mode=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLint basevertex=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawElementsIndirect(CPU* cpu) {
    if (!ext_glDrawElementsIndirect)
        kpanic("ext_glDrawElementsIndirect is NULL");
    {
    GL_FUNC(ext_glDrawElementsIndirect)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, 5));
    GL_LOG ("glDrawElementsIndirect GLenum mode=%d, GLenum type=%d, const void* indirect=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDrawElementsInstanced(CPU* cpu) {
    if (!ext_glDrawElementsInstanced)
        kpanic("ext_glDrawElementsInstanced is NULL");
    {
    GL_FUNC(ext_glDrawElementsInstanced)(ARG1, ARG2, ARG3, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG4:marshalType(cpu, ARG3, ARG2, ARG4), ARG5);
    GL_LOG ("glDrawElementsInstanced GLenum mode=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLsizei instancecount=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawElementsInstancedARB(CPU* cpu) {
    if (!ext_glDrawElementsInstancedARB)
        kpanic("ext_glDrawElementsInstancedARB is NULL");
    {
    GL_FUNC(ext_glDrawElementsInstancedARB)(ARG1, ARG2, ARG3, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG4:marshalType(cpu, ARG3, ARG2, ARG4), ARG5);
    GL_LOG ("glDrawElementsInstancedARB GLenum mode=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawElementsInstancedBaseInstance(CPU* cpu) {
    if (!ext_glDrawElementsInstancedBaseInstance)
        kpanic("ext_glDrawElementsInstancedBaseInstance is NULL");
    {
    GL_FUNC(ext_glDrawElementsInstancedBaseInstance)(ARG1, ARG2, ARG3, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG4:marshalType(cpu, ARG3, ARG2, ARG4), ARG5, ARG6);
    GL_LOG ("glDrawElementsInstancedBaseInstance GLenum mode=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLsizei instancecount=%d, GLuint baseinstance=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDrawElementsInstancedBaseVertex(CPU* cpu) {
    if (!ext_glDrawElementsInstancedBaseVertex)
        kpanic("ext_glDrawElementsInstancedBaseVertex is NULL");
    {
    GL_FUNC(ext_glDrawElementsInstancedBaseVertex)(ARG1, ARG2, ARG3, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG4:marshalType(cpu, ARG3, ARG2, ARG4), ARG5, ARG6);
    GL_LOG ("glDrawElementsInstancedBaseVertex GLenum mode=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLsizei instancecount=%d, GLint basevertex=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDrawElementsInstancedBaseVertexBaseInstance(CPU* cpu) {
    if (!ext_glDrawElementsInstancedBaseVertexBaseInstance)
        kpanic("ext_glDrawElementsInstancedBaseVertexBaseInstance is NULL");
    {
    GL_FUNC(ext_glDrawElementsInstancedBaseVertexBaseInstance)(ARG1, ARG2, ARG3, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG4:marshalType(cpu, ARG3, ARG2, ARG4), ARG5, ARG6, ARG7);
    GL_LOG ("glDrawElementsInstancedBaseVertexBaseInstance GLenum mode=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLsizei instancecount=%d, GLint basevertex=%d, GLuint baseinstance=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glDrawElementsInstancedEXT(CPU* cpu) {
    if (!ext_glDrawElementsInstancedEXT)
        kpanic("ext_glDrawElementsInstancedEXT is NULL");
    {
    GL_FUNC(ext_glDrawElementsInstancedEXT)(ARG1, ARG2, ARG3, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG4:marshalType(cpu, ARG3, ARG2, ARG4), ARG5);
    GL_LOG ("glDrawElementsInstancedEXT GLenum mode=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLsizei primcount=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawMeshArraysSUN(CPU* cpu) {
    if (!ext_glDrawMeshArraysSUN)
        kpanic("ext_glDrawMeshArraysSUN is NULL");
    {
    GL_FUNC(ext_glDrawMeshArraysSUN)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glDrawMeshArraysSUN GLenum mode=%d, GLint first=%d, GLsizei count=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glDrawRangeElementArrayAPPLE(CPU* cpu) {
    if (!ext_glDrawRangeElementArrayAPPLE)
        kpanic("ext_glDrawRangeElementArrayAPPLE is NULL");
    {
    GL_FUNC(ext_glDrawRangeElementArrayAPPLE)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glDrawRangeElementArrayAPPLE GLenum mode=%d, GLuint start=%d, GLuint end=%d, GLint first=%d, GLsizei count=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glDrawRangeElementArrayATI(CPU* cpu) {
    if (!ext_glDrawRangeElementArrayATI)
        kpanic("ext_glDrawRangeElementArrayATI is NULL");
    {
    GL_FUNC(ext_glDrawRangeElementArrayATI)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glDrawRangeElementArrayATI GLenum mode=%d, GLuint start=%d, GLuint end=%d, GLsizei count=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glDrawRangeElements(CPU* cpu) {
    if (!ext_glDrawRangeElements)
        kpanic("ext_glDrawRangeElements is NULL");
    {
    GL_FUNC(ext_glDrawRangeElements)(ARG1, ARG2, ARG3, ARG4, ARG5, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, ARG4, ARG6));
    GL_LOG ("glDrawRangeElements GLenum mode=%d, GLuint start=%d, GLuint end=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDrawRangeElementsBaseVertex(CPU* cpu) {
    if (!ext_glDrawRangeElementsBaseVertex)
        kpanic("ext_glDrawRangeElementsBaseVertex is NULL");
    {
    GL_FUNC(ext_glDrawRangeElementsBaseVertex)(ARG1, ARG2, ARG3, ARG4, ARG5, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, ARG4, ARG6), ARG7);
    GL_LOG ("glDrawRangeElementsBaseVertex GLenum mode=%d, GLuint start=%d, GLuint end=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x, GLint basevertex=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glDrawRangeElementsEXT(CPU* cpu) {
    if (!ext_glDrawRangeElementsEXT)
        kpanic("ext_glDrawRangeElementsEXT is NULL");
    {
    GL_FUNC(ext_glDrawRangeElementsEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ELEMENT_ARRAY_BUFFER()?(GLvoid*)pARG6:marshalType(cpu, ARG5, ARG4, ARG6));
    GL_LOG ("glDrawRangeElementsEXT GLenum mode=%d, GLuint start=%d, GLuint end=%d, GLsizei count=%d, GLenum type=%d, const void* indices=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glDrawTextureNV(CPU* cpu) {
    if (!ext_glDrawTextureNV)
        kpanic("ext_glDrawTextureNV is NULL");
    {
    GL_FUNC(ext_glDrawTextureNV)(ARG1, ARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8, fARG9, fARG10, fARG11);
    GL_LOG ("glDrawTextureNV GLuint texture=%d, GLuint sampler=%d, GLfloat x0=%f, GLfloat y0=%f, GLfloat x1=%f, GLfloat y1=%f, GLfloat z=%f, GLfloat s0=%f, GLfloat t0=%f, GLfloat s1=%f, GLfloat t1=%f",ARG1,ARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8,fARG9,fARG10,fARG11);
    }
}
void glcommon_glDrawTransformFeedback(CPU* cpu) {
    if (!ext_glDrawTransformFeedback)
        kpanic("ext_glDrawTransformFeedback is NULL");
    {
    GL_FUNC(ext_glDrawTransformFeedback)(ARG1, ARG2);
    GL_LOG ("glDrawTransformFeedback GLenum mode=%d, GLuint id=%d",ARG1,ARG2);
    }
}
void glcommon_glDrawTransformFeedbackInstanced(CPU* cpu) {
    if (!ext_glDrawTransformFeedbackInstanced)
        kpanic("ext_glDrawTransformFeedbackInstanced is NULL");
    {
    GL_FUNC(ext_glDrawTransformFeedbackInstanced)(ARG1, ARG2, ARG3);
    GL_LOG ("glDrawTransformFeedbackInstanced GLenum mode=%d, GLuint id=%d, GLsizei instancecount=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDrawTransformFeedbackNV(CPU* cpu) {
    if (!ext_glDrawTransformFeedbackNV)
        kpanic("ext_glDrawTransformFeedbackNV is NULL");
    {
    GL_FUNC(ext_glDrawTransformFeedbackNV)(ARG1, ARG2);
    GL_LOG ("glDrawTransformFeedbackNV GLenum mode=%d, GLuint id=%d",ARG1,ARG2);
    }
}
void glcommon_glDrawTransformFeedbackStream(CPU* cpu) {
    if (!ext_glDrawTransformFeedbackStream)
        kpanic("ext_glDrawTransformFeedbackStream is NULL");
    {
    GL_FUNC(ext_glDrawTransformFeedbackStream)(ARG1, ARG2, ARG3);
    GL_LOG ("glDrawTransformFeedbackStream GLenum mode=%d, GLuint id=%d, GLuint stream=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glDrawTransformFeedbackStreamInstanced(CPU* cpu) {
    if (!ext_glDrawTransformFeedbackStreamInstanced)
        kpanic("ext_glDrawTransformFeedbackStreamInstanced is NULL");
    {
    GL_FUNC(ext_glDrawTransformFeedbackStreamInstanced)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glDrawTransformFeedbackStreamInstanced GLenum mode=%d, GLuint id=%d, GLuint stream=%d, GLsizei instancecount=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glEdgeFlagFormatNV(CPU* cpu) {
    if (!ext_glEdgeFlagFormatNV)
        kpanic("ext_glEdgeFlagFormatNV is NULL");
    {
    GL_FUNC(ext_glEdgeFlagFormatNV)(ARG1);
    GL_LOG ("glEdgeFlagFormatNV GLsizei stride=%d",ARG1);
    }
}
void glcommon_glEdgeFlagPointerEXT(CPU* cpu) {
    if (!ext_glEdgeFlagPointerEXT)
        kpanic("ext_glEdgeFlagPointerEXT is NULL");
    {
    GL_FUNC(ext_glEdgeFlagPointerEXT)(ARG1, ARG2, marshalEdgeFlagPointerEXT(cpu, ARG1, ARG2, ARG3));
    GL_LOG ("glEdgeFlagPointerEXT GLsizei stride=%d, GLsizei count=%d, const GLboolean* pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glEdgeFlagPointerListIBM(CPU* cpu) {
    if (!ext_glEdgeFlagPointerListIBM)
        kpanic("ext_glEdgeFlagPointerListIBM is NULL");
    {
    GL_FUNC(ext_glEdgeFlagPointerListIBM)(ARG1, (const GLboolean**)marshalunhandled("glEdgeFlagPointerListIBM", "pointer", cpu, ARG2), ARG3);
    GL_LOG ("glEdgeFlagPointerListIBM GLint stride=%d, const GLboolean** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glElementPointerAPPLE(CPU* cpu) {
    if (!ext_glElementPointerAPPLE)
        kpanic("ext_glElementPointerAPPLE is NULL");
    {
    GL_FUNC(ext_glElementPointerAPPLE)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glElementPointerAPPLE GLenum type=%d, const void* pointer=%.08x",ARG1,ARG2);
    }
}
void glcommon_glElementPointerATI(CPU* cpu) {
    if (!ext_glElementPointerATI)
        kpanic("ext_glElementPointerATI is NULL");
    {
    GL_FUNC(ext_glElementPointerATI)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glElementPointerATI GLenum type=%d, const void* pointer=%.08x",ARG1,ARG2);
    }
}
void glcommon_glEnableClientStateIndexedEXT(CPU* cpu) {
    if (!ext_glEnableClientStateIndexedEXT)
        kpanic("ext_glEnableClientStateIndexedEXT is NULL");
    {
    GL_FUNC(ext_glEnableClientStateIndexedEXT)(ARG1, ARG2);
    GL_LOG ("glEnableClientStateIndexedEXT GLenum array=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glEnableClientStateiEXT(CPU* cpu) {
    if (!ext_glEnableClientStateiEXT)
        kpanic("ext_glEnableClientStateiEXT is NULL");
    {
    GL_FUNC(ext_glEnableClientStateiEXT)(ARG1, ARG2);
    GL_LOG ("glEnableClientStateiEXT GLenum array=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glEnableIndexedEXT(CPU* cpu) {
    if (!ext_glEnableIndexedEXT)
        kpanic("ext_glEnableIndexedEXT is NULL");
    {
    GL_FUNC(ext_glEnableIndexedEXT)(ARG1, ARG2);
    GL_LOG ("glEnableIndexedEXT GLenum target=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glEnableVariantClientStateEXT(CPU* cpu) {
    if (!ext_glEnableVariantClientStateEXT)
        kpanic("ext_glEnableVariantClientStateEXT is NULL");
    {
    GL_FUNC(ext_glEnableVariantClientStateEXT)(ARG1);
    GL_LOG ("glEnableVariantClientStateEXT GLuint id=%d",ARG1);
    }
}
void glcommon_glEnableVertexArrayAttrib(CPU* cpu) {
    if (!ext_glEnableVertexArrayAttrib)
        kpanic("ext_glEnableVertexArrayAttrib is NULL");
    {
    GL_FUNC(ext_glEnableVertexArrayAttrib)(ARG1, ARG2);
    GL_LOG ("glEnableVertexArrayAttrib GLuint vaobj=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glEnableVertexArrayAttribEXT(CPU* cpu) {
    if (!ext_glEnableVertexArrayAttribEXT)
        kpanic("ext_glEnableVertexArrayAttribEXT is NULL");
    {
    GL_FUNC(ext_glEnableVertexArrayAttribEXT)(ARG1, ARG2);
    GL_LOG ("glEnableVertexArrayAttribEXT GLuint vaobj=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glEnableVertexArrayEXT(CPU* cpu) {
    if (!ext_glEnableVertexArrayEXT)
        kpanic("ext_glEnableVertexArrayEXT is NULL");
    {
    GL_FUNC(ext_glEnableVertexArrayEXT)(ARG1, ARG2);
    GL_LOG ("glEnableVertexArrayEXT GLuint vaobj=%d, GLenum array=%d",ARG1,ARG2);
    }
}
void glcommon_glEnableVertexAttribAPPLE(CPU* cpu) {
    if (!ext_glEnableVertexAttribAPPLE)
        kpanic("ext_glEnableVertexAttribAPPLE is NULL");
    {
    GL_FUNC(ext_glEnableVertexAttribAPPLE)(ARG1, ARG2);
    GL_LOG ("glEnableVertexAttribAPPLE GLuint index=%d, GLenum pname=%d",ARG1,ARG2);
    }
}
void glcommon_glEnableVertexAttribArray(CPU* cpu) {
    if (!ext_glEnableVertexAttribArray)
        kpanic("ext_glEnableVertexAttribArray is NULL");
    {
    GL_FUNC(ext_glEnableVertexAttribArray)(ARG1);
    GL_LOG ("glEnableVertexAttribArray GLuint index=%d",ARG1);
    }
}
void glcommon_glEnableVertexAttribArrayARB(CPU* cpu) {
    if (!ext_glEnableVertexAttribArrayARB)
        kpanic("ext_glEnableVertexAttribArrayARB is NULL");
    {
    GL_FUNC(ext_glEnableVertexAttribArrayARB)(ARG1);
    GL_LOG ("glEnableVertexAttribArrayARB GLuint index=%d",ARG1);
    }
}
void glcommon_glEnablei(CPU* cpu) {
    if (!ext_glEnablei)
        kpanic("ext_glEnablei is NULL");
    {
    GL_FUNC(ext_glEnablei)(ARG1, ARG2);
    GL_LOG ("glEnablei GLenum target=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glEndConditionalRender(CPU* cpu) {
    if (!ext_glEndConditionalRender)
        kpanic("ext_glEndConditionalRender is NULL");
    {
    GL_FUNC(ext_glEndConditionalRender)();
    GL_LOG ("glEndConditionalRender");
    }
}
void glcommon_glEndConditionalRenderNV(CPU* cpu) {
    if (!ext_glEndConditionalRenderNV)
        kpanic("ext_glEndConditionalRenderNV is NULL");
    {
    GL_FUNC(ext_glEndConditionalRenderNV)();
    GL_LOG ("glEndConditionalRenderNV");
    }
}
void glcommon_glEndConditionalRenderNVX(CPU* cpu) {
    if (!ext_glEndConditionalRenderNVX)
        kpanic("ext_glEndConditionalRenderNVX is NULL");
    {
    GL_FUNC(ext_glEndConditionalRenderNVX)();
    GL_LOG ("glEndConditionalRenderNVX");
    }
}
void glcommon_glEndFragmentShaderATI(CPU* cpu) {
    if (!ext_glEndFragmentShaderATI)
        kpanic("ext_glEndFragmentShaderATI is NULL");
    {
    GL_FUNC(ext_glEndFragmentShaderATI)();
    GL_LOG ("glEndFragmentShaderATI");
    }
}
void glcommon_glEndOcclusionQueryNV(CPU* cpu) {
    if (!ext_glEndOcclusionQueryNV)
        kpanic("ext_glEndOcclusionQueryNV is NULL");
    {
    GL_FUNC(ext_glEndOcclusionQueryNV)();
    GL_LOG ("glEndOcclusionQueryNV");
    }
}
void glcommon_glEndPerfMonitorAMD(CPU* cpu) {
    if (!ext_glEndPerfMonitorAMD)
        kpanic("ext_glEndPerfMonitorAMD is NULL");
    {
    GL_FUNC(ext_glEndPerfMonitorAMD)(ARG1);
    GL_LOG ("glEndPerfMonitorAMD GLuint monitor=%d",ARG1);
    }
}
void glcommon_glEndPerfQueryINTEL(CPU* cpu) {
    if (!ext_glEndPerfQueryINTEL)
        kpanic("ext_glEndPerfQueryINTEL is NULL");
    {
    GL_FUNC(ext_glEndPerfQueryINTEL)(ARG1);
    GL_LOG ("glEndPerfQueryINTEL GLuint queryHandle=%d",ARG1);
    }
}
void glcommon_glEndQuery(CPU* cpu) {
    if (!ext_glEndQuery)
        kpanic("ext_glEndQuery is NULL");
    {
    GL_FUNC(ext_glEndQuery)(ARG1);
    GL_LOG ("glEndQuery GLenum target=%d",ARG1);
    }
}
void glcommon_glEndQueryARB(CPU* cpu) {
    if (!ext_glEndQueryARB)
        kpanic("ext_glEndQueryARB is NULL");
    {
    GL_FUNC(ext_glEndQueryARB)(ARG1);
    GL_LOG ("glEndQueryARB GLenum target=%d",ARG1);
    }
}
void glcommon_glEndQueryIndexed(CPU* cpu) {
    if (!ext_glEndQueryIndexed)
        kpanic("ext_glEndQueryIndexed is NULL");
    {
    GL_FUNC(ext_glEndQueryIndexed)(ARG1, ARG2);
    GL_LOG ("glEndQueryIndexed GLenum target=%d, GLuint index=%d",ARG1,ARG2);
    }
}
void glcommon_glEndTransformFeedback(CPU* cpu) {
    if (!ext_glEndTransformFeedback)
        kpanic("ext_glEndTransformFeedback is NULL");
    {
    GL_FUNC(ext_glEndTransformFeedback)();
    GL_LOG ("glEndTransformFeedback");
    }
}
void glcommon_glEndTransformFeedbackEXT(CPU* cpu) {
    if (!ext_glEndTransformFeedbackEXT)
        kpanic("ext_glEndTransformFeedbackEXT is NULL");
    {
    GL_FUNC(ext_glEndTransformFeedbackEXT)();
    GL_LOG ("glEndTransformFeedbackEXT");
    }
}
void glcommon_glEndTransformFeedbackNV(CPU* cpu) {
    if (!ext_glEndTransformFeedbackNV)
        kpanic("ext_glEndTransformFeedbackNV is NULL");
    {
    GL_FUNC(ext_glEndTransformFeedbackNV)();
    GL_LOG ("glEndTransformFeedbackNV");
    }
}
void glcommon_glEndVertexShaderEXT(CPU* cpu) {
    if (!ext_glEndVertexShaderEXT)
        kpanic("ext_glEndVertexShaderEXT is NULL");
    {
    GL_FUNC(ext_glEndVertexShaderEXT)();
    GL_LOG ("glEndVertexShaderEXT");
    }
}
void glcommon_glEndVideoCaptureNV(CPU* cpu) {
    if (!ext_glEndVideoCaptureNV)
        kpanic("ext_glEndVideoCaptureNV is NULL");
    {
    GL_FUNC(ext_glEndVideoCaptureNV)(ARG1);
    GL_LOG ("glEndVideoCaptureNV GLuint video_capture_slot=%d",ARG1);
    }
}
void glcommon_glEvalCoord1xOES(CPU* cpu) {
    if (!ext_glEvalCoord1xOES)
        kpanic("ext_glEvalCoord1xOES is NULL");
    {
    GL_FUNC(ext_glEvalCoord1xOES)(ARG1);
    GL_LOG ("glEvalCoord1xOES GLfixed u=%d",ARG1);
    }
}
void glcommon_glEvalCoord1xvOES(CPU* cpu) {
    if (!ext_glEvalCoord1xvOES)
        kpanic("ext_glEvalCoord1xvOES is NULL");
    {
    GL_FUNC(ext_glEvalCoord1xvOES)(marshalArray<GLint>(cpu, ARG1, 1));
    GL_LOG ("glEvalCoord1xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glEvalCoord2xOES(CPU* cpu) {
    if (!ext_glEvalCoord2xOES)
        kpanic("ext_glEvalCoord2xOES is NULL");
    {
    GL_FUNC(ext_glEvalCoord2xOES)(ARG1, ARG2);
    GL_LOG ("glEvalCoord2xOES GLfixed u=%d, GLfixed v=%d",ARG1,ARG2);
    }
}
void glcommon_glEvalCoord2xvOES(CPU* cpu) {
    if (!ext_glEvalCoord2xvOES)
        kpanic("ext_glEvalCoord2xvOES is NULL");
    {
    GL_FUNC(ext_glEvalCoord2xvOES)(marshalArray<GLint>(cpu, ARG1, 2));
    GL_LOG ("glEvalCoord2xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glEvalMapsNV(CPU* cpu) {
    if (!ext_glEvalMapsNV)
        kpanic("ext_glEvalMapsNV is NULL");
    {
    GL_FUNC(ext_glEvalMapsNV)(ARG1, ARG2);
    GL_LOG ("glEvalMapsNV GLenum target=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glEvaluateDepthValuesARB(CPU* cpu) {
    if (!ext_glEvaluateDepthValuesARB)
        kpanic("ext_glEvaluateDepthValuesARB is NULL");
    {
    GL_FUNC(ext_glEvaluateDepthValuesARB)();
    GL_LOG ("glEvaluateDepthValuesARB");
    }
}
void glcommon_glExecuteProgramNV(CPU* cpu) {
    if (!ext_glExecuteProgramNV)
        kpanic("ext_glExecuteProgramNV is NULL");
    {
    GL_FUNC(ext_glExecuteProgramNV)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, 4));
    GL_LOG ("glExecuteProgramNV GLenum target=%d, GLuint id=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glExtractComponentEXT(CPU* cpu) {
    if (!ext_glExtractComponentEXT)
        kpanic("ext_glExtractComponentEXT is NULL");
    {
    GL_FUNC(ext_glExtractComponentEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glExtractComponentEXT GLuint res=%d, GLuint src=%d, GLuint num=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFeedbackBufferxOES(CPU* cpu) {
    if (!ext_glFeedbackBufferxOES)
        kpanic("ext_glFeedbackBufferxOES is NULL");
    {
    GL_FUNC(ext_glFeedbackBufferxOES)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, ARG1));
    GL_LOG ("glFeedbackBufferxOES GLsizei n=%d, GLenum type=%d, const GLfixed* buffer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFenceSync(CPU* cpu) {
    if (!ext_glFenceSync)
        kpanic("ext_glFenceSync is NULL");
    {
    GLsync ret=GL_FUNC(ext_glFenceSync)(ARG1, ARG2);
    EAX=marshalBackSync(cpu, ret);
    GL_LOG ("glFenceSync GLenum condition=%d, GLbitfield flags=%d",ARG1,ARG2);
    }
}
void glcommon_glFinalCombinerInputNV(CPU* cpu) {
    if (!ext_glFinalCombinerInputNV)
        kpanic("ext_glFinalCombinerInputNV is NULL");
    {
    GL_FUNC(ext_glFinalCombinerInputNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glFinalCombinerInputNV GLenum variable=%d, GLenum input=%d, GLenum mapping=%d, GLenum componentUsage=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFinishAsyncSGIX(CPU* cpu) {
    if (!ext_glFinishAsyncSGIX)
        kpanic("ext_glFinishAsyncSGIX is NULL");
    {
        MarshalReadWrite<GLuint> markerp(cpu, ARG1, 1);
        EAX=GL_FUNC(ext_glFinishAsyncSGIX)(markerp.getPtr());
        GL_LOG ("glFinishAsyncSGIX GLuint* markerp=%.08x",ARG1);
    }
}
void glcommon_glFinishFenceAPPLE(CPU* cpu) {
    if (!ext_glFinishFenceAPPLE)
        kpanic("ext_glFinishFenceAPPLE is NULL");
    {
    GL_FUNC(ext_glFinishFenceAPPLE)(ARG1);
    GL_LOG ("glFinishFenceAPPLE GLuint fence=%d",ARG1);
    }
}
void glcommon_glFinishFenceNV(CPU* cpu) {
    if (!ext_glFinishFenceNV)
        kpanic("ext_glFinishFenceNV is NULL");
    {
    GL_FUNC(ext_glFinishFenceNV)(ARG1);
    GL_LOG ("glFinishFenceNV GLuint fence=%d",ARG1);
    }
}
void glcommon_glFinishObjectAPPLE(CPU* cpu) {
    if (!ext_glFinishObjectAPPLE)
        kpanic("ext_glFinishObjectAPPLE is NULL");
    {
    GL_FUNC(ext_glFinishObjectAPPLE)(ARG1, ARG2);
    GL_LOG ("glFinishObjectAPPLE GLenum object=%d, GLint name=%d",ARG1,ARG2);
    }
}
void glcommon_glFinishTextureSUNX(CPU* cpu) {
    if (!ext_glFinishTextureSUNX)
        kpanic("ext_glFinishTextureSUNX is NULL");
    {
    GL_FUNC(ext_glFinishTextureSUNX)();
    GL_LOG ("glFinishTextureSUNX");
    }
}
void glcommon_glFlushMappedBufferRange(CPU* cpu) {
    if (!ext_glFlushMappedBufferRange)
        kpanic("ext_glFlushMappedBufferRange is NULL");
    {

    GL_FUNC(ext_glFlushMappedBufferRange)(ARG1, ARG2, ARG3);
    GL_LOG ("glFlushMappedBufferRange GLenum target=%d, GLintptr offset=%d, GLsizeiptr length=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFlushMappedBufferRangeAPPLE(CPU* cpu) {
    if (!ext_glFlushMappedBufferRangeAPPLE)
        kpanic("ext_glFlushMappedBufferRangeAPPLE is NULL");
    {
    GL_FUNC(ext_glFlushMappedBufferRangeAPPLE)(ARG1, ARG2, ARG3);
    GL_LOG ("glFlushMappedBufferRangeAPPLE GLenum target=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFlushMappedNamedBufferRange(CPU* cpu) {
    if (!ext_glFlushMappedNamedBufferRange)
        kpanic("ext_glFlushMappedNamedBufferRange is NULL");
    {
    GL_FUNC(ext_glFlushMappedNamedBufferRange)(ARG1, ARG2, ARG3);
    GL_LOG ("glFlushMappedNamedBufferRange GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr length=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFlushMappedNamedBufferRangeEXT(CPU* cpu) {
    if (!ext_glFlushMappedNamedBufferRangeEXT)
        kpanic("ext_glFlushMappedNamedBufferRangeEXT is NULL");
    {
    GL_FUNC(ext_glFlushMappedNamedBufferRangeEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glFlushMappedNamedBufferRangeEXT GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr length=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFlushPixelDataRangeNV(CPU* cpu) {
    if (!ext_glFlushPixelDataRangeNV)
        kpanic("ext_glFlushPixelDataRangeNV is NULL");
    {
    GL_FUNC(ext_glFlushPixelDataRangeNV)(ARG1);
    GL_LOG ("glFlushPixelDataRangeNV GLenum target=%d",ARG1);
    }
}
void glcommon_glFlushRasterSGIX(CPU* cpu) {
    if (!ext_glFlushRasterSGIX)
        kpanic("ext_glFlushRasterSGIX is NULL");
    {
    GL_FUNC(ext_glFlushRasterSGIX)();
    GL_LOG ("glFlushRasterSGIX");
    }
}
void glcommon_glFlushStaticDataIBM(CPU* cpu) {
    if (!ext_glFlushStaticDataIBM)
        kpanic("ext_glFlushStaticDataIBM is NULL");
    {
    GL_FUNC(ext_glFlushStaticDataIBM)(ARG1);
    GL_LOG ("glFlushStaticDataIBM GLenum target=%d",ARG1);
    }
}
void glcommon_glFlushVertexArrayRangeAPPLE(CPU* cpu) {
    if (!ext_glFlushVertexArrayRangeAPPLE)
        kpanic("ext_glFlushVertexArrayRangeAPPLE is NULL");
    {
    GL_FUNC(ext_glFlushVertexArrayRangeAPPLE)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glFlushVertexArrayRangeAPPLE GLsizei length=%d, void* pointer=%.08x",ARG1,ARG2);
    }
}
void glcommon_glFlushVertexArrayRangeNV(CPU* cpu) {
    if (!ext_glFlushVertexArrayRangeNV)
        kpanic("ext_glFlushVertexArrayRangeNV is NULL");
    {
    GL_FUNC(ext_glFlushVertexArrayRangeNV)();
    GL_LOG ("glFlushVertexArrayRangeNV");
    }
}
void glcommon_glFogCoordFormatNV(CPU* cpu) {
    if (!ext_glFogCoordFormatNV)
        kpanic("ext_glFogCoordFormatNV is NULL");
    {
    GL_FUNC(ext_glFogCoordFormatNV)(ARG1, ARG2);
    GL_LOG ("glFogCoordFormatNV GLenum type=%d, GLsizei stride=%d",ARG1,ARG2);
    }
}
void glcommon_glFogCoordPointer(CPU* cpu) {
    if (!ext_glFogCoordPointer)
        kpanic("ext_glFogCoordPointer is NULL");
    {
    GL_FUNC(ext_glFogCoordPointer)(ARG1, ARG2, marshalFogPointer(cpu, ARG1, ARG2, ARG3));
    GL_LOG ("glFogCoordPointer GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFogCoordPointerEXT(CPU* cpu) {
    if (!ext_glFogCoordPointerEXT)
        kpanic("ext_glFogCoordPointerEXT is NULL");
    {
    GL_FUNC(ext_glFogCoordPointerEXT)(ARG1, ARG2, marshalFogPointerEXT(cpu, ARG1, ARG2, ARG3));
    GL_LOG ("glFogCoordPointerEXT GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFogCoordPointerListIBM(CPU* cpu) {
    if (!ext_glFogCoordPointerListIBM)
        kpanic("ext_glFogCoordPointerListIBM is NULL");
    {
    GL_FUNC(ext_glFogCoordPointerListIBM)(ARG1, ARG2, (const void**)marshalunhandled("glFogCoordPointerListIBM", "pointer", cpu, ARG3), ARG4);
    GL_LOG ("glFogCoordPointerListIBM GLenum type=%d, GLint stride=%d, const void** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFogCoordd(CPU* cpu) {
    if (!ext_glFogCoordd)
        kpanic("ext_glFogCoordd is NULL");
    {
    GL_FUNC(ext_glFogCoordd)(dARG1);
    GL_LOG ("glFogCoordd GLdouble coord=%f",dARG1);
    }
}
void glcommon_glFogCoorddEXT(CPU* cpu) {
    if (!ext_glFogCoorddEXT)
        kpanic("ext_glFogCoorddEXT is NULL");
    {
    GL_FUNC(ext_glFogCoorddEXT)(dARG1);
    GL_LOG ("glFogCoorddEXT GLdouble coord=%f",dARG1);
    }
}
void glcommon_glFogCoorddv(CPU* cpu) {
    if (!ext_glFogCoorddv)
        kpanic("ext_glFogCoorddv is NULL");
    {
    GL_FUNC(ext_glFogCoorddv)(marshalArray<GLdouble>(cpu, ARG1, 1));
    GL_LOG ("glFogCoorddv const GLdouble* coord=%.08x",ARG1);
    }
}
void glcommon_glFogCoorddvEXT(CPU* cpu) {
    if (!ext_glFogCoorddvEXT)
        kpanic("ext_glFogCoorddvEXT is NULL");
    {
    GL_FUNC(ext_glFogCoorddvEXT)(marshalArray<GLdouble>(cpu, ARG1, 1));
    GL_LOG ("glFogCoorddvEXT const GLdouble* coord=%.08x",ARG1);
    }
}
void glcommon_glFogCoordf(CPU* cpu) {
    if (!ext_glFogCoordf)
        kpanic("ext_glFogCoordf is NULL");
    {
    GL_FUNC(ext_glFogCoordf)(fARG1);
    GL_LOG ("glFogCoordf GLfloat coord=%f",fARG1);
    }
}
void glcommon_glFogCoordfEXT(CPU* cpu) {
    if (!ext_glFogCoordfEXT)
        kpanic("ext_glFogCoordfEXT is NULL");
    {
    GL_FUNC(ext_glFogCoordfEXT)(fARG1);
    GL_LOG ("glFogCoordfEXT GLfloat coord=%f",fARG1);
    }
}
void glcommon_glFogCoordfv(CPU* cpu) {
    if (!ext_glFogCoordfv)
        kpanic("ext_glFogCoordfv is NULL");
    {
    GL_FUNC(ext_glFogCoordfv)(marshalArray<GLfloat>(cpu, ARG1, 1));
    GL_LOG ("glFogCoordfv const GLfloat* coord=%.08x",ARG1);
    }
}
void glcommon_glFogCoordfvEXT(CPU* cpu) {
    if (!ext_glFogCoordfvEXT)
        kpanic("ext_glFogCoordfvEXT is NULL");
    {
    GL_FUNC(ext_glFogCoordfvEXT)(marshalArray<GLfloat>(cpu, ARG1, 1));
    GL_LOG ("glFogCoordfvEXT const GLfloat* coord=%.08x",ARG1);
    }
}
void glcommon_glFogCoordhNV(CPU* cpu) {
    if (!ext_glFogCoordhNV)
        kpanic("ext_glFogCoordhNV is NULL");
    {
    GL_FUNC(ext_glFogCoordhNV)(sARG1);
    GL_LOG ("glFogCoordhNV GLhalfNV fog=%d",sARG1);
    }
}
void glcommon_glFogCoordhvNV(CPU* cpu) {
    if (!ext_glFogCoordhvNV)
        kpanic("ext_glFogCoordhvNV is NULL");
    {
    GL_FUNC(ext_glFogCoordhvNV)(marshalArray<GLushort>(cpu, ARG1, 1));
    GL_LOG ("glFogCoordhvNV const GLhalfNV* fog=%.08x",ARG1);
    }
}
void glcommon_glFogFuncSGIS(CPU* cpu) {
    if (!ext_glFogFuncSGIS)
        kpanic("ext_glFogFuncSGIS is NULL");
    {
    GL_FUNC(ext_glFogFuncSGIS)(ARG1, marshalArray<GLfloat>(cpu, ARG2, ARG1*2));
    GL_LOG ("glFogFuncSGIS GLsizei n=%d, const GLfloat* points=%.08x",ARG1,ARG2);
    }
}
void glcommon_glFogxOES(CPU* cpu) {
    if (!ext_glFogxOES)
        kpanic("ext_glFogxOES is NULL");
    {
    GL_FUNC(ext_glFogxOES)(ARG1, ARG2);
    GL_LOG ("glFogxOES GLenum pname=%d, GLfixed param=%d",ARG1,ARG2);
    }
}
void glcommon_glFogxvOES(CPU* cpu) {
    if (!ext_glFogxvOES)
        kpanic("ext_glFogxvOES is NULL");
    {
    GL_FUNC(ext_glFogxvOES)(ARG1, (GLfixed*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glFogxvOES GLenum pname=%d, const GLfixed* param=%.08x",ARG1,ARG2);
    }
}
void glcommon_glFragmentColorMaterialSGIX(CPU* cpu) {
    if (!ext_glFragmentColorMaterialSGIX)
        kpanic("ext_glFragmentColorMaterialSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentColorMaterialSGIX)(ARG1, ARG2);
    GL_LOG ("glFragmentColorMaterialSGIX GLenum face=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glFragmentCoverageColorNV(CPU* cpu) {
    if (!ext_glFragmentCoverageColorNV)
        kpanic("ext_glFragmentCoverageColorNV is NULL");
    {
    GL_FUNC(ext_glFragmentCoverageColorNV)(ARG1);
    GL_LOG ("glFragmentCoverageColorNV GLuint color=%d",ARG1);
    }
}
void glcommon_glFragmentLightModelfSGIX(CPU* cpu) {
    if (!ext_glFragmentLightModelfSGIX)
        kpanic("ext_glFragmentLightModelfSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightModelfSGIX)(ARG1, fARG2);
    GL_LOG ("glFragmentLightModelfSGIX GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glFragmentLightModelfvSGIX(CPU* cpu) {
    if (!ext_glFragmentLightModelfvSGIX)
        kpanic("ext_glFragmentLightModelfvSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightModelfvSGIX)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 1));
    GL_LOG ("glFragmentLightModelfvSGIX GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glFragmentLightModeliSGIX(CPU* cpu) {
    if (!ext_glFragmentLightModeliSGIX)
        kpanic("ext_glFragmentLightModeliSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightModeliSGIX)(ARG1, ARG2);
    GL_LOG ("glFragmentLightModeliSGIX GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glFragmentLightModelivSGIX(CPU* cpu) {
    if (!ext_glFragmentLightModelivSGIX)
        kpanic("ext_glFragmentLightModelivSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightModelivSGIX)(ARG1, marshalArray<GLint>(cpu, ARG2, 1));
    GL_LOG ("glFragmentLightModelivSGIX GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glFragmentLightfSGIX(CPU* cpu) {
    if (!ext_glFragmentLightfSGIX)
        kpanic("ext_glFragmentLightfSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightfSGIX)(ARG1, ARG2, fARG3);
    GL_LOG ("glFragmentLightfSGIX GLenum light=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glFragmentLightfvSGIX(CPU* cpu) {
    if (!ext_glFragmentLightfvSGIX)
        kpanic("ext_glFragmentLightfvSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightfvSGIX)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, 1));
    GL_LOG ("glFragmentLightfvSGIX GLenum light=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFragmentLightiSGIX(CPU* cpu) {
    if (!ext_glFragmentLightiSGIX)
        kpanic("ext_glFragmentLightiSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightiSGIX)(ARG1, ARG2, ARG3);
    GL_LOG ("glFragmentLightiSGIX GLenum light=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFragmentLightivSGIX(CPU* cpu) {
    if (!ext_glFragmentLightivSGIX)
        kpanic("ext_glFragmentLightivSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentLightivSGIX)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, 1));
    GL_LOG ("glFragmentLightivSGIX GLenum light=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFragmentMaterialfSGIX(CPU* cpu) {
    if (!ext_glFragmentMaterialfSGIX)
        kpanic("ext_glFragmentMaterialfSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentMaterialfSGIX)(ARG1, ARG2, fARG3);
    GL_LOG ("glFragmentMaterialfSGIX GLenum face=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glFragmentMaterialfvSGIX(CPU* cpu) {
    if (!ext_glFragmentMaterialfvSGIX)
        kpanic("ext_glFragmentMaterialfvSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentMaterialfvSGIX)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, 1));
    GL_LOG ("glFragmentMaterialfvSGIX GLenum face=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFragmentMaterialiSGIX(CPU* cpu) {
    if (!ext_glFragmentMaterialiSGIX)
        kpanic("ext_glFragmentMaterialiSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentMaterialiSGIX)(ARG1, ARG2, ARG3);
    GL_LOG ("glFragmentMaterialiSGIX GLenum face=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFragmentMaterialivSGIX(CPU* cpu) {
    if (!ext_glFragmentMaterialivSGIX)
        kpanic("ext_glFragmentMaterialivSGIX is NULL");
    {
    GL_FUNC(ext_glFragmentMaterialivSGIX)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, 1));
    GL_LOG ("glFragmentMaterialivSGIX GLenum face=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFrameTerminatorGREMEDY(CPU* cpu) {
    if (!ext_glFrameTerminatorGREMEDY)
        kpanic("ext_glFrameTerminatorGREMEDY is NULL");
    {
    GL_FUNC(ext_glFrameTerminatorGREMEDY)();
    GL_LOG ("glFrameTerminatorGREMEDY");
    }
}
void glcommon_glFrameZoomSGIX(CPU* cpu) {
    if (!ext_glFrameZoomSGIX)
        kpanic("ext_glFrameZoomSGIX is NULL");
    {
    GL_FUNC(ext_glFrameZoomSGIX)(ARG1);
    GL_LOG ("glFrameZoomSGIX GLint factor=%d",ARG1);
    }
}
void glcommon_glFramebufferDrawBufferEXT(CPU* cpu) {
    if (!ext_glFramebufferDrawBufferEXT)
        kpanic("ext_glFramebufferDrawBufferEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferDrawBufferEXT)(ARG1, ARG2);
    GL_LOG ("glFramebufferDrawBufferEXT GLuint framebuffer=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glFramebufferDrawBuffersEXT(CPU* cpu) {
    if (!ext_glFramebufferDrawBuffersEXT)
        kpanic("ext_glFramebufferDrawBuffersEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferDrawBuffersEXT)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2));
    GL_LOG ("glFramebufferDrawBuffersEXT GLuint framebuffer=%d, GLsizei n=%d, const GLenum* bufs=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFramebufferParameteri(CPU* cpu) {
    if (!ext_glFramebufferParameteri)
        kpanic("ext_glFramebufferParameteri is NULL");
    {
    GL_FUNC(ext_glFramebufferParameteri)(ARG1, ARG2, ARG3);
    GL_LOG ("glFramebufferParameteri GLenum target=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glFramebufferReadBufferEXT(CPU* cpu) {
    if (!ext_glFramebufferReadBufferEXT)
        kpanic("ext_glFramebufferReadBufferEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferReadBufferEXT)(ARG1, ARG2);
    GL_LOG ("glFramebufferReadBufferEXT GLuint framebuffer=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glFramebufferRenderbuffer(CPU* cpu) {
    if (!ext_glFramebufferRenderbuffer)
        kpanic("ext_glFramebufferRenderbuffer is NULL");
    {
    GL_FUNC(ext_glFramebufferRenderbuffer)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glFramebufferRenderbuffer GLenum target=%d, GLenum attachment=%d, GLenum renderbuffertarget=%d, GLuint renderbuffer=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFramebufferRenderbufferEXT(CPU* cpu) {
    if (!ext_glFramebufferRenderbufferEXT)
        kpanic("ext_glFramebufferRenderbufferEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferRenderbufferEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glFramebufferRenderbufferEXT GLenum target=%d, GLenum attachment=%d, GLenum renderbuffertarget=%d, GLuint renderbuffer=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFramebufferSampleLocationsfvARB(CPU* cpu) {
    if (!ext_glFramebufferSampleLocationsfvARB)
        kpanic("ext_glFramebufferSampleLocationsfvARB is NULL");
    {
    GL_FUNC(ext_glFramebufferSampleLocationsfvARB)(ARG1, ARG2, ARG3, marshalArray<GLfloat>(cpu, ARG4, ARG3));
    GL_LOG ("glFramebufferSampleLocationsfvARB GLenum target=%d, GLuint start=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFramebufferSampleLocationsfvNV(CPU* cpu) {
    if (!ext_glFramebufferSampleLocationsfvNV)
        kpanic("ext_glFramebufferSampleLocationsfvNV is NULL");
    {
    GL_FUNC(ext_glFramebufferSampleLocationsfvNV)(ARG1, ARG2, ARG3, marshalArray<GLfloat>(cpu, ARG4, ARG3));
    GL_LOG ("glFramebufferSampleLocationsfvNV GLenum target=%d, GLuint start=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFramebufferTexture(CPU* cpu) {
    if (!ext_glFramebufferTexture)
        kpanic("ext_glFramebufferTexture is NULL");
    {
    GL_FUNC(ext_glFramebufferTexture)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glFramebufferTexture GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFramebufferTexture1D(CPU* cpu) {
    if (!ext_glFramebufferTexture1D)
        kpanic("ext_glFramebufferTexture1D is NULL");
    {
    GL_FUNC(ext_glFramebufferTexture1D)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTexture1D GLenum target=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTexture1DEXT(CPU* cpu) {
    if (!ext_glFramebufferTexture1DEXT)
        kpanic("ext_glFramebufferTexture1DEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferTexture1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTexture1DEXT GLenum target=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTexture2D(CPU* cpu) {
    if (!ext_glFramebufferTexture2D)
        kpanic("ext_glFramebufferTexture2D is NULL");
    {
    GL_FUNC(ext_glFramebufferTexture2D)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTexture2D GLenum target=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTexture2DEXT(CPU* cpu) {
    if (!ext_glFramebufferTexture2DEXT)
        kpanic("ext_glFramebufferTexture2DEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferTexture2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTexture2DEXT GLenum target=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTexture3D(CPU* cpu) {
    if (!ext_glFramebufferTexture3D)
        kpanic("ext_glFramebufferTexture3D is NULL");
    {
    GL_FUNC(ext_glFramebufferTexture3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glFramebufferTexture3D GLenum target=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d, GLint zoffset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glFramebufferTexture3DEXT(CPU* cpu) {
    if (!ext_glFramebufferTexture3DEXT)
        kpanic("ext_glFramebufferTexture3DEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferTexture3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glFramebufferTexture3DEXT GLenum target=%d, GLenum attachment=%d, GLenum textarget=%d, GLuint texture=%d, GLint level=%d, GLint zoffset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glFramebufferTextureARB(CPU* cpu) {
    if (!ext_glFramebufferTextureARB)
        kpanic("ext_glFramebufferTextureARB is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureARB)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glFramebufferTextureARB GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFramebufferTextureEXT(CPU* cpu) {
    if (!ext_glFramebufferTextureEXT)
        kpanic("ext_glFramebufferTextureEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glFramebufferTextureEXT GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glFramebufferTextureFaceARB(CPU* cpu) {
    if (!ext_glFramebufferTextureFaceARB)
        kpanic("ext_glFramebufferTextureFaceARB is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureFaceARB)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTextureFaceARB GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLenum face=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTextureFaceEXT(CPU* cpu) {
    if (!ext_glFramebufferTextureFaceEXT)
        kpanic("ext_glFramebufferTextureFaceEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureFaceEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTextureFaceEXT GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLenum face=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTextureLayer(CPU* cpu) {
    if (!ext_glFramebufferTextureLayer)
        kpanic("ext_glFramebufferTextureLayer is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureLayer)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTextureLayer GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLint layer=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTextureLayerARB(CPU* cpu) {
    if (!ext_glFramebufferTextureLayerARB)
        kpanic("ext_glFramebufferTextureLayerARB is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureLayerARB)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTextureLayerARB GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLint layer=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTextureLayerEXT(CPU* cpu) {
    if (!ext_glFramebufferTextureLayerEXT)
        kpanic("ext_glFramebufferTextureLayerEXT is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureLayerEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glFramebufferTextureLayerEXT GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLint layer=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glFramebufferTextureMultiviewOVR(CPU* cpu) {
    if (!ext_glFramebufferTextureMultiviewOVR)
        kpanic("ext_glFramebufferTextureMultiviewOVR is NULL");
    {
    GL_FUNC(ext_glFramebufferTextureMultiviewOVR)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glFramebufferTextureMultiviewOVR GLenum target=%d, GLenum attachment=%d, GLuint texture=%d, GLint level=%d, GLint baseViewIndex=%d, GLsizei numViews=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glFreeObjectBufferATI(CPU* cpu) {
    if (!ext_glFreeObjectBufferATI)
        kpanic("ext_glFreeObjectBufferATI is NULL");
    {
    GL_FUNC(ext_glFreeObjectBufferATI)(ARG1);
    GL_LOG ("glFreeObjectBufferATI GLuint buffer=%d",ARG1);
    }
}
void glcommon_glFrustumfOES(CPU* cpu) {
    if (!ext_glFrustumfOES)
        kpanic("ext_glFrustumfOES is NULL");
    {
    GL_FUNC(ext_glFrustumfOES)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6);
    GL_LOG ("glFrustumfOES GLfloat l=%f, GLfloat r=%f, GLfloat b=%f, GLfloat t=%f, GLfloat n=%f, GLfloat f=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6);
    }
}
void glcommon_glFrustumxOES(CPU* cpu) {
    if (!ext_glFrustumxOES)
        kpanic("ext_glFrustumxOES is NULL");
    {
    GL_FUNC(ext_glFrustumxOES)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glFrustumxOES GLfixed l=%d, GLfixed r=%d, GLfixed b=%d, GLfixed t=%d, GLfixed n=%d, GLfixed f=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGenAsyncMarkersSGIX(CPU* cpu) {
    if (!ext_glGenAsyncMarkersSGIX)
        kpanic("ext_glGenAsyncMarkersSGIX is NULL");
    {
    EAX=GL_FUNC(ext_glGenAsyncMarkersSGIX)(ARG1);
    GL_LOG ("glGenAsyncMarkersSGIX GLsizei range=%d",ARG1);
    }
}
void glcommon_glGenBuffers(CPU* cpu) {
    if (!ext_glGenBuffers)
        kpanic("ext_glGenBuffers is NULL");
    {
        MarshalReadWrite<GLuint> buffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenBuffers)(ARG1, buffers.getPtr());
        GL_LOG ("glGenBuffers GLsizei n=%d, GLuint* buffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenBuffersARB(CPU* cpu) {
    if (!ext_glGenBuffersARB)
        kpanic("ext_glGenBuffersARB is NULL");
    {
        MarshalReadWrite<GLuint> buffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenBuffersARB)(ARG1, buffers.getPtr());
        GL_LOG ("glGenBuffersARB GLsizei n=%d, GLuint* buffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenFencesAPPLE(CPU* cpu) {
    if (!ext_glGenFencesAPPLE)
        kpanic("ext_glGenFencesAPPLE is NULL");
    {
        MarshalReadWrite<GLuint> fences(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenFencesAPPLE)(ARG1, fences.getPtr());
        GL_LOG ("glGenFencesAPPLE GLsizei n=%d, GLuint* fences=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenFencesNV(CPU* cpu) {
    if (!ext_glGenFencesNV)
        kpanic("ext_glGenFencesNV is NULL");
    {
        MarshalReadWrite<GLuint> fences(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenFencesNV)(ARG1, fences.getPtr());
        GL_LOG ("glGenFencesNV GLsizei n=%d, GLuint* fences=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenFragmentShadersATI(CPU* cpu) {
    if (!ext_glGenFragmentShadersATI)
        kpanic("ext_glGenFragmentShadersATI is NULL");
    {
    EAX=GL_FUNC(ext_glGenFragmentShadersATI)(ARG1);
    GL_LOG ("glGenFragmentShadersATI GLuint range=%d",ARG1);
    }
}
void glcommon_glGenFramebuffers(CPU* cpu) {
    if (!ext_glGenFramebuffers)
        kpanic("ext_glGenFramebuffers is NULL");
    {
        MarshalReadWrite<GLuint> framebuffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenFramebuffers)(ARG1, framebuffers.getPtr());
        GL_LOG ("glGenFramebuffers GLsizei n=%d, GLuint* framebuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenFramebuffersEXT(CPU* cpu) {
    if (!ext_glGenFramebuffersEXT)
        kpanic("ext_glGenFramebuffersEXT is NULL");
    {
        MarshalReadWrite<GLuint> framebuffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenFramebuffersEXT)(ARG1, framebuffers.getPtr());
        GL_LOG ("glGenFramebuffersEXT GLsizei n=%d, GLuint* framebuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenNamesAMD(CPU* cpu) {
    if (!ext_glGenNamesAMD)
        kpanic("ext_glGenNamesAMD is NULL");
    {
        MarshalReadWrite<GLuint> names(cpu, ARG3, ARG2);
        GL_FUNC(ext_glGenNamesAMD)(ARG1, ARG2, names.getPtr());
        GL_LOG ("glGenNamesAMD GLenum identifier=%d, GLuint num=%d, GLuint* names=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGenOcclusionQueriesNV(CPU* cpu) {
    if (!ext_glGenOcclusionQueriesNV)
        kpanic("ext_glGenOcclusionQueriesNV is NULL");
    {
        MarshalReadWrite<GLuint> ids(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenOcclusionQueriesNV)(ARG1, ids.getPtr());
        GL_LOG ("glGenOcclusionQueriesNV GLsizei n=%d, GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenPathsNV(CPU* cpu) {
    if (!ext_glGenPathsNV)
        kpanic("ext_glGenPathsNV is NULL");
    {
    EAX=GL_FUNC(ext_glGenPathsNV)(ARG1);
    GL_LOG ("glGenPathsNV GLsizei range=%d",ARG1);
    }
}
void glcommon_glGenPerfMonitorsAMD(CPU* cpu) {
    if (!ext_glGenPerfMonitorsAMD)
        kpanic("ext_glGenPerfMonitorsAMD is NULL");
    {
        MarshalReadWrite<GLuint> monitors(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenPerfMonitorsAMD)(ARG1, monitors.getPtr());
        GL_LOG ("glGenPerfMonitorsAMD GLsizei n=%d, GLuint* monitors=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenProgramPipelines(CPU* cpu) {
    if (!ext_glGenProgramPipelines)
        kpanic("ext_glGenProgramPipelines is NULL");
    {
        MarshalReadWrite<GLuint> pipelines(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenProgramPipelines)(ARG1, pipelines.getPtr());
        GL_LOG ("glGenProgramPipelines GLsizei n=%d, GLuint* pipelines=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenProgramsARB(CPU* cpu) {
    if (!ext_glGenProgramsARB)
        kpanic("ext_glGenProgramsARB is NULL");
    {
        MarshalReadWrite<GLuint> programs(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenProgramsARB)(ARG1, programs.getPtr());
        GL_LOG ("glGenProgramsARB GLsizei n=%d, GLuint* programs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenProgramsNV(CPU* cpu) {
    if (!ext_glGenProgramsNV)
        kpanic("ext_glGenProgramsNV is NULL");
    {
        MarshalReadWrite<GLuint> programs(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenProgramsNV)(ARG1, programs.getPtr());
        GL_LOG ("glGenProgramsNV GLsizei n=%d, GLuint* programs=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenQueries(CPU* cpu) {
    if (!ext_glGenQueries)
        kpanic("ext_glGenQueries is NULL");
    {
        MarshalReadWrite<GLuint> ids(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenQueries)(ARG1, ids.getPtr());
        GL_LOG ("glGenQueries GLsizei n=%d, GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenQueriesARB(CPU* cpu) {
    if (!ext_glGenQueriesARB)
        kpanic("ext_glGenQueriesARB is NULL");
    {
        MarshalReadWrite<GLuint> ids(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenQueriesARB)(ARG1, ids.getPtr());
        GL_LOG ("glGenQueriesARB GLsizei n=%d, GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenRenderbuffers(CPU* cpu) {
    if (!ext_glGenRenderbuffers)
        kpanic("ext_glGenRenderbuffers is NULL");
    {
        MarshalReadWrite<GLuint> renderbuffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenRenderbuffers)(ARG1, renderbuffers.getPtr());
        GL_LOG ("glGenRenderbuffers GLsizei n=%d, GLuint* renderbuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenRenderbuffersEXT(CPU* cpu) {
    if (!ext_glGenRenderbuffersEXT)
        kpanic("ext_glGenRenderbuffersEXT is NULL");
    {
        MarshalReadWrite<GLuint> renderbuffers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenRenderbuffersEXT)(ARG1, renderbuffers.getPtr());
        GL_LOG ("glGenRenderbuffersEXT GLsizei n=%d, GLuint* renderbuffers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenSamplers(CPU* cpu) {
    if (!ext_glGenSamplers)
        kpanic("ext_glGenSamplers is NULL");
    {
        MarshalReadWrite<GLuint> samplers(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenSamplers)(ARG1, samplers.getPtr());
        GL_LOG ("glGenSamplers GLsizei count=%d, GLuint* samplers=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenSymbolsEXT(CPU* cpu) {
    if (!ext_glGenSymbolsEXT)
        kpanic("ext_glGenSymbolsEXT is NULL");
    {
    EAX=GL_FUNC(ext_glGenSymbolsEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glGenSymbolsEXT GLenum datatype=%d, GLenum storagetype=%d, GLenum range=%d, GLuint components=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGenTexturesEXT(CPU* cpu) {
    if (!ext_glGenTexturesEXT)
        kpanic("ext_glGenTexturesEXT is NULL");
    {
        MarshalReadWrite<GLuint> textures(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenTexturesEXT)(ARG1, textures.getPtr());
        GL_LOG ("glGenTexturesEXT GLsizei n=%d, GLuint* textures=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenTransformFeedbacks(CPU* cpu) {
    if (!ext_glGenTransformFeedbacks)
        kpanic("ext_glGenTransformFeedbacks is NULL");
    {
        MarshalReadWrite<GLuint> ids(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenTransformFeedbacks)(ARG1, ids.getPtr());
        GL_LOG ("glGenTransformFeedbacks GLsizei n=%d, GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenTransformFeedbacksNV(CPU* cpu) {
    if (!ext_glGenTransformFeedbacksNV)
        kpanic("ext_glGenTransformFeedbacksNV is NULL");
    {
        MarshalReadWrite<GLuint> ids(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenTransformFeedbacksNV)(ARG1, ids.getPtr());
        GL_LOG ("glGenTransformFeedbacksNV GLsizei n=%d, GLuint* ids=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenVertexArrays(CPU* cpu) {
    if (!ext_glGenVertexArrays)
        kpanic("ext_glGenVertexArrays is NULL");
    {
        MarshalReadWrite<GLuint> arrays(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenVertexArrays)(ARG1, arrays.getPtr());
        GL_LOG ("glGenVertexArrays GLsizei n=%d, GLuint* arrays=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenVertexArraysAPPLE(CPU* cpu) {
    if (!ext_glGenVertexArraysAPPLE)
        kpanic("ext_glGenVertexArraysAPPLE is NULL");
    {
        MarshalReadWrite<GLuint> arrays(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGenVertexArraysAPPLE)(ARG1, arrays.getPtr());
        GL_LOG ("glGenVertexArraysAPPLE GLsizei n=%d, GLuint* arrays=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGenVertexShadersEXT(CPU* cpu) {
    if (!ext_glGenVertexShadersEXT)
        kpanic("ext_glGenVertexShadersEXT is NULL");
    {
    EAX=GL_FUNC(ext_glGenVertexShadersEXT)(ARG1);
    GL_LOG ("glGenVertexShadersEXT GLuint range=%d",ARG1);
    }
}
void glcommon_glGenerateMipmap(CPU* cpu) {
    if (!ext_glGenerateMipmap)
        kpanic("ext_glGenerateMipmap is NULL");
    {
    GL_FUNC(ext_glGenerateMipmap)(ARG1);
    GL_LOG ("glGenerateMipmap GLenum target=%d",ARG1);
    }
}
void glcommon_glGenerateMipmapEXT(CPU* cpu) {
    if (!ext_glGenerateMipmapEXT)
        kpanic("ext_glGenerateMipmapEXT is NULL");
    {
    GL_FUNC(ext_glGenerateMipmapEXT)(ARG1);
    GL_LOG ("glGenerateMipmapEXT GLenum target=%d",ARG1);
    }
}
void glcommon_glGenerateMultiTexMipmapEXT(CPU* cpu) {
    if (!ext_glGenerateMultiTexMipmapEXT)
        kpanic("ext_glGenerateMultiTexMipmapEXT is NULL");
    {
    GL_FUNC(ext_glGenerateMultiTexMipmapEXT)(ARG1, ARG2);
    GL_LOG ("glGenerateMultiTexMipmapEXT GLenum texunit=%d, GLenum target=%d",ARG1,ARG2);
    }
}
void glcommon_glGenerateTextureMipmap(CPU* cpu) {
    if (!ext_glGenerateTextureMipmap)
        kpanic("ext_glGenerateTextureMipmap is NULL");
    {
    GL_FUNC(ext_glGenerateTextureMipmap)(ARG1);
    GL_LOG ("glGenerateTextureMipmap GLuint texture=%d",ARG1);
    }
}
void glcommon_glGenerateTextureMipmapEXT(CPU* cpu) {
    if (!ext_glGenerateTextureMipmapEXT)
        kpanic("ext_glGenerateTextureMipmapEXT is NULL");
    {
    GL_FUNC(ext_glGenerateTextureMipmapEXT)(ARG1, ARG2);
    GL_LOG ("glGenerateTextureMipmapEXT GLuint texture=%d, GLenum target=%d",ARG1,ARG2);
    }
}
void glcommon_glGetActiveAtomicCounterBufferiv(CPU* cpu) {
    if (!ext_glGetActiveAtomicCounterBufferiv)
        kpanic("ext_glGetActiveAtomicCounterBufferiv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG4, (ARG3==GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES?marshalGetActiveAtomicCountersCount(ARG1, ARG2):1));
        GL_FUNC(ext_glGetActiveAtomicCounterBufferiv)(ARG1, ARG2, ARG3, params.getPtr());
        GL_LOG ("glGetActiveAtomicCounterBufferiv GLuint program=%d, GLuint bufferIndex=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetActiveAttrib(CPU* cpu) {
    if (!ext_glGetActiveAttrib)
        kpanic("ext_glGetActiveAttrib is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG4, 1);
        MarshalReadWrite<GLint> size(cpu, ARG5, 1);
        MarshalReadWrite<GLenum> type(cpu, ARG6, 1);
        MarshalReadWrite<GLchar> buffer(cpu, ARG7, ARG3);
        GL_FUNC(ext_glGetActiveAttrib)(ARG1, ARG2, ARG3, length.getPtr(), size.getPtr(), type.getPtr(), buffer.getPtr());
        GL_LOG ("glGetActiveAttrib GLuint program=%d, GLuint index=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLint* size=%.08x, GLenum* type=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetActiveAttribARB(CPU* cpu) {
    if (!ext_glGetActiveAttribARB)
        kpanic("ext_glGetActiveAttribARB is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG4, 1);
        MarshalReadWrite<GLint> size(cpu, ARG5, 1);
        MarshalReadWrite<GLenum> type(cpu, ARG6, 1);
        MarshalReadWrite<GLcharARB> name(cpu, ARG7, ARG3);

        GL_FUNC(ext_glGetActiveAttribARB)(INDEX_TO_HANDLE(hARG1), ARG2, ARG3, length.getPtr(), size.getPtr(), type.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveAttribARB GLhandleARB programObj=%d, GLuint index=%d, GLsizei maxLength=%d, GLsizei* length=%.08x, GLint* size=%.08x, GLenum* type=%.08x, GLcharARB* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetActiveSubroutineName(CPU* cpu) {
    if (!ext_glGetActiveSubroutineName)
        kpanic("ext_glGetActiveSubroutineName is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG5, 1);
        MarshalReadWrite<GLchar> name(cpu, ARG6, ARG4);
        GL_FUNC(ext_glGetActiveSubroutineName)(ARG1, ARG2, ARG3, ARG4, length.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveSubroutineName GLuint program=%d, GLenum shadertype=%d, GLuint index=%d, GLsizei bufsize=%d, GLsizei* length=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetActiveSubroutineUniformName(CPU* cpu) {
    if (!ext_glGetActiveSubroutineUniformName)
        kpanic("ext_glGetActiveSubroutineUniformName is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG5, 1);
        MarshalReadWrite<GLchar> name(cpu, ARG6, ARG4);
        GL_FUNC(ext_glGetActiveSubroutineUniformName)(ARG1, ARG2, ARG3, ARG4, length.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveSubroutineUniformName GLuint program=%d, GLenum shadertype=%d, GLuint index=%d, GLsizei bufsize=%d, GLsizei* length=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetActiveSubroutineUniformiv(CPU* cpu) {
    if (!ext_glGetActiveSubroutineUniformiv)
        kpanic("ext_glGetActiveSubroutineUniformiv is NULL");
    {
        MarshalReadWrite<GLint> values(cpu, ARG5, (ARG4==GL_COMPATIBLE_SUBROUTINES?marshalGetCompatibleSubroutinesCount(ARG1, ARG2, ARG3):1));
        GL_FUNC(ext_glGetActiveSubroutineUniformiv)(ARG1, ARG2, ARG3, ARG4, values.getPtr());
        GL_LOG ("glGetActiveSubroutineUniformiv GLuint program=%d, GLenum shadertype=%d, GLuint index=%d, GLenum pname=%d, GLint* values=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetActiveUniform(CPU* cpu) {
    if (!ext_glGetActiveUniform)
        kpanic("ext_glGetActiveUniform is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG4, 1);
        MarshalReadWrite<GLsizei> size(cpu, ARG5, 1);
        MarshalReadWrite<GLenum> type(cpu, ARG6, 1);
        MarshalReadWrite<GLchar> name(cpu, ARG7, ARG3);

        GL_FUNC(ext_glGetActiveUniform)(ARG1, ARG2, ARG3, length.getPtr(), size.getPtr(), type.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveUniform GLuint program=%d, GLuint index=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLint* size=%.08x, GLenum* type=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetActiveUniformARB(CPU* cpu) {
    if (!ext_glGetActiveUniformARB)
        kpanic("ext_glGetActiveUniformARB is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG4, 1);
        MarshalReadWrite<GLsizei> size(cpu, ARG5, 1);
        MarshalReadWrite<GLenum> type(cpu, ARG6, 1);
        MarshalReadWrite<GLcharARB> name(cpu, ARG7, ARG3);

        GL_FUNC(ext_glGetActiveUniformARB)(INDEX_TO_HANDLE(hARG1), ARG2, ARG3, length.getPtr(), size.getPtr(), type.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveUniformARB GLhandleARB programObj=%d, GLuint index=%d, GLsizei maxLength=%d, GLsizei* length=%.08x, GLint* size=%.08x, GLenum* type=%.08x, GLcharARB* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetActiveUniformBlockName(CPU* cpu) {
    if (!ext_glGetActiveUniformBlockName)
        kpanic("ext_glGetActiveUniformBlockName is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG4, 1);
        MarshalReadWrite<GLchar> name(cpu, ARG5, ARG3);
        GL_FUNC(ext_glGetActiveUniformBlockName)(ARG1, ARG2, ARG3, length.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveUniformBlockName GLuint program=%d, GLuint uniformBlockIndex=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* uniformBlockName=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetActiveUniformBlockiv(CPU* cpu) {
    if (!ext_glGetActiveUniformBlockiv)
        kpanic("ext_glGetActiveUniformBlockiv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG4, (ARG3==GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES?marshalGetUniformBlockActiveUnformsCount(ARG1, ARG2):1));
        GL_FUNC(ext_glGetActiveUniformBlockiv)(ARG1, ARG2, ARG3, params.getPtr());
        GL_LOG ("glGetActiveUniformBlockiv GLuint program=%d, GLuint uniformBlockIndex=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetActiveUniformName(CPU* cpu) {
    if (!ext_glGetActiveUniformName)
        kpanic("ext_glGetActiveUniformName is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG4, 1);
        MarshalReadWrite<GLchar> name(cpu, ARG5, ARG3);

        GL_FUNC(ext_glGetActiveUniformName)(ARG1, ARG2, ARG3, length.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveUniformName GLuint program=%d, GLuint uniformIndex=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* uniformName=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetActiveUniformsiv(CPU* cpu) {
    if (!ext_glGetActiveUniformsiv)
        kpanic("ext_glGetActiveUniformsiv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG5, ARG2);
        GL_FUNC(ext_glGetActiveUniformsiv)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2), ARG4, params.getPtr());
        GL_LOG ("glGetActiveUniformsiv GLuint program=%d, GLsizei uniformCount=%d, const GLuint* uniformIndices=%.08x, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetActiveVaryingNV(CPU* cpu) {
    if (!ext_glGetActiveVaryingNV)
        kpanic("ext_glGetActiveVaryingNV is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG4, 1);
        MarshalReadWrite<GLsizei> size(cpu, ARG5, 1);
        MarshalReadWrite<GLenum> type(cpu, ARG6, 1);
        MarshalReadWrite<GLchar> name(cpu, ARG7, ARG3);

        GL_FUNC(ext_glGetActiveVaryingNV)(ARG1, ARG2, ARG3, length.getPtr(), size.getPtr(), type.getPtr(), name.getPtr());
        GL_LOG ("glGetActiveVaryingNV GLuint program=%d, GLuint index=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLsizei* size=%.08x, GLenum* type=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetArrayObjectfvATI(CPU* cpu) {
    if (!ext_glGetArrayObjectfvATI)
        kpanic("ext_glGetArrayObjectfvATI is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetArrayObjectfvATI)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetArrayObjectfvATI GLenum array=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetArrayObjectivATI(CPU* cpu) {
    if (!ext_glGetArrayObjectivATI)
        kpanic("ext_glGetArrayObjectivATI is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetArrayObjectivATI)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetArrayObjectivATI GLenum array=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetAttachedObjectsARB(CPU* cpu) {
    if (!ext_glGetAttachedObjectsARB)
        kpanic("ext_glGetAttachedObjectsARB is NULL");
    {
        MarshalReadWrite<GLsizei> count(cpu, ARG3, 1);
        GLhandleARB* p2=marshalhandle(cpu, ARG4, ARG2);
        GL_FUNC(ext_glGetAttachedObjectsARB)(INDEX_TO_HANDLE(hARG1), ARG2, count.getPtr(), p2);
        marshalBackhandle(cpu, ARG4, p2, ARG2);
        GL_LOG ("glGetAttachedObjectsARB GLhandleARB containerObj=%d, GLsizei maxCount=%d, GLsizei* count=%.08x, GLhandleARB* obj=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetAttachedShaders(CPU* cpu) {
    if (!ext_glGetAttachedShaders)
        kpanic("ext_glGetAttachedShaders is NULL");
    {
        MarshalReadWrite<GLsizei> count(cpu, ARG3, 1);
        MarshalReadWrite<GLuint> shaders(cpu, ARG4, ARG2);
        GL_FUNC(ext_glGetAttachedShaders)(ARG1, ARG2, count.getPtr(), shaders.getPtr());
        GL_LOG ("glGetAttachedShaders GLuint program=%d, GLsizei maxCount=%d, GLsizei* count=%.08x, GLuint* shaders=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetAttribLocation(CPU* cpu) {
    if (!ext_glGetAttribLocation)
        kpanic("ext_glGetAttribLocation is NULL");
    {
    EAX=GL_FUNC(ext_glGetAttribLocation)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glGetAttribLocation GLuint program=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetAttribLocationARB(CPU* cpu) {
    if (!ext_glGetAttribLocationARB)
        kpanic("ext_glGetAttribLocationARB is NULL");
    {
    EAX=GL_FUNC(ext_glGetAttribLocationARB)(INDEX_TO_HANDLE(hARG1), marshalsz(cpu, ARG2));
    GL_LOG ("glGetAttribLocationARB GLhandleARB programObj=%d, const GLcharARB* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetBooleanIndexedvEXT(CPU* cpu) {
    if (!ext_glGetBooleanIndexedvEXT)
        kpanic("ext_glGetBooleanIndexedvEXT is NULL");
    {
        MarshalReadWrite<GLboolean> rw(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetBooleanIndexedvEXT)(ARG1, ARG2, rw.getPtr());
        GL_LOG ("glGetBooleanIndexedvEXT GLenum target=%d, GLuint index=%d, GLboolean* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBooleani_v(CPU* cpu) {
    if (!ext_glGetBooleani_v)
        kpanic("ext_glGetBooleani_v is NULL");
    {
        MarshalReadWrite<GLboolean> rw(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetBooleani_v)(ARG1, ARG2, rw.getPtr());
        GL_LOG ("glGetBooleani_v GLenum target=%d, GLuint index=%d, GLboolean* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBufferParameteri64v(CPU* cpu) {
    if (!ext_glGetBufferParameteri64v)
        kpanic("ext_glGetBufferParameteri64v is NULL");
    {
        MarshalReadWrite<GLint64> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetBufferParameteri64v)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetBufferParameteri64v GLenum target=%d, GLenum pname=%d, GLint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBufferParameteriv(CPU* cpu) {
    if (!ext_glGetBufferParameteriv)
        kpanic("ext_glGetBufferParameteriv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetBufferParameteriv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetBufferParameteriv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBufferParameterivARB(CPU* cpu) {
    if (!ext_glGetBufferParameterivARB)
        kpanic("ext_glGetBufferParameterivARB is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetBufferParameterivARB)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetBufferParameterivARB GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBufferParameterui64vNV(CPU* cpu) {
    if (!ext_glGetBufferParameterui64vNV)
        kpanic("ext_glGetBufferParameterui64vNV is NULL");
    {
        MarshalReadWrite<GLuint64EXT> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetBufferParameterui64vNV)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetBufferParameterui64vNV GLenum target=%d, GLenum pname=%d, GLuint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBufferPointerv(CPU* cpu) {
    if (!ext_glGetBufferPointerv)
        kpanic("ext_glGetBufferPointerv is NULL");
    {
    GLint size=0;void* p;GL_FUNC(ext_glGetBufferPointerv)(ARG1, ARG2, &p);
    ext_glGetBufferParameteriv(ARG1, GL_BUFFER_SIZE, &size); cpu->memory->writed(ARG3, marshalBackp(cpu, p, size));
    GL_LOG ("glGetBufferPointerv GLenum target=%d, GLenum pname=%d, void** params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBufferPointervARB(CPU* cpu) {
    if (!ext_glGetBufferPointervARB)
        kpanic("ext_glGetBufferPointervARB is NULL");
    {
    GLint size=0;void* p=nullptr;GL_FUNC(ext_glGetBufferPointervARB)(ARG1, ARG2, &p);
    ext_glGetBufferParameterivARB(ARG1, GL_BUFFER_SIZE, &size); cpu->memory->writed(ARG3, marshalBackp(cpu, p, size));
    GL_LOG ("glGetBufferPointervARB GLenum target=%d, GLenum pname=%d, void** params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetBufferSubData(CPU* cpu) {
    if (!ext_glGetBufferSubData)
        kpanic("ext_glGetBufferSubData is NULL");
    {
        MarshalReadWrite<GLubyte> data(cpu, ARG4, ARG3);
        GL_FUNC(ext_glGetBufferSubData)(ARG1, ARG2, ARG3, data.getPtr());
        GL_LOG ("glGetBufferSubData GLenum target=%d, GLintptr offset=%d, GLsizeiptr size=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetBufferSubDataARB(CPU* cpu) {
    if (!ext_glGetBufferSubDataARB)
        kpanic("ext_glGetBufferSubDataARB is NULL");
    {
        MarshalReadWrite<GLubyte> data(cpu, ARG4, ARG3);
        GL_FUNC(ext_glGetBufferSubDataARB)(ARG1, ARG2, ARG3, data.getPtr());
        GL_LOG ("glGetBufferSubDataARB GLenum target=%d, GLintptrARB offset=%d, GLsizeiptrARB size=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetClipPlanefOES(CPU* cpu) {
    if (!ext_glGetClipPlanefOES)
        kpanic("ext_glGetClipPlanefOES is NULL");
    {
        MarshalReadWrite<GLfloat> equation(cpu, ARG2, 4);
        GL_FUNC(ext_glGetClipPlanefOES)(ARG1, equation.getPtr());
        GL_LOG ("glGetClipPlanefOES GLenum plane=%d, GLfloat* equation=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetClipPlanexOES(CPU* cpu) {
    if (!ext_glGetClipPlanexOES)
        kpanic("ext_glGetClipPlanexOES is NULL");
    {
        MarshalReadWrite<GLfixed> equation(cpu, ARG2, 4);
        GL_FUNC(ext_glGetClipPlanexOES)(ARG1, equation.getPtr());
        GL_LOG ("glGetClipPlanexOES GLenum plane=%d, GLfixed* equation=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetColorTable(CPU* cpu) {
    if (!ext_glGetColorTable)
        kpanic("ext_glGetColorTable is NULL");
    {
        if (PIXEL_PACK_BUFFER()) {
            GL_FUNC(ext_glGetColorTable)(ARG1, ARG2, ARG3, (GLvoid*)pARG4);
        } else {
            U32 colors = components_in_format(ARG2) * marshalGetColorTableWidth(ARG1);
            MarshalReadWriteType image(cpu, ARG3, ARG4, colors);
            GL_FUNC(ext_glGetColorTable)(ARG1, ARG2, ARG3, image.getPtr());
        }
        GL_LOG ("glGetColorTable GLenum target=%d, GLenum format=%d, GLenum type=%d, void* table=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetColorTableEXT(CPU* cpu) {
    if (!ext_glGetColorTableEXT)
        kpanic("ext_glGetColorTableEXT is NULL");
    {
        U32 colors = components_in_format(ARG2) * marshalGetColorTableWidthEXT(ARG1);
        MarshalReadWriteType data(cpu, ARG3, ARG4, colors);
        GL_FUNC(ext_glGetColorTableEXT)(ARG1, ARG2, ARG3, data.getPtr());
    GL_LOG ("glGetColorTableEXT GLenum target=%d, GLenum format=%d, GLenum type=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetColorTableParameterfv(CPU* cpu) {
    if (!ext_glGetColorTableParameterfv)
        kpanic("ext_glGetColorTableParameterfv is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetColorTableParameterfv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetColorTableParameterfv GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetColorTableParameterfvEXT(CPU* cpu) {
    if (!ext_glGetColorTableParameterfvEXT)
        kpanic("ext_glGetColorTableParameterfvEXT is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetColorTableParameterfvEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetColorTableParameterfvEXT GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetColorTableParameterfvSGI(CPU* cpu) {
    if (!ext_glGetColorTableParameterfvSGI)
        kpanic("ext_glGetColorTableParameterfvSGI is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetColorTableParameterfvSGI)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetColorTableParameterfvSGI GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetColorTableParameteriv(CPU* cpu) {
    if (!ext_glGetColorTableParameteriv)
        kpanic("ext_glGetColorTableParameteriv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetColorTableParameteriv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetColorTableParameteriv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetColorTableParameterivEXT(CPU* cpu) {
    if (!ext_glGetColorTableParameterivEXT)
        kpanic("ext_glGetColorTableParameterivEXT is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetColorTableParameterivEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetColorTableParameterivEXT GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetColorTableParameterivSGI(CPU* cpu) {
    if (!ext_glGetColorTableParameterivSGI)
        kpanic("ext_glGetColorTableParameterivSGI is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetColorTableParameterivSGI)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetColorTableParameterivSGI GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetColorTableSGI(CPU* cpu) {
    if (!ext_glGetColorTableSGI)
        kpanic("ext_glGetColorTableSGI is NULL");
    {
        U32 colors = components_in_format(ARG2) * marshalGetColorTableWidthSGI(ARG1);
        MarshalReadWriteType data(cpu, ARG3, ARG4, colors);
        GL_FUNC(ext_glGetColorTableSGI)(ARG1, ARG2, ARG3, data.getPtr());
        GL_LOG ("glGetColorTableSGI GLenum target=%d, GLenum format=%d, GLenum type=%d, void* table=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetCombinerInputParameterfvNV(CPU* cpu) {
    if (!ext_glGetCombinerInputParameterfvNV)
        kpanic("ext_glGetCombinerInputParameterfvNV is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG5, 1);
        GL_FUNC(ext_glGetCombinerInputParameterfvNV)(ARG1, ARG2, ARG3, ARG4, params.getPtr());
        GL_LOG ("glGetCombinerInputParameterfvNV GLenum stage=%d, GLenum portion=%d, GLenum variable=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetCombinerInputParameterivNV(CPU* cpu) {
    if (!ext_glGetCombinerInputParameterivNV)
        kpanic("ext_glGetCombinerInputParameterivNV is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG5, 1);
        GL_FUNC(ext_glGetCombinerInputParameterivNV)(ARG1, ARG2, ARG3, ARG4, params.getPtr());
        GL_LOG ("glGetCombinerInputParameterivNV GLenum stage=%d, GLenum portion=%d, GLenum variable=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetCombinerOutputParameterfvNV(CPU* cpu) {
    if (!ext_glGetCombinerOutputParameterfvNV)
        kpanic("ext_glGetCombinerOutputParameterfvNV is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG4, 1);
        GL_FUNC(ext_glGetCombinerOutputParameterfvNV)(ARG1, ARG2, ARG3, params.getPtr());
        GL_LOG ("glGetCombinerOutputParameterfvNV GLenum stage=%d, GLenum portion=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetCombinerOutputParameterivNV(CPU* cpu) {
    if (!ext_glGetCombinerOutputParameterivNV)
        kpanic("ext_glGetCombinerOutputParameterivNV is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG4, 1);
        GL_FUNC(ext_glGetCombinerOutputParameterivNV)(ARG1, ARG2, ARG3, params.getPtr());
        GL_LOG ("glGetCombinerOutputParameterivNV GLenum stage=%d, GLenum portion=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetCombinerStageParameterfvNV(CPU* cpu) {
    if (!ext_glGetCombinerStageParameterfvNV)
        kpanic("ext_glGetCombinerStageParameterfvNV is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 4);
        GL_FUNC(ext_glGetCombinerStageParameterfvNV)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetCombinerStageParameterfvNV GLenum stage=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetCommandHeaderNV(CPU* cpu) {
    if (!ext_glGetCommandHeaderNV)
        kpanic("ext_glGetCommandHeaderNV is NULL");
    {
    EAX=GL_FUNC(ext_glGetCommandHeaderNV)(ARG1, ARG2);
    GL_LOG ("glGetCommandHeaderNV GLenum tokenID=%d, GLuint size=%d",ARG1,ARG2);
    }
}
void glcommon_glGetCompressedMultiTexImageEXT(CPU* cpu) {
    if (!ext_glGetCompressedMultiTexImageEXT)
        kpanic("ext_glGetCompressedMultiTexImageEXT is NULL");
    {
        MarshalReadWritePacked<GLbyte> img(cpu, ARG4, [cpu] {
            return marshalGetCompressedMultiImageSizeEXT(ARG1, ARG2, ARG3);
            });
        GL_FUNC(ext_glGetCompressedMultiTexImageEXT)(ARG1, ARG2, ARG3, img.getPtr());
        GL_LOG ("glGetCompressedMultiTexImageEXT GLenum texunit=%d, GLenum target=%d, GLint lod=%d, void* img=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetCompressedTexImage(CPU* cpu) {
    if (!ext_glGetCompressedTexImage)
        kpanic("ext_glGetCompressedTexImage is NULL");
    {
        MarshalReadWritePacked<GLbyte> img(cpu, ARG3, [cpu] {
            return marshalGetCompressedImageSizeARB(ARG1, ARG2);
            });
        if (!img.isPacked() && img.getCount()==0) {
            // 3dmark 2001 needs this otherwise it will crash after first test
            GLenum e = glGetError();
            if (e != GL_NO_ERROR) {
                cpu->thread->glLastError = e;
                return;
            }
        }
        GL_FUNC(ext_glGetCompressedTexImage)(ARG1, ARG2, img.getPtr());
        GL_LOG ("glGetCompressedTexImage GLenum target=%d, GLint level=%d, void* img=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetCompressedTexImageARB(CPU* cpu) {
    if (!ext_glGetCompressedTexImageARB)
        kpanic("ext_glGetCompressedTexImageARB is NULL");
    {
        MarshalReadWritePacked<GLbyte> img(cpu, ARG3, [cpu] {
            return marshalGetCompressedImageSizeARB(ARG1, ARG2);
            });
        GL_FUNC(ext_glGetCompressedTexImageARB)(ARG1, ARG2, img.getPtr());
        GL_LOG ("glGetCompressedTexImageARB GLenum target=%d, GLint level=%d, void* img=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetCompressedTextureImage(CPU* cpu) {
    if (!ext_glGetCompressedTextureImage)
        kpanic("ext_glGetCompressedTextureImage is NULL");
    {
        MarshalReadWritePacked<GLbyte> pixels(cpu, ARG4, ARG3);
        GL_FUNC(ext_glGetCompressedTextureImage)(ARG1, ARG2, ARG3, pixels.getPtr());
        GL_LOG ("glGetCompressedTextureImage GLuint texture=%d, GLint level=%d, GLsizei bufSize=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetCompressedTextureImageEXT(CPU* cpu) {
    if (!ext_glGetCompressedTextureImageEXT)
        kpanic("ext_glGetCompressedTextureImageEXT is NULL");
    {
        MarshalReadWritePacked<GLbyte> img(cpu, ARG4, [cpu] {
            return marshalGetCompressedTextureSizeEXT(ARG1, ARG2, ARG3);
            });

        GL_FUNC(ext_glGetCompressedTextureImageEXT)(ARG1, ARG2, ARG3, img.getPtr());
        GL_LOG ("glGetCompressedTextureImageEXT GLuint texture=%d, GLenum target=%d, GLint lod=%d, void* img=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetCompressedTextureSubImage(CPU* cpu) {
    if (!ext_glGetCompressedTextureSubImage)
        kpanic("ext_glGetCompressedTextureSubImage is NULL");
    {
        MarshalReadWritePacked<GLbyte> pixels(cpu, ARG10, ARG9);

        GL_FUNC(ext_glGetCompressedTextureSubImage)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, pixels.getPtr());
        GL_LOG ("glGetCompressedTextureSubImage GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLsizei bufSize=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glGetConvolutionFilter(CPU* cpu) {
    if (!ext_glGetConvolutionFilter)
        kpanic("ext_glGetConvolutionFilter is NULL");
    {
        if (PIXEL_PACK_BUFFER()) {
            GL_FUNC(ext_glGetConvolutionFilter)(ARG1, ARG2, ARG3, (GLvoid*)pARG4);
        } else {
            U32 count = components_in_format(ARG2) * marshalGetConvolutionWidth(ARG1) * marshalGetConvolutionWidth(ARG2);
            MarshalReadWriteType image(cpu, ARG3, ARG4, count);
            GL_FUNC(ext_glGetConvolutionFilter)(ARG1, ARG2, ARG3, image.getPtr());
        }
        GL_LOG ("glGetConvolutionFilter GLenum target=%d, GLenum format=%d, GLenum type=%d, void* image=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetConvolutionFilterEXT(CPU* cpu) {
    if (!ext_glGetConvolutionFilterEXT)
        kpanic("ext_glGetConvolutionFilterEXT is NULL");
    {
        if (PIXEL_PACK_BUFFER()) {
            GL_FUNC(ext_glGetConvolutionFilterEXT)(ARG1, ARG2, ARG3, (GLvoid*)pARG4);
        } else {
            U32 count = components_in_format(ARG2) * marshalGetConvolutionWidth(ARG1) * marshalGetConvolutionWidth(ARG2);
            MarshalReadWriteType image(cpu, ARG3, ARG4, count);
            GL_FUNC(ext_glGetConvolutionFilterEXT)(ARG1, ARG2, ARG3, image.getPtr());
        }
        GL_LOG ("glGetConvolutionFilterEXT GLenum target=%d, GLenum format=%d, GLenum type=%d, void* image=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetConvolutionParameterfv(CPU* cpu) {
    if (!ext_glGetConvolutionParameterfv)
        kpanic("ext_glGetConvolutionParameterfv is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_COLOR || ARG2==GL_CONVOLUTION_FILTER_SCALE || ARG2==GL_CONVOLUTION_FILTER_BIAS)?4:1);
        GL_FUNC(ext_glGetConvolutionParameterfv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetConvolutionParameterfv GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetConvolutionParameterfvEXT(CPU* cpu) {
    if (!ext_glGetConvolutionParameterfvEXT)
        kpanic("ext_glGetConvolutionParameterfvEXT is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_COLOR || ARG2==GL_CONVOLUTION_FILTER_SCALE || ARG2==GL_CONVOLUTION_FILTER_BIAS)?4:1);
        GL_FUNC(ext_glGetConvolutionParameterfvEXT)(ARG1, ARG2, params.getPtr());        
        GL_LOG ("glGetConvolutionParameterfvEXT GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetConvolutionParameteriv(CPU* cpu) {
    if (!ext_glGetConvolutionParameteriv)
        kpanic("ext_glGetConvolutionParameteriv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_COLOR || ARG2==GL_CONVOLUTION_FILTER_SCALE || ARG2==GL_CONVOLUTION_FILTER_BIAS)?4:1);
        GL_FUNC(ext_glGetConvolutionParameteriv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetConvolutionParameteriv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetConvolutionParameterivEXT(CPU* cpu) {
    if (!ext_glGetConvolutionParameterivEXT)
        kpanic("ext_glGetConvolutionParameterivEXT is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_COLOR || ARG2==GL_CONVOLUTION_FILTER_SCALE || ARG2==GL_CONVOLUTION_FILTER_BIAS)?4:1);
        GL_FUNC(ext_glGetConvolutionParameterivEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetConvolutionParameterivEXT GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetConvolutionParameterxvOES(CPU* cpu) {
    if (!ext_glGetConvolutionParameterxvOES)
        kpanic("ext_glGetConvolutionParameterxvOES is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, (ARG2==GL_CONVOLUTION_BORDER_COLOR || ARG2==GL_CONVOLUTION_FILTER_SCALE || ARG2==GL_CONVOLUTION_FILTER_BIAS)?4:1);
        GL_FUNC(ext_glGetConvolutionParameterxvOES)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetConvolutionParameterxvOES GLenum target=%d, GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetCoverageModulationTableNV(CPU* cpu) {
    if (!ext_glGetCoverageModulationTableNV)
        kpanic("ext_glGetCoverageModulationTableNV is NULL");
    {
        MarshalReadWrite<GLfloat> v(cpu, ARG2, ARG1);
        GL_FUNC(ext_glGetCoverageModulationTableNV)(ARG1, v.getPtr());
        GL_LOG ("glGetCoverageModulationTableNV GLsizei bufsize=%d, GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetDebugMessageLog(CPU* cpu) {
    if (!ext_glGetDebugMessageLog)
        kpanic("ext_glGetDebugMessageLog is NULL");
    {
        MarshalReadWrite<GLenum> sources(cpu, ARG3, ARG1);
        MarshalReadWrite<GLenum> types(cpu, ARG4, ARG1);
        MarshalReadWrite<GLuint> ids(cpu, ARG5, ARG1);
        MarshalReadWrite<GLenum> severities(cpu, ARG6, ARG1);
        MarshalReadWrite<GLsizei> lengths(cpu, ARG7, ARG1);
        MarshalReadWrite<GLchar> messageLog(cpu, ARG8, ARG2);
        EAX=GL_FUNC(ext_glGetDebugMessageLog)(ARG1, ARG2, sources.getPtr(), types.getPtr(), ids.getPtr(), severities.getPtr(), lengths.getPtr(), messageLog.getPtr());
        GL_LOG ("glGetDebugMessageLog GLuint count=%d, GLsizei bufSize=%d, GLenum* sources=%.08x, GLenum* types=%.08x, GLuint* ids=%.08x, GLenum* severities=%.08x, GLsizei* lengths=%.08x, GLchar* messageLog=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glGetDebugMessageLogAMD(CPU* cpu) {
    if (!ext_glGetDebugMessageLogAMD)
        kpanic("ext_glGetDebugMessageLogAMD is NULL");
    {
        MarshalReadWrite<GLenum> categories(cpu, ARG3, ARG1);
        MarshalReadWrite<GLuint> severities(cpu, ARG4, ARG1);
        MarshalReadWrite<GLuint> ids(cpu, ARG5, ARG1);
        MarshalReadWrite<GLsizei> lengths(cpu, ARG6, ARG1);
        MarshalReadWrite<GLchar> message(cpu, ARG7, ARG2);

        EAX=GL_FUNC(ext_glGetDebugMessageLogAMD)(ARG1, ARG2, categories.getPtr(), severities.getPtr(), ids.getPtr(), lengths.getPtr(), message.getPtr());
        GL_LOG ("glGetDebugMessageLogAMD GLuint count=%d, GLsizei bufsize=%d, GLenum* categories=%.08x, GLuint* severities=%.08x, GLuint* ids=%.08x, GLsizei* lengths=%.08x, GLchar* message=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetDebugMessageLogARB(CPU* cpu) {
    if (!ext_glGetDebugMessageLogARB)
        kpanic("ext_glGetDebugMessageLogARB is NULL");
    {
        MarshalReadWrite<GLenum> sources(cpu, ARG3, ARG1);
        MarshalReadWrite<GLenum> types(cpu, ARG4, ARG1);
        MarshalReadWrite<GLuint> ids(cpu, ARG5, ARG1);
        MarshalReadWrite<GLenum> severities(cpu, ARG6, ARG1);
        MarshalReadWrite<GLsizei> lengths(cpu, ARG7, ARG1);
        MarshalReadWrite<GLchar> messageLog(cpu, ARG8, ARG2);

        EAX=GL_FUNC(ext_glGetDebugMessageLogARB)(ARG1, ARG2, sources.getPtr(), types.getPtr(), ids.getPtr(), severities.getPtr(), lengths.getPtr(), messageLog.getPtr());
        GL_LOG ("glGetDebugMessageLogARB GLuint count=%d, GLsizei bufSize=%d, GLenum* sources=%.08x, GLenum* types=%.08x, GLuint* ids=%.08x, GLenum* severities=%.08x, GLsizei* lengths=%.08x, GLchar* messageLog=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glGetDetailTexFuncSGIS(CPU* cpu) {
    if (!ext_glGetDetailTexFuncSGIS)
        kpanic("ext_glGetDetailTexFuncSGIS is NULL");
    {
    GL_FUNC(ext_glGetDetailTexFuncSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetDetailTexFuncSGIS GLenum target=%d, GLfloat* points=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetDoubleIndexedvEXT(CPU* cpu) {
    if (!ext_glGetDoubleIndexedvEXT)
        kpanic("ext_glGetDoubleIndexedvEXT is NULL");
    {
        MarshalReadWrite<GLdouble> data(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetDoubleIndexedvEXT)(ARG1, ARG2, data.getPtr());
        GL_LOG ("glGetDoubleIndexedvEXT GLenum target=%d, GLuint index=%d, GLdouble* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetDoublei_v(CPU* cpu) {
    if (!ext_glGetDoublei_v)
        kpanic("ext_glGetDoublei_v is NULL");
    {
        MarshalReadWrite<GLdouble> data(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetDoublei_v)(ARG1, ARG2, data.getPtr());
        GL_LOG ("glGetDoublei_v GLenum target=%d, GLuint index=%d, GLdouble* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetDoublei_vEXT(CPU* cpu) {
    if (!ext_glGetDoublei_vEXT)
        kpanic("ext_glGetDoublei_vEXT is NULL");
    {
        MarshalReadWrite<GLdouble> params(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetDoublei_vEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetDoublei_vEXT GLenum pname=%d, GLuint index=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFenceivNV(CPU* cpu) {
    if (!ext_glGetFenceivNV)
        kpanic("ext_glGetFenceivNV is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetFenceivNV)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFenceivNV GLuint fence=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFinalCombinerInputParameterfvNV(CPU* cpu) {
    if (!ext_glGetFinalCombinerInputParameterfvNV)
        kpanic("ext_glGetFinalCombinerInputParameterfvNV is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetFinalCombinerInputParameterfvNV)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFinalCombinerInputParameterfvNV GLenum variable=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFinalCombinerInputParameterivNV(CPU* cpu) {
    if (!ext_glGetFinalCombinerInputParameterivNV)
        kpanic("ext_glGetFinalCombinerInputParameterivNV is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetFinalCombinerInputParameterivNV)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFinalCombinerInputParameterivNV GLenum variable=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFirstPerfQueryIdINTEL(CPU* cpu) {
    if (!ext_glGetFirstPerfQueryIdINTEL)
        kpanic("ext_glGetFirstPerfQueryIdINTEL is NULL");
    {
        MarshalReadWrite<GLuint> queryId(cpu, ARG1, 1);
        GL_FUNC(ext_glGetFirstPerfQueryIdINTEL)(queryId.getPtr());
        GL_LOG ("glGetFirstPerfQueryIdINTEL GLuint* queryId=%.08x",ARG1);
    }
}
void glcommon_glGetFixedvOES(CPU* cpu) {
    if (!ext_glGetFixedvOES)
        kpanic("ext_glGetFixedvOES is NULL");
    {
        MarshalReadWrite<GLfixed> params(cpu, ARG2, getSize(ARG1));
        GL_FUNC(ext_glGetFixedvOES)(ARG1, params.getPtr());
        GL_LOG ("glGetFixedvOES GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetFloatIndexedvEXT(CPU* cpu) {
    if (!ext_glGetFloatIndexedvEXT)
        kpanic("ext_glGetFloatIndexedvEXT is NULL");
    {
        MarshalReadWrite<GLfloat> data(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetFloatIndexedvEXT)(ARG1, ARG2, data.getPtr());
        GL_LOG ("glGetFloatIndexedvEXT GLenum target=%d, GLuint index=%d, GLfloat* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFloati_v(CPU* cpu) {
    if (!ext_glGetFloati_v)
        kpanic("ext_glGetFloati_v is NULL");
    {
        MarshalReadWrite<GLfloat> data(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetFloati_v)(ARG1, ARG2, data.getPtr());
        GL_LOG ("glGetFloati_v GLenum target=%d, GLuint index=%d, GLfloat* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFloati_vEXT(CPU* cpu) {
    if (!ext_glGetFloati_vEXT)
        kpanic("ext_glGetFloati_vEXT is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetFloati_vEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFloati_vEXT GLenum pname=%d, GLuint index=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFogFuncSGIS(CPU* cpu) {
    if (!ext_glGetFogFuncSGIS)
        kpanic("ext_glGetFogFuncSGIS is NULL");
    {
    GL_FUNC(ext_glGetFogFuncSGIS)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glGetFogFuncSGIS GLfloat* points=%.08x",ARG1);
    }
}
void glcommon_glGetFragDataIndex(CPU* cpu) {
    if (!ext_glGetFragDataIndex)
        kpanic("ext_glGetFragDataIndex is NULL");
    {
    EAX=GL_FUNC(ext_glGetFragDataIndex)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glGetFragDataIndex GLuint program=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetFragDataLocation(CPU* cpu) {
    if (!ext_glGetFragDataLocation)
        kpanic("ext_glGetFragDataLocation is NULL");
    {
    EAX=GL_FUNC(ext_glGetFragDataLocation)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glGetFragDataLocation GLuint program=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetFragDataLocationEXT(CPU* cpu) {
    if (!ext_glGetFragDataLocationEXT)
        kpanic("ext_glGetFragDataLocationEXT is NULL");
    {
    EAX=GL_FUNC(ext_glGetFragDataLocationEXT)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glGetFragDataLocationEXT GLuint program=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetFragmentLightfvSGIX(CPU* cpu) {
    if (!ext_glGetFragmentLightfvSGIX)
        kpanic("ext_glGetFragmentLightfvSGIX is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, glcommon_glLightv_size(ARG2));
        GL_FUNC(ext_glGetFragmentLightfvSGIX)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFragmentLightfvSGIX GLenum light=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFragmentLightivSGIX(CPU* cpu) {
    if (!ext_glGetFragmentLightivSGIX)
        kpanic("ext_glGetFragmentLightivSGIX is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, glcommon_glLightv_size(ARG2));
        GL_FUNC(ext_glGetFragmentLightivSGIX)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFragmentLightivSGIX GLenum light=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFragmentMaterialfvSGIX(CPU* cpu) {
    if (!ext_glGetFragmentMaterialfvSGIX)
        kpanic("ext_glGetFragmentMaterialfvSGIX is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, glcommon_glMaterialv_size(ARG2));
        GL_FUNC(ext_glGetFragmentMaterialfvSGIX)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFragmentMaterialfvSGIX GLenum face=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFragmentMaterialivSGIX(CPU* cpu) {
    if (!ext_glGetFragmentMaterialivSGIX)
        kpanic("ext_glGetFragmentMaterialivSGIX is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, glcommon_glMaterialv_size(ARG2));
        GL_FUNC(ext_glGetFragmentMaterialivSGIX)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFragmentMaterialivSGIX GLenum face=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFramebufferAttachmentParameteriv(CPU* cpu) {
    if (!ext_glGetFramebufferAttachmentParameteriv)
        kpanic("ext_glGetFramebufferAttachmentParameteriv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG4, 1);
        GL_FUNC(ext_glGetFramebufferAttachmentParameteriv)(ARG1, ARG2, ARG3, params.getPtr());
        GL_LOG ("glGetFramebufferAttachmentParameteriv GLenum target=%d, GLenum attachment=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetFramebufferAttachmentParameterivEXT(CPU* cpu) {
    if (!ext_glGetFramebufferAttachmentParameterivEXT)
        kpanic("ext_glGetFramebufferAttachmentParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetFramebufferAttachmentParameterivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetFramebufferAttachmentParameterivEXT GLenum target=%d, GLenum attachment=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetFramebufferParameteriv(CPU* cpu) {
    if (!ext_glGetFramebufferParameteriv)
        kpanic("ext_glGetFramebufferParameteriv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetFramebufferParameteriv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFramebufferParameteriv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetFramebufferParameterivEXT(CPU* cpu) {
    if (!ext_glGetFramebufferParameterivEXT)
        kpanic("ext_glGetFramebufferParameterivEXT is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetFramebufferParameterivEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetFramebufferParameterivEXT GLuint framebuffer=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetGraphicsResetStatus(CPU* cpu) {
    if (!ext_glGetGraphicsResetStatus)
        kpanic("ext_glGetGraphicsResetStatus is NULL");
    {
    EAX=GL_FUNC(ext_glGetGraphicsResetStatus)();
    GL_LOG ("glGetGraphicsResetStatus");
    }
}
void glcommon_glGetGraphicsResetStatusARB(CPU* cpu) {
    if (!ext_glGetGraphicsResetStatusARB)
        kpanic("ext_glGetGraphicsResetStatusARB is NULL");
    {
    EAX=GL_FUNC(ext_glGetGraphicsResetStatusARB)();
    GL_LOG ("glGetGraphicsResetStatusARB");
    }
}
void glcommon_glGetHandleARB(CPU* cpu) {
    if (!ext_glGetHandleARB)
        kpanic("ext_glGetHandleARB is NULL");
    {
    EAX=HANDLE_TO_INDEX(GL_FUNC(ext_glGetHandleARB)(ARG1));
    GL_LOG ("glGetHandleARB GLenum pname=%d",ARG1);
    }
}
void glcommon_glGetHistogram(CPU* cpu) {
    if (!ext_glGetHistogram)
        kpanic("ext_glGetHistogram is NULL");
    {
        int width = marshalHistogramWidth(ARG1);
        MarshalReadWritePackedPixels values(cpu, 0, width, 1, 1, ARG3, ARG4, ARG5);
        GL_FUNC(ext_glGetHistogram)(ARG1, bARG2, ARG3, ARG4, values.getPtr());
        GL_LOG ("glGetHistogram GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetHistogramEXT(CPU* cpu) {
    if (!ext_glGetHistogramEXT)
        kpanic("ext_glGetHistogramEXT is NULL");
    {
        int width = marshalHistogramWidth(ARG1);
        MarshalReadWritePackedPixels values(cpu, 0, width, 1, 1, ARG3, ARG4, ARG5);
        GL_FUNC(ext_glGetHistogramEXT)(ARG1, bARG2, ARG3, ARG4, values.getPtr());
        GL_LOG ("glGetHistogramEXT GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetHistogramParameterfv(CPU* cpu) {
    if (!ext_glGetHistogramParameterfv)
        kpanic("ext_glGetHistogramParameterfv is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetHistogramParameterfv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetHistogramParameterfv GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetHistogramParameterfvEXT(CPU* cpu) {
    if (!ext_glGetHistogramParameterfvEXT)
        kpanic("ext_glGetHistogramParameterfvEXT is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetHistogramParameterfvEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetHistogramParameterfvEXT GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetHistogramParameteriv(CPU* cpu) {
    if (!ext_glGetHistogramParameteriv)
        kpanic("ext_glGetHistogramParameteriv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetHistogramParameteriv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetHistogramParameteriv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetHistogramParameterivEXT(CPU* cpu) {
    if (!ext_glGetHistogramParameterivEXT)
        kpanic("ext_glGetHistogramParameterivEXT is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetHistogramParameterivEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetHistogramParameterivEXT GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetHistogramParameterxvOES(CPU* cpu) {
    if (!ext_glGetHistogramParameterxvOES)
        kpanic("ext_glGetHistogramParameterxvOES is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetHistogramParameterxvOES)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetHistogramParameterxvOES GLenum target=%d, GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetImageHandleARB(CPU* cpu) {
    if (!ext_glGetImageHandleARB)
        kpanic("ext_glGetImageHandleARB is NULL");
    {
    U64 ret=GL_FUNC(ext_glGetImageHandleARB)(ARG1, ARG2, bARG3, ARG4, ARG5);
    EAX = (U32)ret;EDX = (U32)(ret >> 32);
    GL_LOG ("glGetImageHandleARB GLuint texture=%d, GLint level=%d, GLboolean layered=%d, GLint layer=%d, GLenum format=%d",ARG1,ARG2,bARG3,ARG4,ARG5);
    }
}
void glcommon_glGetImageHandleNV(CPU* cpu) {
    if (!ext_glGetImageHandleNV)
        kpanic("ext_glGetImageHandleNV is NULL");
    {
    U64 ret=GL_FUNC(ext_glGetImageHandleNV)(ARG1, ARG2, bARG3, ARG4, ARG5);
    EAX = (U32)ret;EDX = (U32)(ret >> 32);
    GL_LOG ("glGetImageHandleNV GLuint texture=%d, GLint level=%d, GLboolean layered=%d, GLint layer=%d, GLenum format=%d",ARG1,ARG2,bARG3,ARG4,ARG5);
    }
}
void glcommon_glGetImageTransformParameterfvHP(CPU* cpu) {
    if (!ext_glGetImageTransformParameterfvHP)
        kpanic("ext_glGetImageTransformParameterfvHP is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetImageTransformParameterfvHP)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetImageTransformParameterfvHP GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetImageTransformParameterivHP(CPU* cpu) {
    if (!ext_glGetImageTransformParameterivHP)
        kpanic("ext_glGetImageTransformParameterivHP is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetImageTransformParameterivHP)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetImageTransformParameterivHP GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetInfoLogARB(CPU* cpu) {
    if (!ext_glGetInfoLogARB)
        kpanic("ext_glGetInfoLogARB is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG3, 1);
        MarshalReadWrite< GLcharARB> infoLog(cpu, ARG4, ARG2);
        GL_FUNC(ext_glGetInfoLogARB)(INDEX_TO_HANDLE(hARG1), ARG2, length.getPtr(), infoLog.getPtr());
        GL_LOG ("glGetInfoLogARB GLhandleARB obj=%d, GLsizei maxLength=%d, GLsizei* length=%.08x, GLcharARB* infoLog=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetInstrumentsSGIX(CPU* cpu) {
    if (!ext_glGetInstrumentsSGIX)
        kpanic("ext_glGetInstrumentsSGIX is NULL");
    {
    EAX=GL_FUNC(ext_glGetInstrumentsSGIX)();
    GL_LOG ("glGetInstrumentsSGIX");
    }
}
void glcommon_glGetInteger64i_v(CPU* cpu) {
    if (!ext_glGetInteger64i_v)
        kpanic("ext_glGetInteger64i_v is NULL");
    {
        MarshalReadWrite<GLint64> data(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetInteger64i_v)(ARG1, ARG2, data.getPtr());
        GL_LOG ("glGetInteger64i_v GLenum target=%d, GLuint index=%d, GLint64* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetInteger64v(CPU* cpu) {
    if (!ext_glGetInteger64v)
        kpanic("ext_glGetInteger64v is NULL");
    {
        MarshalReadWrite<GLint64> data(cpu, ARG2, getSize(ARG1));
        GL_FUNC(ext_glGetInteger64v)(ARG1, data.getPtr());
        GL_LOG ("glGetInteger64v GLenum pname=%d, GLint64* data=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetIntegerIndexedvEXT(CPU* cpu) {
    if (!ext_glGetIntegerIndexedvEXT)
        kpanic("ext_glGetIntegerIndexedvEXT is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetIntegerIndexedvEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetIntegerIndexedvEXT GLenum target=%d, GLuint index=%d, GLint* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetIntegeri_v(CPU* cpu) {
    if (!ext_glGetIntegeri_v)
        kpanic("ext_glGetIntegeri_v is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetIntegeri_v)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetIntegeri_v GLenum target=%d, GLuint index=%d, GLint* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetIntegerui64i_vNV(CPU* cpu) {
    if (!ext_glGetIntegerui64i_vNV)
        kpanic("ext_glGetIntegerui64i_vNV is NULL");
    {
        MarshalReadWrite<GLuint64EXT> result(cpu, ARG3, getSize(ARG1));
        GL_FUNC(ext_glGetIntegerui64i_vNV)(ARG1, ARG2, result.getPtr());
        GL_LOG ("glGetIntegerui64i_vNV GLenum value=%d, GLuint index=%d, GLuint64EXT* result=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetIntegerui64vNV(CPU* cpu) {
    if (!ext_glGetIntegerui64vNV)
        kpanic("ext_glGetIntegerui64vNV is NULL");
    {
        MarshalReadWrite<GLuint64EXT> result(cpu, ARG2, getSize(ARG1));
        GL_FUNC(ext_glGetIntegerui64vNV)(ARG1, result.getPtr());
        GL_LOG ("glGetIntegerui64vNV GLenum value=%d, GLuint64EXT* result=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetInternalformatSampleivNV(CPU* cpu) {
    if (!ext_glGetInternalformatSampleivNV)
        kpanic("ext_glGetInternalformatSampleivNV is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG6, ARG5);
        GL_FUNC(ext_glGetInternalformatSampleivNV)(ARG1, ARG2, ARG3, ARG4, ARG5, params.getPtr());
        GL_LOG ("glGetInternalformatSampleivNV GLenum target=%d, GLenum internalformat=%d, GLsizei samples=%d, GLenum pname=%d, GLsizei bufSize=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetInternalformati64v(CPU* cpu) {
    if (!ext_glGetInternalformati64v)
        kpanic("ext_glGetInternalformati64v is NULL");
    {
        MarshalReadWrite<GLint64> params(cpu, ARG5, ARG4);
        GL_FUNC(ext_glGetInternalformati64v)(ARG1, ARG2, ARG3, ARG4, params.getPtr());
        GL_LOG ("glGetInternalformati64v GLenum target=%d, GLenum internalformat=%d, GLenum pname=%d, GLsizei bufSize=%d, GLint64* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetInternalformativ(CPU* cpu) {
    if (!ext_glGetInternalformativ)
        kpanic("ext_glGetInternalformativ is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG5, ARG4);
        GL_FUNC(ext_glGetInternalformativ)(ARG1, ARG2, ARG3, ARG4, params.getPtr());
        GL_LOG ("glGetInternalformativ GLenum target=%d, GLenum internalformat=%d, GLenum pname=%d, GLsizei bufSize=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetInvariantBooleanvEXT(CPU* cpu) {
    if (!ext_glGetInvariantBooleanvEXT)
        kpanic("ext_glGetInvariantBooleanvEXT is NULL");
    {
        MarshalReadWrite<GLboolean> rw(cpu, ARG3, 1);
        GL_FUNC(ext_glGetInvariantBooleanvEXT)(ARG1, ARG2, rw.getPtr());
        GL_LOG ("glGetInvariantBooleanvEXT GLuint id=%d, GLenum value=%d, GLboolean* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetInvariantFloatvEXT(CPU* cpu) {
    if (!ext_glGetInvariantFloatvEXT)
        kpanic("ext_glGetInvariantFloatvEXT is NULL");
    {
        MarshalReadWrite<GLfloat> data(cpu, ARG3, 1);
        GL_FUNC(ext_glGetInvariantFloatvEXT)(ARG1, ARG2, data.getPtr());
        GL_LOG ("glGetInvariantFloatvEXT GLuint id=%d, GLenum value=%d, GLfloat* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetInvariantIntegervEXT(CPU* cpu) {
    if (!ext_glGetInvariantIntegervEXT)
        kpanic("ext_glGetInvariantIntegervEXT is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetInvariantIntegervEXT)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetInvariantIntegervEXT GLuint id=%d, GLenum value=%d, GLint* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetLightxOES(CPU* cpu) {
    if (!ext_glGetLightxOES)
        kpanic("ext_glGetLightxOES is NULL");
    {
    GL_FUNC(ext_glGetLightxOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetLightxOES GLenum light=%d, GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetListParameterfvSGIX(CPU* cpu) {
    if (!ext_glGetListParameterfvSGIX)
        kpanic("ext_glGetListParameterfvSGIX is NULL");
    {
    GL_FUNC(ext_glGetListParameterfvSGIX)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetListParameterfvSGIX GLuint list=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetListParameterivSGIX(CPU* cpu) {
    if (!ext_glGetListParameterivSGIX)
        kpanic("ext_glGetListParameterivSGIX is NULL");
    {
    GL_FUNC(ext_glGetListParameterivSGIX)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetListParameterivSGIX GLuint list=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetLocalConstantBooleanvEXT(CPU* cpu) {
    if (!ext_glGetLocalConstantBooleanvEXT)
        kpanic("ext_glGetLocalConstantBooleanvEXT is NULL");
    {
    GL_FUNC(ext_glGetLocalConstantBooleanvEXT)(ARG1, ARG2, (GLboolean*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetLocalConstantBooleanvEXT GLuint id=%d, GLenum value=%d, GLboolean* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetLocalConstantFloatvEXT(CPU* cpu) {
    if (!ext_glGetLocalConstantFloatvEXT)
        kpanic("ext_glGetLocalConstantFloatvEXT is NULL");
    {
    GL_FUNC(ext_glGetLocalConstantFloatvEXT)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetLocalConstantFloatvEXT GLuint id=%d, GLenum value=%d, GLfloat* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetLocalConstantIntegervEXT(CPU* cpu) {
    if (!ext_glGetLocalConstantIntegervEXT)
        kpanic("ext_glGetLocalConstantIntegervEXT is NULL");
    {
    GL_FUNC(ext_glGetLocalConstantIntegervEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetLocalConstantIntegervEXT GLuint id=%d, GLenum value=%d, GLint* data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMapAttribParameterfvNV(CPU* cpu) {
    if (!ext_glGetMapAttribParameterfvNV)
        kpanic("ext_glGetMapAttribParameterfvNV is NULL");
    {
    GL_FUNC(ext_glGetMapAttribParameterfvNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMapAttribParameterfvNV GLenum target=%d, GLuint index=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMapAttribParameterivNV(CPU* cpu) {
    if (!ext_glGetMapAttribParameterivNV)
        kpanic("ext_glGetMapAttribParameterivNV is NULL");
    {
    GL_FUNC(ext_glGetMapAttribParameterivNV)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMapAttribParameterivNV GLenum target=%d, GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMapControlPointsNV(CPU* cpu) {
    if (!ext_glGetMapControlPointsNV)
        kpanic("ext_glGetMapControlPointsNV is NULL");
    {
    GL_FUNC(ext_glGetMapControlPointsNV)(ARG1, ARG2, ARG3, ARG4, ARG5, bARG6, (void*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glGetMapControlPointsNV GLenum target=%d, GLuint index=%d, GLenum type=%d, GLsizei ustride=%d, GLsizei vstride=%d, GLboolean packed=%d, void* points=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,bARG6,ARG7);
    }
}
void glcommon_glGetMapParameterfvNV(CPU* cpu) {
    if (!ext_glGetMapParameterfvNV)
        kpanic("ext_glGetMapParameterfvNV is NULL");
    {
    GL_FUNC(ext_glGetMapParameterfvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetMapParameterfvNV GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMapParameterivNV(CPU* cpu) {
    if (!ext_glGetMapParameterivNV)
        kpanic("ext_glGetMapParameterivNV is NULL");
    {
    GL_FUNC(ext_glGetMapParameterivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetMapParameterivNV GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMapxvOES(CPU* cpu) {
    if (!ext_glGetMapxvOES)
        kpanic("ext_glGetMapxvOES is NULL");
    {
    GL_FUNC(ext_glGetMapxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetMapxvOES GLenum target=%d, GLenum query=%d, GLfixed* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMaterialxOES(CPU* cpu) {
    if (!ext_glGetMaterialxOES)
        kpanic("ext_glGetMaterialxOES is NULL");
    {
    GL_FUNC(ext_glGetMaterialxOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glGetMaterialxOES GLenum face=%d, GLenum pname=%d, GLfixed param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMinmax(CPU* cpu) {
    if (!ext_glGetMinmax)
        kpanic("ext_glGetMinmax is NULL");
    {
    GL_FUNC(ext_glGetMinmax)(ARG1, bARG2, ARG3, ARG4, marshalArray<GLbyte>(cpu, ARG5, get_bytes_per_pixel(ARG3, ARG4)*2));
    GL_LOG ("glGetMinmax GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetMinmaxEXT(CPU* cpu) {
    if (!ext_glGetMinmaxEXT)
        kpanic("ext_glGetMinmaxEXT is NULL");
    {
    GL_FUNC(ext_glGetMinmaxEXT)(ARG1, bARG2, ARG3, ARG4, marshalArray<GLbyte>(cpu, ARG5, get_bytes_per_pixel(ARG3, ARG4) * 2));
    GL_LOG ("glGetMinmaxEXT GLenum target=%d, GLboolean reset=%d, GLenum format=%d, GLenum type=%d, void* values=%.08x",ARG1,bARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetMinmaxParameterfv(CPU* cpu) {
    if (!ext_glGetMinmaxParameterfv)
        kpanic("ext_glGetMinmaxParameterfv is NULL");
    {
        MarshalReadWrite<GLfloat> rw(cpu, ARG3, 1);
        GL_FUNC(ext_glGetMinmaxParameterfv)(ARG1, ARG2, rw.getPtr());
        GL_LOG ("glGetMinmaxParameterfv GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMinmaxParameterfvEXT(CPU* cpu) {
    if (!ext_glGetMinmaxParameterfvEXT)
        kpanic("ext_glGetMinmaxParameterfvEXT is NULL");
    {
        MarshalReadWrite<GLfloat> rw(cpu, ARG3, 1);
    GL_FUNC(ext_glGetMinmaxParameterfvEXT)(ARG1, ARG2, rw.getPtr());
    GL_LOG ("glGetMinmaxParameterfvEXT GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMinmaxParameteriv(CPU* cpu) {
    if (!ext_glGetMinmaxParameteriv)
        kpanic("ext_glGetMinmaxParameteriv is NULL");
    {
        MarshalReadWrite<GLint> rw(cpu, ARG3, 1);
        GL_FUNC(ext_glGetMinmaxParameteriv)(ARG1, ARG2, rw.getPtr());
        GL_LOG ("glGetMinmaxParameteriv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMinmaxParameterivEXT(CPU* cpu) {
    if (!ext_glGetMinmaxParameterivEXT)
        kpanic("ext_glGetMinmaxParameterivEXT is NULL");
    {
        MarshalReadWrite<GLint> rw(cpu, ARG3, 1);
    GL_FUNC(ext_glGetMinmaxParameterivEXT)(ARG1, ARG2, rw.getPtr());
    GL_LOG ("glGetMinmaxParameterivEXT GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMultiTexEnvfvEXT(CPU* cpu) {
    if (!ext_glGetMultiTexEnvfvEXT)
        kpanic("ext_glGetMultiTexEnvfvEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexEnvfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexEnvfvEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexEnvivEXT(CPU* cpu) {
    if (!ext_glGetMultiTexEnvivEXT)
        kpanic("ext_glGetMultiTexEnvivEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexEnvivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexEnvivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexGendvEXT(CPU* cpu) {
    if (!ext_glGetMultiTexGendvEXT)
        kpanic("ext_glGetMultiTexGendvEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexGendvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexGendvEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexGenfvEXT(CPU* cpu) {
    if (!ext_glGetMultiTexGenfvEXT)
        kpanic("ext_glGetMultiTexGenfvEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexGenfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexGenfvEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexGenivEXT(CPU* cpu) {
    if (!ext_glGetMultiTexGenivEXT)
        kpanic("ext_glGetMultiTexGenivEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexGenivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexGenivEXT GLenum texunit=%d, GLenum coord=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexImageEXT(CPU* cpu) {
    if (!ext_glGetMultiTexImageEXT)
        kpanic("ext_glGetMultiTexImageEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexImageEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetMultiTexImageEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetMultiTexLevelParameterfvEXT(CPU* cpu) {
    if (!ext_glGetMultiTexLevelParameterfvEXT)
        kpanic("ext_glGetMultiTexLevelParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexLevelParameterfvEXT)(ARG1, ARG2, ARG3, ARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetMultiTexLevelParameterfvEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetMultiTexLevelParameterivEXT(CPU* cpu) {
    if (!ext_glGetMultiTexLevelParameterivEXT)
        kpanic("ext_glGetMultiTexLevelParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexLevelParameterivEXT)(ARG1, ARG2, ARG3, ARG4, (GLint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetMultiTexLevelParameterivEXT GLenum texunit=%d, GLenum target=%d, GLint level=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetMultiTexParameterIivEXT(CPU* cpu) {
    if (!ext_glGetMultiTexParameterIivEXT)
        kpanic("ext_glGetMultiTexParameterIivEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexParameterIivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexParameterIivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexParameterIuivEXT(CPU* cpu) {
    if (!ext_glGetMultiTexParameterIuivEXT)
        kpanic("ext_glGetMultiTexParameterIuivEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexParameterIuivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexParameterIuivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexParameterfvEXT(CPU* cpu) {
    if (!ext_glGetMultiTexParameterfvEXT)
        kpanic("ext_glGetMultiTexParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexParameterfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexParameterfvEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultiTexParameterivEXT(CPU* cpu) {
    if (!ext_glGetMultiTexParameterivEXT)
        kpanic("ext_glGetMultiTexParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetMultiTexParameterivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetMultiTexParameterivEXT GLenum texunit=%d, GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetMultisamplefv(CPU* cpu) {
    if (!ext_glGetMultisamplefv)
        kpanic("ext_glGetMultisamplefv is NULL");
    {
    GL_FUNC(ext_glGetMultisamplefv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetMultisamplefv GLenum pname=%d, GLuint index=%d, GLfloat* val=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetMultisamplefvNV(CPU* cpu) {
    if (!ext_glGetMultisamplefvNV)
        kpanic("ext_glGetMultisamplefvNV is NULL");
    {
    GL_FUNC(ext_glGetMultisamplefvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetMultisamplefvNV GLenum pname=%d, GLuint index=%d, GLfloat* val=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedBufferParameteri64v(CPU* cpu) {
    if (!ext_glGetNamedBufferParameteri64v)
        kpanic("ext_glGetNamedBufferParameteri64v is NULL");
    {
    GL_FUNC(ext_glGetNamedBufferParameteri64v)(ARG1, ARG2, (GLint64*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedBufferParameteri64v GLuint buffer=%d, GLenum pname=%d, GLint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedBufferParameteriv(CPU* cpu) {
    if (!ext_glGetNamedBufferParameteriv)
        kpanic("ext_glGetNamedBufferParameteriv is NULL");
    {
    GL_FUNC(ext_glGetNamedBufferParameteriv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedBufferParameteriv GLuint buffer=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedBufferParameterivEXT(CPU* cpu) {
    if (!ext_glGetNamedBufferParameterivEXT)
        kpanic("ext_glGetNamedBufferParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedBufferParameterivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedBufferParameterivEXT GLuint buffer=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedBufferParameterui64vNV(CPU* cpu) {
    if (!ext_glGetNamedBufferParameterui64vNV)
        kpanic("ext_glGetNamedBufferParameterui64vNV is NULL");
    {
    GL_FUNC(ext_glGetNamedBufferParameterui64vNV)(ARG1, ARG2, (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedBufferParameterui64vNV GLuint buffer=%d, GLenum pname=%d, GLuint64EXT* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedBufferPointerv(CPU* cpu) {
    if (!ext_glGetNamedBufferPointerv)
        kpanic("ext_glGetNamedBufferPointerv is NULL");
    {
    GLint size=0;void* p;GL_FUNC(ext_glGetNamedBufferPointerv)(ARG1, ARG2, &p);
    ext_glGetNamedBufferParameteriv(ARG1, GL_BUFFER_SIZE, &size); cpu->memory->writed(ARG3, marshalBackp(cpu, p, size));
    GL_LOG ("glGetNamedBufferPointerv GLuint buffer=%d, GLenum pname=%d, void** params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedBufferPointervEXT(CPU* cpu) {
    if (!ext_glGetNamedBufferPointervEXT)
        kpanic("ext_glGetNamedBufferPointervEXT is NULL");
    {
    GLint size=0;void* p;GL_FUNC(ext_glGetNamedBufferPointervEXT)(ARG1, ARG2, &p);
    ext_glGetNamedBufferParameterivEXT(ARG1, GL_BUFFER_SIZE, &size); cpu->memory->writed(ARG3, marshalBackp(cpu, p, size));
    GL_LOG ("glGetNamedBufferPointervEXT GLuint buffer=%d, GLenum pname=%d, void** params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedBufferSubData(CPU* cpu) {
    if (!ext_glGetNamedBufferSubData)
        kpanic("ext_glGetNamedBufferSubData is NULL");
    {
    GL_FUNC(ext_glGetNamedBufferSubData)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedBufferSubData GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedBufferSubDataEXT(CPU* cpu) {
    if (!ext_glGetNamedBufferSubDataEXT)
        kpanic("ext_glGetNamedBufferSubDataEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedBufferSubDataEXT)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedBufferSubDataEXT GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedFramebufferAttachmentParameteriv(CPU* cpu) {
    if (!ext_glGetNamedFramebufferAttachmentParameteriv)
        kpanic("ext_glGetNamedFramebufferAttachmentParameteriv is NULL");
    {
    GL_FUNC(ext_glGetNamedFramebufferAttachmentParameteriv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedFramebufferAttachmentParameteriv GLuint framebuffer=%d, GLenum attachment=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedFramebufferAttachmentParameterivEXT(CPU* cpu) {
    if (!ext_glGetNamedFramebufferAttachmentParameterivEXT)
        kpanic("ext_glGetNamedFramebufferAttachmentParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedFramebufferAttachmentParameterivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedFramebufferAttachmentParameterivEXT GLuint framebuffer=%d, GLenum attachment=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedFramebufferParameteriv(CPU* cpu) {
    if (!ext_glGetNamedFramebufferParameteriv)
        kpanic("ext_glGetNamedFramebufferParameteriv is NULL");
    {
    GL_FUNC(ext_glGetNamedFramebufferParameteriv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedFramebufferParameteriv GLuint framebuffer=%d, GLenum pname=%d, GLint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedFramebufferParameterivEXT(CPU* cpu) {
    if (!ext_glGetNamedFramebufferParameterivEXT)
        kpanic("ext_glGetNamedFramebufferParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedFramebufferParameterivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedFramebufferParameterivEXT GLuint framebuffer=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedProgramLocalParameterIivEXT(CPU* cpu) {
    if (!ext_glGetNamedProgramLocalParameterIivEXT)
        kpanic("ext_glGetNamedProgramLocalParameterIivEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedProgramLocalParameterIivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedProgramLocalParameterIivEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedProgramLocalParameterIuivEXT(CPU* cpu) {
    if (!ext_glGetNamedProgramLocalParameterIuivEXT)
        kpanic("ext_glGetNamedProgramLocalParameterIuivEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedProgramLocalParameterIuivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedProgramLocalParameterIuivEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedProgramLocalParameterdvEXT(CPU* cpu) {
    if (!ext_glGetNamedProgramLocalParameterdvEXT)
        kpanic("ext_glGetNamedProgramLocalParameterdvEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedProgramLocalParameterdvEXT)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedProgramLocalParameterdvEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedProgramLocalParameterfvEXT(CPU* cpu) {
    if (!ext_glGetNamedProgramLocalParameterfvEXT)
        kpanic("ext_glGetNamedProgramLocalParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedProgramLocalParameterfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedProgramLocalParameterfvEXT GLuint program=%d, GLenum target=%d, GLuint index=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedProgramStringEXT(CPU* cpu) {
    if (!ext_glGetNamedProgramStringEXT)
        kpanic("ext_glGetNamedProgramStringEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedProgramStringEXT)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedProgramStringEXT GLuint program=%d, GLenum target=%d, GLenum pname=%d, void* string=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedProgramivEXT(CPU* cpu) {
    if (!ext_glGetNamedProgramivEXT)
        kpanic("ext_glGetNamedProgramivEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedProgramivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedProgramivEXT GLuint program=%d, GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNamedRenderbufferParameteriv(CPU* cpu) {
    if (!ext_glGetNamedRenderbufferParameteriv)
        kpanic("ext_glGetNamedRenderbufferParameteriv is NULL");
    {
    GL_FUNC(ext_glGetNamedRenderbufferParameteriv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedRenderbufferParameteriv GLuint renderbuffer=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedRenderbufferParameterivEXT(CPU* cpu) {
    if (!ext_glGetNamedRenderbufferParameterivEXT)
        kpanic("ext_glGetNamedRenderbufferParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetNamedRenderbufferParameterivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetNamedRenderbufferParameterivEXT GLuint renderbuffer=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetNamedStringARB(CPU* cpu) {
    if (!ext_glGetNamedStringARB)
        kpanic("ext_glGetNamedStringARB is NULL");
    {
    GL_FUNC(ext_glGetNamedStringARB)(ARG1, marshalsz(cpu, ARG2), ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0), (GLchar*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetNamedStringARB GLint namelen=%d, const GLchar* name=%.08x, GLsizei bufSize=%d, GLint* stringlen=%.08x, GLchar* string=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetNamedStringivARB(CPU* cpu) {
    if (!ext_glGetNamedStringivARB)
        kpanic("ext_glGetNamedStringivARB is NULL");
    {
    GL_FUNC(ext_glGetNamedStringivARB)(ARG1, marshalsz(cpu, ARG2), ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetNamedStringivARB GLint namelen=%d, const GLchar* name=%.08x, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetNextPerfQueryIdINTEL(CPU* cpu) {
    if (!ext_glGetNextPerfQueryIdINTEL)
        kpanic("ext_glGetNextPerfQueryIdINTEL is NULL");
    {
    GL_FUNC(ext_glGetNextPerfQueryIdINTEL)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetNextPerfQueryIdINTEL GLuint queryId=%d, GLuint* nextQueryId=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetObjectBufferfvATI(CPU* cpu) {
    if (!ext_glGetObjectBufferfvATI)
        kpanic("ext_glGetObjectBufferfvATI is NULL");
    {
    GL_FUNC(ext_glGetObjectBufferfvATI)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetObjectBufferfvATI GLuint buffer=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetObjectBufferivATI(CPU* cpu) {
    if (!ext_glGetObjectBufferivATI)
        kpanic("ext_glGetObjectBufferivATI is NULL");
    {
    GL_FUNC(ext_glGetObjectBufferivATI)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetObjectBufferivATI GLuint buffer=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetObjectLabel(CPU* cpu) {
    if (!ext_glGetObjectLabel)
        kpanic("ext_glGetObjectLabel is NULL");
    {
    GL_FUNC(ext_glGetObjectLabel)(ARG1, ARG2, ARG3, (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLchar*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetObjectLabel GLenum identifier=%d, GLuint name=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* label=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetObjectLabelEXT(CPU* cpu) {
    if (!ext_glGetObjectLabelEXT)
        kpanic("ext_glGetObjectLabelEXT is NULL");
    {
    GL_FUNC(ext_glGetObjectLabelEXT)(ARG1, ARG2, ARG3, (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLchar*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetObjectLabelEXT GLenum type=%d, GLuint object=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* label=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetObjectParameterfvARB(CPU* cpu) {
    if (!ext_glGetObjectParameterfvARB)
        kpanic("ext_glGetObjectParameterfvARB is NULL");
    {
        MarshalReadWrite<GLfloat> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetObjectParameterfvARB)(INDEX_TO_HANDLE(hARG1), ARG2, params.getPtr());
     GL_LOG ("glGetObjectParameterfvARB GLhandleARB obj=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetObjectParameterivAPPLE(CPU* cpu) {
    if (!ext_glGetObjectParameterivAPPLE)
        kpanic("ext_glGetObjectParameterivAPPLE is NULL");
    {
    GL_FUNC(ext_glGetObjectParameterivAPPLE)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetObjectParameterivAPPLE GLenum objectType=%d, GLuint name=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetObjectParameterivARB(CPU* cpu) {
    if (!ext_glGetObjectParameterivARB)
        kpanic("ext_glGetObjectParameterivARB is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetObjectParameterivARB)(INDEX_TO_HANDLE(hARG1), ARG2, params.getPtr());
        GL_LOG ("glGetObjectParameterivARB GLhandleARB obj=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetObjectPtrLabel(CPU* cpu) {
    if (!ext_glGetObjectPtrLabel)
        kpanic("ext_glGetObjectPtrLabel is NULL");
    {
    GL_FUNC(ext_glGetObjectPtrLabel)((void*)marshalp(cpu, 0, ARG1, 0), ARG2, (GLsizei*)marshalp(cpu, 0, ARG3, 0), (GLchar*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetObjectPtrLabel const void* ptr=%.08x, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* label=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetOcclusionQueryivNV(CPU* cpu) {
    if (!ext_glGetOcclusionQueryivNV)
        kpanic("ext_glGetOcclusionQueryivNV is NULL");
    {
    GL_FUNC(ext_glGetOcclusionQueryivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetOcclusionQueryivNV GLuint id=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetOcclusionQueryuivNV(CPU* cpu) {
    if (!ext_glGetOcclusionQueryuivNV)
        kpanic("ext_glGetOcclusionQueryuivNV is NULL");
    {
    GL_FUNC(ext_glGetOcclusionQueryuivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetOcclusionQueryuivNV GLuint id=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPathColorGenfvNV(CPU* cpu) {
    if (!ext_glGetPathColorGenfvNV)
        kpanic("ext_glGetPathColorGenfvNV is NULL");
    {
    GL_FUNC(ext_glGetPathColorGenfvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPathColorGenfvNV GLenum color=%d, GLenum pname=%d, GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPathColorGenivNV(CPU* cpu) {
    if (!ext_glGetPathColorGenivNV)
        kpanic("ext_glGetPathColorGenivNV is NULL");
    {
    GL_FUNC(ext_glGetPathColorGenivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPathColorGenivNV GLenum color=%d, GLenum pname=%d, GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPathCommandsNV(CPU* cpu) {
    if (!ext_glGetPathCommandsNV)
        kpanic("ext_glGetPathCommandsNV is NULL");
    {
    GL_FUNC(ext_glGetPathCommandsNV)(ARG1, (GLubyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetPathCommandsNV GLuint path=%d, GLubyte* commands=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetPathCoordsNV(CPU* cpu) {
    if (!ext_glGetPathCoordsNV)
        kpanic("ext_glGetPathCoordsNV is NULL");
    {
    GL_FUNC(ext_glGetPathCoordsNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetPathCoordsNV GLuint path=%d, GLfloat* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetPathDashArrayNV(CPU* cpu) {
    if (!ext_glGetPathDashArrayNV)
        kpanic("ext_glGetPathDashArrayNV is NULL");
    {
    GL_FUNC(ext_glGetPathDashArrayNV)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetPathDashArrayNV GLuint path=%d, GLfloat* dashArray=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetPathLengthNV(CPU* cpu) {
    if (!ext_glGetPathLengthNV)
        kpanic("ext_glGetPathLengthNV is NULL");
    {
    struct int2Float i2f;i2f.f=GL_FUNC(ext_glGetPathLengthNV)(ARG1, ARG2, ARG3);
    EAX=i2f.i;
    GL_LOG ("glGetPathLengthNV GLuint path=%d, GLsizei startSegment=%d, GLsizei numSegments=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPathMetricRangeNV(CPU* cpu) {
    if (!ext_glGetPathMetricRangeNV)
        kpanic("ext_glGetPathMetricRangeNV is NULL");
    {
    GL_FUNC(ext_glGetPathMetricRangeNV)(ARG1, ARG2, ARG3, ARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetPathMetricRangeNV GLbitfield metricQueryMask=%d, GLuint firstPathName=%d, GLsizei numPaths=%d, GLsizei stride=%d, GLfloat* metrics=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetPathMetricsNV(CPU* cpu) {
    if (!ext_glGetPathMetricsNV)
        kpanic("ext_glGetPathMetricsNV is NULL");
    {
    GL_FUNC(ext_glGetPathMetricsNV)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0), ARG5, ARG6, (GLfloat*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glGetPathMetricsNV GLbitfield metricQueryMask=%d, GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLsizei stride=%d, GLfloat* metrics=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetPathParameterfvNV(CPU* cpu) {
    if (!ext_glGetPathParameterfvNV)
        kpanic("ext_glGetPathParameterfvNV is NULL");
    {
    GL_FUNC(ext_glGetPathParameterfvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPathParameterfvNV GLuint path=%d, GLenum pname=%d, GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPathParameterivNV(CPU* cpu) {
    if (!ext_glGetPathParameterivNV)
        kpanic("ext_glGetPathParameterivNV is NULL");
    {
    GL_FUNC(ext_glGetPathParameterivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPathParameterivNV GLuint path=%d, GLenum pname=%d, GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPathSpacingNV(CPU* cpu) {
    if (!ext_glGetPathSpacingNV)
        kpanic("ext_glGetPathSpacingNV is NULL");
    {
    GL_FUNC(ext_glGetPathSpacingNV)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0), ARG5, fARG6, fARG7, ARG8, (GLfloat*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glGetPathSpacingNV GLenum pathListMode=%d, GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLfloat advanceScale=%f, GLfloat kerningScale=%f, GLenum transformType=%d, GLfloat* returnedSpacing=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,fARG6,fARG7,ARG8,ARG9);
    }
}
void glcommon_glGetPathTexGenfvNV(CPU* cpu) {
    if (!ext_glGetPathTexGenfvNV)
        kpanic("ext_glGetPathTexGenfvNV is NULL");
    {
    GL_FUNC(ext_glGetPathTexGenfvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPathTexGenfvNV GLenum texCoordSet=%d, GLenum pname=%d, GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPathTexGenivNV(CPU* cpu) {
    if (!ext_glGetPathTexGenivNV)
        kpanic("ext_glGetPathTexGenivNV is NULL");
    {
    GL_FUNC(ext_glGetPathTexGenivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPathTexGenivNV GLenum texCoordSet=%d, GLenum pname=%d, GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPerfCounterInfoINTEL(CPU* cpu) {
    if (!ext_glGetPerfCounterInfoINTEL)
        kpanic("ext_glGetPerfCounterInfoINTEL is NULL");
    {
    GL_FUNC(ext_glGetPerfCounterInfoINTEL)(ARG1, ARG2, ARG3, (GLchar*)marshalp(cpu, 0, ARG4, 0), ARG5, (GLchar*)marshalp(cpu, 0, ARG6, 0), (GLuint*)marshalp(cpu, 0, ARG7, 0), (GLuint*)marshalp(cpu, 0, ARG8, 0), (GLuint*)marshalp(cpu, 0, ARG9, 0), (GLuint*)marshalp(cpu, 0, ARG10, 0), (GLuint64*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glGetPerfCounterInfoINTEL GLuint queryId=%d, GLuint counterId=%d, GLuint counterNameLength=%d, GLchar* counterName=%.08x, GLuint counterDescLength=%d, GLchar* counterDesc=%.08x, GLuint* counterOffset=%.08x, GLuint* counterDataSize=%.08x, GLuint* counterTypeEnum=%.08x, GLuint* counterDataTypeEnum=%.08x, GLuint64* rawCounterMaxValue=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glGetPerfMonitorCounterDataAMD(CPU* cpu) {
    if (!ext_glGetPerfMonitorCounterDataAMD)
        kpanic("ext_glGetPerfMonitorCounterDataAMD is NULL");
    {
    GL_FUNC(ext_glGetPerfMonitorCounterDataAMD)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0), (GLint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetPerfMonitorCounterDataAMD GLuint monitor=%d, GLenum pname=%d, GLsizei dataSize=%d, GLuint* data=%.08x, GLint* bytesWritten=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetPerfMonitorCounterInfoAMD(CPU* cpu) {
    if (!ext_glGetPerfMonitorCounterInfoAMD)
        kpanic("ext_glGetPerfMonitorCounterInfoAMD is NULL");
    {
    GL_FUNC(ext_glGetPerfMonitorCounterInfoAMD)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetPerfMonitorCounterInfoAMD GLuint group=%d, GLuint counter=%d, GLenum pname=%d, void* data=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetPerfMonitorCounterStringAMD(CPU* cpu) {
    if (!ext_glGetPerfMonitorCounterStringAMD)
        kpanic("ext_glGetPerfMonitorCounterStringAMD is NULL");
    {
    GL_FUNC(ext_glGetPerfMonitorCounterStringAMD)(ARG1, ARG2, ARG3, (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLchar*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetPerfMonitorCounterStringAMD GLuint group=%d, GLuint counter=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* counterString=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetPerfMonitorCountersAMD(CPU* cpu) {
    if (!ext_glGetPerfMonitorCountersAMD)
        kpanic("ext_glGetPerfMonitorCountersAMD is NULL");
    {
    GL_FUNC(ext_glGetPerfMonitorCountersAMD)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0), (GLint*)marshalp(cpu, 0, ARG3, 0), ARG4, (GLuint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetPerfMonitorCountersAMD GLuint group=%d, GLint* numCounters=%.08x, GLint* maxActiveCounters=%.08x, GLsizei counterSize=%d, GLuint* counters=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetPerfMonitorGroupStringAMD(CPU* cpu) {
    if (!ext_glGetPerfMonitorGroupStringAMD)
        kpanic("ext_glGetPerfMonitorGroupStringAMD is NULL");
    {
    GL_FUNC(ext_glGetPerfMonitorGroupStringAMD)(ARG1, ARG2, (GLsizei*)marshalp(cpu, 0, ARG3, 0), (GLchar*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetPerfMonitorGroupStringAMD GLuint group=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* groupString=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetPerfMonitorGroupsAMD(CPU* cpu) {
    if (!ext_glGetPerfMonitorGroupsAMD)
        kpanic("ext_glGetPerfMonitorGroupsAMD is NULL");
    {
    GL_FUNC(ext_glGetPerfMonitorGroupsAMD)((GLint*)marshalp(cpu, 0, ARG1, 0), ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPerfMonitorGroupsAMD GLint* numGroups=%.08x, GLsizei groupsSize=%d, GLuint* groups=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPerfQueryDataINTEL(CPU* cpu) {
    if (!ext_glGetPerfQueryDataINTEL)
        kpanic("ext_glGetPerfQueryDataINTEL is NULL");
    {
    GL_FUNC(ext_glGetPerfQueryDataINTEL)(ARG1, ARG2, ARG3, (GLvoid*)marshalp(cpu, 0, ARG4, 0), (GLuint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetPerfQueryDataINTEL GLuint queryHandle=%d, GLuint flags=%d, GLsizei dataSize=%d, GLvoid* data=%.08x, GLuint* bytesWritten=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetPerfQueryIdByNameINTEL(CPU* cpu) {
    if (!ext_glGetPerfQueryIdByNameINTEL)
        kpanic("ext_glGetPerfQueryIdByNameINTEL is NULL");
    {
    GL_FUNC(ext_glGetPerfQueryIdByNameINTEL)((GLchar*)marshalp(cpu, 0, ARG1, 0), (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetPerfQueryIdByNameINTEL GLchar* queryName=%.08x, GLuint* queryId=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetPerfQueryInfoINTEL(CPU* cpu) {
    if (!ext_glGetPerfQueryInfoINTEL)
        kpanic("ext_glGetPerfQueryInfoINTEL is NULL");
    {
    GL_FUNC(ext_glGetPerfQueryInfoINTEL)(ARG1, ARG2, (GLchar*)marshalp(cpu, 0, ARG3, 0), (GLuint*)marshalp(cpu, 0, ARG4, 0), (GLuint*)marshalp(cpu, 0, ARG5, 0), (GLuint*)marshalp(cpu, 0, ARG6, 0), (GLuint*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glGetPerfQueryInfoINTEL GLuint queryId=%d, GLuint queryNameLength=%d, GLchar* queryName=%.08x, GLuint* dataSize=%.08x, GLuint* noCounters=%.08x, GLuint* noInstances=%.08x, GLuint* capsMask=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetPixelMapxv(CPU* cpu) {
    if (!ext_glGetPixelMapxv)
        kpanic("ext_glGetPixelMapxv is NULL");
    {
    GL_FUNC(ext_glGetPixelMapxv)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPixelMapxv GLenum map=%d, GLint size=%d, GLfixed* values=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPixelTexGenParameterfvSGIS(CPU* cpu) {
    if (!ext_glGetPixelTexGenParameterfvSGIS)
        kpanic("ext_glGetPixelTexGenParameterfvSGIS is NULL");
    {
    GL_FUNC(ext_glGetPixelTexGenParameterfvSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetPixelTexGenParameterfvSGIS GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetPixelTexGenParameterivSGIS(CPU* cpu) {
    if (!ext_glGetPixelTexGenParameterivSGIS)
        kpanic("ext_glGetPixelTexGenParameterivSGIS is NULL");
    {
    GL_FUNC(ext_glGetPixelTexGenParameterivSGIS)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetPixelTexGenParameterivSGIS GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetPixelTransformParameterfvEXT(CPU* cpu) {
    if (!ext_glGetPixelTransformParameterfvEXT)
        kpanic("ext_glGetPixelTransformParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glGetPixelTransformParameterfvEXT)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPixelTransformParameterfvEXT GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPixelTransformParameterivEXT(CPU* cpu) {
    if (!ext_glGetPixelTransformParameterivEXT)
        kpanic("ext_glGetPixelTransformParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetPixelTransformParameterivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetPixelTransformParameterivEXT GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPointerIndexedvEXT(CPU* cpu) {
    if (!ext_glGetPointerIndexedvEXT)
        kpanic("ext_glGetPointerIndexedvEXT is NULL");
    {
    void* p;GL_FUNC(ext_glGetPointerIndexedvEXT)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetPointerIndexedvEXT GLenum target=%d, GLuint index=%d, void** data=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPointeri_vEXT(CPU* cpu) {
    if (!ext_glGetPointeri_vEXT)
        kpanic("ext_glGetPointeri_vEXT is NULL");
    {
    void* p;GL_FUNC(ext_glGetPointeri_vEXT)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetPointeri_vEXT GLenum pname=%d, GLuint index=%d, void** params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetPointervEXT(CPU* cpu) {
    if (!ext_glGetPointervEXT)
        kpanic("ext_glGetPointervEXT is NULL");
    {
    void* p;GL_FUNC(ext_glGetPointervEXT)(ARG1, &p);
    cpu->memory->writed(ARG2, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetPointervEXT GLenum pname=%d, void** params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetProgramBinary(CPU* cpu) {
    if (!ext_glGetProgramBinary)
        kpanic("ext_glGetProgramBinary is NULL");
    {
    GL_FUNC(ext_glGetProgramBinary)(ARG1, ARG2, (GLsizei*)marshalp(cpu, 0, ARG3, 0), (GLenum*)marshalp(cpu, 0, ARG4, 0), (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetProgramBinary GLuint program=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLenum* binaryFormat=%.08x, void* binary=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetProgramEnvParameterIivNV(CPU* cpu) {
    if (!ext_glGetProgramEnvParameterIivNV)
        kpanic("ext_glGetProgramEnvParameterIivNV is NULL");
    {
    GL_FUNC(ext_glGetProgramEnvParameterIivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramEnvParameterIivNV GLenum target=%d, GLuint index=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramEnvParameterIuivNV(CPU* cpu) {
    if (!ext_glGetProgramEnvParameterIuivNV)
        kpanic("ext_glGetProgramEnvParameterIuivNV is NULL");
    {
    GL_FUNC(ext_glGetProgramEnvParameterIuivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramEnvParameterIuivNV GLenum target=%d, GLuint index=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramEnvParameterdvARB(CPU* cpu) {
    if (!ext_glGetProgramEnvParameterdvARB)
        kpanic("ext_glGetProgramEnvParameterdvARB is NULL");
    {
    GL_FUNC(ext_glGetProgramEnvParameterdvARB)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramEnvParameterdvARB GLenum target=%d, GLuint index=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramEnvParameterfvARB(CPU* cpu) {
    if (!ext_glGetProgramEnvParameterfvARB)
        kpanic("ext_glGetProgramEnvParameterfvARB is NULL");
    {
    GL_FUNC(ext_glGetProgramEnvParameterfvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramEnvParameterfvARB GLenum target=%d, GLuint index=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramInfoLog(CPU* cpu) {
    if (!ext_glGetProgramInfoLog)
        kpanic("ext_glGetProgramInfoLog is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG3, 1);
        MarshalReadWrite<GLchar> infoLog(cpu, ARG4, ARG2);
        GL_FUNC(ext_glGetProgramInfoLog)(ARG1, ARG2, length.getPtr(), infoLog.getPtr());
        GL_LOG ("glGetProgramInfoLog GLuint program=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* infoLog=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramInterfaceiv(CPU* cpu) {
    if (!ext_glGetProgramInterfaceiv)
        kpanic("ext_glGetProgramInterfaceiv is NULL");
    {
    GL_FUNC(ext_glGetProgramInterfaceiv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetProgramInterfaceiv GLuint program=%d, GLenum programInterface=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramLocalParameterIivNV(CPU* cpu) {
    if (!ext_glGetProgramLocalParameterIivNV)
        kpanic("ext_glGetProgramLocalParameterIivNV is NULL");
    {
    GL_FUNC(ext_glGetProgramLocalParameterIivNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramLocalParameterIivNV GLenum target=%d, GLuint index=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramLocalParameterIuivNV(CPU* cpu) {
    if (!ext_glGetProgramLocalParameterIuivNV)
        kpanic("ext_glGetProgramLocalParameterIuivNV is NULL");
    {
    GL_FUNC(ext_glGetProgramLocalParameterIuivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramLocalParameterIuivNV GLenum target=%d, GLuint index=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramLocalParameterdvARB(CPU* cpu) {
    if (!ext_glGetProgramLocalParameterdvARB)
        kpanic("ext_glGetProgramLocalParameterdvARB is NULL");
    {
    GL_FUNC(ext_glGetProgramLocalParameterdvARB)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramLocalParameterdvARB GLenum target=%d, GLuint index=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramLocalParameterfvARB(CPU* cpu) {
    if (!ext_glGetProgramLocalParameterfvARB)
        kpanic("ext_glGetProgramLocalParameterfvARB is NULL");
    {
    GL_FUNC(ext_glGetProgramLocalParameterfvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramLocalParameterfvARB GLenum target=%d, GLuint index=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramNamedParameterdvNV(CPU* cpu) {
    if (!ext_glGetProgramNamedParameterdvNV)
        kpanic("ext_glGetProgramNamedParameterdvNV is NULL");
    {
    GL_FUNC(ext_glGetProgramNamedParameterdvNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0), (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetProgramNamedParameterdvNV GLuint id=%d, GLsizei len=%d, const GLubyte* name=%.08x, GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramNamedParameterfvNV(CPU* cpu) {
    if (!ext_glGetProgramNamedParameterfvNV)
        kpanic("ext_glGetProgramNamedParameterfvNV is NULL");
    {
    GL_FUNC(ext_glGetProgramNamedParameterfvNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetProgramNamedParameterfvNV GLuint id=%d, GLsizei len=%d, const GLubyte* name=%.08x, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramParameterdvNV(CPU* cpu) {
    if (!ext_glGetProgramParameterdvNV)
        kpanic("ext_glGetProgramParameterdvNV is NULL");
    {
    GL_FUNC(ext_glGetProgramParameterdvNV)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetProgramParameterdvNV GLenum target=%d, GLuint index=%d, GLenum pname=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramParameterfvNV(CPU* cpu) {
    if (!ext_glGetProgramParameterfvNV)
        kpanic("ext_glGetProgramParameterfvNV is NULL");
    {
    GL_FUNC(ext_glGetProgramParameterfvNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetProgramParameterfvNV GLenum target=%d, GLuint index=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramPipelineInfoLog(CPU* cpu) {
    if (!ext_glGetProgramPipelineInfoLog)
        kpanic("ext_glGetProgramPipelineInfoLog is NULL");
    {
    GL_FUNC(ext_glGetProgramPipelineInfoLog)(ARG1, ARG2, (GLsizei*)marshalp(cpu, 0, ARG3, 0), (GLchar*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetProgramPipelineInfoLog GLuint pipeline=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* infoLog=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramPipelineiv(CPU* cpu) {
    if (!ext_glGetProgramPipelineiv)
        kpanic("ext_glGetProgramPipelineiv is NULL");
    {
    GL_FUNC(ext_glGetProgramPipelineiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramPipelineiv GLuint pipeline=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramResourceIndex(CPU* cpu) {
    if (!ext_glGetProgramResourceIndex)
        kpanic("ext_glGetProgramResourceIndex is NULL");
    {
    EAX=GL_FUNC(ext_glGetProgramResourceIndex)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glGetProgramResourceIndex GLuint program=%d, GLenum programInterface=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramResourceLocation(CPU* cpu) {
    if (!ext_glGetProgramResourceLocation)
        kpanic("ext_glGetProgramResourceLocation is NULL");
    {
    EAX=GL_FUNC(ext_glGetProgramResourceLocation)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glGetProgramResourceLocation GLuint program=%d, GLenum programInterface=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramResourceLocationIndex(CPU* cpu) {
    if (!ext_glGetProgramResourceLocationIndex)
        kpanic("ext_glGetProgramResourceLocationIndex is NULL");
    {
    EAX=GL_FUNC(ext_glGetProgramResourceLocationIndex)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glGetProgramResourceLocationIndex GLuint program=%d, GLenum programInterface=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramResourceName(CPU* cpu) {
    if (!ext_glGetProgramResourceName)
        kpanic("ext_glGetProgramResourceName is NULL");
    {
    GL_FUNC(ext_glGetProgramResourceName)(ARG1, ARG2, ARG3, ARG4, (GLsizei*)marshalp(cpu, 0, ARG5, 0), (GLchar*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetProgramResourceName GLuint program=%d, GLenum programInterface=%d, GLuint index=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetProgramResourcefvNV(CPU* cpu) {
    if (!ext_glGetProgramResourcefvNV)
        kpanic("ext_glGetProgramResourcefvNV is NULL");
    {
    GL_FUNC(ext_glGetProgramResourcefvNV)(ARG1, ARG2, ARG3, ARG4, (GLenum*)marshalp(cpu, 0, ARG5, 0), ARG6, (GLsizei*)marshalp(cpu, 0, ARG7, 0), (GLfloat*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glGetProgramResourcefvNV GLuint program=%d, GLenum programInterface=%d, GLuint index=%d, GLsizei propCount=%d, const GLenum* props=%.08x, GLsizei bufSize=%d, GLsizei* length=%.08x, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glGetProgramResourceiv(CPU* cpu) {
    if (!ext_glGetProgramResourceiv)
        kpanic("ext_glGetProgramResourceiv is NULL");
    {
    GL_FUNC(ext_glGetProgramResourceiv)(ARG1, ARG2, ARG3, ARG4, (GLenum*)marshalp(cpu, 0, ARG5, 0), ARG6, (GLsizei*)marshalp(cpu, 0, ARG7, 0), (GLint*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glGetProgramResourceiv GLuint program=%d, GLenum programInterface=%d, GLuint index=%d, GLsizei propCount=%d, const GLenum* props=%.08x, GLsizei bufSize=%d, GLsizei* length=%.08x, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glGetProgramStageiv(CPU* cpu) {
    if (!ext_glGetProgramStageiv)
        kpanic("ext_glGetProgramStageiv is NULL");
    {
    GL_FUNC(ext_glGetProgramStageiv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetProgramStageiv GLuint program=%d, GLenum shadertype=%d, GLenum pname=%d, GLint* values=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetProgramStringARB(CPU* cpu) {
    if (!ext_glGetProgramStringARB)
        kpanic("ext_glGetProgramStringARB is NULL");
    {
    GL_FUNC(ext_glGetProgramStringARB)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramStringARB GLenum target=%d, GLenum pname=%d, void* string=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramStringNV(CPU* cpu) {
    if (!ext_glGetProgramStringNV)
        kpanic("ext_glGetProgramStringNV is NULL");
    {
    GL_FUNC(ext_glGetProgramStringNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramStringNV GLuint id=%d, GLenum pname=%d, GLubyte* program=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramSubroutineParameteruivNV(CPU* cpu) {
    if (!ext_glGetProgramSubroutineParameteruivNV)
        kpanic("ext_glGetProgramSubroutineParameteruivNV is NULL");
    {
    GL_FUNC(ext_glGetProgramSubroutineParameteruivNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetProgramSubroutineParameteruivNV GLenum target=%d, GLuint index=%d, GLuint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramiv(CPU* cpu) {
    if (!ext_glGetProgramiv)
        kpanic("ext_glGetProgramiv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetProgramiv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetProgramiv GLuint program=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramivARB(CPU* cpu) {
    if (!ext_glGetProgramivARB)
        kpanic("ext_glGetProgramivARB is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetProgramivARB)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetProgramivARB GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetProgramivNV(CPU* cpu) {
    if (!ext_glGetProgramivNV)
        kpanic("ext_glGetProgramivNV is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetProgramivNV)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetProgramivNV GLuint id=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryBufferObjecti64v(CPU* cpu) {
    if (!ext_glGetQueryBufferObjecti64v)
        kpanic("ext_glGetQueryBufferObjecti64v is NULL");
    {
    GL_FUNC(ext_glGetQueryBufferObjecti64v)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glGetQueryBufferObjecti64v GLuint id=%d, GLuint buffer=%d, GLenum pname=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetQueryBufferObjectiv(CPU* cpu) {
    if (!ext_glGetQueryBufferObjectiv)
        kpanic("ext_glGetQueryBufferObjectiv is NULL");
    {
    GL_FUNC(ext_glGetQueryBufferObjectiv)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glGetQueryBufferObjectiv GLuint id=%d, GLuint buffer=%d, GLenum pname=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetQueryBufferObjectui64v(CPU* cpu) {
    if (!ext_glGetQueryBufferObjectui64v)
        kpanic("ext_glGetQueryBufferObjectui64v is NULL");
    {
    GL_FUNC(ext_glGetQueryBufferObjectui64v)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glGetQueryBufferObjectui64v GLuint id=%d, GLuint buffer=%d, GLenum pname=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetQueryBufferObjectuiv(CPU* cpu) {
    if (!ext_glGetQueryBufferObjectuiv)
        kpanic("ext_glGetQueryBufferObjectuiv is NULL");
    {
        MarshalReadWrite<GLint> rw(cpu, ARG4, 1);
        GL_FUNC(ext_glGetQueryBufferObjectuiv)(ARG1, ARG2, ARG3, ARG4);
        GL_LOG ("glGetQueryBufferObjectuiv GLuint id=%d, GLuint buffer=%d, GLenum pname=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetQueryIndexediv(CPU* cpu) {
    if (!ext_glGetQueryIndexediv)
        kpanic("ext_glGetQueryIndexediv is NULL");
    {
        MarshalReadWrite<GLint> rw(cpu, ARG4, 1);
        GL_FUNC(ext_glGetQueryIndexediv)(ARG1, ARG2, ARG3, rw.getPtr());
        GL_LOG ("glGetQueryIndexediv GLenum target=%d, GLuint index=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetQueryObjecti64v(CPU* cpu) {
    if (!ext_glGetQueryObjecti64v)
        kpanic("ext_glGetQueryObjecti64v is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjecti64v)(ARG1, ARG2, (GLint64*)pARG3);
        } else {
            MarshalReadWrite<GLint64> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjecti64v)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjecti64v GLuint id=%d, GLenum pname=%d, GLint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryObjecti64vEXT(CPU* cpu) {
    if (!ext_glGetQueryObjecti64vEXT)
        kpanic("ext_glGetQueryObjecti64vEXT is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjecti64vEXT)(ARG1, ARG2, (GLint64*)pARG3);
        } else {
            MarshalReadWrite<GLint64> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjecti64vEXT)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjecti64vEXT GLuint id=%d, GLenum pname=%d, GLint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryObjectiv(CPU* cpu) {
    if (!ext_glGetQueryObjectiv)
        kpanic("ext_glGetQueryObjectiv is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjectiv)(ARG1, ARG2, (GLint*)pARG3);
        } else {
            MarshalReadWrite<GLint> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjectiv)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjectiv GLuint id=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryObjectivARB(CPU* cpu) {
    if (!ext_glGetQueryObjectivARB)
        kpanic("ext_glGetQueryObjectivARB is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjectivARB)(ARG1, ARG2, (GLint*)pARG3);
        } else {
            MarshalReadWrite<GLint> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjectivARB)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjectivARB GLuint id=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryObjectui64v(CPU* cpu) {
    if (!ext_glGetQueryObjectui64v)
        kpanic("ext_glGetQueryObjectui64v is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjectui64v)(ARG1, ARG2, (GLuint64*)pARG3);
        } else {
            MarshalReadWrite<GLuint64> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjectui64v)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjectui64v GLuint id=%d, GLenum pname=%d, GLuint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryObjectui64vEXT(CPU* cpu) {
    if (!ext_glGetQueryObjectui64vEXT)
        kpanic("ext_glGetQueryObjectui64vEXT is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjectui64vEXT)(ARG1, ARG2, (GLuint64*)pARG3);
        } else {
            MarshalReadWrite<GLuint64> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjectui64vEXT)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjectui64vEXT GLuint id=%d, GLenum pname=%d, GLuint64* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryObjectuiv(CPU* cpu) {
    if (!ext_glGetQueryObjectuiv)
        kpanic("ext_glGetQueryObjectuiv is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjectuiv)(ARG1, ARG2, (GLuint*)pARG3);
        } else {
            MarshalReadWrite<GLuint> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjectuiv)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjectuiv GLuint id=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryObjectuivARB(CPU* cpu) {
    if (!ext_glGetQueryObjectuivARB)
        kpanic("ext_glGetQueryObjectuivARB is NULL");
    {
        if (RESULT_BUFFER()) {
            GL_FUNC(ext_glGetQueryObjectuivARB)(ARG1, ARG2, (GLuint*)pARG3);
        } else {
            MarshalReadWrite<GLuint> rw(cpu, ARG3, 1);
            GL_FUNC(ext_glGetQueryObjectuivARB)(ARG1, ARG2, rw.getPtr());
        }
        GL_LOG ("glGetQueryObjectuivARB GLuint id=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryiv(CPU* cpu) {
    if (!ext_glGetQueryiv)
        kpanic("ext_glGetQueryiv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetQueryiv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetQueryiv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetQueryivARB(CPU* cpu) {
    if (!ext_glGetQueryivARB)
        kpanic("ext_glGetQueryivARB is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetQueryivARB)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetQueryivARB GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetRenderbufferParameteriv(CPU* cpu) {
    if (!ext_glGetRenderbufferParameteriv)
        kpanic("ext_glGetRenderbufferParameteriv is NULL");
    {
        MarshalReadWrite<GLint> rw(cpu, ARG3, 1);
        GL_FUNC(ext_glGetRenderbufferParameteriv)(ARG1, ARG2, rw.getPtr());
        GL_LOG ("glGetRenderbufferParameteriv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetRenderbufferParameterivEXT(CPU* cpu) {
    if (!ext_glGetRenderbufferParameterivEXT)
        kpanic("ext_glGetRenderbufferParameterivEXT is NULL");
    {
        MarshalReadWrite<GLint> rw(cpu, ARG3, 1);
        GL_FUNC(ext_glGetRenderbufferParameterivEXT)(ARG1, ARG2, rw.getPtr());
        GL_LOG ("glGetRenderbufferParameterivEXT GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSamplerParameterIiv(CPU* cpu) {
    if (!ext_glGetSamplerParameterIiv)
        kpanic("ext_glGetSamplerParameterIiv is NULL");
    {
    GL_FUNC(ext_glGetSamplerParameterIiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetSamplerParameterIiv GLuint sampler=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSamplerParameterIuiv(CPU* cpu) {
    if (!ext_glGetSamplerParameterIuiv)
        kpanic("ext_glGetSamplerParameterIuiv is NULL");
    {
    GL_FUNC(ext_glGetSamplerParameterIuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetSamplerParameterIuiv GLuint sampler=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSamplerParameterfv(CPU* cpu) {
    if (!ext_glGetSamplerParameterfv)
        kpanic("ext_glGetSamplerParameterfv is NULL");
    {
    GL_FUNC(ext_glGetSamplerParameterfv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetSamplerParameterfv GLuint sampler=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSamplerParameteriv(CPU* cpu) {
    if (!ext_glGetSamplerParameteriv)
        kpanic("ext_glGetSamplerParameteriv is NULL");
    {
    GL_FUNC(ext_glGetSamplerParameteriv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetSamplerParameteriv GLuint sampler=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSeparableFilter(CPU* cpu) {
    if (!ext_glGetSeparableFilter)
        kpanic("ext_glGetSeparableFilter is NULL");
    {
    GL_FUNC(ext_glGetSeparableFilter)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0), (void*)marshalp(cpu, 0, ARG5, 0), (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetSeparableFilter GLenum target=%d, GLenum format=%d, GLenum type=%d, void* row=%.08x, void* column=%.08x, void* span=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetSeparableFilterEXT(CPU* cpu) {
    if (!ext_glGetSeparableFilterEXT)
        kpanic("ext_glGetSeparableFilterEXT is NULL");
    {
    GL_FUNC(ext_glGetSeparableFilterEXT)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0), (void*)marshalp(cpu, 0, ARG5, 0), (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetSeparableFilterEXT GLenum target=%d, GLenum format=%d, GLenum type=%d, void* row=%.08x, void* column=%.08x, void* span=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetShaderInfoLog(CPU* cpu) {
    if (!ext_glGetShaderInfoLog)
        kpanic("ext_glGetShaderInfoLog is NULL");
    {
    GL_FUNC(ext_glGetShaderInfoLog)(ARG1, ARG2, (GLsizei*)marshalp(cpu, 0, ARG3, 0), (GLchar*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetShaderInfoLog GLuint shader=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* infoLog=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetShaderPrecisionFormat(CPU* cpu) {
    if (!ext_glGetShaderPrecisionFormat)
        kpanic("ext_glGetShaderPrecisionFormat is NULL");
    {
    GL_FUNC(ext_glGetShaderPrecisionFormat)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0), (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetShaderPrecisionFormat GLenum shadertype=%d, GLenum precisiontype=%d, GLint* range=%.08x, GLint* precision=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetShaderSource(CPU* cpu) {
    if (!ext_glGetShaderSource)
        kpanic("ext_glGetShaderSource is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG3, 1);
        MarshalReadWrite<GLchar> source(cpu, ARG4, ARG2);
        GL_FUNC(ext_glGetShaderSource)(ARG1, ARG2, length.getPtr(), source.getPtr());
        GL_LOG ("glGetShaderSource GLuint shader=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLchar* source=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetShaderSourceARB(CPU* cpu) {
    if (!ext_glGetShaderSourceARB)
        kpanic("ext_glGetShaderSourceARB is NULL");
    {
        MarshalReadWrite<GLsizei> length(cpu, ARG3, 1);
        MarshalReadWrite<GLcharARB> source(cpu, ARG4, ARG2);
        GL_FUNC(ext_glGetShaderSourceARB)(INDEX_TO_HANDLE(hARG1), ARG2, length.getPtr(), source.getPtr());
        GL_LOG ("glGetShaderSourceARB GLhandleARB obj=%d, GLsizei maxLength=%d, GLsizei* length=%.08x, GLcharARB* source=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetShaderiv(CPU* cpu) {
    if (!ext_glGetShaderiv)
        kpanic("ext_glGetShaderiv is NULL");
    {
        MarshalReadWrite<GLint> params(cpu, ARG3, 1);
        GL_FUNC(ext_glGetShaderiv)(ARG1, ARG2, params.getPtr());
        GL_LOG ("glGetShaderiv GLuint shader=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSharpenTexFuncSGIS(CPU* cpu) {
    if (!ext_glGetSharpenTexFuncSGIS)
        kpanic("ext_glGetSharpenTexFuncSGIS is NULL");
    {
    GL_FUNC(ext_glGetSharpenTexFuncSGIS)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetSharpenTexFuncSGIS GLenum target=%d, GLfloat* points=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetStageIndexNV(CPU* cpu) {
    if (!ext_glGetStageIndexNV)
        kpanic("ext_glGetStageIndexNV is NULL");
    {
    EAX=GL_FUNC(ext_glGetStageIndexNV)(ARG1);
    GL_LOG ("glGetStageIndexNV GLenum shadertype=%d",ARG1);
    }
}
void glcommon_glGetSubroutineIndex(CPU* cpu) {
    if (!ext_glGetSubroutineIndex)
        kpanic("ext_glGetSubroutineIndex is NULL");
    {
    EAX=GL_FUNC(ext_glGetSubroutineIndex)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glGetSubroutineIndex GLuint program=%d, GLenum shadertype=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSubroutineUniformLocation(CPU* cpu) {
    if (!ext_glGetSubroutineUniformLocation)
        kpanic("ext_glGetSubroutineUniformLocation is NULL");
    {
    EAX=GL_FUNC(ext_glGetSubroutineUniformLocation)(ARG1, ARG2, marshalsz(cpu, ARG3));
    GL_LOG ("glGetSubroutineUniformLocation GLuint program=%d, GLenum shadertype=%d, const GLchar* name=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetSynciv(CPU* cpu) {
    if (!ext_glGetSynciv)
        kpanic("ext_glGetSynciv is NULL");
    {
    GL_FUNC(ext_glGetSynciv)(marshalSync(cpu, ARG1), ARG2, ARG3, (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetSynciv GLsync sync=%d, GLenum pname=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLint* values=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetTexBumpParameterfvATI(CPU* cpu) {
    if (!ext_glGetTexBumpParameterfvATI)
        kpanic("ext_glGetTexBumpParameterfvATI is NULL");
    {
    GL_FUNC(ext_glGetTexBumpParameterfvATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetTexBumpParameterfvATI GLenum pname=%d, GLfloat* param=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetTexBumpParameterivATI(CPU* cpu) {
    if (!ext_glGetTexBumpParameterivATI)
        kpanic("ext_glGetTexBumpParameterivATI is NULL");
    {
    GL_FUNC(ext_glGetTexBumpParameterivATI)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glGetTexBumpParameterivATI GLenum pname=%d, GLint* param=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetTexEnvxvOES(CPU* cpu) {
    if (!ext_glGetTexEnvxvOES)
        kpanic("ext_glGetTexEnvxvOES is NULL");
    {
    GL_FUNC(ext_glGetTexEnvxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexEnvxvOES GLenum target=%d, GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexFilterFuncSGIS(CPU* cpu) {
    if (!ext_glGetTexFilterFuncSGIS)
        kpanic("ext_glGetTexFilterFuncSGIS is NULL");
    {
    GL_FUNC(ext_glGetTexFilterFuncSGIS)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexFilterFuncSGIS GLenum target=%d, GLenum filter=%d, GLfloat* weights=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexGenxvOES(CPU* cpu) {
    if (!ext_glGetTexGenxvOES)
        kpanic("ext_glGetTexGenxvOES is NULL");
    {
    GL_FUNC(ext_glGetTexGenxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexGenxvOES GLenum coord=%d, GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexLevelParameterxvOES(CPU* cpu) {
    if (!ext_glGetTexLevelParameterxvOES)
        kpanic("ext_glGetTexLevelParameterxvOES is NULL");
    {
    GL_FUNC(ext_glGetTexLevelParameterxvOES)(ARG1, ARG2, ARG3, (GLfixed*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTexLevelParameterxvOES GLenum target=%d, GLint level=%d, GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTexParameterIiv(CPU* cpu) {
    if (!ext_glGetTexParameterIiv)
        kpanic("ext_glGetTexParameterIiv is NULL");
    {
    GL_FUNC(ext_glGetTexParameterIiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexParameterIiv GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexParameterIivEXT(CPU* cpu) {
    if (!ext_glGetTexParameterIivEXT)
        kpanic("ext_glGetTexParameterIivEXT is NULL");
    {
    GL_FUNC(ext_glGetTexParameterIivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexParameterIivEXT GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexParameterIuiv(CPU* cpu) {
    if (!ext_glGetTexParameterIuiv)
        kpanic("ext_glGetTexParameterIuiv is NULL");
    {
    GL_FUNC(ext_glGetTexParameterIuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexParameterIuiv GLenum target=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexParameterIuivEXT(CPU* cpu) {
    if (!ext_glGetTexParameterIuivEXT)
        kpanic("ext_glGetTexParameterIuivEXT is NULL");
    {
    GL_FUNC(ext_glGetTexParameterIuivEXT)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexParameterIuivEXT GLenum target=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexParameterPointervAPPLE(CPU* cpu) {
    if (!ext_glGetTexParameterPointervAPPLE)
        kpanic("ext_glGetTexParameterPointervAPPLE is NULL");
    {
    void* p;GL_FUNC(ext_glGetTexParameterPointervAPPLE)(ARG1, ARG2, &p);
    cpu->memory->writed(ARG3, marshalBackp(cpu, p, sizeof(void*)));
    GL_LOG ("glGetTexParameterPointervAPPLE GLenum target=%d, GLenum pname=%d, void** params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTexParameterxvOES(CPU* cpu) {
    if (!ext_glGetTexParameterxvOES)
        kpanic("ext_glGetTexParameterxvOES is NULL");
    {
    GL_FUNC(ext_glGetTexParameterxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTexParameterxvOES GLenum target=%d, GLenum pname=%d, GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTextureHandleARB(CPU* cpu) {
    if (!ext_glGetTextureHandleARB)
        kpanic("ext_glGetTextureHandleARB is NULL");
    {
    U64 ret=GL_FUNC(ext_glGetTextureHandleARB)(ARG1);
    EAX = (U32)ret;EDX = (U32)(ret >> 32);
    GL_LOG ("glGetTextureHandleARB GLuint texture=%d",ARG1);
    }
}
void glcommon_glGetTextureHandleNV(CPU* cpu) {
    if (!ext_glGetTextureHandleNV)
        kpanic("ext_glGetTextureHandleNV is NULL");
    {
    U64 ret=GL_FUNC(ext_glGetTextureHandleNV)(ARG1);
    EAX = (U32)ret;EDX = (U32)(ret >> 32);
    GL_LOG ("glGetTextureHandleNV GLuint texture=%d",ARG1);
    }
}
void glcommon_glGetTextureImage(CPU* cpu) {
    if (!ext_glGetTextureImage)
        kpanic("ext_glGetTextureImage is NULL");
    {
    GL_FUNC(ext_glGetTextureImage)(ARG1, ARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetTextureImage GLuint texture=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetTextureImageEXT(CPU* cpu) {
    if (!ext_glGetTextureImageEXT)
        kpanic("ext_glGetTextureImageEXT is NULL");
    {
    GL_FUNC(ext_glGetTextureImageEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, (void*)marshalp(cpu, 0, ARG6, 0));
    GL_LOG ("glGetTextureImageEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum format=%d, GLenum type=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glGetTextureLevelParameterfv(CPU* cpu) {
    if (!ext_glGetTextureLevelParameterfv)
        kpanic("ext_glGetTextureLevelParameterfv is NULL");
    {
    GL_FUNC(ext_glGetTextureLevelParameterfv)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTextureLevelParameterfv GLuint texture=%d, GLint level=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTextureLevelParameterfvEXT(CPU* cpu) {
    if (!ext_glGetTextureLevelParameterfvEXT)
        kpanic("ext_glGetTextureLevelParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glGetTextureLevelParameterfvEXT)(ARG1, ARG2, ARG3, ARG4, (GLfloat*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetTextureLevelParameterfvEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetTextureLevelParameteriv(CPU* cpu) {
    if (!ext_glGetTextureLevelParameteriv)
        kpanic("ext_glGetTextureLevelParameteriv is NULL");
    {
    GL_FUNC(ext_glGetTextureLevelParameteriv)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTextureLevelParameteriv GLuint texture=%d, GLint level=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTextureLevelParameterivEXT(CPU* cpu) {
    if (!ext_glGetTextureLevelParameterivEXT)
        kpanic("ext_glGetTextureLevelParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetTextureLevelParameterivEXT)(ARG1, ARG2, ARG3, ARG4, (GLint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glGetTextureLevelParameterivEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glGetTextureParameterIiv(CPU* cpu) {
    if (!ext_glGetTextureParameterIiv)
        kpanic("ext_glGetTextureParameterIiv is NULL");
    {
    GL_FUNC(ext_glGetTextureParameterIiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTextureParameterIiv GLuint texture=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTextureParameterIivEXT(CPU* cpu) {
    if (!ext_glGetTextureParameterIivEXT)
        kpanic("ext_glGetTextureParameterIivEXT is NULL");
    {
    GL_FUNC(ext_glGetTextureParameterIivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTextureParameterIivEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTextureParameterIuiv(CPU* cpu) {
    if (!ext_glGetTextureParameterIuiv)
        kpanic("ext_glGetTextureParameterIuiv is NULL");
    {
    GL_FUNC(ext_glGetTextureParameterIuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTextureParameterIuiv GLuint texture=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTextureParameterIuivEXT(CPU* cpu) {
    if (!ext_glGetTextureParameterIuivEXT)
        kpanic("ext_glGetTextureParameterIuivEXT is NULL");
    {
    GL_FUNC(ext_glGetTextureParameterIuivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTextureParameterIuivEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTextureParameterfv(CPU* cpu) {
    if (!ext_glGetTextureParameterfv)
        kpanic("ext_glGetTextureParameterfv is NULL");
    {
    GL_FUNC(ext_glGetTextureParameterfv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTextureParameterfv GLuint texture=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTextureParameterfvEXT(CPU* cpu) {
    if (!ext_glGetTextureParameterfvEXT)
        kpanic("ext_glGetTextureParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glGetTextureParameterfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTextureParameterfvEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTextureParameteriv(CPU* cpu) {
    if (!ext_glGetTextureParameteriv)
        kpanic("ext_glGetTextureParameteriv is NULL");
    {
    GL_FUNC(ext_glGetTextureParameteriv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTextureParameteriv GLuint texture=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTextureParameterivEXT(CPU* cpu) {
    if (!ext_glGetTextureParameterivEXT)
        kpanic("ext_glGetTextureParameterivEXT is NULL");
    {
    GL_FUNC(ext_glGetTextureParameterivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTextureParameterivEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTextureSamplerHandleARB(CPU* cpu) {
    if (!ext_glGetTextureSamplerHandleARB)
        kpanic("ext_glGetTextureSamplerHandleARB is NULL");
    {
    U64 ret=GL_FUNC(ext_glGetTextureSamplerHandleARB)(ARG1, ARG2);
    EAX = (U32)ret;EDX = (U32)(ret >> 32);
    GL_LOG ("glGetTextureSamplerHandleARB GLuint texture=%d, GLuint sampler=%d",ARG1,ARG2);
    }
}
void glcommon_glGetTextureSamplerHandleNV(CPU* cpu) {
    if (!ext_glGetTextureSamplerHandleNV)
        kpanic("ext_glGetTextureSamplerHandleNV is NULL");
    {
    U64 ret=GL_FUNC(ext_glGetTextureSamplerHandleNV)(ARG1, ARG2);
    EAX = (U32)ret;EDX = (U32)(ret >> 32);
    GL_LOG ("glGetTextureSamplerHandleNV GLuint texture=%d, GLuint sampler=%d",ARG1,ARG2);
    }
}
void glcommon_glGetTextureSubImage(CPU* cpu) {
    if (!ext_glGetTextureSubImage)
        kpanic("ext_glGetTextureSubImage is NULL");
    {
    GL_FUNC(ext_glGetTextureSubImage)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, (void*)marshalp(cpu, 0, ARG12, 0));
    GL_LOG ("glGetTextureSubImage GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLenum type=%d, GLsizei bufSize=%d, void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12);
    }
}
void glcommon_glGetTrackMatrixivNV(CPU* cpu) {
    if (!ext_glGetTrackMatrixivNV)
        kpanic("ext_glGetTrackMatrixivNV is NULL");
    {
    GL_FUNC(ext_glGetTrackMatrixivNV)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTrackMatrixivNV GLenum target=%d, GLuint address=%d, GLenum pname=%d, GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTransformFeedbackVarying(CPU* cpu) {
    if (!ext_glGetTransformFeedbackVarying)
        kpanic("ext_glGetTransformFeedbackVarying is NULL");
    {
    GL_FUNC(ext_glGetTransformFeedbackVarying)(ARG1, ARG2, ARG3, (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLsizei*)marshalp(cpu, 0, ARG5, 0), (GLenum*)marshalp(cpu, 0, ARG6, 0), (GLchar*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glGetTransformFeedbackVarying GLuint program=%d, GLuint index=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLsizei* size=%.08x, GLenum* type=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetTransformFeedbackVaryingEXT(CPU* cpu) {
    if (!ext_glGetTransformFeedbackVaryingEXT)
        kpanic("ext_glGetTransformFeedbackVaryingEXT is NULL");
    {
    GL_FUNC(ext_glGetTransformFeedbackVaryingEXT)(ARG1, ARG2, ARG3, (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLsizei*)marshalp(cpu, 0, ARG5, 0), (GLenum*)marshalp(cpu, 0, ARG6, 0), (GLchar*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glGetTransformFeedbackVaryingEXT GLuint program=%d, GLuint index=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLsizei* size=%.08x, GLenum* type=%.08x, GLchar* name=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glGetTransformFeedbackVaryingNV(CPU* cpu) {
    if (!ext_glGetTransformFeedbackVaryingNV)
        kpanic("ext_glGetTransformFeedbackVaryingNV is NULL");
    {
    GL_FUNC(ext_glGetTransformFeedbackVaryingNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTransformFeedbackVaryingNV GLuint program=%d, GLuint index=%d, GLint* location=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetTransformFeedbacki64_v(CPU* cpu) {
    if (!ext_glGetTransformFeedbacki64_v)
        kpanic("ext_glGetTransformFeedbacki64_v is NULL");
    {
    GL_FUNC(ext_glGetTransformFeedbacki64_v)(ARG1, ARG2, ARG3, (GLint64*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTransformFeedbacki64_v GLuint xfb=%d, GLenum pname=%d, GLuint index=%d, GLint64* param=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTransformFeedbacki_v(CPU* cpu) {
    if (!ext_glGetTransformFeedbacki_v)
        kpanic("ext_glGetTransformFeedbacki_v is NULL");
    {
    GL_FUNC(ext_glGetTransformFeedbacki_v)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetTransformFeedbacki_v GLuint xfb=%d, GLenum pname=%d, GLuint index=%d, GLint* param=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetTransformFeedbackiv(CPU* cpu) {
    if (!ext_glGetTransformFeedbackiv)
        kpanic("ext_glGetTransformFeedbackiv is NULL");
    {
    GL_FUNC(ext_glGetTransformFeedbackiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetTransformFeedbackiv GLuint xfb=%d, GLenum pname=%d, GLint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformBlockIndex(CPU* cpu) {
    if (!ext_glGetUniformBlockIndex)
        kpanic("ext_glGetUniformBlockIndex is NULL");
    {
    EAX=GL_FUNC(ext_glGetUniformBlockIndex)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glGetUniformBlockIndex GLuint program=%d, const GLchar* uniformBlockName=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetUniformBufferSizeEXT(CPU* cpu) {
    if (!ext_glGetUniformBufferSizeEXT)
        kpanic("ext_glGetUniformBufferSizeEXT is NULL");
    {
    EAX=GL_FUNC(ext_glGetUniformBufferSizeEXT)(ARG1, ARG2);
    GL_LOG ("glGetUniformBufferSizeEXT GLuint program=%d, GLint location=%d",ARG1,ARG2);
    }
}
void glcommon_glGetUniformIndices(CPU* cpu) {
    if (!ext_glGetUniformIndices)
        kpanic("ext_glGetUniformIndices is NULL");
    {
    GL_FUNC(ext_glGetUniformIndices)(ARG1, ARG2, (GLchar*const*)marshalpp(cpu, ARG3, ARG2, 0, -1, sizeof(GLchar)), (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glGetUniformIndices GLuint program=%d, GLsizei uniformCount=%d, const GLchar*const* uniformNames=%.08x, GLuint* uniformIndices=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glGetUniformLocation(CPU* cpu) {
    if (!ext_glGetUniformLocation)
        kpanic("ext_glGetUniformLocation is NULL");
    {
    EAX=GL_FUNC(ext_glGetUniformLocation)(ARG1, marshalsz(cpu, ARG2));
    GL_LOG ("glGetUniformLocation GLuint program=%d, const GLchar* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetUniformLocationARB(CPU* cpu) {
    if (!ext_glGetUniformLocationARB)
        kpanic("ext_glGetUniformLocationARB is NULL");
    {
    EAX=GL_FUNC(ext_glGetUniformLocationARB)(INDEX_TO_HANDLE(hARG1), marshalsz(cpu, ARG2));
    GL_LOG ("glGetUniformLocationARB GLhandleARB programObj=%d, const GLcharARB* name=%.08x",ARG1,ARG2);
    }
}
void glcommon_glGetUniformOffsetEXT(CPU* cpu) {
    if (!ext_glGetUniformOffsetEXT)
        kpanic("ext_glGetUniformOffsetEXT is NULL");
    {
    EAX=(U32)(GLintptr)GL_FUNC(ext_glGetUniformOffsetEXT)(ARG1, ARG2);
    GL_LOG ("glGetUniformOffsetEXT GLuint program=%d, GLint location=%d",ARG1,ARG2);
    }
}
void glcommon_glGetUniformSubroutineuiv(CPU* cpu) {
    if (!ext_glGetUniformSubroutineuiv)
        kpanic("ext_glGetUniformSubroutineuiv is NULL");
    {
    GL_FUNC(ext_glGetUniformSubroutineuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformSubroutineuiv GLenum shadertype=%d, GLint location=%d, GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glGetUniformdv(CPU* cpu) {
    if (!ext_glGetUniformdv)
        kpanic("ext_glGetUniformdv is NULL");
    {
    GL_FUNC(ext_glGetUniformdv)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glGetUniformdv GLuint program=%d, GLint location=%d, GLdouble* params=%.08x",ARG1,ARG2,ARG3);
    }
}
#endif
#endif
