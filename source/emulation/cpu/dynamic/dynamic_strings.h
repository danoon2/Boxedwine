#include "../normal/normal_strings.h"
void OPCALL dynamic_movsb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsb16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(movsb16, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsb32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(movsb32, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movsw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsw16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(movsw16, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsw32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(movsw32, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movsd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsd16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(movsd16, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsd32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(movsd32, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpsb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsb16r, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(cmpsb16, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsb32r, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(cmpsb32, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpsw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsw16r, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(cmpsw16, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsw32r, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(cmpsw32, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpsd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsd16r, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(cmpsd16, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsd32r, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(cmpsd32, false, false, false, 3, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stosb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosb16r, false, false, false, 1, 0, DYN_PARAM_CPU);
        } else { 
            callHostFunction(stosb16, false, false, false, 1, 0, DYN_PARAM_CPU);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosb32r, false, false, false, 1, 0, DYN_PARAM_CPU);
        } else { 
            callHostFunction(stosb32, false, false, false, 1, 0, DYN_PARAM_CPU);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stosw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosw16r, false, false, false, 1, 0, DYN_PARAM_CPU);
        } else { 
            callHostFunction(stosw16, false, false, false, 1, 0, DYN_PARAM_CPU);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosw32r, false, false, false, 1, 0, DYN_PARAM_CPU);
        } else { 
            callHostFunction(stosw32, false, false, false, 1, 0, DYN_PARAM_CPU);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stosd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosd16r, false, false, false, 1, 0, DYN_PARAM_CPU);
        } else { 
            callHostFunction(stosd16, false, false, false, 1, 0, DYN_PARAM_CPU);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosd32r, false, false, false, 1, 0, DYN_PARAM_CPU);
        } else { 
            callHostFunction(stosd32, false, false, false, 1, 0, DYN_PARAM_CPU);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lodsb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsb16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(lodsb16, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsb32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(lodsb32, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lodsw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsw16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(lodsw16, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsw32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(lodsw32, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lodsd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsd16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(lodsd16, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsd32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(lodsd32, false, false, false, 2, 0, DYN_PARAM_CPU, op->base, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_scasb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasb16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(scasb16, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasb32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(scasb32, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_scasw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasw16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(scasw16, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasw32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(scasw32, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_scasd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasd16r, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(scasd16, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasd32r, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        } else { 
            callHostFunction(scasd32, false, false, false, 2, 0, DYN_PARAM_CPU, op->repZero, DYN_PARAM_CONST_32);
        }
    }
    INCREMENT_EIP(op->len);
}
