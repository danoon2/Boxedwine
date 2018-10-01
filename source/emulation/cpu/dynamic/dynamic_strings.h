#include "../normal/normal_strings.h"
void OPCALL dynamic_movsb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, movsb16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, movsb16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, movsb32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, movsb32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movsw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, movsw16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, movsw16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, movsw32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, movsw32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movsd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, movsd16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, movsd16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, movsd32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, movsd32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpsb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, cmpsb16r, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, cmpsb16, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, cmpsb32r, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, cmpsb32, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpsw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, cmpsw16r, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, cmpsw16, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, cmpsw32r, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, cmpsw32, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpsd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, cmpsd16r, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, cmpsd16, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, cmpsd32r, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, cmpsd32, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stosb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, stosb16r, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(NULL, stosb16, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, stosb32r, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(NULL, stosb32, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stosw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, stosw16r, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(NULL, stosw16, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, stosw32r, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(NULL, stosw32, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stosd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, stosd16r, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(NULL, stosd16, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, stosd32r, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(NULL, stosd32, false, false, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lodsb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, lodsb16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, lodsb16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, lodsb32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, lodsb32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lodsw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, lodsw16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, lodsw16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, lodsw32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, lodsw32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lodsd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, lodsd16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, lodsd16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, lodsd32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, lodsd32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_scasb_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, scasb16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, scasb16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, scasb32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, scasb32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_scasw_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, scasw16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, scasw16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, scasw32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, scasw32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_scasd_op(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, scasd16r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, scasd16, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(NULL, scasd32r, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(NULL, scasd32, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
