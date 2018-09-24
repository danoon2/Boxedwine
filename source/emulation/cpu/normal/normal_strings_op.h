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

void OPCALL normal_movsb_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            movsb16r(cpu, op->base);
        } else { 
            movsb16(cpu, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            movsb32r(cpu, op->base);
        } else { 
            movsb32(cpu, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_movsw_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            movsw16r(cpu, op->base);
        } else { 
            movsw16(cpu, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            movsw32r(cpu, op->base);
        } else { 
            movsw32(cpu, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_movsd_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            movsd16r(cpu, op->base);
        } else { 
            movsd16(cpu, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            movsd32r(cpu, op->base);
        } else { 
            movsd32(cpu, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_cmpsb_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            cmpsb16r(cpu, op->repZero, op->base);
        } else { 
            cmpsb16(cpu, op->repZero, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            cmpsb32r(cpu, op->repZero, op->base);
        } else { 
            cmpsb32(cpu, op->repZero, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_cmpsw_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            cmpsw16r(cpu, op->repZero, op->base);
        } else { 
            cmpsw16(cpu, op->repZero, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            cmpsw32r(cpu, op->repZero, op->base);
        } else { 
            cmpsw32(cpu, op->repZero, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_cmpsd_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            cmpsd16r(cpu, op->repZero, op->base);
        } else { 
            cmpsd16(cpu, op->repZero, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            cmpsd32r(cpu, op->repZero, op->base);
        } else { 
            cmpsd32(cpu, op->repZero, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_stosb_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            stosb16r(cpu);
        } else { 
            stosb16(cpu);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            stosb32r(cpu);
        } else { 
            stosb32(cpu);
        }
    }
    NEXT();
}
void OPCALL normal_stosw_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            stosw16r(cpu);
        } else { 
            stosw16(cpu);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            stosw32r(cpu);
        } else { 
            stosw32(cpu);
        }
    }
    NEXT();
}
void OPCALL normal_stosd_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            stosd16r(cpu);
        } else { 
            stosd16(cpu);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            stosd32r(cpu);
        } else { 
            stosd32(cpu);
        }
    }
    NEXT();
}
void OPCALL normal_lodsb_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            lodsb16r(cpu, op->base);
        } else { 
            lodsb16(cpu, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            lodsb32r(cpu, op->base);
        } else { 
            lodsb32(cpu, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_lodsw_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            lodsw16r(cpu, op->base);
        } else { 
            lodsw16(cpu, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            lodsw32r(cpu, op->base);
        } else { 
            lodsw32(cpu, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_lodsd_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            lodsd16r(cpu, op->base);
        } else { 
            lodsd16(cpu, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            lodsd32r(cpu, op->base);
        } else { 
            lodsd32(cpu, op->base);
        }
    }
    NEXT();
}
void OPCALL normal_scasb_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            scasb16r(cpu, op->repZero);
        } else { 
            scasb16(cpu, op->repZero);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            scasb32r(cpu, op->repZero);
        } else { 
            scasb32(cpu, op->repZero);
        }
    }
    NEXT();
}
void OPCALL normal_scasw_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            scasw16r(cpu, op->repZero);
        } else { 
            scasw16(cpu, op->repZero);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            scasw32r(cpu, op->repZero);
        } else { 
            scasw32(cpu, op->repZero);
        }
    }
    NEXT();
}
void OPCALL normal_scasd_op(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            scasd16r(cpu, op->repZero);
        } else { 
            scasd16(cpu, op->repZero);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            scasd32r(cpu, op->repZero);
        } else { 
            scasd32(cpu, op->repZero);
        }
    }
    NEXT();
}
