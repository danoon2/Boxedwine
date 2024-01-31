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

// This code follows pretty closely a dosbox patch for the Daum build
#include "boxedwine.h"
#include "common_mmx.h"

S8 SaturateWordSToByteS(S16 value)
{
  if(value < -128) return -128;
  if(value >  127) return  127;
  return (S8) value;
}

S16 SaturateDwordSToWordS(S32 value)
{
  if(value < -32768) return -32768;
  if(value >  32767) return  32767;
  return (S16) value;
}

U8 SaturateWordSToByteU(S16 value)
{
  if(value < 0) return 0;
  if(value > 255) return 255;
  return (U8) value;
}

U16 SaturateDwordSToWordU(S32 value)
{
  if(value < 0) return 0;
  if(value > 65535) return 65535;
  return (U16) value;
}

/* State Management */

// EMMS
void common_emms(CPU* cpu) {
    cpu->fpu.reset();
    cpu->resetMMX();
}

/* Data Movement */

void common_movPqR32(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* rmrq=&cpu->reg_mmx[r1];
    rmrq->ud.d0 = cpu->reg[r2].u32;
    rmrq->ud.d1 = 0;
}

void common_movPqE32(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* rmrq=&cpu->reg_mmx[reg];
    rmrq->ud.d0 = cpu->memory->readd(address);
    rmrq->ud.d1 = 0;
}

void common_movR32Pq(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = cpu->reg_mmx[r2].ud.d0;
}

void common_movE32Pq(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writed(address, cpu->reg_mmx[reg].ud.d0);
}

void common_movPqMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
}

void common_movPqE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q = cpu->memory->readq(address);
}

void common_movE64Pq(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->reg_mmx[reg].q);
}

void common_movMmxPq(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
}

/* Boolean Logic */

void common_pxorMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q ^= cpu->reg_mmx[r2].q;
}

void common_pxorE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q ^= cpu->memory->readq(address);
}

void common_porMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q |= cpu->reg_mmx[r2].q;
}

void common_porE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q |= cpu->memory->readq(address);
}

void common_pandMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q &= cpu->reg_mmx[r2].q;
}

void common_pandE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q &= cpu->memory->readq(address);
}

void common_pandnMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = ~cpu->reg_mmx[r1].q & cpu->reg_mmx[r2].q;
}

void common_pandnE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q = ~cpu->reg_mmx[reg].q & cpu->memory->readq(address);
}

/* Shift */
void common_psllwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    if (src->q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 <<= src->ub.b0;
		dest->uw.w1 <<= src->ub.b0;
		dest->uw.w2 <<= src->ub.b0;
		dest->uw.w3 <<= src->ub.b0;
	}
}

void common_psllwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 <<= src.ub.b0;
		dest->uw.w1 <<= src.ub.b0;
		dest->uw.w2 <<= src.ub.b0;
		dest->uw.w3 <<= src.ub.b0;
	}
}

void common_psrlwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    if (src->q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 >>= src->ub.b0;
		dest->uw.w1 >>= src->ub.b0;
		dest->uw.w2 >>= src->ub.b0;
		dest->uw.w3 >>= src->ub.b0;
	}
}

void common_psrlwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 >>= src.ub.b0;
		dest->uw.w1 >>= src.ub.b0;
		dest->uw.w2 >>= src.ub.b0;
		dest->uw.w3 >>= src.ub.b0;
	}
}

void common_psrawMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    U8 shift = 0;

    if (cpu->reg_mmx[r2].q > 15) {
        shift = 16;
    } else {
        shift = cpu->reg_mmx[r2].ub.b0;
    }
	dest->sw.w0 >>= shift;
	dest->sw.w1 >>= shift;
	dest->sw.w2 >>= shift;
	dest->sw.w3 >>= shift;
}

void common_psrawE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 15) {
        src.q = 16;
    }
	dest->sw.w0 >>= src.ub.b0;
	dest->sw.w1 >>= src.ub.b0;
	dest->sw.w2 >>= src.ub.b0;
	dest->sw.w3 >>= src.ub.b0;
}

void common_psllw(CPU* cpu, U32 reg, U8 imm) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    
    dest->uw.w0 <<= imm;
	dest->uw.w1 <<= imm;
	dest->uw.w2 <<= imm;
	dest->uw.w3 <<= imm;	
}

void common_psraw(CPU* cpu, U32 reg, U8 imm) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    
    dest->sw.w0 >>= imm;
	dest->sw.w1 >>= imm;
	dest->sw.w2 >>= imm;
	dest->sw.w3 >>= imm;	
}

void common_psrlw(CPU* cpu, U32 reg, U8 imm) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    
    dest->uw.w0 >>= imm;
	dest->uw.w1 >>= imm;
	dest->uw.w2 >>= imm;
	dest->uw.w3 >>= imm;	
}

void common_pslldMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    if (src->q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 <<= src->ub.b0;
		dest->ud.d1 <<= src->ub.b0;
	}
}

void common_pslldE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 <<= src.ub.b0;
		dest->ud.d1 <<= src.ub.b0;
	}
}

void common_psrldMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    if (src->q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 >>= src->ub.b0;
		dest->ud.d1 >>= src->ub.b0;
	}
}

void common_psrldE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 >>= src.ub.b0;
		dest->ud.d1 >>= src.ub.b0;
	}
}

void common_psradMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    U8 shift = 0;

    if (cpu->reg_mmx[r2].q > 31) {
        shift = 32;
    } else {
        shift = cpu->reg_mmx[r2].ub.b0;
    }
	dest->sd.d0 >>= shift;
	dest->sd.d1 >>= shift;	
}

void common_psradE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 31) {
        src.q = 32;
    }
	dest->sd.d0 >>= src.ub.b0;
	dest->sd.d1 >>= src.ub.b0;	
}

void common_pslld(CPU* cpu, U32 reg, U8 imm) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    
    dest->ud.d0 <<= imm;
	dest->ud.d1 <<= imm;
}

void common_psrad(CPU* cpu, U32 reg, U8 imm) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    
    dest->sd.d0 >>= imm;
	dest->sd.d1 >>= imm;
}

void common_psrld(CPU* cpu, U32 reg, U8 imm) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    
    dest->ud.d0 >>= imm;
	dest->ud.d1 >>= imm;
}

void common_psllqMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    if (src->q > 63) {
        dest->q = 0;
    } else {
		dest->q <<= src->ub.b0;
	}
}

void common_psllqE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q <<= src.ub.b0;
	}
}

void common_psrlqMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    if (src->q > 63) {
        dest->q = 0;
    } else {
		dest->q >>= src->ub.b0;
	}
}

void common_psrlqE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q >>= src.ub.b0;
	}
}

void common_psllq(CPU* cpu, U32 reg, U8 imm) {   
    cpu->reg_mmx[reg].q <<= imm;
}

void common_psrlq(CPU* cpu, U32 reg, U8 imm) {
    cpu->reg_mmx[reg].q >>= imm;
}

/* Math */
void common_paddbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->ub.b0 += src->ub.b0;
	dest->ub.b1 += src->ub.b1;
	dest->ub.b2 += src->ub.b2;
	dest->ub.b3 += src->ub.b3;
	dest->ub.b4 += src->ub.b4;
	dest->ub.b5 += src->ub.b5;
	dest->ub.b6 += src->ub.b6;
	dest->ub.b7 += src->ub.b7;
}

void common_paddbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->ub.b0 += src.ub.b0;
	dest->ub.b1 += src.ub.b1;
	dest->ub.b2 += src.ub.b2;
	dest->ub.b3 += src.ub.b3;
	dest->ub.b4 += src.ub.b4;
	dest->ub.b5 += src.ub.b5;
	dest->ub.b6 += src.ub.b6;
	dest->ub.b7 += src.ub.b7;
}

void common_paddwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->uw.w0 += src->uw.w0;
	dest->uw.w1 += src->uw.w1;
	dest->uw.w2 += src->uw.w2;
	dest->uw.w3 += src->uw.w3;
}

void common_paddwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->uw.w0 += src.uw.w0;
	dest->uw.w1 += src.uw.w1;
	dest->uw.w2 += src.uw.w2;
	dest->uw.w3 += src.uw.w3;
}

void common_padddMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->ud.d0 += src->ud.d0;
	dest->ud.d1 += src->ud.d1;
}

void common_padddE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->ud.d0 += src.ud.d0;
	dest->ud.d1 += src.ud.d1;
}

void common_paddsbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0+(S16)src->sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1+(S16)src->sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2+(S16)src->sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3+(S16)src->sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4+(S16)src->sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5+(S16)src->sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6+(S16)src->sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7+(S16)src->sb.b7);
}

void common_paddsbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0+(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1+(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2+(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3+(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4+(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5+(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6+(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7+(S16)src.sb.b7);
}

void common_paddswMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0+(S32)src->sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1+(S32)src->sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2+(S32)src->sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3+(S32)src->sw.w3);
}

void common_paddswE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0+(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1+(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2+(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3+(S32)src.sw.w3);
}

void common_paddusbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->ub.b0 = SaturateWordSToByteU((S16)dest->ub.b0+(S16)src->ub.b0);
	dest->ub.b1 = SaturateWordSToByteU((S16)dest->ub.b1+(S16)src->ub.b1);
	dest->ub.b2 = SaturateWordSToByteU((S16)dest->ub.b2+(S16)src->ub.b2);
	dest->ub.b3 = SaturateWordSToByteU((S16)dest->ub.b3+(S16)src->ub.b3);
	dest->ub.b4 = SaturateWordSToByteU((S16)dest->ub.b4+(S16)src->ub.b4);
	dest->ub.b5 = SaturateWordSToByteU((S16)dest->ub.b5+(S16)src->ub.b5);
	dest->ub.b6 = SaturateWordSToByteU((S16)dest->ub.b6+(S16)src->ub.b6);
	dest->ub.b7 = SaturateWordSToByteU((S16)dest->ub.b7+(S16)src->ub.b7);
}

void common_paddusbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->ub.b0 = SaturateWordSToByteU((S16)dest->ub.b0+(S16)src.ub.b0);
	dest->ub.b1 = SaturateWordSToByteU((S16)dest->ub.b1+(S16)src.ub.b1);
	dest->ub.b2 = SaturateWordSToByteU((S16)dest->ub.b2+(S16)src.ub.b2);
	dest->ub.b3 = SaturateWordSToByteU((S16)dest->ub.b3+(S16)src.ub.b3);
	dest->ub.b4 = SaturateWordSToByteU((S16)dest->ub.b4+(S16)src.ub.b4);
	dest->ub.b5 = SaturateWordSToByteU((S16)dest->ub.b5+(S16)src.ub.b5);
	dest->ub.b6 = SaturateWordSToByteU((S16)dest->ub.b6+(S16)src.ub.b6);
	dest->ub.b7 = SaturateWordSToByteU((S16)dest->ub.b7+(S16)src.ub.b7);
}

void common_padduswMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->uw.w0 = SaturateDwordSToWordU((S32)dest->uw.w0+(S32)src->uw.w0);
	dest->uw.w1 = SaturateDwordSToWordU((S32)dest->uw.w1+(S32)src->uw.w1);
	dest->uw.w2 = SaturateDwordSToWordU((S32)dest->uw.w2+(S32)src->uw.w2);
	dest->uw.w3 = SaturateDwordSToWordU((S32)dest->uw.w3+(S32)src->uw.w3);
}

void common_padduswE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->uw.w0 = SaturateDwordSToWordU((S32)dest->uw.w0+(S32)src.uw.w0);
	dest->uw.w1 = SaturateDwordSToWordU((S32)dest->uw.w1+(S32)src.uw.w1);
	dest->uw.w2 = SaturateDwordSToWordU((S32)dest->uw.w2+(S32)src.uw.w2);
	dest->uw.w3 = SaturateDwordSToWordU((S32)dest->uw.w3+(S32)src.uw.w3);
}

void common_psubbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->ub.b0 -= src->ub.b0;
	dest->ub.b1 -= src->ub.b1;
	dest->ub.b2 -= src->ub.b2;
	dest->ub.b3 -= src->ub.b3;
	dest->ub.b4 -= src->ub.b4;
	dest->ub.b5 -= src->ub.b5;
	dest->ub.b6 -= src->ub.b6;
	dest->ub.b7 -= src->ub.b7;
}

void common_psubbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->ub.b0 -= src.ub.b0;
	dest->ub.b1 -= src.ub.b1;
	dest->ub.b2 -= src.ub.b2;
	dest->ub.b3 -= src.ub.b3;
	dest->ub.b4 -= src.ub.b4;
	dest->ub.b5 -= src.ub.b5;
	dest->ub.b6 -= src.ub.b6;
	dest->ub.b7 -= src.ub.b7;
}

void common_psubwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->uw.w0 -= src->uw.w0;
	dest->uw.w1 -= src->uw.w1;
	dest->uw.w2 -= src->uw.w2;
	dest->uw.w3 -= src->uw.w3;
}

void common_psubwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->uw.w0 -= src.uw.w0;
	dest->uw.w1 -= src.uw.w1;
	dest->uw.w2 -= src.uw.w2;
	dest->uw.w3 -= src.uw.w3;
}

void common_psubdMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->ud.d0 -= src->ud.d0;
	dest->ud.d1 -= src->ud.d1;
}

void common_psubdE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->ud.d0 -= src.ud.d0;
	dest->ud.d1 -= src.ud.d1;
}

void common_psubsbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0-(S16)src->sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1-(S16)src->sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2-(S16)src->sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3-(S16)src->sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4-(S16)src->sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5-(S16)src->sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6-(S16)src->sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7-(S16)src->sb.b7);
}

void common_psubsbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0-(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1-(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2-(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3-(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4-(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5-(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6-(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7-(S16)src.sb.b7);
}

void common_psubswMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0-(S32)src->sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1-(S32)src->sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2-(S32)src->sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3-(S32)src->sw.w3);
}

void common_psubswE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;
    
    src.q = cpu->memory->readq(address);

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0-(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1-(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2-(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3-(S32)src.sw.w3);
}

void common_psubusbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    MMX_reg result;

    result.q = 0;
	if (dest->ub.b0>src->ub.b0) result.ub.b0 = dest->ub.b0 - src->ub.b0;
	if (dest->ub.b1>src->ub.b1) result.ub.b1 = dest->ub.b1 - src->ub.b1;
	if (dest->ub.b2>src->ub.b2) result.ub.b2 = dest->ub.b2 - src->ub.b2;
	if (dest->ub.b3>src->ub.b3) result.ub.b3 = dest->ub.b3 - src->ub.b3;
	if (dest->ub.b4>src->ub.b4) result.ub.b4 = dest->ub.b4 - src->ub.b4;
	if (dest->ub.b5>src->ub.b5) result.ub.b5 = dest->ub.b5 - src->ub.b5;
	if (dest->ub.b6>src->ub.b6) result.ub.b6 = dest->ub.b6 - src->ub.b6;
	if (dest->ub.b7>src->ub.b7) result.ub.b7 = dest->ub.b7 - src->ub.b7;
	dest->q = result.q;
}

void common_psubusbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src = {};
    MMX_reg result = {};
    
    src.q = cpu->memory->readq(address);

    result.q = 0;
	if (dest->ub.b0>src.ub.b0) result.ub.b0 = dest->ub.b0 - src.ub.b0;
	if (dest->ub.b1>src.ub.b1) result.ub.b1 = dest->ub.b1 - src.ub.b1;
	if (dest->ub.b2>src.ub.b2) result.ub.b2 = dest->ub.b2 - src.ub.b2;
	if (dest->ub.b3>src.ub.b3) result.ub.b3 = dest->ub.b3 - src.ub.b3;
	if (dest->ub.b4>src.ub.b4) result.ub.b4 = dest->ub.b4 - src.ub.b4;
	if (dest->ub.b5>src.ub.b5) result.ub.b5 = dest->ub.b5 - src.ub.b5;
	if (dest->ub.b6>src.ub.b6) result.ub.b6 = dest->ub.b6 - src.ub.b6;
	if (dest->ub.b7>src.ub.b7) result.ub.b7 = dest->ub.b7 - src.ub.b7;
	dest->q = result.q;
}

void common_psubuswMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    MMX_reg result;

    result.q = 0;
	if (dest->uw.w0>src->uw.w0) result.uw.w0 = dest->uw.w0 - src->uw.w0;
    if (dest->uw.w1>src->uw.w1) result.uw.w1 = dest->uw.w1 - src->uw.w1;
    if (dest->uw.w2>src->uw.w2) result.uw.w2 = dest->uw.w2 - src->uw.w2;
    if (dest->uw.w3>src->uw.w3) result.uw.w3 = dest->uw.w3 - src->uw.w3;
	dest->q = result.q;
}

void common_psubuswE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src = {};
    MMX_reg result = {};
    
    src.q = cpu->memory->readq(address);

    result.q = 0;
	if (dest->uw.w0>src.uw.w0) result.uw.w0 = dest->uw.w0 - src.uw.w0;
    if (dest->uw.w1>src.uw.w1) result.uw.w1 = dest->uw.w1 - src.uw.w1;
    if (dest->uw.w2>src.uw.w2) result.uw.w2 = dest->uw.w2 - src.uw.w2;
    if (dest->uw.w3>src.uw.w3) result.uw.w3 = dest->uw.w3 - src.uw.w3;
	dest->q = result.q;
}

void common_pmulhwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->uw.w0 = (U16)(((S32)dest->sw.w0 * (S32)src->sw.w0) >> 16);
	dest->uw.w1 = (U16)(((S32)dest->sw.w1 * (S32)src->sw.w1) >> 16);
	dest->uw.w2 = (U16)(((S32)dest->sw.w2 * (S32)src->sw.w2) >> 16);
	dest->uw.w3 = (U16)(((S32)dest->sw.w3 * (S32)src->sw.w3) >> 16);
}

void common_pmulhwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->uw.w0 = (U16)(((S32)dest->sw.w0 * (S32)src.sw.w0) >> 16);
	dest->uw.w1 = (U16)(((S32)dest->sw.w1 * (S32)src.sw.w1) >> 16);
	dest->uw.w2 = (U16)(((S32)dest->sw.w2 * (S32)src.sw.w2) >> 16);
	dest->uw.w3 = (U16)(((S32)dest->sw.w3 * (S32)src.sw.w3) >> 16);
}

void common_pmullwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->uw.w0 = (U16)((S32)dest->sw.w0 * (S32)src->sw.w0);
	dest->uw.w1 = (U16)((S32)dest->sw.w1 * (S32)src->sw.w1);
	dest->uw.w2 = (U16)((S32)dest->sw.w2 * (S32)src->sw.w2);
	dest->uw.w3 = (U16)((S32)dest->sw.w3 * (S32)src->sw.w3);
}

void common_pmullwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->uw.w0 = (U16)((S32)dest->sw.w0 * (S32)src.sw.w0);
	dest->uw.w1 = (U16)((S32)dest->sw.w1 * (S32)src.sw.w1);
	dest->uw.w2 = (U16)((S32)dest->sw.w2 * (S32)src.sw.w2);
	dest->uw.w3 = (U16)((S32)dest->sw.w3 * (S32)src.sw.w3);
}

void common_pmaddwdMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    if (dest->ud.d0 == 0x80008000 && src->ud.d0 == 0x80008000)
		dest->ud.d0 = 0x80000000;
	else
		dest->ud.d0 = (S32)dest->sw.w0 * (S32)src->sw.w0 + (S32)dest->sw.w1 * (S32)src->sw.w1;

	if (dest->ud.d1 == 0x80008000 && src->ud.d1 == 0x80008000)
		dest->ud.d1 = 0x80000000;
	else
		dest->sd.d1 = (S32)dest->sw.w2 * (S32)src->sw.w2 + (S32)dest->sw.w3 * (S32)src->sw.w3;
}

void common_pmaddwdE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	if (dest->ud.d0 == 0x80008000 && src.ud.d0 == 0x80008000)
		dest->ud.d0 = 0x80000000;
	else
		dest->ud.d0 = (S32)dest->sw.w0 * (S32)src.sw.w0 + (S32)dest->sw.w1 * (S32)src.sw.w1;

	if (dest->ud.d1 == 0x80008000 && src.ud.d1 == 0x80008000)
		dest->ud.d1 = 0x80000000;
	else
		dest->sd.d1 = (S32)dest->sw.w2 * (S32)src.sw.w2 + (S32)dest->sw.w3 * (S32)src.sw.w3;
}

/* Comparison */
void common_pcmpeqbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ub.b0 = dest->ub.b0==src->ub.b0?0xff:0;
	dest->ub.b1 = dest->ub.b1==src->ub.b1?0xff:0;
	dest->ub.b2 = dest->ub.b2==src->ub.b2?0xff:0;
	dest->ub.b3 = dest->ub.b3==src->ub.b3?0xff:0;
	dest->ub.b4 = dest->ub.b4==src->ub.b4?0xff:0;
	dest->ub.b5 = dest->ub.b5==src->ub.b5?0xff:0;
	dest->ub.b6 = dest->ub.b6==src->ub.b6?0xff:0;
	dest->ub.b7 = dest->ub.b7==src->ub.b7?0xff:0;
}

void common_pcmpeqbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ub.b0 = dest->ub.b0==src.ub.b0?0xff:0;
	dest->ub.b1 = dest->ub.b1==src.ub.b1?0xff:0;
	dest->ub.b2 = dest->ub.b2==src.ub.b2?0xff:0;
	dest->ub.b3 = dest->ub.b3==src.ub.b3?0xff:0;
	dest->ub.b4 = dest->ub.b4==src.ub.b4?0xff:0;
	dest->ub.b5 = dest->ub.b5==src.ub.b5?0xff:0;
	dest->ub.b6 = dest->ub.b6==src.ub.b6?0xff:0;
	dest->ub.b7 = dest->ub.b7==src.ub.b7?0xff:0;
}

void common_pcmpeqwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->uw.w0 = dest->uw.w0==src->uw.w0?0xffff:0;
	dest->uw.w1 = dest->uw.w1==src->uw.w1?0xffff:0;
	dest->uw.w2 = dest->uw.w2==src->uw.w2?0xffff:0;
	dest->uw.w3 = dest->uw.w3==src->uw.w3?0xffff:0;
}

void common_pcmpeqwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->uw.w0 = dest->uw.w0==src.uw.w0?0xffff:0;
	dest->uw.w1 = dest->uw.w1==src.uw.w1?0xffff:0;
	dest->uw.w2 = dest->uw.w2==src.uw.w2?0xffff:0;
	dest->uw.w3 = dest->uw.w3==src.uw.w3?0xffff:0;
}

void common_pcmpeqdMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ud.d0 = dest->ud.d0==src->ud.d0?0xffffffff:0;
	dest->ud.d1 = dest->ud.d1==src->ud.d1?0xffffffff:0;
}

void common_pcmpeqdE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ud.d0 = dest->ud.d0==src.ud.d0?0xffffffff:0;
	dest->ud.d1 = dest->ud.d1==src.ud.d1?0xffffffff:0;
}

void common_pcmpgtbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ub.b0 = dest->sb.b0>src->sb.b0?0xff:0;
	dest->ub.b1 = dest->sb.b1>src->sb.b1?0xff:0;
	dest->ub.b2 = dest->sb.b2>src->sb.b2?0xff:0;
	dest->ub.b3 = dest->sb.b3>src->sb.b3?0xff:0;
	dest->ub.b4 = dest->sb.b4>src->sb.b4?0xff:0;
	dest->ub.b5 = dest->sb.b5>src->sb.b5?0xff:0;
	dest->ub.b6 = dest->sb.b6>src->sb.b6?0xff:0;
	dest->ub.b7 = dest->sb.b7>src->sb.b7?0xff:0;
}

void common_pcmpgtbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ub.b0 = dest->sb.b0>src.sb.b0?0xff:0;
	dest->ub.b1 = dest->sb.b1>src.sb.b1?0xff:0;
	dest->ub.b2 = dest->sb.b2>src.sb.b2?0xff:0;
	dest->ub.b3 = dest->sb.b3>src.sb.b3?0xff:0;
	dest->ub.b4 = dest->sb.b4>src.sb.b4?0xff:0;
	dest->ub.b5 = dest->sb.b5>src.sb.b5?0xff:0;
	dest->ub.b6 = dest->sb.b6>src.sb.b6?0xff:0;
	dest->ub.b7 = dest->sb.b7>src.sb.b7?0xff:0;
}

void common_pcmpgtwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->uw.w0 = dest->sw.w0>src->sw.w0?0xffff:0;
	dest->uw.w1 = dest->sw.w1>src->sw.w1?0xffff:0;
	dest->uw.w2 = dest->sw.w2>src->sw.w2?0xffff:0;
	dest->uw.w3 = dest->sw.w3>src->sw.w3?0xffff:0;
}

void common_pcmpgtwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->uw.w0 = dest->sw.w0>src.sw.w0?0xffff:0;
	dest->uw.w1 = dest->sw.w1>src.sw.w1?0xffff:0;
	dest->uw.w2 = dest->sw.w2>src.sw.w2?0xffff:0;
	dest->uw.w3 = dest->sw.w3>src.sw.w3?0xffff:0;
}

void common_pcmpgtdMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ud.d0 = dest->sd.d0>src->sd.d0?0xffffffff:0;
	dest->ud.d1 = dest->sd.d1>src->sd.d1?0xffffffff:0;
}

void common_pcmpgtdE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ud.d0 = dest->sd.d0>src.sd.d0?0xffffffff:0;
	dest->ud.d1 = dest->sd.d1>src.sd.d1?0xffffffff:0;
}

/* Data Packing */
void common_packsswbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->sb.b0 = SaturateWordSToByteS(dest->sw.w0);
	dest->sb.b1 = SaturateWordSToByteS(dest->sw.w1);
	dest->sb.b2 = SaturateWordSToByteS(dest->sw.w2);
	dest->sb.b3 = SaturateWordSToByteS(dest->sw.w3);
	dest->sb.b4 = SaturateWordSToByteS(src->sw.w0);
	dest->sb.b5 = SaturateWordSToByteS(src->sw.w1);
	dest->sb.b6 = SaturateWordSToByteS(src->sw.w2);
	dest->sb.b7 = SaturateWordSToByteS(src->sw.w3);
}

void common_packsswbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->sb.b0 = SaturateWordSToByteS(dest->sw.w0);
	dest->sb.b1 = SaturateWordSToByteS(dest->sw.w1);
	dest->sb.b2 = SaturateWordSToByteS(dest->sw.w2);
	dest->sb.b3 = SaturateWordSToByteS(dest->sw.w3);
	dest->sb.b4 = SaturateWordSToByteS(src.sw.w0);
	dest->sb.b5 = SaturateWordSToByteS(src.sw.w1);
	dest->sb.b6 = SaturateWordSToByteS(src.sw.w2);
	dest->sb.b7 = SaturateWordSToByteS(src.sw.w3);
}

void common_packssdwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->sw.w0 = SaturateDwordSToWordS(dest->sd.d0);
	dest->sw.w1 = SaturateDwordSToWordS(dest->sd.d1);
	dest->sw.w2 = SaturateDwordSToWordS(src->sd.d0);
	dest->sw.w3 = SaturateDwordSToWordS(src->sd.d1);
}

void common_packssdwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->sw.w0 = SaturateDwordSToWordS(dest->sd.d0);
	dest->sw.w1 = SaturateDwordSToWordS(dest->sd.d1);
	dest->sw.w2 = SaturateDwordSToWordS(src.sd.d0);
	dest->sw.w3 = SaturateDwordSToWordS(src.sd.d1);
}

void common_packuswbMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ub.b0 = SaturateWordSToByteU(dest->sw.w0);
	dest->ub.b1 = SaturateWordSToByteU(dest->sw.w1);
	dest->ub.b2 = SaturateWordSToByteU(dest->sw.w2);
	dest->ub.b3 = SaturateWordSToByteU(dest->sw.w3);
	dest->ub.b4 = SaturateWordSToByteU(src->sw.w0);
	dest->ub.b5 = SaturateWordSToByteU(src->sw.w1);
	dest->ub.b6 = SaturateWordSToByteU(src->sw.w2);
	dest->ub.b7 = SaturateWordSToByteU(src->sw.w3);
}

void common_packuswbE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ub.b0 = SaturateWordSToByteU(dest->sw.w0);
	dest->ub.b1 = SaturateWordSToByteU(dest->sw.w1);
	dest->ub.b2 = SaturateWordSToByteU(dest->sw.w2);
	dest->ub.b3 = SaturateWordSToByteU(dest->sw.w3);
	dest->ub.b4 = SaturateWordSToByteU(src.sw.w0);
	dest->ub.b5 = SaturateWordSToByteU(src.sw.w1);
	dest->ub.b6 = SaturateWordSToByteU(src.sw.w2);
	dest->ub.b7 = SaturateWordSToByteU(src.sw.w3);
}

void common_punpckhbwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ub.b0 = dest->ub.b4;
	dest->ub.b1 = src->ub.b4;
	dest->ub.b2 = dest->ub.b5;
	dest->ub.b3 = src->ub.b5;
	dest->ub.b4 = dest->ub.b6;
	dest->ub.b5 = src->ub.b6;
	dest->ub.b6 = dest->ub.b7;
	dest->ub.b7 = src->ub.b7;
}

void common_punpckhbwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ub.b0 = dest->ub.b4;
	dest->ub.b1 = src.ub.b4;
	dest->ub.b2 = dest->ub.b5;
	dest->ub.b3 = src.ub.b5;
	dest->ub.b4 = dest->ub.b6;
	dest->ub.b5 = src.ub.b6;
	dest->ub.b6 = dest->ub.b7;
	dest->ub.b7 = src.ub.b7;
}

void common_punpckhwdMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->uw.w0 = dest->uw.w2;
	dest->uw.w1 = src->uw.w2;
	dest->uw.w2 = dest->uw.w3;
	dest->uw.w3 = src->uw.w3;
}

void common_punpckhwdE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->uw.w0 = dest->uw.w2;
	dest->uw.w1 = src.uw.w2;
	dest->uw.w2 = dest->uw.w3;
	dest->uw.w3 = src.uw.w3;
}

void common_punpckhdqMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ud.d0 = dest->ud.d1;
	dest->ud.d1 = src->ud.d1;
}

void common_punpckhdqE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ud.d0 = dest->ud.d1;
	dest->ud.d1 = src.ud.d1;
}

void common_punpcklbwMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ub.b7 = src->ub.b3;
	dest->ub.b6 = dest->ub.b3;
	dest->ub.b5 = src->ub.b2;
	dest->ub.b4 = dest->ub.b2;
	dest->ub.b3 = src->ub.b1;
	dest->ub.b2 = dest->ub.b1;
	dest->ub.b1 = src->ub.b0;
	dest->ub.b0 = dest->ub.b0;
}

void common_punpcklbwE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ub.b7 = src.ub.b3;
	dest->ub.b6 = dest->ub.b3;
	dest->ub.b5 = src.ub.b2;
	dest->ub.b4 = dest->ub.b2;
	dest->ub.b3 = src.ub.b1;
	dest->ub.b2 = dest->ub.b1;
	dest->ub.b1 = src.ub.b0;
	dest->ub.b0 = dest->ub.b0;
}

void common_punpcklwdMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->uw.w3 = src->uw.w1;
	dest->uw.w2 = dest->uw.w1;
	dest->uw.w1 = src->uw.w0;
	dest->uw.w0 = dest->uw.w0;
}

void common_punpcklwdE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->uw.w3 = src.uw.w1;
	dest->uw.w2 = dest->uw.w1;
	dest->uw.w1 = src.uw.w0;
	dest->uw.w0 = dest->uw.w0;
}

void common_punpckldqMmx(CPU* cpu, U32 r1, U32 r2) {
    MMX_reg* dest=&cpu->reg_mmx[r1];
    MMX_reg* src=&cpu->reg_mmx[r2];
    
    dest->ud.d1 = src->ud.d0;
}

void common_punpckldqE64(CPU* cpu, U32 reg, U32 address) {
    MMX_reg* dest=&cpu->reg_mmx[reg];
    MMX_reg src;    
    
    src.q = cpu->memory->readq(address);

	dest->ud.d1 = src.ud.d0;
}
