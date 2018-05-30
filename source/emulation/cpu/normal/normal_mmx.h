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
void OPCALL emms(CPU* cpu, DecodedOp* op) {
    cpu->fpu.reset();
}

/* Data Movement */

void OPCALL movPqR32(CPU* cpu, DecodedOp* op) {
    MMX_reg* rmrq=&cpu->reg_mmx[op->reg];
    rmrq->ud.d0 = cpu->reg[op->rm].u32;
    rmrq->ud.d1 = 0;
}

void OPCALL movPqE32(CPU* cpu, DecodedOp* op) {
    MMX_reg* rmrq=&cpu->reg_mmx[op->reg];
    rmrq->ud.d0 = readd(eaa(cpu, op));
    rmrq->ud.d1 = 0;
}

void OPCALL movR32Pq(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg_mmx[op->rm].ud.d0;
}

void OPCALL movE32Pq(CPU* cpu, DecodedOp* op) {
    writed(eaa(cpu, op), cpu->reg_mmx[op->reg].ud.d0);
}

void OPCALL movPqMmx(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q = cpu->reg_mmx[op->rm].q;
}

void OPCALL movPqE64(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q = readq(eaa(cpu, op));
}

void OPCALL movE64Pq(CPU* cpu, DecodedOp* op) {
    writeq(eaa(cpu, op), cpu->reg_mmx[op->reg].q);
}

void OPCALL movMmxPq(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q = cpu->reg_mmx[op->rm].q;
}

/* Boolean Logic */

void OPCALL pxorMmx(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q ^= cpu->reg_mmx[op->rm].q;
}

void OPCALL pxorE64(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q ^= readq(eaa(cpu, op));
}

void OPCALL porMmx(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q |= cpu->reg_mmx[op->rm].q;
}

void OPCALL porE64(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q |= readq(eaa(cpu, op));
}

void OPCALL pandMmx(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q &= cpu->reg_mmx[op->rm].q;
}

void OPCALL pandE64(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q &= readq(eaa(cpu, op));
}

void OPCALL pandnMmx(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q = ~cpu->reg_mmx[op->reg].q & cpu->reg_mmx[op->rm].q;
}

void OPCALL pandnE64(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q = ~cpu->reg_mmx[op->reg].q & readq(eaa(cpu, op));
}

/* Shift */
void OPCALL psllwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    if (src->q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 <<= src->ub.b0;
		dest->uw.w1 <<= src->ub.b0;
		dest->uw.w2 <<= src->ub.b0;
		dest->uw.w3 <<= src->ub.b0;
	}
}

void OPCALL psllwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 <<= src.ub.b0;
		dest->uw.w1 <<= src.ub.b0;
		dest->uw.w2 <<= src.ub.b0;
		dest->uw.w3 <<= src.ub.b0;
	}
}

void OPCALL psrlwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    if (src->q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 >>= src->ub.b0;
		dest->uw.w1 >>= src->ub.b0;
		dest->uw.w2 >>= src->ub.b0;
		dest->uw.w3 >>= src->ub.b0;
	}
}

void OPCALL psrlwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 >>= src.ub.b0;
		dest->uw.w1 >>= src.ub.b0;
		dest->uw.w2 >>= src.ub.b0;
		dest->uw.w3 >>= src.ub.b0;
	}
}

void OPCALL psrawMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    U8 shift;

    if (cpu->reg_mmx[op->rm].q > 15) {
        shift = 16;
    } else {
        shift = cpu->reg_mmx[op->rm].ub.b0;
    }
	dest->sw.w0 >>= shift;
	dest->sw.w1 >>= shift;
	dest->sw.w2 >>= shift;
	dest->sw.w3 >>= shift;
}

void OPCALL psrawE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 15) {
        src.q = 16;
    }
	dest->sw.w0 >>= src.ub.b0;
	dest->sw.w1 >>= src.ub.b0;
	dest->sw.w2 >>= src.ub.b0;
	dest->sw.w3 >>= src.ub.b0;
}

void OPCALL psllw(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    
    dest->uw.w0 >>= (U8)op->imm;
	dest->uw.w1 >>= (U8)op->imm;
	dest->uw.w2 >>= (U8)op->imm;
	dest->uw.w3 >>= (U8)op->imm;	
}

void OPCALL psraw(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    
    dest->sw.w0 >>= (U8)op->imm;
	dest->sw.w1 >>= (U8)op->imm;
	dest->sw.w2 >>= (U8)op->imm;
	dest->sw.w3 >>= (U8)op->imm;	
}

void OPCALL psrlw(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    
    dest->uw.w0 <<= (U8)op->imm;
	dest->uw.w1 <<= (U8)op->imm;
	dest->uw.w2 <<= (U8)op->imm;
	dest->uw.w3 <<= (U8)op->imm;	
}

void OPCALL pslldMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    if (src->q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 <<= src->ub.b0;
		dest->ud.d1 <<= src->ub.b0;
	}
}

void OPCALL pslldE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 <<= src.ub.b0;
		dest->ud.d1 <<= src.ub.b0;
	}
}

void OPCALL psrldMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    if (src->q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 >>= src->ub.b0;
		dest->ud.d1 >>= src->ub.b0;
	}
}

void OPCALL psrldE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 >>= src.ub.b0;
		dest->ud.d1 >>= src.ub.b0;
	}
}

void OPCALL psradMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    U8 shift;

    if (cpu->reg_mmx[op->rm].q > 31) {
        shift = 32;
    } else {
        shift = cpu->reg_mmx[op->rm].ub.b0;
    }
	dest->sd.d0 >>= shift;
	dest->sd.d1 >>= shift;	
}

void OPCALL psradE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 31) {
        src.q = 32;
    }
	dest->sd.d0 >>= src.ub.b0;
	dest->sd.d1 >>= src.ub.b0;	
}

void OPCALL pslld(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    
    dest->ud.d0 >>= (U8)op->imm;
	dest->ud.d1 >>= (U8)op->imm;
}

void OPCALL psrad(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    
    dest->sd.d0 >>= (U8)op->imm;
	dest->sd.d1 >>= (U8)op->imm;
}

void OPCALL psrld(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    
    dest->ud.d0 <<= (U8)op->imm;
	dest->ud.d1 <<= (U8)op->imm;
}

void OPCALL psllqMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    if (src->q > 63) {
        dest->q = 0;
    } else {
		dest->q <<= src->ub.b0;
	}
}

void OPCALL psllqE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q <<= src.ub.b0;
	}
}

void OPCALL psrlqMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    if (src->q > 63) {
        dest->q = 0;
    } else {
		dest->q >>= src->ub.b0;
	}
}

void OPCALL psrlqE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q >>= src.ub.b0;
	}
}

void OPCALL psllq(CPU* cpu, DecodedOp* op) {    
    cpu->reg_mmx[op->reg].q >>= (U8)op->imm;
}

void OPCALL psrlq(CPU* cpu, DecodedOp* op) {
    cpu->reg_mmx[op->reg].q <<= (U8)op->imm;
}

/* Math */
void OPCALL paddbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->ub.b0 += src->ub.b0;
	dest->ub.b1 += src->ub.b1;
	dest->ub.b2 += src->ub.b2;
	dest->ub.b3 += src->ub.b3;
	dest->ub.b4 += src->ub.b4;
	dest->ub.b5 += src->ub.b5;
	dest->ub.b6 += src->ub.b6;
	dest->ub.b7 += src->ub.b7;
}

void OPCALL paddbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->ub.b0 += src.ub.b0;
	dest->ub.b1 += src.ub.b1;
	dest->ub.b2 += src.ub.b2;
	dest->ub.b3 += src.ub.b3;
	dest->ub.b4 += src.ub.b4;
	dest->ub.b5 += src.ub.b5;
	dest->ub.b6 += src.ub.b6;
	dest->ub.b7 += src.ub.b7;
}

void OPCALL paddwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->uw.w0 += src->uw.w0;
	dest->uw.w1 += src->uw.w1;
	dest->uw.w2 += src->uw.w2;
	dest->uw.w3 += src->uw.w3;
}

void OPCALL paddwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->uw.w0 += src.uw.w0;
	dest->uw.w1 += src.uw.w1;
	dest->uw.w2 += src.uw.w2;
	dest->uw.w3 += src.uw.w3;
}

void OPCALL padddMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->ud.d0 += src->ud.d0;
	dest->ud.d1 += src->ud.d1;
}

void OPCALL padddE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->ud.d0 += src.ud.d0;
	dest->ud.d1 += src.ud.d1;
}

void OPCALL paddsbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0+(S16)src->sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1+(S16)src->sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2+(S16)src->sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3+(S16)src->sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4+(S16)src->sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5+(S16)src->sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6+(S16)src->sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7+(S16)src->sb.b7);
}

void OPCALL paddsbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0+(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1+(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2+(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3+(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4+(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5+(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6+(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7+(S16)src.sb.b7);
}

void OPCALL paddswMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0+(S32)src->sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1+(S32)src->sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2+(S32)src->sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3+(S32)src->sw.w3);
}

void OPCALL paddswE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0+(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1+(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2+(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3+(S32)src.sw.w3);
}

void OPCALL paddusbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->ub.b0 = SaturateWordSToByteU((S16)dest->ub.b0+(S16)src->ub.b0);
	dest->ub.b1 = SaturateWordSToByteU((S16)dest->ub.b1+(S16)src->ub.b1);
	dest->ub.b2 = SaturateWordSToByteU((S16)dest->ub.b2+(S16)src->ub.b2);
	dest->ub.b3 = SaturateWordSToByteU((S16)dest->ub.b3+(S16)src->ub.b3);
	dest->ub.b4 = SaturateWordSToByteU((S16)dest->ub.b4+(S16)src->ub.b4);
	dest->ub.b5 = SaturateWordSToByteU((S16)dest->ub.b5+(S16)src->ub.b5);
	dest->ub.b6 = SaturateWordSToByteU((S16)dest->ub.b6+(S16)src->ub.b6);
	dest->ub.b7 = SaturateWordSToByteU((S16)dest->ub.b7+(S16)src->ub.b7);
}

void OPCALL paddusbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->ub.b0 = SaturateWordSToByteU((S16)dest->ub.b0+(S16)src.ub.b0);
	dest->ub.b1 = SaturateWordSToByteU((S16)dest->ub.b1+(S16)src.ub.b1);
	dest->ub.b2 = SaturateWordSToByteU((S16)dest->ub.b2+(S16)src.ub.b2);
	dest->ub.b3 = SaturateWordSToByteU((S16)dest->ub.b3+(S16)src.ub.b3);
	dest->ub.b4 = SaturateWordSToByteU((S16)dest->ub.b4+(S16)src.ub.b4);
	dest->ub.b5 = SaturateWordSToByteU((S16)dest->ub.b5+(S16)src.ub.b5);
	dest->ub.b6 = SaturateWordSToByteU((S16)dest->ub.b6+(S16)src.ub.b6);
	dest->ub.b7 = SaturateWordSToByteU((S16)dest->ub.b7+(S16)src.ub.b7);
}

void OPCALL padduswMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->uw.w0 = SaturateDwordSToWordU((S32)dest->uw.w0+(S32)src->uw.w0);
	dest->uw.w1 = SaturateDwordSToWordU((S32)dest->uw.w1+(S32)src->uw.w1);
	dest->uw.w2 = SaturateDwordSToWordU((S32)dest->uw.w2+(S32)src->uw.w2);
	dest->uw.w3 = SaturateDwordSToWordU((S32)dest->uw.w3+(S32)src->uw.w3);
}

void OPCALL padduswE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->uw.w0 = SaturateDwordSToWordU((S32)dest->uw.w0+(S32)src.uw.w0);
	dest->uw.w1 = SaturateDwordSToWordU((S32)dest->uw.w1+(S32)src.uw.w1);
	dest->uw.w2 = SaturateDwordSToWordU((S32)dest->uw.w2+(S32)src.uw.w2);
	dest->uw.w3 = SaturateDwordSToWordU((S32)dest->uw.w3+(S32)src.uw.w3);
}

void OPCALL psubbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->ub.b0 -= src->ub.b0;
	dest->ub.b1 -= src->ub.b1;
	dest->ub.b2 -= src->ub.b2;
	dest->ub.b3 -= src->ub.b3;
	dest->ub.b4 -= src->ub.b4;
	dest->ub.b5 -= src->ub.b5;
	dest->ub.b6 -= src->ub.b6;
	dest->ub.b7 -= src->ub.b7;
}

void OPCALL psubbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->ub.b0 -= src.ub.b0;
	dest->ub.b1 -= src.ub.b1;
	dest->ub.b2 -= src.ub.b2;
	dest->ub.b3 -= src.ub.b3;
	dest->ub.b4 -= src.ub.b4;
	dest->ub.b5 -= src.ub.b5;
	dest->ub.b6 -= src.ub.b6;
	dest->ub.b7 -= src.ub.b7;
}

void OPCALL psubwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->uw.w0 -= src->uw.w0;
	dest->uw.w1 -= src->uw.w1;
	dest->uw.w2 -= src->uw.w2;
	dest->uw.w3 -= src->uw.w3;
}

void OPCALL psubwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->uw.w0 -= src.uw.w0;
	dest->uw.w1 -= src.uw.w1;
	dest->uw.w2 -= src.uw.w2;
	dest->uw.w3 -= src.uw.w3;
}

void OPCALL psubdMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->ud.d0 -= src->ud.d0;
	dest->ud.d1 -= src->ud.d1;
}

void OPCALL psubdE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->ud.d0 -= src.ud.d0;
	dest->ud.d1 -= src.ud.d1;
}

void OPCALL psubsbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0-(S16)src->sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1-(S16)src->sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2-(S16)src->sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3-(S16)src->sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4-(S16)src->sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5-(S16)src->sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6-(S16)src->sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7-(S16)src->sb.b7);
}

void OPCALL psubsbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0-(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1-(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2-(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3-(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4-(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5-(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6-(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7-(S16)src.sb.b7);
}

void OPCALL psubswMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0-(S32)src->sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1-(S32)src->sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2-(S32)src->sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3-(S32)src->sw.w3);
}

void OPCALL psubswE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    
    src.q = readq(eaa(cpu, op));

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0-(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1-(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2-(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3-(S32)src.sw.w3);
}

void OPCALL psubusbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
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

void OPCALL psubusbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    MMX_reg result;
    
    src.q = readq(eaa(cpu, op));

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

void OPCALL psubuswMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
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

void OPCALL psubuswE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;
    MMX_reg result;
    
    src.q = readq(eaa(cpu, op));

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

void OPCALL pmulhwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->uw.w0 = (U16)(((S32)dest->sw.w0 * (S32)src->sw.w0) >> 16);
	dest->uw.w1 = (U16)(((S32)dest->sw.w1 * (S32)src->sw.w1) >> 16);
	dest->uw.w2 = (U16)(((S32)dest->sw.w2 * (S32)src->sw.w2) >> 16);
	dest->uw.w3 = (U16)(((S32)dest->sw.w3 * (S32)src->sw.w3) >> 16);
}

void OPCALL pmulhwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->uw.w0 = (U16)(((S32)dest->sw.w0 * (S32)src.sw.w0) >> 16);
	dest->uw.w1 = (U16)(((S32)dest->sw.w1 * (S32)src.sw.w1) >> 16);
	dest->uw.w2 = (U16)(((S32)dest->sw.w2 * (S32)src.sw.w2) >> 16);
	dest->uw.w3 = (U16)(((S32)dest->sw.w3 * (S32)src.sw.w3) >> 16);
}

void OPCALL pmullwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->uw.w0 = (U16)((S32)dest->sw.w0 * (S32)src->sw.w0);
	dest->uw.w1 = (U16)((S32)dest->sw.w1 * (S32)src->sw.w1);
	dest->uw.w2 = (U16)((S32)dest->sw.w2 * (S32)src->sw.w2);
	dest->uw.w3 = (U16)((S32)dest->sw.w3 * (S32)src->sw.w3);
}

void OPCALL pmullwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->uw.w0 = (U16)((S32)dest->sw.w0 * (S32)src.sw.w0);
	dest->uw.w1 = (U16)((S32)dest->sw.w1 * (S32)src.sw.w1);
	dest->uw.w2 = (U16)((S32)dest->sw.w2 * (S32)src.sw.w2);
	dest->uw.w3 = (U16)((S32)dest->sw.w3 * (S32)src.sw.w3);
}

void OPCALL pmaddwdMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    if (dest->ud.d0 == 0x80008000 && src->ud.d0 == 0x80008000)
		dest->ud.d0 = 0x80000000;
	else
		dest->ud.d0 = (S32)dest->sw.w0 * (S32)src->sw.w0 + (S32)dest->sw.w1 * (S32)src->sw.w1;

	if (dest->ud.d1 == 0x80008000 && src->ud.d1 == 0x80008000)
		dest->ud.d1 = 0x80000000;
	else
		dest->sd.d1 = (S32)dest->sw.w2 * (S32)src->sw.w2 + (S32)dest->sw.w3 * (S32)src->sw.w3;
}

void OPCALL pmaddwdE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

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
void OPCALL pcmpeqbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ub.b0 = dest->ub.b0==src->ub.b0?0xff:0;
	dest->ub.b1 = dest->ub.b1==src->ub.b1?0xff:0;
	dest->ub.b2 = dest->ub.b2==src->ub.b2?0xff:0;
	dest->ub.b3 = dest->ub.b3==src->ub.b3?0xff:0;
	dest->ub.b4 = dest->ub.b4==src->ub.b4?0xff:0;
	dest->ub.b5 = dest->ub.b5==src->ub.b5?0xff:0;
	dest->ub.b6 = dest->ub.b6==src->ub.b6?0xff:0;
	dest->ub.b7 = dest->ub.b7==src->ub.b7?0xff:0;
}

void OPCALL pcmpeqbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ub.b0 = dest->ub.b0==src.ub.b0?0xff:0;
	dest->ub.b1 = dest->ub.b1==src.ub.b1?0xff:0;
	dest->ub.b2 = dest->ub.b2==src.ub.b2?0xff:0;
	dest->ub.b3 = dest->ub.b3==src.ub.b3?0xff:0;
	dest->ub.b4 = dest->ub.b4==src.ub.b4?0xff:0;
	dest->ub.b5 = dest->ub.b5==src.ub.b5?0xff:0;
	dest->ub.b6 = dest->ub.b6==src.ub.b6?0xff:0;
	dest->ub.b7 = dest->ub.b7==src.ub.b7?0xff:0;
}

void OPCALL pcmpeqwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->uw.w0 = dest->uw.w0==src->uw.w0?0xffff:0;
	dest->uw.w1 = dest->uw.w1==src->uw.w1?0xffff:0;
	dest->uw.w2 = dest->uw.w2==src->uw.w2?0xffff:0;
	dest->uw.w3 = dest->uw.w3==src->uw.w3?0xffff:0;
}

void OPCALL pcmpeqwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->uw.w0 = dest->uw.w0==src.uw.w0?0xffff:0;
	dest->uw.w1 = dest->uw.w1==src.uw.w1?0xffff:0;
	dest->uw.w2 = dest->uw.w2==src.uw.w2?0xffff:0;
	dest->uw.w3 = dest->uw.w3==src.uw.w3?0xffff:0;
}

void OPCALL pcmpeqdMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ud.d0 = dest->ud.d0==src->ud.d0?0xffffffff:0;
	dest->ud.d1 = dest->ud.d1==src->ud.d1?0xffffffff:0;
}

void OPCALL pcmpeqdE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ud.d0 = dest->ud.d0==src.ud.d0?0xffffffff:0;
	dest->ud.d1 = dest->ud.d1==src.ud.d1?0xffffffff:0;
}

void OPCALL pcmpgtbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ub.b0 = dest->sb.b0>src->sb.b0?0xff:0;
	dest->ub.b1 = dest->sb.b1>src->sb.b1?0xff:0;
	dest->ub.b2 = dest->sb.b2>src->sb.b2?0xff:0;
	dest->ub.b3 = dest->sb.b3>src->sb.b3?0xff:0;
	dest->ub.b4 = dest->sb.b4>src->sb.b4?0xff:0;
	dest->ub.b5 = dest->sb.b5>src->sb.b5?0xff:0;
	dest->ub.b6 = dest->sb.b6>src->sb.b6?0xff:0;
	dest->ub.b7 = dest->sb.b7>src->sb.b7?0xff:0;
}

void OPCALL pcmpgtbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ub.b0 = dest->sb.b0>src.sb.b0?0xff:0;
	dest->ub.b1 = dest->sb.b1>src.sb.b1?0xff:0;
	dest->ub.b2 = dest->sb.b2>src.sb.b2?0xff:0;
	dest->ub.b3 = dest->sb.b3>src.sb.b3?0xff:0;
	dest->ub.b4 = dest->sb.b4>src.sb.b4?0xff:0;
	dest->ub.b5 = dest->sb.b5>src.sb.b5?0xff:0;
	dest->ub.b6 = dest->sb.b6>src.sb.b6?0xff:0;
	dest->ub.b7 = dest->sb.b7>src.sb.b7?0xff:0;
}

void OPCALL pcmpgtwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->uw.w0 = dest->sw.w0>src->sw.w0?0xffff:0;
	dest->uw.w1 = dest->sw.w1>src->sw.w1?0xffff:0;
	dest->uw.w2 = dest->sw.w2>src->sw.w2?0xffff:0;
	dest->uw.w3 = dest->sw.w3>src->sw.w3?0xffff:0;
}

void OPCALL pcmpgtwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->uw.w0 = dest->sw.w0>src.sw.w0?0xffff:0;
	dest->uw.w1 = dest->sw.w1>src.sw.w1?0xffff:0;
	dest->uw.w2 = dest->sw.w2>src.sw.w2?0xffff:0;
	dest->uw.w3 = dest->sw.w3>src.sw.w3?0xffff:0;
}

void OPCALL pcmpgtdMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ud.d0 = dest->sd.d0>src->sd.d0?0xffffffff:0;
	dest->ud.d1 = dest->sd.d1>src->sd.d1?0xffffffff:0;
}

void OPCALL pcmpgtdE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ud.d0 = dest->sd.d0>src.sd.d0?0xffffffff:0;
	dest->ud.d1 = dest->sd.d1>src.sd.d1?0xffffffff:0;
}

/* Data Packing */
void OPCALL packsswbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->sb.b0 = SaturateWordSToByteS(dest->sw.w0);
	dest->sb.b1 = SaturateWordSToByteS(dest->sw.w1);
	dest->sb.b2 = SaturateWordSToByteS(dest->sw.w2);
	dest->sb.b3 = SaturateWordSToByteS(dest->sw.w3);
	dest->sb.b4 = SaturateWordSToByteS(src->sw.w0);
	dest->sb.b5 = SaturateWordSToByteS(src->sw.w1);
	dest->sb.b6 = SaturateWordSToByteS(src->sw.w2);
	dest->sb.b7 = SaturateWordSToByteS(src->sw.w3);
}

void OPCALL packsswbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->sb.b0 = SaturateWordSToByteS(dest->sw.w0);
	dest->sb.b1 = SaturateWordSToByteS(dest->sw.w1);
	dest->sb.b2 = SaturateWordSToByteS(dest->sw.w2);
	dest->sb.b3 = SaturateWordSToByteS(dest->sw.w3);
	dest->sb.b4 = SaturateWordSToByteS(src.sw.w0);
	dest->sb.b5 = SaturateWordSToByteS(src.sw.w1);
	dest->sb.b6 = SaturateWordSToByteS(src.sw.w2);
	dest->sb.b7 = SaturateWordSToByteS(src.sw.w3);
}

void OPCALL packssdwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->sw.w0 = SaturateDwordSToWordS(dest->sd.d0);
	dest->sw.w1 = SaturateDwordSToWordS(dest->sd.d1);
	dest->sw.w2 = SaturateDwordSToWordS(src->sd.d0);
	dest->sw.w3 = SaturateDwordSToWordS(src->sd.d1);
}

void OPCALL packssdwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->sw.w0 = SaturateDwordSToWordS(dest->sd.d0);
	dest->sw.w1 = SaturateDwordSToWordS(dest->sd.d1);
	dest->sw.w2 = SaturateDwordSToWordS(src.sd.d0);
	dest->sw.w3 = SaturateDwordSToWordS(src.sd.d1);
}

void OPCALL packuswbMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ub.b0 = SaturateWordSToByteU(dest->sw.w0);
	dest->ub.b1 = SaturateWordSToByteU(dest->sw.w1);
	dest->ub.b2 = SaturateWordSToByteU(dest->sw.w2);
	dest->ub.b3 = SaturateWordSToByteU(dest->sw.w3);
	dest->ub.b4 = SaturateWordSToByteU(src->sw.w0);
	dest->ub.b5 = SaturateWordSToByteU(src->sw.w1);
	dest->ub.b6 = SaturateWordSToByteU(src->sw.w2);
	dest->ub.b7 = SaturateWordSToByteU(src->sw.w3);
}

void OPCALL packuswbE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ub.b0 = SaturateWordSToByteU(dest->sw.w0);
	dest->ub.b1 = SaturateWordSToByteU(dest->sw.w1);
	dest->ub.b2 = SaturateWordSToByteU(dest->sw.w2);
	dest->ub.b3 = SaturateWordSToByteU(dest->sw.w3);
	dest->ub.b4 = SaturateWordSToByteU(src.sw.w0);
	dest->ub.b5 = SaturateWordSToByteU(src.sw.w1);
	dest->ub.b6 = SaturateWordSToByteU(src.sw.w2);
	dest->ub.b7 = SaturateWordSToByteU(src.sw.w3);
}

void OPCALL punpckhbwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ub.b0 = dest->ub.b4;
	dest->ub.b1 = src->ub.b4;
	dest->ub.b2 = dest->ub.b5;
	dest->ub.b3 = src->ub.b5;
	dest->ub.b4 = dest->ub.b6;
	dest->ub.b5 = src->ub.b6;
	dest->ub.b6 = dest->ub.b7;
	dest->ub.b7 = src->ub.b7;
}

void OPCALL punpckhbwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ub.b0 = dest->ub.b4;
	dest->ub.b1 = src.ub.b4;
	dest->ub.b2 = dest->ub.b5;
	dest->ub.b3 = src.ub.b5;
	dest->ub.b4 = dest->ub.b6;
	dest->ub.b5 = src.ub.b6;
	dest->ub.b6 = dest->ub.b7;
	dest->ub.b7 = src.ub.b7;
}

void OPCALL punpckhwdMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->uw.w0 = dest->uw.w2;
	dest->uw.w1 = src->uw.w2;
	dest->uw.w2 = dest->uw.w3;
	dest->uw.w3 = src->uw.w3;
}

void OPCALL punpckhwdE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->uw.w0 = dest->uw.w2;
	dest->uw.w1 = src.uw.w2;
	dest->uw.w2 = dest->uw.w3;
	dest->uw.w3 = src.uw.w3;
}

void OPCALL punpckhdqMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ud.d0 = dest->ud.d1;
	dest->ud.d1 = src->ud.d1;
}

void OPCALL punpckhdqE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ud.d0 = dest->ud.d1;
	dest->ud.d1 = src.ud.d1;
}

void OPCALL punpcklbwMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ub.b7 = src->ub.b3;
	dest->ub.b6 = dest->ub.b3;
	dest->ub.b5 = src->ub.b2;
	dest->ub.b4 = dest->ub.b2;
	dest->ub.b3 = src->ub.b1;
	dest->ub.b2 = dest->ub.b1;
	dest->ub.b1 = src->ub.b0;
	dest->ub.b0 = dest->ub.b0;
}

void OPCALL punpcklbwE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ub.b7 = src.ub.b3;
	dest->ub.b6 = dest->ub.b3;
	dest->ub.b5 = src.ub.b2;
	dest->ub.b4 = dest->ub.b2;
	dest->ub.b3 = src.ub.b1;
	dest->ub.b2 = dest->ub.b1;
	dest->ub.b1 = src.ub.b0;
	dest->ub.b0 = dest->ub.b0;
}

void OPCALL punpcklwdMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->uw.w3 = src->uw.w1;
	dest->uw.w2 = dest->uw.w1;
	dest->uw.w1 = src->uw.w0;
	dest->uw.w0 = dest->uw.w0;
}

void OPCALL punpcklwdE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->uw.w3 = src.uw.w1;
	dest->uw.w2 = dest->uw.w1;
	dest->uw.w1 = src.uw.w0;
	dest->uw.w0 = dest->uw.w0;
}

void OPCALL punpckldqMmx(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg* src=&cpu->reg_mmx[op->rm];
    
    dest->ud.d1 = src->ud.d0;
}

void OPCALL punpckldqE64(CPU* cpu, DecodedOp* op) {
    MMX_reg* dest=&cpu->reg_mmx[op->reg];
    MMX_reg src;    
    
    src.q = readq(eaa(cpu, op));

	dest->ud.d1 = src.ud.d0;
}
