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

#include "boxedwine.h"
#include "common_sse.h"
#include <math.h>

void common_addpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_add_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_addpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_add_ps(cpu->xmm[reg].ps, value);
}

void common_addssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_add_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_addssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_add_ss(cpu->xmm[reg].ps, value);
}

void common_subpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_sub_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_subpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_sub_ps(cpu->xmm[reg].ps, value);
}

void common_subssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_sub_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_subssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_sub_ss(cpu->xmm[reg].ps, value);
}

void common_mulpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_mul_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_mulpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_mul_ps(cpu->xmm[reg].ps, value);
}

void common_mulssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_mul_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_mulssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_mul_ss(cpu->xmm[reg].ps, value);
}

void common_divpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_div_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_divpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_div_ps(cpu->xmm[reg].ps, value);
}

void common_divssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_div_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_divssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_div_ss(cpu->xmm[reg].ps, value);
}

void common_rcppsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_rcp_ps(cpu->xmm[r2].ps);
}

void common_rcppsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_rcp_ps(value);
}

void common_rcpssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_rcp_ss(cpu->xmm[r2].ps);
}

void common_rcpssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value = cpu->xmm[reg].ps;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_rcp_ss(value);
}

void common_sqrtpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_sqrt_ps(cpu->xmm[r2].ps);    
}

void common_sqrtpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);  
    cpu->xmm[reg].ps = simde_mm_sqrt_ps(value);
}

void common_sqrtssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_sqrt_ss(cpu->xmm[r2].ps);
}

void common_sqrtssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value = cpu->xmm[reg].ps;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_sqrt_ss(value);
}

void common_rsqrtpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_rsqrt_ps(cpu->xmm[r2].ps);    
}

void common_rsqrtpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);  
    cpu->xmm[reg].ps = simde_mm_rsqrt_ps(value);
}

void common_rsqrtssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_rsqrt_ss(cpu->xmm[r2].ps);
}

void common_rsqrtssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value = cpu->xmm[reg].ps;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_rsqrt_ss(value);
}

void common_maxpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_max_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_maxpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);  
    cpu->xmm[reg].ps = simde_mm_max_ps(cpu->xmm[reg].ps, value);
}

void common_maxssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_max_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_maxssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_max_ss(cpu->xmm[reg].ps, value);
}

void common_minpsXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_min_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_minpsE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);  
    cpu->xmm[reg].ps = simde_mm_min_ps(cpu->xmm[reg].ps, value);
}

void common_minssXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_min_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_minssE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps = simde_mm_min_ss(cpu->xmm[reg].ps, value);
}

void common_pavgbMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_avg_pu8(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pavgbMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_avg_pu8(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_pavgwMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_avg_pu16(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pavgwMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_avg_pu16(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_psadbwMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_sad_pu8(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_psadbwMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_sad_pu8(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_pextrwR32Mmx(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    simde__m64 m = {};
    m.u64[0] = cpu->reg_mmx[r2].q;
    cpu->reg[r1].u32 = simde_mm_extract_pi16(m, imm);
}

void common_pextrwE16Mmx(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m64 m = {};
    m.u64[0] = cpu->reg_mmx[reg].q;
    cpu->memory->writew(address, (U16)simde_mm_extract_pi16(m, imm));
}

void common_pinsrwMmxR32(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    simde__m64 m = {};
    m.u64[0] = cpu->reg_mmx[r1].q;
    m = simde_mm_insert_pi16(m, cpu->reg[r2].u16, imm);
    cpu->reg_mmx[r1].q = m.u64[0];
}

void common_pinsrwMmxE16(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m64 m = {};
    m.u64[0] = cpu->reg_mmx[reg].q;
    m = simde_mm_insert_pi16(m, cpu->memory->readw(address), imm);
    cpu->reg_mmx[reg].q = m.u64[0];
}

void common_pmaxswMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_max_pi16(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pmaxswMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_max_pi16(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_pmaxubMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_max_pu8(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pmaxubMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_max_pu8(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_pminswMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_min_pi16(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pminswMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_min_pi16(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_pminubMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_min_pu8(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pminubMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_min_pu8(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_pmovmskbR32Mmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m = {};
    m.u64[0] = cpu->reg_mmx[r2].q;
    cpu->reg[r1].u32 = (U32)simde_mm_movemask_pi8(m);
}

void common_pmulhuwMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_mulhi_pu16(m1, m2);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pmulhuwMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_mulhi_pu16(m1, m2);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_pshufwMmxMmx(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[r1].q;
    m2.u64[0] = cpu->reg_mmx[r2].q;
    simde__m64 r = simde_mm_shuffle_pi16(m2, imm);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_pshufwMmxE64(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m64 m1 = {};
    simde__m64 m2 = {};
    m1.u64[0] = cpu->reg_mmx[reg].q;
    m2.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_shuffle_pi16(m2, imm);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_andnpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_andnot_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_andnpsXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8); 
    cpu->xmm[reg].ps = simde_mm_andnot_ps(cpu->xmm[reg].ps, value);
}

void common_andpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_and_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_andpsXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8); 
    cpu->xmm[reg].ps = simde_mm_and_ps(cpu->xmm[reg].ps, value);
}

void common_orpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_or_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_orpsXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8); 
    cpu->xmm[reg].ps = simde_mm_or_ps(cpu->xmm[reg].ps, value);
}

void common_xorpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_xor_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_xorpsXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8); 
    cpu->xmm[reg].ps = simde_mm_xor_ps(cpu->xmm[reg].ps, value);
}

void common_cvtpi2psXmmMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 value;
    value.u64[0] = cpu->reg_mmx[r2].q;
    cpu->xmm[r1].ps = simde_mm_cvtpi32_ps(cpu->xmm[r1].ps, value);
}

void common_cvtpi2psXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].ps = simde_mm_cvtpi32_ps(cpu->xmm[reg].ps, value);
}

void common_cvtps2piMmxXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = simde_mm_cvtps_pi32(cpu->xmm[r2].ps).u64[0];
}

void common_cvtps2piMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->reg_mmx[reg].q = simde_mm_cvtps_pi32(value).u64[0];
}

void common_cvtsi2ssXmmR32(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_cvtsi32_ss(cpu->xmm[r1].ps, (S32)cpu->reg[r2].u32);
}

void common_cvtsi2ssXmmE32(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].ps = simde_mm_cvtsi32_ss(cpu->xmm[reg].ps, (S32)cpu->memory->readd(address));
}

void common_cvtss2siR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = (U32)simde_mm_cvtss_si32(cpu->xmm[r2].ps);
}

void common_cvtss2siR32E32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->reg[reg].u32 = (U32)simde_mm_cvtss_si32(value);
}

void common_cvttps2piMmxXmm(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 r = simde_mm_cvttps_pi32(cpu->xmm[r2].ps);
    cpu->reg_mmx[r1].q = r.u64[0];
}

void common_cvttps2piMmxE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    simde__m64 r = simde_mm_cvttps_pi32(value);
    cpu->reg_mmx[reg].q = r.u64[0];
}

void common_cvttss2siR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = simde_mm_cvttss_si32(cpu->xmm[r2].ps);
}

void common_cvttss2siR32E32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->reg[reg].u32 = simde_mm_cvttss_si32(value);
}

void common_movapsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = cpu->xmm[r2].ps;
}

void common_movapsXmmE128(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].ps.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].ps.u64[1] = cpu->memory->readq(address+8);
}

void common_movapsE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].ps.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].ps.u64[1]);
}

void common_movhlpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps.u64[0] = cpu->xmm[r2].ps.u64[1];
}

void common_movlhpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps.u64[1] = cpu->xmm[r2].ps.u64[0];
}

void common_movhpsXmmE64(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].ps.u64[1] = cpu->memory->readq(address);
}

void common_movhpsE64Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].ps.u64[1]);
}

void common_movlpsXmmE64(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].ps.u64[0] = cpu->memory->readq(address);
}

void common_movlpsE64Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].ps.u64[0]);
}

void common_movmskpsR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = (U32)simde_mm_movemask_ps(cpu->xmm[r2].ps);
}

void common_movssXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps.u32[0] = cpu->xmm[r2].ps.u32[0];
}

void common_movssXmmE32(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].ps.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].ps.u32[1] = 0;
    cpu->xmm[reg].ps.u32[2] = 0;
    cpu->xmm[reg].ps.u32[3] = 0;
}

void common_movssE32Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writed(address, cpu->xmm[reg].ps.u32[0]);
}

void common_movupsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = cpu->xmm[r2].ps;
}

void common_movupsXmmE128(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].ps.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].ps.u64[1] = cpu->memory->readq(address+8);
}

void common_movupsE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].ps.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].ps.u64[1]);
}

void common_maskmovqEDIMmxMmx(CPU* cpu, U32 r1, U32 r2, U32 address) {
    simde__m64 a = {};
    simde__m64 mask = {};
    U8 result[8] = {};
    a.u64[0] = cpu->reg_mmx[r1].q;
    mask.u64[0] = cpu->reg_mmx[r2].q;

    cpu->memory->memcpy(result, address, 8);
    simde_mm_maskmove_si64(a, mask, (int8_t*)result);
    cpu->memory->memcpy(address, result, 8);
}

#define G(rm) ((rm >> 3) & 7)
#define E(rm) (rm & 7)

void common_maskmovqEDIMmxMmxRM(CPU* cpu, U32 rm, U32 base, U32 bigAddress) {
    simde__m64 a = {};
    simde__m64 mask = {};
    U32 address = (bigAddress ? EDI : DI) + cpu->seg[base].address;
    U8 result[8] = {};
    a.u64[0] = cpu->reg_mmx[G(rm)].q;
    mask.u64[0] = cpu->reg_mmx[E(rm)].q;

    cpu->memory->memcpy(result, address, 8);
    simde_mm_maskmove_si64(a, mask, (int8_t*)result);
    cpu->memory->memcpy(address, result, 8);
}

void common_movntpsE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].ps.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].ps.u64[1]);
}

void common_movntqE64Mmx(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->reg_mmx[reg].q);
}

void common_shufpsXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].ps = simde_mm_shuffle_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps, imm);
}

void common_shufpsXmmE128(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);  
    cpu->xmm[reg].ps = simde_mm_shuffle_ps(cpu->xmm[reg].ps, value, imm);
}

void common_unpckhpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_unpackhi_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_unpckhpsXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);    
    cpu->xmm[reg].ps = simde_mm_unpackhi_ps(cpu->xmm[reg].ps, value);
}

void common_unpcklpsXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_unpacklo_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps);
}

void common_unpcklpsXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);    
    cpu->xmm[reg].ps = simde_mm_unpacklo_ps(cpu->xmm[reg].ps, value);
}

void common_prefetchT0(CPU* cpu) {
}

void common_prefetchT1(CPU* cpu) {
}

void common_prefetchT2(CPU* cpu) {
}

void common_prefetchNTA(CPU* cpu) {
}

void common_cmppsXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[r1].ps = simde_mm_cmpeq_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 1: cpu->xmm[r1].ps = simde_mm_cmplt_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 2: cpu->xmm[r1].ps = simde_mm_cmple_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 3: cpu->xmm[r1].ps = simde_mm_cmpunord_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 4: cpu->xmm[r1].ps = simde_mm_cmpneq_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 5: cpu->xmm[r1].ps = simde_mm_cmpnlt_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 6: cpu->xmm[r1].ps = simde_mm_cmpnle_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 7: cpu->xmm[r1].ps = simde_mm_cmpord_ps(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    }
}

void common_cmppsXmmE128(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[reg].ps = simde_mm_cmpeq_ps(cpu->xmm[reg].ps, value); break;
    case 1: cpu->xmm[reg].ps = simde_mm_cmplt_ps(cpu->xmm[reg].ps, value); break;
    case 2: cpu->xmm[reg].ps = simde_mm_cmple_ps(cpu->xmm[reg].ps, value); break;
    case 3: cpu->xmm[reg].ps = simde_mm_cmpunord_ps(cpu->xmm[reg].ps, value); break;
    case 4: cpu->xmm[reg].ps = simde_mm_cmpneq_ps(cpu->xmm[reg].ps, value); break;
    case 5: cpu->xmm[reg].ps = simde_mm_cmpnlt_ps(cpu->xmm[reg].ps, value); break;
    case 6: cpu->xmm[reg].ps = simde_mm_cmpnle_ps(cpu->xmm[reg].ps, value); break;
    case 7: cpu->xmm[reg].ps = simde_mm_cmpord_ps(cpu->xmm[reg].ps, value); break;
    }
}

void common_cmpssXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[r1].ps = simde_mm_cmpeq_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 1: cpu->xmm[r1].ps = simde_mm_cmplt_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 2: cpu->xmm[r1].ps = simde_mm_cmple_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 3: cpu->xmm[r1].ps = simde_mm_cmpunord_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 4: cpu->xmm[r1].ps = simde_mm_cmpneq_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 5: cpu->xmm[r1].ps = simde_mm_cmpnlt_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 6: cpu->xmm[r1].ps = simde_mm_cmpnle_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    case 7: cpu->xmm[r1].ps = simde_mm_cmpord_ss(cpu->xmm[r1].ps, cpu->xmm[r2].ps); break;
    }
}

void common_cmpssXmmE32(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[reg].ps = simde_mm_cmpeq_ss(cpu->xmm[reg].ps, value); break;
    case 1: cpu->xmm[reg].ps = simde_mm_cmplt_ss(cpu->xmm[reg].ps, value); break;
    case 2: cpu->xmm[reg].ps = simde_mm_cmple_ss(cpu->xmm[reg].ps, value); break;
    case 3: cpu->xmm[reg].ps = simde_mm_cmpunord_ss(cpu->xmm[reg].ps, value); break;
    case 4: cpu->xmm[reg].ps = simde_mm_cmpneq_ss(cpu->xmm[reg].ps, value); break;
    case 5: cpu->xmm[reg].ps = simde_mm_cmpnlt_ss(cpu->xmm[reg].ps, value); break;
    case 6: cpu->xmm[reg].ps = simde_mm_cmpnle_ss(cpu->xmm[reg].ps, value); break;
    case 7: cpu->xmm[reg].ps = simde_mm_cmpord_ss(cpu->xmm[reg].ps, value); break;
    }
}

// :TODO: not sure about exceptions
void common_comissXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->fillFlags();
    cpu->flags&=~(AF|OF|SF|CF|PF|ZF);
    const simde__m128& a = cpu->xmm[r1].ps;
    const simde__m128& b = cpu->xmm[r2].ps;
    if (isnan(a.f32[0]) || isnan(b.f32[0])) {
        cpu->flags|=CF|ZF|PF;
    } else if (a.f32[0] == b.f32[0]) {
        cpu->flags|=ZF;
    } else if (a.f32[0] < b.f32[0]) {
        cpu->flags|=CF;
    }
}

void common_comissXmmE32(CPU* cpu, U32 reg, U32 address) {
    cpu->fillFlags();
    cpu->flags&=~(AF|OF|SF|CF|PF|ZF);
    const simde__m128& a = cpu->xmm[reg].ps;
    simde__m128 b;
    b.u32[0] = cpu->memory->readd(address);
    if (isnan(a.f32[0]) || isnan(b.f32[0])) {
        cpu->flags|=CF|ZF|PF;
    } else if (a.f32[0] == b.f32[0]) {
        cpu->flags|=ZF;
    } else if (a.f32[0] < b.f32[0]) {
        cpu->flags|=CF;
    }
}

void common_ucomissXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    common_comissXmmXmm(cpu, r1, r2);
}

void common_ucomissXmmE32(CPU* cpu, U32 reg, U32 address) {
    common_comissXmmE32(cpu, reg, address);
}

void common_stmxcsr(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writed(address, 0x1F80);
}

void common_ldmxcsr(CPU* cpu, U32 reg, U32 address) {
}

void common_sfence(CPU* cpu) {
}