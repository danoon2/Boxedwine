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

#ifndef __LAZY_FLAGS_H__
#define __LAZY_FLAGS_H__

class CPU;

class LazyFlags {
public:
    LazyFlags(U32 width) : width(width) {}
    virtual U32 getCF(CPU* cpu) const=0; // will always return 0 or 1, optimizations count on this
    virtual U32 getSF(CPU* cpu) const=0;
    virtual U32 getZF(CPU* cpu) const=0;
    virtual U32 getOF(CPU* cpu) const=0;
    virtual U32 getAF(CPU* cpu) const=0;
    virtual U32 getPF(CPU* cpu) const=0;
    virtual bool usesResult(U32 mask) const= 0;
    virtual bool usesSrc(U32 mask) const= 0;
    virtual bool usesDst(U32 mask) const= 0;
    virtual bool usesOldCF(U32 mask) const = 0;
    U32 width;
};

extern const LazyFlags* lazyFlags[51];

enum LazyFlagType : unsigned char {    
    FLAGS_NONE,
    FLAGS_ADD8,
    FLAGS_ADD16,
    FLAGS_ADD32,
    FLAGS_OR8,
    FLAGS_OR16,
    FLAGS_OR32,
    FLAGS_ADC8,
    FLAGS_ADC16,
    FLAGS_ADC32,
    FLAGS_SBB8,
    FLAGS_SBB16,
    FLAGS_SBB32,
    FLAGS_AND8,
    FLAGS_AND16,
    FLAGS_AND32,
    FLAGS_SUB8,
    FLAGS_SUB16,
    FLAGS_SUB32,
    FLAGS_XOR8,
    FLAGS_XOR16,
    FLAGS_XOR32,
    FLAGS_INC8,
    FLAGS_INC16,
    FLAGS_INC32,
    FLAGS_DEC8,
    FLAGS_DEC16,
    FLAGS_DEC32,
    FLAGS_SHL8,
    FLAGS_SHL16,
    FLAGS_SHL32,
    FLAGS_SHR8,
    FLAGS_SHR16,
    FLAGS_SHR32,
    FLAGS_SAR8,
    FLAGS_SAR16,
    FLAGS_SAR32,
    FLAGS_CMP8,
    FLAGS_CMP16,
    FLAGS_CMP32,
    FLAGS_TEST8,
    FLAGS_TEST16,
    FLAGS_TEST32,
    FLAGS_DSHL16,
    FLAGS_DSHL32,
    FLAGS_DSHR16,
    FLAGS_DSHR32,
    FLAGS_NEG8,
    FLAGS_NEG16,
    FLAGS_NEG32,
    FLAGS_NULL
};

#endif