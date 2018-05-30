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

#ifndef __FPU_H__
#define __FPU_H__

#include "platform.h"

struct FPU_Reg {
    union {
        double d;
        U64 l;
    };
    U64 loadedInteger;
    U8 isIntegerLoaded;
};

struct FPU {
    struct FPU_Reg regs[9];
    U32 tags[9];
    U32 cw;
    U32 cw_mask_all;
    U32 sw;
    U32 top;
    U32 round;
};

void FPU_FINIT(struct FPU* fpu);

#endif