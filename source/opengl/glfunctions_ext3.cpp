#include "boxedwine.h"

#ifndef DISABLE_GL_EXTENSIONS
#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"
#include "glMarshal.h"

void glcommon_glSetFenceNV(CPU* cpu) {
    if (!ext_glSetFenceNV)
        kpanic("ext_glSetFenceNV is NULL");
    {
    GL_FUNC(ext_glSetFenceNV)(ARG1, ARG2);
    GL_LOG ("glSetFenceNV GLuint fence=%d, GLenum condition=%d",ARG1,ARG2);
    }
}
void glcommon_glSetFragmentShaderConstantATI(CPU* cpu) {
    if (!ext_glSetFragmentShaderConstantATI)
        kpanic("ext_glSetFragmentShaderConstantATI is NULL");
    {
    GL_FUNC(ext_glSetFragmentShaderConstantATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glSetFragmentShaderConstantATI GLuint dst=%d, const GLfloat* value=%.08x",ARG1,ARG2);
    }
}
void glcommon_glSetInvariantEXT(CPU* cpu) {
    if (!ext_glSetInvariantEXT)
        kpanic("ext_glSetInvariantEXT is NULL");
    {
    GL_FUNC(ext_glSetInvariantEXT)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glSetInvariantEXT GLuint id=%d, GLenum type=%d, const void* addr=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSetLocalConstantEXT(CPU* cpu) {
    if (!ext_glSetLocalConstantEXT)
        kpanic("ext_glSetLocalConstantEXT is NULL");
    {
    GL_FUNC(ext_glSetLocalConstantEXT)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glSetLocalConstantEXT GLuint id=%d, GLenum type=%d, const void* addr=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSetMultisamplefvAMD(CPU* cpu) {
    if (!ext_glSetMultisamplefvAMD)
        kpanic("ext_glSetMultisamplefvAMD is NULL");
    {
    GL_FUNC(ext_glSetMultisamplefvAMD)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glSetMultisamplefvAMD GLenum pname=%d, GLuint index=%d, const GLfloat* val=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glShaderBinary(CPU* cpu) {
    if (!ext_glShaderBinary)
        kpanic("ext_glShaderBinary is NULL");
    {
    GL_FUNC(ext_glShaderBinary)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0), ARG3, (void*)marshalp(cpu, 0, ARG4, 0), ARG5);
    GL_LOG ("glShaderBinary GLsizei count=%d, const GLuint* shaders=%.08x, GLenum binaryformat=%d, const void* binary=%.08x, GLsizei length=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glShaderOp1EXT(CPU* cpu) {
    if (!ext_glShaderOp1EXT)
        kpanic("ext_glShaderOp1EXT is NULL");
    {
    GL_FUNC(ext_glShaderOp1EXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glShaderOp1EXT GLenum op=%d, GLuint res=%d, GLuint arg1=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glShaderOp2EXT(CPU* cpu) {
    if (!ext_glShaderOp2EXT)
        kpanic("ext_glShaderOp2EXT is NULL");
    {
    GL_FUNC(ext_glShaderOp2EXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glShaderOp2EXT GLenum op=%d, GLuint res=%d, GLuint arg1=%d, GLuint arg2=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glShaderOp3EXT(CPU* cpu) {
    if (!ext_glShaderOp3EXT)
        kpanic("ext_glShaderOp3EXT is NULL");
    {
    GL_FUNC(ext_glShaderOp3EXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glShaderOp3EXT GLenum op=%d, GLuint res=%d, GLuint arg1=%d, GLuint arg2=%d, GLuint arg3=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glShaderSource(CPU* cpu) {
    if (!ext_glShaderSource)
        kpanic("ext_glShaderSource is NULL");
    {
    GL_FUNC(ext_glShaderSource)(ARG1, ARG2, (GLchar*const*)marshalszArray<GLchar>(cpu, ARG2, ARG3, ARG4), marshalArray<GLint>(cpu, ARG4, ARG2));
    GL_LOG ("glShaderSource GLuint shader=%d, GLsizei count=%d, const GLchar*const* string=%.08x, const GLint* length=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glShaderSourceARB(CPU* cpu) {
    if (!ext_glShaderSourceARB)
        kpanic("ext_glShaderSourceARB is NULL");
    {
    GL_FUNC(ext_glShaderSourceARB)(INDEX_TO_HANDLE(hARG1), ARG2, (const GLcharARB**)marshalszArray<GLcharARB>(cpu, ARG2, ARG3, ARG4), marshalArray<GLint>(cpu, ARG4, ARG2));
    GL_LOG ("glShaderSourceARB GLhandleARB shaderObj=%d, GLsizei count=%d, const GLcharARB** string=%.08x, const GLint* length=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glShaderStorageBlockBinding(CPU* cpu) {
    if (!ext_glShaderStorageBlockBinding)
        kpanic("ext_glShaderStorageBlockBinding is NULL");
    {
    GL_FUNC(ext_glShaderStorageBlockBinding)(ARG1, ARG2, ARG3);
    GL_LOG ("glShaderStorageBlockBinding GLuint program=%d, GLuint storageBlockIndex=%d, GLuint storageBlockBinding=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSharpenTexFuncSGIS(CPU* cpu) {
    if (!ext_glSharpenTexFuncSGIS)
        kpanic("ext_glSharpenTexFuncSGIS is NULL");
    {
    // only example I found, seems like points represents pairs of floats
    // GLfloat points[] = { 0., 0.,     ?1., 1.,     ?2., 1.7,     ?4., 2. }; glSharpenTexFuncSGIS(GL_TEXTURE_2D, 4, points);
    GL_FUNC(ext_glSharpenTexFuncSGIS)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, ARG2*2));
    GL_LOG ("glSharpenTexFuncSGIS GLenum target=%d, GLsizei n=%d, const GLfloat* points=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glSpriteParameterfSGIX(CPU* cpu) {
    if (!ext_glSpriteParameterfSGIX)
        kpanic("ext_glSpriteParameterfSGIX is NULL");
    {
    GL_FUNC(ext_glSpriteParameterfSGIX)(ARG1, fARG2);
    GL_LOG ("glSpriteParameterfSGIX GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glSpriteParameterfvSGIX(CPU* cpu) {
    if (!ext_glSpriteParameterfvSGIX)
        kpanic("ext_glSpriteParameterfvSGIX is NULL");
    {
    // I'm not sure if it is always 3 floats
    // spriteTrans[0] = .2; spriteTrans[1] = .2; spriteTrans[2] = .0;    glSpriteParameterfvSGIX(GL_SPRITE_TRANSLATION_SGIX, spriteTrans)
    GL_FUNC(ext_glSpriteParameterfvSGIX)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 3));
    GL_LOG ("glSpriteParameterfvSGIX GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glSpriteParameteriSGIX(CPU* cpu) {
    if (!ext_glSpriteParameteriSGIX)
        kpanic("ext_glSpriteParameteriSGIX is NULL");
    {
    GL_FUNC(ext_glSpriteParameteriSGIX)(ARG1, ARG2);
    GL_LOG ("glSpriteParameteriSGIX GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glSpriteParameterivSGIX(CPU* cpu) {
    if (!ext_glSpriteParameterivSGIX)
        kpanic("ext_glSpriteParameterivSGIX is NULL");
    {
    GL_FUNC(ext_glSpriteParameterivSGIX)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glSpriteParameterivSGIX GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2);
    }
}
void glcommon_glStartInstrumentsSGIX(CPU* cpu) {
    if (!ext_glStartInstrumentsSGIX)
        kpanic("ext_glStartInstrumentsSGIX is NULL");
    {
    GL_FUNC(ext_glStartInstrumentsSGIX)();
    GL_LOG ("glStartInstrumentsSGIX");
    }
}
void glcommon_glStateCaptureNV(CPU* cpu) {
    if (!ext_glStateCaptureNV)
        kpanic("ext_glStateCaptureNV is NULL");
    {
    GL_FUNC(ext_glStateCaptureNV)(ARG1, ARG2);
    GL_LOG ("glStateCaptureNV GLuint state=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glStencilClearTagEXT(CPU* cpu) {
    if (!ext_glStencilClearTagEXT)
        kpanic("ext_glStencilClearTagEXT is NULL");
    {
    GL_FUNC(ext_glStencilClearTagEXT)(ARG1, ARG2);
    GL_LOG ("glStencilClearTagEXT GLsizei stencilTagBits=%d, GLuint stencilClearTag=%d",ARG1,ARG2);
    }
}
void glcommon_glStencilFillPathInstancedNV(CPU* cpu) {
    if (!ext_glStencilFillPathInstancedNV)
        kpanic("ext_glStencilFillPathInstancedNV is NULL");
    {
    GL_FUNC(ext_glStencilFillPathInstancedNV)(ARG1, ARG2, (void*)marshalType(cpu, ARG2, ARG1, ARG3), ARG4, ARG5, ARG6, ARG7, marshalArray<GLfloat>(cpu, ARG8, floatPerTransformList(ARG7)*ARG1));
    GL_LOG ("glStencilFillPathInstancedNV GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLenum fillMode=%d, GLuint mask=%d, GLenum transformType=%d, const GLfloat* transformValues=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glStencilFillPathNV(CPU* cpu) {
    if (!ext_glStencilFillPathNV)
        kpanic("ext_glStencilFillPathNV is NULL");
    {
    GL_FUNC(ext_glStencilFillPathNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glStencilFillPathNV GLuint path=%d, GLenum fillMode=%d, GLuint mask=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glStencilFuncSeparate(CPU* cpu) {
    if (!ext_glStencilFuncSeparate)
        kpanic("ext_glStencilFuncSeparate is NULL");
    {
    GL_FUNC(ext_glStencilFuncSeparate)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glStencilFuncSeparate GLenum face=%d, GLenum func=%d, GLint ref=%d, GLuint mask=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glStencilFuncSeparateATI(CPU* cpu) {
    if (!ext_glStencilFuncSeparateATI)
        kpanic("ext_glStencilFuncSeparateATI is NULL");
    {
    GL_FUNC(ext_glStencilFuncSeparateATI)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glStencilFuncSeparateATI GLenum frontfunc=%d, GLenum backfunc=%d, GLint ref=%d, GLuint mask=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glStencilMaskSeparate(CPU* cpu) {
    if (!ext_glStencilMaskSeparate)
        kpanic("ext_glStencilMaskSeparate is NULL");
    {
    GL_FUNC(ext_glStencilMaskSeparate)(ARG1, ARG2);
    GL_LOG ("glStencilMaskSeparate GLenum face=%d, GLuint mask=%d",ARG1,ARG2);
    }
}
void glcommon_glStencilOpSeparate(CPU* cpu) {
    if (!ext_glStencilOpSeparate)
        kpanic("ext_glStencilOpSeparate is NULL");
    {
    GL_FUNC(ext_glStencilOpSeparate)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glStencilOpSeparate GLenum face=%d, GLenum sfail=%d, GLenum dpfail=%d, GLenum dppass=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glStencilOpSeparateATI(CPU* cpu) {
    if (!ext_glStencilOpSeparateATI)
        kpanic("ext_glStencilOpSeparateATI is NULL");
    {
    GL_FUNC(ext_glStencilOpSeparateATI)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glStencilOpSeparateATI GLenum face=%d, GLenum sfail=%d, GLenum dpfail=%d, GLenum dppass=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glStencilOpValueAMD(CPU* cpu) {
    if (!ext_glStencilOpValueAMD)
        kpanic("ext_glStencilOpValueAMD is NULL");
    {
    GL_FUNC(ext_glStencilOpValueAMD)(ARG1, ARG2);
    GL_LOG ("glStencilOpValueAMD GLenum face=%d, GLuint value=%d",ARG1,ARG2);
    }
}
void glcommon_glStencilStrokePathInstancedNV(CPU* cpu) {
    if (!ext_glStencilStrokePathInstancedNV)
        kpanic("ext_glStencilStrokePathInstancedNV is NULL");
    {
    GL_FUNC(ext_glStencilStrokePathInstancedNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7, marshalArray<GLfloat>(cpu, ARG8, floatPerTransformList(ARG7)*ARG1));
    GL_LOG ("glStencilStrokePathInstancedNV GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLint reference=%d, GLuint mask=%d, GLenum transformType=%d, const GLfloat* transformValues=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glStencilStrokePathNV(CPU* cpu) {
    if (!ext_glStencilStrokePathNV)
        kpanic("ext_glStencilStrokePathNV is NULL");
    {
    GL_FUNC(ext_glStencilStrokePathNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glStencilStrokePathNV GLuint path=%d, GLint reference=%d, GLuint mask=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glStencilThenCoverFillPathInstancedNV(CPU* cpu) {
    if (!ext_glStencilThenCoverFillPathInstancedNV)
        kpanic("ext_glStencilThenCoverFillPathInstancedNV is NULL");
    {
    GL_FUNC(ext_glStencilThenCoverFillPathInstancedNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7, ARG8, (GLfloat*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glStencilThenCoverFillPathInstancedNV GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLenum fillMode=%d, GLuint mask=%d, GLenum coverMode=%d, GLenum transformType=%d, const GLfloat* transformValues=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glStencilThenCoverFillPathNV(CPU* cpu) {
    if (!ext_glStencilThenCoverFillPathNV)
        kpanic("ext_glStencilThenCoverFillPathNV is NULL");
    {
    GL_FUNC(ext_glStencilThenCoverFillPathNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glStencilThenCoverFillPathNV GLuint path=%d, GLenum fillMode=%d, GLuint mask=%d, GLenum coverMode=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glStencilThenCoverStrokePathInstancedNV(CPU* cpu) {
    if (!ext_glStencilThenCoverStrokePathInstancedNV)
        kpanic("ext_glStencilThenCoverStrokePathInstancedNV is NULL");
    {
    GL_FUNC(ext_glStencilThenCoverStrokePathInstancedNV)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0), ARG4, ARG5, ARG6, ARG7, ARG8, (GLfloat*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glStencilThenCoverStrokePathInstancedNV GLsizei numPaths=%d, GLenum pathNameType=%d, const void* paths=%.08x, GLuint pathBase=%d, GLint reference=%d, GLuint mask=%d, GLenum coverMode=%d, GLenum transformType=%d, const GLfloat* transformValues=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glStencilThenCoverStrokePathNV(CPU* cpu) {
    if (!ext_glStencilThenCoverStrokePathNV)
        kpanic("ext_glStencilThenCoverStrokePathNV is NULL");
    {
    GL_FUNC(ext_glStencilThenCoverStrokePathNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glStencilThenCoverStrokePathNV GLuint path=%d, GLint reference=%d, GLuint mask=%d, GLenum coverMode=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glStopInstrumentsSGIX(CPU* cpu) {
    if (!ext_glStopInstrumentsSGIX)
        kpanic("ext_glStopInstrumentsSGIX is NULL");
    {
    GL_FUNC(ext_glStopInstrumentsSGIX)(ARG1);
    GL_LOG ("glStopInstrumentsSGIX GLint marker=%d",ARG1);
    }
}
void glcommon_glStringMarkerGREMEDY(CPU* cpu) {
    if (!ext_glStringMarkerGREMEDY)
        kpanic("ext_glStringMarkerGREMEDY is NULL");
    {
    GL_FUNC(ext_glStringMarkerGREMEDY)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glStringMarkerGREMEDY GLsizei len=%d, const void* string=%.08x",ARG1,ARG2);
    }
}
void glcommon_glSubpixelPrecisionBiasNV(CPU* cpu) {
    if (!ext_glSubpixelPrecisionBiasNV)
        kpanic("ext_glSubpixelPrecisionBiasNV is NULL");
    {
    GL_FUNC(ext_glSubpixelPrecisionBiasNV)(ARG1, ARG2);
    GL_LOG ("glSubpixelPrecisionBiasNV GLuint xbits=%d, GLuint ybits=%d",ARG1,ARG2);
    }
}
void glcommon_glSwizzleEXT(CPU* cpu) {
    if (!ext_glSwizzleEXT)
        kpanic("ext_glSwizzleEXT is NULL");
    {
    GL_FUNC(ext_glSwizzleEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glSwizzleEXT GLuint res=%d, GLuint in=%d, GLenum outX=%d, GLenum outY=%d, GLenum outZ=%d, GLenum outW=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glSyncTextureINTEL(CPU* cpu) {
    if (!ext_glSyncTextureINTEL)
        kpanic("ext_glSyncTextureINTEL is NULL");
    {
    GL_FUNC(ext_glSyncTextureINTEL)(ARG1);
    GL_LOG ("glSyncTextureINTEL GLuint texture=%d",ARG1);
    }
}
void glcommon_glTagSampleBufferSGIX(CPU* cpu) {
    if (!ext_glTagSampleBufferSGIX)
        kpanic("ext_glTagSampleBufferSGIX is NULL");
    {
    GL_FUNC(ext_glTagSampleBufferSGIX)();
    GL_LOG ("glTagSampleBufferSGIX");
    }
}
void glcommon_glTangent3bEXT(CPU* cpu) {
    if (!ext_glTangent3bEXT)
        kpanic("ext_glTangent3bEXT is NULL");
    {
    GL_FUNC(ext_glTangent3bEXT)(bARG1, bARG2, bARG3);
    GL_LOG ("glTangent3bEXT GLbyte tx=%d, GLbyte ty=%d, GLbyte tz=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glTangent3bvEXT(CPU* cpu) {
    if (!ext_glTangent3bvEXT)
        kpanic("ext_glTangent3bvEXT is NULL");
    {
    GL_FUNC(ext_glTangent3bvEXT)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTangent3bvEXT const GLbyte* v=%.08x",ARG1);
    }
}
void glcommon_glTangent3dEXT(CPU* cpu) {
    if (!ext_glTangent3dEXT)
        kpanic("ext_glTangent3dEXT is NULL");
    {
    GL_FUNC(ext_glTangent3dEXT)(dARG1, dARG2, dARG3);
    GL_LOG ("glTangent3dEXT GLdouble tx=%f, GLdouble ty=%f, GLdouble tz=%f",dARG1,dARG2,dARG3);
    }
}
void glcommon_glTangent3dvEXT(CPU* cpu) {
    if (!ext_glTangent3dvEXT)
        kpanic("ext_glTangent3dvEXT is NULL");
    {
    GL_FUNC(ext_glTangent3dvEXT)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTangent3dvEXT const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glTangent3fEXT(CPU* cpu) {
    if (!ext_glTangent3fEXT)
        kpanic("ext_glTangent3fEXT is NULL");
    {
    GL_FUNC(ext_glTangent3fEXT)(fARG1, fARG2, fARG3);
    GL_LOG ("glTangent3fEXT GLfloat tx=%f, GLfloat ty=%f, GLfloat tz=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glTangent3fvEXT(CPU* cpu) {
    if (!ext_glTangent3fvEXT)
        kpanic("ext_glTangent3fvEXT is NULL");
    {
    GL_FUNC(ext_glTangent3fvEXT)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTangent3fvEXT const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glTangent3iEXT(CPU* cpu) {
    if (!ext_glTangent3iEXT)
        kpanic("ext_glTangent3iEXT is NULL");
    {
    GL_FUNC(ext_glTangent3iEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glTangent3iEXT GLint tx=%d, GLint ty=%d, GLint tz=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTangent3ivEXT(CPU* cpu) {
    if (!ext_glTangent3ivEXT)
        kpanic("ext_glTangent3ivEXT is NULL");
    {
    GL_FUNC(ext_glTangent3ivEXT)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTangent3ivEXT const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glTangent3sEXT(CPU* cpu) {
    if (!ext_glTangent3sEXT)
        kpanic("ext_glTangent3sEXT is NULL");
    {
    GL_FUNC(ext_glTangent3sEXT)(sARG1, sARG2, sARG3);
    GL_LOG ("glTangent3sEXT GLshort tx=%d, GLshort ty=%d, GLshort tz=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glTangent3svEXT(CPU* cpu) {
    if (!ext_glTangent3svEXT)
        kpanic("ext_glTangent3svEXT is NULL");
    {
    GL_FUNC(ext_glTangent3svEXT)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTangent3svEXT const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glTangentPointerEXT(CPU* cpu) {
    if (!ext_glTangentPointerEXT)
        kpanic("ext_glTangentPointerEXT is NULL");
    {
    GL_FUNC(ext_glTangentPointerEXT)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTangentPointerEXT GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTbufferMask3DFX(CPU* cpu) {
    if (!ext_glTbufferMask3DFX)
        kpanic("ext_glTbufferMask3DFX is NULL");
    {
    GL_FUNC(ext_glTbufferMask3DFX)(ARG1);
    GL_LOG ("glTbufferMask3DFX GLuint mask=%d",ARG1);
    }
}
void glcommon_glTessellationFactorAMD(CPU* cpu) {
    if (!ext_glTessellationFactorAMD)
        kpanic("ext_glTessellationFactorAMD is NULL");
    {
    GL_FUNC(ext_glTessellationFactorAMD)(fARG1);
    GL_LOG ("glTessellationFactorAMD GLfloat factor=%f",fARG1);
    }
}
void glcommon_glTessellationModeAMD(CPU* cpu) {
    if (!ext_glTessellationModeAMD)
        kpanic("ext_glTessellationModeAMD is NULL");
    {
    GL_FUNC(ext_glTessellationModeAMD)(ARG1);
    GL_LOG ("glTessellationModeAMD GLenum mode=%d",ARG1);
    }
}
void glcommon_glTestFenceAPPLE(CPU* cpu) {
    if (!ext_glTestFenceAPPLE)
        kpanic("ext_glTestFenceAPPLE is NULL");
    {
    EAX=GL_FUNC(ext_glTestFenceAPPLE)(ARG1);
    GL_LOG ("glTestFenceAPPLE GLuint fence=%d",ARG1);
    }
}
void glcommon_glTestFenceNV(CPU* cpu) {
    if (!ext_glTestFenceNV)
        kpanic("ext_glTestFenceNV is NULL");
    {
    EAX=GL_FUNC(ext_glTestFenceNV)(ARG1);
    GL_LOG ("glTestFenceNV GLuint fence=%d",ARG1);
    }
}
void glcommon_glTestObjectAPPLE(CPU* cpu) {
    if (!ext_glTestObjectAPPLE)
        kpanic("ext_glTestObjectAPPLE is NULL");
    {
    EAX=GL_FUNC(ext_glTestObjectAPPLE)(ARG1, ARG2);
    GL_LOG ("glTestObjectAPPLE GLenum object=%d, GLuint name=%d",ARG1,ARG2);
    }
}
void glcommon_glTexBuffer(CPU* cpu) {
    if (!ext_glTexBuffer)
        kpanic("ext_glTexBuffer is NULL");
    {
    GL_FUNC(ext_glTexBuffer)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexBuffer GLenum target=%d, GLenum internalformat=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexBufferARB(CPU* cpu) {
    if (!ext_glTexBufferARB)
        kpanic("ext_glTexBufferARB is NULL");
    {
    GL_FUNC(ext_glTexBufferARB)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexBufferARB GLenum target=%d, GLenum internalformat=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexBufferEXT(CPU* cpu) {
    if (!ext_glTexBufferEXT)
        kpanic("ext_glTexBufferEXT is NULL");
    {
    GL_FUNC(ext_glTexBufferEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexBufferEXT GLenum target=%d, GLenum internalformat=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexBufferRange(CPU* cpu) {
    if (!ext_glTexBufferRange)
        kpanic("ext_glTexBufferRange is NULL");
    {
    GL_FUNC(ext_glTexBufferRange)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glTexBufferRange GLenum target=%d, GLenum internalformat=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTexBumpParameterfvATI(CPU* cpu) {
    if (!ext_glTexBumpParameterfvATI)
        kpanic("ext_glTexBumpParameterfvATI is NULL");
    {
    GL_FUNC(ext_glTexBumpParameterfvATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexBumpParameterfvATI GLenum pname=%d, const GLfloat* param=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexBumpParameterivATI(CPU* cpu) {
    if (!ext_glTexBumpParameterivATI)
        kpanic("ext_glTexBumpParameterivATI is NULL");
    {
    GL_FUNC(ext_glTexBumpParameterivATI)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexBumpParameterivATI GLenum pname=%d, const GLint* param=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexCoord1bOES(CPU* cpu) {
    if (!ext_glTexCoord1bOES)
        kpanic("ext_glTexCoord1bOES is NULL");
    {
    GL_FUNC(ext_glTexCoord1bOES)(bARG1);
    GL_LOG ("glTexCoord1bOES GLbyte s=%d",bARG1);
    }
}
void glcommon_glTexCoord1bvOES(CPU* cpu) {
    if (!ext_glTexCoord1bvOES)
        kpanic("ext_glTexCoord1bvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord1bvOES)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord1bvOES const GLbyte* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoord1hNV(CPU* cpu) {
    if (!ext_glTexCoord1hNV)
        kpanic("ext_glTexCoord1hNV is NULL");
    {
    GL_FUNC(ext_glTexCoord1hNV)(sARG1);
    GL_LOG ("glTexCoord1hNV GLhalfNV s=%d",sARG1);
    }
}
void glcommon_glTexCoord1hvNV(CPU* cpu) {
    if (!ext_glTexCoord1hvNV)
        kpanic("ext_glTexCoord1hvNV is NULL");
    {
    GL_FUNC(ext_glTexCoord1hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord1hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glTexCoord1xOES(CPU* cpu) {
    if (!ext_glTexCoord1xOES)
        kpanic("ext_glTexCoord1xOES is NULL");
    {
    GL_FUNC(ext_glTexCoord1xOES)(ARG1);
    GL_LOG ("glTexCoord1xOES GLfixed s=%d",ARG1);
    }
}
void glcommon_glTexCoord1xvOES(CPU* cpu) {
    if (!ext_glTexCoord1xvOES)
        kpanic("ext_glTexCoord1xvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord1xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord1xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoord2bOES(CPU* cpu) {
    if (!ext_glTexCoord2bOES)
        kpanic("ext_glTexCoord2bOES is NULL");
    {
    GL_FUNC(ext_glTexCoord2bOES)(bARG1, bARG2);
    GL_LOG ("glTexCoord2bOES GLbyte s=%d, GLbyte t=%d",bARG1,bARG2);
    }
}
void glcommon_glTexCoord2bvOES(CPU* cpu) {
    if (!ext_glTexCoord2bvOES)
        kpanic("ext_glTexCoord2bvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord2bvOES)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord2bvOES const GLbyte* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoord2fColor3fVertex3fSUN(CPU* cpu) {
    if (!ext_glTexCoord2fColor3fVertex3fSUN)
        kpanic("ext_glTexCoord2fColor3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fColor3fVertex3fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8);
    GL_LOG ("glTexCoord2fColor3fVertex3fSUN GLfloat s=%f, GLfloat t=%f, GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8);
    }
}
void glcommon_glTexCoord2fColor3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glTexCoord2fColor3fVertex3fvSUN)
        kpanic("ext_glTexCoord2fColor3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fColor3fVertex3fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexCoord2fColor3fVertex3fvSUN const GLfloat* tc=%.08x, const GLfloat* c=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexCoord2fColor4fNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glTexCoord2fColor4fNormal3fVertex3fSUN)
        kpanic("ext_glTexCoord2fColor4fNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fColor4fNormal3fVertex3fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8, fARG9, fARG10, fARG11, fARG12);
    GL_LOG ("glTexCoord2fColor4fNormal3fVertex3fSUN GLfloat s=%f, GLfloat t=%f, GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat a=%f, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8,fARG9,fARG10,fARG11,fARG12);
    }
}
void glcommon_glTexCoord2fColor4fNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glTexCoord2fColor4fNormal3fVertex3fvSUN)
        kpanic("ext_glTexCoord2fColor4fNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fColor4fNormal3fVertex3fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTexCoord2fColor4fNormal3fVertex3fvSUN const GLfloat* tc=%.08x, const GLfloat* c=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTexCoord2fColor4ubVertex3fSUN(CPU* cpu) {
    if (!ext_glTexCoord2fColor4ubVertex3fSUN)
        kpanic("ext_glTexCoord2fColor4ubVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fColor4ubVertex3fSUN)(fARG1, fARG2, bARG3, bARG4, bARG5, bARG6, fARG7, fARG8, fARG9);
    GL_LOG ("glTexCoord2fColor4ubVertex3fSUN GLfloat s=%f, GLfloat t=%f, GLubyte r=%d, GLubyte g=%d, GLubyte b=%d, GLubyte a=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,bARG3,bARG4,bARG5,bARG6,fARG7,fARG8,fARG9);
    }
}
void glcommon_glTexCoord2fColor4ubVertex3fvSUN(CPU* cpu) {
    if (!ext_glTexCoord2fColor4ubVertex3fvSUN)
        kpanic("ext_glTexCoord2fColor4ubVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fColor4ubVertex3fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLubyte*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexCoord2fColor4ubVertex3fvSUN const GLfloat* tc=%.08x, const GLubyte* c=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexCoord2fNormal3fVertex3fSUN(CPU* cpu) {
    if (!ext_glTexCoord2fNormal3fVertex3fSUN)
        kpanic("ext_glTexCoord2fNormal3fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fNormal3fVertex3fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8);
    GL_LOG ("glTexCoord2fNormal3fVertex3fSUN GLfloat s=%f, GLfloat t=%f, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8);
    }
}
void glcommon_glTexCoord2fNormal3fVertex3fvSUN(CPU* cpu) {
    if (!ext_glTexCoord2fNormal3fVertex3fvSUN)
        kpanic("ext_glTexCoord2fNormal3fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fNormal3fVertex3fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexCoord2fNormal3fVertex3fvSUN const GLfloat* tc=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexCoord2fVertex3fSUN(CPU* cpu) {
    if (!ext_glTexCoord2fVertex3fSUN)
        kpanic("ext_glTexCoord2fVertex3fSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fVertex3fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glTexCoord2fVertex3fSUN GLfloat s=%f, GLfloat t=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glTexCoord2fVertex3fvSUN(CPU* cpu) {
    if (!ext_glTexCoord2fVertex3fvSUN)
        kpanic("ext_glTexCoord2fVertex3fvSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord2fVertex3fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexCoord2fVertex3fvSUN const GLfloat* tc=%.08x, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexCoord2hNV(CPU* cpu) {
    if (!ext_glTexCoord2hNV)
        kpanic("ext_glTexCoord2hNV is NULL");
    {
    GL_FUNC(ext_glTexCoord2hNV)(sARG1, sARG2);
    GL_LOG ("glTexCoord2hNV GLhalfNV s=%d, GLhalfNV t=%d",sARG1,sARG2);
    }
}
void glcommon_glTexCoord2hvNV(CPU* cpu) {
    if (!ext_glTexCoord2hvNV)
        kpanic("ext_glTexCoord2hvNV is NULL");
    {
    GL_FUNC(ext_glTexCoord2hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord2hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glTexCoord2xOES(CPU* cpu) {
    if (!ext_glTexCoord2xOES)
        kpanic("ext_glTexCoord2xOES is NULL");
    {
    GL_FUNC(ext_glTexCoord2xOES)(ARG1, ARG2);
    GL_LOG ("glTexCoord2xOES GLfixed s=%d, GLfixed t=%d",ARG1,ARG2);
    }
}
void glcommon_glTexCoord2xvOES(CPU* cpu) {
    if (!ext_glTexCoord2xvOES)
        kpanic("ext_glTexCoord2xvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord2xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord2xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoord3bOES(CPU* cpu) {
    if (!ext_glTexCoord3bOES)
        kpanic("ext_glTexCoord3bOES is NULL");
    {
    GL_FUNC(ext_glTexCoord3bOES)(bARG1, bARG2, bARG3);
    GL_LOG ("glTexCoord3bOES GLbyte s=%d, GLbyte t=%d, GLbyte r=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glTexCoord3bvOES(CPU* cpu) {
    if (!ext_glTexCoord3bvOES)
        kpanic("ext_glTexCoord3bvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord3bvOES)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord3bvOES const GLbyte* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoord3hNV(CPU* cpu) {
    if (!ext_glTexCoord3hNV)
        kpanic("ext_glTexCoord3hNV is NULL");
    {
    GL_FUNC(ext_glTexCoord3hNV)(sARG1, sARG2, sARG3);
    GL_LOG ("glTexCoord3hNV GLhalfNV s=%d, GLhalfNV t=%d, GLhalfNV r=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glTexCoord3hvNV(CPU* cpu) {
    if (!ext_glTexCoord3hvNV)
        kpanic("ext_glTexCoord3hvNV is NULL");
    {
    GL_FUNC(ext_glTexCoord3hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord3hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glTexCoord3xOES(CPU* cpu) {
    if (!ext_glTexCoord3xOES)
        kpanic("ext_glTexCoord3xOES is NULL");
    {
    GL_FUNC(ext_glTexCoord3xOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexCoord3xOES GLfixed s=%d, GLfixed t=%d, GLfixed r=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexCoord3xvOES(CPU* cpu) {
    if (!ext_glTexCoord3xvOES)
        kpanic("ext_glTexCoord3xvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord3xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord3xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoord4bOES(CPU* cpu) {
    if (!ext_glTexCoord4bOES)
        kpanic("ext_glTexCoord4bOES is NULL");
    {
    GL_FUNC(ext_glTexCoord4bOES)(bARG1, bARG2, bARG3, bARG4);
    GL_LOG ("glTexCoord4bOES GLbyte s=%d, GLbyte t=%d, GLbyte r=%d, GLbyte q=%d",bARG1,bARG2,bARG3,bARG4);
    }
}
void glcommon_glTexCoord4bvOES(CPU* cpu) {
    if (!ext_glTexCoord4bvOES)
        kpanic("ext_glTexCoord4bvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord4bvOES)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord4bvOES const GLbyte* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoord4fColor4fNormal3fVertex4fSUN(CPU* cpu) {
    if (!ext_glTexCoord4fColor4fNormal3fVertex4fSUN)
        kpanic("ext_glTexCoord4fColor4fNormal3fVertex4fSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord4fColor4fNormal3fVertex4fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8, fARG9, fARG10, fARG11, fARG12, fARG13, fARG14, fARG15);
    GL_LOG ("glTexCoord4fColor4fNormal3fVertex4fSUN GLfloat s=%f, GLfloat t=%f, GLfloat p=%f, GLfloat q=%f, GLfloat r=%f, GLfloat g=%f, GLfloat b=%f, GLfloat a=%f, GLfloat nx=%f, GLfloat ny=%f, GLfloat nz=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8,fARG9,fARG10,fARG11,fARG12,fARG13,fARG14,fARG15);
    }
}
void glcommon_glTexCoord4fColor4fNormal3fVertex4fvSUN(CPU* cpu) {
    if (!ext_glTexCoord4fColor4fNormal3fVertex4fvSUN)
        kpanic("ext_glTexCoord4fColor4fNormal3fVertex4fvSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord4fColor4fNormal3fVertex4fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0), (GLfloat*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTexCoord4fColor4fNormal3fVertex4fvSUN const GLfloat* tc=%.08x, const GLfloat* c=%.08x, const GLfloat* n=%.08x, const GLfloat* v=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTexCoord4fVertex4fSUN(CPU* cpu) {
    if (!ext_glTexCoord4fVertex4fSUN)
        kpanic("ext_glTexCoord4fVertex4fSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord4fVertex4fSUN)(fARG1, fARG2, fARG3, fARG4, fARG5, fARG6, fARG7, fARG8);
    GL_LOG ("glTexCoord4fVertex4fSUN GLfloat s=%f, GLfloat t=%f, GLfloat p=%f, GLfloat q=%f, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",fARG1,fARG2,fARG3,fARG4,fARG5,fARG6,fARG7,fARG8);
    }
}
void glcommon_glTexCoord4fVertex4fvSUN(CPU* cpu) {
    if (!ext_glTexCoord4fVertex4fvSUN)
        kpanic("ext_glTexCoord4fVertex4fvSUN is NULL");
    {
    GL_FUNC(ext_glTexCoord4fVertex4fvSUN)((GLfloat*)marshalp(cpu, 0, ARG1, 0), (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexCoord4fVertex4fvSUN const GLfloat* tc=%.08x, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexCoord4hNV(CPU* cpu) {
    if (!ext_glTexCoord4hNV)
        kpanic("ext_glTexCoord4hNV is NULL");
    {
    GL_FUNC(ext_glTexCoord4hNV)(sARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glTexCoord4hNV GLhalfNV s=%d, GLhalfNV t=%d, GLhalfNV r=%d, GLhalfNV q=%d",sARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glTexCoord4hvNV(CPU* cpu) {
    if (!ext_glTexCoord4hvNV)
        kpanic("ext_glTexCoord4hvNV is NULL");
    {
    GL_FUNC(ext_glTexCoord4hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord4hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glTexCoord4xOES(CPU* cpu) {
    if (!ext_glTexCoord4xOES)
        kpanic("ext_glTexCoord4xOES is NULL");
    {
    GL_FUNC(ext_glTexCoord4xOES)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glTexCoord4xOES GLfixed s=%d, GLfixed t=%d, GLfixed r=%d, GLfixed q=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTexCoord4xvOES(CPU* cpu) {
    if (!ext_glTexCoord4xvOES)
        kpanic("ext_glTexCoord4xvOES is NULL");
    {
    GL_FUNC(ext_glTexCoord4xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glTexCoord4xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glTexCoordFormatNV(CPU* cpu) {
    if (!ext_glTexCoordFormatNV)
        kpanic("ext_glTexCoordFormatNV is NULL");
    {
    GL_FUNC(ext_glTexCoordFormatNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexCoordFormatNV GLint size=%d, GLenum type=%d, GLsizei stride=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexCoordP1ui(CPU* cpu) {
    if (!ext_glTexCoordP1ui)
        kpanic("ext_glTexCoordP1ui is NULL");
    {
    GL_FUNC(ext_glTexCoordP1ui)(ARG1, ARG2);
    GL_LOG ("glTexCoordP1ui GLenum type=%d, GLuint coords=%d",ARG1,ARG2);
    }
}
void glcommon_glTexCoordP1uiv(CPU* cpu) {
    if (!ext_glTexCoordP1uiv)
        kpanic("ext_glTexCoordP1uiv is NULL");
    {
    GL_FUNC(ext_glTexCoordP1uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexCoordP1uiv GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexCoordP2ui(CPU* cpu) {
    if (!ext_glTexCoordP2ui)
        kpanic("ext_glTexCoordP2ui is NULL");
    {
    GL_FUNC(ext_glTexCoordP2ui)(ARG1, ARG2);
    GL_LOG ("glTexCoordP2ui GLenum type=%d, GLuint coords=%d",ARG1,ARG2);
    }
}
void glcommon_glTexCoordP2uiv(CPU* cpu) {
    if (!ext_glTexCoordP2uiv)
        kpanic("ext_glTexCoordP2uiv is NULL");
    {
    GL_FUNC(ext_glTexCoordP2uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexCoordP2uiv GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexCoordP3ui(CPU* cpu) {
    if (!ext_glTexCoordP3ui)
        kpanic("ext_glTexCoordP3ui is NULL");
    {
    GL_FUNC(ext_glTexCoordP3ui)(ARG1, ARG2);
    GL_LOG ("glTexCoordP3ui GLenum type=%d, GLuint coords=%d",ARG1,ARG2);
    }
}
void glcommon_glTexCoordP3uiv(CPU* cpu) {
    if (!ext_glTexCoordP3uiv)
        kpanic("ext_glTexCoordP3uiv is NULL");
    {
    GL_FUNC(ext_glTexCoordP3uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexCoordP3uiv GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexCoordP4ui(CPU* cpu) {
    if (!ext_glTexCoordP4ui)
        kpanic("ext_glTexCoordP4ui is NULL");
    {
    GL_FUNC(ext_glTexCoordP4ui)(ARG1, ARG2);
    GL_LOG ("glTexCoordP4ui GLenum type=%d, GLuint coords=%d",ARG1,ARG2);
    }
}
void glcommon_glTexCoordP4uiv(CPU* cpu) {
    if (!ext_glTexCoordP4uiv)
        kpanic("ext_glTexCoordP4uiv is NULL");
    {
    GL_FUNC(ext_glTexCoordP4uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glTexCoordP4uiv GLenum type=%d, const GLuint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glTexCoordPointerEXT(CPU* cpu) {
    if (!ext_glTexCoordPointerEXT)
        kpanic("ext_glTexCoordPointerEXT is NULL");
    {
    GL_FUNC(ext_glTexCoordPointerEXT)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glTexCoordPointerEXT GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLsizei count=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTexCoordPointerListIBM(CPU* cpu) {
    if (!ext_glTexCoordPointerListIBM)
        kpanic("ext_glTexCoordPointerListIBM is NULL");
    {
    GL_FUNC(ext_glTexCoordPointerListIBM)(ARG1, ARG2, ARG3, (const void**)marshalunhandled("glTexCoordPointerListIBM", "pointer", cpu, ARG4), ARG5);
    GL_LOG ("glTexCoordPointerListIBM GLint size=%d, GLenum type=%d, GLint stride=%d, const void** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTexCoordPointervINTEL(CPU* cpu) {
    if (!ext_glTexCoordPointervINTEL)
        kpanic("ext_glTexCoordPointervINTEL is NULL");
    {
    GL_FUNC(ext_glTexCoordPointervINTEL)(ARG1, ARG2, (const void**)marshalunhandled("glTexCoordPointervINTEL", "pointer", cpu, ARG3));
    GL_LOG ("glTexCoordPointervINTEL GLint size=%d, GLenum type=%d, const void** pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexEnvxOES(CPU* cpu) {
    if (!ext_glTexEnvxOES)
        kpanic("ext_glTexEnvxOES is NULL");
    {
    GL_FUNC(ext_glTexEnvxOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexEnvxOES GLenum target=%d, GLenum pname=%d, GLfixed param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexEnvxvOES(CPU* cpu) {
    if (!ext_glTexEnvxvOES)
        kpanic("ext_glTexEnvxvOES is NULL");
    {
    GL_FUNC(ext_glTexEnvxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexEnvxvOES GLenum target=%d, GLenum pname=%d, const GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexFilterFuncSGIS(CPU* cpu) {
    if (!ext_glTexFilterFuncSGIS)
        kpanic("ext_glTexFilterFuncSGIS is NULL");
    {
    GL_FUNC(ext_glTexFilterFuncSGIS)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTexFilterFuncSGIS GLenum target=%d, GLenum filter=%d, GLsizei n=%d, const GLfloat* weights=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTexGenxOES(CPU* cpu) {
    if (!ext_glTexGenxOES)
        kpanic("ext_glTexGenxOES is NULL");
    {
    GL_FUNC(ext_glTexGenxOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexGenxOES GLenum coord=%d, GLenum pname=%d, GLfixed param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexGenxvOES(CPU* cpu) {
    if (!ext_glTexGenxvOES)
        kpanic("ext_glTexGenxvOES is NULL");
    {
    GL_FUNC(ext_glTexGenxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexGenxvOES GLenum coord=%d, GLenum pname=%d, const GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexImage2DMultisample(CPU* cpu) {
    if (!ext_glTexImage2DMultisample)
        kpanic("ext_glTexImage2DMultisample is NULL");
    {
    GL_FUNC(ext_glTexImage2DMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5, bARG6);
    GL_LOG ("glTexImage2DMultisample GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,bARG6);
    }
}
void glcommon_glTexImage2DMultisampleCoverageNV(CPU* cpu) {
    if (!ext_glTexImage2DMultisampleCoverageNV)
        kpanic("ext_glTexImage2DMultisampleCoverageNV is NULL");
    {
    GL_FUNC(ext_glTexImage2DMultisampleCoverageNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, bARG7);
    GL_LOG ("glTexImage2DMultisampleCoverageNV GLenum target=%d, GLsizei coverageSamples=%d, GLsizei colorSamples=%d, GLint internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLboolean fixedSampleLocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,bARG7);
    }
}
void glcommon_glTexImage3D(CPU* cpu) {
    if (!ext_glTexImage3D)
        kpanic("ext_glTexImage3D is NULL");
    {
    GL_FUNC(ext_glTexImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, PIXEL_UNPACK_BUFFER()?(GLvoid*)pARG10:marshalPixels(cpu, 1, ARG4, ARG5, ARG6, ARG8, ARG9, ARG10));
    GL_LOG ("glTexImage3D GLenum target=%d, GLint level=%d, GLint internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glTexImage3DEXT(CPU* cpu) {
    if (!ext_glTexImage3DEXT)
        kpanic("ext_glTexImage3DEXT is NULL");
    {
    GL_FUNC(ext_glTexImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, (void*)marshalp(cpu, 0, ARG10, 0));
    GL_LOG ("glTexImage3DEXT GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glTexImage3DMultisample(CPU* cpu) {
    if (!ext_glTexImage3DMultisample)
        kpanic("ext_glTexImage3DMultisample is NULL");
    {
    GL_FUNC(ext_glTexImage3DMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, bARG7);
    GL_LOG ("glTexImage3DMultisample GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,bARG7);
    }
}
void glcommon_glTexImage3DMultisampleCoverageNV(CPU* cpu) {
    if (!ext_glTexImage3DMultisampleCoverageNV)
        kpanic("ext_glTexImage3DMultisampleCoverageNV is NULL");
    {
    GL_FUNC(ext_glTexImage3DMultisampleCoverageNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, bARG8);
    GL_LOG ("glTexImage3DMultisampleCoverageNV GLenum target=%d, GLsizei coverageSamples=%d, GLsizei colorSamples=%d, GLint internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean fixedSampleLocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,bARG8);
    }
}
void glcommon_glTexImage4DSGIS(CPU* cpu) {
    if (!ext_glTexImage4DSGIS)
        kpanic("ext_glTexImage4DSGIS is NULL");
    {
    GL_FUNC(ext_glTexImage4DSGIS)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, (void*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glTexImage4DSGIS GLenum target=%d, GLint level=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLsizei size4d=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glTexPageCommitmentARB(CPU* cpu) {
    if (!ext_glTexPageCommitmentARB)
        kpanic("ext_glTexPageCommitmentARB is NULL");
    {
    GL_FUNC(ext_glTexPageCommitmentARB)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, bARG9);
    GL_LOG ("glTexPageCommitmentARB GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean commit=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,bARG9);
    }
}
void glcommon_glTexParameterIiv(CPU* cpu) {
    if (!ext_glTexParameterIiv)
        kpanic("ext_glTexParameterIiv is NULL");
    {
    GL_FUNC(ext_glTexParameterIiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexParameterIiv GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexParameterIivEXT(CPU* cpu) {
    if (!ext_glTexParameterIivEXT)
        kpanic("ext_glTexParameterIivEXT is NULL");
    {
    GL_FUNC(ext_glTexParameterIivEXT)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexParameterIivEXT GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexParameterIuiv(CPU* cpu) {
    if (!ext_glTexParameterIuiv)
        kpanic("ext_glTexParameterIuiv is NULL");
    {
    GL_FUNC(ext_glTexParameterIuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexParameterIuiv GLenum target=%d, GLenum pname=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexParameterIuivEXT(CPU* cpu) {
    if (!ext_glTexParameterIuivEXT)
        kpanic("ext_glTexParameterIuivEXT is NULL");
    {
    GL_FUNC(ext_glTexParameterIuivEXT)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexParameterIuivEXT GLenum target=%d, GLenum pname=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexParameterxOES(CPU* cpu) {
    if (!ext_glTexParameterxOES)
        kpanic("ext_glTexParameterxOES is NULL");
    {
    GL_FUNC(ext_glTexParameterxOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glTexParameterxOES GLenum target=%d, GLenum pname=%d, GLfixed param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexParameterxvOES(CPU* cpu) {
    if (!ext_glTexParameterxvOES)
        kpanic("ext_glTexParameterxvOES is NULL");
    {
    GL_FUNC(ext_glTexParameterxvOES)(ARG1, ARG2, (GLfixed*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTexParameterxvOES GLenum target=%d, GLenum pname=%d, const GLfixed* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTexRenderbufferNV(CPU* cpu) {
    if (!ext_glTexRenderbufferNV)
        kpanic("ext_glTexRenderbufferNV is NULL");
    {
    GL_FUNC(ext_glTexRenderbufferNV)(ARG1, ARG2);
    GL_LOG ("glTexRenderbufferNV GLenum target=%d, GLuint renderbuffer=%d",ARG1,ARG2);
    }
}
void glcommon_glTexStorage1D(CPU* cpu) {
    if (!ext_glTexStorage1D)
        kpanic("ext_glTexStorage1D is NULL");
    {
    GL_FUNC(ext_glTexStorage1D)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glTexStorage1D GLenum target=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTexStorage2D(CPU* cpu) {
    if (!ext_glTexStorage2D)
        kpanic("ext_glTexStorage2D is NULL");
    {
    GL_FUNC(ext_glTexStorage2D)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glTexStorage2D GLenum target=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTexStorage2DMultisample(CPU* cpu) {
    if (!ext_glTexStorage2DMultisample)
        kpanic("ext_glTexStorage2DMultisample is NULL");
    {
    GL_FUNC(ext_glTexStorage2DMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5, bARG6);
    GL_LOG ("glTexStorage2DMultisample GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,bARG6);
    }
}
void glcommon_glTexStorage3D(CPU* cpu) {
    if (!ext_glTexStorage3D)
        kpanic("ext_glTexStorage3D is NULL");
    {
    GL_FUNC(ext_glTexStorage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glTexStorage3D GLenum target=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glTexStorage3DMultisample(CPU* cpu) {
    if (!ext_glTexStorage3DMultisample)
        kpanic("ext_glTexStorage3DMultisample is NULL");
    {
    GL_FUNC(ext_glTexStorage3DMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, bARG7);
    GL_LOG ("glTexStorage3DMultisample GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,bARG7);
    }
}
void glcommon_glTexStorageSparseAMD(CPU* cpu) {
    if (!ext_glTexStorageSparseAMD)
        kpanic("ext_glTexStorageSparseAMD is NULL");
    {
    GL_FUNC(ext_glTexStorageSparseAMD)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glTexStorageSparseAMD GLenum target=%d, GLenum internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLsizei layers=%d, GLbitfield flags=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glTexSubImage1DEXT(CPU* cpu) {
    if (!ext_glTexSubImage1DEXT)
        kpanic("ext_glTexSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glTexSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, (void*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glTexSubImage1DEXT GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glTexSubImage2DEXT(CPU* cpu) {
    if (!ext_glTexSubImage2DEXT)
        kpanic("ext_glTexSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glTexSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, (void*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glTexSubImage2DEXT GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glTexSubImage3D(CPU* cpu) {
    if (!ext_glTexSubImage3D)
        kpanic("ext_glTexSubImage3D is NULL");
    {
    GL_FUNC(ext_glTexSubImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, (void*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glTexSubImage3D GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glTexSubImage3DEXT(CPU* cpu) {
    if (!ext_glTexSubImage3DEXT)
        kpanic("ext_glTexSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glTexSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, (void*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glTexSubImage3DEXT GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glTexSubImage4DSGIS(CPU* cpu) {
    if (!ext_glTexSubImage4DSGIS)
        kpanic("ext_glTexSubImage4DSGIS is NULL");
    {
    GL_FUNC(ext_glTexSubImage4DSGIS)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, (void*)marshalp(cpu, 0, ARG13, 0));
    GL_LOG ("glTexSubImage4DSGIS GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLint woffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLsizei size4d=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12,ARG13);
    }
}
void glcommon_glTextureBarrier(CPU* cpu) {
    if (!ext_glTextureBarrier)
        kpanic("ext_glTextureBarrier is NULL");
    {
    GL_FUNC(ext_glTextureBarrier)();
    GL_LOG ("glTextureBarrier");
    }
}
void glcommon_glTextureBarrierNV(CPU* cpu) {
    if (!ext_glTextureBarrierNV)
        kpanic("ext_glTextureBarrierNV is NULL");
    {
    GL_FUNC(ext_glTextureBarrierNV)();
    GL_LOG ("glTextureBarrierNV");
    }
}
void glcommon_glTextureBuffer(CPU* cpu) {
    if (!ext_glTextureBuffer)
        kpanic("ext_glTextureBuffer is NULL");
    {
    GL_FUNC(ext_glTextureBuffer)(ARG1, ARG2, ARG3);
    GL_LOG ("glTextureBuffer GLuint texture=%d, GLenum internalformat=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureBufferEXT(CPU* cpu) {
    if (!ext_glTextureBufferEXT)
        kpanic("ext_glTextureBufferEXT is NULL");
    {
    GL_FUNC(ext_glTextureBufferEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glTextureBufferEXT GLuint texture=%d, GLenum target=%d, GLenum internalformat=%d, GLuint buffer=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTextureBufferRange(CPU* cpu) {
    if (!ext_glTextureBufferRange)
        kpanic("ext_glTextureBufferRange is NULL");
    {
    GL_FUNC(ext_glTextureBufferRange)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glTextureBufferRange GLuint texture=%d, GLenum internalformat=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTextureBufferRangeEXT(CPU* cpu) {
    if (!ext_glTextureBufferRangeEXT)
        kpanic("ext_glTextureBufferRangeEXT is NULL");
    {
    GL_FUNC(ext_glTextureBufferRangeEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glTextureBufferRangeEXT GLuint texture=%d, GLenum target=%d, GLenum internalformat=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glTextureColorMaskSGIS(CPU* cpu) {
    if (!ext_glTextureColorMaskSGIS)
        kpanic("ext_glTextureColorMaskSGIS is NULL");
    {
    GL_FUNC(ext_glTextureColorMaskSGIS)(bARG1, bARG2, bARG3, bARG4);
    GL_LOG ("glTextureColorMaskSGIS GLboolean red=%d, GLboolean green=%d, GLboolean blue=%d, GLboolean alpha=%d",bARG1,bARG2,bARG3,bARG4);
    }
}
void glcommon_glTextureImage1DEXT(CPU* cpu) {
    if (!ext_glTextureImage1DEXT)
        kpanic("ext_glTextureImage1DEXT is NULL");
    {
    GL_FUNC(ext_glTextureImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, (void*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glTextureImage1DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint internalformat=%d, GLsizei width=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glTextureImage2DEXT(CPU* cpu) {
    if (!ext_glTextureImage2DEXT)
        kpanic("ext_glTextureImage2DEXT is NULL");
    {
    GL_FUNC(ext_glTextureImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, (void*)marshalp(cpu, 0, ARG10, 0));
    GL_LOG ("glTextureImage2DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glTextureImage2DMultisampleCoverageNV(CPU* cpu) {
    if (!ext_glTextureImage2DMultisampleCoverageNV)
        kpanic("ext_glTextureImage2DMultisampleCoverageNV is NULL");
    {
    GL_FUNC(ext_glTextureImage2DMultisampleCoverageNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, bARG8);
    GL_LOG ("glTextureImage2DMultisampleCoverageNV GLuint texture=%d, GLenum target=%d, GLsizei coverageSamples=%d, GLsizei colorSamples=%d, GLint internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLboolean fixedSampleLocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,bARG8);
    }
}
void glcommon_glTextureImage2DMultisampleNV(CPU* cpu) {
    if (!ext_glTextureImage2DMultisampleNV)
        kpanic("ext_glTextureImage2DMultisampleNV is NULL");
    {
    GL_FUNC(ext_glTextureImage2DMultisampleNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, bARG7);
    GL_LOG ("glTextureImage2DMultisampleNV GLuint texture=%d, GLenum target=%d, GLsizei samples=%d, GLint internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLboolean fixedSampleLocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,bARG7);
    }
}
void glcommon_glTextureImage3DEXT(CPU* cpu) {
    if (!ext_glTextureImage3DEXT)
        kpanic("ext_glTextureImage3DEXT is NULL");
    {
    GL_FUNC(ext_glTextureImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, (void*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glTextureImage3DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLint border=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glTextureImage3DMultisampleCoverageNV(CPU* cpu) {
    if (!ext_glTextureImage3DMultisampleCoverageNV)
        kpanic("ext_glTextureImage3DMultisampleCoverageNV is NULL");
    {
    GL_FUNC(ext_glTextureImage3DMultisampleCoverageNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, bARG9);
    GL_LOG ("glTextureImage3DMultisampleCoverageNV GLuint texture=%d, GLenum target=%d, GLsizei coverageSamples=%d, GLsizei colorSamples=%d, GLint internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean fixedSampleLocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,bARG9);
    }
}
void glcommon_glTextureImage3DMultisampleNV(CPU* cpu) {
    if (!ext_glTextureImage3DMultisampleNV)
        kpanic("ext_glTextureImage3DMultisampleNV is NULL");
    {
    GL_FUNC(ext_glTextureImage3DMultisampleNV)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, bARG8);
    GL_LOG ("glTextureImage3DMultisampleNV GLuint texture=%d, GLenum target=%d, GLsizei samples=%d, GLint internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean fixedSampleLocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,bARG8);
    }
}
void glcommon_glTextureLightEXT(CPU* cpu) {
    if (!ext_glTextureLightEXT)
        kpanic("ext_glTextureLightEXT is NULL");
    {
    GL_FUNC(ext_glTextureLightEXT)(ARG1);
    GL_LOG ("glTextureLightEXT GLenum pname=%d",ARG1);
    }
}
void glcommon_glTextureMaterialEXT(CPU* cpu) {
    if (!ext_glTextureMaterialEXT)
        kpanic("ext_glTextureMaterialEXT is NULL");
    {
    GL_FUNC(ext_glTextureMaterialEXT)(ARG1, ARG2);
    GL_LOG ("glTextureMaterialEXT GLenum face=%d, GLenum mode=%d",ARG1,ARG2);
    }
}
void glcommon_glTextureNormalEXT(CPU* cpu) {
    if (!ext_glTextureNormalEXT)
        kpanic("ext_glTextureNormalEXT is NULL");
    {
    GL_FUNC(ext_glTextureNormalEXT)(ARG1);
    GL_LOG ("glTextureNormalEXT GLenum mode=%d",ARG1);
    }
}
void glcommon_glTexturePageCommitmentEXT(CPU* cpu) {
    if (!ext_glTexturePageCommitmentEXT)
        kpanic("ext_glTexturePageCommitmentEXT is NULL");
    {
    GL_FUNC(ext_glTexturePageCommitmentEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, bARG9);
    GL_LOG ("glTexturePageCommitmentEXT GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean commit=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,bARG9);
    }
}
void glcommon_glTextureParameterIiv(CPU* cpu) {
    if (!ext_glTextureParameterIiv)
        kpanic("ext_glTextureParameterIiv is NULL");
    {
    GL_FUNC(ext_glTextureParameterIiv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTextureParameterIiv GLuint texture=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureParameterIivEXT(CPU* cpu) {
    if (!ext_glTextureParameterIivEXT)
        kpanic("ext_glTextureParameterIivEXT is NULL");
    {
    GL_FUNC(ext_glTextureParameterIivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTextureParameterIivEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTextureParameterIuiv(CPU* cpu) {
    if (!ext_glTextureParameterIuiv)
        kpanic("ext_glTextureParameterIuiv is NULL");
    {
    GL_FUNC(ext_glTextureParameterIuiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTextureParameterIuiv GLuint texture=%d, GLenum pname=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureParameterIuivEXT(CPU* cpu) {
    if (!ext_glTextureParameterIuivEXT)
        kpanic("ext_glTextureParameterIuivEXT is NULL");
    {
    GL_FUNC(ext_glTextureParameterIuivEXT)(ARG1, ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTextureParameterIuivEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, const GLuint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTextureParameterf(CPU* cpu) {
    if (!ext_glTextureParameterf)
        kpanic("ext_glTextureParameterf is NULL");
    {
    GL_FUNC(ext_glTextureParameterf)(ARG1, ARG2, fARG3);
    GL_LOG ("glTextureParameterf GLuint texture=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,fARG3);
    }
}
void glcommon_glTextureParameterfEXT(CPU* cpu) {
    if (!ext_glTextureParameterfEXT)
        kpanic("ext_glTextureParameterfEXT is NULL");
    {
    GL_FUNC(ext_glTextureParameterfEXT)(ARG1, ARG2, ARG3, fARG4);
    GL_LOG ("glTextureParameterfEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, GLfloat param=%f",ARG1,ARG2,ARG3,fARG4);
    }
}
void glcommon_glTextureParameterfv(CPU* cpu) {
    if (!ext_glTextureParameterfv)
        kpanic("ext_glTextureParameterfv is NULL");
    {
    GL_FUNC(ext_glTextureParameterfv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTextureParameterfv GLuint texture=%d, GLenum pname=%d, const GLfloat* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureParameterfvEXT(CPU* cpu) {
    if (!ext_glTextureParameterfvEXT)
        kpanic("ext_glTextureParameterfvEXT is NULL");
    {
    GL_FUNC(ext_glTextureParameterfvEXT)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTextureParameterfvEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTextureParameteri(CPU* cpu) {
    if (!ext_glTextureParameteri)
        kpanic("ext_glTextureParameteri is NULL");
    {
    GL_FUNC(ext_glTextureParameteri)(ARG1, ARG2, ARG3);
    GL_LOG ("glTextureParameteri GLuint texture=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureParameteriEXT(CPU* cpu) {
    if (!ext_glTextureParameteriEXT)
        kpanic("ext_glTextureParameteriEXT is NULL");
    {
    GL_FUNC(ext_glTextureParameteriEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glTextureParameteriEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTextureParameteriv(CPU* cpu) {
    if (!ext_glTextureParameteriv)
        kpanic("ext_glTextureParameteriv is NULL");
    {
    GL_FUNC(ext_glTextureParameteriv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTextureParameteriv GLuint texture=%d, GLenum pname=%d, const GLint* param=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureParameterivEXT(CPU* cpu) {
    if (!ext_glTextureParameterivEXT)
        kpanic("ext_glTextureParameterivEXT is NULL");
    {
    GL_FUNC(ext_glTextureParameterivEXT)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTextureParameterivEXT GLuint texture=%d, GLenum target=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTextureRangeAPPLE(CPU* cpu) {
    if (!ext_glTextureRangeAPPLE)
        kpanic("ext_glTextureRangeAPPLE is NULL");
    {
    GL_FUNC(ext_glTextureRangeAPPLE)(ARG1, ARG2, (void*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glTextureRangeAPPLE GLenum target=%d, GLsizei length=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureRenderbufferEXT(CPU* cpu) {
    if (!ext_glTextureRenderbufferEXT)
        kpanic("ext_glTextureRenderbufferEXT is NULL");
    {
    GL_FUNC(ext_glTextureRenderbufferEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glTextureRenderbufferEXT GLuint texture=%d, GLenum target=%d, GLuint renderbuffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTextureStorage1D(CPU* cpu) {
    if (!ext_glTextureStorage1D)
        kpanic("ext_glTextureStorage1D is NULL");
    {
    GL_FUNC(ext_glTextureStorage1D)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glTextureStorage1D GLuint texture=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTextureStorage1DEXT(CPU* cpu) {
    if (!ext_glTextureStorage1DEXT)
        kpanic("ext_glTextureStorage1DEXT is NULL");
    {
    GL_FUNC(ext_glTextureStorage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glTextureStorage1DEXT GLuint texture=%d, GLenum target=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTextureStorage2D(CPU* cpu) {
    if (!ext_glTextureStorage2D)
        kpanic("ext_glTextureStorage2D is NULL");
    {
    GL_FUNC(ext_glTextureStorage2D)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glTextureStorage2D GLuint texture=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTextureStorage2DEXT(CPU* cpu) {
    if (!ext_glTextureStorage2DEXT)
        kpanic("ext_glTextureStorage2DEXT is NULL");
    {
    GL_FUNC(ext_glTextureStorage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glTextureStorage2DEXT GLuint texture=%d, GLenum target=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glTextureStorage2DMultisample(CPU* cpu) {
    if (!ext_glTextureStorage2DMultisample)
        kpanic("ext_glTextureStorage2DMultisample is NULL");
    {
    GL_FUNC(ext_glTextureStorage2DMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5, bARG6);
    GL_LOG ("glTextureStorage2DMultisample GLuint texture=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,bARG6);
    }
}
void glcommon_glTextureStorage2DMultisampleEXT(CPU* cpu) {
    if (!ext_glTextureStorage2DMultisampleEXT)
        kpanic("ext_glTextureStorage2DMultisampleEXT is NULL");
    {
    GL_FUNC(ext_glTextureStorage2DMultisampleEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, bARG7);
    GL_LOG ("glTextureStorage2DMultisampleEXT GLuint texture=%d, GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,bARG7);
    }
}
void glcommon_glTextureStorage3D(CPU* cpu) {
    if (!ext_glTextureStorage3D)
        kpanic("ext_glTextureStorage3D is NULL");
    {
    GL_FUNC(ext_glTextureStorage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glTextureStorage3D GLuint texture=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glTextureStorage3DEXT(CPU* cpu) {
    if (!ext_glTextureStorage3DEXT)
        kpanic("ext_glTextureStorage3DEXT is NULL");
    {
    GL_FUNC(ext_glTextureStorage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glTextureStorage3DEXT GLuint texture=%d, GLenum target=%d, GLsizei levels=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glTextureStorage3DMultisample(CPU* cpu) {
    if (!ext_glTextureStorage3DMultisample)
        kpanic("ext_glTextureStorage3DMultisample is NULL");
    {
    GL_FUNC(ext_glTextureStorage3DMultisample)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, bARG7);
    GL_LOG ("glTextureStorage3DMultisample GLuint texture=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,bARG7);
    }
}
void glcommon_glTextureStorage3DMultisampleEXT(CPU* cpu) {
    if (!ext_glTextureStorage3DMultisampleEXT)
        kpanic("ext_glTextureStorage3DMultisampleEXT is NULL");
    {
    GL_FUNC(ext_glTextureStorage3DMultisampleEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, bARG8);
    GL_LOG ("glTextureStorage3DMultisampleEXT GLuint texture=%d, GLenum target=%d, GLsizei samples=%d, GLenum internalformat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLboolean fixedsamplelocations=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,bARG8);
    }
}
void glcommon_glTextureStorageSparseAMD(CPU* cpu) {
    if (!ext_glTextureStorageSparseAMD)
        kpanic("ext_glTextureStorageSparseAMD is NULL");
    {
    GL_FUNC(ext_glTextureStorageSparseAMD)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glTextureStorageSparseAMD GLuint texture=%d, GLenum target=%d, GLenum internalFormat=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLsizei layers=%d, GLbitfield flags=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glTextureSubImage1D(CPU* cpu) {
    if (!ext_glTextureSubImage1D)
        kpanic("ext_glTextureSubImage1D is NULL");
    {
    GL_FUNC(ext_glTextureSubImage1D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, (void*)marshalp(cpu, 0, ARG7, 0));
    GL_LOG ("glTextureSubImage1D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glTextureSubImage1DEXT(CPU* cpu) {
    if (!ext_glTextureSubImage1DEXT)
        kpanic("ext_glTextureSubImage1DEXT is NULL");
    {
    GL_FUNC(ext_glTextureSubImage1DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, (void*)marshalp(cpu, 0, ARG8, 0));
    GL_LOG ("glTextureSubImage1DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLsizei width=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glTextureSubImage2D(CPU* cpu) {
    if (!ext_glTextureSubImage2D)
        kpanic("ext_glTextureSubImage2D is NULL");
    {
    GL_FUNC(ext_glTextureSubImage2D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, (void*)marshalp(cpu, 0, ARG9, 0));
    GL_LOG ("glTextureSubImage2D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9);
    }
}
void glcommon_glTextureSubImage2DEXT(CPU* cpu) {
    if (!ext_glTextureSubImage2DEXT)
        kpanic("ext_glTextureSubImage2DEXT is NULL");
    {
    GL_FUNC(ext_glTextureSubImage2DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, (void*)marshalp(cpu, 0, ARG10, 0));
    GL_LOG ("glTextureSubImage2DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLsizei width=%d, GLsizei height=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10);
    }
}
void glcommon_glTextureSubImage3D(CPU* cpu) {
    if (!ext_glTextureSubImage3D)
        kpanic("ext_glTextureSubImage3D is NULL");
    {
    GL_FUNC(ext_glTextureSubImage3D)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, (void*)marshalp(cpu, 0, ARG11, 0));
    GL_LOG ("glTextureSubImage3D GLuint texture=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11);
    }
}
void glcommon_glTextureSubImage3DEXT(CPU* cpu) {
    if (!ext_glTextureSubImage3DEXT)
        kpanic("ext_glTextureSubImage3DEXT is NULL");
    {
    GL_FUNC(ext_glTextureSubImage3DEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, (void*)marshalp(cpu, 0, ARG12, 0));
    GL_LOG ("glTextureSubImage3DEXT GLuint texture=%d, GLenum target=%d, GLint level=%d, GLint xoffset=%d, GLint yoffset=%d, GLint zoffset=%d, GLsizei width=%d, GLsizei height=%d, GLsizei depth=%d, GLenum format=%d, GLenum type=%d, const void* pixels=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8,ARG9,ARG10,ARG11,ARG12);
    }
}
void glcommon_glTextureView(CPU* cpu) {
    if (!ext_glTextureView)
        kpanic("ext_glTextureView is NULL");
    {
    GL_FUNC(ext_glTextureView)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    GL_LOG ("glTextureView GLuint texture=%d, GLenum target=%d, GLuint origtexture=%d, GLenum internalformat=%d, GLuint minlevel=%d, GLuint numlevels=%d, GLuint minlayer=%d, GLuint numlayers=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,ARG8);
    }
}
void glcommon_glTrackMatrixNV(CPU* cpu) {
    if (!ext_glTrackMatrixNV)
        kpanic("ext_glTrackMatrixNV is NULL");
    {
    GL_FUNC(ext_glTrackMatrixNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glTrackMatrixNV GLenum target=%d, GLuint address=%d, GLenum matrix=%d, GLenum transform=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTransformFeedbackAttribsNV(CPU* cpu) {
    if (!ext_glTransformFeedbackAttribsNV)
        kpanic("ext_glTransformFeedbackAttribsNV is NULL");
    {
    GL_FUNC(ext_glTransformFeedbackAttribsNV)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0), ARG3);
    GL_LOG ("glTransformFeedbackAttribsNV GLsizei count=%d, const GLint* attribs=%.08x, GLenum bufferMode=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTransformFeedbackBufferBase(CPU* cpu) {
    if (!ext_glTransformFeedbackBufferBase)
        kpanic("ext_glTransformFeedbackBufferBase is NULL");
    {
    GL_FUNC(ext_glTransformFeedbackBufferBase)(ARG1, ARG2, ARG3);
    GL_LOG ("glTransformFeedbackBufferBase GLuint xfb=%d, GLuint index=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glTransformFeedbackBufferRange(CPU* cpu) {
    if (!ext_glTransformFeedbackBufferRange)
        kpanic("ext_glTransformFeedbackBufferRange is NULL");
    {
    GL_FUNC(ext_glTransformFeedbackBufferRange)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glTransformFeedbackBufferRange GLuint xfb=%d, GLuint index=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizeiptr size=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTransformFeedbackStreamAttribsNV(CPU* cpu) {
    if (!ext_glTransformFeedbackStreamAttribsNV)
        kpanic("ext_glTransformFeedbackStreamAttribsNV is NULL");
    {
    GL_FUNC(ext_glTransformFeedbackStreamAttribsNV)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0), ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0), ARG5);
    GL_LOG ("glTransformFeedbackStreamAttribsNV GLsizei count=%d, const GLint* attribs=%.08x, GLsizei nbuffers=%d, const GLint* bufstreams=%.08x, GLenum bufferMode=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glTransformFeedbackVaryings(CPU* cpu) {
    if (!ext_glTransformFeedbackVaryings)
        kpanic("ext_glTransformFeedbackVaryings is NULL");
    {
    GL_FUNC(ext_glTransformFeedbackVaryings)(ARG1, ARG2, (GLchar*const*)marshalpp(cpu, ARG3, ARG2, 0, -1, sizeof(GLchar)), ARG4);
    GL_LOG ("glTransformFeedbackVaryings GLuint program=%d, GLsizei count=%d, const GLchar*const* varyings=%.08x, GLenum bufferMode=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTransformFeedbackVaryingsEXT(CPU* cpu) {
    if (!ext_glTransformFeedbackVaryingsEXT)
        kpanic("ext_glTransformFeedbackVaryingsEXT is NULL");
    {
    GL_FUNC(ext_glTransformFeedbackVaryingsEXT)(ARG1, ARG2, (GLchar*const*)marshalpp(cpu, ARG3, ARG2, 0, -1, sizeof(GLchar)), ARG4);
    GL_LOG ("glTransformFeedbackVaryingsEXT GLuint program=%d, GLsizei count=%d, const GLchar*const* varyings=%.08x, GLenum bufferMode=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTransformFeedbackVaryingsNV(CPU* cpu) {
    if (!ext_glTransformFeedbackVaryingsNV)
        kpanic("ext_glTransformFeedbackVaryingsNV is NULL");
    {
    GL_FUNC(ext_glTransformFeedbackVaryingsNV)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0), ARG4);
    GL_LOG ("glTransformFeedbackVaryingsNV GLuint program=%d, GLsizei count=%d, const GLint* locations=%.08x, GLenum bufferMode=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTransformPathNV(CPU* cpu) {
    if (!ext_glTransformPathNV)
        kpanic("ext_glTransformPathNV is NULL");
    {
    GL_FUNC(ext_glTransformPathNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glTransformPathNV GLuint resultPath=%d, GLuint srcPath=%d, GLenum transformType=%d, const GLfloat* transformValues=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glTranslatexOES(CPU* cpu) {
    if (!ext_glTranslatexOES)
        kpanic("ext_glTranslatexOES is NULL");
    {
    GL_FUNC(ext_glTranslatexOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glTranslatexOES GLfixed x=%d, GLfixed y=%d, GLfixed z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1d(CPU* cpu) {
    if (!ext_glUniform1d)
        kpanic("ext_glUniform1d is NULL");
    {
    GL_FUNC(ext_glUniform1d)(ARG1, dARG2);
    GL_LOG ("glUniform1d GLint location=%d, GLdouble x=%f",ARG1,dARG2);
    }
}
void glcommon_glUniform1dv(CPU* cpu) {
    if (!ext_glUniform1dv)
        kpanic("ext_glUniform1dv is NULL");
    {
    GL_FUNC(ext_glUniform1dv)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1dv GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1f(CPU* cpu) {
    if (!ext_glUniform1f)
        kpanic("ext_glUniform1f is NULL");
    {
    GL_FUNC(ext_glUniform1f)(ARG1, fARG2);
    GL_LOG ("glUniform1f GLint location=%d, GLfloat v0=%f",ARG1,fARG2);
    }
}
void glcommon_glUniform1fARB(CPU* cpu) {
    if (!ext_glUniform1fARB)
        kpanic("ext_glUniform1fARB is NULL");
    {
    GL_FUNC(ext_glUniform1fARB)(ARG1, fARG2);
    GL_LOG ("glUniform1fARB GLint location=%d, GLfloat v0=%f",ARG1,fARG2);
    }
}
void glcommon_glUniform1fv(CPU* cpu) {
    if (!ext_glUniform1fv)
        kpanic("ext_glUniform1fv is NULL");
    {
    GL_FUNC(ext_glUniform1fv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1fv GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1fvARB(CPU* cpu) {
    if (!ext_glUniform1fvARB)
        kpanic("ext_glUniform1fvARB is NULL");
    {
    GL_FUNC(ext_glUniform1fvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1fvARB GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1i(CPU* cpu) {
    if (!ext_glUniform1i)
        kpanic("ext_glUniform1i is NULL");
    {
    GL_FUNC(ext_glUniform1i)(ARG1, ARG2);
    GL_LOG ("glUniform1i GLint location=%d, GLint v0=%d",ARG1,ARG2);
    }
}
void glcommon_glUniform1i64ARB(CPU* cpu) {
    if (!ext_glUniform1i64ARB)
        kpanic("ext_glUniform1i64ARB is NULL");
    {
    GL_FUNC(ext_glUniform1i64ARB)(ARG1, llARG2);
    GL_LOG ("glUniform1i64ARB GLint location=%d, GLint64 x=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glUniform1i64NV(CPU* cpu) {
    if (!ext_glUniform1i64NV)
        kpanic("ext_glUniform1i64NV is NULL");
    {
    GL_FUNC(ext_glUniform1i64NV)(ARG1, llARG2);
    GL_LOG ("glUniform1i64NV GLint location=%d, GLint64EXT x=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glUniform1i64vARB(CPU* cpu) {
    if (!ext_glUniform1i64vARB)
        kpanic("ext_glUniform1i64vARB is NULL");
    {
    GL_FUNC(ext_glUniform1i64vARB)(ARG1, ARG2, (GLint64*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1i64vARB GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1i64vNV(CPU* cpu) {
    if (!ext_glUniform1i64vNV)
        kpanic("ext_glUniform1i64vNV is NULL");
    {
    GL_FUNC(ext_glUniform1i64vNV)(ARG1, ARG2, (GLint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1i64vNV GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1iARB(CPU* cpu) {
    if (!ext_glUniform1iARB)
        kpanic("ext_glUniform1iARB is NULL");
    {
    GL_FUNC(ext_glUniform1iARB)(ARG1, ARG2);
    GL_LOG ("glUniform1iARB GLint location=%d, GLint v0=%d",ARG1,ARG2);
    }
}
void glcommon_glUniform1iv(CPU* cpu) {
    if (!ext_glUniform1iv)
        kpanic("ext_glUniform1iv is NULL");
    {
    GL_FUNC(ext_glUniform1iv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1iv GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1ivARB(CPU* cpu) {
    if (!ext_glUniform1ivARB)
        kpanic("ext_glUniform1ivARB is NULL");
    {
    GL_FUNC(ext_glUniform1ivARB)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1ivARB GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1ui(CPU* cpu) {
    if (!ext_glUniform1ui)
        kpanic("ext_glUniform1ui is NULL");
    {
    GL_FUNC(ext_glUniform1ui)(ARG1, ARG2);
    GL_LOG ("glUniform1ui GLint location=%d, GLuint v0=%d",ARG1,ARG2);
    }
}
void glcommon_glUniform1ui64ARB(CPU* cpu) {
    if (!ext_glUniform1ui64ARB)
        kpanic("ext_glUniform1ui64ARB is NULL");
    {
    GL_FUNC(ext_glUniform1ui64ARB)(ARG1, llARG2);
    GL_LOG ("glUniform1ui64ARB GLint location=%d, GLuint64 x=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glUniform1ui64NV(CPU* cpu) {
    if (!ext_glUniform1ui64NV)
        kpanic("ext_glUniform1ui64NV is NULL");
    {
    GL_FUNC(ext_glUniform1ui64NV)(ARG1, llARG2);
    GL_LOG ("glUniform1ui64NV GLint location=%d, GLuint64EXT x=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glUniform1ui64vARB(CPU* cpu) {
    if (!ext_glUniform1ui64vARB)
        kpanic("ext_glUniform1ui64vARB is NULL");
    {
    GL_FUNC(ext_glUniform1ui64vARB)(ARG1, ARG2, (GLuint64*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1ui64vARB GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1ui64vNV(CPU* cpu) {
    if (!ext_glUniform1ui64vNV)
        kpanic("ext_glUniform1ui64vNV is NULL");
    {
    GL_FUNC(ext_glUniform1ui64vNV)(ARG1, ARG2, (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1ui64vNV GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1uiEXT(CPU* cpu) {
    if (!ext_glUniform1uiEXT)
        kpanic("ext_glUniform1uiEXT is NULL");
    {
    GL_FUNC(ext_glUniform1uiEXT)(ARG1, ARG2);
    GL_LOG ("glUniform1uiEXT GLint location=%d, GLuint v0=%d",ARG1,ARG2);
    }
}
void glcommon_glUniform1uiv(CPU* cpu) {
    if (!ext_glUniform1uiv)
        kpanic("ext_glUniform1uiv is NULL");
    {
    GL_FUNC(ext_glUniform1uiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1uiv GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform1uivEXT(CPU* cpu) {
    if (!ext_glUniform1uivEXT)
        kpanic("ext_glUniform1uivEXT is NULL");
    {
    GL_FUNC(ext_glUniform1uivEXT)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform1uivEXT GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2d(CPU* cpu) {
    if (!ext_glUniform2d)
        kpanic("ext_glUniform2d is NULL");
    {
    GL_FUNC(ext_glUniform2d)(ARG1, dARG2, dARG3);
    GL_LOG ("glUniform2d GLint location=%d, GLdouble x=%f, GLdouble y=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glUniform2dv(CPU* cpu) {
    if (!ext_glUniform2dv)
        kpanic("ext_glUniform2dv is NULL");
    {
    GL_FUNC(ext_glUniform2dv)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2dv GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2f(CPU* cpu) {
    if (!ext_glUniform2f)
        kpanic("ext_glUniform2f is NULL");
    {
    GL_FUNC(ext_glUniform2f)(ARG1, fARG2, fARG3);
    GL_LOG ("glUniform2f GLint location=%d, GLfloat v0=%f, GLfloat v1=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glUniform2fARB(CPU* cpu) {
    if (!ext_glUniform2fARB)
        kpanic("ext_glUniform2fARB is NULL");
    {
    GL_FUNC(ext_glUniform2fARB)(ARG1, fARG2, fARG3);
    GL_LOG ("glUniform2fARB GLint location=%d, GLfloat v0=%f, GLfloat v1=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glUniform2fv(CPU* cpu) {
    if (!ext_glUniform2fv)
        kpanic("ext_glUniform2fv is NULL");
    {
    GL_FUNC(ext_glUniform2fv)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2fv GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2fvARB(CPU* cpu) {
    if (!ext_glUniform2fvARB)
        kpanic("ext_glUniform2fvARB is NULL");
    {
    GL_FUNC(ext_glUniform2fvARB)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2fvARB GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2i(CPU* cpu) {
    if (!ext_glUniform2i)
        kpanic("ext_glUniform2i is NULL");
    {
    GL_FUNC(ext_glUniform2i)(ARG1, ARG2, ARG3);
    GL_LOG ("glUniform2i GLint location=%d, GLint v0=%d, GLint v1=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2i64ARB(CPU* cpu) {
    if (!ext_glUniform2i64ARB)
        kpanic("ext_glUniform2i64ARB is NULL");
    {
    GL_FUNC(ext_glUniform2i64ARB)(ARG1, llARG2, llARG3);
    GL_LOG ("glUniform2i64ARB GLint location=%d, GLint64 x=" PRIu64 ", GLint64 y=" PRIu64 "",ARG1,llARG2,llARG3);
    }
}
void glcommon_glUniform2i64NV(CPU* cpu) {
    if (!ext_glUniform2i64NV)
        kpanic("ext_glUniform2i64NV is NULL");
    {
    GL_FUNC(ext_glUniform2i64NV)(ARG1, llARG2, llARG3);
    GL_LOG ("glUniform2i64NV GLint location=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 "",ARG1,llARG2,llARG3);
    }
}
void glcommon_glUniform2i64vARB(CPU* cpu) {
    if (!ext_glUniform2i64vARB)
        kpanic("ext_glUniform2i64vARB is NULL");
    {
    GL_FUNC(ext_glUniform2i64vARB)(ARG1, ARG2, (GLint64*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2i64vARB GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2i64vNV(CPU* cpu) {
    if (!ext_glUniform2i64vNV)
        kpanic("ext_glUniform2i64vNV is NULL");
    {
    GL_FUNC(ext_glUniform2i64vNV)(ARG1, ARG2, (GLint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2i64vNV GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2iARB(CPU* cpu) {
    if (!ext_glUniform2iARB)
        kpanic("ext_glUniform2iARB is NULL");
    {
    GL_FUNC(ext_glUniform2iARB)(ARG1, ARG2, ARG3);
    GL_LOG ("glUniform2iARB GLint location=%d, GLint v0=%d, GLint v1=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2iv(CPU* cpu) {
    if (!ext_glUniform2iv)
        kpanic("ext_glUniform2iv is NULL");
    {
    GL_FUNC(ext_glUniform2iv)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2iv GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2ivARB(CPU* cpu) {
    if (!ext_glUniform2ivARB)
        kpanic("ext_glUniform2ivARB is NULL");
    {
    GL_FUNC(ext_glUniform2ivARB)(ARG1, ARG2, (GLint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2ivARB GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2ui(CPU* cpu) {
    if (!ext_glUniform2ui)
        kpanic("ext_glUniform2ui is NULL");
    {
    GL_FUNC(ext_glUniform2ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glUniform2ui GLint location=%d, GLuint v0=%d, GLuint v1=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2ui64ARB(CPU* cpu) {
    if (!ext_glUniform2ui64ARB)
        kpanic("ext_glUniform2ui64ARB is NULL");
    {
    GL_FUNC(ext_glUniform2ui64ARB)(ARG1, llARG2, llARG3);
    GL_LOG ("glUniform2ui64ARB GLint location=%d, GLuint64 x=" PRIu64 ", GLuint64 y=" PRIu64 "",ARG1,llARG2,llARG3);
    }
}
void glcommon_glUniform2ui64NV(CPU* cpu) {
    if (!ext_glUniform2ui64NV)
        kpanic("ext_glUniform2ui64NV is NULL");
    {
    GL_FUNC(ext_glUniform2ui64NV)(ARG1, llARG2, llARG3);
    GL_LOG ("glUniform2ui64NV GLint location=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 "",ARG1,llARG2,llARG3);
    }
}
void glcommon_glUniform2ui64vARB(CPU* cpu) {
    if (!ext_glUniform2ui64vARB)
        kpanic("ext_glUniform2ui64vARB is NULL");
    {
    GL_FUNC(ext_glUniform2ui64vARB)(ARG1, ARG2, (GLuint64*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2ui64vARB GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2ui64vNV(CPU* cpu) {
    if (!ext_glUniform2ui64vNV)
        kpanic("ext_glUniform2ui64vNV is NULL");
    {
    GL_FUNC(ext_glUniform2ui64vNV)(ARG1, ARG2, (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2ui64vNV GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2uiEXT(CPU* cpu) {
    if (!ext_glUniform2uiEXT)
        kpanic("ext_glUniform2uiEXT is NULL");
    {
    GL_FUNC(ext_glUniform2uiEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glUniform2uiEXT GLint location=%d, GLuint v0=%d, GLuint v1=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2uiv(CPU* cpu) {
    if (!ext_glUniform2uiv)
        kpanic("ext_glUniform2uiv is NULL");
    {
    GL_FUNC(ext_glUniform2uiv)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2uiv GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform2uivEXT(CPU* cpu) {
    if (!ext_glUniform2uivEXT)
        kpanic("ext_glUniform2uivEXT is NULL");
    {
    GL_FUNC(ext_glUniform2uivEXT)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform2uivEXT GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3d(CPU* cpu) {
    if (!ext_glUniform3d)
        kpanic("ext_glUniform3d is NULL");
    {
    GL_FUNC(ext_glUniform3d)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glUniform3d GLint location=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glUniform3dv(CPU* cpu) {
    if (!ext_glUniform3dv)
        kpanic("ext_glUniform3dv is NULL");
    {
    GL_FUNC(ext_glUniform3dv)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glUniform3dv GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3f(CPU* cpu) {
    if (!ext_glUniform3f)
        kpanic("ext_glUniform3f is NULL");
    {
    GL_FUNC(ext_glUniform3f)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glUniform3f GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glUniform3fARB(CPU* cpu) {
    if (!ext_glUniform3fARB)
        kpanic("ext_glUniform3fARB is NULL");
    {
    GL_FUNC(ext_glUniform3fARB)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glUniform3fARB GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glUniform3fv(CPU* cpu) {
    if (!ext_glUniform3fv)
        kpanic("ext_glUniform3fv is NULL");
    {
        GL_FUNC(ext_glUniform3fv)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, ARG2 * 3));
        GL_LOG ("glUniform3fv GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3fvARB(CPU* cpu) {
    if (!ext_glUniform3fvARB)
        kpanic("ext_glUniform3fvARB is NULL");
    {
    GL_FUNC(ext_glUniform3fvARB)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3fvARB GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3i(CPU* cpu) {
    if (!ext_glUniform3i)
        kpanic("ext_glUniform3i is NULL");
    {
    GL_FUNC(ext_glUniform3i)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glUniform3i GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glUniform3i64ARB(CPU* cpu) {
    if (!ext_glUniform3i64ARB)
        kpanic("ext_glUniform3i64ARB is NULL");
    {
    GL_FUNC(ext_glUniform3i64ARB)(ARG1, llARG2, llARG3, llARG4);
    GL_LOG ("glUniform3i64ARB GLint location=%d, GLint64 x=" PRIu64 ", GLint64 y=" PRIu64 ", GLint64 z=" PRIu64 "",ARG1,llARG2,llARG3,llARG4);
    }
}
void glcommon_glUniform3i64NV(CPU* cpu) {
    if (!ext_glUniform3i64NV)
        kpanic("ext_glUniform3i64NV is NULL");
    {
    GL_FUNC(ext_glUniform3i64NV)(ARG1, llARG2, llARG3, llARG4);
    GL_LOG ("glUniform3i64NV GLint location=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 ", GLint64EXT z=" PRIu64 "",ARG1,llARG2,llARG3,llARG4);
    }
}
void glcommon_glUniform3i64vARB(CPU* cpu) {
    if (!ext_glUniform3i64vARB)
        kpanic("ext_glUniform3i64vARB is NULL");
    {
    GL_FUNC(ext_glUniform3i64vARB)(ARG1, ARG2, marshalArray<GLint64>(cpu, ARG3, ARG2*3));
    GL_LOG ("glUniform3i64vARB GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3i64vNV(CPU* cpu) {
    if (!ext_glUniform3i64vNV)
        kpanic("ext_glUniform3i64vNV is NULL");
    {
    GL_FUNC(ext_glUniform3i64vNV)(ARG1, ARG2, marshalArray<GLint64EXT>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3i64vNV GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3iARB(CPU* cpu) {
    if (!ext_glUniform3iARB)
        kpanic("ext_glUniform3iARB is NULL");
    {
    GL_FUNC(ext_glUniform3iARB)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glUniform3iARB GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glUniform3iv(CPU* cpu) {
    if (!ext_glUniform3iv)
        kpanic("ext_glUniform3iv is NULL");
    {
    GL_FUNC(ext_glUniform3iv)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3iv GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3ivARB(CPU* cpu) {
    if (!ext_glUniform3ivARB)
        kpanic("ext_glUniform3ivARB is NULL");
    {
    GL_FUNC(ext_glUniform3ivARB)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3ivARB GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3ui(CPU* cpu) {
    if (!ext_glUniform3ui)
        kpanic("ext_glUniform3ui is NULL");
    {
    GL_FUNC(ext_glUniform3ui)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glUniform3ui GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glUniform3ui64ARB(CPU* cpu) {
    if (!ext_glUniform3ui64ARB)
        kpanic("ext_glUniform3ui64ARB is NULL");
    {
    GL_FUNC(ext_glUniform3ui64ARB)(ARG1, llARG2, llARG3, llARG4);
    GL_LOG ("glUniform3ui64ARB GLint location=%d, GLuint64 x=" PRIu64 ", GLuint64 y=" PRIu64 ", GLuint64 z=" PRIu64 "",ARG1,llARG2,llARG3,llARG4);
    }
}
void glcommon_glUniform3ui64NV(CPU* cpu) {
    if (!ext_glUniform3ui64NV)
        kpanic("ext_glUniform3ui64NV is NULL");
    {
    GL_FUNC(ext_glUniform3ui64NV)(ARG1, llARG2, llARG3, llARG4);
    GL_LOG ("glUniform3ui64NV GLint location=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 ", GLuint64EXT z=" PRIu64 "",ARG1,llARG2,llARG3,llARG4);
    }
}
void glcommon_glUniform3ui64vARB(CPU* cpu) {
    if (!ext_glUniform3ui64vARB)
        kpanic("ext_glUniform3ui64vARB is NULL");
    {
    GL_FUNC(ext_glUniform3ui64vARB)(ARG1, ARG2, marshalArray<GLuint64>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3ui64vARB GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3ui64vNV(CPU* cpu) {
    if (!ext_glUniform3ui64vNV)
        kpanic("ext_glUniform3ui64vNV is NULL");
    {
    GL_FUNC(ext_glUniform3ui64vNV)(ARG1, ARG2, marshalArray<GLuint64EXT>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3ui64vNV GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3uiEXT(CPU* cpu) {
    if (!ext_glUniform3uiEXT)
        kpanic("ext_glUniform3uiEXT is NULL");
    {
    GL_FUNC(ext_glUniform3uiEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glUniform3uiEXT GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glUniform3uiv(CPU* cpu) {
    if (!ext_glUniform3uiv)
        kpanic("ext_glUniform3uiv is NULL");
    {
    GL_FUNC(ext_glUniform3uiv)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3uiv GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform3uivEXT(CPU* cpu) {
    if (!ext_glUniform3uivEXT)
        kpanic("ext_glUniform3uivEXT is NULL");
    {
    GL_FUNC(ext_glUniform3uivEXT)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2 * 3));
    GL_LOG ("glUniform3uivEXT GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4d(CPU* cpu) {
    if (!ext_glUniform4d)
        kpanic("ext_glUniform4d is NULL");
    {
    GL_FUNC(ext_glUniform4d)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glUniform4d GLint location=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glUniform4dv(CPU* cpu) {
    if (!ext_glUniform4dv)
        kpanic("ext_glUniform4dv is NULL");
    {
    GL_FUNC(ext_glUniform4dv)(ARG1, ARG2, marshalArray<GLdouble>(cpu, ARG3, ARG2*4));
    GL_LOG ("glUniform4dv GLint location=%d, GLsizei count=%d, const GLdouble* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4f(CPU* cpu) {
    if (!ext_glUniform4f)
        kpanic("ext_glUniform4f is NULL");
    {
    GL_FUNC(ext_glUniform4f)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glUniform4f GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f, GLfloat v3=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glUniform4fARB(CPU* cpu) {
    if (!ext_glUniform4fARB)
        kpanic("ext_glUniform4fARB is NULL");
    {
    GL_FUNC(ext_glUniform4fARB)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glUniform4fARB GLint location=%d, GLfloat v0=%f, GLfloat v1=%f, GLfloat v2=%f, GLfloat v3=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glUniform4fv(CPU* cpu) {
    if (!ext_glUniform4fv)
        kpanic("ext_glUniform4fv is NULL");
    {
    GL_FUNC(ext_glUniform4fv)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4fv GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4fvARB(CPU* cpu) {
    if (!ext_glUniform4fvARB)
        kpanic("ext_glUniform4fvARB is NULL");
    {
    GL_FUNC(ext_glUniform4fvARB)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4fvARB GLint location=%d, GLsizei count=%d, const GLfloat* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4i(CPU* cpu) {
    if (!ext_glUniform4i)
        kpanic("ext_glUniform4i is NULL");
    {
    GL_FUNC(ext_glUniform4i)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glUniform4i GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d, GLint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glUniform4i64ARB(CPU* cpu) {
    if (!ext_glUniform4i64ARB)
        kpanic("ext_glUniform4i64ARB is NULL");
    {
    GL_FUNC(ext_glUniform4i64ARB)(ARG1, llARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glUniform4i64ARB GLint location=%d, GLint64 x=" PRIu64 ", GLint64 y=" PRIu64 ", GLint64 z=" PRIu64 ", GLint64 w=" PRIu64 "",ARG1,llARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glUniform4i64NV(CPU* cpu) {
    if (!ext_glUniform4i64NV)
        kpanic("ext_glUniform4i64NV is NULL");
    {
    GL_FUNC(ext_glUniform4i64NV)(ARG1, llARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glUniform4i64NV GLint location=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 ", GLint64EXT z=" PRIu64 ", GLint64EXT w=" PRIu64 "",ARG1,llARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glUniform4i64vARB(CPU* cpu) {
    if (!ext_glUniform4i64vARB)
        kpanic("ext_glUniform4i64vARB is NULL");
    {
    GL_FUNC(ext_glUniform4i64vARB)(ARG1, ARG2, marshalArray<GLint64>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4i64vARB GLint location=%d, GLsizei count=%d, const GLint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4i64vNV(CPU* cpu) {
    if (!ext_glUniform4i64vNV)
        kpanic("ext_glUniform4i64vNV is NULL");
    {
    GL_FUNC(ext_glUniform4i64vNV)(ARG1, ARG2, marshalArray<GLint64EXT>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4i64vNV GLint location=%d, GLsizei count=%d, const GLint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4iARB(CPU* cpu) {
    if (!ext_glUniform4iARB)
        kpanic("ext_glUniform4iARB is NULL");
    {
    GL_FUNC(ext_glUniform4iARB)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glUniform4iARB GLint location=%d, GLint v0=%d, GLint v1=%d, GLint v2=%d, GLint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glUniform4iv(CPU* cpu) {
    if (!ext_glUniform4iv)
        kpanic("ext_glUniform4iv is NULL");
    {
    GL_FUNC(ext_glUniform4iv)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4iv GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4ivARB(CPU* cpu) {
    if (!ext_glUniform4ivARB)
        kpanic("ext_glUniform4ivARB is NULL");
    {
    GL_FUNC(ext_glUniform4ivARB)(ARG1, ARG2, marshalArray<GLint>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4ivARB GLint location=%d, GLsizei count=%d, const GLint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4ui(CPU* cpu) {
    if (!ext_glUniform4ui)
        kpanic("ext_glUniform4ui is NULL");
    {
    GL_FUNC(ext_glUniform4ui)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glUniform4ui GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d, GLuint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glUniform4ui64ARB(CPU* cpu) {
    if (!ext_glUniform4ui64ARB)
        kpanic("ext_glUniform4ui64ARB is NULL");
    {
    GL_FUNC(ext_glUniform4ui64ARB)(ARG1, llARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glUniform4ui64ARB GLint location=%d, GLuint64 x=" PRIu64 ", GLuint64 y=" PRIu64 ", GLuint64 z=" PRIu64 ", GLuint64 w=" PRIu64 "",ARG1,llARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glUniform4ui64NV(CPU* cpu) {
    if (!ext_glUniform4ui64NV)
        kpanic("ext_glUniform4ui64NV is NULL");
    {
    GL_FUNC(ext_glUniform4ui64NV)(ARG1, llARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glUniform4ui64NV GLint location=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 ", GLuint64EXT z=" PRIu64 ", GLuint64EXT w=" PRIu64 "",ARG1,llARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glUniform4ui64vARB(CPU* cpu) {
    if (!ext_glUniform4ui64vARB)
        kpanic("ext_glUniform4ui64vARB is NULL");
    {
    GL_FUNC(ext_glUniform4ui64vARB)(ARG1, ARG2, marshalArray<GLuint64>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4ui64vARB GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4ui64vNV(CPU* cpu) {
    if (!ext_glUniform4ui64vNV)
        kpanic("ext_glUniform4ui64vNV is NULL");
    {
    GL_FUNC(ext_glUniform4ui64vNV)(ARG1, ARG2, marshalArray<GLuint64EXT>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4ui64vNV GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4uiEXT(CPU* cpu) {
    if (!ext_glUniform4uiEXT)
        kpanic("ext_glUniform4uiEXT is NULL");
    {
    GL_FUNC(ext_glUniform4uiEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glUniform4uiEXT GLint location=%d, GLuint v0=%d, GLuint v1=%d, GLuint v2=%d, GLuint v3=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glUniform4uiv(CPU* cpu) {
    if (!ext_glUniform4uiv)
        kpanic("ext_glUniform4uiv is NULL");
    {
    GL_FUNC(ext_glUniform4uiv)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4uiv GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniform4uivEXT(CPU* cpu) {
    if (!ext_glUniform4uivEXT)
        kpanic("ext_glUniform4uivEXT is NULL");
    {
    GL_FUNC(ext_glUniform4uivEXT)(ARG1, ARG2, marshalArray<GLuint>(cpu, ARG3, ARG2 * 4));
    GL_LOG ("glUniform4uivEXT GLint location=%d, GLsizei count=%d, const GLuint* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniformBlockBinding(CPU* cpu) {
    if (!ext_glUniformBlockBinding)
        kpanic("ext_glUniformBlockBinding is NULL");
    {
    GL_FUNC(ext_glUniformBlockBinding)(ARG1, ARG2, ARG3);
    GL_LOG ("glUniformBlockBinding GLuint program=%d, GLuint uniformBlockIndex=%d, GLuint uniformBlockBinding=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniformBufferEXT(CPU* cpu) {
    if (!ext_glUniformBufferEXT)
        kpanic("ext_glUniformBufferEXT is NULL");
    {
    GL_FUNC(ext_glUniformBufferEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glUniformBufferEXT GLuint program=%d, GLint location=%d, GLuint buffer=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniformHandleui64ARB(CPU* cpu) {
    if (!ext_glUniformHandleui64ARB)
        kpanic("ext_glUniformHandleui64ARB is NULL");
    {
    GL_FUNC(ext_glUniformHandleui64ARB)(ARG1, llARG2);
    GL_LOG ("glUniformHandleui64ARB GLint location=%d, GLuint64 value=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glUniformHandleui64NV(CPU* cpu) {
    if (!ext_glUniformHandleui64NV)
        kpanic("ext_glUniformHandleui64NV is NULL");
    {
    GL_FUNC(ext_glUniformHandleui64NV)(ARG1, llARG2);
    GL_LOG ("glUniformHandleui64NV GLint location=%d, GLuint64 value=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glUniformHandleui64vARB(CPU* cpu) {
    if (!ext_glUniformHandleui64vARB)
        kpanic("ext_glUniformHandleui64vARB is NULL");
    {
    GL_FUNC(ext_glUniformHandleui64vARB)(ARG1, ARG2, marshalArray<GLuint64>(cpu, ARG3, ARG2));
    GL_LOG ("glUniformHandleui64vARB GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniformHandleui64vNV(CPU* cpu) {
    if (!ext_glUniformHandleui64vNV)
        kpanic("ext_glUniformHandleui64vNV is NULL");
    {
    GL_FUNC(ext_glUniformHandleui64vNV)(ARG1, ARG2, marshalArray<GLuint64>(cpu, ARG3, ARG2));
    GL_LOG ("glUniformHandleui64vNV GLint location=%d, GLsizei count=%d, const GLuint64* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniformMatrix2dv(CPU* cpu) {
    if (!ext_glUniformMatrix2dv)
        kpanic("ext_glUniformMatrix2dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix2dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*4));
    GL_LOG ("glUniformMatrix2dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix2fv(CPU* cpu) {
    if (!ext_glUniformMatrix2fv)
        kpanic("ext_glUniformMatrix2fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix2fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*4));
    GL_LOG ("glUniformMatrix2fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix2fvARB(CPU* cpu) {
    if (!ext_glUniformMatrix2fvARB)
        kpanic("ext_glUniformMatrix2fvARB is NULL");
    {
    GL_FUNC(ext_glUniformMatrix2fvARB)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*4));
    GL_LOG ("glUniformMatrix2fvARB GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix2x3dv(CPU* cpu) {
    if (!ext_glUniformMatrix2x3dv)
        kpanic("ext_glUniformMatrix2x3dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix2x3dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*6));
    GL_LOG ("glUniformMatrix2x3dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix2x3fv(CPU* cpu) {
    if (!ext_glUniformMatrix2x3fv)
        kpanic("ext_glUniformMatrix2x3fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix2x3fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*6));
    GL_LOG ("glUniformMatrix2x3fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix2x4dv(CPU* cpu) {
    if (!ext_glUniformMatrix2x4dv)
        kpanic("ext_glUniformMatrix2x4dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix2x4dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*8));
    GL_LOG ("glUniformMatrix2x4dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix2x4fv(CPU* cpu) {
    if (!ext_glUniformMatrix2x4fv)
        kpanic("ext_glUniformMatrix2x4fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix2x4fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*8));
    GL_LOG ("glUniformMatrix2x4fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix3dv(CPU* cpu) {
    if (!ext_glUniformMatrix3dv)
        kpanic("ext_glUniformMatrix3dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix3dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*9));
    GL_LOG ("glUniformMatrix3dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix3fv(CPU* cpu) {
    if (!ext_glUniformMatrix3fv)
        kpanic("ext_glUniformMatrix3fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix3fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*9));
    GL_LOG ("glUniformMatrix3fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix3fvARB(CPU* cpu) {
    if (!ext_glUniformMatrix3fvARB)
        kpanic("ext_glUniformMatrix3fvARB is NULL");
    {
    GL_FUNC(ext_glUniformMatrix3fvARB)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*9));
    GL_LOG ("glUniformMatrix3fvARB GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix3x2dv(CPU* cpu) {
    if (!ext_glUniformMatrix3x2dv)
        kpanic("ext_glUniformMatrix3x2dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix3x2dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*6));
    GL_LOG ("glUniformMatrix3x2dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix3x2fv(CPU* cpu) {
    if (!ext_glUniformMatrix3x2fv)
        kpanic("ext_glUniformMatrix3x2fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix3x2fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*6));
    GL_LOG ("glUniformMatrix3x2fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix3x4dv(CPU* cpu) {
    if (!ext_glUniformMatrix3x4dv)
        kpanic("ext_glUniformMatrix3x4dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix3x4dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*12));
    GL_LOG ("glUniformMatrix3x4dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix3x4fv(CPU* cpu) {
    if (!ext_glUniformMatrix3x4fv)
        kpanic("ext_glUniformMatrix3x4fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix3x4fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*12));
    GL_LOG ("glUniformMatrix3x4fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix4dv(CPU* cpu) {
    if (!ext_glUniformMatrix4dv)
        kpanic("ext_glUniformMatrix4dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix4dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*16));
    GL_LOG ("glUniformMatrix4dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix4fv(CPU* cpu) {
    if (!ext_glUniformMatrix4fv)
        kpanic("ext_glUniformMatrix4fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix4fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*16));
    GL_LOG ("glUniformMatrix4fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix4fvARB(CPU* cpu) {
    if (!ext_glUniformMatrix4fvARB)
        kpanic("ext_glUniformMatrix4fvARB is NULL");
    {
    GL_FUNC(ext_glUniformMatrix4fvARB)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*16));
    GL_LOG ("glUniformMatrix4fvARB GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix4x2dv(CPU* cpu) {
    if (!ext_glUniformMatrix4x2dv)
        kpanic("ext_glUniformMatrix4x2dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix4x2dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*8));
    GL_LOG ("glUniformMatrix4x2dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix4x2fv(CPU* cpu) {
    if (!ext_glUniformMatrix4x2fv)
        kpanic("ext_glUniformMatrix4x2fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix4x2fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*8));
    GL_LOG ("glUniformMatrix4x2fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix4x3dv(CPU* cpu) {
    if (!ext_glUniformMatrix4x3dv)
        kpanic("ext_glUniformMatrix4x3dv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix4x3dv)(ARG1, ARG2, bARG3, marshalArray<GLdouble>(cpu, ARG4, ARG2*12));
    GL_LOG ("glUniformMatrix4x3dv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLdouble* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformMatrix4x3fv(CPU* cpu) {
    if (!ext_glUniformMatrix4x3fv)
        kpanic("ext_glUniformMatrix4x3fv is NULL");
    {
    GL_FUNC(ext_glUniformMatrix4x3fv)(ARG1, ARG2, bARG3, marshalArray<GLfloat>(cpu, ARG4, ARG2*12));
    GL_LOG ("glUniformMatrix4x3fv GLint location=%d, GLsizei count=%d, GLboolean transpose=%d, const GLfloat* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glUniformSubroutinesuiv(CPU* cpu) {
    if (!ext_glUniformSubroutinesuiv)
        kpanic("ext_glUniformSubroutinesuiv is NULL");
    {
    GL_FUNC(ext_glUniformSubroutinesuiv)(ARG1, ARG2, (GLuint*)marshalArray<GLuint>(cpu, ARG3, ARG2));
    GL_LOG ("glUniformSubroutinesuiv GLenum shadertype=%d, GLsizei count=%d, const GLuint* indices=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUniformui64NV(CPU* cpu) {
    if (!ext_glUniformui64NV)
        kpanic("ext_glUniformui64NV is NULL");
    {
    GL_FUNC(ext_glUniformui64NV)(ARG1, llARG2);
    GL_LOG ("glUniformui64NV GLint location=%d, GLuint64EXT value=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glUniformui64vNV(CPU* cpu) {
    if (!ext_glUniformui64vNV)
        kpanic("ext_glUniformui64vNV is NULL");
    {
    GL_FUNC(ext_glUniformui64vNV)(ARG1, ARG2, marshalArray<GLuint64EXT>(cpu, ARG3, ARG2));
    GL_LOG ("glUniformui64vNV GLint location=%d, GLsizei count=%d, const GLuint64EXT* value=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUnlockArraysEXT(CPU* cpu) {
    if (!ext_glUnlockArraysEXT)
        kpanic("ext_glUnlockArraysEXT is NULL");
    {
    GL_FUNC(ext_glUnlockArraysEXT)();
    GL_LOG ("glUnlockArraysEXT");
    }
}
void glcommon_glUnmapBuffer(CPU* cpu) {
    if (!ext_glUnmapBuffer)
        kpanic("ext_glUnmapBuffer is NULL");
    {

    unmapBuffer(cpu, ARG1);

    EAX=GL_FUNC(ext_glUnmapBuffer)(ARG1);
    GL_LOG ("glUnmapBuffer GLenum target=%d",ARG1);
    }
}
void glcommon_glUnmapBufferARB(CPU* cpu) {
    if (!ext_glUnmapBufferARB)
        kpanic("ext_glUnmapBufferARB is NULL");
    {
    EAX=GL_FUNC(ext_glUnmapBufferARB)(ARG1);
    GL_LOG ("glUnmapBufferARB GLenum target=%d",ARG1);
    }
}
void glcommon_glUnmapNamedBuffer(CPU* cpu) {
    if (!ext_glUnmapNamedBuffer)
        kpanic("ext_glUnmapNamedBuffer is NULL");
    {
    EAX=GL_FUNC(ext_glUnmapNamedBuffer)(ARG1);
    GL_LOG ("glUnmapNamedBuffer GLuint buffer=%d",ARG1);
    }
}
void glcommon_glUnmapNamedBufferEXT(CPU* cpu) {
    if (!ext_glUnmapNamedBufferEXT)
        kpanic("ext_glUnmapNamedBufferEXT is NULL");
    {
    EAX=GL_FUNC(ext_glUnmapNamedBufferEXT)(ARG1);
    GL_LOG ("glUnmapNamedBufferEXT GLuint buffer=%d",ARG1);
    }
}
void glcommon_glUnmapObjectBufferATI(CPU* cpu) {
    if (!ext_glUnmapObjectBufferATI)
        kpanic("ext_glUnmapObjectBufferATI is NULL");
    {
    GL_FUNC(ext_glUnmapObjectBufferATI)(ARG1);
    GL_LOG ("glUnmapObjectBufferATI GLuint buffer=%d",ARG1);
    }
}
void glcommon_glUnmapTexture2DINTEL(CPU* cpu) {
    if (!ext_glUnmapTexture2DINTEL)
        kpanic("ext_glUnmapTexture2DINTEL is NULL");
    {
    GL_FUNC(ext_glUnmapTexture2DINTEL)(ARG1, ARG2);
    GL_LOG ("glUnmapTexture2DINTEL GLuint texture=%d, GLint level=%d",ARG1,ARG2);
    }
}
void glcommon_glUpdateObjectBufferATI(CPU* cpu) {
    if (!ext_glUpdateObjectBufferATI)
        kpanic("ext_glUpdateObjectBufferATI is NULL");
    {
    GL_FUNC(ext_glUpdateObjectBufferATI)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0), ARG5);
    GL_LOG ("glUpdateObjectBufferATI GLuint buffer=%d, GLuint offset=%d, GLsizei size=%d, const void* pointer=%.08x, GLenum preserve=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glUseProgram(CPU* cpu) {
    if (!ext_glUseProgram)
        kpanic("ext_glUseProgram is NULL");
    {
    GL_FUNC(ext_glUseProgram)(ARG1);
    GL_LOG ("glUseProgram GLuint program=%d",ARG1);
    }
}
void glcommon_glUseProgramObjectARB(CPU* cpu) {
    if (!ext_glUseProgramObjectARB)
        kpanic("ext_glUseProgramObjectARB is NULL");
    {
    GL_FUNC(ext_glUseProgramObjectARB)(INDEX_TO_HANDLE(hARG1));
    GL_LOG ("glUseProgramObjectARB GLhandleARB programObj=%d",ARG1);
    }
}
void glcommon_glUseProgramStages(CPU* cpu) {
    if (!ext_glUseProgramStages)
        kpanic("ext_glUseProgramStages is NULL");
    {
    GL_FUNC(ext_glUseProgramStages)(ARG1, ARG2, ARG3);
    GL_LOG ("glUseProgramStages GLuint pipeline=%d, GLbitfield stages=%d, GLuint program=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glUseShaderProgramEXT(CPU* cpu) {
    if (!ext_glUseShaderProgramEXT)
        kpanic("ext_glUseShaderProgramEXT is NULL");
    {
    GL_FUNC(ext_glUseShaderProgramEXT)(ARG1, ARG2);
    GL_LOG ("glUseShaderProgramEXT GLenum type=%d, GLuint program=%d",ARG1,ARG2);
    }
}
void glcommon_glVDPAUFiniNV(CPU* cpu) {
    if (!ext_glVDPAUFiniNV)
        kpanic("ext_glVDPAUFiniNV is NULL");
    {
    GL_FUNC(ext_glVDPAUFiniNV)();
    GL_LOG ("glVDPAUFiniNV");
    }
}
void glcommon_glVDPAUGetSurfaceivNV(CPU* cpu) {
    if (!ext_glVDPAUGetSurfaceivNV)
        kpanic("ext_glVDPAUGetSurfaceivNV is NULL");
    {
    GL_FUNC(ext_glVDPAUGetSurfaceivNV)(ARG1, ARG2, ARG3, (GLsizei*)marshalp(cpu, 0, ARG4, 0), (GLint*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glVDPAUGetSurfaceivNV GLvdpauSurfaceNV surface=%d, GLenum pname=%d, GLsizei bufSize=%d, GLsizei* length=%.08x, GLint* values=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVDPAUInitNV(CPU* cpu) {
    if (!ext_glVDPAUInitNV)
        kpanic("ext_glVDPAUInitNV is NULL");
    {
    GL_FUNC(ext_glVDPAUInitNV)((void*)marshalp(cpu, 0, ARG1, 0), (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVDPAUInitNV const void* vdpDevice=%.08x, const void* getProcAddress=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVDPAUIsSurfaceNV(CPU* cpu) {
    if (!ext_glVDPAUIsSurfaceNV)
        kpanic("ext_glVDPAUIsSurfaceNV is NULL");
    {
    EAX=GL_FUNC(ext_glVDPAUIsSurfaceNV)(ARG1);
    GL_LOG ("glVDPAUIsSurfaceNV GLvdpauSurfaceNV surface=%d",ARG1);
    }
}
void glcommon_glVDPAUMapSurfacesNV(CPU* cpu) {
    if (!ext_glVDPAUMapSurfacesNV)
        kpanic("ext_glVDPAUMapSurfacesNV is NULL");
    {
    GL_FUNC(ext_glVDPAUMapSurfacesNV)(ARG1, (GLvdpauSurfaceNV*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVDPAUMapSurfacesNV GLsizei numSurfaces=%d, const GLvdpauSurfaceNV* surfaces=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVDPAURegisterOutputSurfaceNV(CPU* cpu) {
    if (!ext_glVDPAURegisterOutputSurfaceNV)
        kpanic("ext_glVDPAURegisterOutputSurfaceNV is NULL");
    {
    GLvdpauSurfaceNV result=(GLvdpauSurfaceNV)GL_FUNC(ext_glVDPAURegisterOutputSurfaceNV)((void*)marshalp(cpu, 0, ARG1, 0), ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    if (sizeof(GLvdpauSurfaceNV)>4 && result>0xFFFFFFFFl)
        kwarn("problem with glVDPAURegisterOutputSurfaceNV");
    EAX=(U32)result;
    GL_LOG ("glVDPAURegisterOutputSurfaceNV const void* vdpSurface=%.08x, GLenum target=%d, GLsizei numTextureNames=%d, const GLuint* textureNames=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVDPAURegisterVideoSurfaceNV(CPU* cpu) {
    if (!ext_glVDPAURegisterVideoSurfaceNV)
        kpanic("ext_glVDPAURegisterVideoSurfaceNV is NULL");
    {
    GLvdpauSurfaceNV result=(GLvdpauSurfaceNV)GL_FUNC(ext_glVDPAURegisterVideoSurfaceNV)((void*)marshalp(cpu, 0, ARG1, 0), ARG2, ARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    if (sizeof(GLvdpauSurfaceNV)>4 && result>0xffffffffl)
        kwarn("problem with glVDPAURegisterVideoSurfaceNV");
    EAX=(U32)result;
    GL_LOG ("glVDPAURegisterVideoSurfaceNV const void* vdpSurface=%.08x, GLenum target=%d, GLsizei numTextureNames=%d, const GLuint* textureNames=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVDPAUSurfaceAccessNV(CPU* cpu) {
    if (!ext_glVDPAUSurfaceAccessNV)
        kpanic("ext_glVDPAUSurfaceAccessNV is NULL");
    {
    GL_FUNC(ext_glVDPAUSurfaceAccessNV)(ARG1, ARG2);
    GL_LOG ("glVDPAUSurfaceAccessNV GLvdpauSurfaceNV surface=%d, GLenum access=%d",ARG1,ARG2);
    }
}
void glcommon_glVDPAUUnmapSurfacesNV(CPU* cpu) {
    if (!ext_glVDPAUUnmapSurfacesNV)
        kpanic("ext_glVDPAUUnmapSurfacesNV is NULL");
    {
    GL_FUNC(ext_glVDPAUUnmapSurfacesNV)(ARG1, (GLvdpauSurfaceNV*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVDPAUUnmapSurfacesNV GLsizei numSurface=%d, const GLvdpauSurfaceNV* surfaces=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVDPAUUnregisterSurfaceNV(CPU* cpu) {
    if (!ext_glVDPAUUnregisterSurfaceNV)
        kpanic("ext_glVDPAUUnregisterSurfaceNV is NULL");
    {
    GL_FUNC(ext_glVDPAUUnregisterSurfaceNV)(ARG1);
    GL_LOG ("glVDPAUUnregisterSurfaceNV GLvdpauSurfaceNV surface=%d",ARG1);
    }
}
void glcommon_glValidateProgram(CPU* cpu) {
    if (!ext_glValidateProgram)
        kpanic("ext_glValidateProgram is NULL");
    {
    GL_FUNC(ext_glValidateProgram)(ARG1);
    GL_LOG ("glValidateProgram GLuint program=%d",ARG1);
    }
}
void glcommon_glValidateProgramARB(CPU* cpu) {
    if (!ext_glValidateProgramARB)
        kpanic("ext_glValidateProgramARB is NULL");
    {
    GL_FUNC(ext_glValidateProgramARB)(INDEX_TO_HANDLE(hARG1));
    GL_LOG ("glValidateProgramARB GLhandleARB programObj=%d",ARG1);
    }
}
void glcommon_glValidateProgramPipeline(CPU* cpu) {
    if (!ext_glValidateProgramPipeline)
        kpanic("ext_glValidateProgramPipeline is NULL");
    {
    GL_FUNC(ext_glValidateProgramPipeline)(ARG1);
    GL_LOG ("glValidateProgramPipeline GLuint pipeline=%d",ARG1);
    }
}
void glcommon_glVariantArrayObjectATI(CPU* cpu) {
    if (!ext_glVariantArrayObjectATI)
        kpanic("ext_glVariantArrayObjectATI is NULL");
    {
    GL_FUNC(ext_glVariantArrayObjectATI)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVariantArrayObjectATI GLuint id=%d, GLenum type=%d, GLsizei stride=%d, GLuint buffer=%d, GLuint offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVariantPointerEXT(CPU* cpu) {
    if (!ext_glVariantPointerEXT)
        kpanic("ext_glVariantPointerEXT is NULL");
    {
    GL_FUNC(ext_glVariantPointerEXT)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVariantPointerEXT GLuint id=%d, GLenum type=%d, GLuint stride=%d, const void* addr=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVariantbvEXT(CPU* cpu) {
    if (!ext_glVariantbvEXT)
        kpanic("ext_glVariantbvEXT is NULL");
    {
    GL_FUNC(ext_glVariantbvEXT)(ARG1, (GLbyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantbvEXT GLuint id=%d, const GLbyte* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVariantdvEXT(CPU* cpu) {
    if (!ext_glVariantdvEXT)
        kpanic("ext_glVariantdvEXT is NULL");
    {
    GL_FUNC(ext_glVariantdvEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantdvEXT GLuint id=%d, const GLdouble* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVariantfvEXT(CPU* cpu) {
    if (!ext_glVariantfvEXT)
        kpanic("ext_glVariantfvEXT is NULL");
    {
    GL_FUNC(ext_glVariantfvEXT)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantfvEXT GLuint id=%d, const GLfloat* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVariantivEXT(CPU* cpu) {
    if (!ext_glVariantivEXT)
        kpanic("ext_glVariantivEXT is NULL");
    {
    GL_FUNC(ext_glVariantivEXT)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantivEXT GLuint id=%d, const GLint* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVariantsvEXT(CPU* cpu) {
    if (!ext_glVariantsvEXT)
        kpanic("ext_glVariantsvEXT is NULL");
    {
    GL_FUNC(ext_glVariantsvEXT)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantsvEXT GLuint id=%d, const GLshort* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVariantubvEXT(CPU* cpu) {
    if (!ext_glVariantubvEXT)
        kpanic("ext_glVariantubvEXT is NULL");
    {
    GL_FUNC(ext_glVariantubvEXT)(ARG1, (GLubyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantubvEXT GLuint id=%d, const GLubyte* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVariantuivEXT(CPU* cpu) {
    if (!ext_glVariantuivEXT)
        kpanic("ext_glVariantuivEXT is NULL");
    {
    GL_FUNC(ext_glVariantuivEXT)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantuivEXT GLuint id=%d, const GLuint* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVariantusvEXT(CPU* cpu) {
    if (!ext_glVariantusvEXT)
        kpanic("ext_glVariantusvEXT is NULL");
    {
    GL_FUNC(ext_glVariantusvEXT)(ARG1, (GLushort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVariantusvEXT GLuint id=%d, const GLushort* addr=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertex2bOES(CPU* cpu) {
    if (!ext_glVertex2bOES)
        kpanic("ext_glVertex2bOES is NULL");
    {
    GL_FUNC(ext_glVertex2bOES)(bARG1, bARG2);
    GL_LOG ("glVertex2bOES GLbyte x=%d, GLbyte y=%d",bARG1,bARG2);
    }
}
void glcommon_glVertex2bvOES(CPU* cpu) {
    if (!ext_glVertex2bvOES)
        kpanic("ext_glVertex2bvOES is NULL");
    {
    GL_FUNC(ext_glVertex2bvOES)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex2bvOES const GLbyte* coords=%.08x",ARG1);
    }
}
void glcommon_glVertex2hNV(CPU* cpu) {
    if (!ext_glVertex2hNV)
        kpanic("ext_glVertex2hNV is NULL");
    {
    GL_FUNC(ext_glVertex2hNV)(sARG1, sARG2);
    GL_LOG ("glVertex2hNV GLhalfNV x=%d, GLhalfNV y=%d",sARG1,sARG2);
    }
}
void glcommon_glVertex2hvNV(CPU* cpu) {
    if (!ext_glVertex2hvNV)
        kpanic("ext_glVertex2hvNV is NULL");
    {
    GL_FUNC(ext_glVertex2hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex2hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glVertex2xOES(CPU* cpu) {
    if (!ext_glVertex2xOES)
        kpanic("ext_glVertex2xOES is NULL");
    {
    GL_FUNC(ext_glVertex2xOES)(ARG1);
    GL_LOG ("glVertex2xOES GLfixed x=%d",ARG1);
    }
}
void glcommon_glVertex2xvOES(CPU* cpu) {
    if (!ext_glVertex2xvOES)
        kpanic("ext_glVertex2xvOES is NULL");
    {
    GL_FUNC(ext_glVertex2xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex2xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glVertex3bOES(CPU* cpu) {
    if (!ext_glVertex3bOES)
        kpanic("ext_glVertex3bOES is NULL");
    {
    GL_FUNC(ext_glVertex3bOES)(bARG1, bARG2, bARG3);
    GL_LOG ("glVertex3bOES GLbyte x=%d, GLbyte y=%d, GLbyte z=%d",bARG1,bARG2,bARG3);
    }
}
void glcommon_glVertex3bvOES(CPU* cpu) {
    if (!ext_glVertex3bvOES)
        kpanic("ext_glVertex3bvOES is NULL");
    {
    GL_FUNC(ext_glVertex3bvOES)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex3bvOES const GLbyte* coords=%.08x",ARG1);
    }
}
void glcommon_glVertex3hNV(CPU* cpu) {
    if (!ext_glVertex3hNV)
        kpanic("ext_glVertex3hNV is NULL");
    {
    GL_FUNC(ext_glVertex3hNV)(sARG1, sARG2, sARG3);
    GL_LOG ("glVertex3hNV GLhalfNV x=%d, GLhalfNV y=%d, GLhalfNV z=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glVertex3hvNV(CPU* cpu) {
    if (!ext_glVertex3hvNV)
        kpanic("ext_glVertex3hvNV is NULL");
    {
    GL_FUNC(ext_glVertex3hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex3hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glVertex3xOES(CPU* cpu) {
    if (!ext_glVertex3xOES)
        kpanic("ext_glVertex3xOES is NULL");
    {
    GL_FUNC(ext_glVertex3xOES)(ARG1, ARG2);
    GL_LOG ("glVertex3xOES GLfixed x=%d, GLfixed y=%d",ARG1,ARG2);
    }
}
void glcommon_glVertex3xvOES(CPU* cpu) {
    if (!ext_glVertex3xvOES)
        kpanic("ext_glVertex3xvOES is NULL");
    {
    GL_FUNC(ext_glVertex3xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex3xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glVertex4bOES(CPU* cpu) {
    if (!ext_glVertex4bOES)
        kpanic("ext_glVertex4bOES is NULL");
    {
    GL_FUNC(ext_glVertex4bOES)(bARG1, bARG2, bARG3, bARG4);
    GL_LOG ("glVertex4bOES GLbyte x=%d, GLbyte y=%d, GLbyte z=%d, GLbyte w=%d",bARG1,bARG2,bARG3,bARG4);
    }
}
void glcommon_glVertex4bvOES(CPU* cpu) {
    if (!ext_glVertex4bvOES)
        kpanic("ext_glVertex4bvOES is NULL");
    {
    GL_FUNC(ext_glVertex4bvOES)((GLbyte*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex4bvOES const GLbyte* coords=%.08x",ARG1);
    }
}
void glcommon_glVertex4hNV(CPU* cpu) {
    if (!ext_glVertex4hNV)
        kpanic("ext_glVertex4hNV is NULL");
    {
    GL_FUNC(ext_glVertex4hNV)(sARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glVertex4hNV GLhalfNV x=%d, GLhalfNV y=%d, GLhalfNV z=%d, GLhalfNV w=%d",sARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glVertex4hvNV(CPU* cpu) {
    if (!ext_glVertex4hvNV)
        kpanic("ext_glVertex4hvNV is NULL");
    {
    GL_FUNC(ext_glVertex4hvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex4hvNV const GLhalfNV* v=%.08x",ARG1);
    }
}
void glcommon_glVertex4xOES(CPU* cpu) {
    if (!ext_glVertex4xOES)
        kpanic("ext_glVertex4xOES is NULL");
    {
    GL_FUNC(ext_glVertex4xOES)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertex4xOES GLfixed x=%d, GLfixed y=%d, GLfixed z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertex4xvOES(CPU* cpu) {
    if (!ext_glVertex4xvOES)
        kpanic("ext_glVertex4xvOES is NULL");
    {
    GL_FUNC(ext_glVertex4xvOES)((GLfixed*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertex4xvOES const GLfixed* coords=%.08x",ARG1);
    }
}
void glcommon_glVertexArrayAttribBinding(CPU* cpu) {
    if (!ext_glVertexArrayAttribBinding)
        kpanic("ext_glVertexArrayAttribBinding is NULL");
    {
    GL_FUNC(ext_glVertexArrayAttribBinding)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexArrayAttribBinding GLuint vaobj=%d, GLuint attribindex=%d, GLuint bindingindex=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexArrayAttribFormat(CPU* cpu) {
    if (!ext_glVertexArrayAttribFormat)
        kpanic("ext_glVertexArrayAttribFormat is NULL");
    {
    GL_FUNC(ext_glVertexArrayAttribFormat)(ARG1, ARG2, ARG3, ARG4, bARG5, ARG6);
    GL_LOG ("glVertexArrayAttribFormat GLuint vaobj=%d, GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4,bARG5,ARG6);
    }
}
void glcommon_glVertexArrayAttribIFormat(CPU* cpu) {
    if (!ext_glVertexArrayAttribIFormat)
        kpanic("ext_glVertexArrayAttribIFormat is NULL");
    {
    GL_FUNC(ext_glVertexArrayAttribIFormat)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayAttribIFormat GLuint vaobj=%d, GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayAttribLFormat(CPU* cpu) {
    if (!ext_glVertexArrayAttribLFormat)
        kpanic("ext_glVertexArrayAttribLFormat is NULL");
    {
    GL_FUNC(ext_glVertexArrayAttribLFormat)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayAttribLFormat GLuint vaobj=%d, GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayBindVertexBufferEXT(CPU* cpu) {
    if (!ext_glVertexArrayBindVertexBufferEXT)
        kpanic("ext_glVertexArrayBindVertexBufferEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayBindVertexBufferEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayBindVertexBufferEXT GLuint vaobj=%d, GLuint bindingindex=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayBindingDivisor(CPU* cpu) {
    if (!ext_glVertexArrayBindingDivisor)
        kpanic("ext_glVertexArrayBindingDivisor is NULL");
    {
    GL_FUNC(ext_glVertexArrayBindingDivisor)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexArrayBindingDivisor GLuint vaobj=%d, GLuint bindingindex=%d, GLuint divisor=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexArrayColorOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayColorOffsetEXT)
        kpanic("ext_glVertexArrayColorOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayColorOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glVertexArrayColorOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glVertexArrayEdgeFlagOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayEdgeFlagOffsetEXT)
        kpanic("ext_glVertexArrayEdgeFlagOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayEdgeFlagOffsetEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexArrayEdgeFlagOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexArrayElementBuffer(CPU* cpu) {
    if (!ext_glVertexArrayElementBuffer)
        kpanic("ext_glVertexArrayElementBuffer is NULL");
    {
    GL_FUNC(ext_glVertexArrayElementBuffer)(ARG1, ARG2);
    GL_LOG ("glVertexArrayElementBuffer GLuint vaobj=%d, GLuint buffer=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexArrayFogCoordOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayFogCoordOffsetEXT)
        kpanic("ext_glVertexArrayFogCoordOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayFogCoordOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayFogCoordOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayIndexOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayIndexOffsetEXT)
        kpanic("ext_glVertexArrayIndexOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayIndexOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayIndexOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayMultiTexCoordOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayMultiTexCoordOffsetEXT)
        kpanic("ext_glVertexArrayMultiTexCoordOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayMultiTexCoordOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glVertexArrayMultiTexCoordOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLenum texunit=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glVertexArrayNormalOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayNormalOffsetEXT)
        kpanic("ext_glVertexArrayNormalOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayNormalOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayNormalOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayParameteriAPPLE(CPU* cpu) {
    if (!ext_glVertexArrayParameteriAPPLE)
        kpanic("ext_glVertexArrayParameteriAPPLE is NULL");
    {
    GL_FUNC(ext_glVertexArrayParameteriAPPLE)(ARG1, ARG2);
    GL_LOG ("glVertexArrayParameteriAPPLE GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexArrayRangeAPPLE(CPU* cpu) {
    if (!ext_glVertexArrayRangeAPPLE)
        kpanic("ext_glVertexArrayRangeAPPLE is NULL");
    {
    GL_FUNC(ext_glVertexArrayRangeAPPLE)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexArrayRangeAPPLE GLsizei length=%d, void* pointer=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexArrayRangeNV(CPU* cpu) {
    if (!ext_glVertexArrayRangeNV)
        kpanic("ext_glVertexArrayRangeNV is NULL");
    {
    GL_FUNC(ext_glVertexArrayRangeNV)(ARG1, (void*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexArrayRangeNV GLsizei length=%d, const void* pointer=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexArraySecondaryColorOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArraySecondaryColorOffsetEXT)
        kpanic("ext_glVertexArraySecondaryColorOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArraySecondaryColorOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glVertexArraySecondaryColorOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glVertexArrayTexCoordOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayTexCoordOffsetEXT)
        kpanic("ext_glVertexArrayTexCoordOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayTexCoordOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glVertexArrayTexCoordOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glVertexArrayVertexAttribBindingEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribBindingEXT)
        kpanic("ext_glVertexArrayVertexAttribBindingEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribBindingEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexArrayVertexAttribBindingEXT GLuint vaobj=%d, GLuint attribindex=%d, GLuint bindingindex=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexArrayVertexAttribDivisorEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribDivisorEXT)
        kpanic("ext_glVertexArrayVertexAttribDivisorEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribDivisorEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexArrayVertexAttribDivisorEXT GLuint vaobj=%d, GLuint index=%d, GLuint divisor=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexArrayVertexAttribFormatEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribFormatEXT)
        kpanic("ext_glVertexArrayVertexAttribFormatEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribFormatEXT)(ARG1, ARG2, ARG3, ARG4, bARG5, ARG6);
    GL_LOG ("glVertexArrayVertexAttribFormatEXT GLuint vaobj=%d, GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4,bARG5,ARG6);
    }
}
void glcommon_glVertexArrayVertexAttribIFormatEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribIFormatEXT)
        kpanic("ext_glVertexArrayVertexAttribIFormatEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribIFormatEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayVertexAttribIFormatEXT GLuint vaobj=%d, GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayVertexAttribIOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribIOffsetEXT)
        kpanic("ext_glVertexArrayVertexAttribIOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribIOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glVertexArrayVertexAttribIOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glVertexArrayVertexAttribLFormatEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribLFormatEXT)
        kpanic("ext_glVertexArrayVertexAttribLFormatEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribLFormatEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayVertexAttribLFormatEXT GLuint vaobj=%d, GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayVertexAttribLOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribLOffsetEXT)
        kpanic("ext_glVertexArrayVertexAttribLOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribLOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glVertexArrayVertexAttribLOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glVertexArrayVertexAttribOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexAttribOffsetEXT)
        kpanic("ext_glVertexArrayVertexAttribOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexAttribOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, bARG6, ARG7, ARG8);
    GL_LOG ("glVertexArrayVertexAttribOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLuint index=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,bARG6,ARG7,ARG8);
    }
}
void glcommon_glVertexArrayVertexBindingDivisorEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexBindingDivisorEXT)
        kpanic("ext_glVertexArrayVertexBindingDivisorEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexBindingDivisorEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexArrayVertexBindingDivisorEXT GLuint vaobj=%d, GLuint bindingindex=%d, GLuint divisor=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexArrayVertexBuffer(CPU* cpu) {
    if (!ext_glVertexArrayVertexBuffer)
        kpanic("ext_glVertexArrayVertexBuffer is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexBuffer)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexArrayVertexBuffer GLuint vaobj=%d, GLuint bindingindex=%d, GLuint buffer=%d, GLintptr offset=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexArrayVertexBuffers(CPU* cpu) {
    if (!ext_glVertexArrayVertexBuffers)
        kpanic("ext_glVertexArrayVertexBuffers is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexBuffers)(ARG1, ARG2, ARG3, (GLuint*)marshalArray<GLuint>(cpu, ARG4, ARG3), (GLintptr*)marshalip(cpu, ARG5, ARG2), (GLsizei*)marshalArray<GLint>(cpu, ARG6, ARG2));
    GL_LOG ("glVertexArrayVertexBuffers GLuint vaobj=%d, GLuint first=%d, GLsizei count=%d, const GLuint* buffers=%.08x, const GLintptr* offsets=%.08x, const GLsizei* strides=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glVertexArrayVertexOffsetEXT(CPU* cpu) {
    if (!ext_glVertexArrayVertexOffsetEXT)
        kpanic("ext_glVertexArrayVertexOffsetEXT is NULL");
    {
    GL_FUNC(ext_glVertexArrayVertexOffsetEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glVertexArrayVertexOffsetEXT GLuint vaobj=%d, GLuint buffer=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLintptr offset=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
void glcommon_glVertexAttrib1d(CPU* cpu) {
    if (!ext_glVertexAttrib1d)
        kpanic("ext_glVertexAttrib1d is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1d)(ARG1, dARG2);
    GL_LOG ("glVertexAttrib1d GLuint index=%d, GLdouble x=%f",ARG1,dARG2);
    }
}
void glcommon_glVertexAttrib1dARB(CPU* cpu) {
    if (!ext_glVertexAttrib1dARB)
        kpanic("ext_glVertexAttrib1dARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1dARB)(ARG1, dARG2);
    GL_LOG ("glVertexAttrib1dARB GLuint index=%d, GLdouble x=%f",ARG1,dARG2);
    }
}
void glcommon_glVertexAttrib1dNV(CPU* cpu) {
    if (!ext_glVertexAttrib1dNV)
        kpanic("ext_glVertexAttrib1dNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1dNV)(ARG1, dARG2);
    GL_LOG ("glVertexAttrib1dNV GLuint index=%d, GLdouble x=%f",ARG1,dARG2);
    }
}
void glcommon_glVertexAttrib1dv(CPU* cpu) {
    if (!ext_glVertexAttrib1dv)
        kpanic("ext_glVertexAttrib1dv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1dv)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1dvARB(CPU* cpu) {
    if (!ext_glVertexAttrib1dvARB)
        kpanic("ext_glVertexAttrib1dvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1dvARB)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1dvARB GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1dvNV(CPU* cpu) {
    if (!ext_glVertexAttrib1dvNV)
        kpanic("ext_glVertexAttrib1dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1dvNV)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1dvNV GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1f(CPU* cpu) {
    if (!ext_glVertexAttrib1f)
        kpanic("ext_glVertexAttrib1f is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1f)(ARG1, fARG2);
    GL_LOG ("glVertexAttrib1f GLuint index=%d, GLfloat x=%f",ARG1,fARG2);
    }
}
void glcommon_glVertexAttrib1fARB(CPU* cpu) {
    if (!ext_glVertexAttrib1fARB)
        kpanic("ext_glVertexAttrib1fARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1fARB)(ARG1, fARG2);
    GL_LOG ("glVertexAttrib1fARB GLuint index=%d, GLfloat x=%f",ARG1,fARG2);
    }
}
void glcommon_glVertexAttrib1fNV(CPU* cpu) {
    if (!ext_glVertexAttrib1fNV)
        kpanic("ext_glVertexAttrib1fNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1fNV)(ARG1, fARG2);
    GL_LOG ("glVertexAttrib1fNV GLuint index=%d, GLfloat x=%f",ARG1,fARG2);
    }
}
void glcommon_glVertexAttrib1fv(CPU* cpu) {
    if (!ext_glVertexAttrib1fv)
        kpanic("ext_glVertexAttrib1fv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1fv)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1fv GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1fvARB(CPU* cpu) {
    if (!ext_glVertexAttrib1fvARB)
        kpanic("ext_glVertexAttrib1fvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1fvARB)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1fvARB GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1fvNV(CPU* cpu) {
    if (!ext_glVertexAttrib1fvNV)
        kpanic("ext_glVertexAttrib1fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1fvNV)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1fvNV GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1hNV(CPU* cpu) {
    if (!ext_glVertexAttrib1hNV)
        kpanic("ext_glVertexAttrib1hNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1hNV)(ARG1, sARG2);
    GL_LOG ("glVertexAttrib1hNV GLuint index=%d, GLhalfNV x=%d",ARG1,sARG2);
    }
}
void glcommon_glVertexAttrib1hvNV(CPU* cpu) {
    if (!ext_glVertexAttrib1hvNV)
        kpanic("ext_glVertexAttrib1hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1hvNV)(ARG1, (GLhalfNV*)marshalArray<GLhalfNV>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1hvNV GLuint index=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1s(CPU* cpu) {
    if (!ext_glVertexAttrib1s)
        kpanic("ext_glVertexAttrib1s is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1s)(ARG1, sARG2);
    GL_LOG ("glVertexAttrib1s GLuint index=%d, GLshort x=%d",ARG1,sARG2);
    }
}
void glcommon_glVertexAttrib1sARB(CPU* cpu) {
    if (!ext_glVertexAttrib1sARB)
        kpanic("ext_glVertexAttrib1sARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1sARB)(ARG1, sARG2);
    GL_LOG ("glVertexAttrib1sARB GLuint index=%d, GLshort x=%d",ARG1,sARG2);
    }
}
void glcommon_glVertexAttrib1sNV(CPU* cpu) {
    if (!ext_glVertexAttrib1sNV)
        kpanic("ext_glVertexAttrib1sNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1sNV)(ARG1, sARG2);
    GL_LOG ("glVertexAttrib1sNV GLuint index=%d, GLshort x=%d",ARG1,sARG2);
    }
}
void glcommon_glVertexAttrib1sv(CPU* cpu) {
    if (!ext_glVertexAttrib1sv)
        kpanic("ext_glVertexAttrib1sv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1sv)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1sv GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1svARB(CPU* cpu) {
    if (!ext_glVertexAttrib1svARB)
        kpanic("ext_glVertexAttrib1svARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1svARB)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1svARB GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib1svNV(CPU* cpu) {
    if (!ext_glVertexAttrib1svNV)
        kpanic("ext_glVertexAttrib1svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib1svNV)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttrib1svNV GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2d(CPU* cpu) {
    if (!ext_glVertexAttrib2d)
        kpanic("ext_glVertexAttrib2d is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2d)(ARG1, dARG2, dARG3);
    GL_LOG ("glVertexAttrib2d GLuint index=%d, GLdouble x=%f, GLdouble y=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glVertexAttrib2dARB(CPU* cpu) {
    if (!ext_glVertexAttrib2dARB)
        kpanic("ext_glVertexAttrib2dARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2dARB)(ARG1, dARG2, dARG3);
    GL_LOG ("glVertexAttrib2dARB GLuint index=%d, GLdouble x=%f, GLdouble y=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glVertexAttrib2dNV(CPU* cpu) {
    if (!ext_glVertexAttrib2dNV)
        kpanic("ext_glVertexAttrib2dNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2dNV)(ARG1, dARG2, dARG3);
    GL_LOG ("glVertexAttrib2dNV GLuint index=%d, GLdouble x=%f, GLdouble y=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glVertexAttrib2dv(CPU* cpu) {
    if (!ext_glVertexAttrib2dv)
        kpanic("ext_glVertexAttrib2dv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2dv)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2dvARB(CPU* cpu) {
    if (!ext_glVertexAttrib2dvARB)
        kpanic("ext_glVertexAttrib2dvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2dvARB)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2dvARB GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2dvNV(CPU* cpu) {
    if (!ext_glVertexAttrib2dvNV)
        kpanic("ext_glVertexAttrib2dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2dvNV)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2dvNV GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2f(CPU* cpu) {
    if (!ext_glVertexAttrib2f)
        kpanic("ext_glVertexAttrib2f is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2f)(ARG1, fARG2, fARG3);
    GL_LOG ("glVertexAttrib2f GLuint index=%d, GLfloat x=%f, GLfloat y=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glVertexAttrib2fARB(CPU* cpu) {
    if (!ext_glVertexAttrib2fARB)
        kpanic("ext_glVertexAttrib2fARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2fARB)(ARG1, fARG2, fARG3);
    GL_LOG ("glVertexAttrib2fARB GLuint index=%d, GLfloat x=%f, GLfloat y=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glVertexAttrib2fNV(CPU* cpu) {
    if (!ext_glVertexAttrib2fNV)
        kpanic("ext_glVertexAttrib2fNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2fNV)(ARG1, fARG2, fARG3);
    GL_LOG ("glVertexAttrib2fNV GLuint index=%d, GLfloat x=%f, GLfloat y=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glVertexAttrib2fv(CPU* cpu) {
    if (!ext_glVertexAttrib2fv)
        kpanic("ext_glVertexAttrib2fv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2fv)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2fv GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2fvARB(CPU* cpu) {
    if (!ext_glVertexAttrib2fvARB)
        kpanic("ext_glVertexAttrib2fvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2fvARB)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2fvARB GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2fvNV(CPU* cpu) {
    if (!ext_glVertexAttrib2fvNV)
        kpanic("ext_glVertexAttrib2fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2fvNV)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2fvNV GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2hNV(CPU* cpu) {
    if (!ext_glVertexAttrib2hNV)
        kpanic("ext_glVertexAttrib2hNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2hNV)(ARG1, sARG2, sARG3);
    GL_LOG ("glVertexAttrib2hNV GLuint index=%d, GLhalfNV x=%d, GLhalfNV y=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glVertexAttrib2hvNV(CPU* cpu) {
    if (!ext_glVertexAttrib2hvNV)
        kpanic("ext_glVertexAttrib2hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2hvNV)(ARG1, (GLhalfNV*)marshalArray<GLhalfNV>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2hvNV GLuint index=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2s(CPU* cpu) {
    if (!ext_glVertexAttrib2s)
        kpanic("ext_glVertexAttrib2s is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2s)(ARG1, sARG2, sARG3);
    GL_LOG ("glVertexAttrib2s GLuint index=%d, GLshort x=%d, GLshort y=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glVertexAttrib2sARB(CPU* cpu) {
    if (!ext_glVertexAttrib2sARB)
        kpanic("ext_glVertexAttrib2sARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2sARB)(ARG1, sARG2, sARG3);
    GL_LOG ("glVertexAttrib2sARB GLuint index=%d, GLshort x=%d, GLshort y=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glVertexAttrib2sNV(CPU* cpu) {
    if (!ext_glVertexAttrib2sNV)
        kpanic("ext_glVertexAttrib2sNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2sNV)(ARG1, sARG2, sARG3);
    GL_LOG ("glVertexAttrib2sNV GLuint index=%d, GLshort x=%d, GLshort y=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glVertexAttrib2sv(CPU* cpu) {
    if (!ext_glVertexAttrib2sv)
        kpanic("ext_glVertexAttrib2sv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2sv)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2sv GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2svARB(CPU* cpu) {
    if (!ext_glVertexAttrib2svARB)
        kpanic("ext_glVertexAttrib2svARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2svARB)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2svARB GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib2svNV(CPU* cpu) {
    if (!ext_glVertexAttrib2svNV)
        kpanic("ext_glVertexAttrib2svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib2svNV)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttrib2svNV GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3d(CPU* cpu) {
    if (!ext_glVertexAttrib3d)
        kpanic("ext_glVertexAttrib3d is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3d)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glVertexAttrib3d GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glVertexAttrib3dARB(CPU* cpu) {
    if (!ext_glVertexAttrib3dARB)
        kpanic("ext_glVertexAttrib3dARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3dARB)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glVertexAttrib3dARB GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glVertexAttrib3dNV(CPU* cpu) {
    if (!ext_glVertexAttrib3dNV)
        kpanic("ext_glVertexAttrib3dNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3dNV)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glVertexAttrib3dNV GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glVertexAttrib3dv(CPU* cpu) {
    if (!ext_glVertexAttrib3dv)
        kpanic("ext_glVertexAttrib3dv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3dv)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3dvARB(CPU* cpu) {
    if (!ext_glVertexAttrib3dvARB)
        kpanic("ext_glVertexAttrib3dvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3dvARB)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3dvARB GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3dvNV(CPU* cpu) {
    if (!ext_glVertexAttrib3dvNV)
        kpanic("ext_glVertexAttrib3dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3dvNV)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3dvNV GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3f(CPU* cpu) {
    if (!ext_glVertexAttrib3f)
        kpanic("ext_glVertexAttrib3f is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3f)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glVertexAttrib3f GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glVertexAttrib3fARB(CPU* cpu) {
    if (!ext_glVertexAttrib3fARB)
        kpanic("ext_glVertexAttrib3fARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3fARB)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glVertexAttrib3fARB GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glVertexAttrib3fNV(CPU* cpu) {
    if (!ext_glVertexAttrib3fNV)
        kpanic("ext_glVertexAttrib3fNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3fNV)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glVertexAttrib3fNV GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glVertexAttrib3fv(CPU* cpu) {
    if (!ext_glVertexAttrib3fv)
        kpanic("ext_glVertexAttrib3fv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3fv)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3fv GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3fvARB(CPU* cpu) {
    if (!ext_glVertexAttrib3fvARB)
        kpanic("ext_glVertexAttrib3fvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3fvARB)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3fvARB GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3fvNV(CPU* cpu) {
    if (!ext_glVertexAttrib3fvNV)
        kpanic("ext_glVertexAttrib3fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3fvNV)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3fvNV GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3hNV(CPU* cpu) {
    if (!ext_glVertexAttrib3hNV)
        kpanic("ext_glVertexAttrib3hNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3hNV)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glVertexAttrib3hNV GLuint index=%d, GLhalfNV x=%d, GLhalfNV y=%d, GLhalfNV z=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glVertexAttrib3hvNV(CPU* cpu) {
    if (!ext_glVertexAttrib3hvNV)
        kpanic("ext_glVertexAttrib3hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3hvNV)(ARG1, (GLhalfNV*)marshalArray<GLhalfNV>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3hvNV GLuint index=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3s(CPU* cpu) {
    if (!ext_glVertexAttrib3s)
        kpanic("ext_glVertexAttrib3s is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3s)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glVertexAttrib3s GLuint index=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glVertexAttrib3sARB(CPU* cpu) {
    if (!ext_glVertexAttrib3sARB)
        kpanic("ext_glVertexAttrib3sARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3sARB)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glVertexAttrib3sARB GLuint index=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glVertexAttrib3sNV(CPU* cpu) {
    if (!ext_glVertexAttrib3sNV)
        kpanic("ext_glVertexAttrib3sNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3sNV)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glVertexAttrib3sNV GLuint index=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glVertexAttrib3sv(CPU* cpu) {
    if (!ext_glVertexAttrib3sv)
        kpanic("ext_glVertexAttrib3sv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3sv)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3sv GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3svARB(CPU* cpu) {
    if (!ext_glVertexAttrib3svARB)
        kpanic("ext_glVertexAttrib3svARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3svARB)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3svARB GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib3svNV(CPU* cpu) {
    if (!ext_glVertexAttrib3svNV)
        kpanic("ext_glVertexAttrib3svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib3svNV)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttrib3svNV GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4Nbv(CPU* cpu) {
    if (!ext_glVertexAttrib4Nbv)
        kpanic("ext_glVertexAttrib4Nbv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4Nbv)(ARG1, (GLbyte*)marshalArray<GLbyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4Nbv GLuint index=%d, const GLbyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4NbvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4NbvARB)
        kpanic("ext_glVertexAttrib4NbvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4NbvARB)(ARG1, (GLbyte*)marshalArray<GLbyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4NbvARB GLuint index=%d, const GLbyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4Niv(CPU* cpu) {
    if (!ext_glVertexAttrib4Niv)
        kpanic("ext_glVertexAttrib4Niv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4Niv)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4Niv GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4NivARB(CPU* cpu) {
    if (!ext_glVertexAttrib4NivARB)
        kpanic("ext_glVertexAttrib4NivARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4NivARB)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4NivARB GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4Nsv(CPU* cpu) {
    if (!ext_glVertexAttrib4Nsv)
        kpanic("ext_glVertexAttrib4Nsv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4Nsv)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4Nsv GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4NsvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4NsvARB)
        kpanic("ext_glVertexAttrib4NsvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4NsvARB)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4NsvARB GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4Nub(CPU* cpu) {
    if (!ext_glVertexAttrib4Nub)
        kpanic("ext_glVertexAttrib4Nub is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4Nub)(ARG1, bARG2, bARG3, bARG4, bARG5);
    GL_LOG ("glVertexAttrib4Nub GLuint index=%d, GLubyte x=%d, GLubyte y=%d, GLubyte z=%d, GLubyte w=%d",ARG1,bARG2,bARG3,bARG4,bARG5);
    }
}
void glcommon_glVertexAttrib4NubARB(CPU* cpu) {
    if (!ext_glVertexAttrib4NubARB)
        kpanic("ext_glVertexAttrib4NubARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4NubARB)(ARG1, bARG2, bARG3, bARG4, bARG5);
    GL_LOG ("glVertexAttrib4NubARB GLuint index=%d, GLubyte x=%d, GLubyte y=%d, GLubyte z=%d, GLubyte w=%d",ARG1,bARG2,bARG3,bARG4,bARG5);
    }
}
void glcommon_glVertexAttrib4Nubv(CPU* cpu) {
    if (!ext_glVertexAttrib4Nubv)
        kpanic("ext_glVertexAttrib4Nubv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4Nubv)(ARG1, (GLubyte*)marshalArray<GLubyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4Nubv GLuint index=%d, const GLubyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4NubvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4NubvARB)
        kpanic("ext_glVertexAttrib4NubvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4NubvARB)(ARG1, (GLubyte*)marshalArray<GLubyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4NubvARB GLuint index=%d, const GLubyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4Nuiv(CPU* cpu) {
    if (!ext_glVertexAttrib4Nuiv)
        kpanic("ext_glVertexAttrib4Nuiv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4Nuiv)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4Nuiv GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4NuivARB(CPU* cpu) {
    if (!ext_glVertexAttrib4NuivARB)
        kpanic("ext_glVertexAttrib4NuivARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4NuivARB)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4NuivARB GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4Nusv(CPU* cpu) {
    if (!ext_glVertexAttrib4Nusv)
        kpanic("ext_glVertexAttrib4Nusv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4Nusv)(ARG1, (GLushort*)marshalArray<GLushort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4Nusv GLuint index=%d, const GLushort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4NusvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4NusvARB)
        kpanic("ext_glVertexAttrib4NusvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4NusvARB)(ARG1, (GLushort*)marshalArray<GLushort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4NusvARB GLuint index=%d, const GLushort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4bv(CPU* cpu) {
    if (!ext_glVertexAttrib4bv)
        kpanic("ext_glVertexAttrib4bv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4bv)(ARG1, (GLbyte*)marshalArray<GLbyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4bv GLuint index=%d, const GLbyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4bvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4bvARB)
        kpanic("ext_glVertexAttrib4bvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4bvARB)(ARG1, (GLbyte*)marshalArray<GLbyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4bvARB GLuint index=%d, const GLbyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4d(CPU* cpu) {
    if (!ext_glVertexAttrib4d)
        kpanic("ext_glVertexAttrib4d is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4d)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glVertexAttrib4d GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glVertexAttrib4dARB(CPU* cpu) {
    if (!ext_glVertexAttrib4dARB)
        kpanic("ext_glVertexAttrib4dARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4dARB)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glVertexAttrib4dARB GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glVertexAttrib4dNV(CPU* cpu) {
    if (!ext_glVertexAttrib4dNV)
        kpanic("ext_glVertexAttrib4dNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4dNV)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glVertexAttrib4dNV GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glVertexAttrib4dv(CPU* cpu) {
    if (!ext_glVertexAttrib4dv)
        kpanic("ext_glVertexAttrib4dv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4dv)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4dvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4dvARB)
        kpanic("ext_glVertexAttrib4dvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4dvARB)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4dvARB GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4dvNV(CPU* cpu) {
    if (!ext_glVertexAttrib4dvNV)
        kpanic("ext_glVertexAttrib4dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4dvNV)(ARG1, marshalArray<GLdouble>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4dvNV GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4f(CPU* cpu) {
    if (!ext_glVertexAttrib4f)
        kpanic("ext_glVertexAttrib4f is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4f)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glVertexAttrib4f GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glVertexAttrib4fARB(CPU* cpu) {
    if (!ext_glVertexAttrib4fARB)
        kpanic("ext_glVertexAttrib4fARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4fARB)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glVertexAttrib4fARB GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glVertexAttrib4fNV(CPU* cpu) {
    if (!ext_glVertexAttrib4fNV)
        kpanic("ext_glVertexAttrib4fNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4fNV)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glVertexAttrib4fNV GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glVertexAttrib4fv(CPU* cpu) {
    if (!ext_glVertexAttrib4fv)
        kpanic("ext_glVertexAttrib4fv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4fv)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4fv GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4fvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4fvARB)
        kpanic("ext_glVertexAttrib4fvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4fvARB)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4fvARB GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4fvNV(CPU* cpu) {
    if (!ext_glVertexAttrib4fvNV)
        kpanic("ext_glVertexAttrib4fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4fvNV)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4fvNV GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4hNV(CPU* cpu) {
    if (!ext_glVertexAttrib4hNV)
        kpanic("ext_glVertexAttrib4hNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4hNV)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glVertexAttrib4hNV GLuint index=%d, GLhalfNV x=%d, GLhalfNV y=%d, GLhalfNV z=%d, GLhalfNV w=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glVertexAttrib4hvNV(CPU* cpu) {
    if (!ext_glVertexAttrib4hvNV)
        kpanic("ext_glVertexAttrib4hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4hvNV)(ARG1, (GLhalfNV*)marshalArray<GLhalfNV>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4hvNV GLuint index=%d, const GLhalfNV* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4iv(CPU* cpu) {
    if (!ext_glVertexAttrib4iv)
        kpanic("ext_glVertexAttrib4iv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4iv)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4iv GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4ivARB(CPU* cpu) {
    if (!ext_glVertexAttrib4ivARB)
        kpanic("ext_glVertexAttrib4ivARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4ivARB)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4ivARB GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4s(CPU* cpu) {
    if (!ext_glVertexAttrib4s)
        kpanic("ext_glVertexAttrib4s is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4s)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glVertexAttrib4s GLuint index=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d, GLshort w=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glVertexAttrib4sARB(CPU* cpu) {
    if (!ext_glVertexAttrib4sARB)
        kpanic("ext_glVertexAttrib4sARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4sARB)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glVertexAttrib4sARB GLuint index=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d, GLshort w=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glVertexAttrib4sNV(CPU* cpu) {
    if (!ext_glVertexAttrib4sNV)
        kpanic("ext_glVertexAttrib4sNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4sNV)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glVertexAttrib4sNV GLuint index=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d, GLshort w=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glVertexAttrib4sv(CPU* cpu) {
    if (!ext_glVertexAttrib4sv)
        kpanic("ext_glVertexAttrib4sv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4sv)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4sv GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4svARB(CPU* cpu) {
    if (!ext_glVertexAttrib4svARB)
        kpanic("ext_glVertexAttrib4svARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4svARB)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4svARB GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4svNV(CPU* cpu) {
    if (!ext_glVertexAttrib4svNV)
        kpanic("ext_glVertexAttrib4svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4svNV)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4svNV GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4ubNV(CPU* cpu) {
    if (!ext_glVertexAttrib4ubNV)
        kpanic("ext_glVertexAttrib4ubNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4ubNV)(ARG1, bARG2, bARG3, bARG4, bARG5);
    GL_LOG ("glVertexAttrib4ubNV GLuint index=%d, GLubyte x=%d, GLubyte y=%d, GLubyte z=%d, GLubyte w=%d",ARG1,bARG2,bARG3,bARG4,bARG5);
    }
}
void glcommon_glVertexAttrib4ubv(CPU* cpu) {
    if (!ext_glVertexAttrib4ubv)
        kpanic("ext_glVertexAttrib4ubv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4ubv)(ARG1, (GLubyte*)marshalArray<GLubyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4ubv GLuint index=%d, const GLubyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4ubvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4ubvARB)
        kpanic("ext_glVertexAttrib4ubvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4ubvARB)(ARG1, (GLubyte*)marshalArray<GLubyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4ubvARB GLuint index=%d, const GLubyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4ubvNV(CPU* cpu) {
    if (!ext_glVertexAttrib4ubvNV)
        kpanic("ext_glVertexAttrib4ubvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4ubvNV)(ARG1, (GLubyte*)marshalArray<GLubyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4ubvNV GLuint index=%d, const GLubyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4uiv(CPU* cpu) {
    if (!ext_glVertexAttrib4uiv)
        kpanic("ext_glVertexAttrib4uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4uiv)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4uiv GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4uivARB(CPU* cpu) {
    if (!ext_glVertexAttrib4uivARB)
        kpanic("ext_glVertexAttrib4uivARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4uivARB)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4uivARB GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4usv(CPU* cpu) {
    if (!ext_glVertexAttrib4usv)
        kpanic("ext_glVertexAttrib4usv is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4usv)(ARG1, (GLushort*)marshalArray<GLushort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4usv GLuint index=%d, const GLushort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttrib4usvARB(CPU* cpu) {
    if (!ext_glVertexAttrib4usvARB)
        kpanic("ext_glVertexAttrib4usvARB is NULL");
    {
    GL_FUNC(ext_glVertexAttrib4usvARB)(ARG1, (GLushort*)marshalArray<GLushort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttrib4usvARB GLuint index=%d, const GLushort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribArrayObjectATI(CPU* cpu) {
    if (!ext_glVertexAttribArrayObjectATI)
        kpanic("ext_glVertexAttribArrayObjectATI is NULL");
    {
    GL_FUNC(ext_glVertexAttribArrayObjectATI)(ARG1, ARG2, ARG3, bARG4, ARG5, ARG6, ARG7);
    GL_LOG ("glVertexAttribArrayObjectATI GLuint index=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLsizei stride=%d, GLuint buffer=%d, GLuint offset=%d",ARG1,ARG2,ARG3,bARG4,ARG5,ARG6,ARG7);
    }
}
void glcommon_glVertexAttribBinding(CPU* cpu) {
    if (!ext_glVertexAttribBinding)
        kpanic("ext_glVertexAttribBinding is NULL");
    {
    GL_FUNC(ext_glVertexAttribBinding)(ARG1, ARG2);
    GL_LOG ("glVertexAttribBinding GLuint attribindex=%d, GLuint bindingindex=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribDivisor(CPU* cpu) {
    if (!ext_glVertexAttribDivisor)
        kpanic("ext_glVertexAttribDivisor is NULL");
    {
    GL_FUNC(ext_glVertexAttribDivisor)(ARG1, ARG2);
    GL_LOG ("glVertexAttribDivisor GLuint index=%d, GLuint divisor=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribDivisorARB(CPU* cpu) {
    if (!ext_glVertexAttribDivisorARB)
        kpanic("ext_glVertexAttribDivisorARB is NULL");
    {
    GL_FUNC(ext_glVertexAttribDivisorARB)(ARG1, ARG2);
    GL_LOG ("glVertexAttribDivisorARB GLuint index=%d, GLuint divisor=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribFormat(CPU* cpu) {
    if (!ext_glVertexAttribFormat)
        kpanic("ext_glVertexAttribFormat is NULL");
    {
    GL_FUNC(ext_glVertexAttribFormat)(ARG1, ARG2, ARG3, bARG4, ARG5);
    GL_LOG ("glVertexAttribFormat GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glVertexAttribFormatNV(CPU* cpu) {
    if (!ext_glVertexAttribFormatNV)
        kpanic("ext_glVertexAttribFormatNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribFormatNV)(ARG1, ARG2, ARG3, bARG4, ARG5);
    GL_LOG ("glVertexAttribFormatNV GLuint index=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,bARG4,ARG5);
    }
}
void glcommon_glVertexAttribI1i(CPU* cpu) {
    if (!ext_glVertexAttribI1i)
        kpanic("ext_glVertexAttribI1i is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1i)(ARG1, ARG2);
    GL_LOG ("glVertexAttribI1i GLuint index=%d, GLint x=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI1iEXT(CPU* cpu) {
    if (!ext_glVertexAttribI1iEXT)
        kpanic("ext_glVertexAttribI1iEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1iEXT)(ARG1, ARG2);
    GL_LOG ("glVertexAttribI1iEXT GLuint index=%d, GLint x=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI1iv(CPU* cpu) {
    if (!ext_glVertexAttribI1iv)
        kpanic("ext_glVertexAttribI1iv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1iv)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttribI1iv GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI1ivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI1ivEXT)
        kpanic("ext_glVertexAttribI1ivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1ivEXT)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttribI1ivEXT GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI1ui(CPU* cpu) {
    if (!ext_glVertexAttribI1ui)
        kpanic("ext_glVertexAttribI1ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1ui)(ARG1, ARG2);
    GL_LOG ("glVertexAttribI1ui GLuint index=%d, GLuint x=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI1uiEXT(CPU* cpu) {
    if (!ext_glVertexAttribI1uiEXT)
        kpanic("ext_glVertexAttribI1uiEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1uiEXT)(ARG1, ARG2);
    GL_LOG ("glVertexAttribI1uiEXT GLuint index=%d, GLuint x=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI1uiv(CPU* cpu) {
    if (!ext_glVertexAttribI1uiv)
        kpanic("ext_glVertexAttribI1uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1uiv)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttribI1uiv GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI1uivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI1uivEXT)
        kpanic("ext_glVertexAttribI1uivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI1uivEXT)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 1));
    GL_LOG ("glVertexAttribI1uivEXT GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI2i(CPU* cpu) {
    if (!ext_glVertexAttribI2i)
        kpanic("ext_glVertexAttribI2i is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2i)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexAttribI2i GLuint index=%d, GLint x=%d, GLint y=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribI2iEXT(CPU* cpu) {
    if (!ext_glVertexAttribI2iEXT)
        kpanic("ext_glVertexAttribI2iEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2iEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexAttribI2iEXT GLuint index=%d, GLint x=%d, GLint y=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribI2iv(CPU* cpu) {
    if (!ext_glVertexAttribI2iv)
        kpanic("ext_glVertexAttribI2iv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2iv)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttribI2iv GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI2ivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI2ivEXT)
        kpanic("ext_glVertexAttribI2ivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2ivEXT)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttribI2ivEXT GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI2ui(CPU* cpu) {
    if (!ext_glVertexAttribI2ui)
        kpanic("ext_glVertexAttribI2ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2ui)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexAttribI2ui GLuint index=%d, GLuint x=%d, GLuint y=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribI2uiEXT(CPU* cpu) {
    if (!ext_glVertexAttribI2uiEXT)
        kpanic("ext_glVertexAttribI2uiEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2uiEXT)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexAttribI2uiEXT GLuint index=%d, GLuint x=%d, GLuint y=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribI2uiv(CPU* cpu) {
    if (!ext_glVertexAttribI2uiv)
        kpanic("ext_glVertexAttribI2uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2uiv)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttribI2uiv GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI2uivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI2uivEXT)
        kpanic("ext_glVertexAttribI2uivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI2uivEXT)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 2));
    GL_LOG ("glVertexAttribI2uivEXT GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI3i(CPU* cpu) {
    if (!ext_glVertexAttribI3i)
        kpanic("ext_glVertexAttribI3i is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3i)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribI3i GLuint index=%d, GLint x=%d, GLint y=%d, GLint z=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribI3iEXT(CPU* cpu) {
    if (!ext_glVertexAttribI3iEXT)
        kpanic("ext_glVertexAttribI3iEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3iEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribI3iEXT GLuint index=%d, GLint x=%d, GLint y=%d, GLint z=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribI3iv(CPU* cpu) {
    if (!ext_glVertexAttribI3iv)
        kpanic("ext_glVertexAttribI3iv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3iv)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttribI3iv GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI3ivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI3ivEXT)
        kpanic("ext_glVertexAttribI3ivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3ivEXT)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttribI3ivEXT GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI3ui(CPU* cpu) {
    if (!ext_glVertexAttribI3ui)
        kpanic("ext_glVertexAttribI3ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3ui)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribI3ui GLuint index=%d, GLuint x=%d, GLuint y=%d, GLuint z=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribI3uiEXT(CPU* cpu) {
    if (!ext_glVertexAttribI3uiEXT)
        kpanic("ext_glVertexAttribI3uiEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3uiEXT)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribI3uiEXT GLuint index=%d, GLuint x=%d, GLuint y=%d, GLuint z=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribI3uiv(CPU* cpu) {
    if (!ext_glVertexAttribI3uiv)
        kpanic("ext_glVertexAttribI3uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3uiv)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttribI3uiv GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI3uivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI3uivEXT)
        kpanic("ext_glVertexAttribI3uivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI3uivEXT)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 3));
    GL_LOG ("glVertexAttribI3uivEXT GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4bv(CPU* cpu) {
    if (!ext_glVertexAttribI4bv)
        kpanic("ext_glVertexAttribI4bv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4bv)(ARG1, (GLbyte*)marshalArray<GLbyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4bv GLuint index=%d, const GLbyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4bvEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4bvEXT)
        kpanic("ext_glVertexAttribI4bvEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4bvEXT)(ARG1, (GLbyte*)marshalArray<GLbyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4bvEXT GLuint index=%d, const GLbyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4i(CPU* cpu) {
    if (!ext_glVertexAttribI4i)
        kpanic("ext_glVertexAttribI4i is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4i)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexAttribI4i GLuint index=%d, GLint x=%d, GLint y=%d, GLint z=%d, GLint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribI4iEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4iEXT)
        kpanic("ext_glVertexAttribI4iEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4iEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexAttribI4iEXT GLuint index=%d, GLint x=%d, GLint y=%d, GLint z=%d, GLint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribI4iv(CPU* cpu) {
    if (!ext_glVertexAttribI4iv)
        kpanic("ext_glVertexAttribI4iv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4iv)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4iv GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4ivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4ivEXT)
        kpanic("ext_glVertexAttribI4ivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4ivEXT)(ARG1, (GLint*)marshalArray<GLint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4ivEXT GLuint index=%d, const GLint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4sv(CPU* cpu) {
    if (!ext_glVertexAttribI4sv)
        kpanic("ext_glVertexAttribI4sv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4sv)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4sv GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4svEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4svEXT)
        kpanic("ext_glVertexAttribI4svEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4svEXT)(ARG1, (GLshort*)marshalArray<GLshort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4svEXT GLuint index=%d, const GLshort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4ubv(CPU* cpu) {
    if (!ext_glVertexAttribI4ubv)
        kpanic("ext_glVertexAttribI4ubv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4ubv)(ARG1, (GLubyte*)marshalArray<GLubyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4ubv GLuint index=%d, const GLubyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4ubvEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4ubvEXT)
        kpanic("ext_glVertexAttribI4ubvEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4ubvEXT)(ARG1, (GLubyte*)marshalArray<GLubyte>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4ubvEXT GLuint index=%d, const GLubyte* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4ui(CPU* cpu) {
    if (!ext_glVertexAttribI4ui)
        kpanic("ext_glVertexAttribI4ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4ui)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexAttribI4ui GLuint index=%d, GLuint x=%d, GLuint y=%d, GLuint z=%d, GLuint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribI4uiEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4uiEXT)
        kpanic("ext_glVertexAttribI4uiEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4uiEXT)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexAttribI4uiEXT GLuint index=%d, GLuint x=%d, GLuint y=%d, GLuint z=%d, GLuint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribI4uiv(CPU* cpu) {
    if (!ext_glVertexAttribI4uiv)
        kpanic("ext_glVertexAttribI4uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4uiv)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4uiv GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4uivEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4uivEXT)
        kpanic("ext_glVertexAttribI4uivEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4uivEXT)(ARG1, (GLuint*)marshalArray<GLuint>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4uivEXT GLuint index=%d, const GLuint* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4usv(CPU* cpu) {
    if (!ext_glVertexAttribI4usv)
        kpanic("ext_glVertexAttribI4usv is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4usv)(ARG1, (GLushort*)marshalArray<GLushort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4usv GLuint index=%d, const GLushort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribI4usvEXT(CPU* cpu) {
    if (!ext_glVertexAttribI4usvEXT)
        kpanic("ext_glVertexAttribI4usvEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribI4usvEXT)(ARG1, (GLushort*)marshalArray<GLushort>(cpu, ARG2, 4));
    GL_LOG ("glVertexAttribI4usvEXT GLuint index=%d, const GLushort* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribIFormat(CPU* cpu) {
    if (!ext_glVertexAttribIFormat)
        kpanic("ext_glVertexAttribIFormat is NULL");
    {
    GL_FUNC(ext_glVertexAttribIFormat)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribIFormat GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribIFormatNV(CPU* cpu) {
    if (!ext_glVertexAttribIFormatNV)
        kpanic("ext_glVertexAttribIFormatNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribIFormatNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribIFormatNV GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribIPointer(CPU* cpu) {
    if (!ext_glVertexAttribIPointer)
        kpanic("ext_glVertexAttribIPointer is NULL");
    {
    // even though ARG5 is const void*, it is actually an offset and thus does not need to be marshalled
    GL_FUNC(ext_glVertexAttribIPointer)(ARG1, ARG2, ARG3, ARG4, (const void*)pARG5);
    GL_LOG ("glVertexAttribIPointer GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribIPointerEXT(CPU* cpu) {
    if (!ext_glVertexAttribIPointerEXT)
        kpanic("ext_glVertexAttribIPointerEXT is NULL");
    {
        // even though ARG5 is const void*, it is actually an offset and thus does not need to be marshalled
    GL_FUNC(ext_glVertexAttribIPointerEXT)(ARG1, ARG2, ARG3, ARG4, (const void*)pARG5);
    GL_LOG ("glVertexAttribIPointerEXT GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribL1d(CPU* cpu) {
    if (!ext_glVertexAttribL1d)
        kpanic("ext_glVertexAttribL1d is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1d)(ARG1, dARG2);
    GL_LOG ("glVertexAttribL1d GLuint index=%d, GLdouble x=%f",ARG1,dARG2);
    }
}
void glcommon_glVertexAttribL1dEXT(CPU* cpu) {
    if (!ext_glVertexAttribL1dEXT)
        kpanic("ext_glVertexAttribL1dEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1dEXT)(ARG1, dARG2);
    GL_LOG ("glVertexAttribL1dEXT GLuint index=%d, GLdouble x=%f",ARG1,dARG2);
    }
}
void glcommon_glVertexAttribL1dv(CPU* cpu) {
    if (!ext_glVertexAttribL1dv)
        kpanic("ext_glVertexAttribL1dv is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL1dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL1dvEXT(CPU* cpu) {
    if (!ext_glVertexAttribL1dvEXT)
        kpanic("ext_glVertexAttribL1dvEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1dvEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL1dvEXT GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL1i64NV(CPU* cpu) {
    if (!ext_glVertexAttribL1i64NV)
        kpanic("ext_glVertexAttribL1i64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1i64NV)(ARG1, llARG2);
    GL_LOG ("glVertexAttribL1i64NV GLuint index=%d, GLint64EXT x=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glVertexAttribL1i64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL1i64vNV)
        kpanic("ext_glVertexAttribL1i64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1i64vNV)(ARG1, (GLint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL1i64vNV GLuint index=%d, const GLint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL1ui64ARB(CPU* cpu) {
    if (!ext_glVertexAttribL1ui64ARB)
        kpanic("ext_glVertexAttribL1ui64ARB is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1ui64ARB)(ARG1, llARG2);
    GL_LOG ("glVertexAttribL1ui64ARB GLuint index=%d, GLuint64EXT x=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glVertexAttribL1ui64NV(CPU* cpu) {
    if (!ext_glVertexAttribL1ui64NV)
        kpanic("ext_glVertexAttribL1ui64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1ui64NV)(ARG1, llARG2);
    GL_LOG ("glVertexAttribL1ui64NV GLuint index=%d, GLuint64EXT x=" PRIu64 "",ARG1,llARG2);
    }
}
void glcommon_glVertexAttribL1ui64vARB(CPU* cpu) {
    if (!ext_glVertexAttribL1ui64vARB)
        kpanic("ext_glVertexAttribL1ui64vARB is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1ui64vARB)(ARG1, (GLuint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL1ui64vARB GLuint index=%d, const GLuint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL1ui64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL1ui64vNV)
        kpanic("ext_glVertexAttribL1ui64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL1ui64vNV)(ARG1, (GLuint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL1ui64vNV GLuint index=%d, const GLuint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL2d(CPU* cpu) {
    if (!ext_glVertexAttribL2d)
        kpanic("ext_glVertexAttribL2d is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2d)(ARG1, dARG2, dARG3);
    GL_LOG ("glVertexAttribL2d GLuint index=%d, GLdouble x=%f, GLdouble y=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glVertexAttribL2dEXT(CPU* cpu) {
    if (!ext_glVertexAttribL2dEXT)
        kpanic("ext_glVertexAttribL2dEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2dEXT)(ARG1, dARG2, dARG3);
    GL_LOG ("glVertexAttribL2dEXT GLuint index=%d, GLdouble x=%f, GLdouble y=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glVertexAttribL2dv(CPU* cpu) {
    if (!ext_glVertexAttribL2dv)
        kpanic("ext_glVertexAttribL2dv is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL2dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL2dvEXT(CPU* cpu) {
    if (!ext_glVertexAttribL2dvEXT)
        kpanic("ext_glVertexAttribL2dvEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2dvEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL2dvEXT GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL2i64NV(CPU* cpu) {
    if (!ext_glVertexAttribL2i64NV)
        kpanic("ext_glVertexAttribL2i64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2i64NV)(ARG1, llARG2, llARG3);
    GL_LOG ("glVertexAttribL2i64NV GLuint index=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 "",ARG1,llARG2,llARG3);
    }
}
void glcommon_glVertexAttribL2i64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL2i64vNV)
        kpanic("ext_glVertexAttribL2i64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2i64vNV)(ARG1, (GLint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL2i64vNV GLuint index=%d, const GLint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL2ui64NV(CPU* cpu) {
    if (!ext_glVertexAttribL2ui64NV)
        kpanic("ext_glVertexAttribL2ui64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2ui64NV)(ARG1, llARG2, llARG3);
    GL_LOG ("glVertexAttribL2ui64NV GLuint index=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 "",ARG1,llARG2,llARG3);
    }
}
void glcommon_glVertexAttribL2ui64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL2ui64vNV)
        kpanic("ext_glVertexAttribL2ui64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL2ui64vNV)(ARG1, (GLuint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL2ui64vNV GLuint index=%d, const GLuint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL3d(CPU* cpu) {
    if (!ext_glVertexAttribL3d)
        kpanic("ext_glVertexAttribL3d is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3d)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glVertexAttribL3d GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glVertexAttribL3dEXT(CPU* cpu) {
    if (!ext_glVertexAttribL3dEXT)
        kpanic("ext_glVertexAttribL3dEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3dEXT)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glVertexAttribL3dEXT GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glVertexAttribL3dv(CPU* cpu) {
    if (!ext_glVertexAttribL3dv)
        kpanic("ext_glVertexAttribL3dv is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL3dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL3dvEXT(CPU* cpu) {
    if (!ext_glVertexAttribL3dvEXT)
        kpanic("ext_glVertexAttribL3dvEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3dvEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL3dvEXT GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL3i64NV(CPU* cpu) {
    if (!ext_glVertexAttribL3i64NV)
        kpanic("ext_glVertexAttribL3i64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3i64NV)(ARG1, llARG2, llARG3, llARG4);
    GL_LOG ("glVertexAttribL3i64NV GLuint index=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 ", GLint64EXT z=" PRIu64 "",ARG1,llARG2,llARG3,llARG4);
    }
}
void glcommon_glVertexAttribL3i64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL3i64vNV)
        kpanic("ext_glVertexAttribL3i64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3i64vNV)(ARG1, (GLint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL3i64vNV GLuint index=%d, const GLint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL3ui64NV(CPU* cpu) {
    if (!ext_glVertexAttribL3ui64NV)
        kpanic("ext_glVertexAttribL3ui64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3ui64NV)(ARG1, llARG2, llARG3, llARG4);
    GL_LOG ("glVertexAttribL3ui64NV GLuint index=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 ", GLuint64EXT z=" PRIu64 "",ARG1,llARG2,llARG3,llARG4);
    }
}
void glcommon_glVertexAttribL3ui64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL3ui64vNV)
        kpanic("ext_glVertexAttribL3ui64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL3ui64vNV)(ARG1, (GLuint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL3ui64vNV GLuint index=%d, const GLuint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL4d(CPU* cpu) {
    if (!ext_glVertexAttribL4d)
        kpanic("ext_glVertexAttribL4d is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4d)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glVertexAttribL4d GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glVertexAttribL4dEXT(CPU* cpu) {
    if (!ext_glVertexAttribL4dEXT)
        kpanic("ext_glVertexAttribL4dEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4dEXT)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glVertexAttribL4dEXT GLuint index=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glVertexAttribL4dv(CPU* cpu) {
    if (!ext_glVertexAttribL4dv)
        kpanic("ext_glVertexAttribL4dv is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4dv)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL4dv GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL4dvEXT(CPU* cpu) {
    if (!ext_glVertexAttribL4dvEXT)
        kpanic("ext_glVertexAttribL4dvEXT is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4dvEXT)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL4dvEXT GLuint index=%d, const GLdouble* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL4i64NV(CPU* cpu) {
    if (!ext_glVertexAttribL4i64NV)
        kpanic("ext_glVertexAttribL4i64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4i64NV)(ARG1, llARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glVertexAttribL4i64NV GLuint index=%d, GLint64EXT x=" PRIu64 ", GLint64EXT y=" PRIu64 ", GLint64EXT z=" PRIu64 ", GLint64EXT w=" PRIu64 "",ARG1,llARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glVertexAttribL4i64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL4i64vNV)
        kpanic("ext_glVertexAttribL4i64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4i64vNV)(ARG1, (GLint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL4i64vNV GLuint index=%d, const GLint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribL4ui64NV(CPU* cpu) {
    if (!ext_glVertexAttribL4ui64NV)
        kpanic("ext_glVertexAttribL4ui64NV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4ui64NV)(ARG1, llARG2, llARG3, llARG4, llARG5);
    GL_LOG ("glVertexAttribL4ui64NV GLuint index=%d, GLuint64EXT x=" PRIu64 ", GLuint64EXT y=" PRIu64 ", GLuint64EXT z=" PRIu64 ", GLuint64EXT w=" PRIu64 "",ARG1,llARG2,llARG3,llARG4,llARG5);
    }
}
void glcommon_glVertexAttribL4ui64vNV(CPU* cpu) {
    if (!ext_glVertexAttribL4ui64vNV)
        kpanic("ext_glVertexAttribL4ui64vNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribL4ui64vNV)(ARG1, (GLuint64EXT*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexAttribL4ui64vNV GLuint index=%d, const GLuint64EXT* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexAttribLFormat(CPU* cpu) {
    if (!ext_glVertexAttribLFormat)
        kpanic("ext_glVertexAttribLFormat is NULL");
    {
    GL_FUNC(ext_glVertexAttribLFormat)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribLFormat GLuint attribindex=%d, GLint size=%d, GLenum type=%d, GLuint relativeoffset=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribLFormatNV(CPU* cpu) {
    if (!ext_glVertexAttribLFormatNV)
        kpanic("ext_glVertexAttribLFormatNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribLFormatNV)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexAttribLFormatNV GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexAttribLPointer(CPU* cpu) {
    if (!ext_glVertexAttribLPointer)
        kpanic("ext_glVertexAttribLPointer is NULL");
    {
    // even though ARG5 is const void*, it is actually an offset and thus does not need to be marshalled
    GL_FUNC(ext_glVertexAttribLPointer)(ARG1, ARG2, ARG3, ARG4, (const void*)pARG5);
    GL_LOG ("glVertexAttribLPointer GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribLPointerEXT(CPU* cpu) {
    if (!ext_glVertexAttribLPointerEXT)
        kpanic("ext_glVertexAttribLPointerEXT is NULL");
    {
    // even though ARG5 is const void*, it is actually an offset and thus does not need to be marshalled
    GL_FUNC(ext_glVertexAttribLPointerEXT)(ARG1, ARG2, ARG3, ARG4, (const void*)pARG5);
    GL_LOG ("glVertexAttribLPointerEXT GLuint index=%d, GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribP1ui(CPU* cpu) {
    if (!ext_glVertexAttribP1ui)
        kpanic("ext_glVertexAttribP1ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribP1ui)(ARG1, ARG2, bARG3, ARG4);
    GL_LOG ("glVertexAttribP1ui GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, GLuint value=%d",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribP1uiv(CPU* cpu) {
    if (!ext_glVertexAttribP1uiv)
        kpanic("ext_glVertexAttribP1uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribP1uiv)(ARG1, ARG2, bARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVertexAttribP1uiv GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, const GLuint* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribP2ui(CPU* cpu) {
    if (!ext_glVertexAttribP2ui)
        kpanic("ext_glVertexAttribP2ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribP2ui)(ARG1, ARG2, bARG3, ARG4);
    GL_LOG ("glVertexAttribP2ui GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, GLuint value=%d",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribP2uiv(CPU* cpu) {
    if (!ext_glVertexAttribP2uiv)
        kpanic("ext_glVertexAttribP2uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribP2uiv)(ARG1, ARG2, bARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVertexAttribP2uiv GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, const GLuint* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribP3ui(CPU* cpu) {
    if (!ext_glVertexAttribP3ui)
        kpanic("ext_glVertexAttribP3ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribP3ui)(ARG1, ARG2, bARG3, ARG4);
    GL_LOG ("glVertexAttribP3ui GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, GLuint value=%d",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribP3uiv(CPU* cpu) {
    if (!ext_glVertexAttribP3uiv)
        kpanic("ext_glVertexAttribP3uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribP3uiv)(ARG1, ARG2, bARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVertexAttribP3uiv GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, const GLuint* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribP4ui(CPU* cpu) {
    if (!ext_glVertexAttribP4ui)
        kpanic("ext_glVertexAttribP4ui is NULL");
    {
    GL_FUNC(ext_glVertexAttribP4ui)(ARG1, ARG2, bARG3, ARG4);
    GL_LOG ("glVertexAttribP4ui GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, GLuint value=%d",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribP4uiv(CPU* cpu) {
    if (!ext_glVertexAttribP4uiv)
        kpanic("ext_glVertexAttribP4uiv is NULL");
    {
    GL_FUNC(ext_glVertexAttribP4uiv)(ARG1, ARG2, bARG3, (GLuint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVertexAttribP4uiv GLuint index=%d, GLenum type=%d, GLboolean normalized=%d, const GLuint* value=%.08x",ARG1,ARG2,bARG3,ARG4);
    }
}
void glcommon_glVertexAttribParameteriAMD(CPU* cpu) {
    if (!ext_glVertexAttribParameteriAMD)
        kpanic("ext_glVertexAttribParameteriAMD is NULL");
    {
    GL_FUNC(ext_glVertexAttribParameteriAMD)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexAttribParameteriAMD GLuint index=%d, GLenum pname=%d, GLint param=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribPointer(CPU* cpu) {
    if (!ext_glVertexAttribPointer)
        kpanic("ext_glVertexAttribPointer is NULL");
    {
    // even though ARG6 is const void*, it is actually an offset and thus does not need to be marshalled
    GL_FUNC(ext_glVertexAttribPointer)(ARG1, ARG2, ARG3, bARG4, ARG5, (const void*)pARG6);
    GL_LOG ("glVertexAttribPointer GLuint index=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5,ARG6);
    }
}
void glcommon_glVertexAttribPointerARB(CPU* cpu) {
    if (!ext_glVertexAttribPointerARB)
        kpanic("ext_glVertexAttribPointerARB is NULL");
    {
    // even though ARG6 is const void*, it is actually an offset and thus does not need to be marshalled
    GL_FUNC(ext_glVertexAttribPointerARB)(ARG1, ARG2, ARG3, bARG4, ARG5, (const void*)pARG6);
    GL_LOG ("glVertexAttribPointerARB GLuint index=%d, GLint size=%d, GLenum type=%d, GLboolean normalized=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,bARG4,ARG5,ARG6);
    }
}
void glcommon_glVertexAttribPointerNV(CPU* cpu) {
    if (!ext_glVertexAttribPointerNV)
        kpanic("ext_glVertexAttribPointerNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribPointerNV)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glVertexAttribPointerNV GLuint index=%d, GLint fsize=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexAttribs1dvNV(CPU* cpu) {
    if (!ext_glVertexAttribs1dvNV)
        kpanic("ext_glVertexAttribs1dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs1dvNV)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs1dvNV GLuint index=%d, GLsizei count=%d, const GLdouble* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs1fvNV(CPU* cpu) {
    if (!ext_glVertexAttribs1fvNV)
        kpanic("ext_glVertexAttribs1fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs1fvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs1fvNV GLuint index=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs1hvNV(CPU* cpu) {
    if (!ext_glVertexAttribs1hvNV)
        kpanic("ext_glVertexAttribs1hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs1hvNV)(ARG1, ARG2, (GLhalfNV*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs1hvNV GLuint index=%d, GLsizei n=%d, const GLhalfNV* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs1svNV(CPU* cpu) {
    if (!ext_glVertexAttribs1svNV)
        kpanic("ext_glVertexAttribs1svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs1svNV)(ARG1, ARG2, (GLshort*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs1svNV GLuint index=%d, GLsizei count=%d, const GLshort* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs2dvNV(CPU* cpu) {
    if (!ext_glVertexAttribs2dvNV)
        kpanic("ext_glVertexAttribs2dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs2dvNV)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs2dvNV GLuint index=%d, GLsizei count=%d, const GLdouble* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs2fvNV(CPU* cpu) {
    if (!ext_glVertexAttribs2fvNV)
        kpanic("ext_glVertexAttribs2fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs2fvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs2fvNV GLuint index=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs2hvNV(CPU* cpu) {
    if (!ext_glVertexAttribs2hvNV)
        kpanic("ext_glVertexAttribs2hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs2hvNV)(ARG1, ARG2, (GLhalfNV*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs2hvNV GLuint index=%d, GLsizei n=%d, const GLhalfNV* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs2svNV(CPU* cpu) {
    if (!ext_glVertexAttribs2svNV)
        kpanic("ext_glVertexAttribs2svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs2svNV)(ARG1, ARG2, (GLshort*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs2svNV GLuint index=%d, GLsizei count=%d, const GLshort* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs3dvNV(CPU* cpu) {
    if (!ext_glVertexAttribs3dvNV)
        kpanic("ext_glVertexAttribs3dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs3dvNV)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs3dvNV GLuint index=%d, GLsizei count=%d, const GLdouble* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs3fvNV(CPU* cpu) {
    if (!ext_glVertexAttribs3fvNV)
        kpanic("ext_glVertexAttribs3fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs3fvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs3fvNV GLuint index=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs3hvNV(CPU* cpu) {
    if (!ext_glVertexAttribs3hvNV)
        kpanic("ext_glVertexAttribs3hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs3hvNV)(ARG1, ARG2, (GLhalfNV*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs3hvNV GLuint index=%d, GLsizei n=%d, const GLhalfNV* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs3svNV(CPU* cpu) {
    if (!ext_glVertexAttribs3svNV)
        kpanic("ext_glVertexAttribs3svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs3svNV)(ARG1, ARG2, (GLshort*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs3svNV GLuint index=%d, GLsizei count=%d, const GLshort* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs4dvNV(CPU* cpu) {
    if (!ext_glVertexAttribs4dvNV)
        kpanic("ext_glVertexAttribs4dvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs4dvNV)(ARG1, ARG2, (GLdouble*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs4dvNV GLuint index=%d, GLsizei count=%d, const GLdouble* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs4fvNV(CPU* cpu) {
    if (!ext_glVertexAttribs4fvNV)
        kpanic("ext_glVertexAttribs4fvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs4fvNV)(ARG1, ARG2, (GLfloat*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs4fvNV GLuint index=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs4hvNV(CPU* cpu) {
    if (!ext_glVertexAttribs4hvNV)
        kpanic("ext_glVertexAttribs4hvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs4hvNV)(ARG1, ARG2, (GLhalfNV*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs4hvNV GLuint index=%d, GLsizei n=%d, const GLhalfNV* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs4svNV(CPU* cpu) {
    if (!ext_glVertexAttribs4svNV)
        kpanic("ext_glVertexAttribs4svNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs4svNV)(ARG1, ARG2, (GLshort*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs4svNV GLuint index=%d, GLsizei count=%d, const GLshort* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexAttribs4ubvNV(CPU* cpu) {
    if (!ext_glVertexAttribs4ubvNV)
        kpanic("ext_glVertexAttribs4ubvNV is NULL");
    {
    GL_FUNC(ext_glVertexAttribs4ubvNV)(ARG1, ARG2, (GLubyte*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVertexAttribs4ubvNV GLuint index=%d, GLsizei count=%d, const GLubyte* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexBindingDivisor(CPU* cpu) {
    if (!ext_glVertexBindingDivisor)
        kpanic("ext_glVertexBindingDivisor is NULL");
    {
    GL_FUNC(ext_glVertexBindingDivisor)(ARG1, ARG2);
    GL_LOG ("glVertexBindingDivisor GLuint bindingindex=%d, GLuint divisor=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexBlendARB(CPU* cpu) {
    if (!ext_glVertexBlendARB)
        kpanic("ext_glVertexBlendARB is NULL");
    {
    GL_FUNC(ext_glVertexBlendARB)(ARG1);
    GL_LOG ("glVertexBlendARB GLint count=%d",ARG1);
    }
}
void glcommon_glVertexBlendEnvfATI(CPU* cpu) {
    if (!ext_glVertexBlendEnvfATI)
        kpanic("ext_glVertexBlendEnvfATI is NULL");
    {
    GL_FUNC(ext_glVertexBlendEnvfATI)(ARG1, fARG2);
    GL_LOG ("glVertexBlendEnvfATI GLenum pname=%d, GLfloat param=%f",ARG1,fARG2);
    }
}
void glcommon_glVertexBlendEnviATI(CPU* cpu) {
    if (!ext_glVertexBlendEnviATI)
        kpanic("ext_glVertexBlendEnviATI is NULL");
    {
    GL_FUNC(ext_glVertexBlendEnviATI)(ARG1, ARG2);
    GL_LOG ("glVertexBlendEnviATI GLenum pname=%d, GLint param=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexFormatNV(CPU* cpu) {
    if (!ext_glVertexFormatNV)
        kpanic("ext_glVertexFormatNV is NULL");
    {
    GL_FUNC(ext_glVertexFormatNV)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexFormatNV GLint size=%d, GLenum type=%d, GLsizei stride=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexP2ui(CPU* cpu) {
    if (!ext_glVertexP2ui)
        kpanic("ext_glVertexP2ui is NULL");
    {
    GL_FUNC(ext_glVertexP2ui)(ARG1, ARG2);
    GL_LOG ("glVertexP2ui GLenum type=%d, GLuint value=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexP2uiv(CPU* cpu) {
    if (!ext_glVertexP2uiv)
        kpanic("ext_glVertexP2uiv is NULL");
    {
    GL_FUNC(ext_glVertexP2uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexP2uiv GLenum type=%d, const GLuint* value=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexP3ui(CPU* cpu) {
    if (!ext_glVertexP3ui)
        kpanic("ext_glVertexP3ui is NULL");
    {
    GL_FUNC(ext_glVertexP3ui)(ARG1, ARG2);
    GL_LOG ("glVertexP3ui GLenum type=%d, GLuint value=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexP3uiv(CPU* cpu) {
    if (!ext_glVertexP3uiv)
        kpanic("ext_glVertexP3uiv is NULL");
    {
    GL_FUNC(ext_glVertexP3uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexP3uiv GLenum type=%d, const GLuint* value=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexP4ui(CPU* cpu) {
    if (!ext_glVertexP4ui)
        kpanic("ext_glVertexP4ui is NULL");
    {
    GL_FUNC(ext_glVertexP4ui)(ARG1, ARG2);
    GL_LOG ("glVertexP4ui GLenum type=%d, GLuint value=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexP4uiv(CPU* cpu) {
    if (!ext_glVertexP4uiv)
        kpanic("ext_glVertexP4uiv is NULL");
    {
    GL_FUNC(ext_glVertexP4uiv)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexP4uiv GLenum type=%d, const GLuint* value=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexPointerEXT(CPU* cpu) {
    if (!ext_glVertexPointerEXT)
        kpanic("ext_glVertexPointerEXT is NULL");
    {
    GL_FUNC(ext_glVertexPointerEXT)(ARG1, ARG2, ARG3, ARG4, (void*)marshalp(cpu, 0, ARG5, 0));
    GL_LOG ("glVertexPointerEXT GLint size=%d, GLenum type=%d, GLsizei stride=%d, GLsizei count=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexPointerListIBM(CPU* cpu) {
    if (!ext_glVertexPointerListIBM)
        kpanic("ext_glVertexPointerListIBM is NULL");
    {
    GL_FUNC(ext_glVertexPointerListIBM)(ARG1, ARG2, ARG3, (const void**)marshalunhandled("glVertexPointerListIBM", "pointer", cpu, ARG4), ARG5);
    GL_LOG ("glVertexPointerListIBM GLint size=%d, GLenum type=%d, GLint stride=%d, const void** pointer=%.08x, GLint ptrstride=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexPointervINTEL(CPU* cpu) {
    if (!ext_glVertexPointervINTEL)
        kpanic("ext_glVertexPointervINTEL is NULL");
    {
    GL_FUNC(ext_glVertexPointervINTEL)(ARG1, ARG2, (const void**)marshalunhandled("glVertexPointervINTEL", "pointer", cpu, ARG3));
    GL_LOG ("glVertexPointervINTEL GLint size=%d, GLenum type=%d, const void** pointer=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexStream1dATI(CPU* cpu) {
    if (!ext_glVertexStream1dATI)
        kpanic("ext_glVertexStream1dATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1dATI)(ARG1, dARG2);
    GL_LOG ("glVertexStream1dATI GLenum stream=%d, GLdouble x=%f",ARG1,dARG2);
    }
}
void glcommon_glVertexStream1dvATI(CPU* cpu) {
    if (!ext_glVertexStream1dvATI)
        kpanic("ext_glVertexStream1dvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1dvATI)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream1dvATI GLenum stream=%d, const GLdouble* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream1fATI(CPU* cpu) {
    if (!ext_glVertexStream1fATI)
        kpanic("ext_glVertexStream1fATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1fATI)(ARG1, fARG2);
    GL_LOG ("glVertexStream1fATI GLenum stream=%d, GLfloat x=%f",ARG1,fARG2);
    }
}
void glcommon_glVertexStream1fvATI(CPU* cpu) {
    if (!ext_glVertexStream1fvATI)
        kpanic("ext_glVertexStream1fvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1fvATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream1fvATI GLenum stream=%d, const GLfloat* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream1iATI(CPU* cpu) {
    if (!ext_glVertexStream1iATI)
        kpanic("ext_glVertexStream1iATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1iATI)(ARG1, ARG2);
    GL_LOG ("glVertexStream1iATI GLenum stream=%d, GLint x=%d",ARG1,ARG2);
    }
}
void glcommon_glVertexStream1ivATI(CPU* cpu) {
    if (!ext_glVertexStream1ivATI)
        kpanic("ext_glVertexStream1ivATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1ivATI)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream1ivATI GLenum stream=%d, const GLint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream1sATI(CPU* cpu) {
    if (!ext_glVertexStream1sATI)
        kpanic("ext_glVertexStream1sATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1sATI)(ARG1, sARG2);
    GL_LOG ("glVertexStream1sATI GLenum stream=%d, GLshort x=%d",ARG1,sARG2);
    }
}
void glcommon_glVertexStream1svATI(CPU* cpu) {
    if (!ext_glVertexStream1svATI)
        kpanic("ext_glVertexStream1svATI is NULL");
    {
    GL_FUNC(ext_glVertexStream1svATI)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream1svATI GLenum stream=%d, const GLshort* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream2dATI(CPU* cpu) {
    if (!ext_glVertexStream2dATI)
        kpanic("ext_glVertexStream2dATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2dATI)(ARG1, dARG2, dARG3);
    GL_LOG ("glVertexStream2dATI GLenum stream=%d, GLdouble x=%f, GLdouble y=%f",ARG1,dARG2,dARG3);
    }
}
void glcommon_glVertexStream2dvATI(CPU* cpu) {
    if (!ext_glVertexStream2dvATI)
        kpanic("ext_glVertexStream2dvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2dvATI)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream2dvATI GLenum stream=%d, const GLdouble* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream2fATI(CPU* cpu) {
    if (!ext_glVertexStream2fATI)
        kpanic("ext_glVertexStream2fATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2fATI)(ARG1, fARG2, fARG3);
    GL_LOG ("glVertexStream2fATI GLenum stream=%d, GLfloat x=%f, GLfloat y=%f",ARG1,fARG2,fARG3);
    }
}
void glcommon_glVertexStream2fvATI(CPU* cpu) {
    if (!ext_glVertexStream2fvATI)
        kpanic("ext_glVertexStream2fvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2fvATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream2fvATI GLenum stream=%d, const GLfloat* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream2iATI(CPU* cpu) {
    if (!ext_glVertexStream2iATI)
        kpanic("ext_glVertexStream2iATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2iATI)(ARG1, ARG2, ARG3);
    GL_LOG ("glVertexStream2iATI GLenum stream=%d, GLint x=%d, GLint y=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVertexStream2ivATI(CPU* cpu) {
    if (!ext_glVertexStream2ivATI)
        kpanic("ext_glVertexStream2ivATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2ivATI)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream2ivATI GLenum stream=%d, const GLint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream2sATI(CPU* cpu) {
    if (!ext_glVertexStream2sATI)
        kpanic("ext_glVertexStream2sATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2sATI)(ARG1, sARG2, sARG3);
    GL_LOG ("glVertexStream2sATI GLenum stream=%d, GLshort x=%d, GLshort y=%d",ARG1,sARG2,sARG3);
    }
}
void glcommon_glVertexStream2svATI(CPU* cpu) {
    if (!ext_glVertexStream2svATI)
        kpanic("ext_glVertexStream2svATI is NULL");
    {
    GL_FUNC(ext_glVertexStream2svATI)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream2svATI GLenum stream=%d, const GLshort* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream3dATI(CPU* cpu) {
    if (!ext_glVertexStream3dATI)
        kpanic("ext_glVertexStream3dATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3dATI)(ARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glVertexStream3dATI GLenum stream=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",ARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glVertexStream3dvATI(CPU* cpu) {
    if (!ext_glVertexStream3dvATI)
        kpanic("ext_glVertexStream3dvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3dvATI)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream3dvATI GLenum stream=%d, const GLdouble* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream3fATI(CPU* cpu) {
    if (!ext_glVertexStream3fATI)
        kpanic("ext_glVertexStream3fATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3fATI)(ARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glVertexStream3fATI GLenum stream=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",ARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glVertexStream3fvATI(CPU* cpu) {
    if (!ext_glVertexStream3fvATI)
        kpanic("ext_glVertexStream3fvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3fvATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream3fvATI GLenum stream=%d, const GLfloat* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream3iATI(CPU* cpu) {
    if (!ext_glVertexStream3iATI)
        kpanic("ext_glVertexStream3iATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3iATI)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glVertexStream3iATI GLenum stream=%d, GLint x=%d, GLint y=%d, GLint z=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexStream3ivATI(CPU* cpu) {
    if (!ext_glVertexStream3ivATI)
        kpanic("ext_glVertexStream3ivATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3ivATI)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream3ivATI GLenum stream=%d, const GLint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream3sATI(CPU* cpu) {
    if (!ext_glVertexStream3sATI)
        kpanic("ext_glVertexStream3sATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3sATI)(ARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glVertexStream3sATI GLenum stream=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d",ARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glVertexStream3svATI(CPU* cpu) {
    if (!ext_glVertexStream3svATI)
        kpanic("ext_glVertexStream3svATI is NULL");
    {
    GL_FUNC(ext_glVertexStream3svATI)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream3svATI GLenum stream=%d, const GLshort* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream4dATI(CPU* cpu) {
    if (!ext_glVertexStream4dATI)
        kpanic("ext_glVertexStream4dATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4dATI)(ARG1, dARG2, dARG3, dARG4, dARG5);
    GL_LOG ("glVertexStream4dATI GLenum stream=%d, GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",ARG1,dARG2,dARG3,dARG4,dARG5);
    }
}
void glcommon_glVertexStream4dvATI(CPU* cpu) {
    if (!ext_glVertexStream4dvATI)
        kpanic("ext_glVertexStream4dvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4dvATI)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream4dvATI GLenum stream=%d, const GLdouble* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream4fATI(CPU* cpu) {
    if (!ext_glVertexStream4fATI)
        kpanic("ext_glVertexStream4fATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4fATI)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glVertexStream4fATI GLenum stream=%d, GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glVertexStream4fvATI(CPU* cpu) {
    if (!ext_glVertexStream4fvATI)
        kpanic("ext_glVertexStream4fvATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4fvATI)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream4fvATI GLenum stream=%d, const GLfloat* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream4iATI(CPU* cpu) {
    if (!ext_glVertexStream4iATI)
        kpanic("ext_glVertexStream4iATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4iATI)(ARG1, ARG2, ARG3, ARG4, ARG5);
    GL_LOG ("glVertexStream4iATI GLenum stream=%d, GLint x=%d, GLint y=%d, GLint z=%d, GLint w=%d",ARG1,ARG2,ARG3,ARG4,ARG5);
    }
}
void glcommon_glVertexStream4ivATI(CPU* cpu) {
    if (!ext_glVertexStream4ivATI)
        kpanic("ext_glVertexStream4ivATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4ivATI)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream4ivATI GLenum stream=%d, const GLint* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexStream4sATI(CPU* cpu) {
    if (!ext_glVertexStream4sATI)
        kpanic("ext_glVertexStream4sATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4sATI)(ARG1, sARG2, sARG3, sARG4, sARG5);
    GL_LOG ("glVertexStream4sATI GLenum stream=%d, GLshort x=%d, GLshort y=%d, GLshort z=%d, GLshort w=%d",ARG1,sARG2,sARG3,sARG4,sARG5);
    }
}
void glcommon_glVertexStream4svATI(CPU* cpu) {
    if (!ext_glVertexStream4svATI)
        kpanic("ext_glVertexStream4svATI is NULL");
    {
    GL_FUNC(ext_glVertexStream4svATI)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glVertexStream4svATI GLenum stream=%d, const GLshort* coords=%.08x",ARG1,ARG2);
    }
}
void glcommon_glVertexWeightPointerEXT(CPU* cpu) {
    if (!ext_glVertexWeightPointerEXT)
        kpanic("ext_glVertexWeightPointerEXT is NULL");
    {
    GL_FUNC(ext_glVertexWeightPointerEXT)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVertexWeightPointerEXT GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVertexWeightfEXT(CPU* cpu) {
    if (!ext_glVertexWeightfEXT)
        kpanic("ext_glVertexWeightfEXT is NULL");
    {
    GL_FUNC(ext_glVertexWeightfEXT)(fARG1);
    GL_LOG ("glVertexWeightfEXT GLfloat weight=%f",fARG1);
    }
}
void glcommon_glVertexWeightfvEXT(CPU* cpu) {
    if (!ext_glVertexWeightfvEXT)
        kpanic("ext_glVertexWeightfvEXT is NULL");
    {
    GL_FUNC(ext_glVertexWeightfvEXT)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertexWeightfvEXT const GLfloat* weight=%.08x",ARG1);
    }
}
void glcommon_glVertexWeighthNV(CPU* cpu) {
    if (!ext_glVertexWeighthNV)
        kpanic("ext_glVertexWeighthNV is NULL");
    {
    GL_FUNC(ext_glVertexWeighthNV)(sARG1);
    GL_LOG ("glVertexWeighthNV GLhalfNV weight=%d",sARG1);
    }
}
void glcommon_glVertexWeighthvNV(CPU* cpu) {
    if (!ext_glVertexWeighthvNV)
        kpanic("ext_glVertexWeighthvNV is NULL");
    {
    GL_FUNC(ext_glVertexWeighthvNV)((GLhalfNV*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glVertexWeighthvNV const GLhalfNV* weight=%.08x",ARG1);
    }
}
void glcommon_glVideoCaptureNV(CPU* cpu) {
    if (!ext_glVideoCaptureNV)
        kpanic("ext_glVideoCaptureNV is NULL");
    {
    EAX=GL_FUNC(ext_glVideoCaptureNV)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0), (GLuint64EXT*)marshalp(cpu, 0, ARG3, 0));
    GL_LOG ("glVideoCaptureNV GLuint video_capture_slot=%d, GLuint* sequence_num=%.08x, GLuint64EXT* capture_time=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glVideoCaptureStreamParameterdvNV(CPU* cpu) {
    if (!ext_glVideoCaptureStreamParameterdvNV)
        kpanic("ext_glVideoCaptureStreamParameterdvNV is NULL");
    {
    GL_FUNC(ext_glVideoCaptureStreamParameterdvNV)(ARG1, ARG2, ARG3, (GLdouble*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVideoCaptureStreamParameterdvNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum pname=%d, const GLdouble* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVideoCaptureStreamParameterfvNV(CPU* cpu) {
    if (!ext_glVideoCaptureStreamParameterfvNV)
        kpanic("ext_glVideoCaptureStreamParameterfvNV is NULL");
    {
    GL_FUNC(ext_glVideoCaptureStreamParameterfvNV)(ARG1, ARG2, ARG3, (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVideoCaptureStreamParameterfvNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum pname=%d, const GLfloat* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glVideoCaptureStreamParameterivNV(CPU* cpu) {
    if (!ext_glVideoCaptureStreamParameterivNV)
        kpanic("ext_glVideoCaptureStreamParameterivNV is NULL");
    {
    GL_FUNC(ext_glVideoCaptureStreamParameterivNV)(ARG1, ARG2, ARG3, (GLint*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glVideoCaptureStreamParameterivNV GLuint video_capture_slot=%d, GLuint stream=%d, GLenum pname=%d, const GLint* params=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glViewportArrayv(CPU* cpu) {
    if (!ext_glViewportArrayv)
        kpanic("ext_glViewportArrayv is NULL");
    {
    GL_FUNC(ext_glViewportArrayv)(ARG1, ARG2, marshalArray<GLfloat>(cpu, ARG3, ARG2*4));
    GL_LOG ("glViewportArrayv GLuint first=%d, GLsizei count=%d, const GLfloat* v=%.08x",ARG1,ARG2,ARG3);
    }
}
void glcommon_glViewportIndexedf(CPU* cpu) {
    if (!ext_glViewportIndexedf)
        kpanic("ext_glViewportIndexedf is NULL");
    {
    GL_FUNC(ext_glViewportIndexedf)(ARG1, fARG2, fARG3, fARG4, fARG5);
    GL_LOG ("glViewportIndexedf GLuint index=%d, GLfloat x=%f, GLfloat y=%f, GLfloat w=%f, GLfloat h=%f",ARG1,fARG2,fARG3,fARG4,fARG5);
    }
}
void glcommon_glViewportIndexedfv(CPU* cpu) {
    if (!ext_glViewportIndexedfv)
        kpanic("ext_glViewportIndexedfv is NULL");
    {
    GL_FUNC(ext_glViewportIndexedfv)(ARG1, marshalArray<GLfloat>(cpu, ARG2, 4));
    GL_LOG ("glViewportIndexedfv GLuint index=%d, const GLfloat* v=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWaitSync(CPU* cpu) {
    if (!ext_glWaitSync)
        kpanic("ext_glWaitSync is NULL");
    {
    GL_FUNC(ext_glWaitSync)(marshalSync(cpu, ARG1), ARG2, llARG3);
    GL_LOG ("glWaitSync GLsync sync=%d, GLbitfield flags=%d, GLuint64 timeout=" PRIu64 "",ARG1,ARG2,llARG3);
    }
}
void glcommon_glWeightPathsNV(CPU* cpu) {
    if (!ext_glWeightPathsNV)
        kpanic("ext_glWeightPathsNV is NULL");
    {
    GL_FUNC(ext_glWeightPathsNV)(ARG1, ARG2, (GLuint*)marshalp(cpu, 0, ARG3, 0), (GLfloat*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glWeightPathsNV GLuint resultPath=%d, GLsizei numPaths=%d, const GLuint* paths=%.08x, const GLfloat* weights=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glWeightPointerARB(CPU* cpu) {
    if (!ext_glWeightPointerARB)
        kpanic("ext_glWeightPointerARB is NULL");
    {
    GL_FUNC(ext_glWeightPointerARB)(ARG1, ARG2, ARG3, (void*)marshalp(cpu, 0, ARG4, 0));
    GL_LOG ("glWeightPointerARB GLint size=%d, GLenum type=%d, GLsizei stride=%d, const void* pointer=%.08x",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glWeightbvARB(CPU* cpu) {
    if (!ext_glWeightbvARB)
        kpanic("ext_glWeightbvARB is NULL");
    {
    GL_FUNC(ext_glWeightbvARB)(ARG1, (GLbyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightbvARB GLint size=%d, const GLbyte* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWeightdvARB(CPU* cpu) {
    if (!ext_glWeightdvARB)
        kpanic("ext_glWeightdvARB is NULL");
    {
    GL_FUNC(ext_glWeightdvARB)(ARG1, (GLdouble*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightdvARB GLint size=%d, const GLdouble* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWeightfvARB(CPU* cpu) {
    if (!ext_glWeightfvARB)
        kpanic("ext_glWeightfvARB is NULL");
    {
    GL_FUNC(ext_glWeightfvARB)(ARG1, (GLfloat*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightfvARB GLint size=%d, const GLfloat* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWeightivARB(CPU* cpu) {
    if (!ext_glWeightivARB)
        kpanic("ext_glWeightivARB is NULL");
    {
    GL_FUNC(ext_glWeightivARB)(ARG1, (GLint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightivARB GLint size=%d, const GLint* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWeightsvARB(CPU* cpu) {
    if (!ext_glWeightsvARB)
        kpanic("ext_glWeightsvARB is NULL");
    {
    GL_FUNC(ext_glWeightsvARB)(ARG1, (GLshort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightsvARB GLint size=%d, const GLshort* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWeightubvARB(CPU* cpu) {
    if (!ext_glWeightubvARB)
        kpanic("ext_glWeightubvARB is NULL");
    {
    GL_FUNC(ext_glWeightubvARB)(ARG1, (GLubyte*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightubvARB GLint size=%d, const GLubyte* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWeightuivARB(CPU* cpu) {
    if (!ext_glWeightuivARB)
        kpanic("ext_glWeightuivARB is NULL");
    {
    GL_FUNC(ext_glWeightuivARB)(ARG1, (GLuint*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightuivARB GLint size=%d, const GLuint* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWeightusvARB(CPU* cpu) {
    if (!ext_glWeightusvARB)
        kpanic("ext_glWeightusvARB is NULL");
    {
    GL_FUNC(ext_glWeightusvARB)(ARG1, (GLushort*)marshalp(cpu, 0, ARG2, 0));
    GL_LOG ("glWeightusvARB GLint size=%d, const GLushort* weights=%.08x",ARG1,ARG2);
    }
}
void glcommon_glWindowPos2d(CPU* cpu) {
    if (!ext_glWindowPos2d)
        kpanic("ext_glWindowPos2d is NULL");
    {
    GL_FUNC(ext_glWindowPos2d)(dARG1, dARG2);
    GL_LOG ("glWindowPos2d GLdouble x=%f, GLdouble y=%f",dARG1,dARG2);
    }
}
void glcommon_glWindowPos2dARB(CPU* cpu) {
    if (!ext_glWindowPos2dARB)
        kpanic("ext_glWindowPos2dARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2dARB)(dARG1, dARG2);
    GL_LOG ("glWindowPos2dARB GLdouble x=%f, GLdouble y=%f",dARG1,dARG2);
    }
}
void glcommon_glWindowPos2dMESA(CPU* cpu) {
    if (!ext_glWindowPos2dMESA)
        kpanic("ext_glWindowPos2dMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2dMESA)(dARG1, dARG2);
    GL_LOG ("glWindowPos2dMESA GLdouble x=%f, GLdouble y=%f",dARG1,dARG2);
    }
}
void glcommon_glWindowPos2dv(CPU* cpu) {
    if (!ext_glWindowPos2dv)
        kpanic("ext_glWindowPos2dv is NULL");
    {
    GL_FUNC(ext_glWindowPos2dv)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2dv const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2dvARB(CPU* cpu) {
    if (!ext_glWindowPos2dvARB)
        kpanic("ext_glWindowPos2dvARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2dvARB)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2dvARB const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2dvMESA(CPU* cpu) {
    if (!ext_glWindowPos2dvMESA)
        kpanic("ext_glWindowPos2dvMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2dvMESA)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2dvMESA const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2f(CPU* cpu) {
    if (!ext_glWindowPos2f)
        kpanic("ext_glWindowPos2f is NULL");
    {
    GL_FUNC(ext_glWindowPos2f)(fARG1, fARG2);
    GL_LOG ("glWindowPos2f GLfloat x=%f, GLfloat y=%f",fARG1,fARG2);
    }
}
void glcommon_glWindowPos2fARB(CPU* cpu) {
    if (!ext_glWindowPos2fARB)
        kpanic("ext_glWindowPos2fARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2fARB)(fARG1, fARG2);
    GL_LOG ("glWindowPos2fARB GLfloat x=%f, GLfloat y=%f",fARG1,fARG2);
    }
}
void glcommon_glWindowPos2fMESA(CPU* cpu) {
    if (!ext_glWindowPos2fMESA)
        kpanic("ext_glWindowPos2fMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2fMESA)(fARG1, fARG2);
    GL_LOG ("glWindowPos2fMESA GLfloat x=%f, GLfloat y=%f",fARG1,fARG2);
    }
}
void glcommon_glWindowPos2fv(CPU* cpu) {
    if (!ext_glWindowPos2fv)
        kpanic("ext_glWindowPos2fv is NULL");
    {
    GL_FUNC(ext_glWindowPos2fv)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2fv const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2fvARB(CPU* cpu) {
    if (!ext_glWindowPos2fvARB)
        kpanic("ext_glWindowPos2fvARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2fvARB)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2fvARB const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2fvMESA(CPU* cpu) {
    if (!ext_glWindowPos2fvMESA)
        kpanic("ext_glWindowPos2fvMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2fvMESA)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2fvMESA const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2i(CPU* cpu) {
    if (!ext_glWindowPos2i)
        kpanic("ext_glWindowPos2i is NULL");
    {
    GL_FUNC(ext_glWindowPos2i)(ARG1, ARG2);
    GL_LOG ("glWindowPos2i GLint x=%d, GLint y=%d",ARG1,ARG2);
    }
}
void glcommon_glWindowPos2iARB(CPU* cpu) {
    if (!ext_glWindowPos2iARB)
        kpanic("ext_glWindowPos2iARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2iARB)(ARG1, ARG2);
    GL_LOG ("glWindowPos2iARB GLint x=%d, GLint y=%d",ARG1,ARG2);
    }
}
void glcommon_glWindowPos2iMESA(CPU* cpu) {
    if (!ext_glWindowPos2iMESA)
        kpanic("ext_glWindowPos2iMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2iMESA)(ARG1, ARG2);
    GL_LOG ("glWindowPos2iMESA GLint x=%d, GLint y=%d",ARG1,ARG2);
    }
}
void glcommon_glWindowPos2iv(CPU* cpu) {
    if (!ext_glWindowPos2iv)
        kpanic("ext_glWindowPos2iv is NULL");
    {
    GL_FUNC(ext_glWindowPos2iv)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2iv const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2ivARB(CPU* cpu) {
    if (!ext_glWindowPos2ivARB)
        kpanic("ext_glWindowPos2ivARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2ivARB)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2ivARB const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2ivMESA(CPU* cpu) {
    if (!ext_glWindowPos2ivMESA)
        kpanic("ext_glWindowPos2ivMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2ivMESA)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2ivMESA const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2s(CPU* cpu) {
    if (!ext_glWindowPos2s)
        kpanic("ext_glWindowPos2s is NULL");
    {
    GL_FUNC(ext_glWindowPos2s)(sARG1, sARG2);
    GL_LOG ("glWindowPos2s GLshort x=%d, GLshort y=%d",sARG1,sARG2);
    }
}
void glcommon_glWindowPos2sARB(CPU* cpu) {
    if (!ext_glWindowPos2sARB)
        kpanic("ext_glWindowPos2sARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2sARB)(sARG1, sARG2);
    GL_LOG ("glWindowPos2sARB GLshort x=%d, GLshort y=%d",sARG1,sARG2);
    }
}
void glcommon_glWindowPos2sMESA(CPU* cpu) {
    if (!ext_glWindowPos2sMESA)
        kpanic("ext_glWindowPos2sMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2sMESA)(sARG1, sARG2);
    GL_LOG ("glWindowPos2sMESA GLshort x=%d, GLshort y=%d",sARG1,sARG2);
    }
}
void glcommon_glWindowPos2sv(CPU* cpu) {
    if (!ext_glWindowPos2sv)
        kpanic("ext_glWindowPos2sv is NULL");
    {
    GL_FUNC(ext_glWindowPos2sv)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2sv const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2svARB(CPU* cpu) {
    if (!ext_glWindowPos2svARB)
        kpanic("ext_glWindowPos2svARB is NULL");
    {
    GL_FUNC(ext_glWindowPos2svARB)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2svARB const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos2svMESA(CPU* cpu) {
    if (!ext_glWindowPos2svMESA)
        kpanic("ext_glWindowPos2svMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos2svMESA)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos2svMESA const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3d(CPU* cpu) {
    if (!ext_glWindowPos3d)
        kpanic("ext_glWindowPos3d is NULL");
    {
    GL_FUNC(ext_glWindowPos3d)(dARG1, dARG2, dARG3);
    GL_LOG ("glWindowPos3d GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",dARG1,dARG2,dARG3);
    }
}
void glcommon_glWindowPos3dARB(CPU* cpu) {
    if (!ext_glWindowPos3dARB)
        kpanic("ext_glWindowPos3dARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3dARB)(dARG1, dARG2, dARG3);
    GL_LOG ("glWindowPos3dARB GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",dARG1,dARG2,dARG3);
    }
}
void glcommon_glWindowPos3dMESA(CPU* cpu) {
    if (!ext_glWindowPos3dMESA)
        kpanic("ext_glWindowPos3dMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3dMESA)(dARG1, dARG2, dARG3);
    GL_LOG ("glWindowPos3dMESA GLdouble x=%f, GLdouble y=%f, GLdouble z=%f",dARG1,dARG2,dARG3);
    }
}
void glcommon_glWindowPos3dv(CPU* cpu) {
    if (!ext_glWindowPos3dv)
        kpanic("ext_glWindowPos3dv is NULL");
    {
    GL_FUNC(ext_glWindowPos3dv)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3dv const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3dvARB(CPU* cpu) {
    if (!ext_glWindowPos3dvARB)
        kpanic("ext_glWindowPos3dvARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3dvARB)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3dvARB const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3dvMESA(CPU* cpu) {
    if (!ext_glWindowPos3dvMESA)
        kpanic("ext_glWindowPos3dvMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3dvMESA)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3dvMESA const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3f(CPU* cpu) {
    if (!ext_glWindowPos3f)
        kpanic("ext_glWindowPos3f is NULL");
    {
    GL_FUNC(ext_glWindowPos3f)(fARG1, fARG2, fARG3);
    GL_LOG ("glWindowPos3f GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glWindowPos3fARB(CPU* cpu) {
    if (!ext_glWindowPos3fARB)
        kpanic("ext_glWindowPos3fARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3fARB)(fARG1, fARG2, fARG3);
    GL_LOG ("glWindowPos3fARB GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glWindowPos3fMESA(CPU* cpu) {
    if (!ext_glWindowPos3fMESA)
        kpanic("ext_glWindowPos3fMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3fMESA)(fARG1, fARG2, fARG3);
    GL_LOG ("glWindowPos3fMESA GLfloat x=%f, GLfloat y=%f, GLfloat z=%f",fARG1,fARG2,fARG3);
    }
}
void glcommon_glWindowPos3fv(CPU* cpu) {
    if (!ext_glWindowPos3fv)
        kpanic("ext_glWindowPos3fv is NULL");
    {
    GL_FUNC(ext_glWindowPos3fv)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3fv const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3fvARB(CPU* cpu) {
    if (!ext_glWindowPos3fvARB)
        kpanic("ext_glWindowPos3fvARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3fvARB)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3fvARB const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3fvMESA(CPU* cpu) {
    if (!ext_glWindowPos3fvMESA)
        kpanic("ext_glWindowPos3fvMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3fvMESA)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3fvMESA const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3i(CPU* cpu) {
    if (!ext_glWindowPos3i)
        kpanic("ext_glWindowPos3i is NULL");
    {
    GL_FUNC(ext_glWindowPos3i)(ARG1, ARG2, ARG3);
    GL_LOG ("glWindowPos3i GLint x=%d, GLint y=%d, GLint z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glWindowPos3iARB(CPU* cpu) {
    if (!ext_glWindowPos3iARB)
        kpanic("ext_glWindowPos3iARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3iARB)(ARG1, ARG2, ARG3);
    GL_LOG ("glWindowPos3iARB GLint x=%d, GLint y=%d, GLint z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glWindowPos3iMESA(CPU* cpu) {
    if (!ext_glWindowPos3iMESA)
        kpanic("ext_glWindowPos3iMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3iMESA)(ARG1, ARG2, ARG3);
    GL_LOG ("glWindowPos3iMESA GLint x=%d, GLint y=%d, GLint z=%d",ARG1,ARG2,ARG3);
    }
}
void glcommon_glWindowPos3iv(CPU* cpu) {
    if (!ext_glWindowPos3iv)
        kpanic("ext_glWindowPos3iv is NULL");
    {
    GL_FUNC(ext_glWindowPos3iv)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3iv const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3ivARB(CPU* cpu) {
    if (!ext_glWindowPos3ivARB)
        kpanic("ext_glWindowPos3ivARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3ivARB)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3ivARB const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3ivMESA(CPU* cpu) {
    if (!ext_glWindowPos3ivMESA)
        kpanic("ext_glWindowPos3ivMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3ivMESA)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3ivMESA const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3s(CPU* cpu) {
    if (!ext_glWindowPos3s)
        kpanic("ext_glWindowPos3s is NULL");
    {
    GL_FUNC(ext_glWindowPos3s)(sARG1, sARG2, sARG3);
    GL_LOG ("glWindowPos3s GLshort x=%d, GLshort y=%d, GLshort z=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glWindowPos3sARB(CPU* cpu) {
    if (!ext_glWindowPos3sARB)
        kpanic("ext_glWindowPos3sARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3sARB)(sARG1, sARG2, sARG3);
    GL_LOG ("glWindowPos3sARB GLshort x=%d, GLshort y=%d, GLshort z=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glWindowPos3sMESA(CPU* cpu) {
    if (!ext_glWindowPos3sMESA)
        kpanic("ext_glWindowPos3sMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3sMESA)(sARG1, sARG2, sARG3);
    GL_LOG ("glWindowPos3sMESA GLshort x=%d, GLshort y=%d, GLshort z=%d",sARG1,sARG2,sARG3);
    }
}
void glcommon_glWindowPos3sv(CPU* cpu) {
    if (!ext_glWindowPos3sv)
        kpanic("ext_glWindowPos3sv is NULL");
    {
    GL_FUNC(ext_glWindowPos3sv)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3sv const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3svARB(CPU* cpu) {
    if (!ext_glWindowPos3svARB)
        kpanic("ext_glWindowPos3svARB is NULL");
    {
    GL_FUNC(ext_glWindowPos3svARB)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3svARB const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos3svMESA(CPU* cpu) {
    if (!ext_glWindowPos3svMESA)
        kpanic("ext_glWindowPos3svMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos3svMESA)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos3svMESA const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos4dMESA(CPU* cpu) {
    if (!ext_glWindowPos4dMESA)
        kpanic("ext_glWindowPos4dMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4dMESA)(dARG1, dARG2, dARG3, dARG4);
    GL_LOG ("glWindowPos4dMESA GLdouble x=%f, GLdouble y=%f, GLdouble z=%f, GLdouble w=%f",dARG1,dARG2,dARG3,dARG4);
    }
}
void glcommon_glWindowPos4dvMESA(CPU* cpu) {
    if (!ext_glWindowPos4dvMESA)
        kpanic("ext_glWindowPos4dvMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4dvMESA)((GLdouble*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos4dvMESA const GLdouble* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos4fMESA(CPU* cpu) {
    if (!ext_glWindowPos4fMESA)
        kpanic("ext_glWindowPos4fMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4fMESA)(fARG1, fARG2, fARG3, fARG4);
    GL_LOG ("glWindowPos4fMESA GLfloat x=%f, GLfloat y=%f, GLfloat z=%f, GLfloat w=%f",fARG1,fARG2,fARG3,fARG4);
    }
}
void glcommon_glWindowPos4fvMESA(CPU* cpu) {
    if (!ext_glWindowPos4fvMESA)
        kpanic("ext_glWindowPos4fvMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4fvMESA)((GLfloat*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos4fvMESA const GLfloat* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos4iMESA(CPU* cpu) {
    if (!ext_glWindowPos4iMESA)
        kpanic("ext_glWindowPos4iMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4iMESA)(ARG1, ARG2, ARG3, ARG4);
    GL_LOG ("glWindowPos4iMESA GLint x=%d, GLint y=%d, GLint z=%d, GLint w=%d",ARG1,ARG2,ARG3,ARG4);
    }
}
void glcommon_glWindowPos4ivMESA(CPU* cpu) {
    if (!ext_glWindowPos4ivMESA)
        kpanic("ext_glWindowPos4ivMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4ivMESA)((GLint*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos4ivMESA const GLint* v=%.08x",ARG1);
    }
}
void glcommon_glWindowPos4sMESA(CPU* cpu) {
    if (!ext_glWindowPos4sMESA)
        kpanic("ext_glWindowPos4sMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4sMESA)(sARG1, sARG2, sARG3, sARG4);
    GL_LOG ("glWindowPos4sMESA GLshort x=%d, GLshort y=%d, GLshort z=%d, GLshort w=%d",sARG1,sARG2,sARG3,sARG4);
    }
}
void glcommon_glWindowPos4svMESA(CPU* cpu) {
    if (!ext_glWindowPos4svMESA)
        kpanic("ext_glWindowPos4svMESA is NULL");
    {
    GL_FUNC(ext_glWindowPos4svMESA)((GLshort*)marshalp(cpu, 0, ARG1, 0));
    GL_LOG ("glWindowPos4svMESA const GLshort* v=%.08x",ARG1);
    }
}
void glcommon_glWriteMaskEXT(CPU* cpu) {
    if (!ext_glWriteMaskEXT)
        kpanic("ext_glWriteMaskEXT is NULL");
    {
    GL_FUNC(ext_glWriteMaskEXT)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    GL_LOG ("glWriteMaskEXT GLuint res=%d, GLuint in=%d, GLenum outX=%d, GLenum outY=%d, GLenum outZ=%d, GLenum outW=%d",ARG1,ARG2,ARG3,ARG4,ARG5,ARG6);
    }
}
#endif
#endif
