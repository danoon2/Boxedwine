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

#define TO_PD simde_mm_castps_pd
#define FROM_PD simde_mm_castpd_ps

void common_addpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_add_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_addpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_add_pd(cpu->xmm[reg].pd, value);
}

void common_addsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_add_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_addsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_add_sd(cpu->xmm[reg].pd, value);
}

void common_subpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_sub_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_subpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_sub_pd(cpu->xmm[reg].pd, value);
}

void common_subsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_sub_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_subsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_sub_sd(cpu->xmm[reg].pd, value);
}

void common_mulpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_mul_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_mulpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_mul_pd(cpu->xmm[reg].pd, value);
}    

void common_mulsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_mul_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_mulsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_mul_sd(cpu->xmm[reg].pd, value);
}

void common_divpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_div_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_divpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_div_pd(cpu->xmm[reg].pd, value);
}

void common_divsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_div_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_divsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_div_sd(cpu->xmm[reg].pd, value);
}

void common_maxpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_max_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_maxpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_max_pd(cpu->xmm[reg].pd, value);
}

void common_maxsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_max_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_maxsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_max_sd(cpu->xmm[reg].pd, value);
}

void common_minpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_min_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_minpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_min_pd(cpu->xmm[reg].pd, value);
}

void common_minsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_min_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_minsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_min_sd(cpu->xmm[reg].pd, value);
}

void common_paddbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_add_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_paddbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_add_epi8(cpu->xmm[reg].pi, value);
}

void common_paddwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_add_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_paddwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_add_epi16(cpu->xmm[reg].pi, value);
}

void common_padddXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_add_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_padddXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_add_epi32(cpu->xmm[reg].pi, value);
}

void common_paddqMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = cpu->reg_mmx[r1].q + cpu->reg_mmx[r2].q;
}

void common_paddqMmxE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q = cpu->reg_mmx[reg].q + cpu->memory->readq(address);
}

void common_paddqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_add_epi64(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_paddqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_add_epi64(cpu->xmm[reg].pi, value);
}

void common_paddsbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_adds_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_paddsbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_adds_epi8(cpu->xmm[reg].pi, value);
}

void common_paddswXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_adds_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_paddswXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_adds_epi16(cpu->xmm[reg].pi, value);
}

void common_paddusbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_adds_epu8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_paddusbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_adds_epu8(cpu->xmm[reg].pi, value);
}

void common_padduswXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_adds_epu16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_padduswXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_adds_epu16(cpu->xmm[reg].pi, value);
}

void common_psubbXmmXmm(CPU* cpu,U32 r1, U32 r2 ) {
    cpu->xmm[r1].pi = simde_mm_sub_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sub_epi8(cpu->xmm[reg].pi, value);
}

void common_psubwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sub_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sub_epi16(cpu->xmm[reg].pi, value);
}

void common_psubdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sub_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sub_epi32(cpu->xmm[reg].pi, value);
}

void common_psubqMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = cpu->reg_mmx[r1].q - cpu->reg_mmx[r2].q;
}

void common_psubqMmxE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q = cpu->reg_mmx[reg].q - cpu->memory->readq(address);
}

void common_psubqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sub_epi64(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sub_epi64(cpu->xmm[reg].pi, value);
}

void common_psubsbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_subs_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubsbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_subs_epi8(cpu->xmm[reg].pi, value);
}

void common_psubswXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_subs_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubswXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_subs_epi16(cpu->xmm[reg].pi, value);
}

void common_psubusbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_subs_epu8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubusbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_subs_epu8(cpu->xmm[reg].pi, value);
}

void common_psubuswXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_subs_epu16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psubuswXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_subs_epu16(cpu->xmm[reg].pi, value);
}

void common_pmaddwdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_madd_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pmaddwdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_madd_epi16(cpu->xmm[reg].pi, value);
}

void common_pmulhwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_mulhi_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pmulhwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_mulhi_epi16(cpu->xmm[reg].pi, value);
}

void common_pmullwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_mullo_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pmullwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_mullo_epi16(cpu->xmm[reg].pi, value);
}

void common_pmuludqMmxMmx(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = (U64)cpu->reg_mmx[r1].ud.d0 * (U64)cpu->reg_mmx[r2].ud.d0;
}

void common_pmuludqMmxE64(CPU* cpu, U32 reg, U32 address) {
    cpu->reg_mmx[reg].q = (U64)cpu->reg_mmx[reg].ud.d0 * (U64)cpu->memory->readd(address);
}

void common_pmuludqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_mul_epu32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pmuludqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_mul_epu32(cpu->xmm[reg].pi, value);
}

void common_sqrtpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_sqrt_pd(cpu->xmm[r2].pd);
}

void common_sqrtpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_sqrt_pd(value);
}

void common_sqrtsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_sqrt_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_sqrtsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    //value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_sqrt_sd(cpu->xmm[reg].pd, value);
}

void common_andnpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_andnot_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_andnpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_andnot_pd(cpu->xmm[reg].pd, value);
}

void common_andpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_and_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_andpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_and_pd(cpu->xmm[reg].pd, value);
}

void common_pandXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_and_si128(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pandXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_and_pd(cpu->xmm[reg].pd, value);
}

void common_pandnXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_andnot_si128(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pandnXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_andnot_si128(cpu->xmm[reg].pi, value);
}

void common_porXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_or_si128(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_porXmmXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_or_si128(cpu->xmm[reg].pi, value);
}

void common_pslldqXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_slli_si128(cpu->xmm[r1].pi, imm);
}

void common_psllqXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_slli_epi64(cpu->xmm[r1].pi, imm);
}

void common_psllqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sll_epi64(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psllqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sll_epi64(cpu->xmm[reg].pi, value);
}

void common_pslldXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_slli_epi32(cpu->xmm[r1].pi, imm);
}

void common_pslldXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sll_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pslldXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sll_epi32(cpu->xmm[reg].pi, value);
}

void common_psllwXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_slli_epi16(cpu->xmm[r1].pi, imm);
}

void common_psllwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sll_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psllwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sll_epi16(cpu->xmm[reg].pi, value);
}

void common_psradXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_srai_epi32(cpu->xmm[r1].pi, imm);
}

void common_psradXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sra_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psradXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sra_epi32(cpu->xmm[reg].pi, value);
}

void common_psrawXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_srai_epi16(cpu->xmm[r1].pi, imm);
}

void common_psrawXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sra_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psrawXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sra_epi16(cpu->xmm[reg].pi, value);
}

void common_psrldqXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_srli_si128(cpu->xmm[r1].pi, imm);
}

void common_psrlqXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_srli_epi64(cpu->xmm[r1].pi, imm);
}

void common_psrlqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_srl_epi64(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psrlqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_srl_epi64(cpu->xmm[reg].pi, value);
}

void common_psrldXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_srli_epi32(cpu->xmm[r1].pi, imm);
}

void common_psrldXmmXmm(CPU* cpu,U32 r1, U32 r2 ) {
    cpu->xmm[r1].pi = simde_mm_srl_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psrldXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_srl_epi32(cpu->xmm[reg].pi, value);
}

void common_psrlwXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_srli_epi16(cpu->xmm[r1].pi, imm);
}

void common_psrlwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_srl_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psrlwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_srl_epi16(cpu->xmm[reg].pi, value);
}

void common_pxorXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_xor_si128(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pxorXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_xor_si128(cpu->xmm[reg].pi, value);
}

void common_orpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_or_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_orpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_or_pd(cpu->xmm[reg].pd, value);
}

void common_xorpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_xor_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_xorpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_xor_pd(cpu->xmm[reg].pd, value);
}

void common_cmppdXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[r1].pd = simde_mm_cmpeq_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 1: cpu->xmm[r1].pd = simde_mm_cmplt_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 2: cpu->xmm[r1].pd = simde_mm_cmple_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 3: cpu->xmm[r1].pd = simde_mm_cmpunord_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 4: cpu->xmm[r1].pd = simde_mm_cmpneq_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 5: cpu->xmm[r1].pd = simde_mm_cmpnlt_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 6: cpu->xmm[r1].pd = simde_mm_cmpnle_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 7: cpu->xmm[r1].pd = simde_mm_cmpord_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    }
}

void common_cmppdXmmE128(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[reg].pd = simde_mm_cmpeq_pd(cpu->xmm[reg].pd, value); break;
    case 1: cpu->xmm[reg].pd = simde_mm_cmplt_pd(cpu->xmm[reg].pd, value); break;
    case 2: cpu->xmm[reg].pd = simde_mm_cmple_pd(cpu->xmm[reg].pd, value); break;
    case 3: cpu->xmm[reg].pd = simde_mm_cmpunord_pd(cpu->xmm[reg].pd, value); break;
    case 4: cpu->xmm[reg].pd = simde_mm_cmpneq_pd(cpu->xmm[reg].pd, value); break;
    case 5: cpu->xmm[reg].pd = simde_mm_cmpnlt_pd(cpu->xmm[reg].pd, value); break;
    case 6: cpu->xmm[reg].pd = simde_mm_cmpnle_pd(cpu->xmm[reg].pd, value); break;
    case 7: cpu->xmm[reg].pd = simde_mm_cmpord_pd(cpu->xmm[reg].pd, value); break;
    }
}

void common_cmpsdXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[r1].pd = simde_mm_cmpeq_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 1: cpu->xmm[r1].pd = simde_mm_cmplt_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 2: cpu->xmm[r1].pd = simde_mm_cmple_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 3: cpu->xmm[r1].pd = simde_mm_cmpunord_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 4: cpu->xmm[r1].pd = simde_mm_cmpneq_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 5: cpu->xmm[r1].pd = simde_mm_cmpnlt_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 6: cpu->xmm[r1].pd = simde_mm_cmpnle_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    case 7: cpu->xmm[r1].pd = simde_mm_cmpord_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd); break;
    }
}

void common_cmpsdXmmE64(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    int which = imm & 7;
    switch (which) {
    case 0: cpu->xmm[reg].pd = simde_mm_cmpeq_sd(cpu->xmm[reg].pd, value); break;
    case 1: cpu->xmm[reg].pd = simde_mm_cmplt_sd(cpu->xmm[reg].pd, value); break;
    case 2: cpu->xmm[reg].pd = simde_mm_cmple_sd(cpu->xmm[reg].pd, value); break;
    case 3: cpu->xmm[reg].pd = simde_mm_cmpunord_sd(cpu->xmm[reg].pd, value); break;
    case 4: cpu->xmm[reg].pd = simde_mm_cmpneq_sd(cpu->xmm[reg].pd, value); break;
    case 5: cpu->xmm[reg].pd = simde_mm_cmpnlt_sd(cpu->xmm[reg].pd, value); break;
    case 6: cpu->xmm[reg].pd = simde_mm_cmpnle_sd(cpu->xmm[reg].pd, value); break;
    case 7: cpu->xmm[reg].pd = simde_mm_cmpord_sd(cpu->xmm[reg].pd, value); break;
    }
}

void common_comisdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->fillFlags();
    cpu->flags&=~(AF|OF|SF|CF|PF|ZF);
    const simde__m128d& a = cpu->xmm[r1].pd;
    const simde__m128d& b = cpu->xmm[r2].pd;
    if (isnan(a.f64[0]) || isnan(b.f64[0])) {
        cpu->flags|=CF|ZF|PF;
    } else if (a.f64[0] == b.f64[0]) {
        cpu->flags|=ZF;
    } else if (a.f64[0] < b.f64[0]) {
        cpu->flags|=CF;
    }
}

void common_comisdXmmE64(CPU* cpu, U32 reg, U32 address) {
    cpu->fillFlags();
    cpu->flags&=~(AF|OF|SF|CF|PF|ZF);
    const simde__m128d& a = cpu->xmm[reg].pd;
    simde__m128d b;
    b.u64[0] = cpu->memory->readq(address);
    if (isnan(a.f64[0]) || isnan(b.f64[0])) {
        cpu->flags|=CF|ZF|PF;
    } else if (a.f64[0] == b.f64[0]) {
        cpu->flags|=ZF;
    } else if (a.f64[0] < b.f64[0]) {
        cpu->flags|=CF;
    }
}

void common_ucomisdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    common_comisdXmmXmm(cpu, r1, r2);
}

void common_ucomisdXmmE64(CPU* cpu, U32 reg, U32 address) {
    common_comisdXmmE64(cpu, reg, address);
}

void common_pcmpgtbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cmpgt_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pcmpgtbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cmpgt_epi8(cpu->xmm[reg].pi, value);
}

void common_pcmpgtwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cmpgt_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pcmpgtwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cmpgt_epi16(cpu->xmm[reg].pi, value);
}

void common_pcmpgtdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cmpgt_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pcmpgtdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cmpgt_epi32(cpu->xmm[reg].pi, value);
}

void common_pcmpeqbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cmpeq_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pcmpeqbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cmpeq_epi8(cpu->xmm[reg].pi, value);
}

void common_pcmpeqwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cmpeq_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pcmpeqwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cmpeq_epi16(cpu->xmm[reg].pi, value);
}

void common_pcmpeqdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cmpeq_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pcmpeqdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cmpeq_epi32(cpu->xmm[reg].pi, value);
}

void common_cvtdq2pdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_cvtepi32_pd(cpu->xmm[r2].pi);
}

void common_cvtdq2pdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_cvtepi32_pd(value);
}

void common_cvtdq2psXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_cvtepi32_ps(cpu->xmm[r2].pi);
}

void common_cvtdq2psXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_cvtepi32_ps(value);
}

void common_cvtpd2piMmxXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = simde_mm_cvtpd_pi32(cpu->xmm[r2].pd).u64[0];
}

void common_cvtpd2piMmxE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->reg_mmx[reg].q = simde_mm_cvtpd_pi32(value).u64[0];
}

void common_cvtpd2dqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cvtpd_epi32(cpu->xmm[r2].pd);
}

void common_cvtpd2dqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cvtpd_epi32(value);
}

void common_cvtpd2psXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_cvtpd_ps(cpu->xmm[r2].pd);
}

void common_cvtpd2psXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_cvtpd_ps(value);
}

void common_cvtpi2pdXmmMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 value;
    value.u64[0] = cpu->reg_mmx[r2].q;
    cpu->xmm[r1].pd = simde_mm_cvtpi32_pd(value);
}

void common_cvtpi2pdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m64 value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_cvtpi32_pd(value);
}

void common_cvtps2dqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cvtps_epi32(cpu->xmm[r2].ps);
}

void common_cvtps2dqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cvtps_epi32(value);
}

void common_cvtps2pdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_cvtps_pd(cpu->xmm[r2].ps);
}

void common_cvtps2pdXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd = simde_mm_cvtps_pd(value);
}

void common_cvtsd2siR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = simde_mm_cvtsd_si32(cpu->xmm[r2].pd);
}

void common_cvtsd2siR32E64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    //value.u64[1] = cpu->memory->readq(address+8);
    cpu->reg[reg].u32 = simde_mm_cvtsd_si32(value);
}

void common_cvtsd2ssXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].ps = simde_mm_cvtsd_ss(cpu->xmm[r1].ps, cpu->xmm[r2].pd);
}

void common_cvtsd2ssXmmE64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    //value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].ps = simde_mm_cvtsd_ss(cpu->xmm[reg].ps, value);
}

void common_cvtsi2sdXmmR32(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_cvtsi32_sd(cpu->xmm[r1].pd, cpu->reg[r2].u32);
}

void common_cvtsi2sdXmmE32(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pd = simde_mm_cvtsi32_sd(cpu->xmm[reg].pd, cpu->memory->readd(address));
}

void common_cvtss2sdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_cvtss_sd(cpu->xmm[r1].pd, cpu->xmm[r2].ps);
}

void common_cvtss2sdXmmE32(CPU* cpu, U32 reg, U32 address) {
    simde__m128 value;
    value.u32[0] = cpu->memory->readd(address);
    cpu->xmm[reg].pd = simde_mm_cvtss_sd(cpu->xmm[reg].pd, value);
}

void common_cvttpd2piMmxXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = simde_mm_cvttpd_pi32(cpu->xmm[r2].pd).u64[0];
}

void common_cvttpd2piMmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->reg_mmx[reg].q = simde_mm_cvttpd_pi32(value).u64[0];
}

void common_cvttpd2dqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cvttpd_epi32(cpu->xmm[r2].pd);
}

void common_cvttpd2dqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cvttpd_epi32(value);
}

void common_cvttps2dqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cvttps_epi32(cpu->xmm[r2].ps);
}

void common_cvttps2dqXmmE128(CPU* cpu,U32 reg, U32 address ) {
    simde__m128 value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_cvttps_epi32(value);
}

void common_cvttsd2siR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = simde_mm_cvttsd_si32(cpu->xmm[r2].pd);
}

void common_cvttsd2siR32E64(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->reg[reg].u32 = simde_mm_cvttsd_si32(value);
}

void common_movqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_move_epi64(cpu->xmm[r2].pi);
}

void common_movqE64Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pi.u64[0]);
}

void common_movqXmmE64(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pi.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pi.u64[1] = 0;
}

void common_movsdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_move_sd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_movsdXmmE64(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pd.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd.u64[1] = 0; // yes, memory to reg will 0 out the top, but xmm to xmm does not, unlike movq
}

void common_movsdE64Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pd.u64[0]);
}

void common_movapdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = cpu->xmm[r2].pd;
}

void common_movapdXmmE128(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pd.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd.u64[1] = cpu->memory->readq(address+8);
}

void common_movapdE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pd.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].pd.u64[1]);
}

void common_movupdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = cpu->xmm[r2].pd;
}

void common_movupdXmmE128(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pd.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pd.u64[1] = cpu->memory->readq(address+8);
}

void common_movupdE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pd.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].pd.u64[1]);
}

void common_movhpdXmmE64(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pd.u64[1] = cpu->memory->readq(address);
}

void common_movhpdE64Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pd.u64[1]);
}

void common_movlpdXmmE64(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pd.u64[0] = cpu->memory->readq(address);
}

void common_movlpdE64Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pd.u64[0]);
}

void common_movmskpdR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = simde_mm_movemask_pd(cpu->xmm[r2].pd);
}

void common_movdXmmR32(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_cvtsi32_si128(cpu->reg[r2].u32);
}

void common_movdXmmE32(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pi = simde_mm_cvtsi32_si128(cpu->memory->readd(address));
}

void common_movdR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = simde_mm_cvtsi128_si32(cpu->xmm[r2].pi);
}

void common_movdE32Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writed(address, simde_mm_cvtsi128_si32(cpu->xmm[reg].pi));
}

void common_movdqaXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = cpu->xmm[r2].pi;
}

void common_movdqaXmmE128(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pi.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pi.u64[1] = cpu->memory->readq(address+8);
}

void common_movdqaE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pi.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].pi.u64[1]);
}

void common_movdquXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = cpu->xmm[r2].pi;
}

void common_movdquXmmE128(CPU* cpu, U32 reg, U32 address) {
    cpu->xmm[reg].pi.u64[0] = cpu->memory->readq(address);
    cpu->xmm[reg].pi.u64[1] = cpu->memory->readq(address+8);
}

void common_movdquE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pi.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].pi.u64[1]);
}

void common_movdq2qMmxXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg_mmx[r1].q = simde_mm_movepi64_pi64(cpu->xmm[r2].pi).u64[0];
}

void common_movq2dqXmmMmx(CPU* cpu, U32 r1, U32 r2) {
    simde__m64 value;
    value.u64[0] = cpu->reg_mmx[r2].q;
    cpu->xmm[r1].pi = simde_mm_movpi64_epi64(value);
}

void common_movntpdE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pd.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].pd.u64[1]);
}

void common_movntdqE128Xmm(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writeq(address, cpu->xmm[reg].pd.u64[0]);
    cpu->memory->writeq(address+8, cpu->xmm[reg].pd.u64[1]);
}

void common_movntiE32R32(CPU* cpu, U32 reg, U32 address) {
    cpu->memory->writed(address, cpu->reg[reg].u32);
}

void common_maskmovdquE128XmmXmm(CPU* cpu, U32 r1, U32 r2, U32 address) {
    int8_t result[16];
    cpu->memory->memcpy((U8*)result, address, 16);
    simde_mm_maskmoveu_si128(cpu->xmm[r1].pi, cpu->xmm[r2].pi, result);
    cpu->memory->memcpy(address, (U8*)result, 16);
}

#define G(rm) ((rm >> 3) & 7)
#define E(rm) (rm & 7)

void common_maskmovdquE128XmmXmmRM(CPU* cpu, U32 rm, U32 base, U32 bigAddress) {
    int8_t result[16] = {};
    U32 address = (bigAddress ? EDI : DI) + cpu->seg[base].address;
    cpu->memory->memcpy((U8*)result, address, 16);
    simde_mm_maskmoveu_si128(cpu->xmm[G(rm)].pi, cpu->xmm[E(rm)].pi, result);
    cpu->memory->memcpy(address, (U8*)result, 16);
}

void common_pshufdXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_shuffle_epi32(cpu->xmm[r2].pi, imm);
}

void common_pshufdXmmE128(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_shuffle_epi32(value, imm);
}

void common_pshufhwXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_shufflehi_epi16(cpu->xmm[r2].pi, imm);
}

void common_pshufhwXmmE128(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_shufflehi_epi16(value, imm);
}

void common_pshuflwXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_shufflelo_epi16(cpu->xmm[r2].pi, imm);
}

void common_pshuflwXmmE128(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_shufflelo_epi16(value, imm);
}

void common_unpckhpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_unpackhi_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_unpckhpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_unpackhi_pd(cpu->xmm[reg].pd, value);
}

void common_unpcklpdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pd = simde_mm_unpacklo_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd);
}

void common_unpcklpdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_unpacklo_pd(cpu->xmm[reg].pd, value);
}

void common_punpckhbwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpackhi_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpckhbwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpackhi_epi8(cpu->xmm[reg].pi, value);
}

void common_punpckhwdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpackhi_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpckhwdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpackhi_epi16(cpu->xmm[reg].pi, value);
}

void common_punpckhdqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpackhi_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpckhdqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpackhi_epi32(cpu->xmm[reg].pi, value);
}

void common_punpckhqdqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpackhi_epi64(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpckhqdqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpackhi_epi64(cpu->xmm[reg].pi, value);
}

void common_punpcklbwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpacklo_epi8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpcklbwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpacklo_epi8(cpu->xmm[reg].pi, value);
}

void common_punpcklwdXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpacklo_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpcklwdXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpacklo_epi16(cpu->xmm[reg].pi, value);
}

void common_punpckldqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpacklo_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpckldqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpacklo_epi32(cpu->xmm[reg].pi, value);
}

void common_punpcklqdqXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_unpacklo_epi64(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_punpcklqdqXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_unpacklo_epi64(cpu->xmm[reg].pi, value);
}

void common_packssdwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_packs_epi32(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_packssdwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_packs_epi32(cpu->xmm[reg].pi, value);
}

void common_packsswbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_packs_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_packsswbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_packs_epi16(cpu->xmm[reg].pi, value);
}

void common_packuswbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_packus_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_packuswbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_packus_epi16(cpu->xmm[reg].pi, value);
}

void common_shufpdXmmXmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pd = simde_mm_shuffle_pd(cpu->xmm[r1].pd, cpu->xmm[r2].pd, imm);
}

void common_shufpdXmmE128(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128d value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pd = simde_mm_shuffle_pd(cpu->xmm[reg].pd, value, imm);
}

void common_pause(CPU* cpu) {
}

void common_pavgbXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_avg_epu8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pavgbXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_avg_epu8(cpu->xmm[reg].pi, value);
}

void common_pavgwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_avg_epu16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pavgwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_avg_epu16(cpu->xmm[reg].pi, value);
}

void common_psadbwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_sad_epu8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_psadbwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_sad_epu8(cpu->xmm[reg].pi, value);
}

void common_pextrwR32Xmm(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->reg[r1].u32 = simde_mm_extract_epi16(cpu->xmm[r2].pi, imm);
}

void common_pextrwE16Xmm(CPU* cpu, U32 reg, U32 address, U8 imm) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->reg[reg].u32 = simde_mm_extract_epi16(value, imm);
}

void common_pinsrwXmmR32(CPU* cpu, U32 r1, U32 r2, U8 imm) {
    cpu->xmm[r1].pi = simde_mm_insert_epi16(cpu->xmm[r1].pi, cpu->reg[r2].u32, imm);
}

void common_pinsrwXmmE16(CPU* cpu, U32 reg, U32 address, U8 imm) {
    cpu->xmm[reg].pi = simde_mm_insert_epi16(cpu->xmm[reg].pi, cpu->memory->readw(address), imm);
}

void common_pmaxswXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_max_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pmaxswXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_max_epi16(cpu->xmm[reg].pi, value);
}

void common_pmaxubXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_max_epu8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pmaxubXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_max_epu8(cpu->xmm[reg].pi, value);
}

void common_pminswXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_min_epi16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pminswXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_min_epi16(cpu->xmm[reg].pi, value);
}

void common_pminubXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_min_epu8(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pminubXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_min_epu8(cpu->xmm[reg].pi, value);
}

void common_pmovmskbR32Xmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->reg[r1].u32 = simde_mm_movemask_epi8(cpu->xmm[r2].pi);
}

void common_pmulhuwXmmXmm(CPU* cpu, U32 r1, U32 r2) {
    cpu->xmm[r1].pi = simde_mm_mulhi_epu16(cpu->xmm[r1].pi, cpu->xmm[r2].pi);
}

void common_pmulhuwXmmE128(CPU* cpu, U32 reg, U32 address) {
    simde__m128i value;
    value.u64[0] = cpu->memory->readq(address);
    value.u64[1] = cpu->memory->readq(address+8);
    cpu->xmm[reg].pi = simde_mm_mulhi_epu16(cpu->xmm[reg].pi, value);
}

void common_lfence(CPU* cpu) {
}

void common_mfence(CPU* cpu) {
}

void common_clflush(CPU* cpu, U32 address) {
}