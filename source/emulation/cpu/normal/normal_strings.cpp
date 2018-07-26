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
    S32 inc = cpu->df;
        writeb(dBase+DI, readb(sBase+SI));
        DI+=inc;
        SI+=inc;
}
void movsb16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        writeb(dBase+DI, readb(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsb32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df;
        writeb(dBase+EDI, readb(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
}
void movsb32r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        writeb(dBase+EDI, readb(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
        ECX--;
    }
}
void movsw16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 1;
        writew(dBase+DI, readw(sBase+SI));
        DI+=inc;
        SI+=inc;
}
void movsw16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 1;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        writew(dBase+DI, readw(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsw32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 1;
        writew(dBase+EDI, readw(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
}
void movsw32r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 1;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        writew(dBase+EDI, readw(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
        ECX--;
    }
}
void movsd16(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 2;
        writed(dBase+DI, readd(sBase+SI));
        DI+=inc;
        SI+=inc;
}
void movsd16r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 2;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        writed(dBase+DI, readd(sBase+SI));
        DI+=inc;
        SI+=inc;
        CX--;
    }
}
void movsd32(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 2;
        writed(dBase+EDI, readd(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
}
void movsd32r(CPU* cpu, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 2;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        writed(dBase+EDI, readd(sBase+ESI));
        EDI+=inc;
        ESI+=inc;
        ECX--;
    }
}
void cmpsb16(CPU* cpu, U32 rep_zero, U32 base) {
    U32 dBase = cpu->seg[ES].address;
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df;
    U8 v1;
    U8 v2;
            v1 = readb(dBase+DI);
            v2 = readb(sBase+SI);
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
    S32 inc = cpu->df;
    U8 v1;
    U8 v2;
    U32 count = CX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readb(dBase+DI);
            v2 = readb(sBase+SI);
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
    S32 inc = cpu->df;
    U8 v1;
    U8 v2;
            v1 = readb(dBase+EDI);
            v2 = readb(sBase+ESI);
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
    S32 inc = cpu->df;
    U8 v1;
    U8 v2;
    U32 count = ECX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readb(dBase+EDI);
            v2 = readb(sBase+ESI);
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
    S32 inc = cpu->df << 1;
    U16 v1;
    U16 v2;
            v1 = readw(dBase+DI);
            v2 = readw(sBase+SI);
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
    S32 inc = cpu->df << 1;
    U16 v1;
    U16 v2;
    U32 count = CX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readw(dBase+DI);
            v2 = readw(sBase+SI);
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
    S32 inc = cpu->df << 1;
    U16 v1;
    U16 v2;
            v1 = readw(dBase+EDI);
            v2 = readw(sBase+ESI);
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
    S32 inc = cpu->df << 1;
    U16 v1;
    U16 v2;
    U32 count = ECX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readw(dBase+EDI);
            v2 = readw(sBase+ESI);
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
    S32 inc = cpu->df << 2;
    U32 v1;
    U32 v2;
            v1 = readd(dBase+DI);
            v2 = readd(sBase+SI);
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
    S32 inc = cpu->df << 2;
    U32 v1;
    U32 v2;
    U32 count = CX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readd(dBase+DI);
            v2 = readd(sBase+SI);
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
    S32 inc = cpu->df << 2;
    U32 v1;
    U32 v2;
            v1 = readd(dBase+EDI);
            v2 = readd(sBase+ESI);
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
    S32 inc = cpu->df << 2;
    U32 v1;
    U32 v2;
    U32 count = ECX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readd(dBase+EDI);
            v2 = readd(sBase+ESI);
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
    writeb(cpu->seg[ES].address+DI, AL);
    DI+=cpu->df;
}
void stosb16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        writeb(dBase+DI, AL);
        DI+=inc;
        CX--;
    }
}
void stosb32(CPU* cpu) {
    writeb(cpu->seg[ES].address+EDI, AL);
    EDI+=cpu->df;
}
void stosb32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        writeb(dBase+EDI, AL);
        EDI+=inc;
        ECX--;
    }
}
void stosw16(CPU* cpu) {
    writew(cpu->seg[ES].address+DI, AX);
    DI+=cpu->df << 1;
}
void stosw16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 1;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        writew(dBase+DI, AX);
        DI+=inc;
        CX--;
    }
}
void stosw32(CPU* cpu) {
    writew(cpu->seg[ES].address+EDI, AX);
    EDI+=cpu->df << 1;
}
void stosw32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 1;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        writew(dBase+EDI, AX);
        EDI+=inc;
        ECX--;
    }
}
void stosd16(CPU* cpu) {
    writed(cpu->seg[ES].address+DI, EAX);
    DI+=cpu->df << 2;
}
void stosd16r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 2;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        writed(dBase+DI, EAX);
        DI+=inc;
        CX--;
    }
}
void stosd32(CPU* cpu) {
    writed(cpu->seg[ES].address+EDI, EAX);
    EDI+=cpu->df << 2;
}
void stosd32r(CPU* cpu) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 2;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        writed(dBase+EDI, EAX);
        EDI+=inc;
        ECX--;
    }
}
void lodsb16(CPU* cpu, U32 base) {
    AL = readb(cpu->seg[base].address+SI);
    SI+=cpu->df;
}
void lodsb16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        AL = readb(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsb32(CPU* cpu, U32 base) {
    AL = readb(cpu->seg[base].address+ESI);
    ESI+=cpu->df;
}
void lodsb32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        AL = readb(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void lodsw16(CPU* cpu, U32 base) {
    AX = readw(cpu->seg[base].address+SI);
    SI+=cpu->df << 1;
}
void lodsw16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 1;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        AX = readw(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsw32(CPU* cpu, U32 base) {
    AX = readw(cpu->seg[base].address+ESI);
    ESI+=cpu->df << 1;
}
void lodsw32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 1;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        AX = readw(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void lodsd16(CPU* cpu, U32 base) {
    EAX = readd(cpu->seg[base].address+SI);
    SI+=cpu->df << 2;
}
void lodsd16r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 2;
    U32 count = CX;
    U32 i;
    for (i=0;i<count;i++) {
        EAX = readd(sBase+SI);
        SI+=inc;
        CX--;
    }
}
void lodsd32(CPU* cpu, U32 base) {
    EAX = readd(cpu->seg[base].address+ESI);
    ESI+=cpu->df << 2;
}
void lodsd32r(CPU* cpu, U32 base) {
    U32 sBase = cpu->seg[base].address;
    S32 inc = cpu->df << 2;
    U32 count = ECX;
    U32 i;
    for (i=0;i<count;i++) {
        EAX = readd(sBase+ESI);
        ESI+=inc;
        ECX--;
    }
}
void scasb16(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df;
    U8 v1;
            v1 = readb(dBase+DI);
            DI+=inc;
        cpu->dst.u8 = AL;
        cpu->src.u8 = v1;
        cpu->result.u8 = AL - v1;
        cpu->lazyFlags = FLAGS_SUB8;
}
void scasb16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df;
    U8 v1;
    U32 count = CX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readb(dBase+DI);
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
    S32 inc = cpu->df;
    U8 v1;
            v1 = readb(dBase+EDI);
            EDI+=inc;
        cpu->dst.u8 = AL;
        cpu->src.u8 = v1;
        cpu->result.u8 = AL - v1;
        cpu->lazyFlags = FLAGS_SUB8;
}
void scasb32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df;
    U8 v1;
    U32 count = ECX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readb(dBase+EDI);
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
    S32 inc = cpu->df << 1;
    U16 v1;
            v1 = readw(dBase+DI);
            DI+=inc;
        cpu->dst.u16 = AX;
        cpu->src.u16 = v1;
        cpu->result.u16 = AX - v1;
        cpu->lazyFlags = FLAGS_SUB16;
}
void scasw16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 1;
    U16 v1;
    U32 count = CX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readw(dBase+DI);
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
    S32 inc = cpu->df << 1;
    U16 v1;
            v1 = readw(dBase+EDI);
            EDI+=inc;
        cpu->dst.u16 = AX;
        cpu->src.u16 = v1;
        cpu->result.u16 = AX - v1;
        cpu->lazyFlags = FLAGS_SUB16;
}
void scasw32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 1;
    U16 v1;
    U32 count = ECX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readw(dBase+EDI);
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
    S32 inc = cpu->df << 2;
    U32 v1;
            v1 = readd(dBase+DI);
            DI+=inc;
        cpu->dst.u32 = EAX;
        cpu->src.u32 = v1;
        cpu->result.u32 = EAX - v1;
        cpu->lazyFlags = FLAGS_SUB32;
}
void scasd16r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 2;
    U32 v1;
    U32 count = CX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readd(dBase+DI);
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
    S32 inc = cpu->df << 2;
    U32 v1;
            v1 = readd(dBase+EDI);
            EDI+=inc;
        cpu->dst.u32 = EAX;
        cpu->src.u32 = v1;
        cpu->result.u32 = EAX - v1;
        cpu->lazyFlags = FLAGS_SUB32;
}
void scasd32r(CPU* cpu, U32 rep_zero) {
    U32 dBase = cpu->seg[ES].address;
    S32 inc = cpu->df << 2;
    U32 v1;
    U32 count = ECX;
    U32 i;
    if (count) {
        for (i=0;i<count;i++) {
            v1 = readd(dBase+EDI);
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
