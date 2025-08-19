/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

INIT_CPU_LOCK(CmpXchg8b, cmpxchg8b);
INIT_CPU_LOCK(CmpXchgE32R32, cmpxchge32r32);
INIT_CPU_LOCK(CmpXchgE16R16, cmpxchge16r16);
INIT_CPU_LOCK(CmpXchgE8R8, cmpxchge8r8);
INIT_CPU_LOCK(XchgE8R8, xchge8r8)
INIT_CPU_LOCK(XchgE16R16, xchge16r16)
INIT_CPU_LOCK(XchgE32R32, xchge32r32)
INIT_CPU_LOCK(XaddR8E8, xaddr8e8)
INIT_CPU_LOCK(XaddR16E16, xaddr16e16)
INIT_CPU_LOCK(XaddR32E32, xaddr32e32)
INIT_CPU_LOCK(AddE8R8, adde8r8)
INIT_CPU_LOCK(AddE8I8, add8_mem)
INIT_CPU_LOCK(AddE16R16, adde16r16)
INIT_CPU_LOCK(AddE16I16, add16_mem)
INIT_CPU_LOCK(AddE32R32, adde32r32)
INIT_CPU_LOCK(AddE32I32, add32_mem)
INIT_CPU_LOCK(OrE8R8, ore8r8)
INIT_CPU_LOCK(OrE8I8, or8_mem)
INIT_CPU_LOCK(OrE16R16, ore16r16)
INIT_CPU_LOCK(OrE16I16, or16_mem)
INIT_CPU_LOCK(OrE32R32, ore32r32)
INIT_CPU_LOCK(OrE32I32, or32_mem)
INIT_CPU_LOCK(AdcE8R8, adce8r8)
INIT_CPU_LOCK(AdcE8I8, adc8_mem)
INIT_CPU_LOCK(AdcE16R16, adce16r16)
INIT_CPU_LOCK(AdcE16I16, adc16_mem)
INIT_CPU_LOCK(AdcE32R32, adce32r32)
INIT_CPU_LOCK(AdcE32I32, adc32_mem)
INIT_CPU_LOCK(SbbE8R8, sbbe8r8)
INIT_CPU_LOCK(SbbE8I8, sbb8_mem)
INIT_CPU_LOCK(SbbE16R16, sbbe16r16)
INIT_CPU_LOCK(SbbE16I16, sbb16_mem)
INIT_CPU_LOCK(SbbE32R32, sbbe32r32)
INIT_CPU_LOCK(SbbE32I32, sbb32_mem)
INIT_CPU_LOCK(AndE8R8, ande8r8)
INIT_CPU_LOCK(AndE8I8, and8_mem)
INIT_CPU_LOCK(AndE16R16, ande16r16)
INIT_CPU_LOCK(AndE16I16, and16_mem)
INIT_CPU_LOCK(AndE32R32, ande32r32)
INIT_CPU_LOCK(AndE32I32, and32_mem)
INIT_CPU_LOCK(SubE8R8, sube8r8)
INIT_CPU_LOCK(SubE8I8, sub8_mem)
INIT_CPU_LOCK(SubE16R16, sube16r16)
INIT_CPU_LOCK(SubE16I16, sub16_mem)
INIT_CPU_LOCK(SubE32R32, sube32r32)
INIT_CPU_LOCK(SubE32I32, sub32_mem)
INIT_CPU_LOCK(XorE8R8, xore8r8)
INIT_CPU_LOCK(XorE8I8, xor8_mem)
INIT_CPU_LOCK(XorE16R16, xore16r16)
INIT_CPU_LOCK(XorE16I16, xor16_mem)
INIT_CPU_LOCK(XorE32R32, xore32r32)
INIT_CPU_LOCK(XorE32I32, xor32_mem)
INIT_CPU_LOCK(IncE8, inc8_mem8)
INIT_CPU_LOCK(IncE16, inc16_mem16)
INIT_CPU_LOCK(IncE32, inc32_mem32)
INIT_CPU_LOCK(DecE8, dec8_mem8)
INIT_CPU_LOCK(DecE16, dec16_mem16)
INIT_CPU_LOCK(DecE32, dec32_mem32)
INIT_CPU_LOCK(NotE8, note8)
INIT_CPU_LOCK(NotE16, note16)
INIT_CPU_LOCK(NotE32, note32)
INIT_CPU_LOCK(NegE8, nege8)
INIT_CPU_LOCK(NegE16, nege16)
INIT_CPU_LOCK(NegE32, nege32)
INIT_CPU_LOCK(BtsE16R16, btse16r16)
INIT_CPU_LOCK(BtsE16, btse16)
INIT_CPU_LOCK(BtsE32R32, btse32r32)
INIT_CPU_LOCK(BtsE32, btse32)
INIT_CPU_LOCK(BtrE16R16, btre16r16)
INIT_CPU_LOCK(BtrE16, btre16)
INIT_CPU_LOCK(BtrE32R32, btre32r32)
INIT_CPU_LOCK(BtrE32, btre32)
INIT_CPU_LOCK(BtcE16R16, btce16r16)
INIT_CPU_LOCK(BtcE16, btce16)
INIT_CPU_LOCK(BtcE32R32, btce32r32)
INIT_CPU_LOCK(BtcE32, btce32)