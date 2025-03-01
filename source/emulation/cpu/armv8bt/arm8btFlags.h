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

#ifndef __ARM8BT_FLAGS_H__
#define __ARM8BT_FLAGS_H__

#ifdef BOXEDWINE_ARMV8BT

class Armv8btAsm;

class Arm8BtFlags {
public:
    Arm8BtFlags(U32 width) : width(width) {}
    virtual void setCF(Armv8btAsm* data, U8 reg) = 0;
    virtual void setSF(Armv8btAsm* data, U8 reg) = 0;
    virtual void setZF(Armv8btAsm* data, U8 reg) = 0;
    virtual void setOF(Armv8btAsm* data, U8 reg) = 0;
    virtual void setAF(Armv8btAsm* data, U8 reg) = 0;
    virtual void setPF(Armv8btAsm* data, U8 reg) = 0;
    void setAll(Armv8btAsm* data, U8 reg) {
        setCF(data, reg);
        setSF(data, reg);
        setZF(data, reg);
        setOF(data, reg);
        setAF(data, reg);
        setPF(data, reg);
    }
    void setFlags(Armv8btAsm* data, U32 mask, U8 reg=xFLAGS) {
        if (mask & CF) {
            setCF(data, reg);
        }
        if (mask & ZF) {
            setZF(data, reg);
        }
        if (mask & SF) {
            setSF(data, reg);
        }
        if (mask & OF) {
            setOF(data, reg);
        }
        if (mask & AF) {
            setAF(data, reg);
        }
        if (mask & PF) {
            setPF(data, reg);
        }
    }
    virtual bool usesHardwareFlags(U32 mask) {
        return false;
    }
    virtual bool usesResult(U32 mask) = 0;
    virtual bool usesSrc(U32 mask) = 0;
    virtual bool usesDst(U32 mask) = 0;
    U32 width;
};

extern Arm8BtFlags* ARM8BT_FLAGS_ADD8;
extern Arm8BtFlags* ARM8BT_FLAGS_ADD16;
extern Arm8BtFlags* ARM8BT_FLAGS_ADD32;
extern Arm8BtFlags* ARM8BT_FLAGS_OR8;
extern Arm8BtFlags* ARM8BT_FLAGS_OR16;
extern Arm8BtFlags* ARM8BT_FLAGS_OR32;
extern Arm8BtFlags* ARM8BT_FLAGS_ADC8;
extern Arm8BtFlags* ARM8BT_FLAGS_ADC16;
extern Arm8BtFlags* ARM8BT_FLAGS_ADC32;
extern Arm8BtFlags* ARM8BT_FLAGS_SBB8;
extern Arm8BtFlags* ARM8BT_FLAGS_SBB16;
extern Arm8BtFlags* ARM8BT_FLAGS_SBB32;
extern Arm8BtFlags* ARM8BT_FLAGS_AND8;
extern Arm8BtFlags* ARM8BT_FLAGS_AND16;
extern Arm8BtFlags* ARM8BT_FLAGS_AND32;
extern Arm8BtFlags* ARM8BT_FLAGS_SUB8;
extern Arm8BtFlags* ARM8BT_FLAGS_SUB16;
extern Arm8BtFlags* ARM8BT_FLAGS_SUB32;
extern Arm8BtFlags* ARM8BT_FLAGS_XOR8;
extern Arm8BtFlags* ARM8BT_FLAGS_XOR16;
extern Arm8BtFlags* ARM8BT_FLAGS_XOR32;
extern Arm8BtFlags* ARM8BT_FLAGS_INC8;
extern Arm8BtFlags* ARM8BT_FLAGS_INC16;
extern Arm8BtFlags* ARM8BT_FLAGS_INC32;
extern Arm8BtFlags* ARM8BT_FLAGS_DEC8;
extern Arm8BtFlags* ARM8BT_FLAGS_DEC16;
extern Arm8BtFlags* ARM8BT_FLAGS_DEC32;
extern Arm8BtFlags* ARM8BT_FLAGS_SHL8;
extern Arm8BtFlags* ARM8BT_FLAGS_SHL16;
extern Arm8BtFlags* ARM8BT_FLAGS_SHL32;
extern Arm8BtFlags* ARM8BT_FLAGS_SHR8;
extern Arm8BtFlags* ARM8BT_FLAGS_SHR16;
extern Arm8BtFlags* ARM8BT_FLAGS_SHR32;
extern Arm8BtFlags* ARM8BT_FLAGS_SAR8;
extern Arm8BtFlags* ARM8BT_FLAGS_SAR16;
extern Arm8BtFlags* ARM8BT_FLAGS_SAR32;
extern Arm8BtFlags* ARM8BT_FLAGS_CMP8;
extern Arm8BtFlags* ARM8BT_FLAGS_CMP16;
extern Arm8BtFlags* ARM8BT_FLAGS_CMP32;
extern Arm8BtFlags* ARM8BT_FLAGS_TEST8;
extern Arm8BtFlags* ARM8BT_FLAGS_TEST16;
extern Arm8BtFlags* ARM8BT_FLAGS_TEST32;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHL16;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHL16_CL;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHL32;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHL32_CL;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHR16;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHR16_CL;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHR32;
extern Arm8BtFlags* ARM8BT_FLAGS_DSHR32_CL;
extern Arm8BtFlags* ARM8BT_FLAGS_NEG8;
extern Arm8BtFlags* ARM8BT_FLAGS_NEG16;
extern Arm8BtFlags* ARM8BT_FLAGS_NEG32;
#endif

#endif