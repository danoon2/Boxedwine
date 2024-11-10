#include "gldef.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#ifndef GLAPI
#define GLAPI
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

// float parameter
#define F(f) f
// double parameter
#define D(d) &d
// pointer parameter
#define P(p) p

#define LL(l) &l

#define CALL_0_R(index) __asm__("push %0\n\tint $0x99\n\taddl $4, %%esp"::"i"(index):"%eax"); 
#define CALL_1_R(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x99\n\taddl $8, %%esp"::"i"(index), "g"(arg1):"%eax"); 
#define CALL_2_R(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2):"%eax"); 
#define CALL_3_R(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3):"%eax"); 
#define CALL_4_R(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4):"%eax"); 
#define CALL_5_R(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5):"%eax");
#define CALL_6_R(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6):"%eax");
#define CALL_7_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7):"%eax");
#define CALL_8_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8):"%eax");
#define CALL_9_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $40, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9):"%eax");
#define CALL_10_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $44, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10):"%eax");
#define CALL_11_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) __asm__("push %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $48, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11):"%eax");

#define CALL_0(index) __asm__("push %0\n\tint $0x99\n\taddl $4, %%esp"::"i"(index)); 
#define CALL_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x99\n\taddl $8, %%esp"::"i"(index), "g"(arg1)); 
#define CALL_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2)); 
#define CALL_3(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3)); 
#define CALL_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4)); 
#define CALL_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5));
#define CALL_6(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6));
#define CALL_7(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7));
#define CALL_8(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8));
#define CALL_9(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $40, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9));
#define CALL_10(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $44, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10));
#define CALL_11(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) __asm__("push %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $48, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11));
#define CALL_12(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) __asm__("push %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $52, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12));
#define CALL_13(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13) __asm__("push %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $56, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13));
#define CALL_14(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14) __asm__("push %14\n\tpush %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $60, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13), "g"(arg14));
#define CALL_15(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) __asm__("push %15\n\tpush %14\n\tpush %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $64, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13), "g"(arg14), "g"(arg15));

/* Miscellaneous */
GLAPI void APIENTRY glClearIndex( GLfloat c ) {
	CALL_1(ClearIndex, F(c));
}

GLAPI void APIENTRY glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {
	CALL_4(ClearColor, F(red), F(green), F(blue), F(alpha));
}

GLAPI void APIENTRY glClear( GLbitfield mask ) {
	CALL_1(Clear, mask);
}

GLAPI void APIENTRY glIndexMask( GLuint mask ) {
	CALL_1(IndexMask, mask);
}

GLAPI void APIENTRY glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) {
	CALL_4(ColorMask, red, green, blue, alpha);
}

GLAPI void APIENTRY glAlphaFunc( GLenum func, GLclampf ref ) {
	CALL_2(AlphaFunc, func, F(ref));
}

GLAPI void APIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor ) {
	CALL_2(BlendFunc, sfactor, dfactor);
}

GLAPI void APIENTRY glLogicOp( GLenum opcode ) {
	CALL_1(LogicOp, opcode);
}

GLAPI void APIENTRY glCullFace( GLenum mode ) {
	CALL_1(CullFace, mode);
}

GLAPI void APIENTRY glFrontFace( GLenum mode ) {
	CALL_1(FrontFace, mode);
}

GLAPI void APIENTRY glPointSize( GLfloat size ) {
	CALL_1(PointSize, F(size));
}

GLAPI void APIENTRY glLineWidth( GLfloat width ) {
	CALL_1(LineWidth, F(width));
}

GLAPI void APIENTRY glLineStipple( GLint factor, GLushort pattern ) {
	CALL_2(LineStipple, factor, pattern);
}

GLAPI void APIENTRY glPolygonMode( GLenum face, GLenum mode ) {
	CALL_2(PolygonMode, face, mode);
}

GLAPI void APIENTRY glPolygonOffset( GLfloat factor, GLfloat units ) {
	CALL_2(PolygonOffset, F(factor), F(units));
}

GLAPI void APIENTRY glPolygonStipple( const GLubyte *mask ) {
	CALL_1(PolygonStipple, P(mask));
}

GLAPI void APIENTRY glGetPolygonStipple( GLubyte *mask ) {
	CALL_1(GetPolygonStipple, P(mask));
}

GLAPI void APIENTRY glEdgeFlag( GLboolean flag ) {
	CALL_1(EdgeFlag, flag);
}

GLAPI void APIENTRY glEdgeFlagv( const GLboolean *flag ) {
	CALL_1(EdgeFlagv, P(flag));
}

GLAPI void APIENTRY glScissor( GLint x, GLint y, GLsizei width, GLsizei height) {
	CALL_4(Scissor, x, y, width, height);
}

GLAPI void APIENTRY glClipPlane( GLenum plane, const GLdouble *equation ) {
	CALL_2(ClipPlane, plane, P(equation));
}

GLAPI void APIENTRY glGetClipPlane( GLenum plane, GLdouble *equation ) {
	CALL_2(GetClipPlane, plane, P(equation));
}

GLAPI void APIENTRY glDrawBuffer( GLenum mode ) {
	CALL_1(DrawBuffer, mode);
}

GLAPI void APIENTRY glReadBuffer( GLenum mode ) {
	CALL_1(ReadBuffer, mode);
}

GLAPI void APIENTRY glEnable( GLenum cap ) {
	CALL_1(Enable, cap);
}

GLAPI void APIENTRY glDisable( GLenum cap ) {
	CALL_1(Disable, cap);
}

GLAPI GLboolean APIENTRY glIsEnabled( GLenum cap ) {
	CALL_1_R(IsEnabled, cap);
}

GLAPI void APIENTRY glEnableClientState( GLenum cap ) {  /* 1.1 */
	CALL_1(EnableClientState, cap);
}

GLAPI void APIENTRY glDisableClientState( GLenum cap ) {  /* 1.1 */
	CALL_1(DisableClientState, cap);
}

GLAPI void APIENTRY glGetBooleanv( GLenum pname, GLboolean *params ) {
	CALL_2(GetBooleanv, pname, P(params));
}

GLAPI void APIENTRY glGetDoublev( GLenum pname, GLdouble *params ) {
	CALL_2(GetDoublev, pname, P(params));
}

GLAPI void APIENTRY glGetFloatv( GLenum pname, GLfloat *params ) {
	CALL_2(GetFloatv, pname, P(params));
}

GLAPI void APIENTRY glGetIntegerv( GLenum pname, GLint *params ) {
	CALL_2(GetIntegerv, pname, P(params));
}

GLAPI void APIENTRY glPushAttrib( GLbitfield mask ) {
	CALL_1(PushAttrib, mask);
}

GLAPI void APIENTRY glPopAttrib( void ) {
	CALL_0(PopAttrib);
}

GLAPI void APIENTRY glPushClientAttrib( GLbitfield mask ) {  /* 1.1 */
	CALL_1(PushClientAttrib, mask);
}

GLAPI void APIENTRY glPopClientAttrib( void ) {  /* 1.1 */
	CALL_0(PopClientAttrib);
}

GLAPI GLint APIENTRY glRenderMode( GLenum mode ) {
	CALL_1_R(RenderMode, mode);
}

GLAPI GLenum APIENTRY glGetError( void ) {
	CALL_0(GetError);
}

GLAPI const GLubyte* APIENTRY glGetString( GLenum name ) {
	CALL_1_R(GetString, name);
}

GLAPI void APIENTRY glFinish( void ) {
	CALL_0(Finish);
}

GLAPI void APIENTRY glFlush( void ) {
	CALL_0(Flush);
}

GLAPI void APIENTRY glHint( GLenum target, GLenum mode ) {
	CALL_2(Hint, target, mode);
}

/* Depth Buffer */
GLAPI void APIENTRY glClearDepth( GLclampd depth ) {
	CALL_1(ClearDepth, D(depth));
}

GLAPI void APIENTRY glDepthFunc( GLenum func ) {
	CALL_1(DepthFunc, func);
}

GLAPI void APIENTRY glDepthMask( GLboolean flag ) {
	CALL_1(DepthMask, flag);
}

GLAPI void APIENTRY glDepthRange( GLclampd near_val, GLclampd far_val ) {
	CALL_2(DepthRange, D(near_val), D(far_val));
}


/* Accumulation Buffer */
GLAPI void APIENTRY glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) {
	CALL_4(ClearAccum, F(red), F(green), F(blue), F(alpha));
}

GLAPI void APIENTRY glAccum( GLenum op, GLfloat value ) {
	CALL_2(Accum, op, F(value));
}

/* Transformation */
GLAPI void APIENTRY glMatrixMode( GLenum mode ) {
	CALL_1(MatrixMode, mode);
}

GLAPI void APIENTRY glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ) {
	CALL_6(Ortho, D(left), D(right), D(bottom), D(top), D(near_val), D(far_val));
}

GLAPI void APIENTRY glFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ) {
	CALL_6(Frustum, D(left), D(right), D(bottom), D(top), D(near_val), D(far_val));	
}

GLAPI void APIENTRY glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {
	CALL_4(Viewport, x, y, width, height);
}

GLAPI void APIENTRY glPushMatrix( void ) {
	CALL_0(PushMatrix);
}

GLAPI void APIENTRY glPopMatrix( void ) {
	CALL_0(PopMatrix);
}

GLAPI void APIENTRY glLoadIdentity( void ) {
	CALL_0(LoadIdentity);
}

GLAPI void APIENTRY glLoadMatrixd( const GLdouble *m ) {
	CALL_1(LoadMatrixd, P(m));
}

GLAPI void APIENTRY glLoadMatrixf( const GLfloat *m ) {
	CALL_1(LoadMatrixf, P(m));
}

GLAPI void APIENTRY glMultMatrixd( const GLdouble *m ) {
	CALL_1(MultMatrixd, P(m));
}

GLAPI void APIENTRY glMultMatrixf( const GLfloat *m ) {
	CALL_1(MultMatrixf, P(m));
}

GLAPI void APIENTRY glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
	CALL_4(Rotated, D(angle), D(x), D(y), D(z));
}

GLAPI void APIENTRY glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {
	CALL_4(Rotatef, F(angle), F(x), F(y), F(z));
}

GLAPI void APIENTRY glScaled( GLdouble x, GLdouble y, GLdouble z ) {
	CALL_3(Scaled, D(x), D(y), D(z));
}

GLAPI void APIENTRY glScalef( GLfloat x, GLfloat y, GLfloat z ) {
	CALL_3(Scalef, F(x), F(y), F(z));
}

GLAPI void APIENTRY glTranslated( GLdouble x, GLdouble y, GLdouble z ) {
	CALL_3(Translated, D(x), D(y), D(z));
}

GLAPI void APIENTRY glTranslatef( GLfloat x, GLfloat y, GLfloat z ) {
	CALL_3(Translatef, F(x), F(y), F(z));
}

/* Display Lists */
GLAPI GLboolean APIENTRY glIsList( GLuint list ) {
	CALL_1_R(IsList, list);
}

GLAPI void APIENTRY glDeleteLists( GLuint list, GLsizei range ) {
	CALL_2(DeleteLists, list, range); 
}

GLAPI GLuint APIENTRY glGenLists( GLsizei range ) {
	CALL_1_R(GenLists, range);
}

GLAPI void APIENTRY glNewList( GLuint list, GLenum mode ) {
	CALL_2(NewList, list, mode);
}

GLAPI void APIENTRY glEndList( void ) {
	CALL_0(EndList);
}

GLAPI void APIENTRY glCallList( GLuint list ) {
	CALL_1(CallList, list);
}

GLAPI void APIENTRY glCallLists( GLsizei n, GLenum type, const GLvoid *lists ) {
	CALL_3(CallLists, n, type, P(lists));
}

GLAPI void APIENTRY glListBase( GLuint base ) {
	CALL_1(ListBase, base);
}

/* Drawing Functions */
GLAPI void APIENTRY glBegin( GLenum mode ) {
	CALL_1(Begin, mode);
}

GLAPI void APIENTRY glEnd( void ) {
	CALL_0(End);
}

GLAPI void APIENTRY glVertex2d( GLdouble x, GLdouble y ) {
	CALL_2(Vertex2d, D(x), D(y));
}

GLAPI void APIENTRY glVertex2f( GLfloat x, GLfloat y ) {
	CALL_2(Vertex2f, F(x), F(y));
}

GLAPI void APIENTRY glVertex2i( GLint x, GLint y ) {
	CALL_2(Vertex2i, x, y);
}

GLAPI void APIENTRY glVertex2s( GLshort x, GLshort y ) {
	CALL_2(Vertex2s, x, y);
}

GLAPI void APIENTRY glVertex3d( GLdouble x, GLdouble y, GLdouble z ) {
	CALL_3(Vertex3d, D(x), D(y), D(z));
}

GLAPI void APIENTRY glVertex3f( GLfloat x, GLfloat y, GLfloat z ) {
	CALL_3(Vertex3f, F(x), F(y), F(z));
}

GLAPI void APIENTRY glVertex3i( GLint x, GLint y, GLint z ) {
	CALL_3(Vertex3i, x, y, z);
}

GLAPI void APIENTRY glVertex3s( GLshort x, GLshort y, GLshort z ) {
	CALL_3(Vertex3s, x, y, z);
}

GLAPI void APIENTRY glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) {
	CALL_4(Vertex4d, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) {
	CALL_4(Vertex4f, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glVertex4i( GLint x, GLint y, GLint z, GLint w ) {
	CALL_4(Vertex4i, x, y, z, w);
}

GLAPI void APIENTRY glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w ) {
	CALL_4(Vertex4s, x, y, z, w);
}

GLAPI void APIENTRY glVertex2dv( const GLdouble *v ) {
	CALL_1(Vertex2dv, P(v));
}

GLAPI void APIENTRY glVertex2fv( const GLfloat *v ) {
	CALL_1(Vertex2fv, P(v));
}

GLAPI void APIENTRY glVertex2iv( const GLint *v ) {
	CALL_1(Vertex2iv, P(v));
}

GLAPI void APIENTRY glVertex2sv( const GLshort *v ) {
	CALL_0(Vertex2sv);
}

GLAPI void APIENTRY glVertex3dv( const GLdouble *v ) {
	CALL_1(Vertex3dv, P(v));
}

GLAPI void APIENTRY glVertex3fv( const GLfloat *v ) {
	CALL_1(Vertex3fv, P(v));
}

GLAPI void APIENTRY glVertex3iv( const GLint *v ) {
	CALL_1(Vertex3iv, P(v));
}

GLAPI void APIENTRY glVertex3sv( const GLshort *v ) {
	CALL_1(Vertex3sv, P(v));
}

GLAPI void APIENTRY glVertex4dv( const GLdouble *v ) {
	CALL_1(Vertex4dv, P(v));
}

GLAPI void APIENTRY glVertex4fv( const GLfloat *v ) {
	CALL_1(Vertex4fv, P(v));
}

GLAPI void APIENTRY glVertex4iv( const GLint *v ) {
	CALL_1(Vertex4iv, P(v));
}

GLAPI void APIENTRY glVertex4sv( const GLshort *v ) {
	CALL_1(Vertex4sv, P(v));
}

GLAPI void APIENTRY glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz ) {
	CALL_3(Normal3b, nx, ny, nz);
}

GLAPI void APIENTRY glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz ) {
	CALL_3(Normal3d, D(nx), D(ny), D(nz));
}

GLAPI void APIENTRY glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz ) {
	CALL_3(Normal3f, F(nx), F(ny), F(nz));
}

GLAPI void APIENTRY glNormal3i( GLint nx, GLint ny, GLint nz ) {
	CALL_3(Normal3i, nx, ny, nz);
}

GLAPI void APIENTRY glNormal3s( GLshort nx, GLshort ny, GLshort nz ) {
	CALL_3(Normal3s, nx, ny, nz);
}

GLAPI void APIENTRY glNormal3bv( const GLbyte *v ) {
	CALL_1(Normal3bv, P(v));
}

GLAPI void APIENTRY glNormal3dv( const GLdouble *v ) {
	CALL_1(Normal3dv, P(v));
}

GLAPI void APIENTRY glNormal3fv( const GLfloat *v ) {
	CALL_1(Normal3fv, P(v));
}

GLAPI void APIENTRY glNormal3iv( const GLint *v ) {
	CALL_1(Normal3iv, P(v));
}

GLAPI void APIENTRY glNormal3sv( const GLshort *v ) {
	CALL_1(Normal3sv, P(v));
}

GLAPI void APIENTRY glIndexd( GLdouble c ) {
	CALL_1(Indexd, D(c));
}

GLAPI void APIENTRY glIndexf( GLfloat c ) {
	CALL_1(Indexf, F(c));
}

GLAPI void APIENTRY glIndexi( GLint c ) {
	CALL_1(Indexi, c);
}

GLAPI void APIENTRY glIndexs( GLshort c ) {
	CALL_1(Indexs, c);
}

GLAPI void APIENTRY glIndexub( GLubyte c ) {  /* 1.1 */
	CALL_1(Indexub, c);
}

GLAPI void APIENTRY glIndexdv( const GLdouble *c ) {
	CALL_1(Indexdv, P(c));
}

GLAPI void APIENTRY glIndexfv( const GLfloat *c ) {
	CALL_1(Indexfv, P(c));
}

GLAPI void APIENTRY glIndexiv( const GLint *c ) {
	CALL_1(Indexiv, P(c));
}

GLAPI void APIENTRY glIndexsv( const GLshort *c ) {
	CALL_1(Indexsv, P(c));
}

GLAPI void APIENTRY glIndexubv( const GLubyte *c ) {  /* 1.1 */
	CALL_1(Indexubv, P(c));
}

GLAPI void APIENTRY glColor3b( GLbyte red, GLbyte green, GLbyte blue ) {
	CALL_3(Color3b, red, green, blue);
}

GLAPI void APIENTRY glColor3d( GLdouble red, GLdouble green, GLdouble blue ) {
	CALL_3(Color3d, D(red), D(green), D(blue));
}

GLAPI void APIENTRY glColor3f( GLfloat red, GLfloat green, GLfloat blue ) {
	CALL_3(Color3f, F(red), F(green), F(blue));
}

GLAPI void APIENTRY glColor3i( GLint red, GLint green, GLint blue ) {
	CALL_3(Color3i, red, green, blue);
}

GLAPI void APIENTRY glColor3s( GLshort red, GLshort green, GLshort blue ) {
	CALL_3(Color3s, red, green, blue);
}

GLAPI void APIENTRY glColor3ub( GLubyte red, GLubyte green, GLubyte blue ) {
	CALL_3(Color3ub, red, green, blue);
}

GLAPI void APIENTRY glColor3ui( GLuint red, GLuint green, GLuint blue ) {
	CALL_3(Color3ui, red, green, blue);
}

GLAPI void APIENTRY glColor3us( GLushort red, GLushort green, GLushort blue ) {
	CALL_3(Color3us, red, green, blue);
}

GLAPI void APIENTRY glColor4b( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ) {
	CALL_4(Color4b, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4d( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ) {
	CALL_4(Color4d, D(red), D(green), D(blue), D(alpha));
}

GLAPI void APIENTRY glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) {
	CALL_4(Color4f, F(red), F(green), F(blue), F(alpha));
}

GLAPI void APIENTRY glColor4i( GLint red, GLint green, GLint blue, GLint alpha ) {
	CALL_4(Color4i, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4s( GLshort red, GLshort green, GLshort blue, GLshort alpha ) {
	CALL_4(Color4s, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ) {
	CALL_4(Color4ub, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4ui( GLuint red, GLuint green, GLuint blue, GLuint alpha ) {
	CALL_4(Color4ui, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4us( GLushort red, GLushort green, GLushort blue, GLushort alpha ) {
	CALL_4(Color4us, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor3bv( const GLbyte *v ) {
	CALL_1(Color3bv, P(v));
}

GLAPI void APIENTRY glColor3dv( const GLdouble *v ) {
	CALL_1(Color3dv, P(v));
}

GLAPI void APIENTRY glColor3fv( const GLfloat *v ) {
	CALL_1(Color3fv, P(v));
}

GLAPI void APIENTRY glColor3iv( const GLint *v ) {
	CALL_1(Color3iv, P(v));
}

GLAPI void APIENTRY glColor3sv( const GLshort *v ) {
	CALL_1(Color3sv, P(v));
}

GLAPI void APIENTRY glColor3ubv( const GLubyte *v ) {
	CALL_1(Color3ubv, P(v));
}

GLAPI void APIENTRY glColor3uiv( const GLuint *v ) {
	CALL_1(Color3uiv, P(v));
}

GLAPI void APIENTRY glColor3usv( const GLushort *v ) {
	CALL_1(Color3usv, P(v));
}

GLAPI void APIENTRY glColor4bv( const GLbyte *v ) {
	CALL_1(Color4bv, P(v));
}

GLAPI void APIENTRY glColor4dv( const GLdouble *v ) {
	CALL_1(Color4dv, P(v));
}

GLAPI void APIENTRY glColor4fv( const GLfloat *v ) {
	CALL_1(Color4fv, P(v));
}

GLAPI void APIENTRY glColor4iv( const GLint *v ) {
	CALL_1(Color4iv, P(v));
}

GLAPI void APIENTRY glColor4sv( const GLshort *v ) {
	CALL_0(Color4sv);
}

GLAPI void APIENTRY glColor4ubv( const GLubyte *v ) {
	CALL_1(Color4ubv, P(v));
}

GLAPI void APIENTRY glColor4uiv( const GLuint *v ) {
	CALL_1(Color4uiv, P(v));
}

GLAPI void APIENTRY glColor4usv( const GLushort *v ) {
	CALL_1(Color4usv, P(v));
}

GLAPI void APIENTRY glTexCoord1d( GLdouble s ) {
	CALL_1(TexCoord1d, D(s));
}

GLAPI void APIENTRY glTexCoord1f( GLfloat s ) {
	CALL_1(TexCoord1f, F(s));
}

GLAPI void APIENTRY glTexCoord1i( GLint s ) {
	CALL_1(TexCoord1i, s);
}

GLAPI void APIENTRY glTexCoord1s( GLshort s ) {
	CALL_1(TexCoord1s, s);
}

GLAPI void APIENTRY glTexCoord2d( GLdouble s, GLdouble t ) {
	CALL_2(TexCoord2d, D(s), D(t));
}

GLAPI void APIENTRY glTexCoord2f( GLfloat s, GLfloat t ) {
	CALL_2(TexCoord2f, F(s), F(t));
}

GLAPI void APIENTRY glTexCoord2i( GLint s, GLint t ) {
	CALL_2(TexCoord2i, s, t);
}

GLAPI void APIENTRY glTexCoord2s( GLshort s, GLshort t ) {
	CALL_2(TexCoord2s, s, t);
}

GLAPI void APIENTRY glTexCoord3d( GLdouble s, GLdouble t, GLdouble r ) {
	CALL_3(TexCoord3d, D(s), D(t), D(r));
}

GLAPI void APIENTRY glTexCoord3f( GLfloat s, GLfloat t, GLfloat r ) {
	CALL_3(TexCoord3f, F(s), F(t), F(r));
}

GLAPI void APIENTRY glTexCoord3i( GLint s, GLint t, GLint r ) {
	CALL_3(TexCoord3i, s, t, r);
}

GLAPI void APIENTRY glTexCoord3s( GLshort s, GLshort t, GLshort r ) {
	CALL_3(TexCoord3s, s, t, r);
}

GLAPI void APIENTRY glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q ) {
	CALL_4(TexCoord4d, D(s), D(t), D(r), D(q));
}

GLAPI void APIENTRY glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q ) {
	CALL_4(TexCoord4f, F(s), F(t), F(r), F(q));
}

GLAPI void APIENTRY glTexCoord4i( GLint s, GLint t, GLint r, GLint q ) {
	CALL_4(TexCoord4i, s, t, r, q);
}

GLAPI void APIENTRY glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q ) {
	CALL_4(TexCoord4s, s, t, r, q);
}

GLAPI void APIENTRY glTexCoord1dv( const GLdouble *v ) {
	CALL_1(TexCoord1dv, P(v));
}

GLAPI void APIENTRY glTexCoord1fv( const GLfloat *v ) {
	CALL_1(TexCoord1fv, P(v));
}

GLAPI void APIENTRY glTexCoord1iv( const GLint *v ) {
	CALL_1(TexCoord1iv, P(v));
}

GLAPI void APIENTRY glTexCoord1sv( const GLshort *v ) {
	CALL_1(TexCoord1sv, P(v));
}

GLAPI void APIENTRY glTexCoord2dv( const GLdouble *v ) {
	CALL_1(TexCoord2dv, P(v));
}

GLAPI void APIENTRY glTexCoord2fv( const GLfloat *v ) {
	CALL_1(TexCoord2fv, P(v));
}

GLAPI void APIENTRY glTexCoord2iv( const GLint *v ) {
	CALL_1(TexCoord2iv, P(v));
}

GLAPI void APIENTRY glTexCoord2sv( const GLshort *v ) {
	CALL_1(TexCoord2sv, P(v));
}

GLAPI void APIENTRY glTexCoord3dv( const GLdouble *v ) {
	CALL_1(TexCoord3dv, P(v));
}

GLAPI void APIENTRY glTexCoord3fv( const GLfloat *v ) {
	CALL_1(TexCoord3fv, P(v));
}

GLAPI void APIENTRY glTexCoord3iv( const GLint *v ) {
	CALL_1(TexCoord3iv, P(v));
}

GLAPI void APIENTRY glTexCoord3sv( const GLshort *v ) {
	CALL_1(TexCoord3sv, P(v));
}

GLAPI void APIENTRY glTexCoord4dv( const GLdouble *v ) {
	CALL_1(TexCoord4dv, P(v));
}

GLAPI void APIENTRY glTexCoord4fv( const GLfloat *v ) {
	CALL_1(TexCoord4fv, P(v));
}

GLAPI void APIENTRY glTexCoord4iv( const GLint *v ) {
	CALL_1(TexCoord4iv, P(v));
}

GLAPI void APIENTRY glTexCoord4sv( const GLshort *v ) {
	CALL_1(TexCoord4sv, P(v));
}

GLAPI void APIENTRY glRasterPos2d( GLdouble x, GLdouble y ) {
	CALL_2(RasterPos2d, D(x), D(y));
}

GLAPI void APIENTRY glRasterPos2f( GLfloat x, GLfloat y ) {
	CALL_2(RasterPos2f, F(x), F(y));
}

GLAPI void APIENTRY glRasterPos2i( GLint x, GLint y ) {
	CALL_2(RasterPos2i, x, y);
}

GLAPI void APIENTRY glRasterPos2s( GLshort x, GLshort y ) {
	CALL_2(RasterPos2s, x, y);
}

GLAPI void APIENTRY glRasterPos3d( GLdouble x, GLdouble y, GLdouble z ) {
	CALL_3(RasterPos3d, D(x), D(y), D(z));
}

GLAPI void APIENTRY glRasterPos3f( GLfloat x, GLfloat y, GLfloat z ) {
	CALL_3(RasterPos3f, F(x), F(y), F(z));
}

GLAPI void APIENTRY glRasterPos3i( GLint x, GLint y, GLint z ) {
	CALL_3(RasterPos3i, x, y, z);
}

GLAPI void APIENTRY glRasterPos3s( GLshort x, GLshort y, GLshort z ) {
	CALL_3(RasterPos3s, x, y, z);
}

GLAPI void APIENTRY glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) {
	CALL_4(RasterPos4d, D(x), D(y), D(z), D(w));
}

GLAPI void APIENTRY glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) {
	CALL_4(RasterPos4f, F(x), F(y), F(z), F(w));
}

GLAPI void APIENTRY glRasterPos4i( GLint x, GLint y, GLint z, GLint w ) {
	CALL_4(RasterPos4i, x, y, z, w);
}

GLAPI void APIENTRY glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w ) {
	CALL_4(RasterPos4s, x, y, z, w);
}

GLAPI void APIENTRY glRasterPos2dv( const GLdouble *v ) {
	CALL_1(RasterPos2dv, P(v));
}

GLAPI void APIENTRY glRasterPos2fv( const GLfloat *v ) {
	CALL_1(RasterPos2fv, P(v));
}

GLAPI void APIENTRY glRasterPos2iv( const GLint *v ) {
	CALL_1(RasterPos2iv, P(v));
}

GLAPI void APIENTRY glRasterPos2sv( const GLshort *v ) {
	CALL_1(RasterPos2sv, P(v));
}

GLAPI void APIENTRY glRasterPos3dv( const GLdouble *v ) {
	CALL_1(RasterPos3dv, P(v));
}

GLAPI void APIENTRY glRasterPos3fv( const GLfloat *v ) {
	CALL_1(RasterPos3fv, P(v));
}

GLAPI void APIENTRY glRasterPos3iv( const GLint *v ) {
	CALL_1(RasterPos3iv, P(v));
}

GLAPI void APIENTRY glRasterPos3sv( const GLshort *v ) {
	CALL_1(RasterPos3sv, P(v));
}

GLAPI void APIENTRY glRasterPos4dv( const GLdouble *v ) {
	CALL_1(RasterPos4dv, P(v));
}

GLAPI void APIENTRY glRasterPos4fv( const GLfloat *v ) {
	CALL_1(RasterPos4fv, P(v));
}

GLAPI void APIENTRY glRasterPos4iv( const GLint *v ) {
	CALL_1(RasterPos4iv, P(v));
}

GLAPI void APIENTRY glRasterPos4sv( const GLshort *v ) {
	CALL_1(RasterPos4sv, P(v));
}

GLAPI void APIENTRY glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ) {
	CALL_4(Rectd, D(x1), D(y1), D(x2), D(y2));
}

GLAPI void APIENTRY glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ) {
	CALL_4(Rectf, F(x1), F(y1), F(x2), F(y2));
}

GLAPI void APIENTRY glRecti( GLint x1, GLint y1, GLint x2, GLint y2 ) {
	CALL_4(Recti, x1, y1, x2, y2);
}

GLAPI void APIENTRY glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 ) {
	CALL_4(Rects, x1, y1, x2, y2);
}

GLAPI void APIENTRY glRectdv( const GLdouble *v1, const GLdouble *v2 ) {
	CALL_2(Rectdv, P(v1), P(v2));
}

GLAPI void APIENTRY glRectfv( const GLfloat *v1, const GLfloat *v2 ) {
	CALL_2(Rectfv, P(v1), P(v2));
}

GLAPI void APIENTRY glRectiv( const GLint *v1, const GLint *v2 ) {
	CALL_2(Rectiv, P(v1), P(v2));
}

GLAPI void APIENTRY glRectsv( const GLshort *v1, const GLshort *v2 ) {
	CALL_2(Rectsv, P(v1), P(v2));
}

/* Lighting */
GLAPI void APIENTRY glShadeModel( GLenum mode ) {
	CALL_1(ShadeModel, mode);
}

GLAPI void APIENTRY glLightf( GLenum light, GLenum pname, GLfloat param ) {
	CALL_3(Lightf, light, pname, F(param));
}

GLAPI void APIENTRY glLighti( GLenum light, GLenum pname, GLint param ) {
	CALL_3(Lighti, light, pname, param);
}

GLAPI void APIENTRY glLightfv( GLenum light, GLenum pname, const GLfloat *params ) {
	CALL_3(Lightfv, light, pname, P(params));
}

GLAPI void APIENTRY glLightiv( GLenum light, GLenum pname, const GLint *params ) {
	CALL_3(Lightiv, light, pname, P(params));
}

GLAPI void APIENTRY glGetLightfv( GLenum light, GLenum pname, GLfloat *params ) {
	CALL_3(GetLightfv, light, pname, P(params));
}

GLAPI void APIENTRY glGetLightiv( GLenum light, GLenum pname, GLint *params ) {
	CALL_3(GetLightiv, light, pname, P(params));
}

GLAPI void APIENTRY glLightModelf( GLenum pname, GLfloat param ) {
	CALL_2(LightModelf, pname, F(param));
}

GLAPI void APIENTRY glLightModeli( GLenum pname, GLint param ) {
	CALL_2(LightModeli, pname, param);
}

GLAPI void APIENTRY glLightModelfv( GLenum pname, const GLfloat *params ) {
	CALL_2(LightModelfv, pname, P(params));
}

GLAPI void APIENTRY glLightModeliv( GLenum pname, const GLint *params ) {
	CALL_2(LightModeliv, pname, P(params));
}

GLAPI void APIENTRY glMaterialf( GLenum face, GLenum pname, GLfloat param ) {
	CALL_3(Materialf, face, pname, F(param));
}

GLAPI void APIENTRY glMateriali( GLenum face, GLenum pname, GLint param ) {
	CALL_3(Materiali, face, pname, param);
}

GLAPI void APIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params ) {
	CALL_3(Materialfv, face, pname, P(params));
}

GLAPI void APIENTRY glMaterialiv( GLenum face, GLenum pname, const GLint *params ) {
	CALL_3(Materialiv, face, pname, P(params));
}

GLAPI void APIENTRY glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params ) {
	CALL_3(GetMaterialfv, face, pname, P(params));
}

GLAPI void APIENTRY glGetMaterialiv( GLenum face, GLenum pname, GLint *params ) {
	CALL_3(GetMaterialiv, face, pname, P(params));
}

GLAPI void APIENTRY glColorMaterial( GLenum face, GLenum mode ) {
	CALL_2(ColorMaterial, face, mode);
}

/* Raster functions */
GLAPI void APIENTRY glPixelZoom( GLfloat xfactor, GLfloat yfactor ) {
	CALL_2(PixelZoom, F(xfactor), F(yfactor));
}

GLAPI void APIENTRY glPixelStoref( GLenum pname, GLfloat param ) {
	CALL_2(PixelStoref, pname, F(param));
}

GLAPI void APIENTRY glPixelStorei( GLenum pname, GLint param ) {
	CALL_2(PixelStorei, pname, param);
}

GLAPI void APIENTRY glPixelTransferf( GLenum pname, GLfloat param ) {
	CALL_2(PixelTransferf, pname, param);
}

GLAPI void APIENTRY glPixelTransferi( GLenum pname, GLint param ) {
	CALL_2(PixelTransferi, pname, param);
}

GLAPI void APIENTRY glPixelMapfv( GLenum map, GLint mapsize, const GLfloat *values ) {
	CALL_3(PixelMapfv, map, mapsize, P(values));
}

GLAPI void APIENTRY glPixelMapuiv( GLenum map, GLint mapsize, const GLuint *values ) {
	CALL_3(PixelMapuiv, map, mapsize, P(values));
}

GLAPI void APIENTRY glPixelMapusv( GLenum map, GLint mapsize, const GLushort *values ) {
	CALL_3(PixelMapusv, map, mapsize, P(values));
}

GLAPI void APIENTRY glGetPixelMapfv( GLenum map, GLfloat *values ) {
	CALL_2(GetPixelMapfv, map, P(values));
}

GLAPI void APIENTRY glGetPixelMapuiv( GLenum map, GLuint *values ) {
	CALL_2(GetPixelMapuiv, map, P(values));
}

GLAPI void APIENTRY glGetPixelMapusv( GLenum map, GLushort *values ) {
	CALL_2(GetPixelMapusv, map, P(values));
}

GLAPI void APIENTRY glBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ) {
	CALL_7(Bitmap, width, height, F(xorig), F(yorig), F(xmove), F(ymove), P(bitmap));
}

GLAPI void APIENTRY glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) {
	CALL_7(ReadPixels, x, y, width, height, format, type, P(pixels));
}

GLAPI void APIENTRY glDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) {
	CALL_5(DrawPixels, width, height, format, type, P(pixels));
}

GLAPI void APIENTRY glCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type ) {
	CALL_5(CopyPixels, x, y, width, height, type);
}

/* Stenciling */
GLAPI void APIENTRY glStencilFunc( GLenum func, GLint ref, GLuint mask ) {
	CALL_3(StencilFunc, func, ref, mask);
}

GLAPI void APIENTRY glStencilMask( GLuint mask ) {
	CALL_1(StencilMask, mask);
}

GLAPI void APIENTRY glStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) {
	CALL_3(StencilOp, fail, zfail, zpass);
}

GLAPI void APIENTRY glClearStencil( GLint s ) {
	CALL_1(ClearStencil, s);
}

/* Texture mapping */
GLAPI void APIENTRY glTexGend( GLenum coord, GLenum pname, GLdouble param ) {
	CALL_3(TexGend, coord, pname, D(param));
}

GLAPI void APIENTRY glTexGenf( GLenum coord, GLenum pname, GLfloat param ) {
	CALL_3(TexGenf, coord, pname, F(param));
}

GLAPI void APIENTRY glTexGeni( GLenum coord, GLenum pname, GLint param ) {
	CALL_3(TexGeni, coord, pname, param);
}

GLAPI void APIENTRY glTexGendv( GLenum coord, GLenum pname, const GLdouble *params ) {
	CALL_3(TexGendv, coord, pname, P(params));
}

GLAPI void APIENTRY glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params ) {
	CALL_3(TexGenfv, coord, pname, P(params));
}

GLAPI void APIENTRY glTexGeniv( GLenum coord, GLenum pname, const GLint *params ) {
	CALL_3(TexGeniv, coord, pname, P(params));
}

GLAPI void APIENTRY glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params ) {
	CALL_3(GetTexGendv, coord, pname, P(params));
}

GLAPI void APIENTRY glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params ) {
	CALL_3(GetTexGenfv, coord, pname, P(params));
}

GLAPI void APIENTRY glGetTexGeniv( GLenum coord, GLenum pname, GLint *params ) {
	CALL_3(GetTexGeniv, coord, pname, P(params));
}

GLAPI void APIENTRY glTexEnvf( GLenum target, GLenum pname, GLfloat param ) {
	CALL_3(TexEnvf, target, pname, F(param));
}

GLAPI void APIENTRY glTexEnvi( GLenum target, GLenum pname, GLint param ) {
	CALL_3(TexEnvi, target, pname, param);
}

GLAPI void APIENTRY glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params ) {
	CALL_3(TexEnvfv, target, pname, P(params));
}

GLAPI void APIENTRY glTexEnviv( GLenum target, GLenum pname, const GLint *params ) {
	CALL_3(TexEnviv, target, pname, P(params));
}

GLAPI void APIENTRY glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params ) {
	CALL_3(GetTexEnvfv, target, pname, P(params));
}

GLAPI void APIENTRY glGetTexEnviv( GLenum target, GLenum pname, GLint *params ) {
	CALL_3(GetTexEnviv, target, pname, P(params));
}

GLAPI void APIENTRY glTexParameterf( GLenum target, GLenum pname, GLfloat param ) {
	CALL_3(TexParameterf, target, pname, F(param));
}

GLAPI void APIENTRY glTexParameteri( GLenum target, GLenum pname, GLint param ) {
	CALL_3(TexParameteri, target, pname, param);
}

GLAPI void APIENTRY glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params ) {
	CALL_3(TexParameterfv, target, pname, P(params));
}

GLAPI void APIENTRY glTexParameteriv( GLenum target, GLenum pname, const GLint *params ) {
	CALL_3(TexParameteriv, target, pname, P(params));
}

GLAPI void APIENTRY glGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params) {
	CALL_3(GetTexParameterfv, target, pname, P(params));
}

GLAPI void APIENTRY glGetTexParameteriv( GLenum target, GLenum pname, GLint *params ) {
	CALL_3(GetTexParameteriv, target, pname, P(params));
}

GLAPI void APIENTRY glGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params ) {
	CALL_4(GetTexLevelParameterfv, target, level, pname, P(params));
}

GLAPI void APIENTRY glGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params ) {
	CALL_4(GetTexLevelParameteriv, target, level, pname, P(params));
}

GLAPI void APIENTRY glTexImage1D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {
	CALL_8(TexImage1D, target, level, internalFormat, width, border, format, type, P(pixels));
}

GLAPI void APIENTRY glTexImage2D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {
	CALL_9(TexImage2D, target, level, internalFormat, width, height, border, format, type, P(pixels));
}

GLAPI void APIENTRY glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ) {
	CALL_5(GetTexImage, target, level, format, type, P(pixels));
}

/* Evaluators */
GLAPI void APIENTRY glMap1d( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ) {
	CALL_6(Map1d, target, D(u1), D(u2), stride, order, P(points));
}

GLAPI void APIENTRY glMap1f( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ) {
	CALL_6(Map1f, target, F(u1), F(u2), stride, order, P(points));
}

GLAPI void APIENTRY glMap2d( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ) {
	CALL_10(Map2d, target, D(u1), D(u2), ustride, uorder, D(v1), D(v2), vstride, vorder, P(points));
}

GLAPI void APIENTRY glMap2f( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ) {
	CALL_10(Map2f, target, F(u1), F(u2), ustride, uorder, F(v1), F(v2), vstride, vorder, P(points));
}

GLAPI void APIENTRY glGetMapdv( GLenum target, GLenum query, GLdouble *v ) {
	CALL_3(GetMapdv, target, query, P(v));
}

GLAPI void APIENTRY glGetMapfv( GLenum target, GLenum query, GLfloat *v ) {
	CALL_3(GetMapfv, target, query, P(v));
}

GLAPI void APIENTRY glGetMapiv( GLenum target, GLenum query, GLint *v ) {
	CALL_3(GetMapiv, target, query, P(v));
}

GLAPI void APIENTRY glEvalCoord1d( GLdouble u ) {
	CALL_1(EvalCoord1d, D(u));
}

GLAPI void APIENTRY glEvalCoord1f( GLfloat u ) {
	CALL_1(EvalCoord1f, F(u));
}

GLAPI void APIENTRY glEvalCoord1dv( const GLdouble *u ) {
	CALL_1(EvalCoord1dv, P(u));
}

GLAPI void APIENTRY glEvalCoord1fv( const GLfloat *u ) {
	CALL_1(EvalCoord1fv, P(u));
}

GLAPI void APIENTRY glEvalCoord2d( GLdouble u, GLdouble v ) {
	CALL_2(EvalCoord2d, D(u), D(v));
}

GLAPI void APIENTRY glEvalCoord2f( GLfloat u, GLfloat v ) {
	CALL_2(EvalCoord2f, F(u), F(v));
}

GLAPI void APIENTRY glEvalCoord2dv( const GLdouble *u ) {
	CALL_1(EvalCoord2dv, P(u));
}

GLAPI void APIENTRY glEvalCoord2fv( const GLfloat *u ) {
	CALL_1(EvalCoord2fv, P(u));
}

GLAPI void APIENTRY glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 ) {
	CALL_3(MapGrid1d, un, D(u1), D(u2));
}

GLAPI void APIENTRY glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 ) {
	CALL_3(MapGrid1f, un, F(u1), F(u2));
}

GLAPI void APIENTRY glMapGrid2d( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ) {
	CALL_6(MapGrid2d, un, D(u1), D(u2), vn, D(v1), D(v2));
}

GLAPI void APIENTRY glMapGrid2f( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ) {
	CALL_6(MapGrid2f, un, F(u1), F(u2), vn, F(v1), F(v2));
}

GLAPI void APIENTRY glEvalPoint1( GLint i ) {
	CALL_1(EvalPoint1, i);
}

GLAPI void APIENTRY glEvalPoint2( GLint i, GLint j ) {
	CALL_2(EvalPoint2, i, j);
}

GLAPI void APIENTRY glEvalMesh1( GLenum mode, GLint i1, GLint i2 ) {
	CALL_3(EvalMesh1, mode, i1, i2);
}

GLAPI void APIENTRY glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ) {
	CALL_5(EvalMesh2, mode, i1, i2, j1, j2);
}

/* Fog */
GLAPI void APIENTRY glFogf( GLenum pname, GLfloat param ) {
	CALL_2(Fogf, pname, F(param));
}

GLAPI void APIENTRY glFogi( GLenum pname, GLint param ) {
	CALL_2(Fogi, pname, param);
}

GLAPI void APIENTRY glFogfv( GLenum pname, const GLfloat *params ) {
	CALL_2(Fogfv, pname, P(params));
}

GLAPI void APIENTRY glFogiv( GLenum pname, const GLint *params ) {
	CALL_2(Fogiv, pname, P(params));
}

/* Selection and Feedback */
GLAPI void APIENTRY glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer ) {
	CALL_3(FeedbackBuffer, size, type, P(buffer));
}

GLAPI void APIENTRY glPassThrough( GLfloat token ) {
	CALL_1(PassThrough, F(token));
}

GLAPI void APIENTRY glSelectBuffer( GLsizei size, GLuint *buffer ) {
	CALL_2(SelectBuffer, size, P(buffer));
}

GLAPI void APIENTRY glInitNames( void ) {
	CALL_0(InitNames);
}

GLAPI void APIENTRY glLoadName( GLuint name ) {
	CALL_1(LoadName, name);
}

GLAPI void APIENTRY glPushName( GLuint name ) {
	CALL_1(PushName, name);
}

GLAPI void APIENTRY glPopName( void ) {
	CALL_0(PopName);
}

/* 1.1 functions */
/* texture objects */
GLAPI void APIENTRY glGenTextures( GLsizei n, GLuint *textures ) {
	CALL_2(GenTextures, n, P(textures));
}

GLAPI void APIENTRY glDeleteTextures( GLsizei n, const GLuint *textures) {
	CALL_2(DeleteTextures, n, P(textures));
}

GLAPI void APIENTRY glBindTexture( GLenum target, GLuint texture ) {
	CALL_2(BindTexture, target, texture);
}

GLAPI void APIENTRY glPrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities ) {
	CALL_3(PrioritizeTextures, n, P(textures), P(priorities));
}

GLAPI GLboolean APIENTRY glAreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences ) {
	CALL_3(AreTexturesResident, n, P(textures), P(residences));
}

GLAPI GLboolean APIENTRY glIsTexture( GLuint texture ) {
	CALL_1(IsTexture, texture);
}

/* texture mapping */
GLAPI void APIENTRY glTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ) {
	CALL_7(TexSubImage1D, target, level, xoffset, width, format, type, P(pixels));
}

GLAPI void APIENTRY glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) {
	CALL_9(TexSubImage2D, target, level, xoffset, yoffset, width, height, format, type, P(pixels));
}

GLAPI void APIENTRY glCopyTexImage1D( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border ) {
	CALL_7(CopyTexImage1D, target, level, internalformat, x, y, width, border);
}

GLAPI void APIENTRY glCopyTexImage2D( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) {
	CALL_8(CopyTexImage2D, target, level, internalformat, x, y, width, height, border);
}

GLAPI void APIENTRY glCopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width ) {
	CALL_6(CopyTexSubImage1D, target, level, xoffset, x, y, width);
}

GLAPI void APIENTRY glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) {
	CALL_8(CopyTexSubImage2D, target, level, xoffset, yoffset, x, y, width, height);
}

/* vertex arrays */
GLAPI void APIENTRY glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
	CALL_4(VertexPointer, size, type, stride, P(ptr));
}

GLAPI void APIENTRY glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr ) {
	CALL_3(NormalPointer, type, stride, P(ptr));
}

GLAPI void APIENTRY glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
	CALL_4(ColorPointer, size, type, stride, P(ptr));
}

GLAPI void APIENTRY glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr ) {
	CALL_3(IndexPointer, type, stride, P(ptr));
}

GLAPI void APIENTRY glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
	CALL_4(TexCoordPointer, size, type, stride, P(ptr));
}

GLAPI void APIENTRY glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr ) {
	CALL_2(EdgeFlagPointer, stride, ptr);
}

GLAPI void APIENTRY glGetPointerv( GLenum pname, GLvoid **params ) {
	CALL_2(GetPointerv, pname, P(params));
}

GLAPI void APIENTRY glArrayElement( GLint i ) {
	CALL_1(ArrayElement, i);
}

GLAPI void APIENTRY glDrawArrays( GLenum mode, GLint first, GLsizei count ) {
	CALL_3(DrawArrays, mode, first, count);
}

GLAPI void APIENTRY glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) {
	CALL_4(DrawElements, mode, count, type, P(indices));
}

GLAPI void APIENTRY glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ) {
	CALL_3(InterleavedArrays, format, stride, P(pointer));
}

GLAPI void APIENTRY glSamplePass( GLenum pass ) {
	CALL_1(SamplePass, pass);
}

GLAPI const GLubyte * APIENTRY glGetStringi(GLenum name, GLuint index) {
    CALL_2_R(GetStringi, name, index);
}

#include "glfunctions.h"
#include "glxfunctions.h"