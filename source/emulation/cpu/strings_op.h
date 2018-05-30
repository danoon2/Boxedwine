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

#include "strings.h"
void OPCALL movsb32_r_op(struct CPU* cpu, struct Op* op) {
    movsb32_r(cpu, op->base);
    NEXT();
}
void OPCALL movsb16_r_op(struct CPU* cpu, struct Op* op) {
    movsb16_r(cpu, op->base);
    NEXT();
}
void OPCALL movsb32_op(struct CPU* cpu, struct Op* op) {
    movsb32(cpu, op->base);
    NEXT();
}
void OPCALL movsb16_op(struct CPU* cpu, struct Op* op) {
    movsb16(cpu, op->base);
    NEXT();
}
void OPCALL movsw32_r_op(struct CPU* cpu, struct Op* op) {
    movsw32_r(cpu, op->base);
    NEXT();
}
void OPCALL movsw16_r_op(struct CPU* cpu, struct Op* op) {
    movsw16_r(cpu, op->base);
    NEXT();
}
void OPCALL movsw32_op(struct CPU* cpu, struct Op* op) {
    movsw32(cpu, op->base);
    NEXT();
}
void OPCALL movsw16_op(struct CPU* cpu, struct Op* op) {
    movsw16(cpu, op->base);
    NEXT();
}
void OPCALL movsd32_r_op(struct CPU* cpu, struct Op* op) {
    movsd32_r(cpu, op->base);
    NEXT();
}
void OPCALL movsd16_r_op(struct CPU* cpu, struct Op* op) {
    movsd16_r(cpu, op->base);
    NEXT();
}
void OPCALL movsd32_op(struct CPU* cpu, struct Op* op) {
    movsd32(cpu, op->base);
    NEXT();
}
void OPCALL movsd16_op(struct CPU* cpu, struct Op* op) {
    movsd16(cpu, op->base);
    NEXT();
}
void OPCALL cmpsb32_r_op(struct CPU* cpu, struct Op* op) {
    cmpsb32_r(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsb16_r_op(struct CPU* cpu, struct Op* op) {
    cmpsb16_r(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsb32_op(struct CPU* cpu, struct Op* op) {
    cmpsb32(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsb16_op(struct CPU* cpu, struct Op* op) {
    cmpsb16(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsw32_r_op(struct CPU* cpu, struct Op* op) {
    cmpsw32_r(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsw16_r_op(struct CPU* cpu, struct Op* op) {
    cmpsw16_r(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsw32_op(struct CPU* cpu, struct Op* op) {
    cmpsw32(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsw16_op(struct CPU* cpu, struct Op* op) {
    cmpsw16(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsd32_r_op(struct CPU* cpu, struct Op* op) {
    cmpsd32_r(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsd16_r_op(struct CPU* cpu, struct Op* op) {
    cmpsd16_r(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsd32_op(struct CPU* cpu, struct Op* op) {
    cmpsd32(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL cmpsd16_op(struct CPU* cpu, struct Op* op) {
    cmpsd16(cpu, op->data1, op->base);
    NEXT();
}
void OPCALL stosb32_r_op(struct CPU* cpu, struct Op* op) {
    stosb32_r(cpu);
    NEXT();
}
void OPCALL stosb16_r_op(struct CPU* cpu, struct Op* op) {
    stosb16_r(cpu);
    NEXT();
}
void OPCALL stosb32_op(struct CPU* cpu, struct Op* op) {
    stosb32(cpu);
    NEXT();
}
void OPCALL stosb16_op(struct CPU* cpu, struct Op* op) {
    stosb16(cpu);
    NEXT();
}
void OPCALL stosw32_r_op(struct CPU* cpu, struct Op* op) {
    stosw32_r(cpu);
    NEXT();
}
void OPCALL stosw16_r_op(struct CPU* cpu, struct Op* op) {
    stosw16_r(cpu);
    NEXT();
}
void OPCALL stosw32_op(struct CPU* cpu, struct Op* op) {
    stosw32(cpu);
    NEXT();
}
void OPCALL stosw16_op(struct CPU* cpu, struct Op* op) {
    stosw16(cpu);
    NEXT();
}
void OPCALL stosd32_r_op(struct CPU* cpu, struct Op* op) {
    stosd32_r(cpu);
    NEXT();
}
void OPCALL stosd16_r_op(struct CPU* cpu, struct Op* op) {
    stosd16_r(cpu);
    NEXT();
}
void OPCALL stosd32_op(struct CPU* cpu, struct Op* op) {
    stosd32(cpu);
    NEXT();
}
void OPCALL stosd16_op(struct CPU* cpu, struct Op* op) {
    stosd16(cpu);
    NEXT();
}
void OPCALL lodsb32_r_op(struct CPU* cpu, struct Op* op) {
    lodsb32_r(cpu, op->base);
    NEXT();
}
void OPCALL lodsb16_r_op(struct CPU* cpu, struct Op* op) {
    lodsb16_r(cpu, op->base);
    NEXT();
}
void OPCALL lodsb32_op(struct CPU* cpu, struct Op* op) {
    lodsb32(cpu, op->base);
    NEXT();
}
void OPCALL lodsb16_op(struct CPU* cpu, struct Op* op) {
    lodsb16(cpu, op->base);
    NEXT();
}
void OPCALL lodsw32_r_op(struct CPU* cpu, struct Op* op) {
    lodsw32_r(cpu, op->base);
    NEXT();
}
void OPCALL lodsw16_r_op(struct CPU* cpu, struct Op* op) {
    lodsw16_r(cpu, op->base);
    NEXT();
}
void OPCALL lodsw32_op(struct CPU* cpu, struct Op* op) {
    lodsw32(cpu, op->base);
    NEXT();
}
void OPCALL lodsw16_op(struct CPU* cpu, struct Op* op) {
    lodsw16(cpu, op->base);
    NEXT();
}
void OPCALL lodsd32_r_op(struct CPU* cpu, struct Op* op) {
    lodsd32_r(cpu, op->base);
    NEXT();
}
void OPCALL lodsd16_r_op(struct CPU* cpu, struct Op* op) {
    lodsd16_r(cpu, op->base);
    NEXT();
}
void OPCALL lodsd32_op(struct CPU* cpu, struct Op* op) {
    lodsd32(cpu, op->base);
    NEXT();
}
void OPCALL lodsd16_op(struct CPU* cpu, struct Op* op) {
    lodsd16(cpu, op->base);
    NEXT();
}
void OPCALL scasb32_r_op(struct CPU* cpu, struct Op* op) {
    scasb32_r(cpu, op->data1);
    NEXT();
}
void OPCALL scasb16_r_op(struct CPU* cpu, struct Op* op) {
    scasb16_r(cpu, op->data1);
    NEXT();
}
void OPCALL scasb32_op(struct CPU* cpu, struct Op* op) {
    scasb32(cpu, op->data1);
    NEXT();
}
void OPCALL scasb16_op(struct CPU* cpu, struct Op* op) {
    scasb16(cpu, op->data1);
    NEXT();
}
void OPCALL scasw32_r_op(struct CPU* cpu, struct Op* op) {
    scasw32_r(cpu, op->data1);
    NEXT();
}
void OPCALL scasw16_r_op(struct CPU* cpu, struct Op* op) {
    scasw16_r(cpu, op->data1);
    NEXT();
}
void OPCALL scasw32_op(struct CPU* cpu, struct Op* op) {
    scasw32(cpu, op->data1);
    NEXT();
}
void OPCALL scasw16_op(struct CPU* cpu, struct Op* op) {
    scasw16(cpu, op->data1);
    NEXT();
}
void OPCALL scasd32_r_op(struct CPU* cpu, struct Op* op) {
    scasd32_r(cpu, op->data1);
    NEXT();
}
void OPCALL scasd16_r_op(struct CPU* cpu, struct Op* op) {
    scasd16_r(cpu, op->data1);
    NEXT();
}
void OPCALL scasd32_op(struct CPU* cpu, struct Op* op) {
    scasd32(cpu, op->data1);
    NEXT();
}
void OPCALL scasd16_op(struct CPU* cpu, struct Op* op) {
    scasd16(cpu, op->data1);
    NEXT();
}
