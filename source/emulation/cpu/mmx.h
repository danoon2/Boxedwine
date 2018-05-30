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

void setFPU(struct FPU* fpu, U16 tag) {
	FPU_SET_TOP(fpu, 0);
	fpu->top=FPU_GET_TOP(fpu);
	FPU_SetTag(fpu, tag);
}

const char* MMX(int r) {
    switch (r) {
    case 0: return "MM1";
    case 1: return "MM2";
    case 2: return "MM3";
    case 3: return "MM4";
    case 4: return "MM5";
    case 5: return "MM6";
    case 6: return "MM7";
    case 7: return "MM8";
    default: return "0";
    }
}

#ifdef LOG_OPS
void LOG_MMX_IMM(struct DecodeData* data, const char* name, int rm, U32 imm) {
    char tmpm[32];
    sprintf(tmpm, "%X", imm);
    LOG_OP2(name, MMX(rm), tmpm);
}
#else
#define LOG_MMX_IMM(w, x,y,z)
#endif

/* State Management */

void OPCALL mmx_emms(struct CPU* cpu, struct Op* op) {
    setFPU(&cpu->fpu, TAG_Empty);
    CYCLES(1);
    NEXT();
}

// EMMS
void decode377(struct DecodeData* data) {
    data->op->func = mmx_emms;
    LOG_OP("EMMS");
    NEXT_OP(data);
}

/* Data Movement */

void OPCALL mov_mmx_reg(struct CPU* cpu, struct Op* op) {
    MMX_reg* rmrq=&reg_mmx[op->r1];
    rmrq->ud.d0 = cpu->reg[op->r2].u32;
    rmrq->ud.d1 = 0;
    CYCLES(1);
    NEXT();
}

void OPCALL mov_mmx_e32_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* rmrq=&reg_mmx[op->r1];
    rmrq->ud.d0 = readd(cpu->thread, eaa16(cpu, op));
    rmrq->ud.d1 = 0;
    CYCLES(1);
    NEXT();
}

void OPCALL mov_mmx_e32_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* rmrq=&reg_mmx[op->r1];
    rmrq->ud.d0 = readd(cpu->thread, eaa32(cpu, op));
    rmrq->ud.d1 = 0;
    CYCLES(1);
    NEXT();
}

// MOVD Pq,Ed
void decode36e(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = mov_mmx_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOV", MMX(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = mov_mmx_e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", MMX(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = mov_mmx_e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", MMX(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL mov_reg_mmx(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = reg_mmx[op->r2].ud.d0;
    CYCLES(1);
    NEXT();
}

void OPCALL mov_e32_mmx_16(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa16(cpu, op), reg_mmx[op->r1].ud.d0);
    CYCLES(2);
    NEXT();
}

void OPCALL mov_e32_mmx_32(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa32(cpu, op), reg_mmx[op->r1].ud.d0);
    CYCLES(2);
    NEXT();
}

// MOVD Ed,Pq
void decode37e(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = mov_reg_mmx;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", R32(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = mov_e32_mmx_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", M32(data, rm, data->op),MMX(data->op->r1));
    } else {
        data->op->func = mov_e32_mmx_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", M32(data, rm, data->op),MMX(data->op->r1));
    }
    NEXT_OP(data);
}

void OPCALL mov_mmx_mmx(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q = reg_mmx[op->r2].q;
    CYCLES(1);
    NEXT();
}

void OPCALL mov_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q = readq(cpu->thread, eaa16(cpu, op));
    CYCLES(2);
    NEXT();
}

void OPCALL mov_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q = readq(cpu->thread, eaa32(cpu, op));
    CYCLES(2);
    NEXT();
}

// MOVD Pq,Qq
void decode36f(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = mov_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOV", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = mov_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = mov_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL mov_e64_mmx_16(struct CPU* cpu, struct Op* op) {
    writeq(cpu->thread, eaa16(cpu, op), reg_mmx[op->r1].q);
    CYCLES(2);
    NEXT();
}

void OPCALL mov_e64_mmx_32(struct CPU* cpu, struct Op* op) {
    writeq(cpu->thread, eaa32(cpu, op), reg_mmx[op->r1].q);
    CYCLES(2);
    NEXT();
}

// MOVQ Qq,Pq
void decode37f(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = mov_mmx_mmx;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = mov_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", M64(data, rm, data->op), MMX(data->op->r1));
    } else {
        data->op->func = mov_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", M64(data, rm, data->op), MMX(data->op->r1));
    }
    NEXT_OP(data);
}

/* Boolean Logic */

void OPCALL xor_mmx_mmx(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q ^= reg_mmx[op->r2].q;
    CYCLES(1);
    NEXT();
}

void OPCALL xor_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q ^= readq(cpu->thread, eaa16(cpu, op));
    CYCLES(2);
    NEXT();
}

void OPCALL xor_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q ^= readq(cpu->thread, eaa32(cpu, op));
    CYCLES(2);
    NEXT();
}

// PXOR Pq,Qq
void decode3ef(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xor_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PXOR", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xor_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PXOR", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = xor_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PXOR", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL or_mmx_mmx(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q |= reg_mmx[op->r2].q;
    CYCLES(1);
    NEXT();
}

void OPCALL or_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q |= readq(cpu->thread, eaa16(cpu, op));
    CYCLES(2);
    NEXT();
}

void OPCALL or_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q |= readq(cpu->thread, eaa32(cpu, op));
    CYCLES(2);
    NEXT();
}

// POR Pq,Qq
void decode3eb(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = or_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("POR", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = or_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("POR", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = or_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("POR", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL and_mmx_mmx(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q &= reg_mmx[op->r2].q;
    CYCLES(1);
    NEXT();
}

void OPCALL and_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q &= readq(cpu->thread, eaa16(cpu, op));
    CYCLES(2);
    NEXT();
}

void OPCALL and_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q &= readq(cpu->thread, eaa32(cpu, op));
    CYCLES(2);
    NEXT();
}

// PAND Pq,Qq
void decode3db(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = and_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PAND", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = and_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PAND", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = and_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PAND", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL andn_mmx_mmx(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q = ~reg_mmx[op->r1].q & reg_mmx[op->r2].q;
    CYCLES(1);
    NEXT();
}

void OPCALL andn_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q = ~reg_mmx[op->r1].q & readq(cpu->thread, eaa16(cpu, op));
    CYCLES(2);
    NEXT();
}

void OPCALL andn_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q = ~reg_mmx[op->r1].q & readq(cpu->thread, eaa32(cpu, op));
    CYCLES(2);
    NEXT();
}

// PANDN Pq,Qq
void decode3df(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = andn_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PANDN", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = andn_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PANDN", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = andn_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PANDN", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

/* Shift */
void OPCALL psllw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    if (src->q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 <<= src->ub.b0;
		dest->uw.w1 <<= src->ub.b0;
		dest->uw.w2 <<= src->ub.b0;
		dest->uw.w3 <<= src->ub.b0;
	}

    CYCLES(1);
    NEXT();
}

void OPCALL psllw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 <<= src.ub.b0;
		dest->uw.w1 <<= src.ub.b0;
		dest->uw.w2 <<= src.ub.b0;
		dest->uw.w3 <<= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

void OPCALL psllw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 <<= src.ub.b0;
		dest->uw.w1 <<= src.ub.b0;
		dest->uw.w2 <<= src.ub.b0;
		dest->uw.w3 <<= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

// PSLLW Pq,Qq
void decode3f1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psllw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSLLW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psllw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSLLW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psllw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSLLW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psrlw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    if (src->q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 >>= src->ub.b0;
		dest->uw.w1 >>= src->ub.b0;
		dest->uw.w2 >>= src->ub.b0;
		dest->uw.w3 >>= src->ub.b0;
	}

    CYCLES(1);
    NEXT();
}

void OPCALL psrlw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 >>= src.ub.b0;
		dest->uw.w1 >>= src.ub.b0;
		dest->uw.w2 >>= src.ub.b0;
		dest->uw.w3 >>= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

void OPCALL psrlw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 15) {
        dest->q = 0;
    } else {
		dest->uw.w0 >>= src.ub.b0;
		dest->uw.w1 >>= src.ub.b0;
		dest->uw.w2 >>= src.ub.b0;
		dest->uw.w3 >>= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

// PSRLW Pq,Qq
void decode3d1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psrlw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSRLW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psrlw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSRLW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psrlw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSRLW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psraw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    U8 shift;

    if (reg_mmx[op->r2].q > 15) {
        shift = 16;
    } else {
        shift = reg_mmx[op->r2].ub.b0;
    }
	dest->sw.w0 >>= shift;
	dest->sw.w1 >>= shift;
	dest->sw.w2 >>= shift;
	dest->sw.w3 >>= shift;
	

    CYCLES(1);
    NEXT();
}

void OPCALL psraw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 15) {
        src.q = 16;
    }
	dest->sw.w0 >>= src.ub.b0;
	dest->sw.w1 >>= src.ub.b0;
	dest->sw.w2 >>= src.ub.b0;
	dest->sw.w3 >>= src.ub.b0;
	

    CYCLES(2);
    NEXT();
}

void OPCALL psraw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 15) {
        src.q = 16;
    }
	dest->sw.w0 >>= src.ub.b0;
	dest->sw.w1 >>= src.ub.b0;
	dest->sw.w2 >>= src.ub.b0;
	dest->sw.w3 >>= src.ub.b0;
	

    CYCLES(2);
    NEXT();
}

// PSRAW Pq,Qq
void decode3e1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psraw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSRAW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psraw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSRAW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psraw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSRAW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psllw_mmx_ib(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    
    dest->uw.w0 >>= (U8)op->data1;
	dest->uw.w1 >>= (U8)op->data1;
	dest->uw.w2 >>= (U8)op->data1;
	dest->uw.w3 >>= (U8)op->data1;	

    CYCLES(1);
    NEXT();
}

void OPCALL psraw_mmx_ib(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    
    dest->sw.w0 >>= (U8)op->data1;
	dest->sw.w1 >>= (U8)op->data1;
	dest->sw.w2 >>= (U8)op->data1;
	dest->sw.w3 >>= (U8)op->data1;	

    CYCLES(1);
    NEXT();
}

void OPCALL psrlw_mmx_ib(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    
    dest->uw.w0 <<= (U8)op->data1;
	dest->uw.w1 <<= (U8)op->data1;
	dest->uw.w2 <<= (U8)op->data1;
	dest->uw.w3 <<= (U8)op->data1;	

    CYCLES(1);
    NEXT();
}

void OPCALL mmx_0(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q = 0;
    CYCLES(1);
    NEXT();
}

// PSLLW/PSRLW/PSRAW Pq,Ib
void decode371(struct DecodeData* data) {
    U8 rm = FETCH8(data);

    data->op->data1 = FETCH8(data);
    data->op->r1 = E(rm);
    switch ((rm >> 3) & 7) {
        case 0x06: 	// PSLLW
            LOG_MMX_IMM(data, "PSLLW", data->op->r1, data->op->data1);
            if (data->op->data1>15)
                data->op->func = mmx_0;
            else
                data->op->func = psllw_mmx_ib;            
            break;
        case 0x02: 	// PSRLW
            LOG_MMX_IMM(data, "PSRLW", data->op->r1, data->op->data1);
            if (data->op->data1>15)
                data->op->func = mmx_0;
            else
                data->op->func = psrlw_mmx_ib;            
            break;
        case 0x04: 	// PSRAW
            LOG_MMX_IMM(data, "PSRAW", data->op->r1, data->op->data1);
            if (data->op->data1>15)
                data->op->data1 = 16;
            data->op->func = psraw_mmx_ib;            
            break;
    }
    NEXT_OP(data);
}

void OPCALL pslld_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    if (src->q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 <<= src->ub.b0;
		dest->ud.d1 <<= src->ub.b0;
	}

    CYCLES(1);
    NEXT();
}

void OPCALL pslld_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 <<= src.ub.b0;
		dest->ud.d1 <<= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

void OPCALL pslld_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 <<= src.ub.b0;
		dest->ud.d1 <<= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

// PSLLD Pq,Qq
void decode3f2(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pslld_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSLLD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pslld_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSLLD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pslld_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSLLD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psrld_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    if (src->q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 >>= src->ub.b0;
		dest->ud.d1 >>= src->ub.b0;
	}

    CYCLES(1);
    NEXT();
}

void OPCALL psrld_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 >>= src.ub.b0;
		dest->ud.d1 >>= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

void OPCALL psrld_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 31) {
        dest->q = 0;
    } else {
		dest->ud.d0 >>= src.ub.b0;
		dest->ud.d1 >>= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

// PSRLD Pq,Qq
void decode3d2(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psrld_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSRLD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psrld_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSRLD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psrld_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSRLD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psrad_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    U8 shift;

    if (reg_mmx[op->r2].q > 31) {
        shift = 32;
    } else {
        shift = reg_mmx[op->r2].ub.b0;
    }
	dest->sd.d0 >>= shift;
	dest->sd.d1 >>= shift;	

    CYCLES(1);
    NEXT();
}

void OPCALL psrad_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 31) {
        src.q = 32;
    }
	dest->sd.d0 >>= src.ub.b0;
	dest->sd.d1 >>= src.ub.b0;	

    CYCLES(2);
    NEXT();
}

void OPCALL psrad_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 31) {
        src.q = 32;
    }
	dest->sd.d0 >>= src.ub.b0;
	dest->sd.d1 >>= src.ub.b0;
	
    CYCLES(2);
    NEXT();
}

// PSRAD Pq,Qq
void decode3e2(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psrad_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSRAD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psrad_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSRAD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psrad_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSRAD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pslld_mmx_ib(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    
    dest->ud.d0 >>= (U8)op->data1;
	dest->ud.d1 >>= (U8)op->data1;

    CYCLES(1);
    NEXT();
}

void OPCALL psrad_mmx_ib(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    
    dest->sd.d0 >>= (U8)op->data1;
	dest->sd.d1 >>= (U8)op->data1;

    CYCLES(1);
    NEXT();
}

void OPCALL psrld_mmx_ib(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    
    dest->ud.d0 <<= (U8)op->data1;
	dest->ud.d1 <<= (U8)op->data1;

    CYCLES(1);
    NEXT();
}

// PSLLD/PSRLD/PSRAD Pq,Ib
void decode372(struct DecodeData* data) {
    U8 rm = FETCH8(data);

    data->op->data1 = FETCH8(data);
    data->op->r1 = E(rm);
    switch ((rm >> 3) & 7) {
        case 0x06: 	// PSLLD
            LOG_MMX_IMM(data, "PSLLD", data->op->r1, data->op->data1);
            if (data->op->data1>31)
                data->op->func = mmx_0;
            else
                data->op->func = pslld_mmx_ib;            
            break;
        case 0x02: 	// PSRLD
            LOG_MMX_IMM(data, "PSRLD", data->op->r1, data->op->data1);
            if (data->op->data1>31)
                data->op->func = mmx_0;
            else
                data->op->func = psrld_mmx_ib;
            break;
        case 0x04: 	// PSRAD
            LOG_MMX_IMM(data, "PSRAD", data->op->r1, data->op->data1);
            if (data->op->data1>31)
                data->op->data1 = 32;
            data->op->func = psrad_mmx_ib;            
            break;
    }
    NEXT_OP(data);
}

void OPCALL psllq_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    if (src->q > 63) {
        dest->q = 0;
    } else {
		dest->q <<= src->ub.b0;
	}

    CYCLES(1);
    NEXT();
}

void OPCALL psllq_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q <<= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

void OPCALL psllq_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q <<= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

// PSLLQ Pq,Qq
void decode3f3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psllq_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSLLQ", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psllq_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSLLQ", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psllq_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSLLQ", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psrlq_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    if (src->q > 63) {
        dest->q = 0;
    } else {
		dest->q >>= src->ub.b0;
	}

    CYCLES(1);
    NEXT();
}

void OPCALL psrlq_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q >>= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

void OPCALL psrlq_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    if (src.q > 63) {
        dest->q = 0;
    } else {
		dest->q >>= src.ub.b0;
	}

    CYCLES(2);
    NEXT();
}

// PSRLQ Pq,Qq
void decode3d3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psrlq_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSRLQ", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psrlq_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSRLQ", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psrlq_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSRLQ", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psllq_mmx_ib(struct CPU* cpu, struct Op* op) {    
    reg_mmx[op->r1].q >>= (U8)op->data1;

    CYCLES(1);
    NEXT();
}

void OPCALL psrlq_mmx_ib(struct CPU* cpu, struct Op* op) {
    reg_mmx[op->r1].q <<= (U8)op->data1;

    CYCLES(1);
    NEXT();
}

// PSLLQ/PSRLQ Pq,Ib
void decode373(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->data1 = FETCH8(data);
    data->op->r1 = E(rm);

    if (data->op->data1 > 63)
        data->op->func = mmx_0;

    switch ((rm >> 3) & 7) {
        case 0x06: 	// PSLLQ
            LOG_MMX_IMM(data, "PSLLQ", data->op->r1, data->op->data1);
            if (data->op->data1>63)
                data->op->func = mmx_0;
            else
                data->op->func = psllq_mmx_ib;            
            break;
        case 0x02: 	// PSRLQ
            LOG_MMX_IMM(data, "PSRLQ", data->op->r1, data->op->data1);
            if (data->op->data1>63)
                data->op->func = mmx_0;
            else
                data->op->func = psrlq_mmx_ib;
            break;
    }
    NEXT_OP(data);
}

/* Math */
void OPCALL paddb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->ub.b0 += src->ub.b0;
	dest->ub.b1 += src->ub.b1;
	dest->ub.b2 += src->ub.b2;
	dest->ub.b3 += src->ub.b3;
	dest->ub.b4 += src->ub.b4;
	dest->ub.b5 += src->ub.b5;
	dest->ub.b6 += src->ub.b6;
	dest->ub.b7 += src->ub.b7;

    CYCLES(1);
    NEXT();
}

void OPCALL paddb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->ub.b0 += src.ub.b0;
	dest->ub.b1 += src.ub.b1;
	dest->ub.b2 += src.ub.b2;
	dest->ub.b3 += src.ub.b3;
	dest->ub.b4 += src.ub.b4;
	dest->ub.b5 += src.ub.b5;
	dest->ub.b6 += src.ub.b6;
	dest->ub.b7 += src.ub.b7;

    CYCLES(2);
    NEXT();
}

void OPCALL paddb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->ub.b0 += src.ub.b0;
	dest->ub.b1 += src.ub.b1;
	dest->ub.b2 += src.ub.b2;
	dest->ub.b3 += src.ub.b3;
	dest->ub.b4 += src.ub.b4;
	dest->ub.b5 += src.ub.b5;
	dest->ub.b6 += src.ub.b6;
	dest->ub.b7 += src.ub.b7;

    CYCLES(2);
    NEXT();
}

// PADDB Pq,Qq
void decode3fc(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = paddb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PADDB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = paddb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PADDB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = paddb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PADDB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL paddw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->uw.w0 += src->uw.w0;
	dest->uw.w1 += src->uw.w1;
	dest->uw.w2 += src->uw.w2;
	dest->uw.w3 += src->uw.w3;

    CYCLES(1);
    NEXT();
}

void OPCALL paddw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->uw.w0 += src.uw.w0;
	dest->uw.w1 += src.uw.w1;
	dest->uw.w2 += src.uw.w2;
	dest->uw.w3 += src.uw.w3;

    CYCLES(2);
    NEXT();
}

void OPCALL paddw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->uw.w0 += src.uw.w0;
	dest->uw.w1 += src.uw.w1;
	dest->uw.w2 += src.uw.w2;
	dest->uw.w3 += src.uw.w3;

    CYCLES(2);
    NEXT();
}

// PADDW Pq,Qq
void decode3fd(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = paddw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PADDW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = paddw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PADDW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = paddw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PADDW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL paddd_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->ud.d0 += src->ud.d0;
	dest->ud.d1 += src->ud.d1;

    CYCLES(1);
    NEXT();
}

void OPCALL paddd_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->ud.d0 += src.ud.d0;
	dest->ud.d1 += src.ud.d1;

    CYCLES(2);
    NEXT();
}

void OPCALL paddd_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->ud.d0 += src.ud.d0;
	dest->ud.d1 += src.ud.d1;

    CYCLES(2);
    NEXT();
}

// PADDD Pq,Qq
void decode3fe(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = paddd_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PADDD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = paddd_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PADDD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = paddd_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PADDD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL paddsb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0+(S16)src->sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1+(S16)src->sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2+(S16)src->sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3+(S16)src->sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4+(S16)src->sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5+(S16)src->sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6+(S16)src->sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7+(S16)src->sb.b7);

    CYCLES(1);
    NEXT();
}

void OPCALL paddsb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0+(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1+(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2+(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3+(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4+(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5+(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6+(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7+(S16)src.sb.b7);

    CYCLES(2);
    NEXT();
}

void OPCALL paddsb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0+(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1+(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2+(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3+(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4+(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5+(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6+(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7+(S16)src.sb.b7);

    CYCLES(2);
    NEXT();
}

// PADDSB Pq,Qq
void decode3ec(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = paddsb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PADDSB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = paddsb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PADDSB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = paddsb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PADDSB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL paddsw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0+(S32)src->sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1+(S32)src->sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2+(S32)src->sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3+(S32)src->sw.w3);

    CYCLES(1);
    NEXT();
}

void OPCALL paddsw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0+(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1+(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2+(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3+(S32)src.sw.w3);

    CYCLES(2);
    NEXT();
}

void OPCALL paddsw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0+(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1+(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2+(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3+(S32)src.sw.w3);

    CYCLES(2);
    NEXT();
}

// PADDSW Pq,Qq
void decode3ed(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = paddsw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PADDSW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = paddsw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PADDSW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = paddsw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PADDSW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL paddusb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->ub.b0 = SaturateWordSToByteU((S16)dest->ub.b0+(S16)src->ub.b0);
	dest->ub.b1 = SaturateWordSToByteU((S16)dest->ub.b1+(S16)src->ub.b1);
	dest->ub.b2 = SaturateWordSToByteU((S16)dest->ub.b2+(S16)src->ub.b2);
	dest->ub.b3 = SaturateWordSToByteU((S16)dest->ub.b3+(S16)src->ub.b3);
	dest->ub.b4 = SaturateWordSToByteU((S16)dest->ub.b4+(S16)src->ub.b4);
	dest->ub.b5 = SaturateWordSToByteU((S16)dest->ub.b5+(S16)src->ub.b5);
	dest->ub.b6 = SaturateWordSToByteU((S16)dest->ub.b6+(S16)src->ub.b6);
	dest->ub.b7 = SaturateWordSToByteU((S16)dest->ub.b7+(S16)src->ub.b7);

    CYCLES(1);
    NEXT();
}

void OPCALL paddusb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->ub.b0 = SaturateWordSToByteU((S16)dest->ub.b0+(S16)src.ub.b0);
	dest->ub.b1 = SaturateWordSToByteU((S16)dest->ub.b1+(S16)src.ub.b1);
	dest->ub.b2 = SaturateWordSToByteU((S16)dest->ub.b2+(S16)src.ub.b2);
	dest->ub.b3 = SaturateWordSToByteU((S16)dest->ub.b3+(S16)src.ub.b3);
	dest->ub.b4 = SaturateWordSToByteU((S16)dest->ub.b4+(S16)src.ub.b4);
	dest->ub.b5 = SaturateWordSToByteU((S16)dest->ub.b5+(S16)src.ub.b5);
	dest->ub.b6 = SaturateWordSToByteU((S16)dest->ub.b6+(S16)src.ub.b6);
	dest->ub.b7 = SaturateWordSToByteU((S16)dest->ub.b7+(S16)src.ub.b7);

    CYCLES(2);
    NEXT();
}

void OPCALL paddusb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->ub.b0 = SaturateWordSToByteU((S16)dest->ub.b0+(S16)src.ub.b0);
	dest->ub.b1 = SaturateWordSToByteU((S16)dest->ub.b1+(S16)src.ub.b1);
	dest->ub.b2 = SaturateWordSToByteU((S16)dest->ub.b2+(S16)src.ub.b2);
	dest->ub.b3 = SaturateWordSToByteU((S16)dest->ub.b3+(S16)src.ub.b3);
	dest->ub.b4 = SaturateWordSToByteU((S16)dest->ub.b4+(S16)src.ub.b4);
	dest->ub.b5 = SaturateWordSToByteU((S16)dest->ub.b5+(S16)src.ub.b5);
	dest->ub.b6 = SaturateWordSToByteU((S16)dest->ub.b6+(S16)src.ub.b6);
	dest->ub.b7 = SaturateWordSToByteU((S16)dest->ub.b7+(S16)src.ub.b7);

    CYCLES(2);
    NEXT();
}

// PADDUSB Pq,Qq
void decode3dc(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = paddusb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PADDUSB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = paddusb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PADDUSB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = paddusb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PADDUSB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL paddusw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->uw.w0 = SaturateDwordSToWordU((S32)dest->uw.w0+(S32)src->uw.w0);
	dest->uw.w1 = SaturateDwordSToWordU((S32)dest->uw.w1+(S32)src->uw.w1);
	dest->uw.w2 = SaturateDwordSToWordU((S32)dest->uw.w2+(S32)src->uw.w2);
	dest->uw.w3 = SaturateDwordSToWordU((S32)dest->uw.w3+(S32)src->uw.w3);

    CYCLES(1);
    NEXT();
}

void OPCALL paddusw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->uw.w0 = SaturateDwordSToWordU((S32)dest->uw.w0+(S32)src.uw.w0);
	dest->uw.w1 = SaturateDwordSToWordU((S32)dest->uw.w1+(S32)src.uw.w1);
	dest->uw.w2 = SaturateDwordSToWordU((S32)dest->uw.w2+(S32)src.uw.w2);
	dest->uw.w3 = SaturateDwordSToWordU((S32)dest->uw.w3+(S32)src.uw.w3);

    CYCLES(2);
    NEXT();
}

void OPCALL paddusw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->uw.w0 = SaturateDwordSToWordU((S32)dest->uw.w0+(S32)src.uw.w0);
	dest->uw.w1 = SaturateDwordSToWordU((S32)dest->uw.w1+(S32)src.uw.w1);
	dest->uw.w2 = SaturateDwordSToWordU((S32)dest->uw.w2+(S32)src.uw.w2);
	dest->uw.w3 = SaturateDwordSToWordU((S32)dest->uw.w3+(S32)src.uw.w3);

    CYCLES(2);
    NEXT();
}

// PADDUSW Pq,Qq
void decode3dd(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = paddusw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PADDUSW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = paddusw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PADDUSW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = paddusw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PADDUSW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psubb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->ub.b0 -= src->ub.b0;
	dest->ub.b1 -= src->ub.b1;
	dest->ub.b2 -= src->ub.b2;
	dest->ub.b3 -= src->ub.b3;
	dest->ub.b4 -= src->ub.b4;
	dest->ub.b5 -= src->ub.b5;
	dest->ub.b6 -= src->ub.b6;
	dest->ub.b7 -= src->ub.b7;

    CYCLES(1);
    NEXT();
}

void OPCALL psubb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->ub.b0 -= src.ub.b0;
	dest->ub.b1 -= src.ub.b1;
	dest->ub.b2 -= src.ub.b2;
	dest->ub.b3 -= src.ub.b3;
	dest->ub.b4 -= src.ub.b4;
	dest->ub.b5 -= src.ub.b5;
	dest->ub.b6 -= src.ub.b6;
	dest->ub.b7 -= src.ub.b7;

    CYCLES(2);
    NEXT();
}

void OPCALL psubb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->ub.b0 -= src.ub.b0;
	dest->ub.b1 -= src.ub.b1;
	dest->ub.b2 -= src.ub.b2;
	dest->ub.b3 -= src.ub.b3;
	dest->ub.b4 -= src.ub.b4;
	dest->ub.b5 -= src.ub.b5;
	dest->ub.b6 -= src.ub.b6;
	dest->ub.b7 -= src.ub.b7;

    CYCLES(2);
    NEXT();
}

// PSUBB Pq,Qq
void decode3f8(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psubb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSUBB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psubb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSUBB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psubb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSUBB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psubw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->uw.w0 -= src->uw.w0;
	dest->uw.w1 -= src->uw.w1;
	dest->uw.w2 -= src->uw.w2;
	dest->uw.w3 -= src->uw.w3;

    CYCLES(1);
    NEXT();
}

void OPCALL psubw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->uw.w0 -= src.uw.w0;
	dest->uw.w1 -= src.uw.w1;
	dest->uw.w2 -= src.uw.w2;
	dest->uw.w3 -= src.uw.w3;

    CYCLES(2);
    NEXT();
}

void OPCALL psubw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->uw.w0 -= src.uw.w0;
	dest->uw.w1 -= src.uw.w1;
	dest->uw.w2 -= src.uw.w2;
	dest->uw.w3 -= src.uw.w3;

    CYCLES(2);
    NEXT();
}

// PSUBW Pq,Qq
void decode3f9(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psubw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSUBW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psubw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSUBW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psubw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSUBW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psubd_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->ud.d0 -= src->ud.d0;
	dest->ud.d1 -= src->ud.d1;

    CYCLES(1);
    NEXT();
}

void OPCALL psubd_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->ud.d0 -= src.ud.d0;
	dest->ud.d1 -= src.ud.d1;

    CYCLES(2);
    NEXT();
}

void OPCALL psubd_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->ud.d0 -= src.ud.d0;
	dest->ud.d1 -= src.ud.d1;

    CYCLES(2);
    NEXT();
}

// PSUBD Pq,Qq
void decode3fa(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psubd_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSUBD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psubd_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSUBD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psubd_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSUBD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psubsb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0-(S16)src->sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1-(S16)src->sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2-(S16)src->sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3-(S16)src->sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4-(S16)src->sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5-(S16)src->sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6-(S16)src->sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7-(S16)src->sb.b7);

    CYCLES(1);
    NEXT();
}

void OPCALL psubsb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0-(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1-(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2-(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3-(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4-(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5-(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6-(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7-(S16)src.sb.b7);

    CYCLES(2);
    NEXT();
}

void OPCALL psubsb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->sb.b0 = SaturateWordSToByteS((S16)dest->sb.b0-(S16)src.sb.b0);
	dest->sb.b1 = SaturateWordSToByteS((S16)dest->sb.b1-(S16)src.sb.b1);
	dest->sb.b2 = SaturateWordSToByteS((S16)dest->sb.b2-(S16)src.sb.b2);
	dest->sb.b3 = SaturateWordSToByteS((S16)dest->sb.b3-(S16)src.sb.b3);
	dest->sb.b4 = SaturateWordSToByteS((S16)dest->sb.b4-(S16)src.sb.b4);
	dest->sb.b5 = SaturateWordSToByteS((S16)dest->sb.b5-(S16)src.sb.b5);
	dest->sb.b6 = SaturateWordSToByteS((S16)dest->sb.b6-(S16)src.sb.b6);
	dest->sb.b7 = SaturateWordSToByteS((S16)dest->sb.b7-(S16)src.sb.b7);

    CYCLES(2);
    NEXT();
}

// PSUBSB Pq,Qq
void decode3e8(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psubsb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSUBSB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psubsb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSUBSB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psubsb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSUBSB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psubsw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0-(S32)src->sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1-(S32)src->sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2-(S32)src->sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3-(S32)src->sw.w3);

    CYCLES(1);
    NEXT();
}

void OPCALL psubsw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0-(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1-(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2-(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3-(S32)src.sw.w3);

    CYCLES(2);
    NEXT();
}

void OPCALL psubsw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    dest->sw.w0 = SaturateDwordSToWordS((S32)dest->sw.w0-(S32)src.sw.w0);
	dest->sw.w1 = SaturateDwordSToWordS((S32)dest->sw.w1-(S32)src.sw.w1);
	dest->sw.w2 = SaturateDwordSToWordS((S32)dest->sw.w2-(S32)src.sw.w2);
	dest->sw.w3 = SaturateDwordSToWordS((S32)dest->sw.w3-(S32)src.sw.w3);

    CYCLES(2);
    NEXT();
}

// PSUBSW Pq,Qq
void decode3e9(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psubsw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSUBSW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psubsw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSUBSW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psubsw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSUBSW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psubusb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
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

    CYCLES(1);
    NEXT();
}

void OPCALL psubusb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    MMX_reg result;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

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

    CYCLES(2);
    NEXT();
}

void OPCALL psubusb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    MMX_reg result;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

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

    CYCLES(2);
    NEXT();
}

// PSUBUSB Pq,Qq
void decode3d8(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psubusb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSUBUSB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psubusb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSUBUSB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psubusb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSUBUSB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL psubusw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
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

    CYCLES(1);
    NEXT();
}

void OPCALL psubusw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    MMX_reg result;
    
    src.q = readq(cpu->thread, eaa16(cpu, op));

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

    CYCLES(2);
    NEXT();
}

void OPCALL psubusw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;
    MMX_reg result;
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

    result.q = 0;
	if (dest->uw.w0>src.uw.w0) result.uw.w0 = dest->uw.w0 - src.uw.w0;
	if (dest->uw.w1>src.uw.w1) result.uw.w1 = dest->uw.w1 - src.uw.w1;
	if (dest->uw.w2>src.uw.w2) result.uw.w2 = dest->uw.w2 - src.uw.w2;
	if (dest->uw.w3>src.uw.w3) result.uw.w3 = dest->uw.w3 - src.uw.w3;
	dest->q = result.q;

    CYCLES(2);
    NEXT();
}

// PSUBUSW Pq,Qq
void decode3d9(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = psubusw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PSUBUSW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = psubusw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PSUBUSW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = psubusw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PSUBUSW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pmulhw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->uw.w0 = (U16)(((S32)dest->sw.w0 * (S32)src->sw.w0) >> 16);
	dest->uw.w1 = (U16)(((S32)dest->sw.w1 * (S32)src->sw.w1) >> 16);
	dest->uw.w2 = (U16)(((S32)dest->sw.w2 * (S32)src->sw.w2) >> 16);
	dest->uw.w3 = (U16)(((S32)dest->sw.w3 * (S32)src->sw.w3) >> 16);

    CYCLES(1);
    NEXT();
}

void OPCALL pmulhw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = (U16)(((S32)dest->sw.w0 * (S32)src.sw.w0) >> 16);
	dest->uw.w1 = (U16)(((S32)dest->sw.w1 * (S32)src.sw.w1) >> 16);
	dest->uw.w2 = (U16)(((S32)dest->sw.w2 * (S32)src.sw.w2) >> 16);
	dest->uw.w3 = (U16)(((S32)dest->sw.w3 * (S32)src.sw.w3) >> 16);

    CYCLES(2);
    NEXT();
}

void OPCALL pmulhw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = (U16)(((S32)dest->sw.w0 * (S32)src.sw.w0) >> 16);
	dest->uw.w1 = (U16)(((S32)dest->sw.w1 * (S32)src.sw.w1) >> 16);
	dest->uw.w2 = (U16)(((S32)dest->sw.w2 * (S32)src.sw.w2) >> 16);
	dest->uw.w3 = (U16)(((S32)dest->sw.w3 * (S32)src.sw.w3) >> 16);

    CYCLES(2);
    NEXT();
}

// PMULHW Pq,Qq
void decode3e5(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pmulhw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PMULHW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pmulhw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PMULHW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pmulhw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PMULHW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pmullw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->uw.w0 = (U16)((S32)dest->sw.w0 * (S32)src->sw.w0);
	dest->uw.w1 = (U16)((S32)dest->sw.w1 * (S32)src->sw.w1);
	dest->uw.w2 = (U16)((S32)dest->sw.w2 * (S32)src->sw.w2);
	dest->uw.w3 = (U16)((S32)dest->sw.w3 * (S32)src->sw.w3);

    CYCLES(1);
    NEXT();
}

void OPCALL pmullw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = (U16)((S32)dest->sw.w0 * (S32)src.sw.w0);
	dest->uw.w1 = (U16)((S32)dest->sw.w1 * (S32)src.sw.w1);
	dest->uw.w2 = (U16)((S32)dest->sw.w2 * (S32)src.sw.w2);
	dest->uw.w3 = (U16)((S32)dest->sw.w3 * (S32)src.sw.w3);

    CYCLES(2);
    NEXT();
}

void OPCALL pmullw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = (U16)((S32)dest->sw.w0 * (S32)src.sw.w0);
	dest->uw.w1 = (U16)((S32)dest->sw.w1 * (S32)src.sw.w1);
	dest->uw.w2 = (U16)((S32)dest->sw.w2 * (S32)src.sw.w2);
	dest->uw.w3 = (U16)((S32)dest->sw.w3 * (S32)src.sw.w3);

    CYCLES(2);
    NEXT();
}

// PMULLW Pq,Qq
void decode3d5(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pmullw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PMULLW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pmullw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PMULLW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pmullw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PMULLW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pmaddwd_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    if (dest->ud.d0 == 0x80008000 && src->ud.d0 == 0x80008000)
		dest->ud.d0 = 0x80000000;
	else
		dest->ud.d0 = (S32)dest->sw.w0 * (S32)src->sw.w0 + (S32)dest->sw.w1 * (S32)src->sw.w1;

	if (dest->ud.d1 == 0x80008000 && src->ud.d1 == 0x80008000)
		dest->ud.d1 = 0x80000000;
	else
		dest->sd.d1 = (S32)dest->sw.w2 * (S32)src->sw.w2 + (S32)dest->sw.w3 * (S32)src->sw.w3;

    CYCLES(1);
    NEXT();
}

void OPCALL pmaddwd_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	if (dest->ud.d0 == 0x80008000 && src.ud.d0 == 0x80008000)
		dest->ud.d0 = 0x80000000;
	else
		dest->ud.d0 = (S32)dest->sw.w0 * (S32)src.sw.w0 + (S32)dest->sw.w1 * (S32)src.sw.w1;

	if (dest->ud.d1 == 0x80008000 && src.ud.d1 == 0x80008000)
		dest->ud.d1 = 0x80000000;
	else
		dest->sd.d1 = (S32)dest->sw.w2 * (S32)src.sw.w2 + (S32)dest->sw.w3 * (S32)src.sw.w3;

    CYCLES(2);
    NEXT();
}

void OPCALL pmaddwd_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	if (dest->ud.d0 == 0x80008000 && src.ud.d0 == 0x80008000)
		dest->ud.d0 = 0x80000000;
	else
		dest->ud.d0 = (S32)dest->sw.w0 * (S32)src.sw.w0 + (S32)dest->sw.w1 * (S32)src.sw.w1;

	if (dest->ud.d1 == 0x80008000 && src.ud.d1 == 0x80008000)
		dest->ud.d1 = 0x80000000;
	else
		dest->sd.d1 = (S32)dest->sw.w2 * (S32)src.sw.w2 + (S32)dest->sw.w3 * (S32)src.sw.w3;

    CYCLES(2);
    NEXT();
}

// PMADDWD Pq,Qq
void decode3f5(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pmaddwd_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PMADDWD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pmaddwd_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PMADDWD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pmaddwd_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PMADDWD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

/* Comparison */
void OPCALL pcmpeqb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ub.b0 = dest->ub.b0==src->ub.b0?0xff:0;
	dest->ub.b1 = dest->ub.b1==src->ub.b1?0xff:0;
	dest->ub.b2 = dest->ub.b2==src->ub.b2?0xff:0;
	dest->ub.b3 = dest->ub.b3==src->ub.b3?0xff:0;
	dest->ub.b4 = dest->ub.b4==src->ub.b4?0xff:0;
	dest->ub.b5 = dest->ub.b5==src->ub.b5?0xff:0;
	dest->ub.b6 = dest->ub.b6==src->ub.b6?0xff:0;
	dest->ub.b7 = dest->ub.b7==src->ub.b7?0xff:0;

    CYCLES(1);
    NEXT();
}

void OPCALL pcmpeqb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = dest->ub.b0==src.ub.b0?0xff:0;
	dest->ub.b1 = dest->ub.b1==src.ub.b1?0xff:0;
	dest->ub.b2 = dest->ub.b2==src.ub.b2?0xff:0;
	dest->ub.b3 = dest->ub.b3==src.ub.b3?0xff:0;
	dest->ub.b4 = dest->ub.b4==src.ub.b4?0xff:0;
	dest->ub.b5 = dest->ub.b5==src.ub.b5?0xff:0;
	dest->ub.b6 = dest->ub.b6==src.ub.b6?0xff:0;
	dest->ub.b7 = dest->ub.b7==src.ub.b7?0xff:0;

    CYCLES(2);
    NEXT();
}

void OPCALL pcmpeqb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = dest->ub.b0==src.ub.b0?0xff:0;
	dest->ub.b1 = dest->ub.b1==src.ub.b1?0xff:0;
	dest->ub.b2 = dest->ub.b2==src.ub.b2?0xff:0;
	dest->ub.b3 = dest->ub.b3==src.ub.b3?0xff:0;
	dest->ub.b4 = dest->ub.b4==src.ub.b4?0xff:0;
	dest->ub.b5 = dest->ub.b5==src.ub.b5?0xff:0;
	dest->ub.b6 = dest->ub.b6==src.ub.b6?0xff:0;
	dest->ub.b7 = dest->ub.b7==src.ub.b7?0xff:0;

    CYCLES(2);
    NEXT();
}

// PCMPEQB Pq,Qq
void decode374(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pcmpeqb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PCMPEQB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pcmpeqb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PCMPEQB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pcmpeqb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PCMPEQB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pcmpeqw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->uw.w0 = dest->uw.w0==src->uw.w0?0xffff:0;
	dest->uw.w1 = dest->uw.w1==src->uw.w1?0xffff:0;
	dest->uw.w2 = dest->uw.w2==src->uw.w2?0xffff:0;
	dest->uw.w3 = dest->uw.w3==src->uw.w3?0xffff:0;

    CYCLES(1);
    NEXT();
}

void OPCALL pcmpeqw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = dest->uw.w0==src.uw.w0?0xffff:0;
	dest->uw.w1 = dest->uw.w1==src.uw.w1?0xffff:0;
	dest->uw.w2 = dest->uw.w2==src.uw.w2?0xffff:0;
	dest->uw.w3 = dest->uw.w3==src.uw.w3?0xffff:0;

    CYCLES(2);
    NEXT();
}

void OPCALL pcmpeqw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = dest->uw.w0==src.uw.w0?0xffff:0;
	dest->uw.w1 = dest->uw.w1==src.uw.w1?0xffff:0;
	dest->uw.w2 = dest->uw.w2==src.uw.w2?0xffff:0;
	dest->uw.w3 = dest->uw.w3==src.uw.w3?0xffff:0;

    CYCLES(2);
    NEXT();
}

// PCMPEQW Pq,Qq
void decode375(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pcmpeqw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PCMPEQW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pcmpeqw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PCMPEQW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pcmpeqw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PCMPEQW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pcmpeqd_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ud.d0 = dest->ud.d0==src->ud.d0?0xffffffff:0;
	dest->ud.d1 = dest->ud.d1==src->ud.d1?0xffffffff:0;

    CYCLES(1);
    NEXT();
}

void OPCALL pcmpeqd_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d0 = dest->ud.d0==src.ud.d0?0xffffffff:0;
	dest->ud.d1 = dest->ud.d1==src.ud.d1?0xffffffff:0;

    CYCLES(2);
    NEXT();
}

void OPCALL pcmpeqd_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d0 = dest->ud.d0==src.ud.d0?0xffffffff:0;
	dest->ud.d1 = dest->ud.d1==src.ud.d1?0xffffffff:0;

    CYCLES(2);
    NEXT();
}

// PCMPEQD Pq,Qq
void decode376(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pcmpeqd_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PCMPEQD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pcmpeqd_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PCMPEQD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pcmpeqd_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PCMPEQD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pcmpgtb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ub.b0 = dest->sb.b0>src->sb.b0?0xff:0;
	dest->ub.b1 = dest->sb.b1>src->sb.b1?0xff:0;
	dest->ub.b2 = dest->sb.b2>src->sb.b2?0xff:0;
	dest->ub.b3 = dest->sb.b3>src->sb.b3?0xff:0;
	dest->ub.b4 = dest->sb.b4>src->sb.b4?0xff:0;
	dest->ub.b5 = dest->sb.b5>src->sb.b5?0xff:0;
	dest->ub.b6 = dest->sb.b6>src->sb.b6?0xff:0;
	dest->ub.b7 = dest->sb.b7>src->sb.b7?0xff:0;

    CYCLES(1);
    NEXT();
}

void OPCALL pcmpgtb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = dest->sb.b0>src.sb.b0?0xff:0;
	dest->ub.b1 = dest->sb.b1>src.sb.b1?0xff:0;
	dest->ub.b2 = dest->sb.b2>src.sb.b2?0xff:0;
	dest->ub.b3 = dest->sb.b3>src.sb.b3?0xff:0;
	dest->ub.b4 = dest->sb.b4>src.sb.b4?0xff:0;
	dest->ub.b5 = dest->sb.b5>src.sb.b5?0xff:0;
	dest->ub.b6 = dest->sb.b6>src.sb.b6?0xff:0;
	dest->ub.b7 = dest->sb.b7>src.sb.b7?0xff:0;

    CYCLES(2);
    NEXT();
}

void OPCALL pcmpgtb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = dest->sb.b0>src.sb.b0?0xff:0;
	dest->ub.b1 = dest->sb.b1>src.sb.b1?0xff:0;
	dest->ub.b2 = dest->sb.b2>src.sb.b2?0xff:0;
	dest->ub.b3 = dest->sb.b3>src.sb.b3?0xff:0;
	dest->ub.b4 = dest->sb.b4>src.sb.b4?0xff:0;
	dest->ub.b5 = dest->sb.b5>src.sb.b5?0xff:0;
	dest->ub.b6 = dest->sb.b6>src.sb.b6?0xff:0;
	dest->ub.b7 = dest->sb.b7>src.sb.b7?0xff:0;

    CYCLES(2);
    NEXT();
}

// PCMPGTB Pq,Qq
void decode364(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pcmpgtb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PCMPGTB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pcmpgtb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PCMPGTB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pcmpgtb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PCMPGTB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pcmpgtw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->uw.w0 = dest->sw.w0>src->sw.w0?0xffff:0;
	dest->uw.w1 = dest->sw.w1>src->sw.w1?0xffff:0;
	dest->uw.w2 = dest->sw.w2>src->sw.w2?0xffff:0;
	dest->uw.w3 = dest->sw.w3>src->sw.w3?0xffff:0;

    CYCLES(1);
    NEXT();
}

void OPCALL pcmpgtw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = dest->sw.w0>src.sw.w0?0xffff:0;
	dest->uw.w1 = dest->sw.w1>src.sw.w1?0xffff:0;
	dest->uw.w2 = dest->sw.w2>src.sw.w2?0xffff:0;
	dest->uw.w3 = dest->sw.w3>src.sw.w3?0xffff:0;

    CYCLES(2);
    NEXT();
}

void OPCALL pcmpgtw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = dest->sw.w0>src.sw.w0?0xffff:0;
	dest->uw.w1 = dest->sw.w1>src.sw.w1?0xffff:0;
	dest->uw.w2 = dest->sw.w2>src.sw.w2?0xffff:0;
	dest->uw.w3 = dest->sw.w3>src.sw.w3?0xffff:0;

    CYCLES(2);
    NEXT();
}

// PCMPGTW Pq,Qq
void decode365(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pcmpgtw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PCMPGTW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pcmpgtw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PCMPGTW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pcmpgtw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PCMPGTW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL pcmpgtd_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ud.d0 = dest->sd.d0>src->sd.d0?0xffffffff:0;
	dest->ud.d1 = dest->sd.d1>src->sd.d1?0xffffffff:0;

    CYCLES(1);
    NEXT();
}

void OPCALL pcmpgtd_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d0 = dest->sd.d0>src.sd.d0?0xffffffff:0;
	dest->ud.d1 = dest->sd.d1>src.sd.d1?0xffffffff:0;

    CYCLES(2);
    NEXT();
}

void OPCALL pcmpgtd_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d0 = dest->sd.d0>src.sd.d0?0xffffffff:0;
	dest->ud.d1 = dest->sd.d1>src.sd.d1?0xffffffff:0;

    CYCLES(2);
    NEXT();
}

// PCMPGTD Pq,Qq
void decode366(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = pcmpgtd_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PCMPGTD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = pcmpgtd_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PCMPGTD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = pcmpgtd_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PCMPGTD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

/* Data Packing */
void OPCALL packsswb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->sb.b0 = SaturateWordSToByteS(dest->sw.w0);
	dest->sb.b1 = SaturateWordSToByteS(dest->sw.w1);
	dest->sb.b2 = SaturateWordSToByteS(dest->sw.w2);
	dest->sb.b3 = SaturateWordSToByteS(dest->sw.w3);
	dest->sb.b4 = SaturateWordSToByteS(src->sw.w0);
	dest->sb.b5 = SaturateWordSToByteS(src->sw.w1);
	dest->sb.b6 = SaturateWordSToByteS(src->sw.w2);
	dest->sb.b7 = SaturateWordSToByteS(src->sw.w3);

    CYCLES(1);
    NEXT();
}

void OPCALL packsswb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->sb.b0 = SaturateWordSToByteS(dest->sw.w0);
	dest->sb.b1 = SaturateWordSToByteS(dest->sw.w1);
	dest->sb.b2 = SaturateWordSToByteS(dest->sw.w2);
	dest->sb.b3 = SaturateWordSToByteS(dest->sw.w3);
	dest->sb.b4 = SaturateWordSToByteS(src.sw.w0);
	dest->sb.b5 = SaturateWordSToByteS(src.sw.w1);
	dest->sb.b6 = SaturateWordSToByteS(src.sw.w2);
	dest->sb.b7 = SaturateWordSToByteS(src.sw.w3);

    CYCLES(2);
    NEXT();
}

void OPCALL packsswb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->sb.b0 = SaturateWordSToByteS(dest->sw.w0);
	dest->sb.b1 = SaturateWordSToByteS(dest->sw.w1);
	dest->sb.b2 = SaturateWordSToByteS(dest->sw.w2);
	dest->sb.b3 = SaturateWordSToByteS(dest->sw.w3);
	dest->sb.b4 = SaturateWordSToByteS(src.sw.w0);
	dest->sb.b5 = SaturateWordSToByteS(src.sw.w1);
	dest->sb.b6 = SaturateWordSToByteS(src.sw.w2);
	dest->sb.b7 = SaturateWordSToByteS(src.sw.w3);

    CYCLES(2);
    NEXT();
}

// PACKSSWB Pq,Qq
void decode363(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = packsswb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PACKSSWB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = packsswb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PACKSSWB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = packsswb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PACKSSWB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL packssdw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->sw.w0 = SaturateDwordSToWordS(dest->sd.d0);
	dest->sw.w1 = SaturateDwordSToWordS(dest->sd.d1);
	dest->sw.w2 = SaturateDwordSToWordS(src->sd.d0);
	dest->sw.w3 = SaturateDwordSToWordS(src->sd.d1);

    CYCLES(1);
    NEXT();
}

void OPCALL packssdw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->sw.w0 = SaturateDwordSToWordS(dest->sd.d0);
	dest->sw.w1 = SaturateDwordSToWordS(dest->sd.d1);
	dest->sw.w2 = SaturateDwordSToWordS(src.sd.d0);
	dest->sw.w3 = SaturateDwordSToWordS(src.sd.d1);

    CYCLES(2);
    NEXT();
}

void OPCALL packssdw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->sw.w0 = SaturateDwordSToWordS(dest->sd.d0);
	dest->sw.w1 = SaturateDwordSToWordS(dest->sd.d1);
	dest->sw.w2 = SaturateDwordSToWordS(src.sd.d0);
	dest->sw.w3 = SaturateDwordSToWordS(src.sd.d1);

    CYCLES(2);
    NEXT();
}

// PACKSSDW Pq,Qq
void decode36b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = packssdw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PACKSSDW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = packssdw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PACKSSDW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = packssdw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PACKSSDW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL packuswb_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ub.b0 = SaturateWordSToByteU(dest->sw.w0);
	dest->ub.b1 = SaturateWordSToByteU(dest->sw.w1);
	dest->ub.b2 = SaturateWordSToByteU(dest->sw.w2);
	dest->ub.b3 = SaturateWordSToByteU(dest->sw.w3);
	dest->ub.b4 = SaturateWordSToByteU(src->sw.w0);
	dest->ub.b5 = SaturateWordSToByteU(src->sw.w1);
	dest->ub.b6 = SaturateWordSToByteU(src->sw.w2);
	dest->ub.b7 = SaturateWordSToByteU(src->sw.w3);

    CYCLES(1);
    NEXT();
}

void OPCALL packuswb_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = SaturateWordSToByteU(dest->sw.w0);
	dest->ub.b1 = SaturateWordSToByteU(dest->sw.w1);
	dest->ub.b2 = SaturateWordSToByteU(dest->sw.w2);
	dest->ub.b3 = SaturateWordSToByteU(dest->sw.w3);
	dest->ub.b4 = SaturateWordSToByteU(src.sw.w0);
	dest->ub.b5 = SaturateWordSToByteU(src.sw.w1);
	dest->ub.b6 = SaturateWordSToByteU(src.sw.w2);
	dest->ub.b7 = SaturateWordSToByteU(src.sw.w3);

    CYCLES(2);
    NEXT();
}

void OPCALL packuswb_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = SaturateWordSToByteU(dest->sw.w0);
	dest->ub.b1 = SaturateWordSToByteU(dest->sw.w1);
	dest->ub.b2 = SaturateWordSToByteU(dest->sw.w2);
	dest->ub.b3 = SaturateWordSToByteU(dest->sw.w3);
	dest->ub.b4 = SaturateWordSToByteU(src.sw.w0);
	dest->ub.b5 = SaturateWordSToByteU(src.sw.w1);
	dest->ub.b6 = SaturateWordSToByteU(src.sw.w2);
	dest->ub.b7 = SaturateWordSToByteU(src.sw.w3);

    CYCLES(2);
    NEXT();
}

// PACKUSWB Pq,Qq
void decode367(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = packuswb_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PACKUSWB", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = packuswb_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PACKUSWB", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = packuswb_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PACKUSWB", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL punpckhbw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ub.b0 = dest->ub.b4;
	dest->ub.b1 = src->ub.b4;
	dest->ub.b2 = dest->ub.b5;
	dest->ub.b3 = src->ub.b5;
	dest->ub.b4 = dest->ub.b6;
	dest->ub.b5 = src->ub.b6;
	dest->ub.b6 = dest->ub.b7;
	dest->ub.b7 = src->ub.b7;

    CYCLES(1);
    NEXT();
}

void OPCALL punpckhbw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = dest->ub.b4;
	dest->ub.b1 = src.ub.b4;
	dest->ub.b2 = dest->ub.b5;
	dest->ub.b3 = src.ub.b5;
	dest->ub.b4 = dest->ub.b6;
	dest->ub.b5 = src.ub.b6;
	dest->ub.b6 = dest->ub.b7;
	dest->ub.b7 = src.ub.b7;

    CYCLES(2);
    NEXT();
}

void OPCALL punpckhbw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b0 = dest->ub.b4;
	dest->ub.b1 = src.ub.b4;
	dest->ub.b2 = dest->ub.b5;
	dest->ub.b3 = src.ub.b5;
	dest->ub.b4 = dest->ub.b6;
	dest->ub.b5 = src.ub.b6;
	dest->ub.b6 = dest->ub.b7;
	dest->ub.b7 = src.ub.b7;

    CYCLES(2);
    NEXT();
}

// PUNPCKHBW Pq,Qq
void decode368(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = punpckhbw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PUNPCKHBW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = punpckhbw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PUNPCKHBW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = punpckhbw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PUNPCKHBW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL punpckhwd_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->uw.w0 = dest->uw.w2;
	dest->uw.w1 = src->uw.w2;
	dest->uw.w2 = dest->uw.w3;
	dest->uw.w3 = src->uw.w3;

    CYCLES(1);
    NEXT();
}

void OPCALL punpckhwd_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = dest->uw.w2;
	dest->uw.w1 = src.uw.w2;
	dest->uw.w2 = dest->uw.w3;
	dest->uw.w3 = src.uw.w3;

    CYCLES(2);
    NEXT();
}

void OPCALL punpckhwd_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w0 = dest->uw.w2;
	dest->uw.w1 = src.uw.w2;
	dest->uw.w2 = dest->uw.w3;
	dest->uw.w3 = src.uw.w3;

    CYCLES(2);
    NEXT();
}

// PUNPCKHWD Pq,Qq
void decode369(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = punpckhwd_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PUNPCKHWD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = punpckhwd_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PUNPCKHWD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = punpckhwd_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PUNPCKHWD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL punpckhdq_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ud.d0 = dest->ud.d1;
	dest->ud.d1 = src->ud.d1;

    CYCLES(1);
    NEXT();
}

void OPCALL punpckhdq_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d0 = dest->ud.d1;
	dest->ud.d1 = src.ud.d1;

    CYCLES(2);
    NEXT();
}

void OPCALL punpckhdq_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d0 = dest->ud.d1;
	dest->ud.d1 = src.ud.d1;

    CYCLES(2);
    NEXT();
}

// PUNPCKHDQ Pq,Qq
void decode36a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = punpckhdq_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PUNPCKHDQ", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = punpckhdq_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PUNPCKHDQ", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = punpckhdq_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PUNPCKHDQ", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL punpcklbw_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ub.b7 = src->ub.b3;
	dest->ub.b6 = dest->ub.b3;
	dest->ub.b5 = src->ub.b2;
	dest->ub.b4 = dest->ub.b2;
	dest->ub.b3 = src->ub.b1;
	dest->ub.b2 = dest->ub.b1;
	dest->ub.b1 = src->ub.b0;
	dest->ub.b0 = dest->ub.b0;

    CYCLES(1);
    NEXT();
}

void OPCALL punpcklbw_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b7 = src.ub.b3;
	dest->ub.b6 = dest->ub.b3;
	dest->ub.b5 = src.ub.b2;
	dest->ub.b4 = dest->ub.b2;
	dest->ub.b3 = src.ub.b1;
	dest->ub.b2 = dest->ub.b1;
	dest->ub.b1 = src.ub.b0;
	dest->ub.b0 = dest->ub.b0;

    CYCLES(2);
    NEXT();
}

void OPCALL punpcklbw_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ub.b7 = src.ub.b3;
	dest->ub.b6 = dest->ub.b3;
	dest->ub.b5 = src.ub.b2;
	dest->ub.b4 = dest->ub.b2;
	dest->ub.b3 = src.ub.b1;
	dest->ub.b2 = dest->ub.b1;
	dest->ub.b1 = src.ub.b0;
	dest->ub.b0 = dest->ub.b0;

    CYCLES(2);
    NEXT();
}

// PUNPCKLBW Pq,Qq
void decode360(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = punpcklbw_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PUNPCKLBW", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = punpcklbw_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PUNPCKLBW", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = punpcklbw_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PUNPCKLBW", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL punpcklwd_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->uw.w3 = src->uw.w1;
	dest->uw.w2 = dest->uw.w1;
	dest->uw.w1 = src->uw.w0;
	dest->uw.w0 = dest->uw.w0;

    CYCLES(1);
    NEXT();
}

void OPCALL punpcklwd_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w3 = src.uw.w1;
	dest->uw.w2 = dest->uw.w1;
	dest->uw.w1 = src.uw.w0;
	dest->uw.w0 = dest->uw.w0;

    CYCLES(2);
    NEXT();
}

void OPCALL punpcklwd_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->uw.w3 = src.uw.w1;
	dest->uw.w2 = dest->uw.w1;
	dest->uw.w1 = src.uw.w0;
	dest->uw.w0 = dest->uw.w0;

    CYCLES(2);
    NEXT();
}

// PUNPCKLWD Pq,Qq
void decode361(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = punpcklwd_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PUNPCKLWD", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = punpcklwd_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PUNPCKLWD", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = punpcklwd_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PUNPCKLWD", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}

void OPCALL punpckldq_mmx_mmx(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg* src=&reg_mmx[op->r2];
    
    dest->ud.d1 = src->ud.d0;

    CYCLES(1);
    NEXT();
}

void OPCALL punpckldq_mmx_e64_16(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d1 = src.ud.d0;

    CYCLES(2);
    NEXT();
}

void OPCALL punpckldq_mmx_e64_32(struct CPU* cpu, struct Op* op) {
    MMX_reg* dest=&reg_mmx[op->r1];
    MMX_reg src;    
    
    src.q = readq(cpu->thread, eaa32(cpu, op));

	dest->ud.d1 = src.ud.d0;

    CYCLES(2);
    NEXT();
}

// PUNPCKLDQ Pq,Qq
void decode362(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = punpckldq_mmx_mmx;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("PUNPCKLDQ", MMX(data->op->r1),MMX(data->op->r2));
    } else if (data->ea16) {
        data->op->func = punpckldq_mmx_e64_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("PUNPCKLDQ", MMX(data->op->r1),M64(data, rm, data->op));
    } else {
        data->op->func = punpckldq_mmx_e64_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("PUNPCKLDQ", MMX(data->op->r1),M64(data, rm, data->op));
    }
    NEXT_OP(data);
}