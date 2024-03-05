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

#ifndef __GLCOMMON_H__
#define __GLCOMMON_H__

#ifdef BOXEDWINE_OPENGL
#include "../../tools/opengl/gldef.h"
#include <inttypes.h>

//#define GL_LOG klog
#define GL_LOG if (0) klog

#ifdef BOXEDWINE_OPENGL_ES
#define GL_FUNC(name) es_##name
#include "es/esopengl.h"
#else
#define GL_FUNC(name) name
#endif

struct int2Float {
    union {
        U32 i;
        float f;
    };
};

struct long2Double {
    union {
        U64 l;
        double d;
    };
};

// index 0 is the gl call number
#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)
#define ARG5 cpu->peek32(5)
#define ARG6 cpu->peek32(6)
#define ARG7 cpu->peek32(7)
#define ARG8 cpu->peek32(8)
#define ARG9 cpu->peek32(9)
#define ARG10 cpu->peek32(10)
#define ARG11 cpu->peek32(11)
#define ARG12 cpu->peek32(12)
#define ARG13 cpu->peek32(13)
#define ARG14 cpu->peek32(14)
#define ARG15 cpu->peek32(15)

#define pARG1 ((uintptr_t)cpu->peek32(1))
#define pARG2 ((uintptr_t)cpu->peek32(2))
#define pARG3 ((uintptr_t)cpu->peek32(3))
#define pARG4 ((uintptr_t)cpu->peek32(4))
#define pARG5 ((uintptr_t)cpu->peek32(5))
#define pARG6 ((uintptr_t)cpu->peek32(6))
#define pARG7 ((uintptr_t)cpu->peek32(7))
#define pARG8 ((uintptr_t)cpu->peek32(8))
#define pARG9 ((uintptr_t)cpu->peek32(9))
#define pARG10 ((uintptr_t)cpu->peek32(10))
#define pARG11 ((uintptr_t)cpu->peek32(11))
#define pARG12 ((uintptr_t)cpu->peek32(12))
#define pARG13 ((uintptr_t)cpu->peek32(13))
#define pARG14 ((uintptr_t)cpu->peek32(14))
#define pARG15 ((uintptr_t)cpu->peek32(15))

#define bARG1 (cpu->peek32(1) & 0xFF)
#define bARG2 (cpu->peek32(2) & 0xFF)
#define bARG3 (cpu->peek32(3) & 0xFF)
#define bARG4 (cpu->peek32(4) & 0xFF)
#define bARG5 (cpu->peek32(5) & 0xFF)
#define bARG6 (cpu->peek32(6) & 0xFF)
#define bARG7 (cpu->peek32(7) & 0xFF)
#define bARG8 (cpu->peek32(8) & 0xFF)
#define bARG9 (cpu->peek32(9) & 0xFF)
#define bARG10 (cpu->peek32(10) & 0xFF)
#define bARG11 (cpu->peek32(11) & 0xFF)
#define bARG12 (cpu->peek32(12) & 0xFF)
#define bARG13 (cpu->peek32(13) & 0xFF)
#define bARG14 (cpu->peek32(14) & 0xFF)
#define bARG15 (cpu->peek32(15) & 0xFF)

#define sARG1 (cpu->peek32(1) & 0xFFFF)
#define sARG2 (cpu->peek32(2) & 0xFFFF)
#define sARG3 (cpu->peek32(3) & 0xFFFF)
#define sARG4 (cpu->peek32(4) & 0xFFFF)
#define sARG5 (cpu->peek32(5) & 0xFFFF)
#define sARG6 (cpu->peek32(6) & 0xFFFF)
#define sARG7 (cpu->peek32(7) & 0xFFFF)
#define sARG8 (cpu->peek32(8) & 0xFFFF)
#define sARG9 (cpu->peek32(9) & 0xFFFF)
#define sARG10 (cpu->peek32(10) & 0xFFFF)
#define sARG11 (cpu->peek32(11) & 0xFFFF)
#define sARG12 (cpu->peek32(12) & 0xFFFF)
#define sARG13 (cpu->peek32(13) & 0xFFFF)
#define sARG14 (cpu->peek32(14) & 0xFFFF)
#define sARG15 (cpu->peek32(15) & 0xFFFF)

#define hARG1 ARG1
#define hARG2 ARG2
#define hARG3 ARG3
#define hARG4 ARG4
#define hARG5 ARG5
#define hARG6 ARG6
#define hARG7 ARG7
#define hARG8 ARG8

#define ipARG1 ARG1
#define ipARG2 ARG2
#define ipARG3 ARG3
#define ipARG4 ARG4
#define ipARG5 ARG5
#define ipARG6 ARG6
#define ipARG7 ARG7
#define ipARG8 ARG8

#define llARG1 cpu->memory->readq(ARG1)
#define llARG2 cpu->memory->readq(ARG2)
#define llARG3 cpu->memory->readq(ARG3)
#define llARG4 cpu->memory->readq(ARG4)
#define llARG5 cpu->memory->readq(ARG5)
#define llARG6 cpu->memory->readq(ARG6)
#define llARG7 cpu->memory->readq(ARG7)
#define llARG8 cpu->memory->readq(ARG8)

#define fARG1 fARG(cpu, ARG1)
#define fARG2 fARG(cpu, ARG2)
#define fARG3 fARG(cpu, ARG3)
#define fARG4 fARG(cpu, ARG4)
#define fARG5 fARG(cpu, ARG5)
#define fARG6 fARG(cpu, ARG6)
#define fARG7 fARG(cpu, ARG7)
#define fARG8 fARG(cpu, ARG8)
#define fARG9 fARG(cpu, ARG8)
#define fARG10 fARG(cpu, ARG10)
#define fARG11 fARG(cpu, ARG11)
#define fARG12 fARG(cpu, ARG12)
#define fARG13 fARG(cpu, ARG13)
#define fARG14 fARG(cpu, ARG14)
#define fARG15 fARG(cpu, ARG15)

#define dARG1 dARG(cpu, ARG1)
#define dARG2 dARG(cpu, ARG2)
#define dARG3 dARG(cpu, ARG3)
#define dARG4 dARG(cpu, ARG4)
#define dARG5 dARG(cpu, ARG5)
#define dARG6 dARG(cpu, ARG6)
#define dARG7 dARG(cpu, ARG7)
#define dARG8 dARG(cpu, ARG8)
#define dARG9 dARG(cpu, ARG9)
#define dARG10 dARG(cpu, ARG10)
#define dARG11 dARG(cpu, ARG11)

float fARG(CPU* cpu, U32 arg);
double dARG(CPU* cpu, int address);

#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) typedef RET (OPENGL_CALL_TYPE *gl##func##_func)PARAMS; extern gl##func##_func pgl##func;
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) typedef RET (OPENGL_CALL_TYPE *gl##func##_func)PARAMS; extern gl##func##_func pgl##func;
#define GL_EXT_FUNCTION(func, RET, PARAMS) typedef RET (OPENGL_CALL_TYPE *gl##func##_func)PARAMS; extern gl##func##_func ext_gl##func;

#include "glfunctions.h"

#endif
#endif
