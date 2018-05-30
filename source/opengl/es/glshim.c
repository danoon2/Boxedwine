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
#include "glshim.h"
#include "khash.h"

#define MAX_TEX 4
#define bool int

typedef struct {
    unsigned int len, count, cap;
    GLenum mode;
    struct {
        GLfloat tex[MAX_TEX][2];
    } last;

    // TODO: dynamic type support?
    /*
    struct {
        GLenum vert, normal, color, tex;
    } type;
    */

    GLfloat *vert;
    GLfloat *normal;
    GLfloat *color;
    GLfloat *tex[MAX_TEX];
    GLushort *indices;
    GLboolean q2t;

    struct {
        int tex[MAX_TEX], color, normal;
    } incomplete;

    GLboolean open;
    GLboolean artificial;
} block_t;

typedef struct {
    int format;
    block_t *block;
    int refs;
} block_call_t;

// eval.h
typedef struct {
    GLenum type;
} map_state_t;

typedef struct {
    GLdouble _1, _2, n, d;
    GLint stride, order;
} mapcoordd_t;

typedef struct {
    GLdouble _1, _2, n, d;
    GLint stride, order;
} mapcoordf_t;

typedef struct {
    GLenum type;
    GLint dims, width;
    mapcoordd_t u, v;
    GLboolean free;
    const GLdouble *points;
} map_stated_t;

typedef struct {
    GLenum type;
    GLint dims, width;
    mapcoordf_t u, v;
    GLboolean free;
    const GLfloat *points;
} map_statef_t;

typedef struct {
    void **data;
    int len, cap, pos;
} tack_t;

// list.h
typedef struct {
    bool open;
    tack_t calls;
} displaylist_t;

// texture.h
typedef struct {
    GLuint texture;
    GLenum target;
    GLsizei width;
    GLsizei height;
    GLsizei nwidth;
    GLsizei nheight;
    GLboolean uploaded;
} gltexture_t;

KHASH_MAP_INIT_INT(tex, gltexture_t *)

// state.h
typedef struct {
    GLboolean line_stipple,
              blend,
              color_array,
              normal_array,
              tex_coord_array[MAX_TEX],
              texgen_q[MAX_TEX],
              texgen_r[MAX_TEX],
              texgen_s[MAX_TEX],
              texgen_t[MAX_TEX],
              texture_2d[MAX_TEX],
              vertex_array;
} enable_state_t;

typedef struct {
    GLenum R, Q, S, T;
    GLfloat Rv[4], Qv[4], Sv[4], Tv[4];
} texgen_state_t;

typedef struct {
    GLuint unpack_row_length,
           unpack_skip_pixels,
           unpack_skip_rows;
    GLboolean unpack_lsb_first;
    // TODO: do we only need to worry about GL_TEXTURE_2D?
    GLboolean rect_arb[MAX_TEX];
    gltexture_t *bound[MAX_TEX];
    khash_t(tex) *list;
    // active textures
    GLuint active;
    GLuint client;
} texture_state_t;

typedef struct {
    GLint size;
    GLenum type;
    GLsizei stride;
    const GLvoid *pointer;
} pointer_state_t;

typedef struct {
    pointer_state_t vertex, color, normal, tex_coord[MAX_TEX];
} pointer_states_t;

typedef struct {
    GLfloat color[4];
    GLfloat normal[3];
    GLfloat tex[MAX_TEX][2];
} current_state_t;

typedef struct {
    displaylist_t *active;
    current_state_t current;

    GLuint base;
    GLuint name;
    GLenum mode;
} displaylist_state_t;

typedef struct {
    block_t *active;
    GLboolean locked;
} block_state_t;

typedef struct {
    map_state_t *vertex3,
                *vertex4,
                *index,
                *color4,
                *normal,
                *texture1,
                *texture2,
                *texture3,
                *texture4;
} map_states_t;

typedef struct { 
    float x;
    float y; 
    float z; 
    float w;
} simd4f;

typedef struct {
    simd4f x,y,z,w;
} simd4x4f;

// matrix structs
typedef struct {
    simd4x4f matrix;
    tack_t stack;
    bool init;
} matrix_state_t;

typedef struct {
    GLenum mode;
    matrix_state_t model, projection, texture[MAX_TEX], color;
} matrix_states_t;

typedef struct {
    GLboolean overflow;
    GLint count;
    GLsizei size;
    GLuint *buffer;
    tack_t names;
} select_state_t;

typedef struct {
    GLboolean overflow;
    GLenum type;
    GLfloat *buffer;
    GLint count, values;
    GLsizei size;
} feedback_state_t;

typedef struct {
    GLenum mode;
} render_state_t;

typedef struct {
    GLubyte *buf;
    struct {
        GLfloat x, y, z, w;
    } pos;
    GLfloat color[4];
    GLuint pixel;
    GLboolean valid;
} raster_state_t;

typedef struct {
    GLfloat x, y, width, height, nwidth, nheight;
} viewport_state_t;

typedef struct {
    tack_t attrib, client;
} stack_state_t;

// global state struct
typedef struct {
    displaylist_state_t list;
    tack_t lists;

    GLenum error;
    block_state_t block;
    current_state_t current;
    enable_state_t enable;
    feedback_state_t feedback;
    map_state_t *map_grid;
    map_states_t map1, map2;
    matrix_states_t matrix;
    pointer_states_t pointers;
    raster_state_t raster;
    render_state_t render;
    select_state_t select;
    stack_state_t stack;
    texgen_state_t texgen[MAX_TEX];
    texture_state_t texture;
    viewport_state_t viewport;
} glstate_t;

glstate_t state = {
    {0},
    .current = {
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .normal = {0.0f, 0.0f, 1.0f},
        .tex = {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}},
    },
    .matrix = {
        .mode = GL_MODELVIEW,
    },
    .render = {
        .mode = GL_RENDER,
    },
    .raster = {
        .buf = NULL,
        .pos = {0.0f, 0.0f, 0.0f, 1.0f},
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .pixel = 0xFFFFFFFF,
        .valid = 1,
    },
    .texgen = {{
        .R = GL_EYE_LINEAR,
        .Q = GL_EYE_LINEAR,
        .S = GL_EYE_LINEAR,
        .T = GL_EYE_LINEAR,
        .Rv = {0, 0, 0, 0},
        .Qv = {0, 0, 0, 0},
        .Sv = {1, 0, 0, 0},
        .Tv = {0, 1, 0, 0},
    }, {
        .R = GL_EYE_LINEAR,
        .Q = GL_EYE_LINEAR,
        .S = GL_EYE_LINEAR,
        .T = GL_EYE_LINEAR,
        .Rv = {0, 0, 0, 0},
        .Qv = {0, 0, 0, 0},
        .Sv = {1, 0, 0, 0},
        .Tv = {0, 1, 0, 0},
    }, {
        .R = GL_EYE_LINEAR,
        .Q = GL_EYE_LINEAR,
        .S = GL_EYE_LINEAR,
        .T = GL_EYE_LINEAR,
        .Rv = {0, 0, 0, 0},
        .Qv = {0, 0, 0, 0},
        .Sv = {1, 0, 0, 0},
        .Tv = {0, 1, 0, 0},
    }, {
        .R = GL_EYE_LINEAR,
        .Q = GL_EYE_LINEAR,
        .S = GL_EYE_LINEAR,
        .T = GL_EYE_LINEAR,
        .Rv = {0, 0, 0, 0},
        .Qv = {0, 0, 0, 0},
        .Sv = {1, 0, 0, 0},
        .Tv = {0, 1, 0, 0},
    }},
};

GLint stippleFactor = 1;
GLushort stipplePattern = 0xFFFF;
GLubyte *stippleData = NULL;
GLuint stippleTexture = 0;

void glLineStipple(GLint factor, GLushort pattern) {
    stippleFactor = factor;
    stipplePattern = pattern;
    if (stippleData != NULL) {
        free(stippleData);
    }
    stippleData = (GLubyte *)malloc(sizeof(GLubyte) * 16);
    for (int i = 0; i < 16; i++) {
        stippleData[i] = (stipplePattern >> i) & 1 ? 255 : 0;
    }

    //glPushAttrib(GL_TEXTURE_BIT);
    if (! stippleTexture)
        glGenTextures(1, &stippleTexture);

    glBindTexture(GL_TEXTURE_2D, stippleTexture);
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 16, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, stippleData);
    //glPopAttrib();
}

void shim_glEnable(GLenum cap) {
    switch (cap) {
        case GL_BLEND:
            if (state.enable.blend != GL_TRUE) {
                state.enable.blend = GL_TRUE;
                glEnable(cap);
            } 
            break;
        case GL_TEXTURE_2D:
            if (state.enable.texture_2d[state.texture.active] != GL_TRUE) {
                state.enable.texture_2d[state.texture.active] = GL_TRUE;
                glEnable(cap);
            } 
            break;
        case GL_TEXTURE_GEN_R:
            state.enable.texgen_r[state.texture.active] = GL_TRUE;
            break;
        case GL_TEXTURE_GEN_Q:
            state.enable.texgen_q[state.texture.active] = GL_TRUE;
            break;
        case GL_TEXTURE_GEN_S:
            state.enable.texgen_s[state.texture.active] = GL_TRUE;
            break;
        case GL_TEXTURE_GEN_T:
            state.enable.texgen_t[state.texture.active] = GL_TRUE;
            break;
        case GL_LINE_STIPPLE:
            state.enable.line_stipple = GL_TRUE;
            break;

        // for glDrawArrays
        case GL_VERTEX_ARRAY:
            if (state.enable.vertex_array != GL_TRUE) {
                state.enable.vertex_array = GL_TRUE;
                glEnable(cap);
            } 
            break;
        case GL_NORMAL_ARRAY:
            if (state.enable.normal_array != GL_TRUE) {
                state.enable.normal_array = GL_TRUE;
                glEnable(cap);
            } 
            break;
        case GL_COLOR_ARRAY:
            if (state.enable.color_array != GL_TRUE) {
                state.enable.color_array = GL_TRUE;
                glEnable(cap);
            } 
            break;
        case GL_TEXTURE_COORD_ARRAY:
            if (state.enable.tex_coord_array[state.texture.client] != GL_TRUE) {
                state.enable.tex_coord_array[state.texture.client] = GL_TRUE;
                glEnable(cap);
            } 
            break;
        default: 
            glEnable(cap);
            break;
    }
}

void shim_glDisable(GLenum cap) {
    switch (cap) {
        case GL_BLEND:
            if (state.enable.blend != GL_FALSE) {
                state.enable.blend = GL_FALSE;
                glDisable(cap);
            } 
            break;
        case GL_TEXTURE_2D:
            if (state.enable.texture_2d[state.texture.active] != GL_FALSE) {
                state.enable.texture_2d[state.texture.active] = GL_FALSE;
                glDisable(cap);
            } 
            break;
        case GL_TEXTURE_GEN_R:
            state.enable.texgen_r[state.texture.active] = GL_FALSE;
            break;
        case GL_TEXTURE_GEN_Q:
            state.enable.texgen_q[state.texture.active] = GL_FALSE;
            break;
        case GL_TEXTURE_GEN_S:
            state.enable.texgen_s[state.texture.active] = GL_FALSE;
            break;
        case GL_TEXTURE_GEN_T:
            state.enable.texgen_t[state.texture.active] = GL_FALSE;
            break;
        case GL_LINE_STIPPLE:
            state.enable.line_stipple = GL_FALSE;
            break;

        // for glDrawArrays
        case GL_VERTEX_ARRAY:
            if (state.enable.vertex_array != GL_FALSE) {
                state.enable.vertex_array = GL_FALSE;
                glDisable(cap);
            } 
            break;
        case GL_NORMAL_ARRAY:
            if (state.enable.normal_array != GL_FALSE) {
                state.enable.normal_array = GL_FALSE;
                glDisable(cap);
            } 
            break;
        case GL_COLOR_ARRAY:
            if (state.enable.color_array != GL_FALSE) {
                state.enable.color_array = GL_FALSE;
                glDisable(cap);
            } 
            break;
        case GL_TEXTURE_COORD_ARRAY:
            if (state.enable.tex_coord_array[state.texture.client] != GL_FALSE) {
                state.enable.tex_coord_array[state.texture.client] = GL_FALSE;
                glDisable(cap);
            } 
            break;
        default: 
            glDisable(cap);
            break;
    }
}
#endif
