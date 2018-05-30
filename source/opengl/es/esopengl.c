/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef BOXEDWINE_ES
#include "SDL_opengles2.h"
#define GLclampd double
#define GLdouble double
#include "../glcommon.h"
#include "esdisplaylist.h"
#include "log.h"
#include "glshim.h"
#include "devfb.h"

struct DisplayList* activeList;

void es_glClearIndex( GLfloat c ) {
    kpanic("glClearIndex not implemeented");
}

#define COMPILE_OR_RUN_1(func, arg1, arg1Type)	\
if (activeList) {								\
    struct ListOp* op = allocListOp(activeList);\
    op->op = list_##func;						\
    op->a1.arg1Type = arg1;					\
} else {										\
    func(arg1);									\
}

#define COMPILE_OR_RUN_2(func, arg1, arg1Type, arg2, arg2Type) \
if (activeList) {								\
    struct ListOp* op = allocListOp(activeList);\
     op->op = list_##func;						\
     op->a1.arg1Type = arg1;					\
    op->a2.arg2Type = arg2;					\
} else {										\
    func(arg1, arg2);							\
}													

#define COMPILE_OR_RUN_3(func, arg1, arg1Type, arg2, arg2Type, arg3, arg3Type) \
if (activeList) {								\
    struct ListOp* op = allocListOp(activeList);\
    op->op = list_##func;						\
    op->a1.arg1Type = arg1;					\
    op->a2.arg2Type = arg2;					\
    op->a3.arg3Type = arg3;					\
} else {										\
    func(arg1, arg2, arg3);						\
}	

#define COMPILE_OR_RUN_4(func, arg1, arg1Type, arg2, arg2Type, arg3, arg3Type, arg4, arg4Type) \
if (activeList) {								\
    struct ListOp* op = allocListOp(activeList);\
    op->op = list_##func;						\
    op->a1.arg1Type = arg1;					\
    op->a2.arg2Type = arg2;					\
    op->a3.arg3Type = arg3;					\
    op->a4.arg4Type = arg4;					\
} else {										\
    func(arg1, arg2, arg3, arg4);				\
}	

void es_glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {
    COMPILE_OR_RUN_4(glClearColor, red, f, green, f, blue, f, alpha, f);
}

void es_glClear( GLbitfield mask ) {
    COMPILE_OR_RUN_1(glClear, mask, i);
}

void es_glIndexMask( GLuint mask ) {
    kpanic("glIndexMask not implemented");
}

void es_glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) {
    COMPILE_OR_RUN_4(glColorMask, red, i, green, i, blue, i, alpha, i);
}

void es_glAlphaFunc( GLenum func, GLclampf ref ) {
    kpanic("glAlphaFunc not implemented");
}

void es_glBlendFunc( GLenum sfactor, GLenum dfactor ) {
    COMPILE_OR_RUN_2(glBlendFunc, sfactor, i, dfactor, i);
}

void es_glLogicOp( GLenum opcode ) {
    kpanic("glLogicOp not implemented");
}

void es_glCullFace( GLenum mode ) {
    COMPILE_OR_RUN_1(glCullFace, mode, i);
}

void es_glFrontFace( GLenum mode ) {
    COMPILE_OR_RUN_1(glFrontFace, mode, i);
}

void es_glPointSize( GLfloat size ) {
    kpanic("glPointSize not implemented");
}

void es_glLineWidth( GLfloat width ) {
    COMPILE_OR_RUN_1(glLineWidth, width, f);
}

void es_glLineStipple( GLint factor, GLushort pattern ) {
    COMPILE_OR_RUN_2(glLineStipple, factor, i, pattern, i);
}

void es_glPolygonMode( GLenum face, GLenum mode ) {
    kpanic("glPolygonMode not implemented");
}

void es_glPolygonOffset( GLfloat factor, GLfloat units ) {
    COMPILE_OR_RUN_2(glPolygonOffset, factor, f, units, f);
}

void es_glPolygonStipple( const GLubyte *mask ) {
    kpanic("glPolygonStipple not implemented");
}

void es_glGetPolygonStipple( GLubyte *mask ) {
    kpanic("glGetPolygonStipple not implemented");
}

void es_glEdgeFlag( GLboolean flag ) {
    kpanic("glEdgeFlag not implemented");
}

void es_glEdgeFlagv( const GLboolean *flag ) {
    kpanic("glEdgeFlagv not implemented");
}

void es_glScissor( GLint x, GLint y, GLsizei width, GLsizei height) {
    COMPILE_OR_RUN_4(glScissor, x, i, y, i, width, i, height, i);
}

void es_glClipPlane( GLenum plane, const GLdouble *equation ) {
    kpanic("glClipPlane not implemented");
}

void es_glGetClipPlane( GLenum plane, GLdouble *equation ) {
    kpanic("glGetClipPlane not implemented");
}

void es_glDrawBuffer( GLenum mode ) {
    kpanic("glDrawBuffer not implemented");
}

void es_glReadBuffer( GLenum mode ) {
    kpanic("glReadBuffer not implemented");
}

void es_glEnable( GLenum cap ) {
    COMPILE_OR_RUN_1(shim_glEnable, cap, i);
}

void es_glDisable( GLenum cap ) {
    COMPILE_OR_RUN_1(shim_glDisable, cap, i);
}

GLboolean es_glIsEnabled( GLenum cap ) {
    return 0;
}

void es_glEnableClientState( GLenum cap ) {
}

void es_glDisableClientState( GLenum cap ) { 
}

void es_glGetBooleanv( GLenum pname, GLboolean *params ) {
}

void es_glGetDoublev( GLenum pname, GLdouble *params ) {
}

void es_glGetFloatv( GLenum pname, GLfloat *params ) {
}

void es_glGetIntegerv( GLenum pname, GLint *params ) {
}

void es_glPushAttrib( GLbitfield mask ) {
}

void es_glPopAttrib( void ) {
}

void es_glPushClientAttrib( GLbitfield mask ) {
}

void es_glPopClientAttrib( void ) {
}

GLint es_glRenderMode( GLenum mode ) {
    return 0;
}

GLenum es_glGetError( void ) {
    return 0;
}

const GLubyte* es_glGetString( GLenum name ) {
    if (name == GL_VERSION) {
        return (GLubyte*)"1.2";
    } else if (name == GL_EXTENSIONS) {
        return (GLubyte*)"";
    }
    printf("glGetString name=%d result=%s", name, glGetString(name));
    return glGetString(name);
}

void es_glHint( GLenum target, GLenum mode ) {
}

void es_glClearDepth( GLclampd depth ) {
    if (activeList) {
        struct ListOp* op = allocListOp(activeList);
        op->op = list_glClearDepth;
        op->a1.f = (float)depth;
    } else {
        glClearDepthf((float)depth);
    }
}

void es_glDepthFunc( GLenum func ) {
}

void es_glDepthMask( GLboolean flag ) {
}

void es_glDepthRange( GLclampd near_val, GLclampd far_val ) {
}

void es_glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) {
}

void es_glAccum( GLenum op, GLfloat value ) {
}

void es_glMatrixMode( GLenum mode ) {
}

void es_glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ) {
}

void es_glFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ) {
}

void es_glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {
}

void es_glPushMatrix( void ) {
}

void es_glPopMatrix( void ) {
}

void es_glLoadIdentity( void ) {
}

void es_glLoadMatrixd( const GLdouble *m ) {
}

void es_glLoadMatrixf( const GLfloat *m ) {
}

void es_glMultMatrixd( const GLdouble *m ) {
}

void es_glMultMatrixf( const GLfloat *m ) {
}

void es_glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
}

void es_glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {
}

void es_glScaled( GLdouble x, GLdouble y, GLdouble z ) {
}

void es_glScalef( GLfloat x, GLfloat y, GLfloat z ) {
}

void es_glTranslated( GLdouble x, GLdouble y, GLdouble z ) {
}

void es_glTranslatef( GLfloat x, GLfloat y, GLfloat z ) {
}

GLboolean es_glIsList( GLuint list ) {
    return 0;
}

void es_glDeleteLists( GLuint list, GLsizei range ) {
}

GLuint es_glGenLists( GLsizei range ) {
    return 0;
}

void es_glNewList( GLuint list, GLenum mode ) {
}

void es_glEndList( void ) {
}

void es_glCallList( GLuint list ) {
}

void es_glCallLists( GLsizei n, GLenum type, const GLvoid *lists ) {
}

void es_glListBase( GLuint base ) {
}

void es_glBegin( GLenum mode ) {
}

void es_glEnd( void ) {
}

void es_glVertex2d( GLdouble x, GLdouble y ) {
}

void es_glVertex2f( GLfloat x, GLfloat y ) {
}

void es_glVertex2i( GLint x, GLint y ) {
}

void es_glVertex2s( GLshort x, GLshort y ) {
}

void es_glVertex3d( GLdouble x, GLdouble y, GLdouble z ) {
}

void es_glVertex3f( GLfloat x, GLfloat y, GLfloat z ) {
}

void es_glVertex3i( GLint x, GLint y, GLint z ) {
}

void es_glVertex3s( GLshort x, GLshort y, GLshort z ) {
}

void es_glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) {
}

void es_glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) {
}

void es_glVertex4i( GLint x, GLint y, GLint z, GLint w ) {
}

void es_glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w ) {
}

void es_glVertex2dv( const GLdouble *v ) {
}

void es_glVertex2fv( const GLfloat *v ) {
}

void es_glVertex2iv( const GLint *v ) {
}

void es_glVertex2sv( const GLshort *v ) {
}

void es_glVertex3dv( const GLdouble *v ) {
}

void es_glVertex3fv( const GLfloat *v ) {
}

void es_glVertex3iv( const GLint *v ) {
}

void es_glVertex3sv( const GLshort *v ) {
}

void es_glVertex4dv( const GLdouble *v ) {
}

void es_glVertex4fv( const GLfloat *v ) {
}

void es_glVertex4iv( const GLint *v ) {
}

void es_glVertex4sv( const GLshort *v ) {
}

void es_glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz ) {
}

void es_glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz ) {
}

void es_glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz ) {
}

void es_glNormal3i( GLint nx, GLint ny, GLint nz ) {
}

void es_glNormal3s( GLshort nx, GLshort ny, GLshort nz ) {
}

void es_glNormal3bv( const GLbyte *v ) {
}

void es_glNormal3dv( const GLdouble *v ) {
}

void es_glNormal3fv( const GLfloat *v ) {
}

void es_glNormal3iv( const GLint *v ) {
}

void es_glNormal3sv( const GLshort *v ) {
}

void es_glIndexd( GLdouble c ) {
}

void es_glIndexf( GLfloat c ) {
}

void es_glIndexi( GLint c ) {
}

void es_glIndexs( GLshort c ) {
}

void es_glIndexub( GLubyte c ) {
}

void es_glIndexdv( const GLdouble *c ) {
}

void es_glIndexfv( const GLfloat *c ) {
}

void es_glIndexiv( const GLint *c ) {
}

void es_glIndexsv( const GLshort *c ) {
}

void es_glIndexubv( const GLubyte *c ) {
}

void es_glColor3b( GLbyte red, GLbyte green, GLbyte blue ) {
}

void es_glColor3d( GLdouble red, GLdouble green, GLdouble blue ) {
}

void es_glColor3f( GLfloat red, GLfloat green, GLfloat blue ) {
}

void es_glColor3i( GLint red, GLint green, GLint blue ) {
}

void es_glColor3s( GLshort red, GLshort green, GLshort blue ) {
}

void es_glColor3ub( GLubyte red, GLubyte green, GLubyte blue ) {
}

void es_glColor3ui( GLuint red, GLuint green, GLuint blue ) {
}

void es_glColor3us( GLushort red, GLushort green, GLushort blue ) {
}

void es_glColor4b( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ) {
}

void es_glColor4d( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ) {
}

void es_glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) {
}

void es_glColor4i( GLint red, GLint green, GLint blue, GLint alpha ) {
}

void es_glColor4s( GLshort red, GLshort green, GLshort blue, GLshort alpha ) {
}

void es_glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ) {
}

void es_glColor4ui( GLuint red, GLuint green, GLuint blue, GLuint alpha ) {
}

void es_glColor4us( GLushort red, GLushort green, GLushort blue, GLushort alpha ) {
}

void es_glColor3bv( const GLbyte *v ) {
}

void es_glColor3dv( const GLdouble *v ) {
}

void es_glColor3fv( const GLfloat *v ) {
}

void es_glColor3iv( const GLint *v ) {
}

void es_glColor3sv( const GLshort *v ) {
}

void es_glColor3ubv( const GLubyte *v ) {
}

void es_glColor3uiv( const GLuint *v ) {
}

void es_glColor3usv( const GLushort *v ) {
}

void es_glColor4bv( const GLbyte *v ) {
}

void es_glColor4dv( const GLdouble *v ) {
}

void es_glColor4fv( const GLfloat *v ) {
}

void es_glColor4iv( const GLint *v ) {
}

void es_glColor4sv( const GLshort *v ) {
}

void es_glColor4ubv( const GLubyte *v ) {
}

void es_glColor4uiv( const GLuint *v ) {
}

void es_glColor4usv( const GLushort *v ) {
}

void es_glTexCoord1d( GLdouble s ) {
}

void es_glTexCoord1f( GLfloat s ) {
}

void es_glTexCoord1i( GLint s ) {
}

void es_glTexCoord1s( GLshort s ) {
}

void es_glTexCoord2d( GLdouble s, GLdouble t ) {
}

void es_glTexCoord2f( GLfloat s, GLfloat t ) {
}

void es_glTexCoord2i( GLint s, GLint t ) {
}

void es_glTexCoord2s( GLshort s, GLshort t ) {
}

void es_glTexCoord3d( GLdouble s, GLdouble t, GLdouble r ) {
}

void es_glTexCoord3f( GLfloat s, GLfloat t, GLfloat r ) {
}

void es_glTexCoord3i( GLint s, GLint t, GLint r ) {
}

void es_glTexCoord3s( GLshort s, GLshort t, GLshort r ) {
}

void es_glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q ) {
}

void es_glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q ) {
}

void es_glTexCoord4i( GLint s, GLint t, GLint r, GLint q ) {
}

void es_glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q ) {
}

void es_glTexCoord1dv( const GLdouble *v ) {
}

void es_glTexCoord1fv( const GLfloat *v ) {
}

void es_glTexCoord1iv( const GLint *v ) {
}

void es_glTexCoord1sv( const GLshort *v ) {
}

void es_glTexCoord2dv( const GLdouble *v ) {
}

void es_glTexCoord2fv( const GLfloat *v ) {
}

void es_glTexCoord2iv( const GLint *v ) {
}

void es_glTexCoord2sv( const GLshort *v ) {
}

void es_glTexCoord3dv( const GLdouble *v ) {
}

void es_glTexCoord3fv( const GLfloat *v ) {
}

void es_glTexCoord3iv( const GLint *v ) {
}

void es_glTexCoord3sv( const GLshort *v ) {
}

void es_glTexCoord4dv( const GLdouble *v ) {
}

void es_glTexCoord4fv( const GLfloat *v ) {
}

void es_glTexCoord4iv( const GLint *v ) {
}

void es_glTexCoord4sv( const GLshort *v ) {
}

void es_glRasterPos2d( GLdouble x, GLdouble y ) {
}

void es_glRasterPos2f( GLfloat x, GLfloat y ) {
}

void es_glRasterPos2i( GLint x, GLint y ) {
}

void es_glRasterPos2s( GLshort x, GLshort y ) {
}

void es_glRasterPos3d( GLdouble x, GLdouble y, GLdouble z ) {
}

void es_glRasterPos3f( GLfloat x, GLfloat y, GLfloat z ) {
}

void es_glRasterPos3i( GLint x, GLint y, GLint z ) {
}

void es_glRasterPos3s( GLshort x, GLshort y, GLshort z ) {
}

void es_glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) {
}

void es_glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) {
}

void es_glRasterPos4i( GLint x, GLint y, GLint z, GLint w ) {
}

void es_glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w ) {
}

void es_glRasterPos2dv( const GLdouble *v ) {
}

void es_glRasterPos2fv( const GLfloat *v ) {
}

void es_glRasterPos2iv( const GLint *v ) {
}

void es_glRasterPos2sv( const GLshort *v ) {
}

void es_glRasterPos3dv( const GLdouble *v ) {
}

void es_glRasterPos3fv( const GLfloat *v ) {
}

void es_glRasterPos3iv( const GLint *v ) {
}

void es_glRasterPos3sv( const GLshort *v ) {
}

void es_glRasterPos4dv( const GLdouble *v ) {
}

void es_glRasterPos4fv( const GLfloat *v ) {
}

void es_glRasterPos4iv( const GLint *v ) {
}

void es_glRasterPos4sv( const GLshort *v ) {
}

void es_glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ) {
}

void es_glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ) {
}

void es_glRecti( GLint x1, GLint y1, GLint x2, GLint y2 ) {
}

void es_glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 ) {
}

void es_glRectdv( const GLdouble *v1, const GLdouble *v2 ) {
}

void es_glRectfv( const GLfloat *v1, const GLfloat *v2 ) {
}

void es_glRectiv( const GLint *v1, const GLint *v2 ) {
}

void es_glRectsv( const GLshort *v1, const GLshort *v2 ) {
}

void es_glShadeModel( GLenum mode ) {
}

void es_glLightf( GLenum light, GLenum pname, GLfloat param ) {
}

void es_glLighti( GLenum light, GLenum pname, GLint param ) {
}

void es_glLightfv( GLenum light, GLenum pname, const GLfloat *params ) {
}

void es_glLightiv( GLenum light, GLenum pname, const GLint *params ) {
}

void es_glGetLightfv( GLenum light, GLenum pname, GLfloat *params ) {
}

void es_glGetLightiv( GLenum light, GLenum pname, GLint *params ) {
}

void es_glLightModelf( GLenum pname, GLfloat param ) {
}

void es_glLightModeli( GLenum pname, GLint param ) {
}

void es_glLightModelfv( GLenum pname, const GLfloat *params ) {
}

void es_glLightModeliv( GLenum pname, const GLint *params ) {
}

void es_glMaterialf( GLenum face, GLenum pname, GLfloat param ) {
}

void es_glMateriali( GLenum face, GLenum pname, GLint param ) {
}

void es_glMaterialfv( GLenum face, GLenum pname, const GLfloat *params ) {
}

void es_glMaterialiv( GLenum face, GLenum pname, const GLint *params ) {
}

void es_glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params ) {
}

void es_glGetMaterialiv( GLenum face, GLenum pname, GLint *params ) {
}

void es_glColorMaterial( GLenum face, GLenum mode ) {
}

void es_glPixelZoom( GLfloat xfactor, GLfloat yfactor ) {
}

void es_glPixelStoref( GLenum pname, GLfloat param ) {
}

void es_glPixelStorei( GLenum pname, GLint param ) {
}

void es_glPixelTransferf( GLenum pname, GLfloat param ) {
}

void es_glPixelTransferi( GLenum pname, GLint param ) {
}

void es_glPixelMapfv( GLenum map, GLint mapsize, const GLfloat *values ) {
}

void es_glPixelMapuiv( GLenum map, GLint mapsize, const GLuint *values ) {
}

void es_glPixelMapusv( GLenum map, GLint mapsize, const GLushort *values ) {
}

void es_glGetPixelMapfv( GLenum map, GLfloat *values ) {
}

void es_glGetPixelMapuiv( GLenum map, GLuint *values ) {
}

void es_glGetPixelMapusv( GLenum map, GLushort *values ) {
}

void es_glBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ) {
}

void es_glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) {
}

void es_glDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) {
}

void es_glCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type ) {
}

void es_glStencilFunc( GLenum func, GLint ref, GLuint mask ) {
}

void es_glStencilMask( GLuint mask ) {
}

void es_glStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) {
}

void es_glClearStencil( GLint s ) {
    if (activeList) {
        struct ListOp* op = allocListOp(activeList);
        op->op = list_glClearStencil;
        op->a1.i = s;
    } else {
        glClearStencil(s);
    }
}

void es_glTexGend( GLenum coord, GLenum pname, GLdouble param ) {
}

void es_glTexGenf( GLenum coord, GLenum pname, GLfloat param ) {
}

void es_glTexGeni( GLenum coord, GLenum pname, GLint param ) {
}

void es_glTexGendv( GLenum coord, GLenum pname, const GLdouble *params ) {
}

void es_glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params ) {
}

void es_glTexGeniv( GLenum coord, GLenum pname, const GLint *params ) {
}

void es_glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params ) {
}

void es_glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params ) {
}

void es_glGetTexGeniv( GLenum coord, GLenum pname, GLint *params ) {
}

void es_glTexEnvf( GLenum target, GLenum pname, GLfloat param ) {
}

void es_glTexEnvi( GLenum target, GLenum pname, GLint param ) {
}

void es_glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params ) {
}

void es_glTexEnviv( GLenum target, GLenum pname, const GLint *params ) {
}

void es_glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params ) {
}

void es_glGetTexEnviv( GLenum target, GLenum pname, GLint *params ) {
}

void es_glTexParameterf( GLenum target, GLenum pname, GLfloat param ) {
}

void es_glTexParameteri( GLenum target, GLenum pname, GLint param ) {
}

void es_glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params ) {
}

void es_glTexParameteriv( GLenum target, GLenum pname, const GLint *params ) {
}

void es_glGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params) {
}

void es_glGetTexParameteriv( GLenum target, GLenum pname, GLint *params ) {
}

void es_glGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params ) {
}

void es_glGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params ) {
}

void es_glTexImage1D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {
}

void es_glTexImage2D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {
}

void es_glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ) {
}

void es_glMap1d( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ) {
}

void es_glMap1f( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ) {
}

void es_glMap2d( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ) {
}

void es_glMap2f( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ) {
}

void es_glGetMapdv( GLenum target, GLenum query, GLdouble *v ) {
}

void es_glGetMapfv( GLenum target, GLenum query, GLfloat *v ) {
}

void es_glGetMapiv( GLenum target, GLenum query, GLint *v ) {
}

void es_glEvalCoord1d( GLdouble u ) {
}

void es_glEvalCoord1f( GLfloat u ) {
}

void es_glEvalCoord1dv( const GLdouble *u ) {
}

void es_glEvalCoord1fv( const GLfloat *u ) {
}

void es_glEvalCoord2d( GLdouble u, GLdouble v ) {
}

void es_glEvalCoord2f( GLfloat u, GLfloat v ) {
}

void es_glEvalCoord2dv( const GLdouble *u ) {
}

void es_glEvalCoord2fv( const GLfloat *u ) {
}

void es_glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 ) {
}

void es_glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 ) {
}

void es_glMapGrid2d( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ) {
}

void es_glMapGrid2f( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ) {
}

void es_glEvalPoint1( GLint i ) {
}

void es_glEvalPoint2( GLint i, GLint j ) {
}

void es_glEvalMesh1( GLenum mode, GLint i1, GLint i2 ) {
}

void es_glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ) {
}

void es_glFogf( GLenum pname, GLfloat param ) {
}

void es_glFogi( GLenum pname, GLint param ) {
}

void es_glFogfv( GLenum pname, const GLfloat *params ) {
}

void es_glFogiv( GLenum pname, const GLint *params ) {
}

void es_glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer ) {
}

void es_glPassThrough( GLfloat token ) {
}

void es_glSelectBuffer( GLsizei size, GLuint *buffer ) {
}

void es_glInitNames( void ) {
}

void es_glLoadName( GLuint name ) {
}

void es_glPushName( GLuint name ) {
}

void es_glPopName( void ) {
}

void es_glGenTextures( GLsizei n, GLuint *textures ) {
}

void es_glDeleteTextures( GLsizei n, const GLuint *textures) {
}

void es_glBindTexture( GLenum target, GLuint texture ) {
}

void es_glPrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities ) {
}

GLboolean es_glAreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences ) {
    return 0;
}

GLboolean es_glIsTexture( GLuint texture ) {
    return 0;
}

/* texture mapping */
void es_glTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ) {
}

void es_glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) {
}

void es_glCopyTexImage1D( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border ) {
}

void es_glCopyTexImage2D( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) {
}

void es_glCopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width ) {
}

void es_glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) {
}

/* vertex arrays */
void es_glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
}

void es_glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr ) {
}

void es_glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
}

void es_glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr ) {
}

void es_glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ) {
}

void es_glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr ) {
}

void es_glGetPointerv( GLenum pname, GLvoid **params ) {
}

void es_glArrayElement( GLint i ) {
}

void es_glDrawArrays( GLenum mode, GLint first, GLsizei count ) {
}

void es_glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) {
}

void es_glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ) {
}

/* 1.2 functions */
void es_glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices ) {
}

void es_glTexImage3D( GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {
}

#include "../glcommon.h"
#include "kalloc.h"
#include "kprocess.h"
#include "log.h"

#include <stdio.h>
#include <SDL.h>

int extLoaded = 0;

void loadExtensions() {
    if (!extLoaded) {
        extLoaded = 1;
        ext_glTexImage3D = (glTexImage3D_func)SDL_GL_GetProcAddress("glTexImage3D");
    }
}

// GLAPI void APIENTRY glFinish( void ) {
void es_glFinish(struct CPU* cpu) {	
    glFinish();
}

// GLAPI void APIENTRY glFlush( void ) {
void es_glFlush(struct CPU* cpu) {	
    glFlush();	
}

void fbSetupScreenForOpenGL(int width, int height, int depth);
void fbSetupScreen();

// GLXContext glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext share_list, Bool direct)
void es_glXCreateContext(struct CPU* cpu) {
    U32 doubleBuffered = ARG6;
    U32 format = ARG5;
    //U32 share = ARG4;
    U32 accum = ARG3;
    U32 stencil = ARG2;
    U32 depth = ARG1;	

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depth );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, stencil);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, accum);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, doubleBuffered?1:0 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, format==0x1907?24:32);

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES); 
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 ); 
    //SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    EAX = 0x1000;	
}

// void glXDestroyContext(Display *dpy, GLXContext ctx)
void es_glXDestroyContext(struct CPU* cpu) {

}

// Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx) 
void es_glXMakeCurrent(struct CPU* cpu) {
    //U32 isWindow = ARG5;
    U32 depth = ARG4;
    U32 height = ARG3;
    U32 width = ARG2;

    if (width) {
        loadExtensions();
        fbSetupScreenForOpenGL(width, height, depth);
    } else {
        fbSetupScreen();
    }
}

// void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
void es_glXSwapBuffers(struct CPU* cpu) {
    fbSwapOpenGL();
}

void esgl_init() {	
    int99Callback[Finish] = es_glFinish;
    int99Callback[Flush] = es_glFlush;
    int99Callback[XCreateContext] = es_glXCreateContext;
    int99Callback[XMakeCurrent] = es_glXMakeCurrent;
    int99Callback[XDestroyContext] = es_glXDestroyContext;	
    int99Callback[XSwapBuffer] = es_glXSwapBuffers;
}
#endif
