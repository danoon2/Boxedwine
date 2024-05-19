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
void movsb16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    cpu->memory->writeb(dBase+DI, cpu->memory->readb(sBase+SI));
    DI+=inc;
    SI+=inc;
}
void movsb16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writeb(dBase+DI, cpu->memory->readb(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsb32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    cpu->memory->writeb(dBase+EDI, cpu->memory->readb(sBase+ESI));
    EDI+=inc;
    ESI+=inc;
}
void movsb32r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writeb(dBase+EDI, cpu->memory->readb(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
        ECX--;
    }
}
void movsw16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    cpu->memory->writew(dBase+DI, cpu->memory->readw(sBase+SI));
    DI+=inc;
    SI+=inc;
}
void movsw16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;

    for (U32 i=0;i<count;i++) {
        cpu->memory->writew(dBase+DI, cpu->memory->readw(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsw32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    cpu->memory->writew(dBase+EDI, cpu->memory->readw(sBase+ESI));
    EDI+=inc;
    ESI+=inc;
}
void movsw32r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = ECX;

    for (U32 i=0;i<count;i++) {
        cpu->memory->writew(dBase+EDI, cpu->memory->readw(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
        ECX--;
    }
}
void movsd16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    cpu->memory->writed(dBase+DI, cpu->memory->readd(sBase+SI));
    DI+=inc;
    SI+=inc;
}
void movsd16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        cpu->memory->writed(dBase+DI, cpu->memory->readd(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsd32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    cpu->memory->writed(dBase+EDI, cpu->memory->readd(sBase+ESI));
    EDI+=inc;
    ESI+=inc;
}
void movsd32r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        cpu->memory->writed(dBase+EDI, cpu->memory->readd(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
        ECX--;
    }
}
void cmpsb16(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase + DI);
    U8 v2 = cpu->memory->readb(sBase+SI);

    DI+=inc;
    SI+=inc;
    cpu->dst.u8 = v2;
    cpu->src.u8 = v1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
}
void cmpsb16r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;

    if (count) {
        U8 v1=0;
        U8 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+DI);
            v2 = cpu->memory->readb(sBase+SI);
            DI+=inc;
            SI+=inc;
            CX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u8 = v2;
        cpu->src.u8 = v1;
        cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
        cpu->lazyFlags = FLAGS_SUB8;
    }
}
void cmpsb32(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase + EDI);
    U8 v2 = cpu->memory->readb(sBase+ESI);

    EDI+=inc;
    ESI+=inc;
    cpu->dst.u8 = v2;
    cpu->src.u8 = v1;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
}
void cmpsb32r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = ECX;
    if (count) {
        U8 v1=0;
        U8 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+EDI);
            v2 = cpu->memory->readb(sBase+ESI);
            EDI+=inc;
            ESI+=inc;
            ECX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u8 = v2;
        cpu->src.u8 = v1;
        cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
        cpu->lazyFlags = FLAGS_SUB8;
    }
}
void cmpsw16(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase + DI);
    U16 v2 = cpu->memory->readw(sBase + SI);

    DI+=inc;
    SI+=inc;
    cpu->dst.u16 = v2;
    cpu->src.u16 = v1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
}
void cmpsw16r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    if (count) {
        U16 v1=0;
        U16 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+DI);
            v2 = cpu->memory->readw(sBase+SI);
            DI+=inc;
            SI+=inc;
            CX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u16 = v2;
        cpu->src.u16 = v1;
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        cpu->lazyFlags = FLAGS_SUB16;
    }
}
void cmpsw32(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase + EDI);
    U16 v2 = cpu->memory->readw(sBase + ESI);

    EDI+=inc;
    ESI+=inc;
    cpu->dst.u16 = v2;
    cpu->src.u16 = v1;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
}
void cmpsw32r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = ECX;
    if (count) {
        U16 v1=0;
        U16 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+EDI);
            v2 = cpu->memory->readw(sBase+ESI);
            EDI+=inc;
            ESI+=inc;
            ECX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u16 = v2;
        cpu->src.u16 = v1;
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
        cpu->lazyFlags = FLAGS_SUB16;
    }
}
void cmpsd16(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase + DI);
    U32 v2 = cpu->memory->readd(sBase + SI);

    DI+=inc;
    SI+=inc;
    cpu->dst.u32 = v2;
    cpu->src.u32 = v1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
}
void cmpsd16r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    if (count) {
        U32 v1=0;
        U32 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+DI);
            v2 = cpu->memory->readd(sBase+SI);
            DI+=inc;
            SI+=inc;
            CX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u32 = v2;
        cpu->src.u32 = v1;
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        cpu->lazyFlags = FLAGS_SUB32;
    }
}
void cmpsd32(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase + EDI);
    U32 v2 = cpu->memory->readd(sBase + ESI);

    EDI+=inc;
    ESI+=inc;
    cpu->dst.u32 = v2;
    cpu->src.u32 = v1;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
}
void cmpsd32r(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = ECX;
    if (count) {
        U32 v1=0;
        U32 v2=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+EDI);
            v2 = cpu->memory->readd(sBase+ESI);
            EDI+=inc;
            ESI+=inc;
            ECX--;
            if ((v1==v2)!=rep_zero) break;
        }
        cpu->dst.u32 = v2;
        cpu->src.u32 = v1;
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
        cpu->lazyFlags = FLAGS_SUB32;
    }
}
void stosb16(CPU* cpu) {
    cpu->memory->writeb(cpu->seg[ES].address+DI, AL);
    DI += cpu->getDirection();
}
void stosb16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writeb(dBase+DI, AL);
        DI+=inc;
        CX--;
    }
}
void stosb32(CPU* cpu) {
    cpu->memory->writeb(cpu->seg[ES].address+EDI, AL);
    EDI += cpu->getDirection();
}
void stosb32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writeb(dBase+EDI, AL);
        EDI+=inc;
        ECX--;
    }
}
void stosw16(CPU* cpu) {
    cpu->memory->writew(cpu->seg[ES].address+DI, AX);
    DI += cpu->getDirection() << 1;
}
void stosw16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writew(dBase+DI, AX);
        DI+=inc;
        CX--;
    }
}
void stosw32(CPU* cpu) {
    cpu->memory->writew(cpu->seg[ES].address+EDI, AX);
    EDI += cpu->getDirection() << 1;
}
void stosw32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writew(dBase+EDI, AX);
        EDI+=inc;
        ECX--;
    }
}
void stosd16(CPU* cpu) {
    cpu->memory->writed(cpu->seg[ES].address+DI, EAX);
    DI += cpu->getDirection() << 2;
}
void stosd16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writed(dBase+DI, EAX);
        DI+=inc;
        CX--;
    }
}
void stosd32(CPU* cpu) {
    cpu->memory->writed(cpu->seg[ES].address+EDI, EAX);
    EDI += cpu->getDirection() << 2;
}
void stosd32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        cpu->memory->writed(dBase+EDI, EAX);
        EDI+=inc;
        ECX--;
    }
}
void lodsb16(CPU* cpu, U32 base) {
    AL = cpu->memory->readb(cpu->seg[base].address+SI);
    SI += cpu->getDirection();
}
void lodsb16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        AL = cpu->memory->readb(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsb32(CPU* cpu, U32 base) {
    AL = cpu->memory->readb(cpu->seg[base].address+ESI);
    ESI += cpu->getDirection();
}
void lodsb32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection();
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        AL = cpu->memory->readb(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void lodsw16(CPU* cpu, U32 base) {
    AX = cpu->memory->readw(cpu->seg[base].address+SI);
    SI += cpu->getDirection() << 1;
}
void lodsw16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        AX = cpu->memory->readw(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsw32(CPU* cpu, U32 base) {
    AX = cpu->memory->readw(cpu->seg[base].address+ESI);
    ESI += cpu->getDirection() << 1;
}
void lodsw32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        AX = cpu->memory->readw(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void lodsd16(CPU* cpu, U32 base) {
    EAX = cpu->memory->readd(cpu->seg[base].address+SI);
    SI += cpu->getDirection() << 2;
}
void lodsd16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    
    for (U32 i=0;i<count;i++) {
        EAX = cpu->memory->readd(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsd32(CPU* cpu, U32 base) {
    EAX = cpu->memory->readd(cpu->seg[base].address+ESI);
    ESI += cpu->getDirection() << 2;
}
void lodsd32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = ECX;
    
    for (U32 i=0;i<count;i++) {
        EAX = cpu->memory->readd(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void scasb16(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase+DI);
    DI+=inc;
    cpu->dst.u8 = AL;
    cpu->src.u8 = v1;
    cpu->result.u8 = AL - v1;
    cpu->lazyFlags = FLAGS_SUB8;
}
void scasb16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U32 count = CX;
    if (count) {
        U8 v1 = 0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+DI);
            DI+=inc;
            CX--;
            if ((AL==v1)!=rep_zero) break;
        }
        cpu->dst.u8 = AL;
        cpu->src.u8 = v1;
        cpu->result.u8 = AL - v1;
        cpu->lazyFlags = FLAGS_SUB8;
    }
}
void scasb32(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U8 v1 = cpu->memory->readb(dBase+EDI);
    EDI+=inc;
    cpu->dst.u8 = AL;
    cpu->src.u8 = v1;
    cpu->result.u8 = AL - v1;
    cpu->lazyFlags = FLAGS_SUB8;
}
void scasb32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection();
    U32 count = ECX;
    if (count) {
        U8 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readb(dBase+EDI);
            EDI+=inc;
            ECX--;
            if ((AL==v1)!=rep_zero) break;
        }
        cpu->dst.u8 = AL;
        cpu->src.u8 = v1;
        cpu->result.u8 = AL - v1;
        cpu->lazyFlags = FLAGS_SUB8;
    }
}
void scasw16(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase+DI);
    DI+=inc;
    cpu->dst.u16 = AX;
    cpu->src.u16 = v1;
    cpu->result.u16 = AX - v1;
    cpu->lazyFlags = FLAGS_SUB16;
}
void scasw16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = CX;
    if (count) {
        U16 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+DI);
            DI+=inc;
            CX--;
            if ((AX==v1)!=rep_zero) break;
        }
        cpu->dst.u16 = AX;
        cpu->src.u16 = v1;
        cpu->result.u16 = AX - v1;
        cpu->lazyFlags = FLAGS_SUB16;
    }
}
void scasw32(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U16 v1 = cpu->memory->readw(dBase+EDI);
    EDI+=inc;
    cpu->dst.u16 = AX;
    cpu->src.u16 = v1;
    cpu->result.u16 = AX - v1;
    cpu->lazyFlags = FLAGS_SUB16;
}
void scasw32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 1;
    U32 count = ECX;
    if (count) {
        U16 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readw(dBase+EDI);
            EDI+=inc;
            ECX--;
            if ((AX==v1)!=rep_zero) break;
        }
        cpu->dst.u16 = AX;
        cpu->src.u16 = v1;
        cpu->result.u16 = AX - v1;
        cpu->lazyFlags = FLAGS_SUB16;
    }
}
void scasd16(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase+DI);
    DI+=inc;
    cpu->dst.u32 = EAX;
    cpu->src.u32 = v1;
    cpu->result.u32 = EAX - v1;
    cpu->lazyFlags = FLAGS_SUB32;
}
void scasd16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = CX;
    if (count) {
        U32 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+DI);
            DI+=inc;
            CX--;
            if ((EAX==v1)!=rep_zero) break;
        }
        cpu->dst.u32 = EAX;
        cpu->src.u32 = v1;
        cpu->result.u32 = EAX - v1;
        cpu->lazyFlags = FLAGS_SUB32;
    }
}
void scasd32(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 v1 = cpu->memory->readd(dBase+EDI);
    EDI+=inc;
    cpu->dst.u32 = EAX;
    cpu->src.u32 = v1;
    cpu->result.u32 = EAX - v1;
    cpu->lazyFlags = FLAGS_SUB32;
}
void scasd32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->getDirection() << 2;
    U32 count = ECX;
    if (count) {
        U32 v1=0;
        for (U32 i=0;i<count;i++) {
            v1 = cpu->memory->readd(dBase+EDI);
            EDI+=inc;
            ECX--;
            if ((EAX==v1)!=rep_zero) break;
        }
        cpu->dst.u32 = EAX;
        cpu->src.u32 = v1;
        cpu->result.u32 = EAX - v1;
        cpu->lazyFlags = FLAGS_SUB32;
    }
}
