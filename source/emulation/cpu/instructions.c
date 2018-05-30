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

#include "cpu.h"
#include "op.h"

void instruction_daa(struct CPU* cpu) {
    if (((AL & 0x0F)>0x09) || getAF(cpu)) {
        if ((AL > 0x99) || getCF(cpu)) {
            AL+=0x60;
            addFlag(CF);
        } else {
            removeFlag(CF);
        }
        AL+=0x06;
        addFlag(AF);
    } else {
        if ((AL > 0x99) || getCF(cpu)) {
            AL+=0x60;
            addFlag(CF);
        } else {
            removeFlag(CF);
        }
        removeFlag(AF);
    }
    setSF(cpu,(AL & 0x80));
    setZF(cpu,(AL == 0));
    setPF(cpu,parity_lookup[AL]);
    cpu->lazyFlags=FLAGS_NONE;
}

void instruction_das(struct CPU* cpu) {
    U8 osigned=AL & 0x80;
    if (((AL & 0x0f) > 9) || getAF(cpu)) {
        if ((AL>0x99) || getCF(cpu)) {
            AL-=0x60;
            addFlag(CF);
        } else {
            setCF(cpu,(AL<=0x05));
        }
        AL-=6;
        addFlag(AF);
    } else {
        if ((AL>0x99) || getCF(cpu)) {
            AL-=0x60;
            addFlag(CF);
        } else {
            removeFlag(CF);
        }
        removeFlag(AF);
    }
    setOF(cpu,osigned && ((AL & 0x80)==0));
    setSF(cpu,(AL & 0x80));
    setZF(cpu,(AL==0));
    setPF(cpu,parity_lookup[AL]);
    cpu->lazyFlags=FLAGS_NONE;
}

void instruction_aaa(struct CPU* cpu) {
    setSF(cpu,((AL>=0x7a) && (AL<=0xf9)));
    if ((AL & 0xf) > 9) {
        setOF(cpu,(AL & 0xf0)==0x70);
        AX += 0x106;
        addFlag(CF);
        setZF(cpu,(AL == 0));
        addFlag(AF);
    } else if (getAF(cpu)) {
        AX += 0x106;
        removeFlag(OF);
        addFlag(CF);
        removeFlag(ZF);
        addFlag(AF);
    } else {
        removeFlag(OF);
        removeFlag(CF);
        setZF(cpu,(AL == 0));
        removeFlag(AF);
    }
    setPF(cpu,parity_lookup[AL]);
    AL &= 0x0F;
    cpu->lazyFlags=FLAGS_NONE;
}

void instruction_aas(struct CPU* cpu) {
    if ((AL & 0x0f)>9) {
        setSF(cpu,(AL>0x85));
        AX -= 0x106;
        removeFlag(OF);
        addFlag(CF);
        addFlag(AF);
    } else if (getAF(cpu)) {
        setOF(cpu,((AL>=0x80) && (AL<=0x85)));
        setSF(cpu,(AL<0x06) || (AL>0x85));
        AX -= 0x106;
        addFlag(CF);
        addFlag(AF);
    } else {
        setSF(cpu,(AL>=0x80));
        removeFlag(OF);
        removeFlag(CF);
        removeFlag(AF);
    }
    setZF(cpu,(AL == 0));
    setPF(cpu,parity_lookup[AL]);
    AL &= 0x0F;
    cpu->lazyFlags=FLAGS_NONE;
}

void instruction_aad(struct CPU* cpu, U32 value) {
    AL = AH * value + AL;
    AH = 0;
    setSF(cpu, AL & 0x80);
    setZF(cpu, AL == 0);		
    setPF(cpu,parity_lookup[AL]);
    removeFlag(CF);
    removeFlag(OF);
    removeFlag(AF);
    cpu->lazyFlags = FLAGS_NONE;
}

void instruction_aam(struct CPU* cpu, U32 value) {
    if (value) {
        AH = AL / value;
        AL = AL % value;
        setSF(cpu, AL & 0x80);
        setZF(cpu, AL == 0);		
        setPF(cpu,parity_lookup[AL]);
        removeFlag(CF);
        removeFlag(OF);
        removeFlag(AF);
        cpu->lazyFlags = FLAGS_NONE;
    } else {
        exception(cpu, EXCEPTION_DIVIDE);
    } 
}