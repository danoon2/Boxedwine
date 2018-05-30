#ifndef __GLSHIM_H__
#define __GLSHIM_H__

#define GL_TEXTURE_BIT				0x00040000

#define GL_TEXTURE_ENV				0x2300
#define GL_TEXTURE_ENV_MODE			0x2200
#define GL_MODULATE				0x2100
#define GL_MODELVIEW				0x1700
#define GL_RENDER				0x1C00
#define GL_EYE_LINEAR				0x2400

#define GL_VERTEX_ARRAY				0x8074
#define GL_NORMAL_ARRAY				0x8075
#define GL_COLOR_ARRAY				0x8076
#define GL_TEXTURE_COORD_ARRAY			0x8078

#define GL_TEXTURE_GEN_R			0x0C62
#define GL_TEXTURE_GEN_Q			0x0C63
#define GL_TEXTURE_GEN_S			0x0C60
#define GL_TEXTURE_GEN_T			0x0C61
#define GL_LINE_STIPPLE				0x0B24

void glLineStipple(GLint factor, GLushort pattern);
void shim_glEnable(GLenum cap);
void shim_glDisable(GLenum cap);

#endif