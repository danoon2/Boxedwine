GLAPI void APIENTRY glAccumxOES( GLenum op, GLfixed value) {
    CALL_2(AccumxOES, op, value);
}

GLAPI void APIENTRY glActiveProgramEXT( GLuint program) {
    CALL_1(ActiveProgramEXT, program);
}

GLAPI void APIENTRY glActiveShaderProgram( GLuint pipeline, GLuint program) {
    CALL_2(ActiveShaderProgram, pipeline, program);
}

GLAPI void APIENTRY glActiveStencilFaceEXT( GLenum face) {
    CALL_1(ActiveStencilFaceEXT, face);
}

GLAPI void APIENTRY glActiveTexture( GLenum texture) {
    CALL_1(ActiveTexture, texture);
}

GLAPI void APIENTRY glActiveTextureARB( GLenum texture) {
    CALL_1(ActiveTextureARB, texture);
}

GLAPI void APIENTRY glActiveVaryingNV( GLuint program, const GLchar* name) {
    CALL_2(ActiveVaryingNV, program, name);
}

GLAPI void APIENTRY glAlphaFragmentOp1ATI( GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod) {
    CALL_6(AlphaFragmentOp1ATI, op, dst, dstMod, arg1, arg1Rep, arg1Mod);
}

GLAPI void APIENTRY glAlphaFragmentOp2ATI( GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod) {
    CALL_9(AlphaFragmentOp2ATI, op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod);
}

GLAPI void APIENTRY glAlphaFragmentOp3ATI( GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod) {
    CALL_12(AlphaFragmentOp3ATI, op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod);
}

GLAPI void APIENTRY glAlphaFuncxOES( GLenum func, GLfixed ref) {
    CALL_2(AlphaFuncxOES, func, ref);
}

GLAPI void APIENTRY glApplyFramebufferAttachmentCMAAINTEL( void ) {
    CALL_0(ApplyFramebufferAttachmentCMAAINTEL);
}

GLAPI void APIENTRY glApplyTextureEXT( GLenum mode) {
    CALL_1(ApplyTextureEXT, mode);
}

GLAPI GLboolean APIENTRY glAreProgramsResidentNV( GLsizei n, const GLuint* programs, GLboolean* residences) {
    CALL_3_R(AreProgramsResidentNV, n, programs, residences);
}

GLAPI GLboolean APIENTRY glAreTexturesResidentEXT( GLsizei n, const GLuint* textures, GLboolean* residences) {
    CALL_3_R(AreTexturesResidentEXT, n, textures, residences);
}

GLAPI void APIENTRY glArrayElementEXT( GLint i) {
    CALL_1(ArrayElementEXT, i);
}

GLAPI void APIENTRY glArrayObjectATI( GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset) {
    CALL_6(ArrayObjectATI, array, size, type, stride, buffer, offset);
}

GLAPI void APIENTRY glAsyncMarkerSGIX( GLuint marker) {
    CALL_1(AsyncMarkerSGIX, marker);
}

GLAPI void APIENTRY glAttachObjectARB( GLhandleARB containerObj, GLhandleARB obj) {
    CALL_2(AttachObjectARB, containerObj, obj);
}

GLAPI void APIENTRY glAttachShader( GLuint program, GLuint shader) {
    CALL_2(AttachShader, program, shader);
}

GLAPI void APIENTRY glBeginConditionalRender( GLuint id, GLenum mode) {
    CALL_2(BeginConditionalRender, id, mode);
}

GLAPI void APIENTRY glBeginConditionalRenderNV( GLuint id, GLenum mode) {
    CALL_2(BeginConditionalRenderNV, id, mode);
}

GLAPI void APIENTRY glBeginConditionalRenderNVX( GLuint id) {
    CALL_1(BeginConditionalRenderNVX, id);
}

GLAPI void APIENTRY glBeginFragmentShaderATI( void ) {
    CALL_0(BeginFragmentShaderATI);
}

GLAPI void APIENTRY glBeginOcclusionQueryNV( GLuint id) {
    CALL_1(BeginOcclusionQueryNV, id);
}

GLAPI void APIENTRY glBeginPerfMonitorAMD( GLuint monitor) {
    CALL_1(BeginPerfMonitorAMD, monitor);
}

GLAPI void APIENTRY glBeginPerfQueryINTEL( GLuint queryHandle) {
    CALL_1(BeginPerfQueryINTEL, queryHandle);
}

GLAPI void APIENTRY glBeginQuery( GLenum target, GLuint id) {
    CALL_2(BeginQuery, target, id);
}

GLAPI void APIENTRY glBeginQueryARB( GLenum target, GLuint id) {
    CALL_2(BeginQueryARB, target, id);
}

GLAPI void APIENTRY glBeginQueryIndexed( GLenum target, GLuint index, GLuint id) {
    CALL_3(BeginQueryIndexed, target, index, id);
}

GLAPI void APIENTRY glBeginTransformFeedback( GLenum primitiveMode) {
    CALL_1(BeginTransformFeedback, primitiveMode);
}

GLAPI void APIENTRY glBeginTransformFeedbackEXT( GLenum primitiveMode) {
    CALL_1(BeginTransformFeedbackEXT, primitiveMode);
}

GLAPI void APIENTRY glBeginTransformFeedbackNV( GLenum primitiveMode) {
    CALL_1(BeginTransformFeedbackNV, primitiveMode);
}

GLAPI void APIENTRY glBeginVertexShaderEXT( void ) {
    CALL_0(BeginVertexShaderEXT);
}

GLAPI void APIENTRY glBeginVideoCaptureNV( GLuint video_capture_slot) {
    CALL_1(BeginVideoCaptureNV, video_capture_slot);
}

GLAPI void APIENTRY glBindAttribLocation( GLuint program, GLuint index, const GLchar* name) {
    CALL_3(BindAttribLocation, program, index, name);
}

GLAPI void APIENTRY glBindAttribLocationARB( GLhandleARB programObj, GLuint index, const GLcharARB* name) {
    CALL_3(BindAttribLocationARB, programObj, index, name);
}

GLAPI void APIENTRY glBindBuffer( GLenum target, GLuint buffer) {
    CALL_2(BindBuffer, target, buffer);
}

GLAPI void APIENTRY glBindBufferARB( GLenum target, GLuint buffer) {
    CALL_2(BindBufferARB, target, buffer);
}

GLAPI void APIENTRY glBindBufferBase( GLenum target, GLuint index, GLuint buffer) {
    CALL_3(BindBufferBase, target, index, buffer);
}

GLAPI void APIENTRY glBindBufferBaseEXT( GLenum target, GLuint index, GLuint buffer) {
    CALL_3(BindBufferBaseEXT, target, index, buffer);
}

GLAPI void APIENTRY glBindBufferBaseNV( GLenum target, GLuint index, GLuint buffer) {
    CALL_3(BindBufferBaseNV, target, index, buffer);
}

GLAPI void APIENTRY glBindBufferOffsetEXT( GLenum target, GLuint index, GLuint buffer, GLintptr offset) {
    CALL_4(BindBufferOffsetEXT, target, index, buffer, offset);
}

GLAPI void APIENTRY glBindBufferOffsetNV( GLenum target, GLuint index, GLuint buffer, GLintptr offset) {
    CALL_4(BindBufferOffsetNV, target, index, buffer, offset);
}

GLAPI void APIENTRY glBindBufferRange( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    CALL_5(BindBufferRange, target, index, buffer, offset, size);
}

GLAPI void APIENTRY glBindBufferRangeEXT( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    CALL_5(BindBufferRangeEXT, target, index, buffer, offset, size);
}

GLAPI void APIENTRY glBindBufferRangeNV( GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    CALL_5(BindBufferRangeNV, target, index, buffer, offset, size);
}

GLAPI void APIENTRY glBindBuffersBase( GLenum target, GLuint first, GLsizei count, const GLuint* buffers) {
    CALL_4(BindBuffersBase, target, first, count, buffers);
}

GLAPI void APIENTRY glBindBuffersRange( GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes) {
    CALL_6(BindBuffersRange, target, first, count, buffers, offsets, sizes);
}

GLAPI void APIENTRY glBindFragDataLocation( GLuint program, GLuint color, const GLchar* name) {
    CALL_3(BindFragDataLocation, program, color, name);
}

GLAPI void APIENTRY glBindFragDataLocationEXT( GLuint program, GLuint color, const GLchar* name) {
    CALL_3(BindFragDataLocationEXT, program, color, name);
}

GLAPI void APIENTRY glBindFragDataLocationIndexed( GLuint program, GLuint colorNumber, GLuint index, const GLchar* name) {
    CALL_4(BindFragDataLocationIndexed, program, colorNumber, index, name);
}

GLAPI void APIENTRY glBindFragmentShaderATI( GLuint id) {
    CALL_1(BindFragmentShaderATI, id);
}

GLAPI void APIENTRY glBindFramebuffer( GLenum target, GLuint framebuffer) {
    CALL_2(BindFramebuffer, target, framebuffer);
}

GLAPI void APIENTRY glBindFramebufferEXT( GLenum target, GLuint framebuffer) {
    CALL_2(BindFramebufferEXT, target, framebuffer);
}

GLAPI void APIENTRY glBindImageTexture( GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) {
    CALL_7(BindImageTexture, unit, texture, level, layered, layer, access, format);
}

GLAPI void APIENTRY glBindImageTextureEXT( GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format) {
    CALL_7(BindImageTextureEXT, index, texture, level, layered, layer, access, format);
}

GLAPI void APIENTRY glBindImageTextures( GLuint first, GLsizei count, const GLuint* textures) {
    CALL_3(BindImageTextures, first, count, textures);
}

GLAPI GLuint APIENTRY glBindLightParameterEXT( GLenum light, GLenum value) {
    CALL_2_R(BindLightParameterEXT, light, value);
}

GLAPI GLuint APIENTRY glBindMaterialParameterEXT( GLenum face, GLenum value) {
    CALL_2_R(BindMaterialParameterEXT, face, value);
}

GLAPI void APIENTRY glBindMultiTextureEXT( GLenum texunit, GLenum target, GLuint texture) {
    CALL_3(BindMultiTextureEXT, texunit, target, texture);
}

GLAPI GLuint APIENTRY glBindParameterEXT( GLenum value) {
    CALL_1_R(BindParameterEXT, value);
}

GLAPI void APIENTRY glBindProgramARB( GLenum target, GLuint program) {
    CALL_2(BindProgramARB, target, program);
}

GLAPI void APIENTRY glBindProgramNV( GLenum target, GLuint id) {
    CALL_2(BindProgramNV, target, id);
}

GLAPI void APIENTRY glBindProgramPipeline( GLuint pipeline) {
    CALL_1(BindProgramPipeline, pipeline);
}

GLAPI void APIENTRY glBindRenderbuffer( GLenum target, GLuint renderbuffer) {
    CALL_2(BindRenderbuffer, target, renderbuffer);
}

GLAPI void APIENTRY glBindRenderbufferEXT( GLenum target, GLuint renderbuffer) {
    CALL_2(BindRenderbufferEXT, target, renderbuffer);
}

GLAPI void APIENTRY glBindSampler( GLuint unit, GLuint sampler) {
    CALL_2(BindSampler, unit, sampler);
}

GLAPI void APIENTRY glBindSamplers( GLuint first, GLsizei count, const GLuint* samplers) {
    CALL_3(BindSamplers, first, count, samplers);
}

GLAPI GLuint APIENTRY glBindTexGenParameterEXT( GLenum unit, GLenum coord, GLenum value) {
    CALL_3_R(BindTexGenParameterEXT, unit, coord, value);
}

GLAPI void APIENTRY glBindTextureEXT( GLenum target, GLuint texture) {
    CALL_2(BindTextureEXT, target, texture);
}

GLAPI void APIENTRY glBindTextureUnit( GLuint unit, GLuint texture) {
    CALL_2(BindTextureUnit, unit, texture);
}

GLAPI GLuint APIENTRY glBindTextureUnitParameterEXT( GLenum unit, GLenum value) {
    CALL_2_R(BindTextureUnitParameterEXT, unit, value);
}

GLAPI void APIENTRY glBindTextures( GLuint first, GLsizei count, const GLuint* textures) {
    CALL_3(BindTextures, first, count, textures);
}

GLAPI void APIENTRY glBindTransformFeedback( GLenum target, GLuint id) {
    CALL_2(BindTransformFeedback, target, id);
}

GLAPI void APIENTRY glBindTransformFeedbackNV( GLenum target, GLuint id) {
    CALL_2(BindTransformFeedbackNV, target, id);
}

GLAPI void APIENTRY glBindVertexArray( GLuint array) {
    CALL_1(BindVertexArray, array);
}

GLAPI void APIENTRY glBindVertexArrayAPPLE( GLuint array) {
    CALL_1(BindVertexArrayAPPLE, array);
}

GLAPI void APIENTRY glBindVertexBuffer( GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {
    CALL_4(BindVertexBuffer, bindingindex, buffer, offset, stride);
}

GLAPI void APIENTRY glBindVertexBuffers( GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides) {
    CALL_5(BindVertexBuffers, first, count, buffers, offsets, strides);
}

GLAPI void APIENTRY glBindVertexShaderEXT( GLuint id) {
    CALL_1(BindVertexShaderEXT, id);
}

GLAPI void APIENTRY glBindVideoCaptureStreamBufferNV( GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset) {
    CALL_4(BindVideoCaptureStreamBufferNV, video_capture_slot, stream, frame_region, offset);
}

GLAPI void APIENTRY glBindVideoCaptureStreamTextureNV( GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture) {
    CALL_5(BindVideoCaptureStreamTextureNV, video_capture_slot, stream, frame_region, target, texture);
}

GLAPI void APIENTRY glBinormal3bEXT( GLbyte bx, GLbyte by, GLbyte bz) {
    CALL_3(Binormal3bEXT, bx, by, bz);
}

GLAPI void APIENTRY glBinormal3bvEXT( const GLbyte* v) {
    CALL_1(Binormal3bvEXT, v);
}

GLAPI void APIENTRY glBinormal3dEXT( GLdouble bx, GLdouble by, GLdouble bz) {
    CALL_3(Binormal3dEXT, D(bx), D(by), D(bz));
}

GLAPI void APIENTRY glBinormal3dvEXT( const GLdouble* v) {
    CALL_1(Binormal3dvEXT, v);
}

GLAPI void APIENTRY glBinormal3fEXT( GLfloat bx, GLfloat by, GLfloat bz) {
    CALL_3(Binormal3fEXT, F(bx), F(by), F(bz));
}

GLAPI void APIENTRY glBinormal3fvEXT( const GLfloat* v) {
    CALL_1(Binormal3fvEXT, v);
}

GLAPI void APIENTRY glBinormal3iEXT( GLint bx, GLint by, GLint bz) {
    CALL_3(Binormal3iEXT, bx, by, bz);
}

GLAPI void APIENTRY glBinormal3ivEXT( const GLint* v) {
    CALL_1(Binormal3ivEXT, v);
}

GLAPI void APIENTRY glBinormal3sEXT( GLshort bx, GLshort by, GLshort bz) {
    CALL_3(Binormal3sEXT, bx, by, bz);
}

GLAPI void APIENTRY glBinormal3svEXT( const GLshort* v) {
    CALL_1(Binormal3svEXT, v);
}

GLAPI void APIENTRY glBinormalPointerEXT( GLenum type, GLsizei stride, const void* pointer) {
    CALL_3(BinormalPointerEXT, type, stride, pointer);
}

GLAPI void APIENTRY glBitmapxOES( GLsizei width, GLsizei height, GLfixed xorig, GLfixed yorig, GLfixed xmove, GLfixed ymove, const GLubyte* bitmap) {
    CALL_7(BitmapxOES, width, height, xorig, yorig, xmove, ymove, bitmap);
}

GLAPI void APIENTRY glBlendBarrierKHR( void ) {
    CALL_0(BlendBarrierKHR);
}

GLAPI void APIENTRY glBlendBarrierNV( void ) {
    CALL_0(BlendBarrierNV);
}

GLAPI void APIENTRY glBlendColor( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    CALL_4(BlendColor, F(red), F(green), F(blue), F(alpha));
}

GLAPI void APIENTRY glBlendColorEXT( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    CALL_4(BlendColorEXT, F(red), F(green), F(blue), F(alpha));
}

GLAPI void APIENTRY glBlendColorxOES( GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    CALL_4(BlendColorxOES, red, green, blue, alpha);
}

GLAPI void APIENTRY glBlendEquation( GLenum mode) {
    CALL_1(BlendEquation, mode);
}

GLAPI void APIENTRY glBlendEquationEXT( GLenum mode) {
    CALL_1(BlendEquationEXT, mode);
}

GLAPI void APIENTRY glBlendEquationIndexedAMD( GLuint buf, GLenum mode) {
    CALL_2(BlendEquationIndexedAMD, buf, mode);
}

GLAPI void APIENTRY glBlendEquationSeparate( GLenum modeRGB, GLenum modeAlpha) {
    CALL_2(BlendEquationSeparate, modeRGB, modeAlpha);
}

GLAPI void APIENTRY glBlendEquationSeparateEXT( GLenum modeRGB, GLenum modeAlpha) {
    CALL_2(BlendEquationSeparateEXT, modeRGB, modeAlpha);
}

GLAPI void APIENTRY glBlendEquationSeparateIndexedAMD( GLuint buf, GLenum modeRGB, GLenum modeAlpha) {
    CALL_3(BlendEquationSeparateIndexedAMD, buf, modeRGB, modeAlpha);
}

GLAPI void APIENTRY glBlendEquationSeparatei( GLuint buf, GLenum modeRGB, GLenum modeAlpha) {
    CALL_3(BlendEquationSeparatei, buf, modeRGB, modeAlpha);
}

GLAPI void APIENTRY glBlendEquationSeparateiARB( GLuint buf, GLenum modeRGB, GLenum modeAlpha) {
    CALL_3(BlendEquationSeparateiARB, buf, modeRGB, modeAlpha);
}

GLAPI void APIENTRY glBlendEquationi( GLuint buf, GLenum mode) {
    CALL_2(BlendEquationi, buf, mode);
}

GLAPI void APIENTRY glBlendEquationiARB( GLuint buf, GLenum mode) {
    CALL_2(BlendEquationiARB, buf, mode);
}

GLAPI void APIENTRY glBlendFuncIndexedAMD( GLuint buf, GLenum src, GLenum dst) {
    CALL_3(BlendFuncIndexedAMD, buf, src, dst);
}

GLAPI void APIENTRY glBlendFuncSeparate( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    CALL_4(BlendFuncSeparate, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

GLAPI void APIENTRY glBlendFuncSeparateEXT( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    CALL_4(BlendFuncSeparateEXT, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

GLAPI void APIENTRY glBlendFuncSeparateINGR( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    CALL_4(BlendFuncSeparateINGR, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

GLAPI void APIENTRY glBlendFuncSeparateIndexedAMD( GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    CALL_5(BlendFuncSeparateIndexedAMD, buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

GLAPI void APIENTRY glBlendFuncSeparatei( GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    CALL_5(BlendFuncSeparatei, buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

GLAPI void APIENTRY glBlendFuncSeparateiARB( GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    CALL_5(BlendFuncSeparateiARB, buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

GLAPI void APIENTRY glBlendFunci( GLuint buf, GLenum src, GLenum dst) {
    CALL_3(BlendFunci, buf, src, dst);
}

GLAPI void APIENTRY glBlendFunciARB( GLuint buf, GLenum src, GLenum dst) {
    CALL_3(BlendFunciARB, buf, src, dst);
}

GLAPI void APIENTRY glBlendParameteriNV( GLenum pname, GLint value) {
    CALL_2(BlendParameteriNV, pname, value);
}

GLAPI void APIENTRY glBlitFramebuffer( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
    CALL_10(BlitFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLAPI void APIENTRY glBlitFramebufferEXT( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
    CALL_10(BlitFramebufferEXT, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLAPI void APIENTRY glBlitNamedFramebuffer( GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
    CALL_12(BlitNamedFramebuffer, readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLAPI void APIENTRY glBufferAddressRangeNV( GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length) {
    CALL_4(BufferAddressRangeNV, pname, index, LL(address), length);
}

GLAPI void APIENTRY glBufferData( GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    CALL_4(BufferData, target, size, data, usage);
}

GLAPI void APIENTRY glBufferDataARB( GLenum target, GLsizeiptrARB size, const void* data, GLenum usage) {
    CALL_4(BufferDataARB, target, size, data, usage);
}

GLAPI void APIENTRY glBufferPageCommitmentARB( GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit) {
    CALL_4(BufferPageCommitmentARB, target, offset, size, commit);
}

GLAPI void APIENTRY glBufferParameteriAPPLE( GLenum target, GLenum pname, GLint param) {
    CALL_3(BufferParameteriAPPLE, target, pname, param);
}

GLAPI GLuint APIENTRY glBufferRegionEnabled( void ) {
    CALL_0_R(BufferRegionEnabled);
}

GLAPI void APIENTRY glBufferStorage( GLenum target, GLsizeiptr size, const void* data, GLbitfield flags) {
    CALL_4(BufferStorage, target, size, data, flags);
}

GLAPI void APIENTRY glBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
    CALL_4(BufferSubData, target, offset, size, data);
}

GLAPI void APIENTRY glBufferSubDataARB( GLenum target, GLintptrARB offset, GLsizeiptrARB size, const void* data) {
    CALL_4(BufferSubDataARB, target, offset, size, data);
}

GLAPI void APIENTRY glCallCommandListNV( GLuint list) {
    CALL_1(CallCommandListNV, list);
}

GLAPI GLenum APIENTRY glCheckFramebufferStatus( GLenum target) {
    CALL_1_R(CheckFramebufferStatus, target);
}

GLAPI GLenum APIENTRY glCheckFramebufferStatusEXT( GLenum target) {
    CALL_1_R(CheckFramebufferStatusEXT, target);
}

GLAPI GLenum APIENTRY glCheckNamedFramebufferStatus( GLuint framebuffer, GLenum target) {
    CALL_2_R(CheckNamedFramebufferStatus, framebuffer, target);
}

GLAPI GLenum APIENTRY glCheckNamedFramebufferStatusEXT( GLuint framebuffer, GLenum target) {
    CALL_2_R(CheckNamedFramebufferStatusEXT, framebuffer, target);
}

GLAPI void APIENTRY glClampColor( GLenum target, GLenum clamp) {
    CALL_2(ClampColor, target, clamp);
}

GLAPI void APIENTRY glClampColorARB( GLenum target, GLenum clamp) {
    CALL_2(ClampColorARB, target, clamp);
}

GLAPI void APIENTRY glClearAccumxOES( GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    CALL_4(ClearAccumxOES, red, green, blue, alpha);
}

GLAPI void APIENTRY glClearBufferData( GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data) {
    CALL_5(ClearBufferData, target, internalformat, format, type, data);
}

GLAPI void APIENTRY glClearBufferSubData( GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) {
    CALL_7(ClearBufferSubData, target, internalformat, offset, size, format, type, data);
}

GLAPI void APIENTRY glClearBufferfi( GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
    CALL_4(ClearBufferfi, buffer, drawbuffer, F(depth), stencil);
}

GLAPI void APIENTRY glClearBufferfv( GLenum buffer, GLint drawbuffer, const GLfloat* value) {
    CALL_3(ClearBufferfv, buffer, drawbuffer, value);
}

GLAPI void APIENTRY glClearBufferiv( GLenum buffer, GLint drawbuffer, const GLint* value) {
    CALL_3(ClearBufferiv, buffer, drawbuffer, value);
}

GLAPI void APIENTRY glClearBufferuiv( GLenum buffer, GLint drawbuffer, const GLuint* value) {
    CALL_3(ClearBufferuiv, buffer, drawbuffer, value);
}

GLAPI void APIENTRY glClearColorIiEXT( GLint red, GLint green, GLint blue, GLint alpha) {
    CALL_4(ClearColorIiEXT, red, green, blue, alpha);
}

GLAPI void APIENTRY glClearColorIuiEXT( GLuint red, GLuint green, GLuint blue, GLuint alpha) {
    CALL_4(ClearColorIuiEXT, red, green, blue, alpha);
}

GLAPI void APIENTRY glClearColorxOES( GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    CALL_4(ClearColorxOES, red, green, blue, alpha);
}

GLAPI void APIENTRY glClearDepthdNV( GLdouble depth) {
    CALL_1(ClearDepthdNV, D(depth));
}

GLAPI void APIENTRY glClearDepthf( GLfloat d) {
    CALL_1(ClearDepthf, F(d));
}

GLAPI void APIENTRY glClearDepthfOES( GLclampf depth) {
    CALL_1(ClearDepthfOES, F(depth));
}

GLAPI void APIENTRY glClearDepthxOES( GLfixed depth) {
    CALL_1(ClearDepthxOES, depth);
}

GLAPI void APIENTRY glClearNamedBufferData( GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data) {
    CALL_5(ClearNamedBufferData, buffer, internalformat, format, type, data);
}

GLAPI void APIENTRY glClearNamedBufferDataEXT( GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data) {
    CALL_5(ClearNamedBufferDataEXT, buffer, internalformat, format, type, data);
}

GLAPI void APIENTRY glClearNamedBufferSubData( GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) {
    CALL_7(ClearNamedBufferSubData, buffer, internalformat, offset, size, format, type, data);
}

GLAPI void APIENTRY glClearNamedBufferSubDataEXT( GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) {
    CALL_7(ClearNamedBufferSubDataEXT, buffer, internalformat, offset, size, format, type, data);
}

GLAPI void APIENTRY glClearNamedFramebufferfi( GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
    CALL_5(ClearNamedFramebufferfi, framebuffer, buffer, drawbuffer, F(depth), stencil);
}

GLAPI void APIENTRY glClearNamedFramebufferfv( GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value) {
    CALL_4(ClearNamedFramebufferfv, framebuffer, buffer, drawbuffer, value);
}

GLAPI void APIENTRY glClearNamedFramebufferiv( GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value) {
    CALL_4(ClearNamedFramebufferiv, framebuffer, buffer, drawbuffer, value);
}

GLAPI void APIENTRY glClearNamedFramebufferuiv( GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value) {
    CALL_4(ClearNamedFramebufferuiv, framebuffer, buffer, drawbuffer, value);
}

GLAPI void APIENTRY glClearTexImage( GLuint texture, GLint level, GLenum format, GLenum type, const void* data) {
    CALL_5(ClearTexImage, texture, level, format, type, data);
}

GLAPI void APIENTRY glClearTexSubImage( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data) {
    CALL_11(ClearTexSubImage, texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
}

GLAPI void APIENTRY glClientActiveTexture( GLenum texture) {
    CALL_1(ClientActiveTexture, texture);
}

GLAPI void APIENTRY glClientActiveTextureARB( GLenum texture) {
    CALL_1(ClientActiveTextureARB, texture);
}

GLAPI void APIENTRY glClientActiveVertexStreamATI( GLenum stream) {
    CALL_1(ClientActiveVertexStreamATI, stream);
}

GLAPI void APIENTRY glClientAttribDefaultEXT( GLbitfield mask) {
    CALL_1(ClientAttribDefaultEXT, mask);
}

GLAPI GLenum APIENTRY glClientWaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout) {
    CALL_3_R(ClientWaitSync, sync, flags, LL(timeout));
}

GLAPI void APIENTRY glClipControl( GLenum origin, GLenum depth) {
    CALL_2(ClipControl, origin, depth);
}

GLAPI void APIENTRY glClipPlanefOES( GLenum plane, const GLfloat* equation) {
    CALL_2(ClipPlanefOES, plane, equation);
}

GLAPI void APIENTRY glClipPlanexOES( GLenum plane, const GLfixed* equation) {
    CALL_2(ClipPlanexOES, plane, equation);
}

GLAPI void APIENTRY glColor3fVertex3fSUN( GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z) {
    CALL_6(Color3fVertex3fSUN, F(r), F(g), F(b), F(x), F(y), F(z));
}

GLAPI void APIENTRY glColor3fVertex3fvSUN( const GLfloat* c, const GLfloat* v) {
    CALL_2(Color3fVertex3fvSUN, c, v);
}

GLAPI void APIENTRY glColor3hNV( GLhalfNV red, GLhalfNV green, GLhalfNV blue) {
    CALL_3(Color3hNV, red, green, blue);
}

GLAPI void APIENTRY glColor3hvNV( const GLhalfNV* v) {
    CALL_1(Color3hvNV, v);
}

GLAPI void APIENTRY glColor3xOES( GLfixed red, GLfixed green, GLfixed blue) {
    CALL_3(Color3xOES, red, green, blue);
}

GLAPI void APIENTRY glColor3xvOES( const GLfixed* components) {
    CALL_1(Color3xvOES, components);
}

GLAPI void APIENTRY glColor4fNormal3fVertex3fSUN( GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_10(Color4fNormal3fVertex3fSUN, F(r), F(g), F(b), F(a), F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glColor4fNormal3fVertex3fvSUN( const GLfloat* c, const GLfloat* n, const GLfloat* v) {
    CALL_3(Color4fNormal3fVertex3fvSUN, c, n, v);
}

GLAPI void APIENTRY glColor4hNV( GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha) {
    CALL_4(Color4hNV, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4hvNV( const GLhalfNV* v) {
    CALL_1(Color4hvNV, v);
}

GLAPI void APIENTRY glColor4ubVertex2fSUN( GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y) {
    CALL_6(Color4ubVertex2fSUN, r, g, b, a, F(x), F(y));
}

GLAPI void APIENTRY glColor4ubVertex2fvSUN( const GLubyte* c, const GLfloat* v) {
    CALL_2(Color4ubVertex2fvSUN, c, v);
}

GLAPI void APIENTRY glColor4ubVertex3fSUN( GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z) {
    CALL_7(Color4ubVertex3fSUN, r, g, b, a, F(x), F(y), F(z));
}

GLAPI void APIENTRY glColor4ubVertex3fvSUN( const GLubyte* c, const GLfloat* v) {
    CALL_2(Color4ubVertex3fvSUN, c, v);
}

GLAPI void APIENTRY glColor4xOES( GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) {
    CALL_4(Color4xOES, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4xvOES( const GLfixed* components) {
    CALL_1(Color4xvOES, components);
}

GLAPI void APIENTRY glColorFormatNV( GLint size, GLenum type, GLsizei stride) {
    CALL_3(ColorFormatNV, size, type, stride);
}

GLAPI void APIENTRY glColorFragmentOp1ATI( GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod) {
    CALL_7(ColorFragmentOp1ATI, op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod);
}

GLAPI void APIENTRY glColorFragmentOp2ATI( GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod) {
    CALL_10(ColorFragmentOp2ATI, op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod);
}

GLAPI void APIENTRY glColorFragmentOp3ATI( GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod) {
    CALL_13(ColorFragmentOp3ATI, op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod);
}

GLAPI void APIENTRY glColorMaskIndexedEXT( GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    CALL_5(ColorMaskIndexedEXT, index, r, g, b, a);
}

GLAPI void APIENTRY glColorMaski( GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    CALL_5(ColorMaski, index, r, g, b, a);
}

GLAPI void APIENTRY glColorP3ui( GLenum type, GLuint color) {
    CALL_2(ColorP3ui, type, color);
}

GLAPI void APIENTRY glColorP3uiv( GLenum type, const GLuint* color) {
    CALL_2(ColorP3uiv, type, color);
}

GLAPI void APIENTRY glColorP4ui( GLenum type, GLuint color) {
    CALL_2(ColorP4ui, type, color);
}

GLAPI void APIENTRY glColorP4uiv( GLenum type, const GLuint* color) {
    CALL_2(ColorP4uiv, type, color);
}

GLAPI void APIENTRY glColorPointerEXT( GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    CALL_5(ColorPointerEXT, size, type, stride, count, pointer);
}

GLAPI void APIENTRY glColorPointerListIBM( GLint size, GLenum type, GLint stride, const void** pointer, GLint ptrstride) {
    CALL_5(ColorPointerListIBM, size, type, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glColorPointervINTEL( GLint size, GLenum type, const void** pointer) {
    CALL_3(ColorPointervINTEL, size, type, pointer);
}

GLAPI void APIENTRY glColorSubTable( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void* data) {
    CALL_6(ColorSubTable, target, start, count, format, type, data);
}

GLAPI void APIENTRY glColorSubTableEXT( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void* data) {
    CALL_6(ColorSubTableEXT, target, start, count, format, type, data);
}

GLAPI void APIENTRY glColorTable( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* table) {
    CALL_6(ColorTable, target, internalformat, width, format, type, table);
}

GLAPI void APIENTRY glColorTableEXT( GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void* table) {
    CALL_6(ColorTableEXT, target, internalFormat, width, format, type, table);
}

GLAPI void APIENTRY glColorTableParameterfv( GLenum target, GLenum pname, const GLfloat* params) {
    CALL_3(ColorTableParameterfv, target, pname, params);
}

GLAPI void APIENTRY glColorTableParameterfvSGI( GLenum target, GLenum pname, const GLfloat* params) {
    CALL_3(ColorTableParameterfvSGI, target, pname, params);
}

GLAPI void APIENTRY glColorTableParameteriv( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(ColorTableParameteriv, target, pname, params);
}

GLAPI void APIENTRY glColorTableParameterivSGI( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(ColorTableParameterivSGI, target, pname, params);
}

GLAPI void APIENTRY glColorTableSGI( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* table) {
    CALL_6(ColorTableSGI, target, internalformat, width, format, type, table);
}

GLAPI void APIENTRY glCombinerInputNV( GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage) {
    CALL_6(CombinerInputNV, stage, portion, variable, input, mapping, componentUsage);
}

GLAPI void APIENTRY glCombinerOutputNV( GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum) {
    CALL_10(CombinerOutputNV, stage, portion, abOutput, cdOutput, sumOutput, scale, bias, abDotProduct, cdDotProduct, muxSum);
}

GLAPI void APIENTRY glCombinerParameterfNV( GLenum pname, GLfloat param) {
    CALL_2(CombinerParameterfNV, pname, F(param));
}

GLAPI void APIENTRY glCombinerParameterfvNV( GLenum pname, const GLfloat* params) {
    CALL_2(CombinerParameterfvNV, pname, params);
}

GLAPI void APIENTRY glCombinerParameteriNV( GLenum pname, GLint param) {
    CALL_2(CombinerParameteriNV, pname, param);
}

GLAPI void APIENTRY glCombinerParameterivNV( GLenum pname, const GLint* params) {
    CALL_2(CombinerParameterivNV, pname, params);
}

GLAPI void APIENTRY glCombinerStageParameterfvNV( GLenum stage, GLenum pname, const GLfloat* params) {
    CALL_3(CombinerStageParameterfvNV, stage, pname, params);
}

GLAPI void APIENTRY glCommandListSegmentsNV( GLuint list, GLuint segments) {
    CALL_2(CommandListSegmentsNV, list, segments);
}

GLAPI void APIENTRY glCompileCommandListNV( GLuint list) {
    CALL_1(CompileCommandListNV, list);
}

GLAPI void APIENTRY glCompileShader( GLuint shader) {
    CALL_1(CompileShader, shader);
}

GLAPI void APIENTRY glCompileShaderARB( GLhandleARB shaderObj) {
    CALL_1(CompileShaderARB, shaderObj);
}

GLAPI void APIENTRY glCompileShaderIncludeARB( GLuint shader, GLsizei count, const GLchar*const* path, const GLint* length) {
    CALL_4(CompileShaderIncludeARB, shader, count, path, length);
}

GLAPI void APIENTRY glCompressedMultiTexImage1DEXT( GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits) {
    CALL_8(CompressedMultiTexImage1DEXT, texunit, target, level, internalformat, width, border, imageSize, bits);
}

GLAPI void APIENTRY glCompressedMultiTexImage2DEXT( GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits) {
    CALL_9(CompressedMultiTexImage2DEXT, texunit, target, level, internalformat, width, height, border, imageSize, bits);
}

GLAPI void APIENTRY glCompressedMultiTexImage3DEXT( GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits) {
    CALL_10(CompressedMultiTexImage3DEXT, texunit, target, level, internalformat, width, height, depth, border, imageSize, bits);
}

GLAPI void APIENTRY glCompressedMultiTexSubImage1DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits) {
    CALL_8(CompressedMultiTexSubImage1DEXT, texunit, target, level, xoffset, width, format, imageSize, bits);
}

GLAPI void APIENTRY glCompressedMultiTexSubImage2DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits) {
    CALL_10(CompressedMultiTexSubImage2DEXT, texunit, target, level, xoffset, yoffset, width, height, format, imageSize, bits);
}

GLAPI void APIENTRY glCompressedMultiTexSubImage3DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits) {
    CALL_12(CompressedMultiTexSubImage3DEXT, texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
}

GLAPI void APIENTRY glCompressedTexImage1D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data) {
    CALL_7(CompressedTexImage1D, target, level, internalformat, width, border, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexImage1DARB( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data) {
    CALL_7(CompressedTexImage1DARB, target, level, internalformat, width, border, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) {
    CALL_8(CompressedTexImage2D, target, level, internalformat, width, height, border, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexImage2DARB( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) {
    CALL_8(CompressedTexImage2DARB, target, level, internalformat, width, height, border, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data) {
    CALL_9(CompressedTexImage3D, target, level, internalformat, width, height, depth, border, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexImage3DARB( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data) {
    CALL_9(CompressedTexImage3DARB, target, level, internalformat, width, height, depth, border, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data) {
    CALL_7(CompressedTexSubImage1D, target, level, xoffset, width, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexSubImage1DARB( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data) {
    CALL_7(CompressedTexSubImage1DARB, target, level, xoffset, width, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
    CALL_9(CompressedTexSubImage2D, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexSubImage2DARB( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
    CALL_9(CompressedTexSubImage2DARB, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) {
    CALL_11(CompressedTexSubImage3D, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTexSubImage3DARB( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) {
    CALL_11(CompressedTexSubImage3DARB, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTextureImage1DEXT( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits) {
    CALL_8(CompressedTextureImage1DEXT, texture, target, level, internalformat, width, border, imageSize, bits);
}

GLAPI void APIENTRY glCompressedTextureImage2DEXT( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits) {
    CALL_9(CompressedTextureImage2DEXT, texture, target, level, internalformat, width, height, border, imageSize, bits);
}

GLAPI void APIENTRY glCompressedTextureImage3DEXT( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits) {
    CALL_10(CompressedTextureImage3DEXT, texture, target, level, internalformat, width, height, depth, border, imageSize, bits);
}

GLAPI void APIENTRY glCompressedTextureSubImage1D( GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data) {
    CALL_7(CompressedTextureSubImage1D, texture, level, xoffset, width, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTextureSubImage1DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits) {
    CALL_8(CompressedTextureSubImage1DEXT, texture, target, level, xoffset, width, format, imageSize, bits);
}

GLAPI void APIENTRY glCompressedTextureSubImage2D( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) {
    CALL_9(CompressedTextureSubImage2D, texture, level, xoffset, yoffset, width, height, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTextureSubImage2DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits) {
    CALL_10(CompressedTextureSubImage2DEXT, texture, target, level, xoffset, yoffset, width, height, format, imageSize, bits);
}

GLAPI void APIENTRY glCompressedTextureSubImage3D( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) {
    CALL_11(CompressedTextureSubImage3D, texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

GLAPI void APIENTRY glCompressedTextureSubImage3DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits) {
    CALL_12(CompressedTextureSubImage3DEXT, texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
}

GLAPI void APIENTRY glConservativeRasterParameterfNV( GLenum pname, GLfloat value) {
    CALL_2(ConservativeRasterParameterfNV, pname, F(value));
}

GLAPI void APIENTRY glConvolutionFilter1D( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* image) {
    CALL_6(ConvolutionFilter1D, target, internalformat, width, format, type, image);
}

GLAPI void APIENTRY glConvolutionFilter1DEXT( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* image) {
    CALL_6(ConvolutionFilter1DEXT, target, internalformat, width, format, type, image);
}

GLAPI void APIENTRY glConvolutionFilter2D( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* image) {
    CALL_7(ConvolutionFilter2D, target, internalformat, width, height, format, type, image);
}

GLAPI void APIENTRY glConvolutionFilter2DEXT( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* image) {
    CALL_7(ConvolutionFilter2DEXT, target, internalformat, width, height, format, type, image);
}

GLAPI void APIENTRY glConvolutionParameterf( GLenum target, GLenum pname, GLfloat params) {
    CALL_3(ConvolutionParameterf, target, pname, F(params));
}

GLAPI void APIENTRY glConvolutionParameterfEXT( GLenum target, GLenum pname, GLfloat params) {
    CALL_3(ConvolutionParameterfEXT, target, pname, F(params));
}

GLAPI void APIENTRY glConvolutionParameterfv( GLenum target, GLenum pname, const GLfloat* params) {
    CALL_3(ConvolutionParameterfv, target, pname, params);
}

GLAPI void APIENTRY glConvolutionParameterfvEXT( GLenum target, GLenum pname, const GLfloat* params) {
    CALL_3(ConvolutionParameterfvEXT, target, pname, params);
}

GLAPI void APIENTRY glConvolutionParameteri( GLenum target, GLenum pname, GLint params) {
    CALL_3(ConvolutionParameteri, target, pname, params);
}

GLAPI void APIENTRY glConvolutionParameteriEXT( GLenum target, GLenum pname, GLint params) {
    CALL_3(ConvolutionParameteriEXT, target, pname, params);
}

GLAPI void APIENTRY glConvolutionParameteriv( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(ConvolutionParameteriv, target, pname, params);
}

GLAPI void APIENTRY glConvolutionParameterivEXT( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(ConvolutionParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glConvolutionParameterxOES( GLenum target, GLenum pname, GLfixed param) {
    CALL_3(ConvolutionParameterxOES, target, pname, param);
}

GLAPI void APIENTRY glConvolutionParameterxvOES( GLenum target, GLenum pname, const GLfixed* params) {
    CALL_3(ConvolutionParameterxvOES, target, pname, params);
}

GLAPI void APIENTRY glCopyBufferSubData( GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
    CALL_5(CopyBufferSubData, readTarget, writeTarget, readOffset, writeOffset, size);
}

GLAPI void APIENTRY glCopyColorSubTable( GLenum target, GLsizei start, GLint x, GLint y, GLsizei width) {
    CALL_5(CopyColorSubTable, target, start, x, y, width);
}

GLAPI void APIENTRY glCopyColorSubTableEXT( GLenum target, GLsizei start, GLint x, GLint y, GLsizei width) {
    CALL_5(CopyColorSubTableEXT, target, start, x, y, width);
}

GLAPI void APIENTRY glCopyColorTable( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width) {
    CALL_5(CopyColorTable, target, internalformat, x, y, width);
}

GLAPI void APIENTRY glCopyColorTableSGI( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width) {
    CALL_5(CopyColorTableSGI, target, internalformat, x, y, width);
}

GLAPI void APIENTRY glCopyConvolutionFilter1D( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width) {
    CALL_5(CopyConvolutionFilter1D, target, internalformat, x, y, width);
}

GLAPI void APIENTRY glCopyConvolutionFilter1DEXT( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width) {
    CALL_5(CopyConvolutionFilter1DEXT, target, internalformat, x, y, width);
}

GLAPI void APIENTRY glCopyConvolutionFilter2D( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_6(CopyConvolutionFilter2D, target, internalformat, x, y, width, height);
}

GLAPI void APIENTRY glCopyConvolutionFilter2DEXT( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_6(CopyConvolutionFilter2DEXT, target, internalformat, x, y, width, height);
}

GLAPI void APIENTRY glCopyImageSubData( GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) {
    CALL_15(CopyImageSubData, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}

GLAPI void APIENTRY glCopyImageSubDataNV( GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth) {
    CALL_15(CopyImageSubDataNV, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, width, height, depth);
}

GLAPI void APIENTRY glCopyMultiTexImage1DEXT( GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    CALL_8(CopyMultiTexImage1DEXT, texunit, target, level, internalformat, x, y, width, border);
}

GLAPI void APIENTRY glCopyMultiTexImage2DEXT( GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    CALL_9(CopyMultiTexImage2DEXT, texunit, target, level, internalformat, x, y, width, height, border);
}

GLAPI void APIENTRY glCopyMultiTexSubImage1DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    CALL_7(CopyMultiTexSubImage1DEXT, texunit, target, level, xoffset, x, y, width);
}

GLAPI void APIENTRY glCopyMultiTexSubImage2DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_9(CopyMultiTexSubImage2DEXT, texunit, target, level, xoffset, yoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyMultiTexSubImage3DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_10(CopyMultiTexSubImage3DEXT, texunit, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyNamedBufferSubData( GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
    CALL_5(CopyNamedBufferSubData, readBuffer, writeBuffer, readOffset, writeOffset, size);
}

GLAPI void APIENTRY glCopyPathNV( GLuint resultPath, GLuint srcPath) {
    CALL_2(CopyPathNV, resultPath, srcPath);
}

GLAPI void APIENTRY glCopyTexImage1DEXT( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    CALL_7(CopyTexImage1DEXT, target, level, internalformat, x, y, width, border);
}

GLAPI void APIENTRY glCopyTexImage2DEXT( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    CALL_8(CopyTexImage2DEXT, target, level, internalformat, x, y, width, height, border);
}

GLAPI void APIENTRY glCopyTexSubImage1DEXT( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    CALL_6(CopyTexSubImage1DEXT, target, level, xoffset, x, y, width);
}

GLAPI void APIENTRY glCopyTexSubImage2DEXT( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_8(CopyTexSubImage2DEXT, target, level, xoffset, yoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_9(CopyTexSubImage3D, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyTexSubImage3DEXT( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_9(CopyTexSubImage3DEXT, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyTextureImage1DEXT( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    CALL_8(CopyTextureImage1DEXT, texture, target, level, internalformat, x, y, width, border);
}

GLAPI void APIENTRY glCopyTextureImage2DEXT( GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    CALL_9(CopyTextureImage2DEXT, texture, target, level, internalformat, x, y, width, height, border);
}

GLAPI void APIENTRY glCopyTextureSubImage1D( GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    CALL_6(CopyTextureSubImage1D, texture, level, xoffset, x, y, width);
}

GLAPI void APIENTRY glCopyTextureSubImage1DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    CALL_7(CopyTextureSubImage1DEXT, texture, target, level, xoffset, x, y, width);
}

GLAPI void APIENTRY glCopyTextureSubImage2D( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_8(CopyTextureSubImage2D, texture, level, xoffset, yoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyTextureSubImage2DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_9(CopyTextureSubImage2DEXT, texture, target, level, xoffset, yoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyTextureSubImage3D( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_9(CopyTextureSubImage3D, texture, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLAPI void APIENTRY glCopyTextureSubImage3DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_10(CopyTextureSubImage3DEXT, texture, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLAPI void APIENTRY glCoverFillPathInstancedNV( GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) {
    CALL_7(CoverFillPathInstancedNV, numPaths, pathNameType, paths, pathBase, coverMode, transformType, transformValues);
}

GLAPI void APIENTRY glCoverFillPathNV( GLuint path, GLenum coverMode) {
    CALL_2(CoverFillPathNV, path, coverMode);
}

GLAPI void APIENTRY glCoverStrokePathInstancedNV( GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) {
    CALL_7(CoverStrokePathInstancedNV, numPaths, pathNameType, paths, pathBase, coverMode, transformType, transformValues);
}

GLAPI void APIENTRY glCoverStrokePathNV( GLuint path, GLenum coverMode) {
    CALL_2(CoverStrokePathNV, path, coverMode);
}

GLAPI void APIENTRY glCoverageModulationNV( GLenum components) {
    CALL_1(CoverageModulationNV, components);
}

GLAPI void APIENTRY glCoverageModulationTableNV( GLsizei n, const GLfloat* v) {
    CALL_2(CoverageModulationTableNV, n, v);
}

GLAPI void APIENTRY glCreateBuffers( GLsizei n, GLuint* buffers) {
    CALL_2(CreateBuffers, n, buffers);
}

GLAPI void APIENTRY glCreateCommandListsNV( GLsizei n, GLuint* lists) {
    CALL_2(CreateCommandListsNV, n, lists);
}

GLAPI void APIENTRY glCreateFramebuffers( GLsizei n, GLuint* framebuffers) {
    CALL_2(CreateFramebuffers, n, framebuffers);
}

GLAPI void APIENTRY glCreatePerfQueryINTEL( GLuint queryId, GLuint* queryHandle) {
    CALL_2(CreatePerfQueryINTEL, queryId, queryHandle);
}

GLAPI GLuint APIENTRY glCreateProgram( void ) {
    CALL_0_R(CreateProgram);
}

GLAPI GLhandleARB APIENTRY glCreateProgramObjectARB( void ) {
    CALL_0_R(CreateProgramObjectARB);
}

GLAPI void APIENTRY glCreateProgramPipelines( GLsizei n, GLuint* pipelines) {
    CALL_2(CreateProgramPipelines, n, pipelines);
}

GLAPI void APIENTRY glCreateQueries( GLenum target, GLsizei n, GLuint* ids) {
    CALL_3(CreateQueries, target, n, ids);
}

GLAPI void APIENTRY glCreateRenderbuffers( GLsizei n, GLuint* renderbuffers) {
    CALL_2(CreateRenderbuffers, n, renderbuffers);
}

GLAPI void APIENTRY glCreateSamplers( GLsizei n, GLuint* samplers) {
    CALL_2(CreateSamplers, n, samplers);
}

GLAPI GLuint APIENTRY glCreateShader( GLenum type) {
    CALL_1_R(CreateShader, type);
}

GLAPI GLhandleARB APIENTRY glCreateShaderObjectARB( GLenum shaderType) {
    CALL_1_R(CreateShaderObjectARB, shaderType);
}

GLAPI GLuint APIENTRY glCreateShaderProgramEXT( GLenum type, const GLchar* string) {
    CALL_2_R(CreateShaderProgramEXT, type, string);
}

GLAPI GLuint APIENTRY glCreateShaderProgramv( GLenum type, GLsizei count, const GLchar*const* strings) {
    CALL_3_R(CreateShaderProgramv, type, count, strings);
}

GLAPI void APIENTRY glCreateStatesNV( GLsizei n, GLuint* states) {
    CALL_2(CreateStatesNV, n, states);
}

GLAPI GLsync APIENTRY glCreateSyncFromCLeventARB( void* context, void* event, GLbitfield flags) {
    CALL_3_R(CreateSyncFromCLeventARB, context, event, flags);
}

GLAPI void APIENTRY glCreateTextures( GLenum target, GLsizei n, GLuint* textures) {
    CALL_3(CreateTextures, target, n, textures);
}

GLAPI void APIENTRY glCreateTransformFeedbacks( GLsizei n, GLuint* ids) {
    CALL_2(CreateTransformFeedbacks, n, ids);
}

GLAPI void APIENTRY glCreateVertexArrays( GLsizei n, GLuint* arrays) {
    CALL_2(CreateVertexArrays, n, arrays);
}

GLAPI void APIENTRY glCullParameterdvEXT( GLenum pname, GLdouble* params) {
    CALL_2(CullParameterdvEXT, pname, params);
}

GLAPI void APIENTRY glCullParameterfvEXT( GLenum pname, GLfloat* params) {
    CALL_2(CullParameterfvEXT, pname, params);
}

GLAPI void APIENTRY glCurrentPaletteMatrixARB( GLint index) {
    CALL_1(CurrentPaletteMatrixARB, index);
}

GLAPI void APIENTRY glDebugMessageCallback( void * callback, const void* userParam) {
    CALL_2(DebugMessageCallback, callback, userParam);
}

GLAPI void APIENTRY glDebugMessageCallbackAMD( void * callback, void* userParam) {
    CALL_2(DebugMessageCallbackAMD, callback, userParam);
}

GLAPI void APIENTRY glDebugMessageCallbackARB( void * callback, const void* userParam) {
    CALL_2(DebugMessageCallbackARB, callback, userParam);
}

GLAPI void APIENTRY glDebugMessageControl( GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) {
    CALL_6(DebugMessageControl, source, type, severity, count, ids, enabled);
}

GLAPI void APIENTRY glDebugMessageControlARB( GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) {
    CALL_6(DebugMessageControlARB, source, type, severity, count, ids, enabled);
}

GLAPI void APIENTRY glDebugMessageEnableAMD( GLenum category, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) {
    CALL_5(DebugMessageEnableAMD, category, severity, count, ids, enabled);
}

GLAPI void APIENTRY glDebugMessageInsert( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf) {
    CALL_6(DebugMessageInsert, source, type, id, severity, length, buf);
}

GLAPI void APIENTRY glDebugMessageInsertAMD( GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar* buf) {
    CALL_5(DebugMessageInsertAMD, category, severity, id, length, buf);
}

GLAPI void APIENTRY glDebugMessageInsertARB( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf) {
    CALL_6(DebugMessageInsertARB, source, type, id, severity, length, buf);
}

GLAPI void APIENTRY glDeformSGIX( GLbitfield mask) {
    CALL_1(DeformSGIX, mask);
}

GLAPI void APIENTRY glDeformationMap3dSGIX( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble* points) {
    CALL_14(DeformationMap3dSGIX, target, D(u1), D(u2), ustride, uorder, D(v1), D(v2), vstride, vorder, D(w1), D(w2), wstride, worder, points);
}

GLAPI void APIENTRY glDeformationMap3fSGIX( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat* points) {
    CALL_14(DeformationMap3fSGIX, target, F(u1), F(u2), ustride, uorder, F(v1), F(v2), vstride, vorder, F(w1), F(w2), wstride, worder, points);
}

GLAPI void APIENTRY glDeleteAsyncMarkersSGIX( GLuint marker, GLsizei range) {
    CALL_2(DeleteAsyncMarkersSGIX, marker, range);
}

GLAPI void APIENTRY glDeleteBufferRegion( GLenum region) {
    CALL_1(DeleteBufferRegion, region);
}

GLAPI void APIENTRY glDeleteBuffers( GLsizei n, const GLuint* buffers) {
    CALL_2(DeleteBuffers, n, buffers);
}

GLAPI void APIENTRY glDeleteBuffersARB( GLsizei n, const GLuint* buffers) {
    CALL_2(DeleteBuffersARB, n, buffers);
}

GLAPI void APIENTRY glDeleteCommandListsNV( GLsizei n, const GLuint* lists) {
    CALL_2(DeleteCommandListsNV, n, lists);
}

GLAPI void APIENTRY glDeleteFencesAPPLE( GLsizei n, const GLuint* fences) {
    CALL_2(DeleteFencesAPPLE, n, fences);
}

GLAPI void APIENTRY glDeleteFencesNV( GLsizei n, const GLuint* fences) {
    CALL_2(DeleteFencesNV, n, fences);
}

GLAPI void APIENTRY glDeleteFragmentShaderATI( GLuint id) {
    CALL_1(DeleteFragmentShaderATI, id);
}

GLAPI void APIENTRY glDeleteFramebuffers( GLsizei n, const GLuint* framebuffers) {
    CALL_2(DeleteFramebuffers, n, framebuffers);
}

GLAPI void APIENTRY glDeleteFramebuffersEXT( GLsizei n, const GLuint* framebuffers) {
    CALL_2(DeleteFramebuffersEXT, n, framebuffers);
}

GLAPI void APIENTRY glDeleteNamedStringARB( GLint namelen, const GLchar* name) {
    CALL_2(DeleteNamedStringARB, namelen, name);
}

GLAPI void APIENTRY glDeleteNamesAMD( GLenum identifier, GLuint num, const GLuint* names) {
    CALL_3(DeleteNamesAMD, identifier, num, names);
}

GLAPI void APIENTRY glDeleteObjectARB( GLhandleARB obj) {
    CALL_1(DeleteObjectARB, obj);
}

GLAPI void APIENTRY glDeleteObjectBufferATI( GLuint buffer) {
    CALL_1(DeleteObjectBufferATI, buffer);
}

GLAPI void APIENTRY glDeleteOcclusionQueriesNV( GLsizei n, const GLuint* ids) {
    CALL_2(DeleteOcclusionQueriesNV, n, ids);
}

GLAPI void APIENTRY glDeletePathsNV( GLuint path, GLsizei range) {
    CALL_2(DeletePathsNV, path, range);
}

GLAPI void APIENTRY glDeletePerfMonitorsAMD( GLsizei n, GLuint* monitors) {
    CALL_2(DeletePerfMonitorsAMD, n, monitors);
}

GLAPI void APIENTRY glDeletePerfQueryINTEL( GLuint queryHandle) {
    CALL_1(DeletePerfQueryINTEL, queryHandle);
}

GLAPI void APIENTRY glDeleteProgram( GLuint program) {
    CALL_1(DeleteProgram, program);
}

GLAPI void APIENTRY glDeleteProgramPipelines( GLsizei n, const GLuint* pipelines) {
    CALL_2(DeleteProgramPipelines, n, pipelines);
}

GLAPI void APIENTRY glDeleteProgramsARB( GLsizei n, const GLuint* programs) {
    CALL_2(DeleteProgramsARB, n, programs);
}

GLAPI void APIENTRY glDeleteProgramsNV( GLsizei n, const GLuint* programs) {
    CALL_2(DeleteProgramsNV, n, programs);
}

GLAPI void APIENTRY glDeleteQueries( GLsizei n, const GLuint* ids) {
    CALL_2(DeleteQueries, n, ids);
}

GLAPI void APIENTRY glDeleteQueriesARB( GLsizei n, const GLuint* ids) {
    CALL_2(DeleteQueriesARB, n, ids);
}

GLAPI void APIENTRY glDeleteRenderbuffers( GLsizei n, const GLuint* renderbuffers) {
    CALL_2(DeleteRenderbuffers, n, renderbuffers);
}

GLAPI void APIENTRY glDeleteRenderbuffersEXT( GLsizei n, const GLuint* renderbuffers) {
    CALL_2(DeleteRenderbuffersEXT, n, renderbuffers);
}

GLAPI void APIENTRY glDeleteSamplers( GLsizei count, const GLuint* samplers) {
    CALL_2(DeleteSamplers, count, samplers);
}

GLAPI void APIENTRY glDeleteShader( GLuint shader) {
    CALL_1(DeleteShader, shader);
}

GLAPI void APIENTRY glDeleteStatesNV( GLsizei n, const GLuint* states) {
    CALL_2(DeleteStatesNV, n, states);
}

GLAPI void APIENTRY glDeleteSync( GLsync sync) {
    CALL_1(DeleteSync, sync);
}

GLAPI void APIENTRY glDeleteTexturesEXT( GLsizei n, const GLuint* textures) {
    CALL_2(DeleteTexturesEXT, n, textures);
}

GLAPI void APIENTRY glDeleteTransformFeedbacks( GLsizei n, const GLuint* ids) {
    CALL_2(DeleteTransformFeedbacks, n, ids);
}

GLAPI void APIENTRY glDeleteTransformFeedbacksNV( GLsizei n, const GLuint* ids) {
    CALL_2(DeleteTransformFeedbacksNV, n, ids);
}

GLAPI void APIENTRY glDeleteVertexArrays( GLsizei n, const GLuint* arrays) {
    CALL_2(DeleteVertexArrays, n, arrays);
}

GLAPI void APIENTRY glDeleteVertexArraysAPPLE( GLsizei n, const GLuint* arrays) {
    CALL_2(DeleteVertexArraysAPPLE, n, arrays);
}

GLAPI void APIENTRY glDeleteVertexShaderEXT( GLuint id) {
    CALL_1(DeleteVertexShaderEXT, id);
}

GLAPI void APIENTRY glDepthBoundsEXT( GLclampd zmin, GLclampd zmax) {
    CALL_2(DepthBoundsEXT, D(zmin), D(zmax));
}

GLAPI void APIENTRY glDepthBoundsdNV( GLdouble zmin, GLdouble zmax) {
    CALL_2(DepthBoundsdNV, D(zmin), D(zmax));
}

GLAPI void APIENTRY glDepthRangeArrayv( GLuint first, GLsizei count, const GLdouble* v) {
    CALL_3(DepthRangeArrayv, first, count, v);
}

GLAPI void APIENTRY glDepthRangeIndexed( GLuint index, GLdouble n, GLdouble f) {
    CALL_3(DepthRangeIndexed, index, D(n), D(f));
}

GLAPI void APIENTRY glDepthRangedNV( GLdouble zNear, GLdouble zFar) {
    CALL_2(DepthRangedNV, D(zNear), D(zFar));
}

GLAPI void APIENTRY glDepthRangef( GLfloat n, GLfloat f) {
    CALL_2(DepthRangef, F(n), F(f));
}

GLAPI void APIENTRY glDepthRangefOES( GLclampf n, GLclampf f) {
    CALL_2(DepthRangefOES, F(n), F(f));
}

GLAPI void APIENTRY glDepthRangexOES( GLfixed n, GLfixed f) {
    CALL_2(DepthRangexOES, n, f);
}

GLAPI void APIENTRY glDetachObjectARB( GLhandleARB containerObj, GLhandleARB attachedObj) {
    CALL_2(DetachObjectARB, containerObj, attachedObj);
}

GLAPI void APIENTRY glDetachShader( GLuint program, GLuint shader) {
    CALL_2(DetachShader, program, shader);
}

GLAPI void APIENTRY glDetailTexFuncSGIS( GLenum target, GLsizei n, const GLfloat* points) {
    CALL_3(DetailTexFuncSGIS, target, n, points);
}

GLAPI void APIENTRY glDisableClientStateIndexedEXT( GLenum array, GLuint index) {
    CALL_2(DisableClientStateIndexedEXT, array, index);
}

GLAPI void APIENTRY glDisableClientStateiEXT( GLenum array, GLuint index) {
    CALL_2(DisableClientStateiEXT, array, index);
}

GLAPI void APIENTRY glDisableIndexedEXT( GLenum target, GLuint index) {
    CALL_2(DisableIndexedEXT, target, index);
}

GLAPI void APIENTRY glDisableVariantClientStateEXT( GLuint id) {
    CALL_1(DisableVariantClientStateEXT, id);
}

GLAPI void APIENTRY glDisableVertexArrayAttrib( GLuint vaobj, GLuint index) {
    CALL_2(DisableVertexArrayAttrib, vaobj, index);
}

GLAPI void APIENTRY glDisableVertexArrayAttribEXT( GLuint vaobj, GLuint index) {
    CALL_2(DisableVertexArrayAttribEXT, vaobj, index);
}

GLAPI void APIENTRY glDisableVertexArrayEXT( GLuint vaobj, GLenum array) {
    CALL_2(DisableVertexArrayEXT, vaobj, array);
}

GLAPI void APIENTRY glDisableVertexAttribAPPLE( GLuint index, GLenum pname) {
    CALL_2(DisableVertexAttribAPPLE, index, pname);
}

GLAPI void APIENTRY glDisableVertexAttribArray( GLuint index) {
    CALL_1(DisableVertexAttribArray, index);
}

GLAPI void APIENTRY glDisableVertexAttribArrayARB( GLuint index) {
    CALL_1(DisableVertexAttribArrayARB, index);
}

GLAPI void APIENTRY glDisablei( GLenum target, GLuint index) {
    CALL_2(Disablei, target, index);
}

GLAPI void APIENTRY glDispatchCompute( GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
    CALL_3(DispatchCompute, num_groups_x, num_groups_y, num_groups_z);
}

GLAPI void APIENTRY glDispatchComputeGroupSizeARB( GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z) {
    CALL_6(DispatchComputeGroupSizeARB, num_groups_x, num_groups_y, num_groups_z, group_size_x, group_size_y, group_size_z);
}

GLAPI void APIENTRY glDispatchComputeIndirect( GLintptr indirect) {
    CALL_1(DispatchComputeIndirect, indirect);
}

GLAPI void APIENTRY glDrawArraysEXT( GLenum mode, GLint first, GLsizei count) {
    CALL_3(DrawArraysEXT, mode, first, count);
}

GLAPI void APIENTRY glDrawArraysIndirect( GLenum mode, const void* indirect) {
    CALL_2(DrawArraysIndirect, mode, indirect);
}

GLAPI void APIENTRY glDrawArraysInstanced( GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
    CALL_4(DrawArraysInstanced, mode, first, count, instancecount);
}

GLAPI void APIENTRY glDrawArraysInstancedARB( GLenum mode, GLint first, GLsizei count, GLsizei primcount) {
    CALL_4(DrawArraysInstancedARB, mode, first, count, primcount);
}

GLAPI void APIENTRY glDrawArraysInstancedBaseInstance( GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance) {
    CALL_5(DrawArraysInstancedBaseInstance, mode, first, count, instancecount, baseinstance);
}

GLAPI void APIENTRY glDrawArraysInstancedEXT( GLenum mode, GLint start, GLsizei count, GLsizei primcount) {
    CALL_4(DrawArraysInstancedEXT, mode, start, count, primcount);
}

GLAPI void APIENTRY glDrawBufferRegion( GLenum region, GLint x, GLint y, GLsizei width, GLsizei height, GLint xDest, GLint yDest) {
    CALL_7(DrawBufferRegion, region, x, y, width, height, xDest, yDest);
}

GLAPI void APIENTRY glDrawBuffers( GLsizei n, const GLenum* bufs) {
    CALL_2(DrawBuffers, n, bufs);
}

GLAPI void APIENTRY glDrawBuffersARB( GLsizei n, const GLenum* bufs) {
    CALL_2(DrawBuffersARB, n, bufs);
}

GLAPI void APIENTRY glDrawBuffersATI( GLsizei n, const GLenum* bufs) {
    CALL_2(DrawBuffersATI, n, bufs);
}

GLAPI void APIENTRY glDrawCommandsAddressNV( GLenum primitiveMode, const GLuint64* indirects, const GLsizei* sizes, GLuint count) {
    CALL_4(DrawCommandsAddressNV, primitiveMode, indirects, sizes, count);
}

GLAPI void APIENTRY glDrawCommandsNV( GLenum primitiveMode, GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, GLuint count) {
    CALL_5(DrawCommandsNV, primitiveMode, buffer, indirects, sizes, count);
}

GLAPI void APIENTRY glDrawCommandsStatesAddressNV( const GLuint64* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count) {
    CALL_5(DrawCommandsStatesAddressNV, indirects, sizes, states, fbos, count);
}

GLAPI void APIENTRY glDrawCommandsStatesNV( GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count) {
    CALL_6(DrawCommandsStatesNV, buffer, indirects, sizes, states, fbos, count);
}

GLAPI void APIENTRY glDrawElementArrayAPPLE( GLenum mode, GLint first, GLsizei count) {
    CALL_3(DrawElementArrayAPPLE, mode, first, count);
}

GLAPI void APIENTRY glDrawElementArrayATI( GLenum mode, GLsizei count) {
    CALL_2(DrawElementArrayATI, mode, count);
}

GLAPI void APIENTRY glDrawElementsBaseVertex( GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) {
    CALL_5(DrawElementsBaseVertex, mode, count, type, indices, basevertex);
}

GLAPI void APIENTRY glDrawElementsIndirect( GLenum mode, GLenum type, const void* indirect) {
    CALL_3(DrawElementsIndirect, mode, type, indirect);
}

GLAPI void APIENTRY glDrawElementsInstanced( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) {
    CALL_5(DrawElementsInstanced, mode, count, type, indices, instancecount);
}

GLAPI void APIENTRY glDrawElementsInstancedARB( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) {
    CALL_5(DrawElementsInstancedARB, mode, count, type, indices, primcount);
}

GLAPI void APIENTRY glDrawElementsInstancedBaseInstance( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance) {
    CALL_6(DrawElementsInstancedBaseInstance, mode, count, type, indices, instancecount, baseinstance);
}

GLAPI void APIENTRY glDrawElementsInstancedBaseVertex( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex) {
    CALL_6(DrawElementsInstancedBaseVertex, mode, count, type, indices, instancecount, basevertex);
}

GLAPI void APIENTRY glDrawElementsInstancedBaseVertexBaseInstance( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance) {
    CALL_7(DrawElementsInstancedBaseVertexBaseInstance, mode, count, type, indices, instancecount, basevertex, baseinstance);
}

GLAPI void APIENTRY glDrawElementsInstancedEXT( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) {
    CALL_5(DrawElementsInstancedEXT, mode, count, type, indices, primcount);
}

GLAPI void APIENTRY glDrawMeshArraysSUN( GLenum mode, GLint first, GLsizei count, GLsizei width) {
    CALL_4(DrawMeshArraysSUN, mode, first, count, width);
}

GLAPI void APIENTRY glDrawRangeElementArrayAPPLE( GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count) {
    CALL_5(DrawRangeElementArrayAPPLE, mode, start, end, first, count);
}

GLAPI void APIENTRY glDrawRangeElementArrayATI( GLenum mode, GLuint start, GLuint end, GLsizei count) {
    CALL_4(DrawRangeElementArrayATI, mode, start, end, count);
}

GLAPI void APIENTRY glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {
    CALL_6(DrawRangeElements, mode, start, end, count, type, indices);
}

GLAPI void APIENTRY glDrawRangeElementsBaseVertex( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex) {
    CALL_7(DrawRangeElementsBaseVertex, mode, start, end, count, type, indices, basevertex);
}

GLAPI void APIENTRY glDrawRangeElementsEXT( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {
    CALL_6(DrawRangeElementsEXT, mode, start, end, count, type, indices);
}

GLAPI void APIENTRY glDrawTextureNV( GLuint texture, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1) {
    CALL_11(DrawTextureNV, texture, sampler, F(x0), F(y0), F(x1), F(y1), F(z), F(s0), F(t0), F(s1), F(t1));
}

GLAPI void APIENTRY glDrawTransformFeedback( GLenum mode, GLuint id) {
    CALL_2(DrawTransformFeedback, mode, id);
}

GLAPI void APIENTRY glDrawTransformFeedbackInstanced( GLenum mode, GLuint id, GLsizei instancecount) {
    CALL_3(DrawTransformFeedbackInstanced, mode, id, instancecount);
}

GLAPI void APIENTRY glDrawTransformFeedbackNV( GLenum mode, GLuint id) {
    CALL_2(DrawTransformFeedbackNV, mode, id);
}

GLAPI void APIENTRY glDrawTransformFeedbackStream( GLenum mode, GLuint id, GLuint stream) {
    CALL_3(DrawTransformFeedbackStream, mode, id, stream);
}

GLAPI void APIENTRY glDrawTransformFeedbackStreamInstanced( GLenum mode, GLuint id, GLuint stream, GLsizei instancecount) {
    CALL_4(DrawTransformFeedbackStreamInstanced, mode, id, stream, instancecount);
}

GLAPI void APIENTRY glEdgeFlagFormatNV( GLsizei stride) {
    CALL_1(EdgeFlagFormatNV, stride);
}

GLAPI void APIENTRY glEdgeFlagPointerEXT( GLsizei stride, GLsizei count, const GLboolean* pointer) {
    CALL_3(EdgeFlagPointerEXT, stride, count, pointer);
}

GLAPI void APIENTRY glEdgeFlagPointerListIBM( GLint stride, const GLboolean** pointer, GLint ptrstride) {
    CALL_3(EdgeFlagPointerListIBM, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glElementPointerAPPLE( GLenum type, const void* pointer) {
    CALL_2(ElementPointerAPPLE, type, pointer);
}

GLAPI void APIENTRY glElementPointerATI( GLenum type, const void* pointer) {
    CALL_2(ElementPointerATI, type, pointer);
}

GLAPI void APIENTRY glEnableClientStateIndexedEXT( GLenum array, GLuint index) {
    CALL_2(EnableClientStateIndexedEXT, array, index);
}

GLAPI void APIENTRY glEnableClientStateiEXT( GLenum array, GLuint index) {
    CALL_2(EnableClientStateiEXT, array, index);
}

GLAPI void APIENTRY glEnableIndexedEXT( GLenum target, GLuint index) {
    CALL_2(EnableIndexedEXT, target, index);
}

GLAPI void APIENTRY glEnableVariantClientStateEXT( GLuint id) {
    CALL_1(EnableVariantClientStateEXT, id);
}

GLAPI void APIENTRY glEnableVertexArrayAttrib( GLuint vaobj, GLuint index) {
    CALL_2(EnableVertexArrayAttrib, vaobj, index);
}

GLAPI void APIENTRY glEnableVertexArrayAttribEXT( GLuint vaobj, GLuint index) {
    CALL_2(EnableVertexArrayAttribEXT, vaobj, index);
}

GLAPI void APIENTRY glEnableVertexArrayEXT( GLuint vaobj, GLenum array) {
    CALL_2(EnableVertexArrayEXT, vaobj, array);
}

GLAPI void APIENTRY glEnableVertexAttribAPPLE( GLuint index, GLenum pname) {
    CALL_2(EnableVertexAttribAPPLE, index, pname);
}

GLAPI void APIENTRY glEnableVertexAttribArray( GLuint index) {
    CALL_1(EnableVertexAttribArray, index);
}

GLAPI void APIENTRY glEnableVertexAttribArrayARB( GLuint index) {
    CALL_1(EnableVertexAttribArrayARB, index);
}

GLAPI void APIENTRY glEnablei( GLenum target, GLuint index) {
    CALL_2(Enablei, target, index);
}

GLAPI void APIENTRY glEndConditionalRender( void ) {
    CALL_0(EndConditionalRender);
}

GLAPI void APIENTRY glEndConditionalRenderNV( void ) {
    CALL_0(EndConditionalRenderNV);
}

GLAPI void APIENTRY glEndConditionalRenderNVX( void ) {
    CALL_0(EndConditionalRenderNVX);
}

GLAPI void APIENTRY glEndFragmentShaderATI( void ) {
    CALL_0(EndFragmentShaderATI);
}

GLAPI void APIENTRY glEndOcclusionQueryNV( void ) {
    CALL_0(EndOcclusionQueryNV);
}

GLAPI void APIENTRY glEndPerfMonitorAMD( GLuint monitor) {
    CALL_1(EndPerfMonitorAMD, monitor);
}

GLAPI void APIENTRY glEndPerfQueryINTEL( GLuint queryHandle) {
    CALL_1(EndPerfQueryINTEL, queryHandle);
}

GLAPI void APIENTRY glEndQuery( GLenum target) {
    CALL_1(EndQuery, target);
}

GLAPI void APIENTRY glEndQueryARB( GLenum target) {
    CALL_1(EndQueryARB, target);
}

GLAPI void APIENTRY glEndQueryIndexed( GLenum target, GLuint index) {
    CALL_2(EndQueryIndexed, target, index);
}

GLAPI void APIENTRY glEndTransformFeedback( void ) {
    CALL_0(EndTransformFeedback);
}

GLAPI void APIENTRY glEndTransformFeedbackEXT( void ) {
    CALL_0(EndTransformFeedbackEXT);
}

GLAPI void APIENTRY glEndTransformFeedbackNV( void ) {
    CALL_0(EndTransformFeedbackNV);
}

GLAPI void APIENTRY glEndVertexShaderEXT( void ) {
    CALL_0(EndVertexShaderEXT);
}

GLAPI void APIENTRY glEndVideoCaptureNV( GLuint video_capture_slot) {
    CALL_1(EndVideoCaptureNV, video_capture_slot);
}

GLAPI void APIENTRY glEvalCoord1xOES( GLfixed u) {
    CALL_1(EvalCoord1xOES, u);
}

GLAPI void APIENTRY glEvalCoord1xvOES( const GLfixed* coords) {
    CALL_1(EvalCoord1xvOES, coords);
}

GLAPI void APIENTRY glEvalCoord2xOES( GLfixed u, GLfixed v) {
    CALL_2(EvalCoord2xOES, u, v);
}

GLAPI void APIENTRY glEvalCoord2xvOES( const GLfixed* coords) {
    CALL_1(EvalCoord2xvOES, coords);
}

GLAPI void APIENTRY glEvalMapsNV( GLenum target, GLenum mode) {
    CALL_2(EvalMapsNV, target, mode);
}

GLAPI void APIENTRY glEvaluateDepthValuesARB( void ) {
    CALL_0(EvaluateDepthValuesARB);
}

GLAPI void APIENTRY glExecuteProgramNV( GLenum target, GLuint id, const GLfloat* params) {
    CALL_3(ExecuteProgramNV, target, id, params);
}

GLAPI void APIENTRY glExtractComponentEXT( GLuint res, GLuint src, GLuint num) {
    CALL_3(ExtractComponentEXT, res, src, num);
}

GLAPI void APIENTRY glFeedbackBufferxOES( GLsizei n, GLenum type, const GLfixed* buffer) {
    CALL_3(FeedbackBufferxOES, n, type, buffer);
}

GLAPI GLsync APIENTRY glFenceSync( GLenum condition, GLbitfield flags) {
    CALL_2_R(FenceSync, condition, flags);
}

GLAPI void APIENTRY glFinalCombinerInputNV( GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage) {
    CALL_4(FinalCombinerInputNV, variable, input, mapping, componentUsage);
}

GLAPI GLint APIENTRY glFinishAsyncSGIX( GLuint* markerp) {
    CALL_1_R(FinishAsyncSGIX, markerp);
}

GLAPI void APIENTRY glFinishFenceAPPLE( GLuint fence) {
    CALL_1(FinishFenceAPPLE, fence);
}

GLAPI void APIENTRY glFinishFenceNV( GLuint fence) {
    CALL_1(FinishFenceNV, fence);
}

GLAPI void APIENTRY glFinishObjectAPPLE( GLenum object, GLint name) {
    CALL_2(FinishObjectAPPLE, object, name);
}

GLAPI void APIENTRY glFinishTextureSUNX( void ) {
    CALL_0(FinishTextureSUNX);
}

GLAPI void APIENTRY glFlushMappedBufferRange( GLenum target, GLintptr offset, GLsizeiptr length) {
    CALL_3(FlushMappedBufferRange, target, offset, length);
}

GLAPI void APIENTRY glFlushMappedBufferRangeAPPLE( GLenum target, GLintptr offset, GLsizeiptr size) {
    CALL_3(FlushMappedBufferRangeAPPLE, target, offset, size);
}

GLAPI void APIENTRY glFlushMappedNamedBufferRange( GLuint buffer, GLintptr offset, GLsizeiptr length) {
    CALL_3(FlushMappedNamedBufferRange, buffer, offset, length);
}

GLAPI void APIENTRY glFlushMappedNamedBufferRangeEXT( GLuint buffer, GLintptr offset, GLsizeiptr length) {
    CALL_3(FlushMappedNamedBufferRangeEXT, buffer, offset, length);
}

GLAPI void APIENTRY glFlushPixelDataRangeNV( GLenum target) {
    CALL_1(FlushPixelDataRangeNV, target);
}

GLAPI void APIENTRY glFlushRasterSGIX( void ) {
    CALL_0(FlushRasterSGIX);
}

GLAPI void APIENTRY glFlushStaticDataIBM( GLenum target) {
    CALL_1(FlushStaticDataIBM, target);
}

GLAPI void APIENTRY glFlushVertexArrayRangeAPPLE( GLsizei length, void* pointer) {
    CALL_2(FlushVertexArrayRangeAPPLE, length, pointer);
}

GLAPI void APIENTRY glFlushVertexArrayRangeNV( void ) {
    CALL_0(FlushVertexArrayRangeNV);
}

GLAPI void APIENTRY glFogCoordFormatNV( GLenum type, GLsizei stride) {
    CALL_2(FogCoordFormatNV, type, stride);
}

GLAPI void APIENTRY glFogCoordPointer( GLenum type, GLsizei stride, const void* pointer) {
    CALL_3(FogCoordPointer, type, stride, pointer);
}

GLAPI void APIENTRY glFogCoordPointerEXT( GLenum type, GLsizei stride, const void* pointer) {
    CALL_3(FogCoordPointerEXT, type, stride, pointer);
}

GLAPI void APIENTRY glFogCoordPointerListIBM( GLenum type, GLint stride, const void** pointer, GLint ptrstride) {
    CALL_4(FogCoordPointerListIBM, type, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glFogCoordd( GLdouble coord) {
    CALL_1(FogCoordd, D(coord));
}

GLAPI void APIENTRY glFogCoorddEXT( GLdouble coord) {
    CALL_1(FogCoorddEXT, D(coord));
}

GLAPI void APIENTRY glFogCoorddv( const GLdouble* coord) {
    CALL_1(FogCoorddv, coord);
}

GLAPI void APIENTRY glFogCoorddvEXT( const GLdouble* coord) {
    CALL_1(FogCoorddvEXT, coord);
}

GLAPI void APIENTRY glFogCoordf( GLfloat coord) {
    CALL_1(FogCoordf, F(coord));
}

GLAPI void APIENTRY glFogCoordfEXT( GLfloat coord) {
    CALL_1(FogCoordfEXT, F(coord));
}

GLAPI void APIENTRY glFogCoordfv( const GLfloat* coord) {
    CALL_1(FogCoordfv, coord);
}

GLAPI void APIENTRY glFogCoordfvEXT( const GLfloat* coord) {
    CALL_1(FogCoordfvEXT, coord);
}

GLAPI void APIENTRY glFogCoordhNV( GLhalfNV fog) {
    CALL_1(FogCoordhNV, fog);
}

GLAPI void APIENTRY glFogCoordhvNV( const GLhalfNV* fog) {
    CALL_1(FogCoordhvNV, fog);
}

GLAPI void APIENTRY glFogFuncSGIS( GLsizei n, const GLfloat* points) {
    CALL_2(FogFuncSGIS, n, points);
}

GLAPI void APIENTRY glFogxOES( GLenum pname, GLfixed param) {
    CALL_2(FogxOES, pname, param);
}

GLAPI void APIENTRY glFogxvOES( GLenum pname, const GLfixed* param) {
    CALL_2(FogxvOES, pname, param);
}

GLAPI void APIENTRY glFragmentColorMaterialSGIX( GLenum face, GLenum mode) {
    CALL_2(FragmentColorMaterialSGIX, face, mode);
}

GLAPI void APIENTRY glFragmentCoverageColorNV( GLuint color) {
    CALL_1(FragmentCoverageColorNV, color);
}

GLAPI void APIENTRY glFragmentLightModelfSGIX( GLenum pname, GLfloat param) {
    CALL_2(FragmentLightModelfSGIX, pname, F(param));
}

GLAPI void APIENTRY glFragmentLightModelfvSGIX( GLenum pname, const GLfloat* params) {
    CALL_2(FragmentLightModelfvSGIX, pname, params);
}

GLAPI void APIENTRY glFragmentLightModeliSGIX( GLenum pname, GLint param) {
    CALL_2(FragmentLightModeliSGIX, pname, param);
}

GLAPI void APIENTRY glFragmentLightModelivSGIX( GLenum pname, const GLint* params) {
    CALL_2(FragmentLightModelivSGIX, pname, params);
}

GLAPI void APIENTRY glFragmentLightfSGIX( GLenum light, GLenum pname, GLfloat param) {
    CALL_3(FragmentLightfSGIX, light, pname, F(param));
}

GLAPI void APIENTRY glFragmentLightfvSGIX( GLenum light, GLenum pname, const GLfloat* params) {
    CALL_3(FragmentLightfvSGIX, light, pname, params);
}

GLAPI void APIENTRY glFragmentLightiSGIX( GLenum light, GLenum pname, GLint param) {
    CALL_3(FragmentLightiSGIX, light, pname, param);
}

GLAPI void APIENTRY glFragmentLightivSGIX( GLenum light, GLenum pname, const GLint* params) {
    CALL_3(FragmentLightivSGIX, light, pname, params);
}

GLAPI void APIENTRY glFragmentMaterialfSGIX( GLenum face, GLenum pname, GLfloat param) {
    CALL_3(FragmentMaterialfSGIX, face, pname, F(param));
}

GLAPI void APIENTRY glFragmentMaterialfvSGIX( GLenum face, GLenum pname, const GLfloat* params) {
    CALL_3(FragmentMaterialfvSGIX, face, pname, params);
}

GLAPI void APIENTRY glFragmentMaterialiSGIX( GLenum face, GLenum pname, GLint param) {
    CALL_3(FragmentMaterialiSGIX, face, pname, param);
}

GLAPI void APIENTRY glFragmentMaterialivSGIX( GLenum face, GLenum pname, const GLint* params) {
    CALL_3(FragmentMaterialivSGIX, face, pname, params);
}

GLAPI void APIENTRY glFrameTerminatorGREMEDY( void ) {
    CALL_0(FrameTerminatorGREMEDY);
}

GLAPI void APIENTRY glFrameZoomSGIX( GLint factor) {
    CALL_1(FrameZoomSGIX, factor);
}

GLAPI void APIENTRY glFramebufferDrawBufferEXT( GLuint framebuffer, GLenum mode) {
    CALL_2(FramebufferDrawBufferEXT, framebuffer, mode);
}

GLAPI void APIENTRY glFramebufferDrawBuffersEXT( GLuint framebuffer, GLsizei n, const GLenum* bufs) {
    CALL_3(FramebufferDrawBuffersEXT, framebuffer, n, bufs);
}

GLAPI void APIENTRY glFramebufferParameteri( GLenum target, GLenum pname, GLint param) {
    CALL_3(FramebufferParameteri, target, pname, param);
}

GLAPI void APIENTRY glFramebufferReadBufferEXT( GLuint framebuffer, GLenum mode) {
    CALL_2(FramebufferReadBufferEXT, framebuffer, mode);
}

GLAPI void APIENTRY glFramebufferRenderbuffer( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    CALL_4(FramebufferRenderbuffer, target, attachment, renderbuffertarget, renderbuffer);
}

GLAPI void APIENTRY glFramebufferRenderbufferEXT( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    CALL_4(FramebufferRenderbufferEXT, target, attachment, renderbuffertarget, renderbuffer);
}

GLAPI void APIENTRY glFramebufferSampleLocationsfvARB( GLenum target, GLuint start, GLsizei count, const GLfloat* v) {
    CALL_4(FramebufferSampleLocationsfvARB, target, start, count, v);
}

GLAPI void APIENTRY glFramebufferSampleLocationsfvNV( GLenum target, GLuint start, GLsizei count, const GLfloat* v) {
    CALL_4(FramebufferSampleLocationsfvNV, target, start, count, v);
}

GLAPI void APIENTRY glFramebufferTexture( GLenum target, GLenum attachment, GLuint texture, GLint level) {
    CALL_4(FramebufferTexture, target, attachment, texture, level);
}

GLAPI void APIENTRY glFramebufferTexture1D( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    CALL_5(FramebufferTexture1D, target, attachment, textarget, texture, level);
}

GLAPI void APIENTRY glFramebufferTexture1DEXT( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    CALL_5(FramebufferTexture1DEXT, target, attachment, textarget, texture, level);
}

GLAPI void APIENTRY glFramebufferTexture2D( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    CALL_5(FramebufferTexture2D, target, attachment, textarget, texture, level);
}

GLAPI void APIENTRY glFramebufferTexture2DEXT( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    CALL_5(FramebufferTexture2DEXT, target, attachment, textarget, texture, level);
}

GLAPI void APIENTRY glFramebufferTexture3D( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {
    CALL_6(FramebufferTexture3D, target, attachment, textarget, texture, level, zoffset);
}

GLAPI void APIENTRY glFramebufferTexture3DEXT( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {
    CALL_6(FramebufferTexture3DEXT, target, attachment, textarget, texture, level, zoffset);
}

GLAPI void APIENTRY glFramebufferTextureARB( GLenum target, GLenum attachment, GLuint texture, GLint level) {
    CALL_4(FramebufferTextureARB, target, attachment, texture, level);
}

GLAPI void APIENTRY glFramebufferTextureEXT( GLenum target, GLenum attachment, GLuint texture, GLint level) {
    CALL_4(FramebufferTextureEXT, target, attachment, texture, level);
}

GLAPI void APIENTRY glFramebufferTextureFaceARB( GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face) {
    CALL_5(FramebufferTextureFaceARB, target, attachment, texture, level, face);
}

GLAPI void APIENTRY glFramebufferTextureFaceEXT( GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face) {
    CALL_5(FramebufferTextureFaceEXT, target, attachment, texture, level, face);
}

GLAPI void APIENTRY glFramebufferTextureLayer( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    CALL_5(FramebufferTextureLayer, target, attachment, texture, level, layer);
}

GLAPI void APIENTRY glFramebufferTextureLayerARB( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    CALL_5(FramebufferTextureLayerARB, target, attachment, texture, level, layer);
}

GLAPI void APIENTRY glFramebufferTextureLayerEXT( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    CALL_5(FramebufferTextureLayerEXT, target, attachment, texture, level, layer);
}

GLAPI void APIENTRY glFramebufferTextureMultiviewOVR( GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews) {
    CALL_6(FramebufferTextureMultiviewOVR, target, attachment, texture, level, baseViewIndex, numViews);
}

GLAPI void APIENTRY glFreeObjectBufferATI( GLuint buffer) {
    CALL_1(FreeObjectBufferATI, buffer);
}

GLAPI void APIENTRY glFrustumfOES( GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) {
    CALL_6(FrustumfOES, F(l), F(r), F(b), F(t), F(n), F(f));
}

GLAPI void APIENTRY glFrustumxOES( GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f) {
    CALL_6(FrustumxOES, l, r, b, t, n, f);
}

GLAPI GLuint APIENTRY glGenAsyncMarkersSGIX( GLsizei range) {
    CALL_1_R(GenAsyncMarkersSGIX, range);
}

GLAPI void APIENTRY glGenBuffers( GLsizei n, GLuint* buffers) {
    CALL_2(GenBuffers, n, buffers);
}

GLAPI void APIENTRY glGenBuffersARB( GLsizei n, GLuint* buffers) {
    CALL_2(GenBuffersARB, n, buffers);
}

GLAPI void APIENTRY glGenFencesAPPLE( GLsizei n, GLuint* fences) {
    CALL_2(GenFencesAPPLE, n, fences);
}

GLAPI void APIENTRY glGenFencesNV( GLsizei n, GLuint* fences) {
    CALL_2(GenFencesNV, n, fences);
}

GLAPI GLuint APIENTRY glGenFragmentShadersATI( GLuint range) {
    CALL_1_R(GenFragmentShadersATI, range);
}

GLAPI void APIENTRY glGenFramebuffers( GLsizei n, GLuint* framebuffers) {
    CALL_2(GenFramebuffers, n, framebuffers);
}

GLAPI void APIENTRY glGenFramebuffersEXT( GLsizei n, GLuint* framebuffers) {
    CALL_2(GenFramebuffersEXT, n, framebuffers);
}

GLAPI void APIENTRY glGenNamesAMD( GLenum identifier, GLuint num, GLuint* names) {
    CALL_3(GenNamesAMD, identifier, num, names);
}

GLAPI void APIENTRY glGenOcclusionQueriesNV( GLsizei n, GLuint* ids) {
    CALL_2(GenOcclusionQueriesNV, n, ids);
}

GLAPI GLuint APIENTRY glGenPathsNV( GLsizei range) {
    CALL_1_R(GenPathsNV, range);
}

GLAPI void APIENTRY glGenPerfMonitorsAMD( GLsizei n, GLuint* monitors) {
    CALL_2(GenPerfMonitorsAMD, n, monitors);
}

GLAPI void APIENTRY glGenProgramPipelines( GLsizei n, GLuint* pipelines) {
    CALL_2(GenProgramPipelines, n, pipelines);
}

GLAPI void APIENTRY glGenProgramsARB( GLsizei n, GLuint* programs) {
    CALL_2(GenProgramsARB, n, programs);
}

GLAPI void APIENTRY glGenProgramsNV( GLsizei n, GLuint* programs) {
    CALL_2(GenProgramsNV, n, programs);
}

GLAPI void APIENTRY glGenQueries( GLsizei n, GLuint* ids) {
    CALL_2(GenQueries, n, ids);
}

GLAPI void APIENTRY glGenQueriesARB( GLsizei n, GLuint* ids) {
    CALL_2(GenQueriesARB, n, ids);
}

GLAPI void APIENTRY glGenRenderbuffers( GLsizei n, GLuint* renderbuffers) {
    CALL_2(GenRenderbuffers, n, renderbuffers);
}

GLAPI void APIENTRY glGenRenderbuffersEXT( GLsizei n, GLuint* renderbuffers) {
    CALL_2(GenRenderbuffersEXT, n, renderbuffers);
}

GLAPI void APIENTRY glGenSamplers( GLsizei count, GLuint* samplers) {
    CALL_2(GenSamplers, count, samplers);
}

GLAPI GLuint APIENTRY glGenSymbolsEXT( GLenum datatype, GLenum storagetype, GLenum range, GLuint components) {
    CALL_4_R(GenSymbolsEXT, datatype, storagetype, range, components);
}

GLAPI void APIENTRY glGenTexturesEXT( GLsizei n, GLuint* textures) {
    CALL_2(GenTexturesEXT, n, textures);
}

GLAPI void APIENTRY glGenTransformFeedbacks( GLsizei n, GLuint* ids) {
    CALL_2(GenTransformFeedbacks, n, ids);
}

GLAPI void APIENTRY glGenTransformFeedbacksNV( GLsizei n, GLuint* ids) {
    CALL_2(GenTransformFeedbacksNV, n, ids);
}

GLAPI void APIENTRY glGenVertexArrays( GLsizei n, GLuint* arrays) {
    CALL_2(GenVertexArrays, n, arrays);
}

GLAPI void APIENTRY glGenVertexArraysAPPLE( GLsizei n, GLuint* arrays) {
    CALL_2(GenVertexArraysAPPLE, n, arrays);
}

GLAPI GLuint APIENTRY glGenVertexShadersEXT( GLuint range) {
    CALL_1_R(GenVertexShadersEXT, range);
}

GLAPI void APIENTRY glGenerateMipmap( GLenum target) {
    CALL_1(GenerateMipmap, target);
}

GLAPI void APIENTRY glGenerateMipmapEXT( GLenum target) {
    CALL_1(GenerateMipmapEXT, target);
}

GLAPI void APIENTRY glGenerateMultiTexMipmapEXT( GLenum texunit, GLenum target) {
    CALL_2(GenerateMultiTexMipmapEXT, texunit, target);
}

GLAPI void APIENTRY glGenerateTextureMipmap( GLuint texture) {
    CALL_1(GenerateTextureMipmap, texture);
}

GLAPI void APIENTRY glGenerateTextureMipmapEXT( GLuint texture, GLenum target) {
    CALL_2(GenerateTextureMipmapEXT, texture, target);
}

GLAPI void APIENTRY glGetActiveAtomicCounterBufferiv( GLuint program, GLuint bufferIndex, GLenum pname, GLint* params) {
    CALL_4(GetActiveAtomicCounterBufferiv, program, bufferIndex, pname, params);
}

GLAPI void APIENTRY glGetActiveAttrib( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
    CALL_7(GetActiveAttrib, program, index, bufSize, length, size, type, name);
}

GLAPI void APIENTRY glGetActiveAttribARB( GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLcharARB* name) {
    CALL_7(GetActiveAttribARB, programObj, index, maxLength, length, size, type, name);
}

GLAPI void APIENTRY glGetActiveSubroutineName( GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name) {
    CALL_6(GetActiveSubroutineName, program, shadertype, index, bufsize, length, name);
}

GLAPI void APIENTRY glGetActiveSubroutineUniformName( GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name) {
    CALL_6(GetActiveSubroutineUniformName, program, shadertype, index, bufsize, length, name);
}

GLAPI void APIENTRY glGetActiveSubroutineUniformiv( GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values) {
    CALL_5(GetActiveSubroutineUniformiv, program, shadertype, index, pname, values);
}

GLAPI void APIENTRY glGetActiveUniform( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
    CALL_7(GetActiveUniform, program, index, bufSize, length, size, type, name);
}

GLAPI void APIENTRY glGetActiveUniformARB( GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLcharARB* name) {
    CALL_7(GetActiveUniformARB, programObj, index, maxLength, length, size, type, name);
}

GLAPI void APIENTRY glGetActiveUniformBlockName( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName) {
    CALL_5(GetActiveUniformBlockName, program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

GLAPI void APIENTRY glGetActiveUniformBlockiv( GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params) {
    CALL_4(GetActiveUniformBlockiv, program, uniformBlockIndex, pname, params);
}

GLAPI void APIENTRY glGetActiveUniformName( GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName) {
    CALL_5(GetActiveUniformName, program, uniformIndex, bufSize, length, uniformName);
}

GLAPI void APIENTRY glGetActiveUniformsiv( GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params) {
    CALL_5(GetActiveUniformsiv, program, uniformCount, uniformIndices, pname, params);
}

GLAPI void APIENTRY glGetActiveVaryingNV( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) {
    CALL_7(GetActiveVaryingNV, program, index, bufSize, length, size, type, name);
}

GLAPI void APIENTRY glGetArrayObjectfvATI( GLenum array, GLenum pname, GLfloat* params) {
    CALL_3(GetArrayObjectfvATI, array, pname, params);
}

GLAPI void APIENTRY glGetArrayObjectivATI( GLenum array, GLenum pname, GLint* params) {
    CALL_3(GetArrayObjectivATI, array, pname, params);
}

GLAPI void APIENTRY glGetAttachedObjectsARB( GLhandleARB containerObj, GLsizei maxCount, GLsizei* count, GLhandleARB* obj) {
    CALL_4(GetAttachedObjectsARB, containerObj, maxCount, count, obj);
}

GLAPI void APIENTRY glGetAttachedShaders( GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders) {
    CALL_4(GetAttachedShaders, program, maxCount, count, shaders);
}

GLAPI GLint APIENTRY glGetAttribLocation( GLuint program, const GLchar* name) {
    CALL_2_R(GetAttribLocation, program, name);
}

GLAPI GLint APIENTRY glGetAttribLocationARB( GLhandleARB programObj, const GLcharARB* name) {
    CALL_2_R(GetAttribLocationARB, programObj, name);
}

GLAPI void APIENTRY glGetBooleanIndexedvEXT( GLenum target, GLuint index, GLboolean* data) {
    CALL_3(GetBooleanIndexedvEXT, target, index, data);
}

GLAPI void APIENTRY glGetBooleani_v( GLenum target, GLuint index, GLboolean* data) {
    CALL_3(GetBooleani_v, target, index, data);
}

GLAPI void APIENTRY glGetBufferParameteri64v( GLenum target, GLenum pname, GLint64* params) {
    CALL_3(GetBufferParameteri64v, target, pname, params);
}

GLAPI void APIENTRY glGetBufferParameteriv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetBufferParameteriv, target, pname, params);
}

GLAPI void APIENTRY glGetBufferParameterivARB( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetBufferParameterivARB, target, pname, params);
}

GLAPI void APIENTRY glGetBufferParameterui64vNV( GLenum target, GLenum pname, GLuint64EXT* params) {
    CALL_3(GetBufferParameterui64vNV, target, pname, params);
}

GLAPI void APIENTRY glGetBufferPointerv( GLenum target, GLenum pname, void** params) {
    CALL_3(GetBufferPointerv, target, pname, params);
}

GLAPI void APIENTRY glGetBufferPointervARB( GLenum target, GLenum pname, void** params) {
    CALL_3(GetBufferPointervARB, target, pname, params);
}

GLAPI void APIENTRY glGetBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, void* data) {
    CALL_4(GetBufferSubData, target, offset, size, data);
}

GLAPI void APIENTRY glGetBufferSubDataARB( GLenum target, GLintptrARB offset, GLsizeiptrARB size, void* data) {
    CALL_4(GetBufferSubDataARB, target, offset, size, data);
}

GLAPI void APIENTRY glGetClipPlanefOES( GLenum plane, GLfloat* equation) {
    CALL_2(GetClipPlanefOES, plane, equation);
}

GLAPI void APIENTRY glGetClipPlanexOES( GLenum plane, GLfixed* equation) {
    CALL_2(GetClipPlanexOES, plane, equation);
}

GLAPI void APIENTRY glGetColorTable( GLenum target, GLenum format, GLenum type, void* table) {
    CALL_4(GetColorTable, target, format, type, table);
}

GLAPI void APIENTRY glGetColorTableEXT( GLenum target, GLenum format, GLenum type, void* data) {
    CALL_4(GetColorTableEXT, target, format, type, data);
}

GLAPI void APIENTRY glGetColorTableParameterfv( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetColorTableParameterfv, target, pname, params);
}

GLAPI void APIENTRY glGetColorTableParameterfvEXT( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetColorTableParameterfvEXT, target, pname, params);
}

GLAPI void APIENTRY glGetColorTableParameterfvSGI( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetColorTableParameterfvSGI, target, pname, params);
}

GLAPI void APIENTRY glGetColorTableParameteriv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetColorTableParameteriv, target, pname, params);
}

GLAPI void APIENTRY glGetColorTableParameterivEXT( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetColorTableParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetColorTableParameterivSGI( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetColorTableParameterivSGI, target, pname, params);
}

GLAPI void APIENTRY glGetColorTableSGI( GLenum target, GLenum format, GLenum type, void* table) {
    CALL_4(GetColorTableSGI, target, format, type, table);
}

GLAPI void APIENTRY glGetCombinerInputParameterfvNV( GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat* params) {
    CALL_5(GetCombinerInputParameterfvNV, stage, portion, variable, pname, params);
}

GLAPI void APIENTRY glGetCombinerInputParameterivNV( GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint* params) {
    CALL_5(GetCombinerInputParameterivNV, stage, portion, variable, pname, params);
}

GLAPI void APIENTRY glGetCombinerOutputParameterfvNV( GLenum stage, GLenum portion, GLenum pname, GLfloat* params) {
    CALL_4(GetCombinerOutputParameterfvNV, stage, portion, pname, params);
}

GLAPI void APIENTRY glGetCombinerOutputParameterivNV( GLenum stage, GLenum portion, GLenum pname, GLint* params) {
    CALL_4(GetCombinerOutputParameterivNV, stage, portion, pname, params);
}

GLAPI void APIENTRY glGetCombinerStageParameterfvNV( GLenum stage, GLenum pname, GLfloat* params) {
    CALL_3(GetCombinerStageParameterfvNV, stage, pname, params);
}

GLAPI GLuint APIENTRY glGetCommandHeaderNV( GLenum tokenID, GLuint size) {
    CALL_2_R(GetCommandHeaderNV, tokenID, size);
}

GLAPI void APIENTRY glGetCompressedMultiTexImageEXT( GLenum texunit, GLenum target, GLint lod, void* img) {
    CALL_4(GetCompressedMultiTexImageEXT, texunit, target, lod, img);
}

GLAPI void APIENTRY glGetCompressedTexImage( GLenum target, GLint level, void* img) {
    CALL_3(GetCompressedTexImage, target, level, img);
}

GLAPI void APIENTRY glGetCompressedTexImageARB( GLenum target, GLint level, void* img) {
    CALL_3(GetCompressedTexImageARB, target, level, img);
}

GLAPI void APIENTRY glGetCompressedTextureImage( GLuint texture, GLint level, GLsizei bufSize, void* pixels) {
    CALL_4(GetCompressedTextureImage, texture, level, bufSize, pixels);
}

GLAPI void APIENTRY glGetCompressedTextureImageEXT( GLuint texture, GLenum target, GLint lod, void* img) {
    CALL_4(GetCompressedTextureImageEXT, texture, target, lod, img);
}

GLAPI void APIENTRY glGetCompressedTextureSubImage( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void* pixels) {
    CALL_10(GetCompressedTextureSubImage, texture, level, xoffset, yoffset, zoffset, width, height, depth, bufSize, pixels);
}

GLAPI void APIENTRY glGetConvolutionFilter( GLenum target, GLenum format, GLenum type, void* image) {
    CALL_4(GetConvolutionFilter, target, format, type, image);
}

GLAPI void APIENTRY glGetConvolutionFilterEXT( GLenum target, GLenum format, GLenum type, void* image) {
    CALL_4(GetConvolutionFilterEXT, target, format, type, image);
}

GLAPI void APIENTRY glGetConvolutionParameterfv( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetConvolutionParameterfv, target, pname, params);
}

GLAPI void APIENTRY glGetConvolutionParameterfvEXT( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetConvolutionParameterfvEXT, target, pname, params);
}

GLAPI void APIENTRY glGetConvolutionParameteriv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetConvolutionParameteriv, target, pname, params);
}

GLAPI void APIENTRY glGetConvolutionParameterivEXT( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetConvolutionParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetConvolutionParameterxvOES( GLenum target, GLenum pname, GLfixed* params) {
    CALL_3(GetConvolutionParameterxvOES, target, pname, params);
}

GLAPI void APIENTRY glGetCoverageModulationTableNV( GLsizei bufsize, GLfloat* v) {
    CALL_2(GetCoverageModulationTableNV, bufsize, v);
}

GLAPI GLuint APIENTRY glGetDebugMessageLog( GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog) {
    CALL_8_R(GetDebugMessageLog, count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

GLAPI GLuint APIENTRY glGetDebugMessageLogAMD( GLuint count, GLsizei bufsize, GLenum* categories, GLuint* severities, GLuint* ids, GLsizei* lengths, GLchar* message) {
    CALL_7_R(GetDebugMessageLogAMD, count, bufsize, categories, severities, ids, lengths, message);
}

GLAPI GLuint APIENTRY glGetDebugMessageLogARB( GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog) {
    CALL_8_R(GetDebugMessageLogARB, count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

GLAPI void APIENTRY glGetDetailTexFuncSGIS( GLenum target, GLfloat* points) {
    CALL_2(GetDetailTexFuncSGIS, target, points);
}

GLAPI void APIENTRY glGetDoubleIndexedvEXT( GLenum target, GLuint index, GLdouble* data) {
    CALL_3(GetDoubleIndexedvEXT, target, index, data);
}

GLAPI void APIENTRY glGetDoublei_v( GLenum target, GLuint index, GLdouble* data) {
    CALL_3(GetDoublei_v, target, index, data);
}

GLAPI void APIENTRY glGetDoublei_vEXT( GLenum pname, GLuint index, GLdouble* params) {
    CALL_3(GetDoublei_vEXT, pname, index, params);
}

GLAPI void APIENTRY glGetFenceivNV( GLuint fence, GLenum pname, GLint* params) {
    CALL_3(GetFenceivNV, fence, pname, params);
}

GLAPI void APIENTRY glGetFinalCombinerInputParameterfvNV( GLenum variable, GLenum pname, GLfloat* params) {
    CALL_3(GetFinalCombinerInputParameterfvNV, variable, pname, params);
}

GLAPI void APIENTRY glGetFinalCombinerInputParameterivNV( GLenum variable, GLenum pname, GLint* params) {
    CALL_3(GetFinalCombinerInputParameterivNV, variable, pname, params);
}

GLAPI void APIENTRY glGetFirstPerfQueryIdINTEL( GLuint* queryId) {
    CALL_1(GetFirstPerfQueryIdINTEL, queryId);
}

GLAPI void APIENTRY glGetFixedvOES( GLenum pname, GLfixed* params) {
    CALL_2(GetFixedvOES, pname, params);
}

GLAPI void APIENTRY glGetFloatIndexedvEXT( GLenum target, GLuint index, GLfloat* data) {
    CALL_3(GetFloatIndexedvEXT, target, index, data);
}

GLAPI void APIENTRY glGetFloati_v( GLenum target, GLuint index, GLfloat* data) {
    CALL_3(GetFloati_v, target, index, data);
}

GLAPI void APIENTRY glGetFloati_vEXT( GLenum pname, GLuint index, GLfloat* params) {
    CALL_3(GetFloati_vEXT, pname, index, params);
}

GLAPI void APIENTRY glGetFogFuncSGIS( GLfloat* points) {
    CALL_1(GetFogFuncSGIS, points);
}

GLAPI GLint APIENTRY glGetFragDataIndex( GLuint program, const GLchar* name) {
    CALL_2_R(GetFragDataIndex, program, name);
}

GLAPI GLint APIENTRY glGetFragDataLocation( GLuint program, const GLchar* name) {
    CALL_2_R(GetFragDataLocation, program, name);
}

GLAPI GLint APIENTRY glGetFragDataLocationEXT( GLuint program, const GLchar* name) {
    CALL_2_R(GetFragDataLocationEXT, program, name);
}

GLAPI void APIENTRY glGetFragmentLightfvSGIX( GLenum light, GLenum pname, GLfloat* params) {
    CALL_3(GetFragmentLightfvSGIX, light, pname, params);
}

GLAPI void APIENTRY glGetFragmentLightivSGIX( GLenum light, GLenum pname, GLint* params) {
    CALL_3(GetFragmentLightivSGIX, light, pname, params);
}

GLAPI void APIENTRY glGetFragmentMaterialfvSGIX( GLenum face, GLenum pname, GLfloat* params) {
    CALL_3(GetFragmentMaterialfvSGIX, face, pname, params);
}

GLAPI void APIENTRY glGetFragmentMaterialivSGIX( GLenum face, GLenum pname, GLint* params) {
    CALL_3(GetFragmentMaterialivSGIX, face, pname, params);
}

GLAPI void APIENTRY glGetFramebufferAttachmentParameteriv( GLenum target, GLenum attachment, GLenum pname, GLint* params) {
    CALL_4(GetFramebufferAttachmentParameteriv, target, attachment, pname, params);
}

GLAPI void APIENTRY glGetFramebufferAttachmentParameterivEXT( GLenum target, GLenum attachment, GLenum pname, GLint* params) {
    CALL_4(GetFramebufferAttachmentParameterivEXT, target, attachment, pname, params);
}

GLAPI void APIENTRY glGetFramebufferParameteriv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetFramebufferParameteriv, target, pname, params);
}

GLAPI void APIENTRY glGetFramebufferParameterivEXT( GLuint framebuffer, GLenum pname, GLint* params) {
    CALL_3(GetFramebufferParameterivEXT, framebuffer, pname, params);
}

GLAPI GLenum APIENTRY glGetGraphicsResetStatus( void ) {
    CALL_0_R(GetGraphicsResetStatus);
}

GLAPI GLenum APIENTRY glGetGraphicsResetStatusARB( void ) {
    CALL_0_R(GetGraphicsResetStatusARB);
}

GLAPI GLhandleARB APIENTRY glGetHandleARB( GLenum pname) {
    CALL_1_R(GetHandleARB, pname);
}

GLAPI void APIENTRY glGetHistogram( GLenum target, GLboolean reset, GLenum format, GLenum type, void* values) {
    CALL_5(GetHistogram, target, reset, format, type, values);
}

GLAPI void APIENTRY glGetHistogramEXT( GLenum target, GLboolean reset, GLenum format, GLenum type, void* values) {
    CALL_5(GetHistogramEXT, target, reset, format, type, values);
}

GLAPI void APIENTRY glGetHistogramParameterfv( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetHistogramParameterfv, target, pname, params);
}

GLAPI void APIENTRY glGetHistogramParameterfvEXT( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetHistogramParameterfvEXT, target, pname, params);
}

GLAPI void APIENTRY glGetHistogramParameteriv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetHistogramParameteriv, target, pname, params);
}

GLAPI void APIENTRY glGetHistogramParameterivEXT( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetHistogramParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetHistogramParameterxvOES( GLenum target, GLenum pname, GLfixed* params) {
    CALL_3(GetHistogramParameterxvOES, target, pname, params);
}

GLAPI GLuint64 APIENTRY glGetImageHandleARB( GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format) {
    CALL_5_R(GetImageHandleARB, texture, level, layered, layer, format);
}

GLAPI GLuint64 APIENTRY glGetImageHandleNV( GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format) {
    CALL_5_R(GetImageHandleNV, texture, level, layered, layer, format);
}

GLAPI void APIENTRY glGetImageTransformParameterfvHP( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetImageTransformParameterfvHP, target, pname, params);
}

GLAPI void APIENTRY glGetImageTransformParameterivHP( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetImageTransformParameterivHP, target, pname, params);
}

GLAPI void APIENTRY glGetInfoLogARB( GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB* infoLog) {
    CALL_4(GetInfoLogARB, obj, maxLength, length, infoLog);
}

GLAPI GLint APIENTRY glGetInstrumentsSGIX( void ) {
    CALL_0_R(GetInstrumentsSGIX);
}

GLAPI void APIENTRY glGetInteger64i_v( GLenum target, GLuint index, GLint64* data) {
    CALL_3(GetInteger64i_v, target, index, data);
}

GLAPI void APIENTRY glGetInteger64v( GLenum pname, GLint64* data) {
    CALL_2(GetInteger64v, pname, data);
}

GLAPI void APIENTRY glGetIntegerIndexedvEXT( GLenum target, GLuint index, GLint* data) {
    CALL_3(GetIntegerIndexedvEXT, target, index, data);
}

GLAPI void APIENTRY glGetIntegeri_v( GLenum target, GLuint index, GLint* data) {
    CALL_3(GetIntegeri_v, target, index, data);
}

GLAPI void APIENTRY glGetIntegerui64i_vNV( GLenum value, GLuint index, GLuint64EXT* result) {
    CALL_3(GetIntegerui64i_vNV, value, index, result);
}

GLAPI void APIENTRY glGetIntegerui64vNV( GLenum value, GLuint64EXT* result) {
    CALL_2(GetIntegerui64vNV, value, result);
}

GLAPI void APIENTRY glGetInternalformatSampleivNV( GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei bufSize, GLint* params) {
    CALL_6(GetInternalformatSampleivNV, target, internalformat, samples, pname, bufSize, params);
}

GLAPI void APIENTRY glGetInternalformati64v( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64* params) {
    CALL_5(GetInternalformati64v, target, internalformat, pname, bufSize, params);
}

GLAPI void APIENTRY glGetInternalformativ( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params) {
    CALL_5(GetInternalformativ, target, internalformat, pname, bufSize, params);
}

GLAPI void APIENTRY glGetInvariantBooleanvEXT( GLuint id, GLenum value, GLboolean* data) {
    CALL_3(GetInvariantBooleanvEXT, id, value, data);
}

GLAPI void APIENTRY glGetInvariantFloatvEXT( GLuint id, GLenum value, GLfloat* data) {
    CALL_3(GetInvariantFloatvEXT, id, value, data);
}

GLAPI void APIENTRY glGetInvariantIntegervEXT( GLuint id, GLenum value, GLint* data) {
    CALL_3(GetInvariantIntegervEXT, id, value, data);
}

GLAPI void APIENTRY glGetLightxOES( GLenum light, GLenum pname, GLfixed* params) {
    CALL_3(GetLightxOES, light, pname, params);
}

GLAPI void APIENTRY glGetListParameterfvSGIX( GLuint list, GLenum pname, GLfloat* params) {
    CALL_3(GetListParameterfvSGIX, list, pname, params);
}

GLAPI void APIENTRY glGetListParameterivSGIX( GLuint list, GLenum pname, GLint* params) {
    CALL_3(GetListParameterivSGIX, list, pname, params);
}

GLAPI void APIENTRY glGetLocalConstantBooleanvEXT( GLuint id, GLenum value, GLboolean* data) {
    CALL_3(GetLocalConstantBooleanvEXT, id, value, data);
}

GLAPI void APIENTRY glGetLocalConstantFloatvEXT( GLuint id, GLenum value, GLfloat* data) {
    CALL_3(GetLocalConstantFloatvEXT, id, value, data);
}

GLAPI void APIENTRY glGetLocalConstantIntegervEXT( GLuint id, GLenum value, GLint* data) {
    CALL_3(GetLocalConstantIntegervEXT, id, value, data);
}

GLAPI void APIENTRY glGetMapAttribParameterfvNV( GLenum target, GLuint index, GLenum pname, GLfloat* params) {
    CALL_4(GetMapAttribParameterfvNV, target, index, pname, params);
}

GLAPI void APIENTRY glGetMapAttribParameterivNV( GLenum target, GLuint index, GLenum pname, GLint* params) {
    CALL_4(GetMapAttribParameterivNV, target, index, pname, params);
}

GLAPI void APIENTRY glGetMapControlPointsNV( GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, void* points) {
    CALL_7(GetMapControlPointsNV, target, index, type, ustride, vstride, packed, points);
}

GLAPI void APIENTRY glGetMapParameterfvNV( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetMapParameterfvNV, target, pname, params);
}

GLAPI void APIENTRY glGetMapParameterivNV( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetMapParameterivNV, target, pname, params);
}

GLAPI void APIENTRY glGetMapxvOES( GLenum target, GLenum query, GLfixed* v) {
    CALL_3(GetMapxvOES, target, query, v);
}

GLAPI void APIENTRY glGetMaterialxOES( GLenum face, GLenum pname, GLfixed param) {
    CALL_3(GetMaterialxOES, face, pname, param);
}

GLAPI void APIENTRY glGetMinmax( GLenum target, GLboolean reset, GLenum format, GLenum type, void* values) {
    CALL_5(GetMinmax, target, reset, format, type, values);
}

GLAPI void APIENTRY glGetMinmaxEXT( GLenum target, GLboolean reset, GLenum format, GLenum type, void* values) {
    CALL_5(GetMinmaxEXT, target, reset, format, type, values);
}

GLAPI void APIENTRY glGetMinmaxParameterfv( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetMinmaxParameterfv, target, pname, params);
}

GLAPI void APIENTRY glGetMinmaxParameterfvEXT( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetMinmaxParameterfvEXT, target, pname, params);
}

GLAPI void APIENTRY glGetMinmaxParameteriv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetMinmaxParameteriv, target, pname, params);
}

GLAPI void APIENTRY glGetMinmaxParameterivEXT( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetMinmaxParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetMultiTexEnvfvEXT( GLenum texunit, GLenum target, GLenum pname, GLfloat* params) {
    CALL_4(GetMultiTexEnvfvEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glGetMultiTexEnvivEXT( GLenum texunit, GLenum target, GLenum pname, GLint* params) {
    CALL_4(GetMultiTexEnvivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glGetMultiTexGendvEXT( GLenum texunit, GLenum coord, GLenum pname, GLdouble* params) {
    CALL_4(GetMultiTexGendvEXT, texunit, coord, pname, params);
}

GLAPI void APIENTRY glGetMultiTexGenfvEXT( GLenum texunit, GLenum coord, GLenum pname, GLfloat* params) {
    CALL_4(GetMultiTexGenfvEXT, texunit, coord, pname, params);
}

GLAPI void APIENTRY glGetMultiTexGenivEXT( GLenum texunit, GLenum coord, GLenum pname, GLint* params) {
    CALL_4(GetMultiTexGenivEXT, texunit, coord, pname, params);
}

GLAPI void APIENTRY glGetMultiTexImageEXT( GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
    CALL_6(GetMultiTexImageEXT, texunit, target, level, format, type, pixels);
}

GLAPI void APIENTRY glGetMultiTexLevelParameterfvEXT( GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* params) {
    CALL_5(GetMultiTexLevelParameterfvEXT, texunit, target, level, pname, params);
}

GLAPI void APIENTRY glGetMultiTexLevelParameterivEXT( GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* params) {
    CALL_5(GetMultiTexLevelParameterivEXT, texunit, target, level, pname, params);
}

GLAPI void APIENTRY glGetMultiTexParameterIivEXT( GLenum texunit, GLenum target, GLenum pname, GLint* params) {
    CALL_4(GetMultiTexParameterIivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glGetMultiTexParameterIuivEXT( GLenum texunit, GLenum target, GLenum pname, GLuint* params) {
    CALL_4(GetMultiTexParameterIuivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glGetMultiTexParameterfvEXT( GLenum texunit, GLenum target, GLenum pname, GLfloat* params) {
    CALL_4(GetMultiTexParameterfvEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glGetMultiTexParameterivEXT( GLenum texunit, GLenum target, GLenum pname, GLint* params) {
    CALL_4(GetMultiTexParameterivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glGetMultisamplefv( GLenum pname, GLuint index, GLfloat* val) {
    CALL_3(GetMultisamplefv, pname, index, val);
}

GLAPI void APIENTRY glGetMultisamplefvNV( GLenum pname, GLuint index, GLfloat* val) {
    CALL_3(GetMultisamplefvNV, pname, index, val);
}

GLAPI void APIENTRY glGetNamedBufferParameteri64v( GLuint buffer, GLenum pname, GLint64* params) {
    CALL_3(GetNamedBufferParameteri64v, buffer, pname, params);
}

GLAPI void APIENTRY glGetNamedBufferParameteriv( GLuint buffer, GLenum pname, GLint* params) {
    CALL_3(GetNamedBufferParameteriv, buffer, pname, params);
}

GLAPI void APIENTRY glGetNamedBufferParameterivEXT( GLuint buffer, GLenum pname, GLint* params) {
    CALL_3(GetNamedBufferParameterivEXT, buffer, pname, params);
}

GLAPI void APIENTRY glGetNamedBufferParameterui64vNV( GLuint buffer, GLenum pname, GLuint64EXT* params) {
    CALL_3(GetNamedBufferParameterui64vNV, buffer, pname, params);
}

GLAPI void APIENTRY glGetNamedBufferPointerv( GLuint buffer, GLenum pname, void** params) {
    CALL_3(GetNamedBufferPointerv, buffer, pname, params);
}

GLAPI void APIENTRY glGetNamedBufferPointervEXT( GLuint buffer, GLenum pname, void** params) {
    CALL_3(GetNamedBufferPointervEXT, buffer, pname, params);
}

GLAPI void APIENTRY glGetNamedBufferSubData( GLuint buffer, GLintptr offset, GLsizeiptr size, void* data) {
    CALL_4(GetNamedBufferSubData, buffer, offset, size, data);
}

GLAPI void APIENTRY glGetNamedBufferSubDataEXT( GLuint buffer, GLintptr offset, GLsizeiptr size, void* data) {
    CALL_4(GetNamedBufferSubDataEXT, buffer, offset, size, data);
}

GLAPI void APIENTRY glGetNamedFramebufferAttachmentParameteriv( GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params) {
    CALL_4(GetNamedFramebufferAttachmentParameteriv, framebuffer, attachment, pname, params);
}

GLAPI void APIENTRY glGetNamedFramebufferAttachmentParameterivEXT( GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params) {
    CALL_4(GetNamedFramebufferAttachmentParameterivEXT, framebuffer, attachment, pname, params);
}

GLAPI void APIENTRY glGetNamedFramebufferParameteriv( GLuint framebuffer, GLenum pname, GLint* param) {
    CALL_3(GetNamedFramebufferParameteriv, framebuffer, pname, param);
}

GLAPI void APIENTRY glGetNamedFramebufferParameterivEXT( GLuint framebuffer, GLenum pname, GLint* params) {
    CALL_3(GetNamedFramebufferParameterivEXT, framebuffer, pname, params);
}

GLAPI void APIENTRY glGetNamedProgramLocalParameterIivEXT( GLuint program, GLenum target, GLuint index, GLint* params) {
    CALL_4(GetNamedProgramLocalParameterIivEXT, program, target, index, params);
}

GLAPI void APIENTRY glGetNamedProgramLocalParameterIuivEXT( GLuint program, GLenum target, GLuint index, GLuint* params) {
    CALL_4(GetNamedProgramLocalParameterIuivEXT, program, target, index, params);
}

GLAPI void APIENTRY glGetNamedProgramLocalParameterdvEXT( GLuint program, GLenum target, GLuint index, GLdouble* params) {
    CALL_4(GetNamedProgramLocalParameterdvEXT, program, target, index, params);
}

GLAPI void APIENTRY glGetNamedProgramLocalParameterfvEXT( GLuint program, GLenum target, GLuint index, GLfloat* params) {
    CALL_4(GetNamedProgramLocalParameterfvEXT, program, target, index, params);
}

GLAPI void APIENTRY glGetNamedProgramStringEXT( GLuint program, GLenum target, GLenum pname, void* string) {
    CALL_4(GetNamedProgramStringEXT, program, target, pname, string);
}

GLAPI void APIENTRY glGetNamedProgramivEXT( GLuint program, GLenum target, GLenum pname, GLint* params) {
    CALL_4(GetNamedProgramivEXT, program, target, pname, params);
}

GLAPI void APIENTRY glGetNamedRenderbufferParameteriv( GLuint renderbuffer, GLenum pname, GLint* params) {
    CALL_3(GetNamedRenderbufferParameteriv, renderbuffer, pname, params);
}

GLAPI void APIENTRY glGetNamedRenderbufferParameterivEXT( GLuint renderbuffer, GLenum pname, GLint* params) {
    CALL_3(GetNamedRenderbufferParameterivEXT, renderbuffer, pname, params);
}

GLAPI void APIENTRY glGetNamedStringARB( GLint namelen, const GLchar* name, GLsizei bufSize, GLint* stringlen, GLchar* string) {
    CALL_5(GetNamedStringARB, namelen, name, bufSize, stringlen, string);
}

GLAPI void APIENTRY glGetNamedStringivARB( GLint namelen, const GLchar* name, GLenum pname, GLint* params) {
    CALL_4(GetNamedStringivARB, namelen, name, pname, params);
}

GLAPI void APIENTRY glGetNextPerfQueryIdINTEL( GLuint queryId, GLuint* nextQueryId) {
    CALL_2(GetNextPerfQueryIdINTEL, queryId, nextQueryId);
}

GLAPI void APIENTRY glGetObjectBufferfvATI( GLuint buffer, GLenum pname, GLfloat* params) {
    CALL_3(GetObjectBufferfvATI, buffer, pname, params);
}

GLAPI void APIENTRY glGetObjectBufferivATI( GLuint buffer, GLenum pname, GLint* params) {
    CALL_3(GetObjectBufferivATI, buffer, pname, params);
}

GLAPI void APIENTRY glGetObjectLabel( GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label) {
    CALL_5(GetObjectLabel, identifier, name, bufSize, length, label);
}

GLAPI void APIENTRY glGetObjectLabelEXT( GLenum type, GLuint object, GLsizei bufSize, GLsizei* length, GLchar* label) {
    CALL_5(GetObjectLabelEXT, type, object, bufSize, length, label);
}

GLAPI void APIENTRY glGetObjectParameterfvARB( GLhandleARB obj, GLenum pname, GLfloat* params) {
    CALL_3(GetObjectParameterfvARB, obj, pname, params);
}

GLAPI void APIENTRY glGetObjectParameterivAPPLE( GLenum objectType, GLuint name, GLenum pname, GLint* params) {
    CALL_4(GetObjectParameterivAPPLE, objectType, name, pname, params);
}

GLAPI void APIENTRY glGetObjectParameterivARB( GLhandleARB obj, GLenum pname, GLint* params) {
    CALL_3(GetObjectParameterivARB, obj, pname, params);
}

GLAPI void APIENTRY glGetObjectPtrLabel( const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label) {
    CALL_4(GetObjectPtrLabel, ptr, bufSize, length, label);
}

GLAPI void APIENTRY glGetOcclusionQueryivNV( GLuint id, GLenum pname, GLint* params) {
    CALL_3(GetOcclusionQueryivNV, id, pname, params);
}

GLAPI void APIENTRY glGetOcclusionQueryuivNV( GLuint id, GLenum pname, GLuint* params) {
    CALL_3(GetOcclusionQueryuivNV, id, pname, params);
}

GLAPI void APIENTRY glGetPathColorGenfvNV( GLenum color, GLenum pname, GLfloat* value) {
    CALL_3(GetPathColorGenfvNV, color, pname, value);
}

GLAPI void APIENTRY glGetPathColorGenivNV( GLenum color, GLenum pname, GLint* value) {
    CALL_3(GetPathColorGenivNV, color, pname, value);
}

GLAPI void APIENTRY glGetPathCommandsNV( GLuint path, GLubyte* commands) {
    CALL_2(GetPathCommandsNV, path, commands);
}

GLAPI void APIENTRY glGetPathCoordsNV( GLuint path, GLfloat* coords) {
    CALL_2(GetPathCoordsNV, path, coords);
}

GLAPI void APIENTRY glGetPathDashArrayNV( GLuint path, GLfloat* dashArray) {
    CALL_2(GetPathDashArrayNV, path, dashArray);
}

GLAPI GLfloat APIENTRY glGetPathLengthNV( GLuint path, GLsizei startSegment, GLsizei numSegments) {
    CALL_3_R(GetPathLengthNV, path, startSegment, numSegments);
}

GLAPI void APIENTRY glGetPathMetricRangeNV( GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat* metrics) {
    CALL_5(GetPathMetricRangeNV, metricQueryMask, firstPathName, numPaths, stride, metrics);
}

GLAPI void APIENTRY glGetPathMetricsNV( GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLsizei stride, GLfloat* metrics) {
    CALL_7(GetPathMetricsNV, metricQueryMask, numPaths, pathNameType, paths, pathBase, stride, metrics);
}

GLAPI void APIENTRY glGetPathParameterfvNV( GLuint path, GLenum pname, GLfloat* value) {
    CALL_3(GetPathParameterfvNV, path, pname, value);
}

GLAPI void APIENTRY glGetPathParameterivNV( GLuint path, GLenum pname, GLint* value) {
    CALL_3(GetPathParameterivNV, path, pname, value);
}

GLAPI void APIENTRY glGetPathSpacingNV( GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat* returnedSpacing) {
    CALL_9(GetPathSpacingNV, pathListMode, numPaths, pathNameType, paths, pathBase, F(advanceScale), F(kerningScale), transformType, returnedSpacing);
}

GLAPI void APIENTRY glGetPathTexGenfvNV( GLenum texCoordSet, GLenum pname, GLfloat* value) {
    CALL_3(GetPathTexGenfvNV, texCoordSet, pname, value);
}

GLAPI void APIENTRY glGetPathTexGenivNV( GLenum texCoordSet, GLenum pname, GLint* value) {
    CALL_3(GetPathTexGenivNV, texCoordSet, pname, value);
}

GLAPI void APIENTRY glGetPerfCounterInfoINTEL( GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar* counterName, GLuint counterDescLength, GLchar* counterDesc, GLuint* counterOffset, GLuint* counterDataSize, GLuint* counterTypeEnum, GLuint* counterDataTypeEnum, GLuint64* rawCounterMaxValue) {
    CALL_11(GetPerfCounterInfoINTEL, queryId, counterId, counterNameLength, counterName, counterDescLength, counterDesc, counterOffset, counterDataSize, counterTypeEnum, counterDataTypeEnum, rawCounterMaxValue);
}

GLAPI void APIENTRY glGetPerfMonitorCounterDataAMD( GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten) {
    CALL_5(GetPerfMonitorCounterDataAMD, monitor, pname, dataSize, data, bytesWritten);
}

GLAPI void APIENTRY glGetPerfMonitorCounterInfoAMD( GLuint group, GLuint counter, GLenum pname, void* data) {
    CALL_4(GetPerfMonitorCounterInfoAMD, group, counter, pname, data);
}

GLAPI void APIENTRY glGetPerfMonitorCounterStringAMD( GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, GLchar* counterString) {
    CALL_5(GetPerfMonitorCounterStringAMD, group, counter, bufSize, length, counterString);
}

GLAPI void APIENTRY glGetPerfMonitorCountersAMD( GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters) {
    CALL_5(GetPerfMonitorCountersAMD, group, numCounters, maxActiveCounters, counterSize, counters);
}

GLAPI void APIENTRY glGetPerfMonitorGroupStringAMD( GLuint group, GLsizei bufSize, GLsizei* length, GLchar* groupString) {
    CALL_4(GetPerfMonitorGroupStringAMD, group, bufSize, length, groupString);
}

GLAPI void APIENTRY glGetPerfMonitorGroupsAMD( GLint* numGroups, GLsizei groupsSize, GLuint* groups) {
    CALL_3(GetPerfMonitorGroupsAMD, numGroups, groupsSize, groups);
}

GLAPI void APIENTRY glGetPerfQueryDataINTEL( GLuint queryHandle, GLuint flags, GLsizei dataSize, GLvoid* data, GLuint* bytesWritten) {
    CALL_5(GetPerfQueryDataINTEL, queryHandle, flags, dataSize, data, bytesWritten);
}

GLAPI void APIENTRY glGetPerfQueryIdByNameINTEL( GLchar* queryName, GLuint* queryId) {
    CALL_2(GetPerfQueryIdByNameINTEL, queryName, queryId);
}

GLAPI void APIENTRY glGetPerfQueryInfoINTEL( GLuint queryId, GLuint queryNameLength, GLchar* queryName, GLuint* dataSize, GLuint* noCounters, GLuint* noInstances, GLuint* capsMask) {
    CALL_7(GetPerfQueryInfoINTEL, queryId, queryNameLength, queryName, dataSize, noCounters, noInstances, capsMask);
}

GLAPI void APIENTRY glGetPixelMapxv( GLenum map, GLint size, GLfixed* values) {
    CALL_3(GetPixelMapxv, map, size, values);
}

GLAPI void APIENTRY glGetPixelTexGenParameterfvSGIS( GLenum pname, GLfloat* params) {
    CALL_2(GetPixelTexGenParameterfvSGIS, pname, params);
}

GLAPI void APIENTRY glGetPixelTexGenParameterivSGIS( GLenum pname, GLint* params) {
    CALL_2(GetPixelTexGenParameterivSGIS, pname, params);
}

GLAPI void APIENTRY glGetPixelTransformParameterfvEXT( GLenum target, GLenum pname, GLfloat* params) {
    CALL_3(GetPixelTransformParameterfvEXT, target, pname, params);
}

GLAPI void APIENTRY glGetPixelTransformParameterivEXT( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetPixelTransformParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetPointerIndexedvEXT( GLenum target, GLuint index, void** data) {
    CALL_3(GetPointerIndexedvEXT, target, index, data);
}

GLAPI void APIENTRY glGetPointeri_vEXT( GLenum pname, GLuint index, void** params) {
    CALL_3(GetPointeri_vEXT, pname, index, params);
}

GLAPI void APIENTRY glGetPointervEXT( GLenum pname, void** params) {
    CALL_2(GetPointervEXT, pname, params);
}

GLAPI void APIENTRY glGetProgramBinary( GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary) {
    CALL_5(GetProgramBinary, program, bufSize, length, binaryFormat, binary);
}

GLAPI void APIENTRY glGetProgramEnvParameterIivNV( GLenum target, GLuint index, GLint* params) {
    CALL_3(GetProgramEnvParameterIivNV, target, index, params);
}

GLAPI void APIENTRY glGetProgramEnvParameterIuivNV( GLenum target, GLuint index, GLuint* params) {
    CALL_3(GetProgramEnvParameterIuivNV, target, index, params);
}

GLAPI void APIENTRY glGetProgramEnvParameterdvARB( GLenum target, GLuint index, GLdouble* params) {
    CALL_3(GetProgramEnvParameterdvARB, target, index, params);
}

GLAPI void APIENTRY glGetProgramEnvParameterfvARB( GLenum target, GLuint index, GLfloat* params) {
    CALL_3(GetProgramEnvParameterfvARB, target, index, params);
}

GLAPI void APIENTRY glGetProgramInfoLog( GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
    CALL_4(GetProgramInfoLog, program, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetProgramInterfaceiv( GLuint program, GLenum programInterface, GLenum pname, GLint* params) {
    CALL_4(GetProgramInterfaceiv, program, programInterface, pname, params);
}

GLAPI void APIENTRY glGetProgramLocalParameterIivNV( GLenum target, GLuint index, GLint* params) {
    CALL_3(GetProgramLocalParameterIivNV, target, index, params);
}

GLAPI void APIENTRY glGetProgramLocalParameterIuivNV( GLenum target, GLuint index, GLuint* params) {
    CALL_3(GetProgramLocalParameterIuivNV, target, index, params);
}

GLAPI void APIENTRY glGetProgramLocalParameterdvARB( GLenum target, GLuint index, GLdouble* params) {
    CALL_3(GetProgramLocalParameterdvARB, target, index, params);
}

GLAPI void APIENTRY glGetProgramLocalParameterfvARB( GLenum target, GLuint index, GLfloat* params) {
    CALL_3(GetProgramLocalParameterfvARB, target, index, params);
}

GLAPI void APIENTRY glGetProgramNamedParameterdvNV( GLuint id, GLsizei len, const GLubyte* name, GLdouble* params) {
    CALL_4(GetProgramNamedParameterdvNV, id, len, name, params);
}

GLAPI void APIENTRY glGetProgramNamedParameterfvNV( GLuint id, GLsizei len, const GLubyte* name, GLfloat* params) {
    CALL_4(GetProgramNamedParameterfvNV, id, len, name, params);
}

GLAPI void APIENTRY glGetProgramParameterdvNV( GLenum target, GLuint index, GLenum pname, GLdouble* params) {
    CALL_4(GetProgramParameterdvNV, target, index, pname, params);
}

GLAPI void APIENTRY glGetProgramParameterfvNV( GLenum target, GLuint index, GLenum pname, GLfloat* params) {
    CALL_4(GetProgramParameterfvNV, target, index, pname, params);
}

GLAPI void APIENTRY glGetProgramPipelineInfoLog( GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
    CALL_4(GetProgramPipelineInfoLog, pipeline, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetProgramPipelineiv( GLuint pipeline, GLenum pname, GLint* params) {
    CALL_3(GetProgramPipelineiv, pipeline, pname, params);
}

GLAPI GLuint APIENTRY glGetProgramResourceIndex( GLuint program, GLenum programInterface, const GLchar* name) {
    CALL_3_R(GetProgramResourceIndex, program, programInterface, name);
}

GLAPI GLint APIENTRY glGetProgramResourceLocation( GLuint program, GLenum programInterface, const GLchar* name) {
    CALL_3_R(GetProgramResourceLocation, program, programInterface, name);
}

GLAPI GLint APIENTRY glGetProgramResourceLocationIndex( GLuint program, GLenum programInterface, const GLchar* name) {
    CALL_3_R(GetProgramResourceLocationIndex, program, programInterface, name);
}

GLAPI void APIENTRY glGetProgramResourceName( GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name) {
    CALL_6(GetProgramResourceName, program, programInterface, index, bufSize, length, name);
}

GLAPI void APIENTRY glGetProgramResourcefvNV( GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLfloat* params) {
    CALL_8(GetProgramResourcefvNV, program, programInterface, index, propCount, props, bufSize, length, params);
}

GLAPI void APIENTRY glGetProgramResourceiv( GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params) {
    CALL_8(GetProgramResourceiv, program, programInterface, index, propCount, props, bufSize, length, params);
}

GLAPI void APIENTRY glGetProgramStageiv( GLuint program, GLenum shadertype, GLenum pname, GLint* values) {
    CALL_4(GetProgramStageiv, program, shadertype, pname, values);
}

GLAPI void APIENTRY glGetProgramStringARB( GLenum target, GLenum pname, void* string) {
    CALL_3(GetProgramStringARB, target, pname, string);
}

GLAPI void APIENTRY glGetProgramStringNV( GLuint id, GLenum pname, GLubyte* program) {
    CALL_3(GetProgramStringNV, id, pname, program);
}

GLAPI void APIENTRY glGetProgramSubroutineParameteruivNV( GLenum target, GLuint index, GLuint* param) {
    CALL_3(GetProgramSubroutineParameteruivNV, target, index, param);
}

GLAPI void APIENTRY glGetProgramiv( GLuint program, GLenum pname, GLint* params) {
    CALL_3(GetProgramiv, program, pname, params);
}

GLAPI void APIENTRY glGetProgramivARB( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetProgramivARB, target, pname, params);
}

GLAPI void APIENTRY glGetProgramivNV( GLuint id, GLenum pname, GLint* params) {
    CALL_3(GetProgramivNV, id, pname, params);
}

GLAPI void APIENTRY glGetQueryBufferObjecti64v( GLuint id, GLuint buffer, GLenum pname, GLintptr offset) {
    CALL_4(GetQueryBufferObjecti64v, id, buffer, pname, offset);
}

GLAPI void APIENTRY glGetQueryBufferObjectiv( GLuint id, GLuint buffer, GLenum pname, GLintptr offset) {
    CALL_4(GetQueryBufferObjectiv, id, buffer, pname, offset);
}

GLAPI void APIENTRY glGetQueryBufferObjectui64v( GLuint id, GLuint buffer, GLenum pname, GLintptr offset) {
    CALL_4(GetQueryBufferObjectui64v, id, buffer, pname, offset);
}

GLAPI void APIENTRY glGetQueryBufferObjectuiv( GLuint id, GLuint buffer, GLenum pname, GLintptr offset) {
    CALL_4(GetQueryBufferObjectuiv, id, buffer, pname, offset);
}

GLAPI void APIENTRY glGetQueryIndexediv( GLenum target, GLuint index, GLenum pname, GLint* params) {
    CALL_4(GetQueryIndexediv, target, index, pname, params);
}

GLAPI void APIENTRY glGetQueryObjecti64v( GLuint id, GLenum pname, GLint64* params) {
    CALL_3(GetQueryObjecti64v, id, pname, params);
}

GLAPI void APIENTRY glGetQueryObjecti64vEXT( GLuint id, GLenum pname, GLint64* params) {
    CALL_3(GetQueryObjecti64vEXT, id, pname, params);
}

GLAPI void APIENTRY glGetQueryObjectiv( GLuint id, GLenum pname, GLint* params) {
    CALL_3(GetQueryObjectiv, id, pname, params);
}

GLAPI void APIENTRY glGetQueryObjectivARB( GLuint id, GLenum pname, GLint* params) {
    CALL_3(GetQueryObjectivARB, id, pname, params);
}

GLAPI void APIENTRY glGetQueryObjectui64v( GLuint id, GLenum pname, GLuint64* params) {
    CALL_3(GetQueryObjectui64v, id, pname, params);
}

GLAPI void APIENTRY glGetQueryObjectui64vEXT( GLuint id, GLenum pname, GLuint64* params) {
    CALL_3(GetQueryObjectui64vEXT, id, pname, params);
}

GLAPI void APIENTRY glGetQueryObjectuiv( GLuint id, GLenum pname, GLuint* params) {
    CALL_3(GetQueryObjectuiv, id, pname, params);
}

GLAPI void APIENTRY glGetQueryObjectuivARB( GLuint id, GLenum pname, GLuint* params) {
    CALL_3(GetQueryObjectuivARB, id, pname, params);
}

GLAPI void APIENTRY glGetQueryiv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetQueryiv, target, pname, params);
}

GLAPI void APIENTRY glGetQueryivARB( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetQueryivARB, target, pname, params);
}

GLAPI void APIENTRY glGetRenderbufferParameteriv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetRenderbufferParameteriv, target, pname, params);
}

GLAPI void APIENTRY glGetRenderbufferParameterivEXT( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetRenderbufferParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetSamplerParameterIiv( GLuint sampler, GLenum pname, GLint* params) {
    CALL_3(GetSamplerParameterIiv, sampler, pname, params);
}

GLAPI void APIENTRY glGetSamplerParameterIuiv( GLuint sampler, GLenum pname, GLuint* params) {
    CALL_3(GetSamplerParameterIuiv, sampler, pname, params);
}

GLAPI void APIENTRY glGetSamplerParameterfv( GLuint sampler, GLenum pname, GLfloat* params) {
    CALL_3(GetSamplerParameterfv, sampler, pname, params);
}

GLAPI void APIENTRY glGetSamplerParameteriv( GLuint sampler, GLenum pname, GLint* params) {
    CALL_3(GetSamplerParameteriv, sampler, pname, params);
}

GLAPI void APIENTRY glGetSeparableFilter( GLenum target, GLenum format, GLenum type, void* row, void* column, void* span) {
    CALL_6(GetSeparableFilter, target, format, type, row, column, span);
}

GLAPI void APIENTRY glGetSeparableFilterEXT( GLenum target, GLenum format, GLenum type, void* row, void* column, void* span) {
    CALL_6(GetSeparableFilterEXT, target, format, type, row, column, span);
}

GLAPI void APIENTRY glGetShaderInfoLog( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
    CALL_4(GetShaderInfoLog, shader, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetShaderPrecisionFormat( GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) {
    CALL_4(GetShaderPrecisionFormat, shadertype, precisiontype, range, precision);
}

GLAPI void APIENTRY glGetShaderSource( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source) {
    CALL_4(GetShaderSource, shader, bufSize, length, source);
}

GLAPI void APIENTRY glGetShaderSourceARB( GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB* source) {
    CALL_4(GetShaderSourceARB, obj, maxLength, length, source);
}

GLAPI void APIENTRY glGetShaderiv( GLuint shader, GLenum pname, GLint* params) {
    CALL_3(GetShaderiv, shader, pname, params);
}

GLAPI void APIENTRY glGetSharpenTexFuncSGIS( GLenum target, GLfloat* points) {
    CALL_2(GetSharpenTexFuncSGIS, target, points);
}

GLAPI GLushort APIENTRY glGetStageIndexNV( GLenum shadertype) {
    CALL_1_R(GetStageIndexNV, shadertype);
}

GLAPI GLuint APIENTRY glGetSubroutineIndex( GLuint program, GLenum shadertype, const GLchar* name) {
    CALL_3_R(GetSubroutineIndex, program, shadertype, name);
}

GLAPI GLint APIENTRY glGetSubroutineUniformLocation( GLuint program, GLenum shadertype, const GLchar* name) {
    CALL_3_R(GetSubroutineUniformLocation, program, shadertype, name);
}

GLAPI void APIENTRY glGetSynciv( GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) {
    CALL_5(GetSynciv, sync, pname, bufSize, length, values);
}

GLAPI void APIENTRY glGetTexBumpParameterfvATI( GLenum pname, GLfloat* param) {
    CALL_2(GetTexBumpParameterfvATI, pname, param);
}

GLAPI void APIENTRY glGetTexBumpParameterivATI( GLenum pname, GLint* param) {
    CALL_2(GetTexBumpParameterivATI, pname, param);
}

GLAPI void APIENTRY glGetTexEnvxvOES( GLenum target, GLenum pname, GLfixed* params) {
    CALL_3(GetTexEnvxvOES, target, pname, params);
}

GLAPI void APIENTRY glGetTexFilterFuncSGIS( GLenum target, GLenum filter, GLfloat* weights) {
    CALL_3(GetTexFilterFuncSGIS, target, filter, weights);
}

GLAPI void APIENTRY glGetTexGenxvOES( GLenum coord, GLenum pname, GLfixed* params) {
    CALL_3(GetTexGenxvOES, coord, pname, params);
}

GLAPI void APIENTRY glGetTexLevelParameterxvOES( GLenum target, GLint level, GLenum pname, GLfixed* params) {
    CALL_4(GetTexLevelParameterxvOES, target, level, pname, params);
}

GLAPI void APIENTRY glGetTexParameterIiv( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetTexParameterIiv, target, pname, params);
}

GLAPI void APIENTRY glGetTexParameterIivEXT( GLenum target, GLenum pname, GLint* params) {
    CALL_3(GetTexParameterIivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetTexParameterIuiv( GLenum target, GLenum pname, GLuint* params) {
    CALL_3(GetTexParameterIuiv, target, pname, params);
}

GLAPI void APIENTRY glGetTexParameterIuivEXT( GLenum target, GLenum pname, GLuint* params) {
    CALL_3(GetTexParameterIuivEXT, target, pname, params);
}

GLAPI void APIENTRY glGetTexParameterPointervAPPLE( GLenum target, GLenum pname, void** params) {
    CALL_3(GetTexParameterPointervAPPLE, target, pname, params);
}

GLAPI void APIENTRY glGetTexParameterxvOES( GLenum target, GLenum pname, GLfixed* params) {
    CALL_3(GetTexParameterxvOES, target, pname, params);
}

GLAPI GLuint64 APIENTRY glGetTextureHandleARB( GLuint texture) {
    CALL_1_R(GetTextureHandleARB, texture);
}

GLAPI GLuint64 APIENTRY glGetTextureHandleNV( GLuint texture) {
    CALL_1_R(GetTextureHandleNV, texture);
}

GLAPI void APIENTRY glGetTextureImage( GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels) {
    CALL_6(GetTextureImage, texture, level, format, type, bufSize, pixels);
}

GLAPI void APIENTRY glGetTextureImageEXT( GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
    CALL_6(GetTextureImageEXT, texture, target, level, format, type, pixels);
}

GLAPI void APIENTRY glGetTextureLevelParameterfv( GLuint texture, GLint level, GLenum pname, GLfloat* params) {
    CALL_4(GetTextureLevelParameterfv, texture, level, pname, params);
}

GLAPI void APIENTRY glGetTextureLevelParameterfvEXT( GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params) {
    CALL_5(GetTextureLevelParameterfvEXT, texture, target, level, pname, params);
}

GLAPI void APIENTRY glGetTextureLevelParameteriv( GLuint texture, GLint level, GLenum pname, GLint* params) {
    CALL_4(GetTextureLevelParameteriv, texture, level, pname, params);
}

GLAPI void APIENTRY glGetTextureLevelParameterivEXT( GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params) {
    CALL_5(GetTextureLevelParameterivEXT, texture, target, level, pname, params);
}

GLAPI void APIENTRY glGetTextureParameterIiv( GLuint texture, GLenum pname, GLint* params) {
    CALL_3(GetTextureParameterIiv, texture, pname, params);
}

GLAPI void APIENTRY glGetTextureParameterIivEXT( GLuint texture, GLenum target, GLenum pname, GLint* params) {
    CALL_4(GetTextureParameterIivEXT, texture, target, pname, params);
}

GLAPI void APIENTRY glGetTextureParameterIuiv( GLuint texture, GLenum pname, GLuint* params) {
    CALL_3(GetTextureParameterIuiv, texture, pname, params);
}

GLAPI void APIENTRY glGetTextureParameterIuivEXT( GLuint texture, GLenum target, GLenum pname, GLuint* params) {
    CALL_4(GetTextureParameterIuivEXT, texture, target, pname, params);
}

GLAPI void APIENTRY glGetTextureParameterfv( GLuint texture, GLenum pname, GLfloat* params) {
    CALL_3(GetTextureParameterfv, texture, pname, params);
}

GLAPI void APIENTRY glGetTextureParameterfvEXT( GLuint texture, GLenum target, GLenum pname, GLfloat* params) {
    CALL_4(GetTextureParameterfvEXT, texture, target, pname, params);
}

GLAPI void APIENTRY glGetTextureParameteriv( GLuint texture, GLenum pname, GLint* params) {
    CALL_3(GetTextureParameteriv, texture, pname, params);
}

GLAPI void APIENTRY glGetTextureParameterivEXT( GLuint texture, GLenum target, GLenum pname, GLint* params) {
    CALL_4(GetTextureParameterivEXT, texture, target, pname, params);
}

GLAPI GLuint64 APIENTRY glGetTextureSamplerHandleARB( GLuint texture, GLuint sampler) {
    CALL_2_R(GetTextureSamplerHandleARB, texture, sampler);
}

GLAPI GLuint64 APIENTRY glGetTextureSamplerHandleNV( GLuint texture, GLuint sampler) {
    CALL_2_R(GetTextureSamplerHandleNV, texture, sampler);
}

GLAPI void APIENTRY glGetTextureSubImage( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels) {
    CALL_12(GetTextureSubImage, texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, bufSize, pixels);
}

GLAPI void APIENTRY glGetTrackMatrixivNV( GLenum target, GLuint address, GLenum pname, GLint* params) {
    CALL_4(GetTrackMatrixivNV, target, address, pname, params);
}

GLAPI void APIENTRY glGetTransformFeedbackVarying( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) {
    CALL_7(GetTransformFeedbackVarying, program, index, bufSize, length, size, type, name);
}

GLAPI void APIENTRY glGetTransformFeedbackVaryingEXT( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) {
    CALL_7(GetTransformFeedbackVaryingEXT, program, index, bufSize, length, size, type, name);
}

GLAPI void APIENTRY glGetTransformFeedbackVaryingNV( GLuint program, GLuint index, GLint* location) {
    CALL_3(GetTransformFeedbackVaryingNV, program, index, location);
}

GLAPI void APIENTRY glGetTransformFeedbacki64_v( GLuint xfb, GLenum pname, GLuint index, GLint64* param) {
    CALL_4(GetTransformFeedbacki64_v, xfb, pname, index, param);
}

GLAPI void APIENTRY glGetTransformFeedbacki_v( GLuint xfb, GLenum pname, GLuint index, GLint* param) {
    CALL_4(GetTransformFeedbacki_v, xfb, pname, index, param);
}

GLAPI void APIENTRY glGetTransformFeedbackiv( GLuint xfb, GLenum pname, GLint* param) {
    CALL_3(GetTransformFeedbackiv, xfb, pname, param);
}

GLAPI GLuint APIENTRY glGetUniformBlockIndex( GLuint program, const GLchar* uniformBlockName) {
    CALL_2_R(GetUniformBlockIndex, program, uniformBlockName);
}

GLAPI GLint APIENTRY glGetUniformBufferSizeEXT( GLuint program, GLint location) {
    CALL_2_R(GetUniformBufferSizeEXT, program, location);
}

GLAPI void APIENTRY glGetUniformIndices( GLuint program, GLsizei uniformCount, const GLchar*const* uniformNames, GLuint* uniformIndices) {
    CALL_4(GetUniformIndices, program, uniformCount, uniformNames, uniformIndices);
}

GLAPI GLint APIENTRY glGetUniformLocation( GLuint program, const GLchar* name) {
    CALL_2_R(GetUniformLocation, program, name);
}

GLAPI GLint APIENTRY glGetUniformLocationARB( GLhandleARB programObj, const GLcharARB* name) {
    CALL_2_R(GetUniformLocationARB, programObj, name);
}

GLAPI GLintptr APIENTRY glGetUniformOffsetEXT( GLuint program, GLint location) {
    CALL_2_R(GetUniformOffsetEXT, program, location);
}

GLAPI void APIENTRY glGetUniformSubroutineuiv( GLenum shadertype, GLint location, GLuint* params) {
    CALL_3(GetUniformSubroutineuiv, shadertype, location, params);
}

GLAPI void APIENTRY glGetUniformdv( GLuint program, GLint location, GLdouble* params) {
    CALL_3(GetUniformdv, program, location, params);
}

GLAPI void APIENTRY glGetUniformfv( GLuint program, GLint location, GLfloat* params) {
    CALL_3(GetUniformfv, program, location, params);
}

GLAPI void APIENTRY glGetUniformfvARB( GLhandleARB programObj, GLint location, GLfloat* params) {
    CALL_3(GetUniformfvARB, programObj, location, params);
}

GLAPI void APIENTRY glGetUniformi64vARB( GLuint program, GLint location, GLint64* params) {
    CALL_3(GetUniformi64vARB, program, location, params);
}

GLAPI void APIENTRY glGetUniformi64vNV( GLuint program, GLint location, GLint64EXT* params) {
    CALL_3(GetUniformi64vNV, program, location, params);
}

GLAPI void APIENTRY glGetUniformiv( GLuint program, GLint location, GLint* params) {
    CALL_3(GetUniformiv, program, location, params);
}

GLAPI void APIENTRY glGetUniformivARB( GLhandleARB programObj, GLint location, GLint* params) {
    CALL_3(GetUniformivARB, programObj, location, params);
}

GLAPI void APIENTRY glGetUniformui64vARB( GLuint program, GLint location, GLuint64* params) {
    CALL_3(GetUniformui64vARB, program, location, params);
}

GLAPI void APIENTRY glGetUniformui64vNV( GLuint program, GLint location, GLuint64EXT* params) {
    CALL_3(GetUniformui64vNV, program, location, params);
}

GLAPI void APIENTRY glGetUniformuiv( GLuint program, GLint location, GLuint* params) {
    CALL_3(GetUniformuiv, program, location, params);
}

GLAPI void APIENTRY glGetUniformuivEXT( GLuint program, GLint location, GLuint* params) {
    CALL_3(GetUniformuivEXT, program, location, params);
}

GLAPI void APIENTRY glGetVariantArrayObjectfvATI( GLuint id, GLenum pname, GLfloat* params) {
    CALL_3(GetVariantArrayObjectfvATI, id, pname, params);
}

GLAPI void APIENTRY glGetVariantArrayObjectivATI( GLuint id, GLenum pname, GLint* params) {
    CALL_3(GetVariantArrayObjectivATI, id, pname, params);
}

GLAPI void APIENTRY glGetVariantBooleanvEXT( GLuint id, GLenum value, GLboolean* data) {
    CALL_3(GetVariantBooleanvEXT, id, value, data);
}

GLAPI void APIENTRY glGetVariantFloatvEXT( GLuint id, GLenum value, GLfloat* data) {
    CALL_3(GetVariantFloatvEXT, id, value, data);
}

GLAPI void APIENTRY glGetVariantIntegervEXT( GLuint id, GLenum value, GLint* data) {
    CALL_3(GetVariantIntegervEXT, id, value, data);
}

GLAPI void APIENTRY glGetVariantPointervEXT( GLuint id, GLenum value, void** data) {
    CALL_3(GetVariantPointervEXT, id, value, data);
}

GLAPI GLint APIENTRY glGetVaryingLocationNV( GLuint program, const GLchar* name) {
    CALL_2_R(GetVaryingLocationNV, program, name);
}

GLAPI void APIENTRY glGetVertexArrayIndexed64iv( GLuint vaobj, GLuint index, GLenum pname, GLint64* param) {
    CALL_4(GetVertexArrayIndexed64iv, vaobj, index, pname, param);
}

GLAPI void APIENTRY glGetVertexArrayIndexediv( GLuint vaobj, GLuint index, GLenum pname, GLint* param) {
    CALL_4(GetVertexArrayIndexediv, vaobj, index, pname, param);
}

GLAPI void APIENTRY glGetVertexArrayIntegeri_vEXT( GLuint vaobj, GLuint index, GLenum pname, GLint* param) {
    CALL_4(GetVertexArrayIntegeri_vEXT, vaobj, index, pname, param);
}

GLAPI void APIENTRY glGetVertexArrayIntegervEXT( GLuint vaobj, GLenum pname, GLint* param) {
    CALL_3(GetVertexArrayIntegervEXT, vaobj, pname, param);
}

GLAPI void APIENTRY glGetVertexArrayPointeri_vEXT( GLuint vaobj, GLuint index, GLenum pname, void** param) {
    CALL_4(GetVertexArrayPointeri_vEXT, vaobj, index, pname, param);
}

GLAPI void APIENTRY glGetVertexArrayPointervEXT( GLuint vaobj, GLenum pname, void** param) {
    CALL_3(GetVertexArrayPointervEXT, vaobj, pname, param);
}

GLAPI void APIENTRY glGetVertexArrayiv( GLuint vaobj, GLenum pname, GLint* param) {
    CALL_3(GetVertexArrayiv, vaobj, pname, param);
}

GLAPI void APIENTRY glGetVertexAttribArrayObjectfvATI( GLuint index, GLenum pname, GLfloat* params) {
    CALL_3(GetVertexAttribArrayObjectfvATI, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribArrayObjectivATI( GLuint index, GLenum pname, GLint* params) {
    CALL_3(GetVertexAttribArrayObjectivATI, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribIiv( GLuint index, GLenum pname, GLint* params) {
    CALL_3(GetVertexAttribIiv, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribIivEXT( GLuint index, GLenum pname, GLint* params) {
    CALL_3(GetVertexAttribIivEXT, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribIuiv( GLuint index, GLenum pname, GLuint* params) {
    CALL_3(GetVertexAttribIuiv, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribIuivEXT( GLuint index, GLenum pname, GLuint* params) {
    CALL_3(GetVertexAttribIuivEXT, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribLdv( GLuint index, GLenum pname, GLdouble* params) {
    CALL_3(GetVertexAttribLdv, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribLdvEXT( GLuint index, GLenum pname, GLdouble* params) {
    CALL_3(GetVertexAttribLdvEXT, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribLi64vNV( GLuint index, GLenum pname, GLint64EXT* params) {
    CALL_3(GetVertexAttribLi64vNV, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribLui64vARB( GLuint index, GLenum pname, GLuint64EXT* params) {
    CALL_3(GetVertexAttribLui64vARB, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribLui64vNV( GLuint index, GLenum pname, GLuint64EXT* params) {
    CALL_3(GetVertexAttribLui64vNV, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribPointerv( GLuint index, GLenum pname, void** pointer) {
    CALL_3(GetVertexAttribPointerv, index, pname, pointer);
}

GLAPI void APIENTRY glGetVertexAttribPointervARB( GLuint index, GLenum pname, void** pointer) {
    CALL_3(GetVertexAttribPointervARB, index, pname, pointer);
}

GLAPI void APIENTRY glGetVertexAttribPointervNV( GLuint index, GLenum pname, void** pointer) {
    CALL_3(GetVertexAttribPointervNV, index, pname, pointer);
}

GLAPI void APIENTRY glGetVertexAttribdv( GLuint index, GLenum pname, GLdouble* params) {
    CALL_3(GetVertexAttribdv, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribdvARB( GLuint index, GLenum pname, GLdouble* params) {
    CALL_3(GetVertexAttribdvARB, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribdvNV( GLuint index, GLenum pname, GLdouble* params) {
    CALL_3(GetVertexAttribdvNV, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribfv( GLuint index, GLenum pname, GLfloat* params) {
    CALL_3(GetVertexAttribfv, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribfvARB( GLuint index, GLenum pname, GLfloat* params) {
    CALL_3(GetVertexAttribfvARB, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribfvNV( GLuint index, GLenum pname, GLfloat* params) {
    CALL_3(GetVertexAttribfvNV, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribiv( GLuint index, GLenum pname, GLint* params) {
    CALL_3(GetVertexAttribiv, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribivARB( GLuint index, GLenum pname, GLint* params) {
    CALL_3(GetVertexAttribivARB, index, pname, params);
}

GLAPI void APIENTRY glGetVertexAttribivNV( GLuint index, GLenum pname, GLint* params) {
    CALL_3(GetVertexAttribivNV, index, pname, params);
}

GLAPI void APIENTRY glGetVideoCaptureStreamdvNV( GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble* params) {
    CALL_4(GetVideoCaptureStreamdvNV, video_capture_slot, stream, pname, params);
}

GLAPI void APIENTRY glGetVideoCaptureStreamfvNV( GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat* params) {
    CALL_4(GetVideoCaptureStreamfvNV, video_capture_slot, stream, pname, params);
}

GLAPI void APIENTRY glGetVideoCaptureStreamivNV( GLuint video_capture_slot, GLuint stream, GLenum pname, GLint* params) {
    CALL_4(GetVideoCaptureStreamivNV, video_capture_slot, stream, pname, params);
}

GLAPI void APIENTRY glGetVideoCaptureivNV( GLuint video_capture_slot, GLenum pname, GLint* params) {
    CALL_3(GetVideoCaptureivNV, video_capture_slot, pname, params);
}

GLAPI void APIENTRY glGetVideoi64vNV( GLuint video_slot, GLenum pname, GLint64EXT* params) {
    CALL_3(GetVideoi64vNV, video_slot, pname, params);
}

GLAPI void APIENTRY glGetVideoivNV( GLuint video_slot, GLenum pname, GLint* params) {
    CALL_3(GetVideoivNV, video_slot, pname, params);
}

GLAPI void APIENTRY glGetVideoui64vNV( GLuint video_slot, GLenum pname, GLuint64EXT* params) {
    CALL_3(GetVideoui64vNV, video_slot, pname, params);
}

GLAPI void APIENTRY glGetVideouivNV( GLuint video_slot, GLenum pname, GLuint* params) {
    CALL_3(GetVideouivNV, video_slot, pname, params);
}

GLAPI void APIENTRY glGetnColorTable( GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* table) {
    CALL_5(GetnColorTable, target, format, type, bufSize, table);
}

GLAPI void APIENTRY glGetnColorTableARB( GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* table) {
    CALL_5(GetnColorTableARB, target, format, type, bufSize, table);
}

GLAPI void APIENTRY glGetnCompressedTexImage( GLenum target, GLint lod, GLsizei bufSize, void* pixels) {
    CALL_4(GetnCompressedTexImage, target, lod, bufSize, pixels);
}

GLAPI void APIENTRY glGetnCompressedTexImageARB( GLenum target, GLint lod, GLsizei bufSize, void* img) {
    CALL_4(GetnCompressedTexImageARB, target, lod, bufSize, img);
}

GLAPI void APIENTRY glGetnConvolutionFilter( GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* image) {
    CALL_5(GetnConvolutionFilter, target, format, type, bufSize, image);
}

GLAPI void APIENTRY glGetnConvolutionFilterARB( GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* image) {
    CALL_5(GetnConvolutionFilterARB, target, format, type, bufSize, image);
}

GLAPI void APIENTRY glGetnHistogram( GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) {
    CALL_6(GetnHistogram, target, reset, format, type, bufSize, values);
}

GLAPI void APIENTRY glGetnHistogramARB( GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) {
    CALL_6(GetnHistogramARB, target, reset, format, type, bufSize, values);
}

GLAPI void APIENTRY glGetnMapdv( GLenum target, GLenum query, GLsizei bufSize, GLdouble* v) {
    CALL_4(GetnMapdv, target, query, bufSize, v);
}

GLAPI void APIENTRY glGetnMapdvARB( GLenum target, GLenum query, GLsizei bufSize, GLdouble* v) {
    CALL_4(GetnMapdvARB, target, query, bufSize, v);
}

GLAPI void APIENTRY glGetnMapfv( GLenum target, GLenum query, GLsizei bufSize, GLfloat* v) {
    CALL_4(GetnMapfv, target, query, bufSize, v);
}

GLAPI void APIENTRY glGetnMapfvARB( GLenum target, GLenum query, GLsizei bufSize, GLfloat* v) {
    CALL_4(GetnMapfvARB, target, query, bufSize, v);
}

GLAPI void APIENTRY glGetnMapiv( GLenum target, GLenum query, GLsizei bufSize, GLint* v) {
    CALL_4(GetnMapiv, target, query, bufSize, v);
}

GLAPI void APIENTRY glGetnMapivARB( GLenum target, GLenum query, GLsizei bufSize, GLint* v) {
    CALL_4(GetnMapivARB, target, query, bufSize, v);
}

GLAPI void APIENTRY glGetnMinmax( GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) {
    CALL_6(GetnMinmax, target, reset, format, type, bufSize, values);
}

GLAPI void APIENTRY glGetnMinmaxARB( GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values) {
    CALL_6(GetnMinmaxARB, target, reset, format, type, bufSize, values);
}

GLAPI void APIENTRY glGetnPixelMapfv( GLenum map, GLsizei bufSize, GLfloat* values) {
    CALL_3(GetnPixelMapfv, map, bufSize, values);
}

GLAPI void APIENTRY glGetnPixelMapfvARB( GLenum map, GLsizei bufSize, GLfloat* values) {
    CALL_3(GetnPixelMapfvARB, map, bufSize, values);
}

GLAPI void APIENTRY glGetnPixelMapuiv( GLenum map, GLsizei bufSize, GLuint* values) {
    CALL_3(GetnPixelMapuiv, map, bufSize, values);
}

GLAPI void APIENTRY glGetnPixelMapuivARB( GLenum map, GLsizei bufSize, GLuint* values) {
    CALL_3(GetnPixelMapuivARB, map, bufSize, values);
}

GLAPI void APIENTRY glGetnPixelMapusv( GLenum map, GLsizei bufSize, GLushort* values) {
    CALL_3(GetnPixelMapusv, map, bufSize, values);
}

GLAPI void APIENTRY glGetnPixelMapusvARB( GLenum map, GLsizei bufSize, GLushort* values) {
    CALL_3(GetnPixelMapusvARB, map, bufSize, values);
}

GLAPI void APIENTRY glGetnPolygonStipple( GLsizei bufSize, GLubyte* pattern) {
    CALL_2(GetnPolygonStipple, bufSize, pattern);
}

GLAPI void APIENTRY glGetnPolygonStippleARB( GLsizei bufSize, GLubyte* pattern) {
    CALL_2(GetnPolygonStippleARB, bufSize, pattern);
}

GLAPI void APIENTRY glGetnSeparableFilter( GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void* row, GLsizei columnBufSize, void* column, void* span) {
    CALL_8(GetnSeparableFilter, target, format, type, rowBufSize, row, columnBufSize, column, span);
}

GLAPI void APIENTRY glGetnSeparableFilterARB( GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void* row, GLsizei columnBufSize, void* column, void* span) {
    CALL_8(GetnSeparableFilterARB, target, format, type, rowBufSize, row, columnBufSize, column, span);
}

GLAPI void APIENTRY glGetnTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels) {
    CALL_6(GetnTexImage, target, level, format, type, bufSize, pixels);
}

GLAPI void APIENTRY glGetnTexImageARB( GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* img) {
    CALL_6(GetnTexImageARB, target, level, format, type, bufSize, img);
}

GLAPI void APIENTRY glGetnUniformdv( GLuint program, GLint location, GLsizei bufSize, GLdouble* params) {
    CALL_4(GetnUniformdv, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformdvARB( GLuint program, GLint location, GLsizei bufSize, GLdouble* params) {
    CALL_4(GetnUniformdvARB, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformfv( GLuint program, GLint location, GLsizei bufSize, GLfloat* params) {
    CALL_4(GetnUniformfv, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformfvARB( GLuint program, GLint location, GLsizei bufSize, GLfloat* params) {
    CALL_4(GetnUniformfvARB, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformi64vARB( GLuint program, GLint location, GLsizei bufSize, GLint64* params) {
    CALL_4(GetnUniformi64vARB, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformiv( GLuint program, GLint location, GLsizei bufSize, GLint* params) {
    CALL_4(GetnUniformiv, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformivARB( GLuint program, GLint location, GLsizei bufSize, GLint* params) {
    CALL_4(GetnUniformivARB, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformui64vARB( GLuint program, GLint location, GLsizei bufSize, GLuint64* params) {
    CALL_4(GetnUniformui64vARB, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformuiv( GLuint program, GLint location, GLsizei bufSize, GLuint* params) {
    CALL_4(GetnUniformuiv, program, location, bufSize, params);
}

GLAPI void APIENTRY glGetnUniformuivARB( GLuint program, GLint location, GLsizei bufSize, GLuint* params) {
    CALL_4(GetnUniformuivARB, program, location, bufSize, params);
}

GLAPI void APIENTRY glGlobalAlphaFactorbSUN( GLbyte factor) {
    CALL_1(GlobalAlphaFactorbSUN, factor);
}

GLAPI void APIENTRY glGlobalAlphaFactordSUN( GLdouble factor) {
    CALL_1(GlobalAlphaFactordSUN, D(factor));
}

GLAPI void APIENTRY glGlobalAlphaFactorfSUN( GLfloat factor) {
    CALL_1(GlobalAlphaFactorfSUN, F(factor));
}

GLAPI void APIENTRY glGlobalAlphaFactoriSUN( GLint factor) {
    CALL_1(GlobalAlphaFactoriSUN, factor);
}

GLAPI void APIENTRY glGlobalAlphaFactorsSUN( GLshort factor) {
    CALL_1(GlobalAlphaFactorsSUN, factor);
}

GLAPI void APIENTRY glGlobalAlphaFactorubSUN( GLubyte factor) {
    CALL_1(GlobalAlphaFactorubSUN, factor);
}

GLAPI void APIENTRY glGlobalAlphaFactoruiSUN( GLuint factor) {
    CALL_1(GlobalAlphaFactoruiSUN, factor);
}

GLAPI void APIENTRY glGlobalAlphaFactorusSUN( GLushort factor) {
    CALL_1(GlobalAlphaFactorusSUN, factor);
}

GLAPI void APIENTRY glHintPGI( GLenum target, GLint mode) {
    CALL_2(HintPGI, target, mode);
}

GLAPI void APIENTRY glHistogram( GLenum target, GLsizei width, GLenum internalformat, GLboolean sink) {
    CALL_4(Histogram, target, width, internalformat, sink);
}

GLAPI void APIENTRY glHistogramEXT( GLenum target, GLsizei width, GLenum internalformat, GLboolean sink) {
    CALL_4(HistogramEXT, target, width, internalformat, sink);
}

GLAPI void APIENTRY glIglooInterfaceSGIX( GLenum pname, const void* params) {
    CALL_2(IglooInterfaceSGIX, pname, params);
}

GLAPI void APIENTRY glImageTransformParameterfHP( GLenum target, GLenum pname, GLfloat param) {
    CALL_3(ImageTransformParameterfHP, target, pname, F(param));
}

GLAPI void APIENTRY glImageTransformParameterfvHP( GLenum target, GLenum pname, const GLfloat* params) {
    CALL_3(ImageTransformParameterfvHP, target, pname, params);
}

GLAPI void APIENTRY glImageTransformParameteriHP( GLenum target, GLenum pname, GLint param) {
    CALL_3(ImageTransformParameteriHP, target, pname, param);
}

GLAPI void APIENTRY glImageTransformParameterivHP( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(ImageTransformParameterivHP, target, pname, params);
}

GLAPI GLsync APIENTRY glImportSyncEXT( GLenum external_sync_type, GLintptr external_sync, GLbitfield flags) {
    CALL_3_R(ImportSyncEXT, external_sync_type, external_sync, flags);
}

GLAPI void APIENTRY glIndexFormatNV( GLenum type, GLsizei stride) {
    CALL_2(IndexFormatNV, type, stride);
}

GLAPI void APIENTRY glIndexFuncEXT( GLenum func, GLclampf ref) {
    CALL_2(IndexFuncEXT, func, F(ref));
}

GLAPI void APIENTRY glIndexMaterialEXT( GLenum face, GLenum mode) {
    CALL_2(IndexMaterialEXT, face, mode);
}

GLAPI void APIENTRY glIndexPointerEXT( GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    CALL_4(IndexPointerEXT, type, stride, count, pointer);
}

GLAPI void APIENTRY glIndexPointerListIBM( GLenum type, GLint stride, const void** pointer, GLint ptrstride) {
    CALL_4(IndexPointerListIBM, type, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glIndexxOES( GLfixed component) {
    CALL_1(IndexxOES, component);
}

GLAPI void APIENTRY glIndexxvOES( const GLfixed* component) {
    CALL_1(IndexxvOES, component);
}

GLAPI void APIENTRY glInsertComponentEXT( GLuint res, GLuint src, GLuint num) {
    CALL_3(InsertComponentEXT, res, src, num);
}

GLAPI void APIENTRY glInsertEventMarkerEXT( GLsizei length, const GLchar* marker) {
    CALL_2(InsertEventMarkerEXT, length, marker);
}

GLAPI void APIENTRY glInstrumentsBufferSGIX( GLsizei size, GLint* buffer) {
    CALL_2(InstrumentsBufferSGIX, size, buffer);
}

GLAPI void APIENTRY glInterpolatePathsNV( GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight) {
    CALL_4(InterpolatePathsNV, resultPath, pathA, pathB, F(weight));
}

GLAPI void APIENTRY glInvalidateBufferData( GLuint buffer) {
    CALL_1(InvalidateBufferData, buffer);
}

GLAPI void APIENTRY glInvalidateBufferSubData( GLuint buffer, GLintptr offset, GLsizeiptr length) {
    CALL_3(InvalidateBufferSubData, buffer, offset, length);
}

GLAPI void APIENTRY glInvalidateFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments) {
    CALL_3(InvalidateFramebuffer, target, numAttachments, attachments);
}

GLAPI void APIENTRY glInvalidateNamedFramebufferData( GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments) {
    CALL_3(InvalidateNamedFramebufferData, framebuffer, numAttachments, attachments);
}

GLAPI void APIENTRY glInvalidateNamedFramebufferSubData( GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_7(InvalidateNamedFramebufferSubData, framebuffer, numAttachments, attachments, x, y, width, height);
}

GLAPI void APIENTRY glInvalidateSubFramebuffer( GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_7(InvalidateSubFramebuffer, target, numAttachments, attachments, x, y, width, height);
}

GLAPI void APIENTRY glInvalidateTexImage( GLuint texture, GLint level) {
    CALL_2(InvalidateTexImage, texture, level);
}

GLAPI void APIENTRY glInvalidateTexSubImage( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth) {
    CALL_8(InvalidateTexSubImage, texture, level, xoffset, yoffset, zoffset, width, height, depth);
}

GLAPI GLboolean APIENTRY glIsAsyncMarkerSGIX( GLuint marker) {
    CALL_1_R(IsAsyncMarkerSGIX, marker);
}

GLAPI GLboolean APIENTRY glIsBuffer( GLuint buffer) {
    CALL_1_R(IsBuffer, buffer);
}

GLAPI GLboolean APIENTRY glIsBufferARB( GLuint buffer) {
    CALL_1_R(IsBufferARB, buffer);
}

GLAPI GLboolean APIENTRY glIsBufferResidentNV( GLenum target) {
    CALL_1_R(IsBufferResidentNV, target);
}

GLAPI GLboolean APIENTRY glIsCommandListNV( GLuint list) {
    CALL_1_R(IsCommandListNV, list);
}

GLAPI GLboolean APIENTRY glIsEnabledIndexedEXT( GLenum target, GLuint index) {
    CALL_2_R(IsEnabledIndexedEXT, target, index);
}

GLAPI GLboolean APIENTRY glIsEnabledi( GLenum target, GLuint index) {
    CALL_2_R(IsEnabledi, target, index);
}

GLAPI GLboolean APIENTRY glIsFenceAPPLE( GLuint fence) {
    CALL_1_R(IsFenceAPPLE, fence);
}

GLAPI GLboolean APIENTRY glIsFenceNV( GLuint fence) {
    CALL_1_R(IsFenceNV, fence);
}

GLAPI GLboolean APIENTRY glIsFramebuffer( GLuint framebuffer) {
    CALL_1_R(IsFramebuffer, framebuffer);
}

GLAPI GLboolean APIENTRY glIsFramebufferEXT( GLuint framebuffer) {
    CALL_1_R(IsFramebufferEXT, framebuffer);
}

GLAPI GLboolean APIENTRY glIsImageHandleResidentARB( GLuint64 handle) {
    CALL_1_R(IsImageHandleResidentARB, LL(handle));
}

GLAPI GLboolean APIENTRY glIsImageHandleResidentNV( GLuint64 handle) {
    CALL_1_R(IsImageHandleResidentNV, LL(handle));
}

GLAPI GLboolean APIENTRY glIsNameAMD( GLenum identifier, GLuint name) {
    CALL_2_R(IsNameAMD, identifier, name);
}

GLAPI GLboolean APIENTRY glIsNamedBufferResidentNV( GLuint buffer) {
    CALL_1_R(IsNamedBufferResidentNV, buffer);
}

GLAPI GLboolean APIENTRY glIsNamedStringARB( GLint namelen, const GLchar* name) {
    CALL_2_R(IsNamedStringARB, namelen, name);
}

GLAPI GLboolean APIENTRY glIsObjectBufferATI( GLuint buffer) {
    CALL_1_R(IsObjectBufferATI, buffer);
}

GLAPI GLboolean APIENTRY glIsOcclusionQueryNV( GLuint id) {
    CALL_1_R(IsOcclusionQueryNV, id);
}

GLAPI GLboolean APIENTRY glIsPathNV( GLuint path) {
    CALL_1_R(IsPathNV, path);
}

GLAPI GLboolean APIENTRY glIsPointInFillPathNV( GLuint path, GLuint mask, GLfloat x, GLfloat y) {
    CALL_4_R(IsPointInFillPathNV, path, mask, F(x), F(y));
}

GLAPI GLboolean APIENTRY glIsPointInStrokePathNV( GLuint path, GLfloat x, GLfloat y) {
    CALL_3_R(IsPointInStrokePathNV, path, F(x), F(y));
}

GLAPI GLboolean APIENTRY glIsProgram( GLuint program) {
    CALL_1_R(IsProgram, program);
}

GLAPI GLboolean APIENTRY glIsProgramARB( GLuint program) {
    CALL_1_R(IsProgramARB, program);
}

GLAPI GLboolean APIENTRY glIsProgramNV( GLuint id) {
    CALL_1_R(IsProgramNV, id);
}

GLAPI GLboolean APIENTRY glIsProgramPipeline( GLuint pipeline) {
    CALL_1_R(IsProgramPipeline, pipeline);
}

GLAPI GLboolean APIENTRY glIsQuery( GLuint id) {
    CALL_1_R(IsQuery, id);
}

GLAPI GLboolean APIENTRY glIsQueryARB( GLuint id) {
    CALL_1_R(IsQueryARB, id);
}

GLAPI GLboolean APIENTRY glIsRenderbuffer( GLuint renderbuffer) {
    CALL_1_R(IsRenderbuffer, renderbuffer);
}

GLAPI GLboolean APIENTRY glIsRenderbufferEXT( GLuint renderbuffer) {
    CALL_1_R(IsRenderbufferEXT, renderbuffer);
}

GLAPI GLboolean APIENTRY glIsSampler( GLuint sampler) {
    CALL_1_R(IsSampler, sampler);
}

GLAPI GLboolean APIENTRY glIsShader( GLuint shader) {
    CALL_1_R(IsShader, shader);
}

GLAPI GLboolean APIENTRY glIsStateNV( GLuint state) {
    CALL_1_R(IsStateNV, state);
}

GLAPI GLboolean APIENTRY glIsSync( GLsync sync) {
    CALL_1_R(IsSync, sync);
}

GLAPI GLboolean APIENTRY glIsTextureEXT( GLuint texture) {
    CALL_1_R(IsTextureEXT, texture);
}

GLAPI GLboolean APIENTRY glIsTextureHandleResidentARB( GLuint64 handle) {
    CALL_1_R(IsTextureHandleResidentARB, LL(handle));
}

GLAPI GLboolean APIENTRY glIsTextureHandleResidentNV( GLuint64 handle) {
    CALL_1_R(IsTextureHandleResidentNV, LL(handle));
}

GLAPI GLboolean APIENTRY glIsTransformFeedback( GLuint id) {
    CALL_1_R(IsTransformFeedback, id);
}

GLAPI GLboolean APIENTRY glIsTransformFeedbackNV( GLuint id) {
    CALL_1_R(IsTransformFeedbackNV, id);
}

GLAPI GLboolean APIENTRY glIsVariantEnabledEXT( GLuint id, GLenum cap) {
    CALL_2_R(IsVariantEnabledEXT, id, cap);
}

GLAPI GLboolean APIENTRY glIsVertexArray( GLuint array) {
    CALL_1_R(IsVertexArray, array);
}

GLAPI GLboolean APIENTRY glIsVertexArrayAPPLE( GLuint array) {
    CALL_1_R(IsVertexArrayAPPLE, array);
}

GLAPI GLboolean APIENTRY glIsVertexAttribEnabledAPPLE( GLuint index, GLenum pname) {
    CALL_2_R(IsVertexAttribEnabledAPPLE, index, pname);
}

GLAPI void APIENTRY glLabelObjectEXT( GLenum type, GLuint object, GLsizei length, const GLchar* label) {
    CALL_4(LabelObjectEXT, type, object, length, label);
}

GLAPI void APIENTRY glLightEnviSGIX( GLenum pname, GLint param) {
    CALL_2(LightEnviSGIX, pname, param);
}

GLAPI void APIENTRY glLightModelxOES( GLenum pname, GLfixed param) {
    CALL_2(LightModelxOES, pname, param);
}

GLAPI void APIENTRY glLightModelxvOES( GLenum pname, const GLfixed* param) {
    CALL_2(LightModelxvOES, pname, param);
}

GLAPI void APIENTRY glLightxOES( GLenum light, GLenum pname, GLfixed param) {
    CALL_3(LightxOES, light, pname, param);
}

GLAPI void APIENTRY glLightxvOES( GLenum light, GLenum pname, const GLfixed* params) {
    CALL_3(LightxvOES, light, pname, params);
}

GLAPI void APIENTRY glLineWidthxOES( GLfixed width) {
    CALL_1(LineWidthxOES, width);
}

GLAPI void APIENTRY glLinkProgram( GLuint program) {
    CALL_1(LinkProgram, program);
}

GLAPI void APIENTRY glLinkProgramARB( GLhandleARB programObj) {
    CALL_1(LinkProgramARB, programObj);
}

GLAPI void APIENTRY glListDrawCommandsStatesClientNV( GLuint list, GLuint segment, const void** indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count) {
    CALL_7(ListDrawCommandsStatesClientNV, list, segment, indirects, sizes, states, fbos, count);
}

GLAPI void APIENTRY glListParameterfSGIX( GLuint list, GLenum pname, GLfloat param) {
    CALL_3(ListParameterfSGIX, list, pname, F(param));
}

GLAPI void APIENTRY glListParameterfvSGIX( GLuint list, GLenum pname, const GLfloat* params) {
    CALL_3(ListParameterfvSGIX, list, pname, params);
}

GLAPI void APIENTRY glListParameteriSGIX( GLuint list, GLenum pname, GLint param) {
    CALL_3(ListParameteriSGIX, list, pname, param);
}

GLAPI void APIENTRY glListParameterivSGIX( GLuint list, GLenum pname, const GLint* params) {
    CALL_3(ListParameterivSGIX, list, pname, params);
}

GLAPI void APIENTRY glLoadIdentityDeformationMapSGIX( GLbitfield mask) {
    CALL_1(LoadIdentityDeformationMapSGIX, mask);
}

GLAPI void APIENTRY glLoadMatrixxOES( const GLfixed* m) {
    CALL_1(LoadMatrixxOES, m);
}

GLAPI void APIENTRY glLoadProgramNV( GLenum target, GLuint id, GLsizei len, const GLubyte* program) {
    CALL_4(LoadProgramNV, target, id, len, program);
}

GLAPI void APIENTRY glLoadTransposeMatrixd( const GLdouble* m) {
    CALL_1(LoadTransposeMatrixd, m);
}

GLAPI void APIENTRY glLoadTransposeMatrixdARB( const GLdouble* m) {
    CALL_1(LoadTransposeMatrixdARB, m);
}

GLAPI void APIENTRY glLoadTransposeMatrixf( const GLfloat* m) {
    CALL_1(LoadTransposeMatrixf, m);
}

GLAPI void APIENTRY glLoadTransposeMatrixfARB( const GLfloat* m) {
    CALL_1(LoadTransposeMatrixfARB, m);
}

GLAPI void APIENTRY glLoadTransposeMatrixxOES( const GLfixed* m) {
    CALL_1(LoadTransposeMatrixxOES, m);
}

GLAPI void APIENTRY glLockArraysEXT( GLint first, GLsizei count) {
    CALL_2(LockArraysEXT, first, count);
}

GLAPI void APIENTRY glMTexCoord2fSGIS( GLenum target, GLfloat s, GLfloat t) {
    CALL_3(MTexCoord2fSGIS, target, F(s), F(t));
}

GLAPI void APIENTRY glMTexCoord2fvSGIS( GLenum target, GLfloat * v) {
    CALL_2(MTexCoord2fvSGIS, target, v);
}

GLAPI void APIENTRY glMakeBufferNonResidentNV( GLenum target) {
    CALL_1(MakeBufferNonResidentNV, target);
}

GLAPI void APIENTRY glMakeBufferResidentNV( GLenum target, GLenum access) {
    CALL_2(MakeBufferResidentNV, target, access);
}

GLAPI void APIENTRY glMakeImageHandleNonResidentARB( GLuint64 handle) {
    CALL_1(MakeImageHandleNonResidentARB, LL(handle));
}

GLAPI void APIENTRY glMakeImageHandleNonResidentNV( GLuint64 handle) {
    CALL_1(MakeImageHandleNonResidentNV, LL(handle));
}

GLAPI void APIENTRY glMakeImageHandleResidentARB( GLuint64 handle, GLenum access) {
    CALL_2(MakeImageHandleResidentARB, LL(handle), access);
}

GLAPI void APIENTRY glMakeImageHandleResidentNV( GLuint64 handle, GLenum access) {
    CALL_2(MakeImageHandleResidentNV, LL(handle), access);
}

GLAPI void APIENTRY glMakeNamedBufferNonResidentNV( GLuint buffer) {
    CALL_1(MakeNamedBufferNonResidentNV, buffer);
}

GLAPI void APIENTRY glMakeNamedBufferResidentNV( GLuint buffer, GLenum access) {
    CALL_2(MakeNamedBufferResidentNV, buffer, access);
}

GLAPI void APIENTRY glMakeTextureHandleNonResidentARB( GLuint64 handle) {
    CALL_1(MakeTextureHandleNonResidentARB, LL(handle));
}

GLAPI void APIENTRY glMakeTextureHandleNonResidentNV( GLuint64 handle) {
    CALL_1(MakeTextureHandleNonResidentNV, LL(handle));
}

GLAPI void APIENTRY glMakeTextureHandleResidentARB( GLuint64 handle) {
    CALL_1(MakeTextureHandleResidentARB, LL(handle));
}

GLAPI void APIENTRY glMakeTextureHandleResidentNV( GLuint64 handle) {
    CALL_1(MakeTextureHandleResidentNV, LL(handle));
}

GLAPI void APIENTRY glMap1xOES( GLenum target, GLfixed u1, GLfixed u2, GLint stride, GLint order, GLfixed points) {
    CALL_6(Map1xOES, target, u1, u2, stride, order, points);
}

GLAPI void APIENTRY glMap2xOES( GLenum target, GLfixed u1, GLfixed u2, GLint ustride, GLint uorder, GLfixed v1, GLfixed v2, GLint vstride, GLint vorder, GLfixed points) {
    CALL_10(Map2xOES, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

GLAPI void* APIENTRY glMapBuffer( GLenum target, GLenum access) {
    CALL_2_R(MapBuffer, target, access);
}

GLAPI void* APIENTRY glMapBufferARB( GLenum target, GLenum access) {
    CALL_2_R(MapBufferARB, target, access);
}

GLAPI void* APIENTRY glMapBufferRange( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
    CALL_4_R(MapBufferRange, target, offset, length, access);
}

GLAPI void APIENTRY glMapControlPointsNV( GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const void* points) {
    CALL_9(MapControlPointsNV, target, index, type, ustride, vstride, uorder, vorder, packed, points);
}

GLAPI void APIENTRY glMapGrid1xOES( GLint n, GLfixed u1, GLfixed u2) {
    CALL_3(MapGrid1xOES, n, u1, u2);
}

GLAPI void APIENTRY glMapGrid2xOES( GLint n, GLfixed u1, GLfixed u2, GLfixed v1, GLfixed v2) {
    CALL_5(MapGrid2xOES, n, u1, u2, v1, v2);
}

GLAPI void* APIENTRY glMapNamedBuffer( GLuint buffer, GLenum access) {
    CALL_2_R(MapNamedBuffer, buffer, access);
}

GLAPI void* APIENTRY glMapNamedBufferEXT( GLuint buffer, GLenum access) {
    CALL_2_R(MapNamedBufferEXT, buffer, access);
}

GLAPI void* APIENTRY glMapNamedBufferRange( GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access) {
    CALL_4_R(MapNamedBufferRange, buffer, offset, length, access);
}

GLAPI void* APIENTRY glMapNamedBufferRangeEXT( GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access) {
    CALL_4_R(MapNamedBufferRangeEXT, buffer, offset, length, access);
}

GLAPI void* APIENTRY glMapObjectBufferATI( GLuint buffer) {
    CALL_1_R(MapObjectBufferATI, buffer);
}

GLAPI void APIENTRY glMapParameterfvNV( GLenum target, GLenum pname, const GLfloat* params) {
    CALL_3(MapParameterfvNV, target, pname, params);
}

GLAPI void APIENTRY glMapParameterivNV( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(MapParameterivNV, target, pname, params);
}

GLAPI void* APIENTRY glMapTexture2DINTEL( GLuint texture, GLint level, GLbitfield access, GLint* stride, GLenum* layout) {
    CALL_5_R(MapTexture2DINTEL, texture, level, access, stride, layout);
}

GLAPI void APIENTRY glMapVertexAttrib1dAPPLE( GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points) {
    CALL_7(MapVertexAttrib1dAPPLE, index, size, D(u1), D(u2), stride, order, points);
}

GLAPI void APIENTRY glMapVertexAttrib1fAPPLE( GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points) {
    CALL_7(MapVertexAttrib1fAPPLE, index, size, F(u1), F(u2), stride, order, points);
}

GLAPI void APIENTRY glMapVertexAttrib2dAPPLE( GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points) {
    CALL_11(MapVertexAttrib2dAPPLE, index, size, D(u1), D(u2), ustride, uorder, D(v1), D(v2), vstride, vorder, points);
}

GLAPI void APIENTRY glMapVertexAttrib2fAPPLE( GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points) {
    CALL_11(MapVertexAttrib2fAPPLE, index, size, F(u1), F(u2), ustride, uorder, F(v1), F(v2), vstride, vorder, points);
}

GLAPI void APIENTRY glMaterialxOES( GLenum face, GLenum pname, GLfixed param) {
    CALL_3(MaterialxOES, face, pname, param);
}

GLAPI void APIENTRY glMaterialxvOES( GLenum face, GLenum pname, const GLfixed* param) {
    CALL_3(MaterialxvOES, face, pname, param);
}

GLAPI void APIENTRY glMatrixFrustumEXT( GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    CALL_7(MatrixFrustumEXT, mode, D(left), D(right), D(bottom), D(top), D(zNear), D(zFar));
}

GLAPI void APIENTRY glMatrixIndexPointerARB( GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_4(MatrixIndexPointerARB, size, type, stride, pointer);
}

GLAPI void APIENTRY glMatrixIndexubvARB( GLint size, const GLubyte* indices) {
    CALL_2(MatrixIndexubvARB, size, indices);
}

GLAPI void APIENTRY glMatrixIndexuivARB( GLint size, const GLuint* indices) {
    CALL_2(MatrixIndexuivARB, size, indices);
}

GLAPI void APIENTRY glMatrixIndexusvARB( GLint size, const GLushort* indices) {
    CALL_2(MatrixIndexusvARB, size, indices);
}

GLAPI void APIENTRY glMatrixLoad3x2fNV( GLenum matrixMode, const GLfloat* m) {
    CALL_2(MatrixLoad3x2fNV, matrixMode, m);
}

GLAPI void APIENTRY glMatrixLoad3x3fNV( GLenum matrixMode, const GLfloat* m) {
    CALL_2(MatrixLoad3x3fNV, matrixMode, m);
}

GLAPI void APIENTRY glMatrixLoadIdentityEXT( GLenum mode) {
    CALL_1(MatrixLoadIdentityEXT, mode);
}

GLAPI void APIENTRY glMatrixLoadTranspose3x3fNV( GLenum matrixMode, const GLfloat* m) {
    CALL_2(MatrixLoadTranspose3x3fNV, matrixMode, m);
}

GLAPI void APIENTRY glMatrixLoadTransposedEXT( GLenum mode, const GLdouble* m) {
    CALL_2(MatrixLoadTransposedEXT, mode, m);
}

GLAPI void APIENTRY glMatrixLoadTransposefEXT( GLenum mode, const GLfloat* m) {
    CALL_2(MatrixLoadTransposefEXT, mode, m);
}

GLAPI void APIENTRY glMatrixLoaddEXT( GLenum mode, const GLdouble* m) {
    CALL_2(MatrixLoaddEXT, mode, m);
}

GLAPI void APIENTRY glMatrixLoadfEXT( GLenum mode, const GLfloat* m) {
    CALL_2(MatrixLoadfEXT, mode, m);
}

GLAPI void APIENTRY glMatrixMult3x2fNV( GLenum matrixMode, const GLfloat* m) {
    CALL_2(MatrixMult3x2fNV, matrixMode, m);
}

GLAPI void APIENTRY glMatrixMult3x3fNV( GLenum matrixMode, const GLfloat* m) {
    CALL_2(MatrixMult3x3fNV, matrixMode, m);
}

GLAPI void APIENTRY glMatrixMultTranspose3x3fNV( GLenum matrixMode, const GLfloat* m) {
    CALL_2(MatrixMultTranspose3x3fNV, matrixMode, m);
}

GLAPI void APIENTRY glMatrixMultTransposedEXT( GLenum mode, const GLdouble* m) {
    CALL_2(MatrixMultTransposedEXT, mode, m);
}

GLAPI void APIENTRY glMatrixMultTransposefEXT( GLenum mode, const GLfloat* m) {
    CALL_2(MatrixMultTransposefEXT, mode, m);
}

GLAPI void APIENTRY glMatrixMultdEXT( GLenum mode, const GLdouble* m) {
    CALL_2(MatrixMultdEXT, mode, m);
}

GLAPI void APIENTRY glMatrixMultfEXT( GLenum mode, const GLfloat* m) {
    CALL_2(MatrixMultfEXT, mode, m);
}

GLAPI void APIENTRY glMatrixOrthoEXT( GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    CALL_7(MatrixOrthoEXT, mode, D(left), D(right), D(bottom), D(top), D(zNear), D(zFar));
}

GLAPI void APIENTRY glMatrixPopEXT( GLenum mode) {
    CALL_1(MatrixPopEXT, mode);
}

GLAPI void APIENTRY glMatrixPushEXT( GLenum mode) {
    CALL_1(MatrixPushEXT, mode);
}

GLAPI void APIENTRY glMatrixRotatedEXT( GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
    CALL_5(MatrixRotatedEXT, mode, D(angle), D(x), D(y), D(z));
}

GLAPI void APIENTRY glMatrixRotatefEXT( GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    CALL_5(MatrixRotatefEXT, mode, F(angle), F(x), F(y), F(z));
}

GLAPI void APIENTRY glMatrixScaledEXT( GLenum mode, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(MatrixScaledEXT, mode, D(x), D(y), D(z));
}

GLAPI void APIENTRY glMatrixScalefEXT( GLenum mode, GLfloat x, GLfloat y, GLfloat z) {
    CALL_4(MatrixScalefEXT, mode, F(x), F(y), F(z));
}

GLAPI void APIENTRY glMatrixTranslatedEXT( GLenum mode, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(MatrixTranslatedEXT, mode, D(x), D(y), D(z));
}

GLAPI void APIENTRY glMatrixTranslatefEXT( GLenum mode, GLfloat x, GLfloat y, GLfloat z) {
    CALL_4(MatrixTranslatefEXT, mode, F(x), F(y), F(z));
}

GLAPI void APIENTRY glMaxShaderCompilerThreadsARB( GLuint count) {
    CALL_1(MaxShaderCompilerThreadsARB, count);
}

GLAPI void APIENTRY glMemoryBarrier( GLbitfield barriers) {
    CALL_1(MemoryBarrier, barriers);
}

GLAPI void APIENTRY glMemoryBarrierByRegion( GLbitfield barriers) {
    CALL_1(MemoryBarrierByRegion, barriers);
}

GLAPI void APIENTRY glMemoryBarrierEXT( GLbitfield barriers) {
    CALL_1(MemoryBarrierEXT, barriers);
}

GLAPI void APIENTRY glMinSampleShading( GLfloat value) {
    CALL_1(MinSampleShading, F(value));
}

GLAPI void APIENTRY glMinSampleShadingARB( GLfloat value) {
    CALL_1(MinSampleShadingARB, F(value));
}

GLAPI void APIENTRY glMinmax( GLenum target, GLenum internalformat, GLboolean sink) {
    CALL_3(Minmax, target, internalformat, sink);
}

GLAPI void APIENTRY glMinmaxEXT( GLenum target, GLenum internalformat, GLboolean sink) {
    CALL_3(MinmaxEXT, target, internalformat, sink);
}

GLAPI void APIENTRY glMultMatrixxOES( const GLfixed* m) {
    CALL_1(MultMatrixxOES, m);
}

GLAPI void APIENTRY glMultTransposeMatrixd( const GLdouble* m) {
    CALL_1(MultTransposeMatrixd, m);
}

GLAPI void APIENTRY glMultTransposeMatrixdARB( const GLdouble* m) {
    CALL_1(MultTransposeMatrixdARB, m);
}

GLAPI void APIENTRY glMultTransposeMatrixf( const GLfloat* m) {
    CALL_1(MultTransposeMatrixf, m);
}

GLAPI void APIENTRY glMultTransposeMatrixfARB( const GLfloat* m) {
    CALL_1(MultTransposeMatrixfARB, m);
}

GLAPI void APIENTRY glMultTransposeMatrixxOES( const GLfixed* m) {
    CALL_1(MultTransposeMatrixxOES, m);
}

GLAPI void APIENTRY glMultiDrawArrays( GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount) {
    CALL_4(MultiDrawArrays, mode, first, count, drawcount);
}

GLAPI void APIENTRY glMultiDrawArraysEXT( GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount) {
    CALL_4(MultiDrawArraysEXT, mode, first, count, primcount);
}

GLAPI void APIENTRY glMultiDrawArraysIndirect( GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) {
    CALL_4(MultiDrawArraysIndirect, mode, indirect, drawcount, stride);
}

GLAPI void APIENTRY glMultiDrawArraysIndirectAMD( GLenum mode, const void* indirect, GLsizei primcount, GLsizei stride) {
    CALL_4(MultiDrawArraysIndirectAMD, mode, indirect, primcount, stride);
}

GLAPI void APIENTRY glMultiDrawArraysIndirectBindlessCountNV( GLenum mode, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount) {
    CALL_6(MultiDrawArraysIndirectBindlessCountNV, mode, indirect, drawCount, maxDrawCount, stride, vertexBufferCount);
}

GLAPI void APIENTRY glMultiDrawArraysIndirectBindlessNV( GLenum mode, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount) {
    CALL_5(MultiDrawArraysIndirectBindlessNV, mode, indirect, drawCount, stride, vertexBufferCount);
}

GLAPI void APIENTRY glMultiDrawArraysIndirectCountARB( GLenum mode, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride) {
    CALL_5(MultiDrawArraysIndirectCountARB, mode, indirect, drawcount, maxdrawcount, stride);
}

GLAPI void APIENTRY glMultiDrawElementArrayAPPLE( GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount) {
    CALL_4(MultiDrawElementArrayAPPLE, mode, first, count, primcount);
}

GLAPI void APIENTRY glMultiDrawElements( GLenum mode, const GLsizei* count, GLenum type, const void*const* indices, GLsizei drawcount) {
    CALL_5(MultiDrawElements, mode, count, type, indices, drawcount);
}

GLAPI void APIENTRY glMultiDrawElementsBaseVertex( GLenum mode, const GLsizei* count, GLenum type, const void*const* indices, GLsizei drawcount, const GLint* basevertex) {
    CALL_6(MultiDrawElementsBaseVertex, mode, count, type, indices, drawcount, basevertex);
}

GLAPI void APIENTRY glMultiDrawElementsEXT( GLenum mode, const GLsizei* count, GLenum type, const void*const* indices, GLsizei primcount) {
    CALL_5(MultiDrawElementsEXT, mode, count, type, indices, primcount);
}

GLAPI void APIENTRY glMultiDrawElementsIndirect( GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride) {
    CALL_5(MultiDrawElementsIndirect, mode, type, indirect, drawcount, stride);
}

GLAPI void APIENTRY glMultiDrawElementsIndirectAMD( GLenum mode, GLenum type, const void* indirect, GLsizei primcount, GLsizei stride) {
    CALL_5(MultiDrawElementsIndirectAMD, mode, type, indirect, primcount, stride);
}

GLAPI void APIENTRY glMultiDrawElementsIndirectBindlessCountNV( GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount) {
    CALL_7(MultiDrawElementsIndirectBindlessCountNV, mode, type, indirect, drawCount, maxDrawCount, stride, vertexBufferCount);
}

GLAPI void APIENTRY glMultiDrawElementsIndirectBindlessNV( GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount) {
    CALL_6(MultiDrawElementsIndirectBindlessNV, mode, type, indirect, drawCount, stride, vertexBufferCount);
}

GLAPI void APIENTRY glMultiDrawElementsIndirectCountARB( GLenum mode, GLenum type, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride) {
    CALL_6(MultiDrawElementsIndirectCountARB, mode, type, indirect, drawcount, maxdrawcount, stride);
}

GLAPI void APIENTRY glMultiDrawRangeElementArrayAPPLE( GLenum mode, GLuint start, GLuint end, const GLint* first, const GLsizei* count, GLsizei primcount) {
    CALL_6(MultiDrawRangeElementArrayAPPLE, mode, start, end, first, count, primcount);
}

GLAPI void APIENTRY glMultiModeDrawArraysIBM( const GLenum* mode, const GLint* first, const GLsizei* count, GLsizei primcount, GLint modestride) {
    CALL_5(MultiModeDrawArraysIBM, mode, first, count, primcount, modestride);
}

GLAPI void APIENTRY glMultiModeDrawElementsIBM( const GLenum* mode, const GLsizei* count, GLenum type, const void*const* indices, GLsizei primcount, GLint modestride) {
    CALL_6(MultiModeDrawElementsIBM, mode, count, type, indices, primcount, modestride);
}

GLAPI void APIENTRY glMultiTexBufferEXT( GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer) {
    CALL_4(MultiTexBufferEXT, texunit, target, internalformat, buffer);
}

GLAPI void APIENTRY glMultiTexCoord1bOES( GLenum texture, GLbyte s) {
    CALL_2(MultiTexCoord1bOES, texture, s);
}

GLAPI void APIENTRY glMultiTexCoord1bvOES( GLenum texture, const GLbyte* coords) {
    CALL_2(MultiTexCoord1bvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoord1d( GLenum target, GLdouble s) {
    CALL_2(MultiTexCoord1d, target, D(s));
}

GLAPI void APIENTRY glMultiTexCoord1dARB( GLenum target, GLdouble s) {
    CALL_2(MultiTexCoord1dARB, target, D(s));
}

GLAPI void APIENTRY glMultiTexCoord1dSGIS( GLenum target, GLdouble s) {
    CALL_2(MultiTexCoord1dSGIS, target, D(s));
}

GLAPI void APIENTRY glMultiTexCoord1dv( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord1dv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1dvARB( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord1dvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1dvSGIS( GLenum target, GLdouble * v) {
    CALL_2(MultiTexCoord1dvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1f( GLenum target, GLfloat s) {
    CALL_2(MultiTexCoord1f, target, F(s));
}

GLAPI void APIENTRY glMultiTexCoord1fARB( GLenum target, GLfloat s) {
    CALL_2(MultiTexCoord1fARB, target, F(s));
}

GLAPI void APIENTRY glMultiTexCoord1fSGIS( GLenum target, GLfloat s) {
    CALL_2(MultiTexCoord1fSGIS, target, F(s));
}

GLAPI void APIENTRY glMultiTexCoord1fv( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord1fv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1fvARB( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord1fvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1fvSGIS( GLenum target, const GLfloat * v) {
    CALL_2(MultiTexCoord1fvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1hNV( GLenum target, GLhalfNV s) {
    CALL_2(MultiTexCoord1hNV, target, s);
}

GLAPI void APIENTRY glMultiTexCoord1hvNV( GLenum target, const GLhalfNV* v) {
    CALL_2(MultiTexCoord1hvNV, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1i( GLenum target, GLint s) {
    CALL_2(MultiTexCoord1i, target, s);
}

GLAPI void APIENTRY glMultiTexCoord1iARB( GLenum target, GLint s) {
    CALL_2(MultiTexCoord1iARB, target, s);
}

GLAPI void APIENTRY glMultiTexCoord1iSGIS( GLenum target, GLint s) {
    CALL_2(MultiTexCoord1iSGIS, target, s);
}

GLAPI void APIENTRY glMultiTexCoord1iv( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord1iv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1ivARB( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord1ivARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1ivSGIS( GLenum target, GLint * v) {
    CALL_2(MultiTexCoord1ivSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1s( GLenum target, GLshort s) {
    CALL_2(MultiTexCoord1s, target, s);
}

GLAPI void APIENTRY glMultiTexCoord1sARB( GLenum target, GLshort s) {
    CALL_2(MultiTexCoord1sARB, target, s);
}

GLAPI void APIENTRY glMultiTexCoord1sSGIS( GLenum target, GLshort s) {
    CALL_2(MultiTexCoord1sSGIS, target, s);
}

GLAPI void APIENTRY glMultiTexCoord1sv( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord1sv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1svARB( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord1svARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1svSGIS( GLenum target, GLshort * v) {
    CALL_2(MultiTexCoord1svSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord1xOES( GLenum texture, GLfixed s) {
    CALL_2(MultiTexCoord1xOES, texture, s);
}

GLAPI void APIENTRY glMultiTexCoord1xvOES( GLenum texture, const GLfixed* coords) {
    CALL_2(MultiTexCoord1xvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoord2bOES( GLenum texture, GLbyte s, GLbyte t) {
    CALL_3(MultiTexCoord2bOES, texture, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2bvOES( GLenum texture, const GLbyte* coords) {
    CALL_2(MultiTexCoord2bvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoord2d( GLenum target, GLdouble s, GLdouble t) {
    CALL_3(MultiTexCoord2d, target, D(s), D(t));
}

GLAPI void APIENTRY glMultiTexCoord2dARB( GLenum target, GLdouble s, GLdouble t) {
    CALL_3(MultiTexCoord2dARB, target, D(s), D(t));
}

GLAPI void APIENTRY glMultiTexCoord2dSGIS( GLenum target, GLdouble s, GLdouble t) {
    CALL_3(MultiTexCoord2dSGIS, target, D(s), D(t));
}

GLAPI void APIENTRY glMultiTexCoord2dv( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord2dv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2dvARB( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord2dvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2dvSGIS( GLenum target, GLdouble * v) {
    CALL_2(MultiTexCoord2dvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2f( GLenum target, GLfloat s, GLfloat t) {
    CALL_3(MultiTexCoord2f, target, F(s), F(t));
}

GLAPI void APIENTRY glMultiTexCoord2fARB( GLenum target, GLfloat s, GLfloat t) {
    CALL_3(MultiTexCoord2fARB, target, F(s), F(t));
}

GLAPI void APIENTRY glMultiTexCoord2fSGIS( GLenum target, GLfloat s, GLfloat t) {
    CALL_3(MultiTexCoord2fSGIS, target, F(s), F(t));
}

GLAPI void APIENTRY glMultiTexCoord2fv( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord2fv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2fvARB( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord2fvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2fvSGIS( GLenum target, GLfloat * v) {
    CALL_2(MultiTexCoord2fvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2hNV( GLenum target, GLhalfNV s, GLhalfNV t) {
    CALL_3(MultiTexCoord2hNV, target, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2hvNV( GLenum target, const GLhalfNV* v) {
    CALL_2(MultiTexCoord2hvNV, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2i( GLenum target, GLint s, GLint t) {
    CALL_3(MultiTexCoord2i, target, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2iARB( GLenum target, GLint s, GLint t) {
    CALL_3(MultiTexCoord2iARB, target, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2iSGIS( GLenum target, GLint s, GLint t) {
    CALL_3(MultiTexCoord2iSGIS, target, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2iv( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord2iv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2ivARB( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord2ivARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2ivSGIS( GLenum target, GLint * v) {
    CALL_2(MultiTexCoord2ivSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2s( GLenum target, GLshort s, GLshort t) {
    CALL_3(MultiTexCoord2s, target, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2sARB( GLenum target, GLshort s, GLshort t) {
    CALL_3(MultiTexCoord2sARB, target, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2sSGIS( GLenum target, GLshort s, GLshort t) {
    CALL_3(MultiTexCoord2sSGIS, target, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2sv( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord2sv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2svARB( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord2svARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2svSGIS( GLenum target, GLshort * v) {
    CALL_2(MultiTexCoord2svSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord2xOES( GLenum texture, GLfixed s, GLfixed t) {
    CALL_3(MultiTexCoord2xOES, texture, s, t);
}

GLAPI void APIENTRY glMultiTexCoord2xvOES( GLenum texture, const GLfixed* coords) {
    CALL_2(MultiTexCoord2xvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoord3bOES( GLenum texture, GLbyte s, GLbyte t, GLbyte r) {
    CALL_4(MultiTexCoord3bOES, texture, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3bvOES( GLenum texture, const GLbyte* coords) {
    CALL_2(MultiTexCoord3bvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoord3d( GLenum target, GLdouble s, GLdouble t, GLdouble r) {
    CALL_4(MultiTexCoord3d, target, D(s), D(t), D(r));
}

GLAPI void APIENTRY glMultiTexCoord3dARB( GLenum target, GLdouble s, GLdouble t, GLdouble r) {
    CALL_4(MultiTexCoord3dARB, target, D(s), D(t), D(r));
}

GLAPI void APIENTRY glMultiTexCoord3dSGIS( GLenum target, GLdouble s, GLdouble t, GLdouble r) {
    CALL_4(MultiTexCoord3dSGIS, target, D(s), D(t), D(r));
}

GLAPI void APIENTRY glMultiTexCoord3dv( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord3dv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3dvARB( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord3dvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3dvSGIS( GLenum target, GLdouble * v) {
    CALL_2(MultiTexCoord3dvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t, GLfloat r) {
    CALL_4(MultiTexCoord3f, target, F(s), F(t), F(r));
}

GLAPI void APIENTRY glMultiTexCoord3fARB( GLenum target, GLfloat s, GLfloat t, GLfloat r) {
    CALL_4(MultiTexCoord3fARB, target, F(s), F(t), F(r));
}

GLAPI void APIENTRY glMultiTexCoord3fSGIS( GLenum target, GLfloat s, GLfloat t, GLfloat r) {
    CALL_4(MultiTexCoord3fSGIS, target, F(s), F(t), F(r));
}

GLAPI void APIENTRY glMultiTexCoord3fv( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord3fv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3fvARB( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord3fvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3fvSGIS( GLenum target, GLfloat * v) {
    CALL_2(MultiTexCoord3fvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3hNV( GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r) {
    CALL_4(MultiTexCoord3hNV, target, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3hvNV( GLenum target, const GLhalfNV* v) {
    CALL_2(MultiTexCoord3hvNV, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3i( GLenum target, GLint s, GLint t, GLint r) {
    CALL_4(MultiTexCoord3i, target, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3iARB( GLenum target, GLint s, GLint t, GLint r) {
    CALL_4(MultiTexCoord3iARB, target, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3iSGIS( GLenum target, GLint s, GLint t, GLint r) {
    CALL_4(MultiTexCoord3iSGIS, target, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3iv( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord3iv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3ivARB( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord3ivARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3ivSGIS( GLenum target, GLint * v) {
    CALL_2(MultiTexCoord3ivSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3s( GLenum target, GLshort s, GLshort t, GLshort r) {
    CALL_4(MultiTexCoord3s, target, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3sARB( GLenum target, GLshort s, GLshort t, GLshort r) {
    CALL_4(MultiTexCoord3sARB, target, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3sSGIS( GLenum target, GLshort s, GLshort t, GLshort r) {
    CALL_4(MultiTexCoord3sSGIS, target, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3sv( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord3sv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3svARB( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord3svARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3svSGIS( GLenum target, GLshort * v) {
    CALL_2(MultiTexCoord3svSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord3xOES( GLenum texture, GLfixed s, GLfixed t, GLfixed r) {
    CALL_4(MultiTexCoord3xOES, texture, s, t, r);
}

GLAPI void APIENTRY glMultiTexCoord3xvOES( GLenum texture, const GLfixed* coords) {
    CALL_2(MultiTexCoord3xvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoord4bOES( GLenum texture, GLbyte s, GLbyte t, GLbyte r, GLbyte q) {
    CALL_5(MultiTexCoord4bOES, texture, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4bvOES( GLenum texture, const GLbyte* coords) {
    CALL_2(MultiTexCoord4bvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoord4d( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    CALL_5(MultiTexCoord4d, target, D(s), D(t), D(r), D(q));
}

GLAPI void APIENTRY glMultiTexCoord4dARB( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    CALL_5(MultiTexCoord4dARB, target, D(s), D(t), D(r), D(q));
}

GLAPI void APIENTRY glMultiTexCoord4dSGIS( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    CALL_5(MultiTexCoord4dSGIS, target, D(s), D(t), D(r), D(q));
}

GLAPI void APIENTRY glMultiTexCoord4dv( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord4dv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4dvARB( GLenum target, const GLdouble* v) {
    CALL_2(MultiTexCoord4dvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4dvSGIS( GLenum target, GLdouble * v) {
    CALL_2(MultiTexCoord4dvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4f( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    CALL_5(MultiTexCoord4f, target, F(s), F(t), F(r), F(q));
}

GLAPI void APIENTRY glMultiTexCoord4fARB( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    CALL_5(MultiTexCoord4fARB, target, F(s), F(t), F(r), F(q));
}

GLAPI void APIENTRY glMultiTexCoord4fSGIS( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    CALL_5(MultiTexCoord4fSGIS, target, F(s), F(t), F(r), F(q));
}

GLAPI void APIENTRY glMultiTexCoord4fv( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord4fv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4fvARB( GLenum target, const GLfloat* v) {
    CALL_2(MultiTexCoord4fvARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4fvSGIS( GLenum target, GLfloat * v) {
    CALL_2(MultiTexCoord4fvSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4hNV( GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q) {
    CALL_5(MultiTexCoord4hNV, target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4hvNV( GLenum target, const GLhalfNV* v) {
    CALL_2(MultiTexCoord4hvNV, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4i( GLenum target, GLint s, GLint t, GLint r, GLint q) {
    CALL_5(MultiTexCoord4i, target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4iARB( GLenum target, GLint s, GLint t, GLint r, GLint q) {
    CALL_5(MultiTexCoord4iARB, target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4iSGIS( GLenum target, GLint s, GLint t, GLint r, GLint q) {
    CALL_5(MultiTexCoord4iSGIS, target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4iv( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord4iv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4ivARB( GLenum target, const GLint* v) {
    CALL_2(MultiTexCoord4ivARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4ivSGIS( GLenum target, GLint * v) {
    CALL_2(MultiTexCoord4ivSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4s( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) {
    CALL_5(MultiTexCoord4s, target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4sARB( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) {
    CALL_5(MultiTexCoord4sARB, target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4sSGIS( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) {
    CALL_5(MultiTexCoord4sSGIS, target, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4sv( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord4sv, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4svARB( GLenum target, const GLshort* v) {
    CALL_2(MultiTexCoord4svARB, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4svSGIS( GLenum target, GLshort * v) {
    CALL_2(MultiTexCoord4svSGIS, target, v);
}

GLAPI void APIENTRY glMultiTexCoord4xOES( GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q) {
    CALL_5(MultiTexCoord4xOES, texture, s, t, r, q);
}

GLAPI void APIENTRY glMultiTexCoord4xvOES( GLenum texture, const GLfixed* coords) {
    CALL_2(MultiTexCoord4xvOES, texture, coords);
}

GLAPI void APIENTRY glMultiTexCoordP1ui( GLenum texture, GLenum type, GLuint coords) {
    CALL_3(MultiTexCoordP1ui, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordP1uiv( GLenum texture, GLenum type, const GLuint* coords) {
    CALL_3(MultiTexCoordP1uiv, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordP2ui( GLenum texture, GLenum type, GLuint coords) {
    CALL_3(MultiTexCoordP2ui, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordP2uiv( GLenum texture, GLenum type, const GLuint* coords) {
    CALL_3(MultiTexCoordP2uiv, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordP3ui( GLenum texture, GLenum type, GLuint coords) {
    CALL_3(MultiTexCoordP3ui, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordP3uiv( GLenum texture, GLenum type, const GLuint* coords) {
    CALL_3(MultiTexCoordP3uiv, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordP4ui( GLenum texture, GLenum type, GLuint coords) {
    CALL_3(MultiTexCoordP4ui, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordP4uiv( GLenum texture, GLenum type, const GLuint* coords) {
    CALL_3(MultiTexCoordP4uiv, texture, type, coords);
}

GLAPI void APIENTRY glMultiTexCoordPointerEXT( GLenum texunit, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_5(MultiTexCoordPointerEXT, texunit, size, type, stride, pointer);
}

GLAPI void APIENTRY glMultiTexCoordPointerSGIS( GLenum target, GLint size, GLenum type, GLsizei stride, GLvoid * pointer) {
    CALL_5(MultiTexCoordPointerSGIS, target, size, type, stride, pointer);
}

GLAPI void APIENTRY glMultiTexEnvfEXT( GLenum texunit, GLenum target, GLenum pname, GLfloat param) {
    CALL_4(MultiTexEnvfEXT, texunit, target, pname, F(param));
}

GLAPI void APIENTRY glMultiTexEnvfvEXT( GLenum texunit, GLenum target, GLenum pname, const GLfloat* params) {
    CALL_4(MultiTexEnvfvEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glMultiTexEnviEXT( GLenum texunit, GLenum target, GLenum pname, GLint param) {
    CALL_4(MultiTexEnviEXT, texunit, target, pname, param);
}

GLAPI void APIENTRY glMultiTexEnvivEXT( GLenum texunit, GLenum target, GLenum pname, const GLint* params) {
    CALL_4(MultiTexEnvivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glMultiTexGendEXT( GLenum texunit, GLenum coord, GLenum pname, GLdouble param) {
    CALL_4(MultiTexGendEXT, texunit, coord, pname, D(param));
}

GLAPI void APIENTRY glMultiTexGendvEXT( GLenum texunit, GLenum coord, GLenum pname, const GLdouble* params) {
    CALL_4(MultiTexGendvEXT, texunit, coord, pname, params);
}

GLAPI void APIENTRY glMultiTexGenfEXT( GLenum texunit, GLenum coord, GLenum pname, GLfloat param) {
    CALL_4(MultiTexGenfEXT, texunit, coord, pname, F(param));
}

GLAPI void APIENTRY glMultiTexGenfvEXT( GLenum texunit, GLenum coord, GLenum pname, const GLfloat* params) {
    CALL_4(MultiTexGenfvEXT, texunit, coord, pname, params);
}

GLAPI void APIENTRY glMultiTexGeniEXT( GLenum texunit, GLenum coord, GLenum pname, GLint param) {
    CALL_4(MultiTexGeniEXT, texunit, coord, pname, param);
}

GLAPI void APIENTRY glMultiTexGenivEXT( GLenum texunit, GLenum coord, GLenum pname, const GLint* params) {
    CALL_4(MultiTexGenivEXT, texunit, coord, pname, params);
}

GLAPI void APIENTRY glMultiTexImage1DEXT( GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_9(MultiTexImage1DEXT, texunit, target, level, internalformat, width, border, format, type, pixels);
}

GLAPI void APIENTRY glMultiTexImage2DEXT( GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_10(MultiTexImage2DEXT, texunit, target, level, internalformat, width, height, border, format, type, pixels);
}

GLAPI void APIENTRY glMultiTexImage3DEXT( GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_11(MultiTexImage3DEXT, texunit, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

GLAPI void APIENTRY glMultiTexParameterIivEXT( GLenum texunit, GLenum target, GLenum pname, const GLint* params) {
    CALL_4(MultiTexParameterIivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glMultiTexParameterIuivEXT( GLenum texunit, GLenum target, GLenum pname, const GLuint* params) {
    CALL_4(MultiTexParameterIuivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glMultiTexParameterfEXT( GLenum texunit, GLenum target, GLenum pname, GLfloat param) {
    CALL_4(MultiTexParameterfEXT, texunit, target, pname, F(param));
}

GLAPI void APIENTRY glMultiTexParameterfvEXT( GLenum texunit, GLenum target, GLenum pname, const GLfloat* params) {
    CALL_4(MultiTexParameterfvEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glMultiTexParameteriEXT( GLenum texunit, GLenum target, GLenum pname, GLint param) {
    CALL_4(MultiTexParameteriEXT, texunit, target, pname, param);
}

GLAPI void APIENTRY glMultiTexParameterivEXT( GLenum texunit, GLenum target, GLenum pname, const GLint* params) {
    CALL_4(MultiTexParameterivEXT, texunit, target, pname, params);
}

GLAPI void APIENTRY glMultiTexRenderbufferEXT( GLenum texunit, GLenum target, GLuint renderbuffer) {
    CALL_3(MultiTexRenderbufferEXT, texunit, target, renderbuffer);
}

GLAPI void APIENTRY glMultiTexSubImage1DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    CALL_8(MultiTexSubImage1DEXT, texunit, target, level, xoffset, width, format, type, pixels);
}

GLAPI void APIENTRY glMultiTexSubImage2DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    CALL_10(MultiTexSubImage2DEXT, texunit, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLAPI void APIENTRY glMultiTexSubImage3DEXT( GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    CALL_12(MultiTexSubImage3DEXT, texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLAPI void APIENTRY glNamedBufferData( GLuint buffer, GLsizeiptr size, const void* data, GLenum usage) {
    CALL_4(NamedBufferData, buffer, size, data, usage);
}

GLAPI void APIENTRY glNamedBufferDataEXT( GLuint buffer, GLsizeiptr size, const void* data, GLenum usage) {
    CALL_4(NamedBufferDataEXT, buffer, size, data, usage);
}

GLAPI void APIENTRY glNamedBufferPageCommitmentARB( GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit) {
    CALL_4(NamedBufferPageCommitmentARB, buffer, offset, size, commit);
}

GLAPI void APIENTRY glNamedBufferPageCommitmentEXT( GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit) {
    CALL_4(NamedBufferPageCommitmentEXT, buffer, offset, size, commit);
}

GLAPI void APIENTRY glNamedBufferStorage( GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags) {
    CALL_4(NamedBufferStorage, buffer, size, data, flags);
}

GLAPI void APIENTRY glNamedBufferStorageEXT( GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags) {
    CALL_4(NamedBufferStorageEXT, buffer, size, data, flags);
}

GLAPI void APIENTRY glNamedBufferSubData( GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data) {
    CALL_4(NamedBufferSubData, buffer, offset, size, data);
}

GLAPI void APIENTRY glNamedBufferSubDataEXT( GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data) {
    CALL_4(NamedBufferSubDataEXT, buffer, offset, size, data);
}

GLAPI void APIENTRY glNamedCopyBufferSubDataEXT( GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
    CALL_5(NamedCopyBufferSubDataEXT, readBuffer, writeBuffer, readOffset, writeOffset, size);
}

GLAPI void APIENTRY glNamedFramebufferDrawBuffer( GLuint framebuffer, GLenum buf) {
    CALL_2(NamedFramebufferDrawBuffer, framebuffer, buf);
}

GLAPI void APIENTRY glNamedFramebufferDrawBuffers( GLuint framebuffer, GLsizei n, const GLenum* bufs) {
    CALL_3(NamedFramebufferDrawBuffers, framebuffer, n, bufs);
}

GLAPI void APIENTRY glNamedFramebufferParameteri( GLuint framebuffer, GLenum pname, GLint param) {
    CALL_3(NamedFramebufferParameteri, framebuffer, pname, param);
}

GLAPI void APIENTRY glNamedFramebufferParameteriEXT( GLuint framebuffer, GLenum pname, GLint param) {
    CALL_3(NamedFramebufferParameteriEXT, framebuffer, pname, param);
}

GLAPI void APIENTRY glNamedFramebufferReadBuffer( GLuint framebuffer, GLenum src) {
    CALL_2(NamedFramebufferReadBuffer, framebuffer, src);
}

GLAPI void APIENTRY glNamedFramebufferRenderbuffer( GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    CALL_4(NamedFramebufferRenderbuffer, framebuffer, attachment, renderbuffertarget, renderbuffer);
}

GLAPI void APIENTRY glNamedFramebufferRenderbufferEXT( GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    CALL_4(NamedFramebufferRenderbufferEXT, framebuffer, attachment, renderbuffertarget, renderbuffer);
}

GLAPI void APIENTRY glNamedFramebufferSampleLocationsfvARB( GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v) {
    CALL_4(NamedFramebufferSampleLocationsfvARB, framebuffer, start, count, v);
}

GLAPI void APIENTRY glNamedFramebufferSampleLocationsfvNV( GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v) {
    CALL_4(NamedFramebufferSampleLocationsfvNV, framebuffer, start, count, v);
}

GLAPI void APIENTRY glNamedFramebufferTexture( GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) {
    CALL_4(NamedFramebufferTexture, framebuffer, attachment, texture, level);
}

GLAPI void APIENTRY glNamedFramebufferTexture1DEXT( GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    CALL_5(NamedFramebufferTexture1DEXT, framebuffer, attachment, textarget, texture, level);
}

GLAPI void APIENTRY glNamedFramebufferTexture2DEXT( GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    CALL_5(NamedFramebufferTexture2DEXT, framebuffer, attachment, textarget, texture, level);
}

GLAPI void APIENTRY glNamedFramebufferTexture3DEXT( GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {
    CALL_6(NamedFramebufferTexture3DEXT, framebuffer, attachment, textarget, texture, level, zoffset);
}

GLAPI void APIENTRY glNamedFramebufferTextureEXT( GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) {
    CALL_4(NamedFramebufferTextureEXT, framebuffer, attachment, texture, level);
}

GLAPI void APIENTRY glNamedFramebufferTextureFaceEXT( GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face) {
    CALL_5(NamedFramebufferTextureFaceEXT, framebuffer, attachment, texture, level, face);
}

GLAPI void APIENTRY glNamedFramebufferTextureLayer( GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    CALL_5(NamedFramebufferTextureLayer, framebuffer, attachment, texture, level, layer);
}

GLAPI void APIENTRY glNamedFramebufferTextureLayerEXT( GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    CALL_5(NamedFramebufferTextureLayerEXT, framebuffer, attachment, texture, level, layer);
}

GLAPI void APIENTRY glNamedProgramLocalParameter4dEXT( GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_7(NamedProgramLocalParameter4dEXT, program, target, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glNamedProgramLocalParameter4dvEXT( GLuint program, GLenum target, GLuint index, const GLdouble* params) {
    CALL_4(NamedProgramLocalParameter4dvEXT, program, target, index, params);
}

GLAPI void APIENTRY glNamedProgramLocalParameter4fEXT( GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_7(NamedProgramLocalParameter4fEXT, program, target, index, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glNamedProgramLocalParameter4fvEXT( GLuint program, GLenum target, GLuint index, const GLfloat* params) {
    CALL_4(NamedProgramLocalParameter4fvEXT, program, target, index, params);
}

GLAPI void APIENTRY glNamedProgramLocalParameterI4iEXT( GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w) {
    CALL_7(NamedProgramLocalParameterI4iEXT, program, target, index, x, y, z, w);
}

GLAPI void APIENTRY glNamedProgramLocalParameterI4ivEXT( GLuint program, GLenum target, GLuint index, const GLint* params) {
    CALL_4(NamedProgramLocalParameterI4ivEXT, program, target, index, params);
}

GLAPI void APIENTRY glNamedProgramLocalParameterI4uiEXT( GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    CALL_7(NamedProgramLocalParameterI4uiEXT, program, target, index, x, y, z, w);
}

GLAPI void APIENTRY glNamedProgramLocalParameterI4uivEXT( GLuint program, GLenum target, GLuint index, const GLuint* params) {
    CALL_4(NamedProgramLocalParameterI4uivEXT, program, target, index, params);
}

GLAPI void APIENTRY glNamedProgramLocalParameters4fvEXT( GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params) {
    CALL_5(NamedProgramLocalParameters4fvEXT, program, target, index, count, params);
}

GLAPI void APIENTRY glNamedProgramLocalParametersI4ivEXT( GLuint program, GLenum target, GLuint index, GLsizei count, const GLint* params) {
    CALL_5(NamedProgramLocalParametersI4ivEXT, program, target, index, count, params);
}

GLAPI void APIENTRY glNamedProgramLocalParametersI4uivEXT( GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint* params) {
    CALL_5(NamedProgramLocalParametersI4uivEXT, program, target, index, count, params);
}

GLAPI void APIENTRY glNamedProgramStringEXT( GLuint program, GLenum target, GLenum format, GLsizei len, const void* string) {
    CALL_5(NamedProgramStringEXT, program, target, format, len, string);
}

GLAPI void APIENTRY glNamedRenderbufferStorage( GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_4(NamedRenderbufferStorage, renderbuffer, internalformat, width, height);
}

GLAPI void APIENTRY glNamedRenderbufferStorageEXT( GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_4(NamedRenderbufferStorageEXT, renderbuffer, internalformat, width, height);
}

GLAPI void APIENTRY glNamedRenderbufferStorageMultisample( GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_5(NamedRenderbufferStorageMultisample, renderbuffer, samples, internalformat, width, height);
}

GLAPI void APIENTRY glNamedRenderbufferStorageMultisampleCoverageEXT( GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_6(NamedRenderbufferStorageMultisampleCoverageEXT, renderbuffer, coverageSamples, colorSamples, internalformat, width, height);
}

GLAPI void APIENTRY glNamedRenderbufferStorageMultisampleEXT( GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_5(NamedRenderbufferStorageMultisampleEXT, renderbuffer, samples, internalformat, width, height);
}

GLAPI void APIENTRY glNamedStringARB( GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar* string) {
    CALL_5(NamedStringARB, type, namelen, name, stringlen, string);
}

GLAPI GLuint APIENTRY glNewBufferRegion( GLenum type) {
    CALL_1_R(NewBufferRegion, type);
}

GLAPI GLuint APIENTRY glNewObjectBufferATI( GLsizei size, const void* pointer, GLenum usage) {
    CALL_3_R(NewObjectBufferATI, size, pointer, usage);
}

GLAPI void APIENTRY glNormal3fVertex3fSUN( GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_6(Normal3fVertex3fSUN, F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glNormal3fVertex3fvSUN( const GLfloat* n, const GLfloat* v) {
    CALL_2(Normal3fVertex3fvSUN, n, v);
}

GLAPI void APIENTRY glNormal3hNV( GLhalfNV nx, GLhalfNV ny, GLhalfNV nz) {
    CALL_3(Normal3hNV, nx, ny, nz);
}

GLAPI void APIENTRY glNormal3hvNV( const GLhalfNV* v) {
    CALL_1(Normal3hvNV, v);
}

GLAPI void APIENTRY glNormal3xOES( GLfixed nx, GLfixed ny, GLfixed nz) {
    CALL_3(Normal3xOES, nx, ny, nz);
}

GLAPI void APIENTRY glNormal3xvOES( const GLfixed* coords) {
    CALL_1(Normal3xvOES, coords);
}

GLAPI void APIENTRY glNormalFormatNV( GLenum type, GLsizei stride) {
    CALL_2(NormalFormatNV, type, stride);
}

GLAPI void APIENTRY glNormalP3ui( GLenum type, GLuint coords) {
    CALL_2(NormalP3ui, type, coords);
}

GLAPI void APIENTRY glNormalP3uiv( GLenum type, const GLuint* coords) {
    CALL_2(NormalP3uiv, type, coords);
}

GLAPI void APIENTRY glNormalPointerEXT( GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    CALL_4(NormalPointerEXT, type, stride, count, pointer);
}

GLAPI void APIENTRY glNormalPointerListIBM( GLenum type, GLint stride, const void** pointer, GLint ptrstride) {
    CALL_4(NormalPointerListIBM, type, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glNormalPointervINTEL( GLenum type, const void** pointer) {
    CALL_2(NormalPointervINTEL, type, pointer);
}

GLAPI void APIENTRY glNormalStream3bATI( GLenum stream, GLbyte nx, GLbyte ny, GLbyte nz) {
    CALL_4(NormalStream3bATI, stream, nx, ny, nz);
}

GLAPI void APIENTRY glNormalStream3bvATI( GLenum stream, const GLbyte* coords) {
    CALL_2(NormalStream3bvATI, stream, coords);
}

GLAPI void APIENTRY glNormalStream3dATI( GLenum stream, GLdouble nx, GLdouble ny, GLdouble nz) {
    CALL_4(NormalStream3dATI, stream, D(nx), D(ny), D(nz));
}

GLAPI void APIENTRY glNormalStream3dvATI( GLenum stream, const GLdouble* coords) {
    CALL_2(NormalStream3dvATI, stream, coords);
}

GLAPI void APIENTRY glNormalStream3fATI( GLenum stream, GLfloat nx, GLfloat ny, GLfloat nz) {
    CALL_4(NormalStream3fATI, stream, F(nx), F(ny), F(nz));
}

GLAPI void APIENTRY glNormalStream3fvATI( GLenum stream, const GLfloat* coords) {
    CALL_2(NormalStream3fvATI, stream, coords);
}

GLAPI void APIENTRY glNormalStream3iATI( GLenum stream, GLint nx, GLint ny, GLint nz) {
    CALL_4(NormalStream3iATI, stream, nx, ny, nz);
}

GLAPI void APIENTRY glNormalStream3ivATI( GLenum stream, const GLint* coords) {
    CALL_2(NormalStream3ivATI, stream, coords);
}

GLAPI void APIENTRY glNormalStream3sATI( GLenum stream, GLshort nx, GLshort ny, GLshort nz) {
    CALL_4(NormalStream3sATI, stream, nx, ny, nz);
}

GLAPI void APIENTRY glNormalStream3svATI( GLenum stream, const GLshort* coords) {
    CALL_2(NormalStream3svATI, stream, coords);
}

GLAPI void APIENTRY glObjectLabel( GLenum identifier, GLuint name, GLsizei length, const GLchar* label) {
    CALL_4(ObjectLabel, identifier, name, length, label);
}

GLAPI void APIENTRY glObjectPtrLabel( const void* ptr, GLsizei length, const GLchar* label) {
    CALL_3(ObjectPtrLabel, ptr, length, label);
}

GLAPI GLenum APIENTRY glObjectPurgeableAPPLE( GLenum objectType, GLuint name, GLenum option) {
    CALL_3_R(ObjectPurgeableAPPLE, objectType, name, option);
}

GLAPI GLenum APIENTRY glObjectUnpurgeableAPPLE( GLenum objectType, GLuint name, GLenum option) {
    CALL_3_R(ObjectUnpurgeableAPPLE, objectType, name, option);
}

GLAPI void APIENTRY glOrthofOES( GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) {
    CALL_6(OrthofOES, F(l), F(r), F(b), F(t), F(n), F(f));
}

GLAPI void APIENTRY glOrthoxOES( GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f) {
    CALL_6(OrthoxOES, l, r, b, t, n, f);
}

GLAPI void APIENTRY glPNTrianglesfATI( GLenum pname, GLfloat param) {
    CALL_2(PNTrianglesfATI, pname, F(param));
}

GLAPI void APIENTRY glPNTrianglesiATI( GLenum pname, GLint param) {
    CALL_2(PNTrianglesiATI, pname, param);
}

GLAPI void APIENTRY glPassTexCoordATI( GLuint dst, GLuint coord, GLenum swizzle) {
    CALL_3(PassTexCoordATI, dst, coord, swizzle);
}

GLAPI void APIENTRY glPassThroughxOES( GLfixed token) {
    CALL_1(PassThroughxOES, token);
}

GLAPI void APIENTRY glPatchParameterfv( GLenum pname, const GLfloat* values) {
    CALL_2(PatchParameterfv, pname, values);
}

GLAPI void APIENTRY glPatchParameteri( GLenum pname, GLint value) {
    CALL_2(PatchParameteri, pname, value);
}

GLAPI void APIENTRY glPathColorGenNV( GLenum color, GLenum genMode, GLenum colorFormat, const GLfloat* coeffs) {
    CALL_4(PathColorGenNV, color, genMode, colorFormat, coeffs);
}

GLAPI void APIENTRY glPathCommandsNV( GLuint path, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords) {
    CALL_6(PathCommandsNV, path, numCommands, commands, numCoords, coordType, coords);
}

GLAPI void APIENTRY glPathCoordsNV( GLuint path, GLsizei numCoords, GLenum coordType, const void* coords) {
    CALL_4(PathCoordsNV, path, numCoords, coordType, coords);
}

GLAPI void APIENTRY glPathCoverDepthFuncNV( GLenum func) {
    CALL_1(PathCoverDepthFuncNV, func);
}

GLAPI void APIENTRY glPathDashArrayNV( GLuint path, GLsizei dashCount, const GLfloat* dashArray) {
    CALL_3(PathDashArrayNV, path, dashCount, dashArray);
}

GLAPI void APIENTRY glPathFogGenNV( GLenum genMode) {
    CALL_1(PathFogGenNV, genMode);
}

GLAPI GLenum APIENTRY glPathGlyphIndexArrayNV( GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale) {
    CALL_8_R(PathGlyphIndexArrayNV, firstPathName, fontTarget, fontName, fontStyle, firstGlyphIndex, numGlyphs, pathParameterTemplate, F(emScale));
}

GLAPI GLenum APIENTRY glPathGlyphIndexRangeNV( GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint baseAndCount[2]) {
    CALL_6_R(PathGlyphIndexRangeNV, fontTarget, fontName, fontStyle, pathParameterTemplate, F(emScale), baseAndCount[2]);
}

GLAPI void APIENTRY glPathGlyphRangeNV( GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale) {
    CALL_9(PathGlyphRangeNV, firstPathName, fontTarget, fontName, fontStyle, firstGlyph, numGlyphs, handleMissingGlyphs, pathParameterTemplate, F(emScale));
}

GLAPI void APIENTRY glPathGlyphsNV( GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void* charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale) {
    CALL_10(PathGlyphsNV, firstPathName, fontTarget, fontName, fontStyle, numGlyphs, type, charcodes, handleMissingGlyphs, pathParameterTemplate, F(emScale));
}

GLAPI GLenum APIENTRY glPathMemoryGlyphIndexArrayNV( GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void* fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale) {
    CALL_9_R(PathMemoryGlyphIndexArrayNV, firstPathName, fontTarget, fontSize, fontData, faceIndex, firstGlyphIndex, numGlyphs, pathParameterTemplate, F(emScale));
}

GLAPI void APIENTRY glPathParameterfNV( GLuint path, GLenum pname, GLfloat value) {
    CALL_3(PathParameterfNV, path, pname, F(value));
}

GLAPI void APIENTRY glPathParameterfvNV( GLuint path, GLenum pname, const GLfloat* value) {
    CALL_3(PathParameterfvNV, path, pname, value);
}

GLAPI void APIENTRY glPathParameteriNV( GLuint path, GLenum pname, GLint value) {
    CALL_3(PathParameteriNV, path, pname, value);
}

GLAPI void APIENTRY glPathParameterivNV( GLuint path, GLenum pname, const GLint* value) {
    CALL_3(PathParameterivNV, path, pname, value);
}

GLAPI void APIENTRY glPathStencilDepthOffsetNV( GLfloat factor, GLfloat units) {
    CALL_2(PathStencilDepthOffsetNV, F(factor), F(units));
}

GLAPI void APIENTRY glPathStencilFuncNV( GLenum func, GLint ref, GLuint mask) {
    CALL_3(PathStencilFuncNV, func, ref, mask);
}

GLAPI void APIENTRY glPathStringNV( GLuint path, GLenum format, GLsizei length, const void* pathString) {
    CALL_4(PathStringNV, path, format, length, pathString);
}

GLAPI void APIENTRY glPathSubCommandsNV( GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords) {
    CALL_8(PathSubCommandsNV, path, commandStart, commandsToDelete, numCommands, commands, numCoords, coordType, coords);
}

GLAPI void APIENTRY glPathSubCoordsNV( GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void* coords) {
    CALL_5(PathSubCoordsNV, path, coordStart, numCoords, coordType, coords);
}

GLAPI void APIENTRY glPathTexGenNV( GLenum texCoordSet, GLenum genMode, GLint components, const GLfloat* coeffs) {
    CALL_4(PathTexGenNV, texCoordSet, genMode, components, coeffs);
}

GLAPI void APIENTRY glPauseTransformFeedback( void ) {
    CALL_0(PauseTransformFeedback);
}

GLAPI void APIENTRY glPauseTransformFeedbackNV( void ) {
    CALL_0(PauseTransformFeedbackNV);
}

GLAPI void APIENTRY glPixelDataRangeNV( GLenum target, GLsizei length, const void* pointer) {
    CALL_3(PixelDataRangeNV, target, length, pointer);
}

GLAPI void APIENTRY glPixelMapx( GLenum map, GLint size, const GLfixed* values) {
    CALL_3(PixelMapx, map, size, values);
}

GLAPI void APIENTRY glPixelStorex( GLenum pname, GLfixed param) {
    CALL_2(PixelStorex, pname, param);
}

GLAPI void APIENTRY glPixelTexGenParameterfSGIS( GLenum pname, GLfloat param) {
    CALL_2(PixelTexGenParameterfSGIS, pname, F(param));
}

GLAPI void APIENTRY glPixelTexGenParameterfvSGIS( GLenum pname, const GLfloat* params) {
    CALL_2(PixelTexGenParameterfvSGIS, pname, params);
}

GLAPI void APIENTRY glPixelTexGenParameteriSGIS( GLenum pname, GLint param) {
    CALL_2(PixelTexGenParameteriSGIS, pname, param);
}

GLAPI void APIENTRY glPixelTexGenParameterivSGIS( GLenum pname, const GLint* params) {
    CALL_2(PixelTexGenParameterivSGIS, pname, params);
}

GLAPI void APIENTRY glPixelTexGenSGIX( GLenum mode) {
    CALL_1(PixelTexGenSGIX, mode);
}

GLAPI void APIENTRY glPixelTransferxOES( GLenum pname, GLfixed param) {
    CALL_2(PixelTransferxOES, pname, param);
}

GLAPI void APIENTRY glPixelTransformParameterfEXT( GLenum target, GLenum pname, GLfloat param) {
    CALL_3(PixelTransformParameterfEXT, target, pname, F(param));
}

GLAPI void APIENTRY glPixelTransformParameterfvEXT( GLenum target, GLenum pname, const GLfloat* params) {
    CALL_3(PixelTransformParameterfvEXT, target, pname, params);
}

GLAPI void APIENTRY glPixelTransformParameteriEXT( GLenum target, GLenum pname, GLint param) {
    CALL_3(PixelTransformParameteriEXT, target, pname, param);
}

GLAPI void APIENTRY glPixelTransformParameterivEXT( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(PixelTransformParameterivEXT, target, pname, params);
}

GLAPI void APIENTRY glPixelZoomxOES( GLfixed xfactor, GLfixed yfactor) {
    CALL_2(PixelZoomxOES, xfactor, yfactor);
}

GLAPI GLboolean APIENTRY glPointAlongPathNV( GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat* x, GLfloat* y, GLfloat* tangentX, GLfloat* tangentY) {
    CALL_8_R(PointAlongPathNV, path, startSegment, numSegments, F(distance), x, y, tangentX, tangentY);
}

GLAPI void APIENTRY glPointParameterf( GLenum pname, GLfloat param) {
    CALL_2(PointParameterf, pname, F(param));
}

GLAPI void APIENTRY glPointParameterfARB( GLenum pname, GLfloat param) {
    CALL_2(PointParameterfARB, pname, F(param));
}

GLAPI void APIENTRY glPointParameterfEXT( GLenum pname, GLfloat param) {
    CALL_2(PointParameterfEXT, pname, F(param));
}

GLAPI void APIENTRY glPointParameterfSGIS( GLenum pname, GLfloat param) {
    CALL_2(PointParameterfSGIS, pname, F(param));
}

GLAPI void APIENTRY glPointParameterfv( GLenum pname, const GLfloat* params) {
    CALL_2(PointParameterfv, pname, params);
}

GLAPI void APIENTRY glPointParameterfvARB( GLenum pname, const GLfloat* params) {
    CALL_2(PointParameterfvARB, pname, params);
}

GLAPI void APIENTRY glPointParameterfvEXT( GLenum pname, const GLfloat* params) {
    CALL_2(PointParameterfvEXT, pname, params);
}

GLAPI void APIENTRY glPointParameterfvSGIS( GLenum pname, const GLfloat* params) {
    CALL_2(PointParameterfvSGIS, pname, params);
}

GLAPI void APIENTRY glPointParameteri( GLenum pname, GLint param) {
    CALL_2(PointParameteri, pname, param);
}

GLAPI void APIENTRY glPointParameteriNV( GLenum pname, GLint param) {
    CALL_2(PointParameteriNV, pname, param);
}

GLAPI void APIENTRY glPointParameteriv( GLenum pname, const GLint* params) {
    CALL_2(PointParameteriv, pname, params);
}

GLAPI void APIENTRY glPointParameterivNV( GLenum pname, const GLint* params) {
    CALL_2(PointParameterivNV, pname, params);
}

GLAPI void APIENTRY glPointParameterxvOES( GLenum pname, const GLfixed* params) {
    CALL_2(PointParameterxvOES, pname, params);
}

GLAPI void APIENTRY glPointSizexOES( GLfixed size) {
    CALL_1(PointSizexOES, size);
}

GLAPI GLint APIENTRY glPollAsyncSGIX( GLuint* markerp) {
    CALL_1_R(PollAsyncSGIX, markerp);
}

GLAPI GLint APIENTRY glPollInstrumentsSGIX( GLint* marker_p) {
    CALL_1_R(PollInstrumentsSGIX, marker_p);
}

GLAPI void APIENTRY glPolygonOffsetClampEXT( GLfloat factor, GLfloat units, GLfloat clamp) {
    CALL_3(PolygonOffsetClampEXT, F(factor), F(units), F(clamp));
}

GLAPI void APIENTRY glPolygonOffsetEXT( GLfloat factor, GLfloat bias) {
    CALL_2(PolygonOffsetEXT, F(factor), F(bias));
}

GLAPI void APIENTRY glPolygonOffsetxOES( GLfixed factor, GLfixed units) {
    CALL_2(PolygonOffsetxOES, factor, units);
}

GLAPI void APIENTRY glPopDebugGroup( void ) {
    CALL_0(PopDebugGroup);
}

GLAPI void APIENTRY glPopGroupMarkerEXT( void ) {
    CALL_0(PopGroupMarkerEXT);
}

GLAPI void APIENTRY glPresentFrameDualFillNV( GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3) {
    CALL_13(PresentFrameDualFillNV, video_slot, LL(minPresentTime), beginPresentTimeId, presentDurationId, type, target0, fill0, target1, fill1, target2, fill2, target3, fill3);
}

GLAPI void APIENTRY glPresentFrameKeyedNV( GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1) {
    CALL_11(PresentFrameKeyedNV, video_slot, LL(minPresentTime), beginPresentTimeId, presentDurationId, type, target0, fill0, key0, target1, fill1, key1);
}

GLAPI void APIENTRY glPrimitiveBoundingBoxARB( GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) {
    CALL_8(PrimitiveBoundingBoxARB, F(minX), F(minY), F(minZ), F(minW), F(maxX), F(maxY), F(maxZ), F(maxW));
}

GLAPI void APIENTRY glPrimitiveRestartIndex( GLuint index) {
    CALL_1(PrimitiveRestartIndex, index);
}

GLAPI void APIENTRY glPrimitiveRestartIndexNV( GLuint index) {
    CALL_1(PrimitiveRestartIndexNV, index);
}

GLAPI void APIENTRY glPrimitiveRestartNV( void ) {
    CALL_0(PrimitiveRestartNV);
}

GLAPI void APIENTRY glPrioritizeTexturesEXT( GLsizei n, const GLuint* textures, const GLclampf* priorities) {
    CALL_3(PrioritizeTexturesEXT, n, textures, priorities);
}

GLAPI void APIENTRY glPrioritizeTexturesxOES( GLsizei n, const GLuint* textures, const GLfixed* priorities) {
    CALL_3(PrioritizeTexturesxOES, n, textures, priorities);
}

GLAPI void APIENTRY glProgramBinary( GLuint program, GLenum binaryFormat, const void* binary, GLsizei length) {
    CALL_4(ProgramBinary, program, binaryFormat, binary, length);
}

GLAPI void APIENTRY glProgramBufferParametersIivNV( GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLint* params) {
    CALL_5(ProgramBufferParametersIivNV, target, bindingIndex, wordIndex, count, params);
}

GLAPI void APIENTRY glProgramBufferParametersIuivNV( GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLuint* params) {
    CALL_5(ProgramBufferParametersIuivNV, target, bindingIndex, wordIndex, count, params);
}

GLAPI void APIENTRY glProgramBufferParametersfvNV( GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLfloat* params) {
    CALL_5(ProgramBufferParametersfvNV, target, bindingIndex, wordIndex, count, params);
}

GLAPI void APIENTRY glProgramEnvParameter4dARB( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_6(ProgramEnvParameter4dARB, target, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glProgramEnvParameter4dvARB( GLenum target, GLuint index, const GLdouble* params) {
    CALL_3(ProgramEnvParameter4dvARB, target, index, params);
}

GLAPI void APIENTRY glProgramEnvParameter4fARB( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_6(ProgramEnvParameter4fARB, target, index, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glProgramEnvParameter4fvARB( GLenum target, GLuint index, const GLfloat* params) {
    CALL_3(ProgramEnvParameter4fvARB, target, index, params);
}

GLAPI void APIENTRY glProgramEnvParameterI4iNV( GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w) {
    CALL_6(ProgramEnvParameterI4iNV, target, index, x, y, z, w);
}

GLAPI void APIENTRY glProgramEnvParameterI4ivNV( GLenum target, GLuint index, const GLint* params) {
    CALL_3(ProgramEnvParameterI4ivNV, target, index, params);
}

GLAPI void APIENTRY glProgramEnvParameterI4uiNV( GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    CALL_6(ProgramEnvParameterI4uiNV, target, index, x, y, z, w);
}

GLAPI void APIENTRY glProgramEnvParameterI4uivNV( GLenum target, GLuint index, const GLuint* params) {
    CALL_3(ProgramEnvParameterI4uivNV, target, index, params);
}

GLAPI void APIENTRY glProgramEnvParameters4fvEXT( GLenum target, GLuint index, GLsizei count, const GLfloat* params) {
    CALL_4(ProgramEnvParameters4fvEXT, target, index, count, params);
}

GLAPI void APIENTRY glProgramEnvParametersI4ivNV( GLenum target, GLuint index, GLsizei count, const GLint* params) {
    CALL_4(ProgramEnvParametersI4ivNV, target, index, count, params);
}

GLAPI void APIENTRY glProgramEnvParametersI4uivNV( GLenum target, GLuint index, GLsizei count, const GLuint* params) {
    CALL_4(ProgramEnvParametersI4uivNV, target, index, count, params);
}

GLAPI void APIENTRY glProgramLocalParameter4dARB( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_6(ProgramLocalParameter4dARB, target, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glProgramLocalParameter4dvARB( GLenum target, GLuint index, const GLdouble* params) {
    CALL_3(ProgramLocalParameter4dvARB, target, index, params);
}

GLAPI void APIENTRY glProgramLocalParameter4fARB( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_6(ProgramLocalParameter4fARB, target, index, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glProgramLocalParameter4fvARB( GLenum target, GLuint index, const GLfloat* params) {
    CALL_3(ProgramLocalParameter4fvARB, target, index, params);
}

GLAPI void APIENTRY glProgramLocalParameterI4iNV( GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w) {
    CALL_6(ProgramLocalParameterI4iNV, target, index, x, y, z, w);
}

GLAPI void APIENTRY glProgramLocalParameterI4ivNV( GLenum target, GLuint index, const GLint* params) {
    CALL_3(ProgramLocalParameterI4ivNV, target, index, params);
}

GLAPI void APIENTRY glProgramLocalParameterI4uiNV( GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    CALL_6(ProgramLocalParameterI4uiNV, target, index, x, y, z, w);
}

GLAPI void APIENTRY glProgramLocalParameterI4uivNV( GLenum target, GLuint index, const GLuint* params) {
    CALL_3(ProgramLocalParameterI4uivNV, target, index, params);
}

GLAPI void APIENTRY glProgramLocalParameters4fvEXT( GLenum target, GLuint index, GLsizei count, const GLfloat* params) {
    CALL_4(ProgramLocalParameters4fvEXT, target, index, count, params);
}

GLAPI void APIENTRY glProgramLocalParametersI4ivNV( GLenum target, GLuint index, GLsizei count, const GLint* params) {
    CALL_4(ProgramLocalParametersI4ivNV, target, index, count, params);
}

GLAPI void APIENTRY glProgramLocalParametersI4uivNV( GLenum target, GLuint index, GLsizei count, const GLuint* params) {
    CALL_4(ProgramLocalParametersI4uivNV, target, index, count, params);
}

GLAPI void APIENTRY glProgramNamedParameter4dNV( GLuint id, GLsizei len, const GLubyte* name, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_7(ProgramNamedParameter4dNV, id, len, name, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glProgramNamedParameter4dvNV( GLuint id, GLsizei len, const GLubyte* name, const GLdouble* v) {
    CALL_4(ProgramNamedParameter4dvNV, id, len, name, v);
}

GLAPI void APIENTRY glProgramNamedParameter4fNV( GLuint id, GLsizei len, const GLubyte* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_7(ProgramNamedParameter4fNV, id, len, name, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glProgramNamedParameter4fvNV( GLuint id, GLsizei len, const GLubyte* name, const GLfloat* v) {
    CALL_4(ProgramNamedParameter4fvNV, id, len, name, v);
}

GLAPI void APIENTRY glProgramParameter4dNV( GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_6(ProgramParameter4dNV, target, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glProgramParameter4dvNV( GLenum target, GLuint index, const GLdouble* v) {
    CALL_3(ProgramParameter4dvNV, target, index, v);
}

GLAPI void APIENTRY glProgramParameter4fNV( GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_6(ProgramParameter4fNV, target, index, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glProgramParameter4fvNV( GLenum target, GLuint index, const GLfloat* v) {
    CALL_3(ProgramParameter4fvNV, target, index, v);
}

GLAPI void APIENTRY glProgramParameteri( GLuint program, GLenum pname, GLint value) {
    CALL_3(ProgramParameteri, program, pname, value);
}

GLAPI void APIENTRY glProgramParameteriARB( GLuint program, GLenum pname, GLint value) {
    CALL_3(ProgramParameteriARB, program, pname, value);
}

GLAPI void APIENTRY glProgramParameteriEXT( GLuint program, GLenum pname, GLint value) {
    CALL_3(ProgramParameteriEXT, program, pname, value);
}

GLAPI void APIENTRY glProgramParameters4dvNV( GLenum target, GLuint index, GLsizei count, const GLdouble* v) {
    CALL_4(ProgramParameters4dvNV, target, index, count, v);
}

GLAPI void APIENTRY glProgramParameters4fvNV( GLenum target, GLuint index, GLsizei count, const GLfloat* v) {
    CALL_4(ProgramParameters4fvNV, target, index, count, v);
}

GLAPI void APIENTRY glProgramPathFragmentInputGenNV( GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat* coeffs) {
    CALL_5(ProgramPathFragmentInputGenNV, program, location, genMode, components, coeffs);
}

GLAPI void APIENTRY glProgramStringARB( GLenum target, GLenum format, GLsizei len, const void* string) {
    CALL_4(ProgramStringARB, target, format, len, string);
}

GLAPI void APIENTRY glProgramSubroutineParametersuivNV( GLenum target, GLsizei count, const GLuint* params) {
    CALL_3(ProgramSubroutineParametersuivNV, target, count, params);
}

GLAPI void APIENTRY glProgramUniform1d( GLuint program, GLint location, GLdouble v0) {
    CALL_3(ProgramUniform1d, program, location, D(v0));
}

GLAPI void APIENTRY glProgramUniform1dEXT( GLuint program, GLint location, GLdouble x) {
    CALL_3(ProgramUniform1dEXT, program, location, D(x));
}

GLAPI void APIENTRY glProgramUniform1dv( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform1dv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1dvEXT( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform1dvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1f( GLuint program, GLint location, GLfloat v0) {
    CALL_3(ProgramUniform1f, program, location, F(v0));
}

GLAPI void APIENTRY glProgramUniform1fEXT( GLuint program, GLint location, GLfloat v0) {
    CALL_3(ProgramUniform1fEXT, program, location, F(v0));
}

GLAPI void APIENTRY glProgramUniform1fv( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform1fv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1fvEXT( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform1fvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1i( GLuint program, GLint location, GLint v0) {
    CALL_3(ProgramUniform1i, program, location, v0);
}

GLAPI void APIENTRY glProgramUniform1i64ARB( GLuint program, GLint location, GLint64 x) {
    CALL_3(ProgramUniform1i64ARB, program, location, LL(x));
}

GLAPI void APIENTRY glProgramUniform1i64NV( GLuint program, GLint location, GLint64EXT x) {
    CALL_3(ProgramUniform1i64NV, program, location, LL(x));
}

GLAPI void APIENTRY glProgramUniform1i64vARB( GLuint program, GLint location, GLsizei count, const GLint64* value) {
    CALL_4(ProgramUniform1i64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1i64vNV( GLuint program, GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_4(ProgramUniform1i64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1iEXT( GLuint program, GLint location, GLint v0) {
    CALL_3(ProgramUniform1iEXT, program, location, v0);
}

GLAPI void APIENTRY glProgramUniform1iv( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform1iv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1ivEXT( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform1ivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1ui( GLuint program, GLint location, GLuint v0) {
    CALL_3(ProgramUniform1ui, program, location, v0);
}

GLAPI void APIENTRY glProgramUniform1ui64ARB( GLuint program, GLint location, GLuint64 x) {
    CALL_3(ProgramUniform1ui64ARB, program, location, LL(x));
}

GLAPI void APIENTRY glProgramUniform1ui64NV( GLuint program, GLint location, GLuint64EXT x) {
    CALL_3(ProgramUniform1ui64NV, program, location, LL(x));
}

GLAPI void APIENTRY glProgramUniform1ui64vARB( GLuint program, GLint location, GLsizei count, const GLuint64* value) {
    CALL_4(ProgramUniform1ui64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1ui64vNV( GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_4(ProgramUniform1ui64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1uiEXT( GLuint program, GLint location, GLuint v0) {
    CALL_3(ProgramUniform1uiEXT, program, location, v0);
}

GLAPI void APIENTRY glProgramUniform1uiv( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform1uiv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform1uivEXT( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform1uivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2d( GLuint program, GLint location, GLdouble v0, GLdouble v1) {
    CALL_4(ProgramUniform2d, program, location, D(v0), D(v1));
}

GLAPI void APIENTRY glProgramUniform2dEXT( GLuint program, GLint location, GLdouble x, GLdouble y) {
    CALL_4(ProgramUniform2dEXT, program, location, D(x), D(y));
}

GLAPI void APIENTRY glProgramUniform2dv( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform2dv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2dvEXT( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform2dvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2f( GLuint program, GLint location, GLfloat v0, GLfloat v1) {
    CALL_4(ProgramUniform2f, program, location, F(v0), F(v1));
}

GLAPI void APIENTRY glProgramUniform2fEXT( GLuint program, GLint location, GLfloat v0, GLfloat v1) {
    CALL_4(ProgramUniform2fEXT, program, location, F(v0), F(v1));
}

GLAPI void APIENTRY glProgramUniform2fv( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform2fv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2fvEXT( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform2fvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2i( GLuint program, GLint location, GLint v0, GLint v1) {
    CALL_4(ProgramUniform2i, program, location, v0, v1);
}

GLAPI void APIENTRY glProgramUniform2i64ARB( GLuint program, GLint location, GLint64 x, GLint64 y) {
    CALL_4(ProgramUniform2i64ARB, program, location, LL(x), LL(y));
}

GLAPI void APIENTRY glProgramUniform2i64NV( GLuint program, GLint location, GLint64EXT x, GLint64EXT y) {
    CALL_4(ProgramUniform2i64NV, program, location, LL(x), LL(y));
}

GLAPI void APIENTRY glProgramUniform2i64vARB( GLuint program, GLint location, GLsizei count, const GLint64* value) {
    CALL_4(ProgramUniform2i64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2i64vNV( GLuint program, GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_4(ProgramUniform2i64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2iEXT( GLuint program, GLint location, GLint v0, GLint v1) {
    CALL_4(ProgramUniform2iEXT, program, location, v0, v1);
}

GLAPI void APIENTRY glProgramUniform2iv( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform2iv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2ivEXT( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform2ivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2ui( GLuint program, GLint location, GLuint v0, GLuint v1) {
    CALL_4(ProgramUniform2ui, program, location, v0, v1);
}

GLAPI void APIENTRY glProgramUniform2ui64ARB( GLuint program, GLint location, GLuint64 x, GLuint64 y) {
    CALL_4(ProgramUniform2ui64ARB, program, location, LL(x), LL(y));
}

GLAPI void APIENTRY glProgramUniform2ui64NV( GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y) {
    CALL_4(ProgramUniform2ui64NV, program, location, LL(x), LL(y));
}

GLAPI void APIENTRY glProgramUniform2ui64vARB( GLuint program, GLint location, GLsizei count, const GLuint64* value) {
    CALL_4(ProgramUniform2ui64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2ui64vNV( GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_4(ProgramUniform2ui64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2uiEXT( GLuint program, GLint location, GLuint v0, GLuint v1) {
    CALL_4(ProgramUniform2uiEXT, program, location, v0, v1);
}

GLAPI void APIENTRY glProgramUniform2uiv( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform2uiv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform2uivEXT( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform2uivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3d( GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2) {
    CALL_5(ProgramUniform3d, program, location, D(v0), D(v1), D(v2));
}

GLAPI void APIENTRY glProgramUniform3dEXT( GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z) {
    CALL_5(ProgramUniform3dEXT, program, location, D(x), D(y), D(z));
}

GLAPI void APIENTRY glProgramUniform3dv( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform3dv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3dvEXT( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform3dvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3f( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    CALL_5(ProgramUniform3f, program, location, F(v0), F(v1), F(v2));
}

GLAPI void APIENTRY glProgramUniform3fEXT( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    CALL_5(ProgramUniform3fEXT, program, location, F(v0), F(v1), F(v2));
}

GLAPI void APIENTRY glProgramUniform3fv( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform3fv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3fvEXT( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform3fvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3i( GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {
    CALL_5(ProgramUniform3i, program, location, v0, v1, v2);
}

GLAPI void APIENTRY glProgramUniform3i64ARB( GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z) {
    CALL_5(ProgramUniform3i64ARB, program, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glProgramUniform3i64NV( GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z) {
    CALL_5(ProgramUniform3i64NV, program, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glProgramUniform3i64vARB( GLuint program, GLint location, GLsizei count, const GLint64* value) {
    CALL_4(ProgramUniform3i64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3i64vNV( GLuint program, GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_4(ProgramUniform3i64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3iEXT( GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {
    CALL_5(ProgramUniform3iEXT, program, location, v0, v1, v2);
}

GLAPI void APIENTRY glProgramUniform3iv( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform3iv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3ivEXT( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform3ivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3ui( GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) {
    CALL_5(ProgramUniform3ui, program, location, v0, v1, v2);
}

GLAPI void APIENTRY glProgramUniform3ui64ARB( GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z) {
    CALL_5(ProgramUniform3ui64ARB, program, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glProgramUniform3ui64NV( GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z) {
    CALL_5(ProgramUniform3ui64NV, program, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glProgramUniform3ui64vARB( GLuint program, GLint location, GLsizei count, const GLuint64* value) {
    CALL_4(ProgramUniform3ui64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3ui64vNV( GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_4(ProgramUniform3ui64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3uiEXT( GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) {
    CALL_5(ProgramUniform3uiEXT, program, location, v0, v1, v2);
}

GLAPI void APIENTRY glProgramUniform3uiv( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform3uiv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform3uivEXT( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform3uivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4d( GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3) {
    CALL_6(ProgramUniform4d, program, location, D(v0), D(v1), D(v2), D(v3));
}

GLAPI void APIENTRY glProgramUniform4dEXT( GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_6(ProgramUniform4dEXT, program, location, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glProgramUniform4dv( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform4dv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4dvEXT( GLuint program, GLint location, GLsizei count, const GLdouble* value) {
    CALL_4(ProgramUniform4dvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4f( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    CALL_6(ProgramUniform4f, program, location, F(v0), F(v1), F(v2), F(v3));
}

GLAPI void APIENTRY glProgramUniform4fEXT( GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    CALL_6(ProgramUniform4fEXT, program, location, F(v0), F(v1), F(v2), F(v3));
}

GLAPI void APIENTRY glProgramUniform4fv( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform4fv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4fvEXT( GLuint program, GLint location, GLsizei count, const GLfloat* value) {
    CALL_4(ProgramUniform4fvEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4i( GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    CALL_6(ProgramUniform4i, program, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glProgramUniform4i64ARB( GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w) {
    CALL_6(ProgramUniform4i64ARB, program, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glProgramUniform4i64NV( GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w) {
    CALL_6(ProgramUniform4i64NV, program, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glProgramUniform4i64vARB( GLuint program, GLint location, GLsizei count, const GLint64* value) {
    CALL_4(ProgramUniform4i64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4i64vNV( GLuint program, GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_4(ProgramUniform4i64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4iEXT( GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    CALL_6(ProgramUniform4iEXT, program, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glProgramUniform4iv( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform4iv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4ivEXT( GLuint program, GLint location, GLsizei count, const GLint* value) {
    CALL_4(ProgramUniform4ivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4ui( GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    CALL_6(ProgramUniform4ui, program, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glProgramUniform4ui64ARB( GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w) {
    CALL_6(ProgramUniform4ui64ARB, program, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glProgramUniform4ui64NV( GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w) {
    CALL_6(ProgramUniform4ui64NV, program, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glProgramUniform4ui64vARB( GLuint program, GLint location, GLsizei count, const GLuint64* value) {
    CALL_4(ProgramUniform4ui64vARB, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4ui64vNV( GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_4(ProgramUniform4ui64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4uiEXT( GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    CALL_6(ProgramUniform4uiEXT, program, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glProgramUniform4uiv( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform4uiv, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniform4uivEXT( GLuint program, GLint location, GLsizei count, const GLuint* value) {
    CALL_4(ProgramUniform4uivEXT, program, location, count, value);
}

GLAPI void APIENTRY glProgramUniformHandleui64ARB( GLuint program, GLint location, GLuint64 value) {
    CALL_3(ProgramUniformHandleui64ARB, program, location, LL(value));
}

GLAPI void APIENTRY glProgramUniformHandleui64NV( GLuint program, GLint location, GLuint64 value) {
    CALL_3(ProgramUniformHandleui64NV, program, location, LL(value));
}

GLAPI void APIENTRY glProgramUniformHandleui64vARB( GLuint program, GLint location, GLsizei count, const GLuint64* values) {
    CALL_4(ProgramUniformHandleui64vARB, program, location, count, values);
}

GLAPI void APIENTRY glProgramUniformHandleui64vNV( GLuint program, GLint location, GLsizei count, const GLuint64* values) {
    CALL_4(ProgramUniformHandleui64vNV, program, location, count, values);
}

GLAPI void APIENTRY glProgramUniformMatrix2dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix2dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix2dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix2fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix2fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x3dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix2x3dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x3dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix2x3dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x3fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix2x3fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x3fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix2x3fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x4dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix2x4dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x4dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix2x4dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x4fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix2x4fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix2x4fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix2x4fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix3dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix3dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix3fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix3fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x2dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix3x2dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x2dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix3x2dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x2fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix3x2fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x2fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix3x2fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x4dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix3x4dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x4dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix3x4dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x4fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix3x4fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix3x4fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix3x4fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix4dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix4dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix4fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix4fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x2dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix4x2dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x2dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix4x2dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x2fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix4x2fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x2fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix4x2fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x3dv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix4x3dv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x3dvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_5(ProgramUniformMatrix4x3dvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x3fv( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix4x3fv, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformMatrix4x3fvEXT( GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_5(ProgramUniformMatrix4x3fvEXT, program, location, count, transpose, value);
}

GLAPI void APIENTRY glProgramUniformui64NV( GLuint program, GLint location, GLuint64EXT value) {
    CALL_3(ProgramUniformui64NV, program, location, LL(value));
}

GLAPI void APIENTRY glProgramUniformui64vNV( GLuint program, GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_4(ProgramUniformui64vNV, program, location, count, value);
}

GLAPI void APIENTRY glProgramVertexLimitNV( GLenum target, GLint limit) {
    CALL_2(ProgramVertexLimitNV, target, limit);
}

GLAPI void APIENTRY glProvokingVertex( GLenum mode) {
    CALL_1(ProvokingVertex, mode);
}

GLAPI void APIENTRY glProvokingVertexEXT( GLenum mode) {
    CALL_1(ProvokingVertexEXT, mode);
}

GLAPI void APIENTRY glPushClientAttribDefaultEXT( GLbitfield mask) {
    CALL_1(PushClientAttribDefaultEXT, mask);
}

GLAPI void APIENTRY glPushDebugGroup( GLenum source, GLuint id, GLsizei length, const GLchar* message) {
    CALL_4(PushDebugGroup, source, id, length, message);
}

GLAPI void APIENTRY glPushGroupMarkerEXT( GLsizei length, const GLchar* marker) {
    CALL_2(PushGroupMarkerEXT, length, marker);
}

GLAPI void APIENTRY glQueryCounter( GLuint id, GLenum target) {
    CALL_2(QueryCounter, id, target);
}

GLAPI GLbitfield APIENTRY glQueryMatrixxOES( GLfixed* mantissa, GLint* exponent) {
    CALL_2_R(QueryMatrixxOES, mantissa, exponent);
}

GLAPI void APIENTRY glQueryObjectParameteruiAMD( GLenum target, GLuint id, GLenum pname, GLuint param) {
    CALL_4(QueryObjectParameteruiAMD, target, id, pname, param);
}

GLAPI void APIENTRY glRasterPos2xOES( GLfixed x, GLfixed y) {
    CALL_2(RasterPos2xOES, x, y);
}

GLAPI void APIENTRY glRasterPos2xvOES( const GLfixed* coords) {
    CALL_1(RasterPos2xvOES, coords);
}

GLAPI void APIENTRY glRasterPos3xOES( GLfixed x, GLfixed y, GLfixed z) {
    CALL_3(RasterPos3xOES, x, y, z);
}

GLAPI void APIENTRY glRasterPos3xvOES( const GLfixed* coords) {
    CALL_1(RasterPos3xvOES, coords);
}

GLAPI void APIENTRY glRasterPos4xOES( GLfixed x, GLfixed y, GLfixed z, GLfixed w) {
    CALL_4(RasterPos4xOES, x, y, z, w);
}

GLAPI void APIENTRY glRasterPos4xvOES( const GLfixed* coords) {
    CALL_1(RasterPos4xvOES, coords);
}

GLAPI void APIENTRY glRasterSamplesEXT( GLuint samples, GLboolean fixedsamplelocations) {
    CALL_2(RasterSamplesEXT, samples, fixedsamplelocations);
}

GLAPI void APIENTRY glReadBufferRegion( GLenum region, GLint x, GLint y, GLsizei width, GLsizei height) {
    CALL_5(ReadBufferRegion, region, x, y, width, height);
}

GLAPI void APIENTRY glReadInstrumentsSGIX( GLint marker) {
    CALL_1(ReadInstrumentsSGIX, marker);
}

GLAPI void APIENTRY glReadnPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data) {
    CALL_8(ReadnPixels, x, y, width, height, format, type, bufSize, data);
}

GLAPI void APIENTRY glReadnPixelsARB( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data) {
    CALL_8(ReadnPixelsARB, x, y, width, height, format, type, bufSize, data);
}

GLAPI void APIENTRY glRectxOES( GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2) {
    CALL_4(RectxOES, x1, y1, x2, y2);
}

GLAPI void APIENTRY glRectxvOES( const GLfixed* v1, const GLfixed* v2) {
    CALL_2(RectxvOES, v1, v2);
}

GLAPI void APIENTRY glReferencePlaneSGIX( const GLdouble* equation) {
    CALL_1(ReferencePlaneSGIX, equation);
}

GLAPI void APIENTRY glReleaseShaderCompiler( void ) {
    CALL_0(ReleaseShaderCompiler);
}

GLAPI void APIENTRY glRenderbufferStorage( GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_4(RenderbufferStorage, target, internalformat, width, height);
}

GLAPI void APIENTRY glRenderbufferStorageEXT( GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_4(RenderbufferStorageEXT, target, internalformat, width, height);
}

GLAPI void APIENTRY glRenderbufferStorageMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_5(RenderbufferStorageMultisample, target, samples, internalformat, width, height);
}

GLAPI void APIENTRY glRenderbufferStorageMultisampleCoverageNV( GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_6(RenderbufferStorageMultisampleCoverageNV, target, coverageSamples, colorSamples, internalformat, width, height);
}

GLAPI void APIENTRY glRenderbufferStorageMultisampleEXT( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_5(RenderbufferStorageMultisampleEXT, target, samples, internalformat, width, height);
}

GLAPI void APIENTRY glReplacementCodePointerSUN( GLenum type, GLsizei stride, const void** pointer) {
    CALL_3(ReplacementCodePointerSUN, type, stride, pointer);
}

GLAPI void APIENTRY glReplacementCodeubSUN( GLubyte code) {
    CALL_1(ReplacementCodeubSUN, code);
}

GLAPI void APIENTRY glReplacementCodeubvSUN( const GLubyte* code) {
    CALL_1(ReplacementCodeubvSUN, code);
}

GLAPI void APIENTRY glReplacementCodeuiColor3fVertex3fSUN( GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z) {
    CALL_7(ReplacementCodeuiColor3fVertex3fSUN, rc, F(r), F(g), F(b), F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiColor3fVertex3fvSUN( const GLuint* rc, const GLfloat* c, const GLfloat* v) {
    CALL_3(ReplacementCodeuiColor3fVertex3fvSUN, rc, c, v);
}

GLAPI void APIENTRY glReplacementCodeuiColor4fNormal3fVertex3fSUN( GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_11(ReplacementCodeuiColor4fNormal3fVertex3fSUN, rc, F(r), F(g), F(b), F(a), F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiColor4fNormal3fVertex3fvSUN( const GLuint* rc, const GLfloat* c, const GLfloat* n, const GLfloat* v) {
    CALL_4(ReplacementCodeuiColor4fNormal3fVertex3fvSUN, rc, c, n, v);
}

GLAPI void APIENTRY glReplacementCodeuiColor4ubVertex3fSUN( GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z) {
    CALL_8(ReplacementCodeuiColor4ubVertex3fSUN, rc, r, g, b, a, F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiColor4ubVertex3fvSUN( const GLuint* rc, const GLubyte* c, const GLfloat* v) {
    CALL_3(ReplacementCodeuiColor4ubVertex3fvSUN, rc, c, v);
}

GLAPI void APIENTRY glReplacementCodeuiNormal3fVertex3fSUN( GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_7(ReplacementCodeuiNormal3fVertex3fSUN, rc, F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiNormal3fVertex3fvSUN( const GLuint* rc, const GLfloat* n, const GLfloat* v) {
    CALL_3(ReplacementCodeuiNormal3fVertex3fvSUN, rc, n, v);
}

GLAPI void APIENTRY glReplacementCodeuiSUN( GLuint code) {
    CALL_1(ReplacementCodeuiSUN, code);
}

GLAPI void APIENTRY glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN( GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_13(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN, rc, F(s), F(t), F(r), F(g), F(b), F(a), F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN( const GLuint* rc, const GLfloat* tc, const GLfloat* c, const GLfloat* n, const GLfloat* v) {
    CALL_5(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN, rc, tc, c, n, v);
}

GLAPI void APIENTRY glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN( GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_9(ReplacementCodeuiTexCoord2fNormal3fVertex3fSUN, rc, F(s), F(t), F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN( const GLuint* rc, const GLfloat* tc, const GLfloat* n, const GLfloat* v) {
    CALL_4(ReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN, rc, tc, n, v);
}

GLAPI void APIENTRY glReplacementCodeuiTexCoord2fVertex3fSUN( GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z) {
    CALL_6(ReplacementCodeuiTexCoord2fVertex3fSUN, rc, F(s), F(t), F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiTexCoord2fVertex3fvSUN( const GLuint* rc, const GLfloat* tc, const GLfloat* v) {
    CALL_3(ReplacementCodeuiTexCoord2fVertex3fvSUN, rc, tc, v);
}

GLAPI void APIENTRY glReplacementCodeuiVertex3fSUN( GLuint rc, GLfloat x, GLfloat y, GLfloat z) {
    CALL_4(ReplacementCodeuiVertex3fSUN, rc, F(x), F(y), F(z));
}

GLAPI void APIENTRY glReplacementCodeuiVertex3fvSUN( const GLuint* rc, const GLfloat* v) {
    CALL_2(ReplacementCodeuiVertex3fvSUN, rc, v);
}

GLAPI void APIENTRY glReplacementCodeuivSUN( const GLuint* code) {
    CALL_1(ReplacementCodeuivSUN, code);
}

GLAPI void APIENTRY glReplacementCodeusSUN( GLushort code) {
    CALL_1(ReplacementCodeusSUN, code);
}

GLAPI void APIENTRY glReplacementCodeusvSUN( const GLushort* code) {
    CALL_1(ReplacementCodeusvSUN, code);
}

GLAPI void APIENTRY glRequestResidentProgramsNV( GLsizei n, const GLuint* programs) {
    CALL_2(RequestResidentProgramsNV, n, programs);
}

GLAPI void APIENTRY glResetHistogram( GLenum target) {
    CALL_1(ResetHistogram, target);
}

GLAPI void APIENTRY glResetHistogramEXT( GLenum target) {
    CALL_1(ResetHistogramEXT, target);
}

GLAPI void APIENTRY glResetMinmax( GLenum target) {
    CALL_1(ResetMinmax, target);
}

GLAPI void APIENTRY glResetMinmaxEXT( GLenum target) {
    CALL_1(ResetMinmaxEXT, target);
}

GLAPI void APIENTRY glResizeBuffersMESA( void ) {
    CALL_0(ResizeBuffersMESA);
}

GLAPI void APIENTRY glResolveDepthValuesNV( void ) {
    CALL_0(ResolveDepthValuesNV);
}

GLAPI void APIENTRY glResumeTransformFeedback( void ) {
    CALL_0(ResumeTransformFeedback);
}

GLAPI void APIENTRY glResumeTransformFeedbackNV( void ) {
    CALL_0(ResumeTransformFeedbackNV);
}

GLAPI void APIENTRY glRotatexOES( GLfixed angle, GLfixed x, GLfixed y, GLfixed z) {
    CALL_4(RotatexOES, angle, x, y, z);
}

GLAPI void APIENTRY glSampleCoverage( GLfloat value, GLboolean invert) {
    CALL_2(SampleCoverage, F(value), invert);
}

GLAPI void APIENTRY glSampleCoverageARB( GLfloat value, GLboolean invert) {
    CALL_2(SampleCoverageARB, F(value), invert);
}

GLAPI void APIENTRY glSampleMapATI( GLuint dst, GLuint interp, GLenum swizzle) {
    CALL_3(SampleMapATI, dst, interp, swizzle);
}

GLAPI void APIENTRY glSampleMaskEXT( GLclampf value, GLboolean invert) {
    CALL_2(SampleMaskEXT, F(value), invert);
}

GLAPI void APIENTRY glSampleMaskIndexedNV( GLuint index, GLbitfield mask) {
    CALL_2(SampleMaskIndexedNV, index, mask);
}

GLAPI void APIENTRY glSampleMaskSGIS( GLclampf value, GLboolean invert) {
    CALL_2(SampleMaskSGIS, F(value), invert);
}

GLAPI void APIENTRY glSampleMaski( GLuint maskNumber, GLbitfield mask) {
    CALL_2(SampleMaski, maskNumber, mask);
}

GLAPI void APIENTRY glSamplePatternEXT( GLenum pattern) {
    CALL_1(SamplePatternEXT, pattern);
}

GLAPI void APIENTRY glSamplePatternSGIS( GLenum pattern) {
    CALL_1(SamplePatternSGIS, pattern);
}

GLAPI void APIENTRY glSamplerParameterIiv( GLuint sampler, GLenum pname, const GLint* param) {
    CALL_3(SamplerParameterIiv, sampler, pname, param);
}

GLAPI void APIENTRY glSamplerParameterIuiv( GLuint sampler, GLenum pname, const GLuint* param) {
    CALL_3(SamplerParameterIuiv, sampler, pname, param);
}

GLAPI void APIENTRY glSamplerParameterf( GLuint sampler, GLenum pname, GLfloat param) {
    CALL_3(SamplerParameterf, sampler, pname, F(param));
}

GLAPI void APIENTRY glSamplerParameterfv( GLuint sampler, GLenum pname, const GLfloat* param) {
    CALL_3(SamplerParameterfv, sampler, pname, param);
}

GLAPI void APIENTRY glSamplerParameteri( GLuint sampler, GLenum pname, GLint param) {
    CALL_3(SamplerParameteri, sampler, pname, param);
}

GLAPI void APIENTRY glSamplerParameteriv( GLuint sampler, GLenum pname, const GLint* param) {
    CALL_3(SamplerParameteriv, sampler, pname, param);
}

GLAPI void APIENTRY glScalexOES( GLfixed x, GLfixed y, GLfixed z) {
    CALL_3(ScalexOES, x, y, z);
}

GLAPI void APIENTRY glScissorArrayv( GLuint first, GLsizei count, const GLint* v) {
    CALL_3(ScissorArrayv, first, count, v);
}

GLAPI void APIENTRY glScissorIndexed( GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height) {
    CALL_5(ScissorIndexed, index, left, bottom, width, height);
}

GLAPI void APIENTRY glScissorIndexedv( GLuint index, const GLint* v) {
    CALL_2(ScissorIndexedv, index, v);
}

GLAPI void APIENTRY glSecondaryColor3b( GLbyte red, GLbyte green, GLbyte blue) {
    CALL_3(SecondaryColor3b, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3bEXT( GLbyte red, GLbyte green, GLbyte blue) {
    CALL_3(SecondaryColor3bEXT, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3bv( const GLbyte* v) {
    CALL_1(SecondaryColor3bv, v);
}

GLAPI void APIENTRY glSecondaryColor3bvEXT( const GLbyte* v) {
    CALL_1(SecondaryColor3bvEXT, v);
}

GLAPI void APIENTRY glSecondaryColor3d( GLdouble red, GLdouble green, GLdouble blue) {
    CALL_3(SecondaryColor3d, D(red), D(green), D(blue));
}

GLAPI void APIENTRY glSecondaryColor3dEXT( GLdouble red, GLdouble green, GLdouble blue) {
    CALL_3(SecondaryColor3dEXT, D(red), D(green), D(blue));
}

GLAPI void APIENTRY glSecondaryColor3dv( const GLdouble* v) {
    CALL_1(SecondaryColor3dv, v);
}

GLAPI void APIENTRY glSecondaryColor3dvEXT( const GLdouble* v) {
    CALL_1(SecondaryColor3dvEXT, v);
}

GLAPI void APIENTRY glSecondaryColor3f( GLfloat red, GLfloat green, GLfloat blue) {
    CALL_3(SecondaryColor3f, F(red), F(green), F(blue));
}

GLAPI void APIENTRY glSecondaryColor3fEXT( GLfloat red, GLfloat green, GLfloat blue) {
    CALL_3(SecondaryColor3fEXT, F(red), F(green), F(blue));
}

GLAPI void APIENTRY glSecondaryColor3fv( const GLfloat* v) {
    CALL_1(SecondaryColor3fv, v);
}

GLAPI void APIENTRY glSecondaryColor3fvEXT( const GLfloat* v) {
    CALL_1(SecondaryColor3fvEXT, v);
}

GLAPI void APIENTRY glSecondaryColor3hNV( GLhalfNV red, GLhalfNV green, GLhalfNV blue) {
    CALL_3(SecondaryColor3hNV, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3hvNV( const GLhalfNV* v) {
    CALL_1(SecondaryColor3hvNV, v);
}

GLAPI void APIENTRY glSecondaryColor3i( GLint red, GLint green, GLint blue) {
    CALL_3(SecondaryColor3i, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3iEXT( GLint red, GLint green, GLint blue) {
    CALL_3(SecondaryColor3iEXT, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3iv( const GLint* v) {
    CALL_1(SecondaryColor3iv, v);
}

GLAPI void APIENTRY glSecondaryColor3ivEXT( const GLint* v) {
    CALL_1(SecondaryColor3ivEXT, v);
}

GLAPI void APIENTRY glSecondaryColor3s( GLshort red, GLshort green, GLshort blue) {
    CALL_3(SecondaryColor3s, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3sEXT( GLshort red, GLshort green, GLshort blue) {
    CALL_3(SecondaryColor3sEXT, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3sv( const GLshort* v) {
    CALL_1(SecondaryColor3sv, v);
}

GLAPI void APIENTRY glSecondaryColor3svEXT( const GLshort* v) {
    CALL_1(SecondaryColor3svEXT, v);
}

GLAPI void APIENTRY glSecondaryColor3ub( GLubyte red, GLubyte green, GLubyte blue) {
    CALL_3(SecondaryColor3ub, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3ubEXT( GLubyte red, GLubyte green, GLubyte blue) {
    CALL_3(SecondaryColor3ubEXT, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3ubv( const GLubyte* v) {
    CALL_1(SecondaryColor3ubv, v);
}

GLAPI void APIENTRY glSecondaryColor3ubvEXT( const GLubyte* v) {
    CALL_1(SecondaryColor3ubvEXT, v);
}

GLAPI void APIENTRY glSecondaryColor3ui( GLuint red, GLuint green, GLuint blue) {
    CALL_3(SecondaryColor3ui, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3uiEXT( GLuint red, GLuint green, GLuint blue) {
    CALL_3(SecondaryColor3uiEXT, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3uiv( const GLuint* v) {
    CALL_1(SecondaryColor3uiv, v);
}

GLAPI void APIENTRY glSecondaryColor3uivEXT( const GLuint* v) {
    CALL_1(SecondaryColor3uivEXT, v);
}

GLAPI void APIENTRY glSecondaryColor3us( GLushort red, GLushort green, GLushort blue) {
    CALL_3(SecondaryColor3us, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3usEXT( GLushort red, GLushort green, GLushort blue) {
    CALL_3(SecondaryColor3usEXT, red, green, blue);
}

GLAPI void APIENTRY glSecondaryColor3usv( const GLushort* v) {
    CALL_1(SecondaryColor3usv, v);
}

GLAPI void APIENTRY glSecondaryColor3usvEXT( const GLushort* v) {
    CALL_1(SecondaryColor3usvEXT, v);
}

GLAPI void APIENTRY glSecondaryColorFormatNV( GLint size, GLenum type, GLsizei stride) {
    CALL_3(SecondaryColorFormatNV, size, type, stride);
}

GLAPI void APIENTRY glSecondaryColorP3ui( GLenum type, GLuint color) {
    CALL_2(SecondaryColorP3ui, type, color);
}

GLAPI void APIENTRY glSecondaryColorP3uiv( GLenum type, const GLuint* color) {
    CALL_2(SecondaryColorP3uiv, type, color);
}

GLAPI void APIENTRY glSecondaryColorPointer( GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_4(SecondaryColorPointer, size, type, stride, pointer);
}

GLAPI void APIENTRY glSecondaryColorPointerEXT( GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_4(SecondaryColorPointerEXT, size, type, stride, pointer);
}

GLAPI void APIENTRY glSecondaryColorPointerListIBM( GLint size, GLenum type, GLint stride, const void** pointer, GLint ptrstride) {
    CALL_5(SecondaryColorPointerListIBM, size, type, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glSelectPerfMonitorCountersAMD( GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* counterList) {
    CALL_5(SelectPerfMonitorCountersAMD, monitor, enable, group, numCounters, counterList);
}

GLAPI void APIENTRY glSelectTextureCoordSetSGIS( GLenum target) {
    CALL_1(SelectTextureCoordSetSGIS, target);
}

GLAPI void APIENTRY glSelectTextureSGIS( GLenum target) {
    CALL_1(SelectTextureSGIS, target);
}

GLAPI void APIENTRY glSeparableFilter2D( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* row, const void* column) {
    CALL_8(SeparableFilter2D, target, internalformat, width, height, format, type, row, column);
}

GLAPI void APIENTRY glSeparableFilter2DEXT( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* row, const void* column) {
    CALL_8(SeparableFilter2DEXT, target, internalformat, width, height, format, type, row, column);
}

GLAPI void APIENTRY glSetFenceAPPLE( GLuint fence) {
    CALL_1(SetFenceAPPLE, fence);
}

GLAPI void APIENTRY glSetFenceNV( GLuint fence, GLenum condition) {
    CALL_2(SetFenceNV, fence, condition);
}

GLAPI void APIENTRY glSetFragmentShaderConstantATI( GLuint dst, const GLfloat* value) {
    CALL_2(SetFragmentShaderConstantATI, dst, value);
}

GLAPI void APIENTRY glSetInvariantEXT( GLuint id, GLenum type, const void* addr) {
    CALL_3(SetInvariantEXT, id, type, addr);
}

GLAPI void APIENTRY glSetLocalConstantEXT( GLuint id, GLenum type, const void* addr) {
    CALL_3(SetLocalConstantEXT, id, type, addr);
}

GLAPI void APIENTRY glSetMultisamplefvAMD( GLenum pname, GLuint index, const GLfloat* val) {
    CALL_3(SetMultisamplefvAMD, pname, index, val);
}

GLAPI void APIENTRY glShaderBinary( GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length) {
    CALL_5(ShaderBinary, count, shaders, binaryformat, binary, length);
}

GLAPI void APIENTRY glShaderOp1EXT( GLenum op, GLuint res, GLuint arg1) {
    CALL_3(ShaderOp1EXT, op, res, arg1);
}

GLAPI void APIENTRY glShaderOp2EXT( GLenum op, GLuint res, GLuint arg1, GLuint arg2) {
    CALL_4(ShaderOp2EXT, op, res, arg1, arg2);
}

GLAPI void APIENTRY glShaderOp3EXT( GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3) {
    CALL_5(ShaderOp3EXT, op, res, arg1, arg2, arg3);
}

GLAPI void APIENTRY glShaderSource( GLuint shader, GLsizei count, const GLchar*const* string, const GLint* length) {
    CALL_4(ShaderSource, shader, count, string, length);
}

GLAPI void APIENTRY glShaderSourceARB( GLhandleARB shaderObj, GLsizei count, const GLcharARB** string, const GLint* length) {
    CALL_4(ShaderSourceARB, shaderObj, count, string, length);
}

GLAPI void APIENTRY glShaderStorageBlockBinding( GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding) {
    CALL_3(ShaderStorageBlockBinding, program, storageBlockIndex, storageBlockBinding);
}

GLAPI void APIENTRY glSharpenTexFuncSGIS( GLenum target, GLsizei n, const GLfloat* points) {
    CALL_3(SharpenTexFuncSGIS, target, n, points);
}

GLAPI void APIENTRY glSpriteParameterfSGIX( GLenum pname, GLfloat param) {
    CALL_2(SpriteParameterfSGIX, pname, F(param));
}

GLAPI void APIENTRY glSpriteParameterfvSGIX( GLenum pname, const GLfloat* params) {
    CALL_2(SpriteParameterfvSGIX, pname, params);
}

GLAPI void APIENTRY glSpriteParameteriSGIX( GLenum pname, GLint param) {
    CALL_2(SpriteParameteriSGIX, pname, param);
}

GLAPI void APIENTRY glSpriteParameterivSGIX( GLenum pname, const GLint* params) {
    CALL_2(SpriteParameterivSGIX, pname, params);
}

GLAPI void APIENTRY glStartInstrumentsSGIX( void ) {
    CALL_0(StartInstrumentsSGIX);
}

GLAPI void APIENTRY glStateCaptureNV( GLuint state, GLenum mode) {
    CALL_2(StateCaptureNV, state, mode);
}

GLAPI void APIENTRY glStencilClearTagEXT( GLsizei stencilTagBits, GLuint stencilClearTag) {
    CALL_2(StencilClearTagEXT, stencilTagBits, stencilClearTag);
}

GLAPI void APIENTRY glStencilFillPathInstancedNV( GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat* transformValues) {
    CALL_8(StencilFillPathInstancedNV, numPaths, pathNameType, paths, pathBase, fillMode, mask, transformType, transformValues);
}

GLAPI void APIENTRY glStencilFillPathNV( GLuint path, GLenum fillMode, GLuint mask) {
    CALL_3(StencilFillPathNV, path, fillMode, mask);
}

GLAPI void APIENTRY glStencilFuncSeparate( GLenum face, GLenum func, GLint ref, GLuint mask) {
    CALL_4(StencilFuncSeparate, face, func, ref, mask);
}

GLAPI void APIENTRY glStencilFuncSeparateATI( GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask) {
    CALL_4(StencilFuncSeparateATI, frontfunc, backfunc, ref, mask);
}

GLAPI void APIENTRY glStencilMaskSeparate( GLenum face, GLuint mask) {
    CALL_2(StencilMaskSeparate, face, mask);
}

GLAPI void APIENTRY glStencilOpSeparate( GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    CALL_4(StencilOpSeparate, face, sfail, dpfail, dppass);
}

GLAPI void APIENTRY glStencilOpSeparateATI( GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    CALL_4(StencilOpSeparateATI, face, sfail, dpfail, dppass);
}

GLAPI void APIENTRY glStencilOpValueAMD( GLenum face, GLuint value) {
    CALL_2(StencilOpValueAMD, face, value);
}

GLAPI void APIENTRY glStencilStrokePathInstancedNV( GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat* transformValues) {
    CALL_8(StencilStrokePathInstancedNV, numPaths, pathNameType, paths, pathBase, reference, mask, transformType, transformValues);
}

GLAPI void APIENTRY glStencilStrokePathNV( GLuint path, GLint reference, GLuint mask) {
    CALL_3(StencilStrokePathNV, path, reference, mask);
}

GLAPI void APIENTRY glStencilThenCoverFillPathInstancedNV( GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) {
    CALL_9(StencilThenCoverFillPathInstancedNV, numPaths, pathNameType, paths, pathBase, fillMode, mask, coverMode, transformType, transformValues);
}

GLAPI void APIENTRY glStencilThenCoverFillPathNV( GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode) {
    CALL_4(StencilThenCoverFillPathNV, path, fillMode, mask, coverMode);
}

GLAPI void APIENTRY glStencilThenCoverStrokePathInstancedNV( GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues) {
    CALL_9(StencilThenCoverStrokePathInstancedNV, numPaths, pathNameType, paths, pathBase, reference, mask, coverMode, transformType, transformValues);
}

GLAPI void APIENTRY glStencilThenCoverStrokePathNV( GLuint path, GLint reference, GLuint mask, GLenum coverMode) {
    CALL_4(StencilThenCoverStrokePathNV, path, reference, mask, coverMode);
}

GLAPI void APIENTRY glStopInstrumentsSGIX( GLint marker) {
    CALL_1(StopInstrumentsSGIX, marker);
}

GLAPI void APIENTRY glStringMarkerGREMEDY( GLsizei len, const void* string) {
    CALL_2(StringMarkerGREMEDY, len, string);
}

GLAPI void APIENTRY glSubpixelPrecisionBiasNV( GLuint xbits, GLuint ybits) {
    CALL_2(SubpixelPrecisionBiasNV, xbits, ybits);
}

GLAPI void APIENTRY glSwizzleEXT( GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW) {
    CALL_6(SwizzleEXT, res, in, outX, outY, outZ, outW);
}

GLAPI void APIENTRY glSyncTextureINTEL( GLuint texture) {
    CALL_1(SyncTextureINTEL, texture);
}

GLAPI void APIENTRY glTagSampleBufferSGIX( void ) {
    CALL_0(TagSampleBufferSGIX);
}

GLAPI void APIENTRY glTangent3bEXT( GLbyte tx, GLbyte ty, GLbyte tz) {
    CALL_3(Tangent3bEXT, tx, ty, tz);
}

GLAPI void APIENTRY glTangent3bvEXT( const GLbyte* v) {
    CALL_1(Tangent3bvEXT, v);
}

GLAPI void APIENTRY glTangent3dEXT( GLdouble tx, GLdouble ty, GLdouble tz) {
    CALL_3(Tangent3dEXT, D(tx), D(ty), D(tz));
}

GLAPI void APIENTRY glTangent3dvEXT( const GLdouble* v) {
    CALL_1(Tangent3dvEXT, v);
}

GLAPI void APIENTRY glTangent3fEXT( GLfloat tx, GLfloat ty, GLfloat tz) {
    CALL_3(Tangent3fEXT, F(tx), F(ty), F(tz));
}

GLAPI void APIENTRY glTangent3fvEXT( const GLfloat* v) {
    CALL_1(Tangent3fvEXT, v);
}

GLAPI void APIENTRY glTangent3iEXT( GLint tx, GLint ty, GLint tz) {
    CALL_3(Tangent3iEXT, tx, ty, tz);
}

GLAPI void APIENTRY glTangent3ivEXT( const GLint* v) {
    CALL_1(Tangent3ivEXT, v);
}

GLAPI void APIENTRY glTangent3sEXT( GLshort tx, GLshort ty, GLshort tz) {
    CALL_3(Tangent3sEXT, tx, ty, tz);
}

GLAPI void APIENTRY glTangent3svEXT( const GLshort* v) {
    CALL_1(Tangent3svEXT, v);
}

GLAPI void APIENTRY glTangentPointerEXT( GLenum type, GLsizei stride, const void* pointer) {
    CALL_3(TangentPointerEXT, type, stride, pointer);
}

GLAPI void APIENTRY glTbufferMask3DFX( GLuint mask) {
    CALL_1(TbufferMask3DFX, mask);
}

GLAPI void APIENTRY glTessellationFactorAMD( GLfloat factor) {
    CALL_1(TessellationFactorAMD, F(factor));
}

GLAPI void APIENTRY glTessellationModeAMD( GLenum mode) {
    CALL_1(TessellationModeAMD, mode);
}

GLAPI GLboolean APIENTRY glTestFenceAPPLE( GLuint fence) {
    CALL_1_R(TestFenceAPPLE, fence);
}

GLAPI GLboolean APIENTRY glTestFenceNV( GLuint fence) {
    CALL_1_R(TestFenceNV, fence);
}

GLAPI GLboolean APIENTRY glTestObjectAPPLE( GLenum object, GLuint name) {
    CALL_2_R(TestObjectAPPLE, object, name);
}

GLAPI void APIENTRY glTexBuffer( GLenum target, GLenum internalformat, GLuint buffer) {
    CALL_3(TexBuffer, target, internalformat, buffer);
}

GLAPI void APIENTRY glTexBufferARB( GLenum target, GLenum internalformat, GLuint buffer) {
    CALL_3(TexBufferARB, target, internalformat, buffer);
}

GLAPI void APIENTRY glTexBufferEXT( GLenum target, GLenum internalformat, GLuint buffer) {
    CALL_3(TexBufferEXT, target, internalformat, buffer);
}

GLAPI void APIENTRY glTexBufferRange( GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    CALL_5(TexBufferRange, target, internalformat, buffer, offset, size);
}

GLAPI void APIENTRY glTexBumpParameterfvATI( GLenum pname, const GLfloat* param) {
    CALL_2(TexBumpParameterfvATI, pname, param);
}

GLAPI void APIENTRY glTexBumpParameterivATI( GLenum pname, const GLint* param) {
    CALL_2(TexBumpParameterivATI, pname, param);
}

GLAPI void APIENTRY glTexCoord1bOES( GLbyte s) {
    CALL_1(TexCoord1bOES, s);
}

GLAPI void APIENTRY glTexCoord1bvOES( const GLbyte* coords) {
    CALL_1(TexCoord1bvOES, coords);
}

GLAPI void APIENTRY glTexCoord1hNV( GLhalfNV s) {
    CALL_1(TexCoord1hNV, s);
}

GLAPI void APIENTRY glTexCoord1hvNV( const GLhalfNV* v) {
    CALL_1(TexCoord1hvNV, v);
}

GLAPI void APIENTRY glTexCoord1xOES( GLfixed s) {
    CALL_1(TexCoord1xOES, s);
}

GLAPI void APIENTRY glTexCoord1xvOES( const GLfixed* coords) {
    CALL_1(TexCoord1xvOES, coords);
}

GLAPI void APIENTRY glTexCoord2bOES( GLbyte s, GLbyte t) {
    CALL_2(TexCoord2bOES, s, t);
}

GLAPI void APIENTRY glTexCoord2bvOES( const GLbyte* coords) {
    CALL_1(TexCoord2bvOES, coords);
}

GLAPI void APIENTRY glTexCoord2fColor3fVertex3fSUN( GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z) {
    CALL_8(TexCoord2fColor3fVertex3fSUN, F(s), F(t), F(r), F(g), F(b), F(x), F(y), F(z));
}

GLAPI void APIENTRY glTexCoord2fColor3fVertex3fvSUN( const GLfloat* tc, const GLfloat* c, const GLfloat* v) {
    CALL_3(TexCoord2fColor3fVertex3fvSUN, tc, c, v);
}

GLAPI void APIENTRY glTexCoord2fColor4fNormal3fVertex3fSUN( GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_12(TexCoord2fColor4fNormal3fVertex3fSUN, F(s), F(t), F(r), F(g), F(b), F(a), F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glTexCoord2fColor4fNormal3fVertex3fvSUN( const GLfloat* tc, const GLfloat* c, const GLfloat* n, const GLfloat* v) {
    CALL_4(TexCoord2fColor4fNormal3fVertex3fvSUN, tc, c, n, v);
}

GLAPI void APIENTRY glTexCoord2fColor4ubVertex3fSUN( GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z) {
    CALL_9(TexCoord2fColor4ubVertex3fSUN, F(s), F(t), r, g, b, a, F(x), F(y), F(z));
}

GLAPI void APIENTRY glTexCoord2fColor4ubVertex3fvSUN( const GLfloat* tc, const GLubyte* c, const GLfloat* v) {
    CALL_3(TexCoord2fColor4ubVertex3fvSUN, tc, c, v);
}

GLAPI void APIENTRY glTexCoord2fNormal3fVertex3fSUN( GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z) {
    CALL_8(TexCoord2fNormal3fVertex3fSUN, F(s), F(t), F(nx), F(ny), F(nz), F(x), F(y), F(z));
}

GLAPI void APIENTRY glTexCoord2fNormal3fVertex3fvSUN( const GLfloat* tc, const GLfloat* n, const GLfloat* v) {
    CALL_3(TexCoord2fNormal3fVertex3fvSUN, tc, n, v);
}

GLAPI void APIENTRY glTexCoord2fVertex3fSUN( GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z) {
    CALL_5(TexCoord2fVertex3fSUN, F(s), F(t), F(x), F(y), F(z));
}

GLAPI void APIENTRY glTexCoord2fVertex3fvSUN( const GLfloat* tc, const GLfloat* v) {
    CALL_2(TexCoord2fVertex3fvSUN, tc, v);
}

GLAPI void APIENTRY glTexCoord2hNV( GLhalfNV s, GLhalfNV t) {
    CALL_2(TexCoord2hNV, s, t);
}

GLAPI void APIENTRY glTexCoord2hvNV( const GLhalfNV* v) {
    CALL_1(TexCoord2hvNV, v);
}

GLAPI void APIENTRY glTexCoord2xOES( GLfixed s, GLfixed t) {
    CALL_2(TexCoord2xOES, s, t);
}

GLAPI void APIENTRY glTexCoord2xvOES( const GLfixed* coords) {
    CALL_1(TexCoord2xvOES, coords);
}

GLAPI void APIENTRY glTexCoord3bOES( GLbyte s, GLbyte t, GLbyte r) {
    CALL_3(TexCoord3bOES, s, t, r);
}

GLAPI void APIENTRY glTexCoord3bvOES( const GLbyte* coords) {
    CALL_1(TexCoord3bvOES, coords);
}

GLAPI void APIENTRY glTexCoord3hNV( GLhalfNV s, GLhalfNV t, GLhalfNV r) {
    CALL_3(TexCoord3hNV, s, t, r);
}

GLAPI void APIENTRY glTexCoord3hvNV( const GLhalfNV* v) {
    CALL_1(TexCoord3hvNV, v);
}

GLAPI void APIENTRY glTexCoord3xOES( GLfixed s, GLfixed t, GLfixed r) {
    CALL_3(TexCoord3xOES, s, t, r);
}

GLAPI void APIENTRY glTexCoord3xvOES( const GLfixed* coords) {
    CALL_1(TexCoord3xvOES, coords);
}

GLAPI void APIENTRY glTexCoord4bOES( GLbyte s, GLbyte t, GLbyte r, GLbyte q) {
    CALL_4(TexCoord4bOES, s, t, r, q);
}

GLAPI void APIENTRY glTexCoord4bvOES( const GLbyte* coords) {
    CALL_1(TexCoord4bvOES, coords);
}

GLAPI void APIENTRY glTexCoord4fColor4fNormal3fVertex4fSUN( GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_15(TexCoord4fColor4fNormal3fVertex4fSUN, F(s), F(t), F(p), F(q), F(r), F(g), F(b), F(a), F(nx), F(ny), F(nz), F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glTexCoord4fColor4fNormal3fVertex4fvSUN( const GLfloat* tc, const GLfloat* c, const GLfloat* n, const GLfloat* v) {
    CALL_4(TexCoord4fColor4fNormal3fVertex4fvSUN, tc, c, n, v);
}

GLAPI void APIENTRY glTexCoord4fVertex4fSUN( GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_8(TexCoord4fVertex4fSUN, F(s), F(t), F(p), F(q), F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glTexCoord4fVertex4fvSUN( const GLfloat* tc, const GLfloat* v) {
    CALL_2(TexCoord4fVertex4fvSUN, tc, v);
}

GLAPI void APIENTRY glTexCoord4hNV( GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q) {
    CALL_4(TexCoord4hNV, s, t, r, q);
}

GLAPI void APIENTRY glTexCoord4hvNV( const GLhalfNV* v) {
    CALL_1(TexCoord4hvNV, v);
}

GLAPI void APIENTRY glTexCoord4xOES( GLfixed s, GLfixed t, GLfixed r, GLfixed q) {
    CALL_4(TexCoord4xOES, s, t, r, q);
}

GLAPI void APIENTRY glTexCoord4xvOES( const GLfixed* coords) {
    CALL_1(TexCoord4xvOES, coords);
}

GLAPI void APIENTRY glTexCoordFormatNV( GLint size, GLenum type, GLsizei stride) {
    CALL_3(TexCoordFormatNV, size, type, stride);
}

GLAPI void APIENTRY glTexCoordP1ui( GLenum type, GLuint coords) {
    CALL_2(TexCoordP1ui, type, coords);
}

GLAPI void APIENTRY glTexCoordP1uiv( GLenum type, const GLuint* coords) {
    CALL_2(TexCoordP1uiv, type, coords);
}

GLAPI void APIENTRY glTexCoordP2ui( GLenum type, GLuint coords) {
    CALL_2(TexCoordP2ui, type, coords);
}

GLAPI void APIENTRY glTexCoordP2uiv( GLenum type, const GLuint* coords) {
    CALL_2(TexCoordP2uiv, type, coords);
}

GLAPI void APIENTRY glTexCoordP3ui( GLenum type, GLuint coords) {
    CALL_2(TexCoordP3ui, type, coords);
}

GLAPI void APIENTRY glTexCoordP3uiv( GLenum type, const GLuint* coords) {
    CALL_2(TexCoordP3uiv, type, coords);
}

GLAPI void APIENTRY glTexCoordP4ui( GLenum type, GLuint coords) {
    CALL_2(TexCoordP4ui, type, coords);
}

GLAPI void APIENTRY glTexCoordP4uiv( GLenum type, const GLuint* coords) {
    CALL_2(TexCoordP4uiv, type, coords);
}

GLAPI void APIENTRY glTexCoordPointerEXT( GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    CALL_5(TexCoordPointerEXT, size, type, stride, count, pointer);
}

GLAPI void APIENTRY glTexCoordPointerListIBM( GLint size, GLenum type, GLint stride, const void** pointer, GLint ptrstride) {
    CALL_5(TexCoordPointerListIBM, size, type, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glTexCoordPointervINTEL( GLint size, GLenum type, const void** pointer) {
    CALL_3(TexCoordPointervINTEL, size, type, pointer);
}

GLAPI void APIENTRY glTexEnvxOES( GLenum target, GLenum pname, GLfixed param) {
    CALL_3(TexEnvxOES, target, pname, param);
}

GLAPI void APIENTRY glTexEnvxvOES( GLenum target, GLenum pname, const GLfixed* params) {
    CALL_3(TexEnvxvOES, target, pname, params);
}

GLAPI void APIENTRY glTexFilterFuncSGIS( GLenum target, GLenum filter, GLsizei n, const GLfloat* weights) {
    CALL_4(TexFilterFuncSGIS, target, filter, n, weights);
}

GLAPI void APIENTRY glTexGenxOES( GLenum coord, GLenum pname, GLfixed param) {
    CALL_3(TexGenxOES, coord, pname, param);
}

GLAPI void APIENTRY glTexGenxvOES( GLenum coord, GLenum pname, const GLfixed* params) {
    CALL_3(TexGenxvOES, coord, pname, params);
}

GLAPI void APIENTRY glTexImage2DMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
    CALL_6(TexImage2DMultisample, target, samples, internalformat, width, height, fixedsamplelocations);
}

GLAPI void APIENTRY glTexImage2DMultisampleCoverageNV( GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations) {
    CALL_7(TexImage2DMultisampleCoverageNV, target, coverageSamples, colorSamples, internalFormat, width, height, fixedSampleLocations);
}

GLAPI void APIENTRY glTexImage3D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_10(TexImage3D, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

GLAPI void APIENTRY glTexImage3DEXT( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_10(TexImage3DEXT, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

GLAPI void APIENTRY glTexImage3DMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
    CALL_7(TexImage3DMultisample, target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

GLAPI void APIENTRY glTexImage3DMultisampleCoverageNV( GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations) {
    CALL_8(TexImage3DMultisampleCoverageNV, target, coverageSamples, colorSamples, internalFormat, width, height, depth, fixedSampleLocations);
}

GLAPI void APIENTRY glTexImage4DSGIS( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_11(TexImage4DSGIS, target, level, internalformat, width, height, depth, size4d, border, format, type, pixels);
}

GLAPI void APIENTRY glTexPageCommitmentARB( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) {
    CALL_9(TexPageCommitmentARB, target, level, xoffset, yoffset, zoffset, width, height, depth, commit);
}

GLAPI void APIENTRY glTexParameterIiv( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(TexParameterIiv, target, pname, params);
}

GLAPI void APIENTRY glTexParameterIivEXT( GLenum target, GLenum pname, const GLint* params) {
    CALL_3(TexParameterIivEXT, target, pname, params);
}

GLAPI void APIENTRY glTexParameterIuiv( GLenum target, GLenum pname, const GLuint* params) {
    CALL_3(TexParameterIuiv, target, pname, params);
}

GLAPI void APIENTRY glTexParameterIuivEXT( GLenum target, GLenum pname, const GLuint* params) {
    CALL_3(TexParameterIuivEXT, target, pname, params);
}

GLAPI void APIENTRY glTexParameterxOES( GLenum target, GLenum pname, GLfixed param) {
    CALL_3(TexParameterxOES, target, pname, param);
}

GLAPI void APIENTRY glTexParameterxvOES( GLenum target, GLenum pname, const GLfixed* params) {
    CALL_3(TexParameterxvOES, target, pname, params);
}

GLAPI void APIENTRY glTexRenderbufferNV( GLenum target, GLuint renderbuffer) {
    CALL_2(TexRenderbufferNV, target, renderbuffer);
}

GLAPI void APIENTRY glTexStorage1D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) {
    CALL_4(TexStorage1D, target, levels, internalformat, width);
}

GLAPI void APIENTRY glTexStorage2D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_5(TexStorage2D, target, levels, internalformat, width, height);
}

GLAPI void APIENTRY glTexStorage2DMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
    CALL_6(TexStorage2DMultisample, target, samples, internalformat, width, height, fixedsamplelocations);
}

GLAPI void APIENTRY glTexStorage3D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {
    CALL_6(TexStorage3D, target, levels, internalformat, width, height, depth);
}

GLAPI void APIENTRY glTexStorage3DMultisample( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
    CALL_7(TexStorage3DMultisample, target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

GLAPI void APIENTRY glTexStorageSparseAMD( GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags) {
    CALL_7(TexStorageSparseAMD, target, internalFormat, width, height, depth, layers, flags);
}

GLAPI void APIENTRY glTexSubImage1DEXT( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    CALL_7(TexSubImage1DEXT, target, level, xoffset, width, format, type, pixels);
}

GLAPI void APIENTRY glTexSubImage2DEXT( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    CALL_9(TexSubImage2DEXT, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLAPI void APIENTRY glTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    CALL_11(TexSubImage3D, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLAPI void APIENTRY glTexSubImage3DEXT( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    CALL_11(TexSubImage3DEXT, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLAPI void APIENTRY glTexSubImage4DSGIS( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const void* pixels) {
    CALL_13(TexSubImage4DSGIS, target, level, xoffset, yoffset, zoffset, woffset, width, height, depth, size4d, format, type, pixels);
}

GLAPI void APIENTRY glTextureBarrier( void ) {
    CALL_0(TextureBarrier);
}

GLAPI void APIENTRY glTextureBarrierNV( void ) {
    CALL_0(TextureBarrierNV);
}

GLAPI void APIENTRY glTextureBuffer( GLuint texture, GLenum internalformat, GLuint buffer) {
    CALL_3(TextureBuffer, texture, internalformat, buffer);
}

GLAPI void APIENTRY glTextureBufferEXT( GLuint texture, GLenum target, GLenum internalformat, GLuint buffer) {
    CALL_4(TextureBufferEXT, texture, target, internalformat, buffer);
}

GLAPI void APIENTRY glTextureBufferRange( GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    CALL_5(TextureBufferRange, texture, internalformat, buffer, offset, size);
}

GLAPI void APIENTRY glTextureBufferRangeEXT( GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    CALL_6(TextureBufferRangeEXT, texture, target, internalformat, buffer, offset, size);
}

GLAPI void APIENTRY glTextureColorMaskSGIS( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    CALL_4(TextureColorMaskSGIS, red, green, blue, alpha);
}

GLAPI void APIENTRY glTextureImage1DEXT( GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_9(TextureImage1DEXT, texture, target, level, internalformat, width, border, format, type, pixels);
}

GLAPI void APIENTRY glTextureImage2DEXT( GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_10(TextureImage2DEXT, texture, target, level, internalformat, width, height, border, format, type, pixels);
}

GLAPI void APIENTRY glTextureImage2DMultisampleCoverageNV( GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations) {
    CALL_8(TextureImage2DMultisampleCoverageNV, texture, target, coverageSamples, colorSamples, internalFormat, width, height, fixedSampleLocations);
}

GLAPI void APIENTRY glTextureImage2DMultisampleNV( GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations) {
    CALL_7(TextureImage2DMultisampleNV, texture, target, samples, internalFormat, width, height, fixedSampleLocations);
}

GLAPI void APIENTRY glTextureImage3DEXT( GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) {
    CALL_11(TextureImage3DEXT, texture, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

GLAPI void APIENTRY glTextureImage3DMultisampleCoverageNV( GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations) {
    CALL_9(TextureImage3DMultisampleCoverageNV, texture, target, coverageSamples, colorSamples, internalFormat, width, height, depth, fixedSampleLocations);
}

GLAPI void APIENTRY glTextureImage3DMultisampleNV( GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations) {
    CALL_8(TextureImage3DMultisampleNV, texture, target, samples, internalFormat, width, height, depth, fixedSampleLocations);
}

GLAPI void APIENTRY glTextureLightEXT( GLenum pname) {
    CALL_1(TextureLightEXT, pname);
}

GLAPI void APIENTRY glTextureMaterialEXT( GLenum face, GLenum mode) {
    CALL_2(TextureMaterialEXT, face, mode);
}

GLAPI void APIENTRY glTextureNormalEXT( GLenum mode) {
    CALL_1(TextureNormalEXT, mode);
}

GLAPI void APIENTRY glTexturePageCommitmentEXT( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) {
    CALL_9(TexturePageCommitmentEXT, texture, level, xoffset, yoffset, zoffset, width, height, depth, commit);
}

GLAPI void APIENTRY glTextureParameterIiv( GLuint texture, GLenum pname, const GLint* params) {
    CALL_3(TextureParameterIiv, texture, pname, params);
}

GLAPI void APIENTRY glTextureParameterIivEXT( GLuint texture, GLenum target, GLenum pname, const GLint* params) {
    CALL_4(TextureParameterIivEXT, texture, target, pname, params);
}

GLAPI void APIENTRY glTextureParameterIuiv( GLuint texture, GLenum pname, const GLuint* params) {
    CALL_3(TextureParameterIuiv, texture, pname, params);
}

GLAPI void APIENTRY glTextureParameterIuivEXT( GLuint texture, GLenum target, GLenum pname, const GLuint* params) {
    CALL_4(TextureParameterIuivEXT, texture, target, pname, params);
}

GLAPI void APIENTRY glTextureParameterf( GLuint texture, GLenum pname, GLfloat param) {
    CALL_3(TextureParameterf, texture, pname, F(param));
}

GLAPI void APIENTRY glTextureParameterfEXT( GLuint texture, GLenum target, GLenum pname, GLfloat param) {
    CALL_4(TextureParameterfEXT, texture, target, pname, F(param));
}

GLAPI void APIENTRY glTextureParameterfv( GLuint texture, GLenum pname, const GLfloat* param) {
    CALL_3(TextureParameterfv, texture, pname, param);
}

GLAPI void APIENTRY glTextureParameterfvEXT( GLuint texture, GLenum target, GLenum pname, const GLfloat* params) {
    CALL_4(TextureParameterfvEXT, texture, target, pname, params);
}

GLAPI void APIENTRY glTextureParameteri( GLuint texture, GLenum pname, GLint param) {
    CALL_3(TextureParameteri, texture, pname, param);
}

GLAPI void APIENTRY glTextureParameteriEXT( GLuint texture, GLenum target, GLenum pname, GLint param) {
    CALL_4(TextureParameteriEXT, texture, target, pname, param);
}

GLAPI void APIENTRY glTextureParameteriv( GLuint texture, GLenum pname, const GLint* param) {
    CALL_3(TextureParameteriv, texture, pname, param);
}

GLAPI void APIENTRY glTextureParameterivEXT( GLuint texture, GLenum target, GLenum pname, const GLint* params) {
    CALL_4(TextureParameterivEXT, texture, target, pname, params);
}

GLAPI void APIENTRY glTextureRangeAPPLE( GLenum target, GLsizei length, const void* pointer) {
    CALL_3(TextureRangeAPPLE, target, length, pointer);
}

GLAPI void APIENTRY glTextureRenderbufferEXT( GLuint texture, GLenum target, GLuint renderbuffer) {
    CALL_3(TextureRenderbufferEXT, texture, target, renderbuffer);
}

GLAPI void APIENTRY glTextureStorage1D( GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width) {
    CALL_4(TextureStorage1D, texture, levels, internalformat, width);
}

GLAPI void APIENTRY glTextureStorage1DEXT( GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) {
    CALL_5(TextureStorage1DEXT, texture, target, levels, internalformat, width);
}

GLAPI void APIENTRY glTextureStorage2D( GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_5(TextureStorage2D, texture, levels, internalformat, width, height);
}

GLAPI void APIENTRY glTextureStorage2DEXT( GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
    CALL_6(TextureStorage2DEXT, texture, target, levels, internalformat, width, height);
}

GLAPI void APIENTRY glTextureStorage2DMultisample( GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
    CALL_6(TextureStorage2DMultisample, texture, samples, internalformat, width, height, fixedsamplelocations);
}

GLAPI void APIENTRY glTextureStorage2DMultisampleEXT( GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
    CALL_7(TextureStorage2DMultisampleEXT, texture, target, samples, internalformat, width, height, fixedsamplelocations);
}

GLAPI void APIENTRY glTextureStorage3D( GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {
    CALL_6(TextureStorage3D, texture, levels, internalformat, width, height, depth);
}

GLAPI void APIENTRY glTextureStorage3DEXT( GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {
    CALL_7(TextureStorage3DEXT, texture, target, levels, internalformat, width, height, depth);
}

GLAPI void APIENTRY glTextureStorage3DMultisample( GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
    CALL_7(TextureStorage3DMultisample, texture, samples, internalformat, width, height, depth, fixedsamplelocations);
}

GLAPI void APIENTRY glTextureStorage3DMultisampleEXT( GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
    CALL_8(TextureStorage3DMultisampleEXT, texture, target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

GLAPI void APIENTRY glTextureStorageSparseAMD( GLuint texture, GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags) {
    CALL_8(TextureStorageSparseAMD, texture, target, internalFormat, width, height, depth, layers, flags);
}

GLAPI void APIENTRY glTextureSubImage1D( GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    CALL_7(TextureSubImage1D, texture, level, xoffset, width, format, type, pixels);
}

GLAPI void APIENTRY glTextureSubImage1DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels) {
    CALL_8(TextureSubImage1DEXT, texture, target, level, xoffset, width, format, type, pixels);
}

GLAPI void APIENTRY glTextureSubImage2D( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    CALL_9(TextureSubImage2D, texture, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLAPI void APIENTRY glTextureSubImage2DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) {
    CALL_10(TextureSubImage2DEXT, texture, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLAPI void APIENTRY glTextureSubImage3D( GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    CALL_11(TextureSubImage3D, texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLAPI void APIENTRY glTextureSubImage3DEXT( GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) {
    CALL_12(TextureSubImage3DEXT, texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLAPI void APIENTRY glTextureView( GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers) {
    CALL_8(TextureView, texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers);
}

GLAPI void APIENTRY glTrackMatrixNV( GLenum target, GLuint address, GLenum matrix, GLenum transform) {
    CALL_4(TrackMatrixNV, target, address, matrix, transform);
}

GLAPI void APIENTRY glTransformFeedbackAttribsNV( GLsizei count, const GLint* attribs, GLenum bufferMode) {
    CALL_3(TransformFeedbackAttribsNV, count, attribs, bufferMode);
}

GLAPI void APIENTRY glTransformFeedbackBufferBase( GLuint xfb, GLuint index, GLuint buffer) {
    CALL_3(TransformFeedbackBufferBase, xfb, index, buffer);
}

GLAPI void APIENTRY glTransformFeedbackBufferRange( GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    CALL_5(TransformFeedbackBufferRange, xfb, index, buffer, offset, size);
}

GLAPI void APIENTRY glTransformFeedbackStreamAttribsNV( GLsizei count, const GLint* attribs, GLsizei nbuffers, const GLint* bufstreams, GLenum bufferMode) {
    CALL_5(TransformFeedbackStreamAttribsNV, count, attribs, nbuffers, bufstreams, bufferMode);
}

GLAPI void APIENTRY glTransformFeedbackVaryings( GLuint program, GLsizei count, const GLchar*const* varyings, GLenum bufferMode) {
    CALL_4(TransformFeedbackVaryings, program, count, varyings, bufferMode);
}

GLAPI void APIENTRY glTransformFeedbackVaryingsEXT( GLuint program, GLsizei count, const GLchar*const* varyings, GLenum bufferMode) {
    CALL_4(TransformFeedbackVaryingsEXT, program, count, varyings, bufferMode);
}

GLAPI void APIENTRY glTransformFeedbackVaryingsNV( GLuint program, GLsizei count, const GLint* locations, GLenum bufferMode) {
    CALL_4(TransformFeedbackVaryingsNV, program, count, locations, bufferMode);
}

GLAPI void APIENTRY glTransformPathNV( GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat* transformValues) {
    CALL_4(TransformPathNV, resultPath, srcPath, transformType, transformValues);
}

GLAPI void APIENTRY glTranslatexOES( GLfixed x, GLfixed y, GLfixed z) {
    CALL_3(TranslatexOES, x, y, z);
}

GLAPI void APIENTRY glUniform1d( GLint location, GLdouble x) {
    CALL_2(Uniform1d, location, D(x));
}

GLAPI void APIENTRY glUniform1dv( GLint location, GLsizei count, const GLdouble* value) {
    CALL_3(Uniform1dv, location, count, value);
}

GLAPI void APIENTRY glUniform1f( GLint location, GLfloat v0) {
    CALL_2(Uniform1f, location, F(v0));
}

GLAPI void APIENTRY glUniform1fARB( GLint location, GLfloat v0) {
    CALL_2(Uniform1fARB, location, F(v0));
}

GLAPI void APIENTRY glUniform1fv( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform1fv, location, count, value);
}

GLAPI void APIENTRY glUniform1fvARB( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform1fvARB, location, count, value);
}

GLAPI void APIENTRY glUniform1i( GLint location, GLint v0) {
    CALL_2(Uniform1i, location, v0);
}

GLAPI void APIENTRY glUniform1i64ARB( GLint location, GLint64 x) {
    CALL_2(Uniform1i64ARB, location, LL(x));
}

GLAPI void APIENTRY glUniform1i64NV( GLint location, GLint64EXT x) {
    CALL_2(Uniform1i64NV, location, LL(x));
}

GLAPI void APIENTRY glUniform1i64vARB( GLint location, GLsizei count, const GLint64* value) {
    CALL_3(Uniform1i64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform1i64vNV( GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_3(Uniform1i64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform1iARB( GLint location, GLint v0) {
    CALL_2(Uniform1iARB, location, v0);
}

GLAPI void APIENTRY glUniform1iv( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform1iv, location, count, value);
}

GLAPI void APIENTRY glUniform1ivARB( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform1ivARB, location, count, value);
}

GLAPI void APIENTRY glUniform1ui( GLint location, GLuint v0) {
    CALL_2(Uniform1ui, location, v0);
}

GLAPI void APIENTRY glUniform1ui64ARB( GLint location, GLuint64 x) {
    CALL_2(Uniform1ui64ARB, location, LL(x));
}

GLAPI void APIENTRY glUniform1ui64NV( GLint location, GLuint64EXT x) {
    CALL_2(Uniform1ui64NV, location, LL(x));
}

GLAPI void APIENTRY glUniform1ui64vARB( GLint location, GLsizei count, const GLuint64* value) {
    CALL_3(Uniform1ui64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform1ui64vNV( GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_3(Uniform1ui64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform1uiEXT( GLint location, GLuint v0) {
    CALL_2(Uniform1uiEXT, location, v0);
}

GLAPI void APIENTRY glUniform1uiv( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform1uiv, location, count, value);
}

GLAPI void APIENTRY glUniform1uivEXT( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform1uivEXT, location, count, value);
}

GLAPI void APIENTRY glUniform2d( GLint location, GLdouble x, GLdouble y) {
    CALL_3(Uniform2d, location, D(x), D(y));
}

GLAPI void APIENTRY glUniform2dv( GLint location, GLsizei count, const GLdouble* value) {
    CALL_3(Uniform2dv, location, count, value);
}

GLAPI void APIENTRY glUniform2f( GLint location, GLfloat v0, GLfloat v1) {
    CALL_3(Uniform2f, location, F(v0), F(v1));
}

GLAPI void APIENTRY glUniform2fARB( GLint location, GLfloat v0, GLfloat v1) {
    CALL_3(Uniform2fARB, location, F(v0), F(v1));
}

GLAPI void APIENTRY glUniform2fv( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform2fv, location, count, value);
}

GLAPI void APIENTRY glUniform2fvARB( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform2fvARB, location, count, value);
}

GLAPI void APIENTRY glUniform2i( GLint location, GLint v0, GLint v1) {
    CALL_3(Uniform2i, location, v0, v1);
}

GLAPI void APIENTRY glUniform2i64ARB( GLint location, GLint64 x, GLint64 y) {
    CALL_3(Uniform2i64ARB, location, LL(x), LL(y));
}

GLAPI void APIENTRY glUniform2i64NV( GLint location, GLint64EXT x, GLint64EXT y) {
    CALL_3(Uniform2i64NV, location, LL(x), LL(y));
}

GLAPI void APIENTRY glUniform2i64vARB( GLint location, GLsizei count, const GLint64* value) {
    CALL_3(Uniform2i64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform2i64vNV( GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_3(Uniform2i64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform2iARB( GLint location, GLint v0, GLint v1) {
    CALL_3(Uniform2iARB, location, v0, v1);
}

GLAPI void APIENTRY glUniform2iv( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform2iv, location, count, value);
}

GLAPI void APIENTRY glUniform2ivARB( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform2ivARB, location, count, value);
}

GLAPI void APIENTRY glUniform2ui( GLint location, GLuint v0, GLuint v1) {
    CALL_3(Uniform2ui, location, v0, v1);
}

GLAPI void APIENTRY glUniform2ui64ARB( GLint location, GLuint64 x, GLuint64 y) {
    CALL_3(Uniform2ui64ARB, location, LL(x), LL(y));
}

GLAPI void APIENTRY glUniform2ui64NV( GLint location, GLuint64EXT x, GLuint64EXT y) {
    CALL_3(Uniform2ui64NV, location, LL(x), LL(y));
}

GLAPI void APIENTRY glUniform2ui64vARB( GLint location, GLsizei count, const GLuint64* value) {
    CALL_3(Uniform2ui64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform2ui64vNV( GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_3(Uniform2ui64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform2uiEXT( GLint location, GLuint v0, GLuint v1) {
    CALL_3(Uniform2uiEXT, location, v0, v1);
}

GLAPI void APIENTRY glUniform2uiv( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform2uiv, location, count, value);
}

GLAPI void APIENTRY glUniform2uivEXT( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform2uivEXT, location, count, value);
}

GLAPI void APIENTRY glUniform3d( GLint location, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(Uniform3d, location, D(x), D(y), D(z));
}

GLAPI void APIENTRY glUniform3dv( GLint location, GLsizei count, const GLdouble* value) {
    CALL_3(Uniform3dv, location, count, value);
}

GLAPI void APIENTRY glUniform3f( GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    CALL_4(Uniform3f, location, F(v0), F(v1), F(v2));
}

GLAPI void APIENTRY glUniform3fARB( GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    CALL_4(Uniform3fARB, location, F(v0), F(v1), F(v2));
}

GLAPI void APIENTRY glUniform3fv( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform3fv, location, count, value);
}

GLAPI void APIENTRY glUniform3fvARB( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform3fvARB, location, count, value);
}

GLAPI void APIENTRY glUniform3i( GLint location, GLint v0, GLint v1, GLint v2) {
    CALL_4(Uniform3i, location, v0, v1, v2);
}

GLAPI void APIENTRY glUniform3i64ARB( GLint location, GLint64 x, GLint64 y, GLint64 z) {
    CALL_4(Uniform3i64ARB, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glUniform3i64NV( GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z) {
    CALL_4(Uniform3i64NV, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glUniform3i64vARB( GLint location, GLsizei count, const GLint64* value) {
    CALL_3(Uniform3i64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform3i64vNV( GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_3(Uniform3i64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform3iARB( GLint location, GLint v0, GLint v1, GLint v2) {
    CALL_4(Uniform3iARB, location, v0, v1, v2);
}

GLAPI void APIENTRY glUniform3iv( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform3iv, location, count, value);
}

GLAPI void APIENTRY glUniform3ivARB( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform3ivARB, location, count, value);
}

GLAPI void APIENTRY glUniform3ui( GLint location, GLuint v0, GLuint v1, GLuint v2) {
    CALL_4(Uniform3ui, location, v0, v1, v2);
}

GLAPI void APIENTRY glUniform3ui64ARB( GLint location, GLuint64 x, GLuint64 y, GLuint64 z) {
    CALL_4(Uniform3ui64ARB, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glUniform3ui64NV( GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z) {
    CALL_4(Uniform3ui64NV, location, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glUniform3ui64vARB( GLint location, GLsizei count, const GLuint64* value) {
    CALL_3(Uniform3ui64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform3ui64vNV( GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_3(Uniform3ui64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform3uiEXT( GLint location, GLuint v0, GLuint v1, GLuint v2) {
    CALL_4(Uniform3uiEXT, location, v0, v1, v2);
}

GLAPI void APIENTRY glUniform3uiv( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform3uiv, location, count, value);
}

GLAPI void APIENTRY glUniform3uivEXT( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform3uivEXT, location, count, value);
}

GLAPI void APIENTRY glUniform4d( GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_5(Uniform4d, location, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glUniform4dv( GLint location, GLsizei count, const GLdouble* value) {
    CALL_3(Uniform4dv, location, count, value);
}

GLAPI void APIENTRY glUniform4f( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    CALL_5(Uniform4f, location, F(v0), F(v1), F(v2), F(v3));
}

GLAPI void APIENTRY glUniform4fARB( GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    CALL_5(Uniform4fARB, location, F(v0), F(v1), F(v2), F(v3));
}

GLAPI void APIENTRY glUniform4fv( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform4fv, location, count, value);
}

GLAPI void APIENTRY glUniform4fvARB( GLint location, GLsizei count, const GLfloat* value) {
    CALL_3(Uniform4fvARB, location, count, value);
}

GLAPI void APIENTRY glUniform4i( GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    CALL_5(Uniform4i, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glUniform4i64ARB( GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w) {
    CALL_5(Uniform4i64ARB, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glUniform4i64NV( GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w) {
    CALL_5(Uniform4i64NV, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glUniform4i64vARB( GLint location, GLsizei count, const GLint64* value) {
    CALL_3(Uniform4i64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform4i64vNV( GLint location, GLsizei count, const GLint64EXT* value) {
    CALL_3(Uniform4i64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform4iARB( GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    CALL_5(Uniform4iARB, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glUniform4iv( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform4iv, location, count, value);
}

GLAPI void APIENTRY glUniform4ivARB( GLint location, GLsizei count, const GLint* value) {
    CALL_3(Uniform4ivARB, location, count, value);
}

GLAPI void APIENTRY glUniform4ui( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    CALL_5(Uniform4ui, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glUniform4ui64ARB( GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w) {
    CALL_5(Uniform4ui64ARB, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glUniform4ui64NV( GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w) {
    CALL_5(Uniform4ui64NV, location, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glUniform4ui64vARB( GLint location, GLsizei count, const GLuint64* value) {
    CALL_3(Uniform4ui64vARB, location, count, value);
}

GLAPI void APIENTRY glUniform4ui64vNV( GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_3(Uniform4ui64vNV, location, count, value);
}

GLAPI void APIENTRY glUniform4uiEXT( GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    CALL_5(Uniform4uiEXT, location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glUniform4uiv( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform4uiv, location, count, value);
}

GLAPI void APIENTRY glUniform4uivEXT( GLint location, GLsizei count, const GLuint* value) {
    CALL_3(Uniform4uivEXT, location, count, value);
}

GLAPI void APIENTRY glUniformBlockBinding( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
    CALL_3(UniformBlockBinding, program, uniformBlockIndex, uniformBlockBinding);
}

GLAPI void APIENTRY glUniformBufferEXT( GLuint program, GLint location, GLuint buffer) {
    CALL_3(UniformBufferEXT, program, location, buffer);
}

GLAPI void APIENTRY glUniformHandleui64ARB( GLint location, GLuint64 value) {
    CALL_2(UniformHandleui64ARB, location, LL(value));
}

GLAPI void APIENTRY glUniformHandleui64NV( GLint location, GLuint64 value) {
    CALL_2(UniformHandleui64NV, location, LL(value));
}

GLAPI void APIENTRY glUniformHandleui64vARB( GLint location, GLsizei count, const GLuint64* value) {
    CALL_3(UniformHandleui64vARB, location, count, value);
}

GLAPI void APIENTRY glUniformHandleui64vNV( GLint location, GLsizei count, const GLuint64* value) {
    CALL_3(UniformHandleui64vNV, location, count, value);
}

GLAPI void APIENTRY glUniformMatrix2dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix2dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix2fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix2fvARB( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix2fvARB, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix2x3dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix2x3dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix2x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix2x3fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix2x4dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix2x4dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix2x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix2x4fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix3dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix3fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3fvARB( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix3fvARB, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3x2dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix3x2dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix3x2fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3x4dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix3x4dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3x4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix3x4fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix4dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix4dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix4fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix4fvARB( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix4fvARB, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix4x2dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix4x2dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix4x2fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix4x2fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix4x3dv( GLint location, GLsizei count, GLboolean transpose, const GLdouble* value) {
    CALL_4(UniformMatrix4x3dv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix4x3fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    CALL_4(UniformMatrix4x3fv, location, count, transpose, value);
}

GLAPI void APIENTRY glUniformSubroutinesuiv( GLenum shadertype, GLsizei count, const GLuint* indices) {
    CALL_3(UniformSubroutinesuiv, shadertype, count, indices);
}

GLAPI void APIENTRY glUniformui64NV( GLint location, GLuint64EXT value) {
    CALL_2(Uniformui64NV, location, LL(value));
}

GLAPI void APIENTRY glUniformui64vNV( GLint location, GLsizei count, const GLuint64EXT* value) {
    CALL_3(Uniformui64vNV, location, count, value);
}

GLAPI void APIENTRY glUnlockArraysEXT( void ) {
    CALL_0(UnlockArraysEXT);
}

GLAPI GLboolean APIENTRY glUnmapBuffer( GLenum target) {
    CALL_1_R(UnmapBuffer, target);
}

GLAPI GLboolean APIENTRY glUnmapBufferARB( GLenum target) {
    CALL_1_R(UnmapBufferARB, target);
}

GLAPI GLboolean APIENTRY glUnmapNamedBuffer( GLuint buffer) {
    CALL_1_R(UnmapNamedBuffer, buffer);
}

GLAPI GLboolean APIENTRY glUnmapNamedBufferEXT( GLuint buffer) {
    CALL_1_R(UnmapNamedBufferEXT, buffer);
}

GLAPI void APIENTRY glUnmapObjectBufferATI( GLuint buffer) {
    CALL_1(UnmapObjectBufferATI, buffer);
}

GLAPI void APIENTRY glUnmapTexture2DINTEL( GLuint texture, GLint level) {
    CALL_2(UnmapTexture2DINTEL, texture, level);
}

GLAPI void APIENTRY glUpdateObjectBufferATI( GLuint buffer, GLuint offset, GLsizei size, const void* pointer, GLenum preserve) {
    CALL_5(UpdateObjectBufferATI, buffer, offset, size, pointer, preserve);
}

GLAPI void APIENTRY glUseProgram( GLuint program) {
    CALL_1(UseProgram, program);
}

GLAPI void APIENTRY glUseProgramObjectARB( GLhandleARB programObj) {
    CALL_1(UseProgramObjectARB, programObj);
}

GLAPI void APIENTRY glUseProgramStages( GLuint pipeline, GLbitfield stages, GLuint program) {
    CALL_3(UseProgramStages, pipeline, stages, program);
}

GLAPI void APIENTRY glUseShaderProgramEXT( GLenum type, GLuint program) {
    CALL_2(UseShaderProgramEXT, type, program);
}

GLAPI void APIENTRY glVDPAUFiniNV( void ) {
    CALL_0(VDPAUFiniNV);
}

GLAPI void APIENTRY glVDPAUGetSurfaceivNV( GLvdpauSurfaceNV surface, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) {
    CALL_5(VDPAUGetSurfaceivNV, surface, pname, bufSize, length, values);
}

GLAPI void APIENTRY glVDPAUInitNV( const void* vdpDevice, const void* getProcAddress) {
    CALL_2(VDPAUInitNV, vdpDevice, getProcAddress);
}

GLAPI GLboolean APIENTRY glVDPAUIsSurfaceNV( GLvdpauSurfaceNV surface) {
    CALL_1_R(VDPAUIsSurfaceNV, surface);
}

GLAPI void APIENTRY glVDPAUMapSurfacesNV( GLsizei numSurfaces, const GLvdpauSurfaceNV* surfaces) {
    CALL_2(VDPAUMapSurfacesNV, numSurfaces, surfaces);
}

GLAPI GLvdpauSurfaceNV APIENTRY glVDPAURegisterOutputSurfaceNV( const void* vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint* textureNames) {
    CALL_4_R(VDPAURegisterOutputSurfaceNV, vdpSurface, target, numTextureNames, textureNames);
}

GLAPI GLvdpauSurfaceNV APIENTRY glVDPAURegisterVideoSurfaceNV( const void* vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint* textureNames) {
    CALL_4_R(VDPAURegisterVideoSurfaceNV, vdpSurface, target, numTextureNames, textureNames);
}

GLAPI void APIENTRY glVDPAUSurfaceAccessNV( GLvdpauSurfaceNV surface, GLenum access) {
    CALL_2(VDPAUSurfaceAccessNV, surface, access);
}

GLAPI void APIENTRY glVDPAUUnmapSurfacesNV( GLsizei numSurface, const GLvdpauSurfaceNV* surfaces) {
    CALL_2(VDPAUUnmapSurfacesNV, numSurface, surfaces);
}

GLAPI void APIENTRY glVDPAUUnregisterSurfaceNV( GLvdpauSurfaceNV surface) {
    CALL_1(VDPAUUnregisterSurfaceNV, surface);
}

GLAPI void APIENTRY glValidateProgram( GLuint program) {
    CALL_1(ValidateProgram, program);
}

GLAPI void APIENTRY glValidateProgramARB( GLhandleARB programObj) {
    CALL_1(ValidateProgramARB, programObj);
}

GLAPI void APIENTRY glValidateProgramPipeline( GLuint pipeline) {
    CALL_1(ValidateProgramPipeline, pipeline);
}

GLAPI void APIENTRY glVariantArrayObjectATI( GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset) {
    CALL_5(VariantArrayObjectATI, id, type, stride, buffer, offset);
}

GLAPI void APIENTRY glVariantPointerEXT( GLuint id, GLenum type, GLuint stride, const void* addr) {
    CALL_4(VariantPointerEXT, id, type, stride, addr);
}

GLAPI void APIENTRY glVariantbvEXT( GLuint id, const GLbyte* addr) {
    CALL_2(VariantbvEXT, id, addr);
}

GLAPI void APIENTRY glVariantdvEXT( GLuint id, const GLdouble* addr) {
    CALL_2(VariantdvEXT, id, addr);
}

GLAPI void APIENTRY glVariantfvEXT( GLuint id, const GLfloat* addr) {
    CALL_2(VariantfvEXT, id, addr);
}

GLAPI void APIENTRY glVariantivEXT( GLuint id, const GLint* addr) {
    CALL_2(VariantivEXT, id, addr);
}

GLAPI void APIENTRY glVariantsvEXT( GLuint id, const GLshort* addr) {
    CALL_2(VariantsvEXT, id, addr);
}

GLAPI void APIENTRY glVariantubvEXT( GLuint id, const GLubyte* addr) {
    CALL_2(VariantubvEXT, id, addr);
}

GLAPI void APIENTRY glVariantuivEXT( GLuint id, const GLuint* addr) {
    CALL_2(VariantuivEXT, id, addr);
}

GLAPI void APIENTRY glVariantusvEXT( GLuint id, const GLushort* addr) {
    CALL_2(VariantusvEXT, id, addr);
}

GLAPI void APIENTRY glVertex2bOES( GLbyte x, GLbyte y) {
    CALL_2(Vertex2bOES, x, y);
}

GLAPI void APIENTRY glVertex2bvOES( const GLbyte* coords) {
    CALL_1(Vertex2bvOES, coords);
}

GLAPI void APIENTRY glVertex2hNV( GLhalfNV x, GLhalfNV y) {
    CALL_2(Vertex2hNV, x, y);
}

GLAPI void APIENTRY glVertex2hvNV( const GLhalfNV* v) {
    CALL_1(Vertex2hvNV, v);
}

GLAPI void APIENTRY glVertex2xOES( GLfixed x) {
    CALL_1(Vertex2xOES, x);
}

GLAPI void APIENTRY glVertex2xvOES( const GLfixed* coords) {
    CALL_1(Vertex2xvOES, coords);
}

GLAPI void APIENTRY glVertex3bOES( GLbyte x, GLbyte y, GLbyte z) {
    CALL_3(Vertex3bOES, x, y, z);
}

GLAPI void APIENTRY glVertex3bvOES( const GLbyte* coords) {
    CALL_1(Vertex3bvOES, coords);
}

GLAPI void APIENTRY glVertex3hNV( GLhalfNV x, GLhalfNV y, GLhalfNV z) {
    CALL_3(Vertex3hNV, x, y, z);
}

GLAPI void APIENTRY glVertex3hvNV( const GLhalfNV* v) {
    CALL_1(Vertex3hvNV, v);
}

GLAPI void APIENTRY glVertex3xOES( GLfixed x, GLfixed y) {
    CALL_2(Vertex3xOES, x, y);
}

GLAPI void APIENTRY glVertex3xvOES( const GLfixed* coords) {
    CALL_1(Vertex3xvOES, coords);
}

GLAPI void APIENTRY glVertex4bOES( GLbyte x, GLbyte y, GLbyte z, GLbyte w) {
    CALL_4(Vertex4bOES, x, y, z, w);
}

GLAPI void APIENTRY glVertex4bvOES( const GLbyte* coords) {
    CALL_1(Vertex4bvOES, coords);
}

GLAPI void APIENTRY glVertex4hNV( GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w) {
    CALL_4(Vertex4hNV, x, y, z, w);
}

GLAPI void APIENTRY glVertex4hvNV( const GLhalfNV* v) {
    CALL_1(Vertex4hvNV, v);
}

GLAPI void APIENTRY glVertex4xOES( GLfixed x, GLfixed y, GLfixed z) {
    CALL_3(Vertex4xOES, x, y, z);
}

GLAPI void APIENTRY glVertex4xvOES( const GLfixed* coords) {
    CALL_1(Vertex4xvOES, coords);
}

GLAPI void APIENTRY glVertexArrayAttribBinding( GLuint vaobj, GLuint attribindex, GLuint bindingindex) {
    CALL_3(VertexArrayAttribBinding, vaobj, attribindex, bindingindex);
}

GLAPI void APIENTRY glVertexArrayAttribFormat( GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) {
    CALL_6(VertexArrayAttribFormat, vaobj, attribindex, size, type, normalized, relativeoffset);
}

GLAPI void APIENTRY glVertexArrayAttribIFormat( GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
    CALL_5(VertexArrayAttribIFormat, vaobj, attribindex, size, type, relativeoffset);
}

GLAPI void APIENTRY glVertexArrayAttribLFormat( GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
    CALL_5(VertexArrayAttribLFormat, vaobj, attribindex, size, type, relativeoffset);
}

GLAPI void APIENTRY glVertexArrayBindVertexBufferEXT( GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {
    CALL_5(VertexArrayBindVertexBufferEXT, vaobj, bindingindex, buffer, offset, stride);
}

GLAPI void APIENTRY glVertexArrayBindingDivisor( GLuint vaobj, GLuint bindingindex, GLuint divisor) {
    CALL_3(VertexArrayBindingDivisor, vaobj, bindingindex, divisor);
}

GLAPI void APIENTRY glVertexArrayColorOffsetEXT( GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_6(VertexArrayColorOffsetEXT, vaobj, buffer, size, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayEdgeFlagOffsetEXT( GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset) {
    CALL_4(VertexArrayEdgeFlagOffsetEXT, vaobj, buffer, stride, offset);
}

GLAPI void APIENTRY glVertexArrayElementBuffer( GLuint vaobj, GLuint buffer) {
    CALL_2(VertexArrayElementBuffer, vaobj, buffer);
}

GLAPI void APIENTRY glVertexArrayFogCoordOffsetEXT( GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_5(VertexArrayFogCoordOffsetEXT, vaobj, buffer, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayIndexOffsetEXT( GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_5(VertexArrayIndexOffsetEXT, vaobj, buffer, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayMultiTexCoordOffsetEXT( GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_7(VertexArrayMultiTexCoordOffsetEXT, vaobj, buffer, texunit, size, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayNormalOffsetEXT( GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_5(VertexArrayNormalOffsetEXT, vaobj, buffer, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayParameteriAPPLE( GLenum pname, GLint param) {
    CALL_2(VertexArrayParameteriAPPLE, pname, param);
}

GLAPI void APIENTRY glVertexArrayRangeAPPLE( GLsizei length, void* pointer) {
    CALL_2(VertexArrayRangeAPPLE, length, pointer);
}

GLAPI void APIENTRY glVertexArrayRangeNV( GLsizei length, const void* pointer) {
    CALL_2(VertexArrayRangeNV, length, pointer);
}

GLAPI void APIENTRY glVertexArraySecondaryColorOffsetEXT( GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_6(VertexArraySecondaryColorOffsetEXT, vaobj, buffer, size, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayTexCoordOffsetEXT( GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_6(VertexArrayTexCoordOffsetEXT, vaobj, buffer, size, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayVertexAttribBindingEXT( GLuint vaobj, GLuint attribindex, GLuint bindingindex) {
    CALL_3(VertexArrayVertexAttribBindingEXT, vaobj, attribindex, bindingindex);
}

GLAPI void APIENTRY glVertexArrayVertexAttribDivisorEXT( GLuint vaobj, GLuint index, GLuint divisor) {
    CALL_3(VertexArrayVertexAttribDivisorEXT, vaobj, index, divisor);
}

GLAPI void APIENTRY glVertexArrayVertexAttribFormatEXT( GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) {
    CALL_6(VertexArrayVertexAttribFormatEXT, vaobj, attribindex, size, type, normalized, relativeoffset);
}

GLAPI void APIENTRY glVertexArrayVertexAttribIFormatEXT( GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
    CALL_5(VertexArrayVertexAttribIFormatEXT, vaobj, attribindex, size, type, relativeoffset);
}

GLAPI void APIENTRY glVertexArrayVertexAttribIOffsetEXT( GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_7(VertexArrayVertexAttribIOffsetEXT, vaobj, buffer, index, size, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayVertexAttribLFormatEXT( GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
    CALL_5(VertexArrayVertexAttribLFormatEXT, vaobj, attribindex, size, type, relativeoffset);
}

GLAPI void APIENTRY glVertexArrayVertexAttribLOffsetEXT( GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_7(VertexArrayVertexAttribLOffsetEXT, vaobj, buffer, index, size, type, stride, offset);
}

GLAPI void APIENTRY glVertexArrayVertexAttribOffsetEXT( GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset) {
    CALL_8(VertexArrayVertexAttribOffsetEXT, vaobj, buffer, index, size, type, normalized, stride, offset);
}

GLAPI void APIENTRY glVertexArrayVertexBindingDivisorEXT( GLuint vaobj, GLuint bindingindex, GLuint divisor) {
    CALL_3(VertexArrayVertexBindingDivisorEXT, vaobj, bindingindex, divisor);
}

GLAPI void APIENTRY glVertexArrayVertexBuffer( GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {
    CALL_5(VertexArrayVertexBuffer, vaobj, bindingindex, buffer, offset, stride);
}

GLAPI void APIENTRY glVertexArrayVertexBuffers( GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides) {
    CALL_6(VertexArrayVertexBuffers, vaobj, first, count, buffers, offsets, strides);
}

GLAPI void APIENTRY glVertexArrayVertexOffsetEXT( GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset) {
    CALL_6(VertexArrayVertexOffsetEXT, vaobj, buffer, size, type, stride, offset);
}

GLAPI void APIENTRY glVertexAttrib1d( GLuint index, GLdouble x) {
    CALL_2(VertexAttrib1d, index, D(x));
}

GLAPI void APIENTRY glVertexAttrib1dARB( GLuint index, GLdouble x) {
    CALL_2(VertexAttrib1dARB, index, D(x));
}

GLAPI void APIENTRY glVertexAttrib1dNV( GLuint index, GLdouble x) {
    CALL_2(VertexAttrib1dNV, index, D(x));
}

GLAPI void APIENTRY glVertexAttrib1dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib1dv, index, v);
}

GLAPI void APIENTRY glVertexAttrib1dvARB( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib1dvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib1dvNV( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib1dvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib1f( GLuint index, GLfloat x) {
    CALL_2(VertexAttrib1f, index, F(x));
}

GLAPI void APIENTRY glVertexAttrib1fARB( GLuint index, GLfloat x) {
    CALL_2(VertexAttrib1fARB, index, F(x));
}

GLAPI void APIENTRY glVertexAttrib1fNV( GLuint index, GLfloat x) {
    CALL_2(VertexAttrib1fNV, index, F(x));
}

GLAPI void APIENTRY glVertexAttrib1fv( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib1fv, index, v);
}

GLAPI void APIENTRY glVertexAttrib1fvARB( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib1fvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib1fvNV( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib1fvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib1hNV( GLuint index, GLhalfNV x) {
    CALL_2(VertexAttrib1hNV, index, x);
}

GLAPI void APIENTRY glVertexAttrib1hvNV( GLuint index, const GLhalfNV* v) {
    CALL_2(VertexAttrib1hvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib1s( GLuint index, GLshort x) {
    CALL_2(VertexAttrib1s, index, x);
}

GLAPI void APIENTRY glVertexAttrib1sARB( GLuint index, GLshort x) {
    CALL_2(VertexAttrib1sARB, index, x);
}

GLAPI void APIENTRY glVertexAttrib1sNV( GLuint index, GLshort x) {
    CALL_2(VertexAttrib1sNV, index, x);
}

GLAPI void APIENTRY glVertexAttrib1sv( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib1sv, index, v);
}

GLAPI void APIENTRY glVertexAttrib1svARB( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib1svARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib1svNV( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib1svNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib2d( GLuint index, GLdouble x, GLdouble y) {
    CALL_3(VertexAttrib2d, index, D(x), D(y));
}

GLAPI void APIENTRY glVertexAttrib2dARB( GLuint index, GLdouble x, GLdouble y) {
    CALL_3(VertexAttrib2dARB, index, D(x), D(y));
}

GLAPI void APIENTRY glVertexAttrib2dNV( GLuint index, GLdouble x, GLdouble y) {
    CALL_3(VertexAttrib2dNV, index, D(x), D(y));
}

GLAPI void APIENTRY glVertexAttrib2dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib2dv, index, v);
}

GLAPI void APIENTRY glVertexAttrib2dvARB( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib2dvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib2dvNV( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib2dvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib2f( GLuint index, GLfloat x, GLfloat y) {
    CALL_3(VertexAttrib2f, index, F(x), F(y));
}

GLAPI void APIENTRY glVertexAttrib2fARB( GLuint index, GLfloat x, GLfloat y) {
    CALL_3(VertexAttrib2fARB, index, F(x), F(y));
}

GLAPI void APIENTRY glVertexAttrib2fNV( GLuint index, GLfloat x, GLfloat y) {
    CALL_3(VertexAttrib2fNV, index, F(x), F(y));
}

GLAPI void APIENTRY glVertexAttrib2fv( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib2fv, index, v);
}

GLAPI void APIENTRY glVertexAttrib2fvARB( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib2fvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib2fvNV( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib2fvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib2hNV( GLuint index, GLhalfNV x, GLhalfNV y) {
    CALL_3(VertexAttrib2hNV, index, x, y);
}

GLAPI void APIENTRY glVertexAttrib2hvNV( GLuint index, const GLhalfNV* v) {
    CALL_2(VertexAttrib2hvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib2s( GLuint index, GLshort x, GLshort y) {
    CALL_3(VertexAttrib2s, index, x, y);
}

GLAPI void APIENTRY glVertexAttrib2sARB( GLuint index, GLshort x, GLshort y) {
    CALL_3(VertexAttrib2sARB, index, x, y);
}

GLAPI void APIENTRY glVertexAttrib2sNV( GLuint index, GLshort x, GLshort y) {
    CALL_3(VertexAttrib2sNV, index, x, y);
}

GLAPI void APIENTRY glVertexAttrib2sv( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib2sv, index, v);
}

GLAPI void APIENTRY glVertexAttrib2svARB( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib2svARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib2svNV( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib2svNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib3d( GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(VertexAttrib3d, index, D(x), D(y), D(z));
}

GLAPI void APIENTRY glVertexAttrib3dARB( GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(VertexAttrib3dARB, index, D(x), D(y), D(z));
}

GLAPI void APIENTRY glVertexAttrib3dNV( GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(VertexAttrib3dNV, index, D(x), D(y), D(z));
}

GLAPI void APIENTRY glVertexAttrib3dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib3dv, index, v);
}

GLAPI void APIENTRY glVertexAttrib3dvARB( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib3dvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib3dvNV( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib3dvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib3f( GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    CALL_4(VertexAttrib3f, index, F(x), F(y), F(z));
}

GLAPI void APIENTRY glVertexAttrib3fARB( GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    CALL_4(VertexAttrib3fARB, index, F(x), F(y), F(z));
}

GLAPI void APIENTRY glVertexAttrib3fNV( GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    CALL_4(VertexAttrib3fNV, index, F(x), F(y), F(z));
}

GLAPI void APIENTRY glVertexAttrib3fv( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib3fv, index, v);
}

GLAPI void APIENTRY glVertexAttrib3fvARB( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib3fvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib3fvNV( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib3fvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib3hNV( GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z) {
    CALL_4(VertexAttrib3hNV, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttrib3hvNV( GLuint index, const GLhalfNV* v) {
    CALL_2(VertexAttrib3hvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib3s( GLuint index, GLshort x, GLshort y, GLshort z) {
    CALL_4(VertexAttrib3s, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttrib3sARB( GLuint index, GLshort x, GLshort y, GLshort z) {
    CALL_4(VertexAttrib3sARB, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttrib3sNV( GLuint index, GLshort x, GLshort y, GLshort z) {
    CALL_4(VertexAttrib3sNV, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttrib3sv( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib3sv, index, v);
}

GLAPI void APIENTRY glVertexAttrib3svARB( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib3svARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib3svNV( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib3svNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib4Nbv( GLuint index, const GLbyte* v) {
    CALL_2(VertexAttrib4Nbv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4NbvARB( GLuint index, const GLbyte* v) {
    CALL_2(VertexAttrib4NbvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4Niv( GLuint index, const GLint* v) {
    CALL_2(VertexAttrib4Niv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4NivARB( GLuint index, const GLint* v) {
    CALL_2(VertexAttrib4NivARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4Nsv( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib4Nsv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4NsvARB( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib4NsvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4Nub( GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {
    CALL_5(VertexAttrib4Nub, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttrib4NubARB( GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {
    CALL_5(VertexAttrib4NubARB, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttrib4Nubv( GLuint index, const GLubyte* v) {
    CALL_2(VertexAttrib4Nubv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4NubvARB( GLuint index, const GLubyte* v) {
    CALL_2(VertexAttrib4NubvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4Nuiv( GLuint index, const GLuint* v) {
    CALL_2(VertexAttrib4Nuiv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4NuivARB( GLuint index, const GLuint* v) {
    CALL_2(VertexAttrib4NuivARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4Nusv( GLuint index, const GLushort* v) {
    CALL_2(VertexAttrib4Nusv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4NusvARB( GLuint index, const GLushort* v) {
    CALL_2(VertexAttrib4NusvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4bv( GLuint index, const GLbyte* v) {
    CALL_2(VertexAttrib4bv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4bvARB( GLuint index, const GLbyte* v) {
    CALL_2(VertexAttrib4bvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4d( GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_5(VertexAttrib4d, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glVertexAttrib4dARB( GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_5(VertexAttrib4dARB, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glVertexAttrib4dNV( GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_5(VertexAttrib4dNV, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glVertexAttrib4dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib4dv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4dvARB( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib4dvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4dvNV( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttrib4dvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib4f( GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_5(VertexAttrib4f, index, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glVertexAttrib4fARB( GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_5(VertexAttrib4fARB, index, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glVertexAttrib4fNV( GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_5(VertexAttrib4fNV, index, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glVertexAttrib4fv( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib4fv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4fvARB( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib4fvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4fvNV( GLuint index, const GLfloat* v) {
    CALL_2(VertexAttrib4fvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib4hNV( GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w) {
    CALL_5(VertexAttrib4hNV, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttrib4hvNV( GLuint index, const GLhalfNV* v) {
    CALL_2(VertexAttrib4hvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib4iv( GLuint index, const GLint* v) {
    CALL_2(VertexAttrib4iv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4ivARB( GLuint index, const GLint* v) {
    CALL_2(VertexAttrib4ivARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4s( GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {
    CALL_5(VertexAttrib4s, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttrib4sARB( GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {
    CALL_5(VertexAttrib4sARB, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttrib4sNV( GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {
    CALL_5(VertexAttrib4sNV, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttrib4sv( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib4sv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4svARB( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib4svARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4svNV( GLuint index, const GLshort* v) {
    CALL_2(VertexAttrib4svNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib4ubNV( GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {
    CALL_5(VertexAttrib4ubNV, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttrib4ubv( GLuint index, const GLubyte* v) {
    CALL_2(VertexAttrib4ubv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4ubvARB( GLuint index, const GLubyte* v) {
    CALL_2(VertexAttrib4ubvARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4ubvNV( GLuint index, const GLubyte* v) {
    CALL_2(VertexAttrib4ubvNV, index, v);
}

GLAPI void APIENTRY glVertexAttrib4uiv( GLuint index, const GLuint* v) {
    CALL_2(VertexAttrib4uiv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4uivARB( GLuint index, const GLuint* v) {
    CALL_2(VertexAttrib4uivARB, index, v);
}

GLAPI void APIENTRY glVertexAttrib4usv( GLuint index, const GLushort* v) {
    CALL_2(VertexAttrib4usv, index, v);
}

GLAPI void APIENTRY glVertexAttrib4usvARB( GLuint index, const GLushort* v) {
    CALL_2(VertexAttrib4usvARB, index, v);
}

GLAPI void APIENTRY glVertexAttribArrayObjectATI( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset) {
    CALL_7(VertexAttribArrayObjectATI, index, size, type, normalized, stride, buffer, offset);
}

GLAPI void APIENTRY glVertexAttribBinding( GLuint attribindex, GLuint bindingindex) {
    CALL_2(VertexAttribBinding, attribindex, bindingindex);
}

GLAPI void APIENTRY glVertexAttribDivisor( GLuint index, GLuint divisor) {
    CALL_2(VertexAttribDivisor, index, divisor);
}

GLAPI void APIENTRY glVertexAttribDivisorARB( GLuint index, GLuint divisor) {
    CALL_2(VertexAttribDivisorARB, index, divisor);
}

GLAPI void APIENTRY glVertexAttribFormat( GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) {
    CALL_5(VertexAttribFormat, attribindex, size, type, normalized, relativeoffset);
}

GLAPI void APIENTRY glVertexAttribFormatNV( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride) {
    CALL_5(VertexAttribFormatNV, index, size, type, normalized, stride);
}

GLAPI void APIENTRY glVertexAttribI1i( GLuint index, GLint x) {
    CALL_2(VertexAttribI1i, index, x);
}

GLAPI void APIENTRY glVertexAttribI1iEXT( GLuint index, GLint x) {
    CALL_2(VertexAttribI1iEXT, index, x);
}

GLAPI void APIENTRY glVertexAttribI1iv( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI1iv, index, v);
}

GLAPI void APIENTRY glVertexAttribI1ivEXT( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI1ivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI1ui( GLuint index, GLuint x) {
    CALL_2(VertexAttribI1ui, index, x);
}

GLAPI void APIENTRY glVertexAttribI1uiEXT( GLuint index, GLuint x) {
    CALL_2(VertexAttribI1uiEXT, index, x);
}

GLAPI void APIENTRY glVertexAttribI1uiv( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI1uiv, index, v);
}

GLAPI void APIENTRY glVertexAttribI1uivEXT( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI1uivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI2i( GLuint index, GLint x, GLint y) {
    CALL_3(VertexAttribI2i, index, x, y);
}

GLAPI void APIENTRY glVertexAttribI2iEXT( GLuint index, GLint x, GLint y) {
    CALL_3(VertexAttribI2iEXT, index, x, y);
}

GLAPI void APIENTRY glVertexAttribI2iv( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI2iv, index, v);
}

GLAPI void APIENTRY glVertexAttribI2ivEXT( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI2ivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI2ui( GLuint index, GLuint x, GLuint y) {
    CALL_3(VertexAttribI2ui, index, x, y);
}

GLAPI void APIENTRY glVertexAttribI2uiEXT( GLuint index, GLuint x, GLuint y) {
    CALL_3(VertexAttribI2uiEXT, index, x, y);
}

GLAPI void APIENTRY glVertexAttribI2uiv( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI2uiv, index, v);
}

GLAPI void APIENTRY glVertexAttribI2uivEXT( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI2uivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI3i( GLuint index, GLint x, GLint y, GLint z) {
    CALL_4(VertexAttribI3i, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttribI3iEXT( GLuint index, GLint x, GLint y, GLint z) {
    CALL_4(VertexAttribI3iEXT, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttribI3iv( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI3iv, index, v);
}

GLAPI void APIENTRY glVertexAttribI3ivEXT( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI3ivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI3ui( GLuint index, GLuint x, GLuint y, GLuint z) {
    CALL_4(VertexAttribI3ui, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttribI3uiEXT( GLuint index, GLuint x, GLuint y, GLuint z) {
    CALL_4(VertexAttribI3uiEXT, index, x, y, z);
}

GLAPI void APIENTRY glVertexAttribI3uiv( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI3uiv, index, v);
}

GLAPI void APIENTRY glVertexAttribI3uivEXT( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI3uivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI4bv( GLuint index, const GLbyte* v) {
    CALL_2(VertexAttribI4bv, index, v);
}

GLAPI void APIENTRY glVertexAttribI4bvEXT( GLuint index, const GLbyte* v) {
    CALL_2(VertexAttribI4bvEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI4i( GLuint index, GLint x, GLint y, GLint z, GLint w) {
    CALL_5(VertexAttribI4i, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttribI4iEXT( GLuint index, GLint x, GLint y, GLint z, GLint w) {
    CALL_5(VertexAttribI4iEXT, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttribI4iv( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI4iv, index, v);
}

GLAPI void APIENTRY glVertexAttribI4ivEXT( GLuint index, const GLint* v) {
    CALL_2(VertexAttribI4ivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI4sv( GLuint index, const GLshort* v) {
    CALL_2(VertexAttribI4sv, index, v);
}

GLAPI void APIENTRY glVertexAttribI4svEXT( GLuint index, const GLshort* v) {
    CALL_2(VertexAttribI4svEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI4ubv( GLuint index, const GLubyte* v) {
    CALL_2(VertexAttribI4ubv, index, v);
}

GLAPI void APIENTRY glVertexAttribI4ubvEXT( GLuint index, const GLubyte* v) {
    CALL_2(VertexAttribI4ubvEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI4ui( GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    CALL_5(VertexAttribI4ui, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttribI4uiEXT( GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    CALL_5(VertexAttribI4uiEXT, index, x, y, z, w);
}

GLAPI void APIENTRY glVertexAttribI4uiv( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI4uiv, index, v);
}

GLAPI void APIENTRY glVertexAttribI4uivEXT( GLuint index, const GLuint* v) {
    CALL_2(VertexAttribI4uivEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribI4usv( GLuint index, const GLushort* v) {
    CALL_2(VertexAttribI4usv, index, v);
}

GLAPI void APIENTRY glVertexAttribI4usvEXT( GLuint index, const GLushort* v) {
    CALL_2(VertexAttribI4usvEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribIFormat( GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
    CALL_4(VertexAttribIFormat, attribindex, size, type, relativeoffset);
}

GLAPI void APIENTRY glVertexAttribIFormatNV( GLuint index, GLint size, GLenum type, GLsizei stride) {
    CALL_4(VertexAttribIFormatNV, index, size, type, stride);
}

GLAPI void APIENTRY glVertexAttribIPointer( GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_5(VertexAttribIPointer, index, size, type, stride, pointer);
}

GLAPI void APIENTRY glVertexAttribIPointerEXT( GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_5(VertexAttribIPointerEXT, index, size, type, stride, pointer);
}

GLAPI void APIENTRY glVertexAttribL1d( GLuint index, GLdouble x) {
    CALL_2(VertexAttribL1d, index, D(x));
}

GLAPI void APIENTRY glVertexAttribL1dEXT( GLuint index, GLdouble x) {
    CALL_2(VertexAttribL1dEXT, index, D(x));
}

GLAPI void APIENTRY glVertexAttribL1dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL1dv, index, v);
}

GLAPI void APIENTRY glVertexAttribL1dvEXT( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL1dvEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribL1i64NV( GLuint index, GLint64EXT x) {
    CALL_2(VertexAttribL1i64NV, index, LL(x));
}

GLAPI void APIENTRY glVertexAttribL1i64vNV( GLuint index, const GLint64EXT* v) {
    CALL_2(VertexAttribL1i64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribL1ui64ARB( GLuint index, GLuint64EXT x) {
    CALL_2(VertexAttribL1ui64ARB, index, LL(x));
}

GLAPI void APIENTRY glVertexAttribL1ui64NV( GLuint index, GLuint64EXT x) {
    CALL_2(VertexAttribL1ui64NV, index, LL(x));
}

GLAPI void APIENTRY glVertexAttribL1ui64vARB( GLuint index, const GLuint64EXT* v) {
    CALL_2(VertexAttribL1ui64vARB, index, v);
}

GLAPI void APIENTRY glVertexAttribL1ui64vNV( GLuint index, const GLuint64EXT* v) {
    CALL_2(VertexAttribL1ui64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribL2d( GLuint index, GLdouble x, GLdouble y) {
    CALL_3(VertexAttribL2d, index, D(x), D(y));
}

GLAPI void APIENTRY glVertexAttribL2dEXT( GLuint index, GLdouble x, GLdouble y) {
    CALL_3(VertexAttribL2dEXT, index, D(x), D(y));
}

GLAPI void APIENTRY glVertexAttribL2dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL2dv, index, v);
}

GLAPI void APIENTRY glVertexAttribL2dvEXT( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL2dvEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribL2i64NV( GLuint index, GLint64EXT x, GLint64EXT y) {
    CALL_3(VertexAttribL2i64NV, index, LL(x), LL(y));
}

GLAPI void APIENTRY glVertexAttribL2i64vNV( GLuint index, const GLint64EXT* v) {
    CALL_2(VertexAttribL2i64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribL2ui64NV( GLuint index, GLuint64EXT x, GLuint64EXT y) {
    CALL_3(VertexAttribL2ui64NV, index, LL(x), LL(y));
}

GLAPI void APIENTRY glVertexAttribL2ui64vNV( GLuint index, const GLuint64EXT* v) {
    CALL_2(VertexAttribL2ui64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribL3d( GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(VertexAttribL3d, index, D(x), D(y), D(z));
}

GLAPI void APIENTRY glVertexAttribL3dEXT( GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(VertexAttribL3dEXT, index, D(x), D(y), D(z));
}

GLAPI void APIENTRY glVertexAttribL3dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL3dv, index, v);
}

GLAPI void APIENTRY glVertexAttribL3dvEXT( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL3dvEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribL3i64NV( GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z) {
    CALL_4(VertexAttribL3i64NV, index, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glVertexAttribL3i64vNV( GLuint index, const GLint64EXT* v) {
    CALL_2(VertexAttribL3i64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribL3ui64NV( GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z) {
    CALL_4(VertexAttribL3ui64NV, index, LL(x), LL(y), LL(z));
}

GLAPI void APIENTRY glVertexAttribL3ui64vNV( GLuint index, const GLuint64EXT* v) {
    CALL_2(VertexAttribL3ui64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribL4d( GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_5(VertexAttribL4d, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glVertexAttribL4dEXT( GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_5(VertexAttribL4dEXT, index, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glVertexAttribL4dv( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL4dv, index, v);
}

GLAPI void APIENTRY glVertexAttribL4dvEXT( GLuint index, const GLdouble* v) {
    CALL_2(VertexAttribL4dvEXT, index, v);
}

GLAPI void APIENTRY glVertexAttribL4i64NV( GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w) {
    CALL_5(VertexAttribL4i64NV, index, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glVertexAttribL4i64vNV( GLuint index, const GLint64EXT* v) {
    CALL_2(VertexAttribL4i64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribL4ui64NV( GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w) {
    CALL_5(VertexAttribL4ui64NV, index, LL(x), LL(y), LL(z), LL(w));
}

GLAPI void APIENTRY glVertexAttribL4ui64vNV( GLuint index, const GLuint64EXT* v) {
    CALL_2(VertexAttribL4ui64vNV, index, v);
}

GLAPI void APIENTRY glVertexAttribLFormat( GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
    CALL_4(VertexAttribLFormat, attribindex, size, type, relativeoffset);
}

GLAPI void APIENTRY glVertexAttribLFormatNV( GLuint index, GLint size, GLenum type, GLsizei stride) {
    CALL_4(VertexAttribLFormatNV, index, size, type, stride);
}

GLAPI void APIENTRY glVertexAttribLPointer( GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_5(VertexAttribLPointer, index, size, type, stride, pointer);
}

GLAPI void APIENTRY glVertexAttribLPointerEXT( GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_5(VertexAttribLPointerEXT, index, size, type, stride, pointer);
}

GLAPI void APIENTRY glVertexAttribP1ui( GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    CALL_4(VertexAttribP1ui, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribP1uiv( GLuint index, GLenum type, GLboolean normalized, const GLuint* value) {
    CALL_4(VertexAttribP1uiv, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribP2ui( GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    CALL_4(VertexAttribP2ui, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribP2uiv( GLuint index, GLenum type, GLboolean normalized, const GLuint* value) {
    CALL_4(VertexAttribP2uiv, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribP3ui( GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    CALL_4(VertexAttribP3ui, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribP3uiv( GLuint index, GLenum type, GLboolean normalized, const GLuint* value) {
    CALL_4(VertexAttribP3uiv, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribP4ui( GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    CALL_4(VertexAttribP4ui, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribP4uiv( GLuint index, GLenum type, GLboolean normalized, const GLuint* value) {
    CALL_4(VertexAttribP4uiv, index, type, normalized, value);
}

GLAPI void APIENTRY glVertexAttribParameteriAMD( GLuint index, GLenum pname, GLint param) {
    CALL_3(VertexAttribParameteriAMD, index, pname, param);
}

GLAPI void APIENTRY glVertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
    CALL_6(VertexAttribPointer, index, size, type, normalized, stride, pointer);
}

GLAPI void APIENTRY glVertexAttribPointerARB( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
    CALL_6(VertexAttribPointerARB, index, size, type, normalized, stride, pointer);
}

GLAPI void APIENTRY glVertexAttribPointerNV( GLuint index, GLint fsize, GLenum type, GLsizei stride, const void* pointer) {
    CALL_5(VertexAttribPointerNV, index, fsize, type, stride, pointer);
}

GLAPI void APIENTRY glVertexAttribs1dvNV( GLuint index, GLsizei count, const GLdouble* v) {
    CALL_3(VertexAttribs1dvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs1fvNV( GLuint index, GLsizei count, const GLfloat* v) {
    CALL_3(VertexAttribs1fvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs1hvNV( GLuint index, GLsizei n, const GLhalfNV* v) {
    CALL_3(VertexAttribs1hvNV, index, n, v);
}

GLAPI void APIENTRY glVertexAttribs1svNV( GLuint index, GLsizei count, const GLshort* v) {
    CALL_3(VertexAttribs1svNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs2dvNV( GLuint index, GLsizei count, const GLdouble* v) {
    CALL_3(VertexAttribs2dvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs2fvNV( GLuint index, GLsizei count, const GLfloat* v) {
    CALL_3(VertexAttribs2fvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs2hvNV( GLuint index, GLsizei n, const GLhalfNV* v) {
    CALL_3(VertexAttribs2hvNV, index, n, v);
}

GLAPI void APIENTRY glVertexAttribs2svNV( GLuint index, GLsizei count, const GLshort* v) {
    CALL_3(VertexAttribs2svNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs3dvNV( GLuint index, GLsizei count, const GLdouble* v) {
    CALL_3(VertexAttribs3dvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs3fvNV( GLuint index, GLsizei count, const GLfloat* v) {
    CALL_3(VertexAttribs3fvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs3hvNV( GLuint index, GLsizei n, const GLhalfNV* v) {
    CALL_3(VertexAttribs3hvNV, index, n, v);
}

GLAPI void APIENTRY glVertexAttribs3svNV( GLuint index, GLsizei count, const GLshort* v) {
    CALL_3(VertexAttribs3svNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs4dvNV( GLuint index, GLsizei count, const GLdouble* v) {
    CALL_3(VertexAttribs4dvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs4fvNV( GLuint index, GLsizei count, const GLfloat* v) {
    CALL_3(VertexAttribs4fvNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs4hvNV( GLuint index, GLsizei n, const GLhalfNV* v) {
    CALL_3(VertexAttribs4hvNV, index, n, v);
}

GLAPI void APIENTRY glVertexAttribs4svNV( GLuint index, GLsizei count, const GLshort* v) {
    CALL_3(VertexAttribs4svNV, index, count, v);
}

GLAPI void APIENTRY glVertexAttribs4ubvNV( GLuint index, GLsizei count, const GLubyte* v) {
    CALL_3(VertexAttribs4ubvNV, index, count, v);
}

GLAPI void APIENTRY glVertexBindingDivisor( GLuint bindingindex, GLuint divisor) {
    CALL_2(VertexBindingDivisor, bindingindex, divisor);
}

GLAPI void APIENTRY glVertexBlendARB( GLint count) {
    CALL_1(VertexBlendARB, count);
}

GLAPI void APIENTRY glVertexBlendEnvfATI( GLenum pname, GLfloat param) {
    CALL_2(VertexBlendEnvfATI, pname, F(param));
}

GLAPI void APIENTRY glVertexBlendEnviATI( GLenum pname, GLint param) {
    CALL_2(VertexBlendEnviATI, pname, param);
}

GLAPI void APIENTRY glVertexFormatNV( GLint size, GLenum type, GLsizei stride) {
    CALL_3(VertexFormatNV, size, type, stride);
}

GLAPI void APIENTRY glVertexP2ui( GLenum type, GLuint value) {
    CALL_2(VertexP2ui, type, value);
}

GLAPI void APIENTRY glVertexP2uiv( GLenum type, const GLuint* value) {
    CALL_2(VertexP2uiv, type, value);
}

GLAPI void APIENTRY glVertexP3ui( GLenum type, GLuint value) {
    CALL_2(VertexP3ui, type, value);
}

GLAPI void APIENTRY glVertexP3uiv( GLenum type, const GLuint* value) {
    CALL_2(VertexP3uiv, type, value);
}

GLAPI void APIENTRY glVertexP4ui( GLenum type, GLuint value) {
    CALL_2(VertexP4ui, type, value);
}

GLAPI void APIENTRY glVertexP4uiv( GLenum type, const GLuint* value) {
    CALL_2(VertexP4uiv, type, value);
}

GLAPI void APIENTRY glVertexPointerEXT( GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer) {
    CALL_5(VertexPointerEXT, size, type, stride, count, pointer);
}

GLAPI void APIENTRY glVertexPointerListIBM( GLint size, GLenum type, GLint stride, const void** pointer, GLint ptrstride) {
    CALL_5(VertexPointerListIBM, size, type, stride, pointer, ptrstride);
}

GLAPI void APIENTRY glVertexPointervINTEL( GLint size, GLenum type, const void** pointer) {
    CALL_3(VertexPointervINTEL, size, type, pointer);
}

GLAPI void APIENTRY glVertexStream1dATI( GLenum stream, GLdouble x) {
    CALL_2(VertexStream1dATI, stream, D(x));
}

GLAPI void APIENTRY glVertexStream1dvATI( GLenum stream, const GLdouble* coords) {
    CALL_2(VertexStream1dvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream1fATI( GLenum stream, GLfloat x) {
    CALL_2(VertexStream1fATI, stream, F(x));
}

GLAPI void APIENTRY glVertexStream1fvATI( GLenum stream, const GLfloat* coords) {
    CALL_2(VertexStream1fvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream1iATI( GLenum stream, GLint x) {
    CALL_2(VertexStream1iATI, stream, x);
}

GLAPI void APIENTRY glVertexStream1ivATI( GLenum stream, const GLint* coords) {
    CALL_2(VertexStream1ivATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream1sATI( GLenum stream, GLshort x) {
    CALL_2(VertexStream1sATI, stream, x);
}

GLAPI void APIENTRY glVertexStream1svATI( GLenum stream, const GLshort* coords) {
    CALL_2(VertexStream1svATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream2dATI( GLenum stream, GLdouble x, GLdouble y) {
    CALL_3(VertexStream2dATI, stream, D(x), D(y));
}

GLAPI void APIENTRY glVertexStream2dvATI( GLenum stream, const GLdouble* coords) {
    CALL_2(VertexStream2dvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream2fATI( GLenum stream, GLfloat x, GLfloat y) {
    CALL_3(VertexStream2fATI, stream, F(x), F(y));
}

GLAPI void APIENTRY glVertexStream2fvATI( GLenum stream, const GLfloat* coords) {
    CALL_2(VertexStream2fvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream2iATI( GLenum stream, GLint x, GLint y) {
    CALL_3(VertexStream2iATI, stream, x, y);
}

GLAPI void APIENTRY glVertexStream2ivATI( GLenum stream, const GLint* coords) {
    CALL_2(VertexStream2ivATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream2sATI( GLenum stream, GLshort x, GLshort y) {
    CALL_3(VertexStream2sATI, stream, x, y);
}

GLAPI void APIENTRY glVertexStream2svATI( GLenum stream, const GLshort* coords) {
    CALL_2(VertexStream2svATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream3dATI( GLenum stream, GLdouble x, GLdouble y, GLdouble z) {
    CALL_4(VertexStream3dATI, stream, D(x), D(y), D(z));
}

GLAPI void APIENTRY glVertexStream3dvATI( GLenum stream, const GLdouble* coords) {
    CALL_2(VertexStream3dvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream3fATI( GLenum stream, GLfloat x, GLfloat y, GLfloat z) {
    CALL_4(VertexStream3fATI, stream, F(x), F(y), F(z));
}

GLAPI void APIENTRY glVertexStream3fvATI( GLenum stream, const GLfloat* coords) {
    CALL_2(VertexStream3fvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream3iATI( GLenum stream, GLint x, GLint y, GLint z) {
    CALL_4(VertexStream3iATI, stream, x, y, z);
}

GLAPI void APIENTRY glVertexStream3ivATI( GLenum stream, const GLint* coords) {
    CALL_2(VertexStream3ivATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream3sATI( GLenum stream, GLshort x, GLshort y, GLshort z) {
    CALL_4(VertexStream3sATI, stream, x, y, z);
}

GLAPI void APIENTRY glVertexStream3svATI( GLenum stream, const GLshort* coords) {
    CALL_2(VertexStream3svATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream4dATI( GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_5(VertexStream4dATI, stream, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glVertexStream4dvATI( GLenum stream, const GLdouble* coords) {
    CALL_2(VertexStream4dvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream4fATI( GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_5(VertexStream4fATI, stream, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glVertexStream4fvATI( GLenum stream, const GLfloat* coords) {
    CALL_2(VertexStream4fvATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream4iATI( GLenum stream, GLint x, GLint y, GLint z, GLint w) {
    CALL_5(VertexStream4iATI, stream, x, y, z, w);
}

GLAPI void APIENTRY glVertexStream4ivATI( GLenum stream, const GLint* coords) {
    CALL_2(VertexStream4ivATI, stream, coords);
}

GLAPI void APIENTRY glVertexStream4sATI( GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w) {
    CALL_5(VertexStream4sATI, stream, x, y, z, w);
}

GLAPI void APIENTRY glVertexStream4svATI( GLenum stream, const GLshort* coords) {
    CALL_2(VertexStream4svATI, stream, coords);
}

GLAPI void APIENTRY glVertexWeightPointerEXT( GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_4(VertexWeightPointerEXT, size, type, stride, pointer);
}

GLAPI void APIENTRY glVertexWeightfEXT( GLfloat weight) {
    CALL_1(VertexWeightfEXT, F(weight));
}

GLAPI void APIENTRY glVertexWeightfvEXT( const GLfloat* weight) {
    CALL_1(VertexWeightfvEXT, weight);
}

GLAPI void APIENTRY glVertexWeighthNV( GLhalfNV weight) {
    CALL_1(VertexWeighthNV, weight);
}

GLAPI void APIENTRY glVertexWeighthvNV( const GLhalfNV* weight) {
    CALL_1(VertexWeighthvNV, weight);
}

GLAPI GLenum APIENTRY glVideoCaptureNV( GLuint video_capture_slot, GLuint* sequence_num, GLuint64EXT* capture_time) {
    CALL_3_R(VideoCaptureNV, video_capture_slot, sequence_num, capture_time);
}

GLAPI void APIENTRY glVideoCaptureStreamParameterdvNV( GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble* params) {
    CALL_4(VideoCaptureStreamParameterdvNV, video_capture_slot, stream, pname, params);
}

GLAPI void APIENTRY glVideoCaptureStreamParameterfvNV( GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat* params) {
    CALL_4(VideoCaptureStreamParameterfvNV, video_capture_slot, stream, pname, params);
}

GLAPI void APIENTRY glVideoCaptureStreamParameterivNV( GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint* params) {
    CALL_4(VideoCaptureStreamParameterivNV, video_capture_slot, stream, pname, params);
}

GLAPI void APIENTRY glViewportArrayv( GLuint first, GLsizei count, const GLfloat* v) {
    CALL_3(ViewportArrayv, first, count, v);
}

GLAPI void APIENTRY glViewportIndexedf( GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
    CALL_5(ViewportIndexedf, index, F(x), F(y), F(w), F(h));
}

GLAPI void APIENTRY glViewportIndexedfv( GLuint index, const GLfloat* v) {
    CALL_2(ViewportIndexedfv, index, v);
}

GLAPI void APIENTRY glWaitSync( GLsync sync, GLbitfield flags, GLuint64 timeout) {
    CALL_3(WaitSync, sync, flags, LL(timeout));
}

GLAPI void APIENTRY glWeightPathsNV( GLuint resultPath, GLsizei numPaths, const GLuint* paths, const GLfloat* weights) {
    CALL_4(WeightPathsNV, resultPath, numPaths, paths, weights);
}

GLAPI void APIENTRY glWeightPointerARB( GLint size, GLenum type, GLsizei stride, const void* pointer) {
    CALL_4(WeightPointerARB, size, type, stride, pointer);
}

GLAPI void APIENTRY glWeightbvARB( GLint size, const GLbyte* weights) {
    CALL_2(WeightbvARB, size, weights);
}

GLAPI void APIENTRY glWeightdvARB( GLint size, const GLdouble* weights) {
    CALL_2(WeightdvARB, size, weights);
}

GLAPI void APIENTRY glWeightfvARB( GLint size, const GLfloat* weights) {
    CALL_2(WeightfvARB, size, weights);
}

GLAPI void APIENTRY glWeightivARB( GLint size, const GLint* weights) {
    CALL_2(WeightivARB, size, weights);
}

GLAPI void APIENTRY glWeightsvARB( GLint size, const GLshort* weights) {
    CALL_2(WeightsvARB, size, weights);
}

GLAPI void APIENTRY glWeightubvARB( GLint size, const GLubyte* weights) {
    CALL_2(WeightubvARB, size, weights);
}

GLAPI void APIENTRY glWeightuivARB( GLint size, const GLuint* weights) {
    CALL_2(WeightuivARB, size, weights);
}

GLAPI void APIENTRY glWeightusvARB( GLint size, const GLushort* weights) {
    CALL_2(WeightusvARB, size, weights);
}

GLAPI void APIENTRY glWindowPos2d( GLdouble x, GLdouble y) {
    CALL_2(WindowPos2d, D(x), D(y));
}

GLAPI void APIENTRY glWindowPos2dARB( GLdouble x, GLdouble y) {
    CALL_2(WindowPos2dARB, D(x), D(y));
}

GLAPI void APIENTRY glWindowPos2dMESA( GLdouble x, GLdouble y) {
    CALL_2(WindowPos2dMESA, D(x), D(y));
}

GLAPI void APIENTRY glWindowPos2dv( const GLdouble* v) {
    CALL_1(WindowPos2dv, v);
}

GLAPI void APIENTRY glWindowPos2dvARB( const GLdouble* v) {
    CALL_1(WindowPos2dvARB, v);
}

GLAPI void APIENTRY glWindowPos2dvMESA( const GLdouble* v) {
    CALL_1(WindowPos2dvMESA, v);
}

GLAPI void APIENTRY glWindowPos2f( GLfloat x, GLfloat y) {
    CALL_2(WindowPos2f, F(x), F(y));
}

GLAPI void APIENTRY glWindowPos2fARB( GLfloat x, GLfloat y) {
    CALL_2(WindowPos2fARB, F(x), F(y));
}

GLAPI void APIENTRY glWindowPos2fMESA( GLfloat x, GLfloat y) {
    CALL_2(WindowPos2fMESA, F(x), F(y));
}

GLAPI void APIENTRY glWindowPos2fv( const GLfloat* v) {
    CALL_1(WindowPos2fv, v);
}

GLAPI void APIENTRY glWindowPos2fvARB( const GLfloat* v) {
    CALL_1(WindowPos2fvARB, v);
}

GLAPI void APIENTRY glWindowPos2fvMESA( const GLfloat* v) {
    CALL_1(WindowPos2fvMESA, v);
}

GLAPI void APIENTRY glWindowPos2i( GLint x, GLint y) {
    CALL_2(WindowPos2i, x, y);
}

GLAPI void APIENTRY glWindowPos2iARB( GLint x, GLint y) {
    CALL_2(WindowPos2iARB, x, y);
}

GLAPI void APIENTRY glWindowPos2iMESA( GLint x, GLint y) {
    CALL_2(WindowPos2iMESA, x, y);
}

GLAPI void APIENTRY glWindowPos2iv( const GLint* v) {
    CALL_1(WindowPos2iv, v);
}

GLAPI void APIENTRY glWindowPos2ivARB( const GLint* v) {
    CALL_1(WindowPos2ivARB, v);
}

GLAPI void APIENTRY glWindowPos2ivMESA( const GLint* v) {
    CALL_1(WindowPos2ivMESA, v);
}

GLAPI void APIENTRY glWindowPos2s( GLshort x, GLshort y) {
    CALL_2(WindowPos2s, x, y);
}

GLAPI void APIENTRY glWindowPos2sARB( GLshort x, GLshort y) {
    CALL_2(WindowPos2sARB, x, y);
}

GLAPI void APIENTRY glWindowPos2sMESA( GLshort x, GLshort y) {
    CALL_2(WindowPos2sMESA, x, y);
}

GLAPI void APIENTRY glWindowPos2sv( const GLshort* v) {
    CALL_1(WindowPos2sv, v);
}

GLAPI void APIENTRY glWindowPos2svARB( const GLshort* v) {
    CALL_1(WindowPos2svARB, v);
}

GLAPI void APIENTRY glWindowPos2svMESA( const GLshort* v) {
    CALL_1(WindowPos2svMESA, v);
}

GLAPI void APIENTRY glWindowPos3d( GLdouble x, GLdouble y, GLdouble z) {
    CALL_3(WindowPos3d, D(x), D(y), D(z));
}

GLAPI void APIENTRY glWindowPos3dARB( GLdouble x, GLdouble y, GLdouble z) {
    CALL_3(WindowPos3dARB, D(x), D(y), D(z));
}

GLAPI void APIENTRY glWindowPos3dMESA( GLdouble x, GLdouble y, GLdouble z) {
    CALL_3(WindowPos3dMESA, D(x), D(y), D(z));
}

GLAPI void APIENTRY glWindowPos3dv( const GLdouble* v) {
    CALL_1(WindowPos3dv, v);
}

GLAPI void APIENTRY glWindowPos3dvARB( const GLdouble* v) {
    CALL_1(WindowPos3dvARB, v);
}

GLAPI void APIENTRY glWindowPos3dvMESA( const GLdouble* v) {
    CALL_1(WindowPos3dvMESA, v);
}

GLAPI void APIENTRY glWindowPos3f( GLfloat x, GLfloat y, GLfloat z) {
    CALL_3(WindowPos3f, F(x), F(y), F(z));
}

GLAPI void APIENTRY glWindowPos3fARB( GLfloat x, GLfloat y, GLfloat z) {
    CALL_3(WindowPos3fARB, F(x), F(y), F(z));
}

GLAPI void APIENTRY glWindowPos3fMESA( GLfloat x, GLfloat y, GLfloat z) {
    CALL_3(WindowPos3fMESA, F(x), F(y), F(z));
}

GLAPI void APIENTRY glWindowPos3fv( const GLfloat* v) {
    CALL_1(WindowPos3fv, v);
}

GLAPI void APIENTRY glWindowPos3fvARB( const GLfloat* v) {
    CALL_1(WindowPos3fvARB, v);
}

GLAPI void APIENTRY glWindowPos3fvMESA( const GLfloat* v) {
    CALL_1(WindowPos3fvMESA, v);
}

GLAPI void APIENTRY glWindowPos3i( GLint x, GLint y, GLint z) {
    CALL_3(WindowPos3i, x, y, z);
}

GLAPI void APIENTRY glWindowPos3iARB( GLint x, GLint y, GLint z) {
    CALL_3(WindowPos3iARB, x, y, z);
}

GLAPI void APIENTRY glWindowPos3iMESA( GLint x, GLint y, GLint z) {
    CALL_3(WindowPos3iMESA, x, y, z);
}

GLAPI void APIENTRY glWindowPos3iv( const GLint* v) {
    CALL_1(WindowPos3iv, v);
}

GLAPI void APIENTRY glWindowPos3ivARB( const GLint* v) {
    CALL_1(WindowPos3ivARB, v);
}

GLAPI void APIENTRY glWindowPos3ivMESA( const GLint* v) {
    CALL_1(WindowPos3ivMESA, v);
}

GLAPI void APIENTRY glWindowPos3s( GLshort x, GLshort y, GLshort z) {
    CALL_3(WindowPos3s, x, y, z);
}

GLAPI void APIENTRY glWindowPos3sARB( GLshort x, GLshort y, GLshort z) {
    CALL_3(WindowPos3sARB, x, y, z);
}

GLAPI void APIENTRY glWindowPos3sMESA( GLshort x, GLshort y, GLshort z) {
    CALL_3(WindowPos3sMESA, x, y, z);
}

GLAPI void APIENTRY glWindowPos3sv( const GLshort* v) {
    CALL_1(WindowPos3sv, v);
}

GLAPI void APIENTRY glWindowPos3svARB( const GLshort* v) {
    CALL_1(WindowPos3svARB, v);
}

GLAPI void APIENTRY glWindowPos3svMESA( const GLshort* v) {
    CALL_1(WindowPos3svMESA, v);
}

GLAPI void APIENTRY glWindowPos4dMESA( GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    CALL_4(WindowPos4dMESA, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glWindowPos4dvMESA( const GLdouble* v) {
    CALL_1(WindowPos4dvMESA, v);
}

GLAPI void APIENTRY glWindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    CALL_4(WindowPos4fMESA, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glWindowPos4fvMESA( const GLfloat* v) {
    CALL_1(WindowPos4fvMESA, v);
}

GLAPI void APIENTRY glWindowPos4iMESA( GLint x, GLint y, GLint z, GLint w) {
    CALL_4(WindowPos4iMESA, x, y, z, w);
}

GLAPI void APIENTRY glWindowPos4ivMESA( const GLint* v) {
    CALL_1(WindowPos4ivMESA, v);
}

GLAPI void APIENTRY glWindowPos4sMESA( GLshort x, GLshort y, GLshort z, GLshort w) {
    CALL_4(WindowPos4sMESA, x, y, z, w);
}

GLAPI void APIENTRY glWindowPos4svMESA( const GLshort* v) {
    CALL_1(WindowPos4svMESA, v);
}

GLAPI void APIENTRY glWriteMaskEXT( GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW) {
    CALL_6(WriteMaskEXT, res, in, outX, outY, outZ, outW);
}

