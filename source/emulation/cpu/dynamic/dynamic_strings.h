#include "../normal/normal_strings.h"
void dynamic_movsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsb16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(movsb16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsb32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(movsb32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_movsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsw16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(movsw16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsw32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(movsw32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_movsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsd16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(movsd16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(movsd32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(movsd32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_cmpsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsb16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(cmpsb16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsb32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(cmpsb32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_cmpsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsw16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(cmpsw16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsw32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(cmpsw32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_cmpsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsd16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(cmpsd16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(cmpsd32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(cmpsd32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_stosb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosb16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(stosb16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosb32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(stosb32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_stosw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosw16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(stosw16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosw32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(stosw32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_stosd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosd16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(stosd16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(stosd32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(stosd32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_lodsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsb16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(lodsb16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsb32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(lodsb32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_lodsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsw16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(lodsw16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsw32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(lodsw32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_lodsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsd16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(lodsd16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(lodsd32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(lodsd32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_scasb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasb16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(scasb16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasb32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(scasb32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_scasw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasw16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(scasw16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasw32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(scasw32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
void dynamic_scasd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasd16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(scasd16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(scasd32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(scasd32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(op->len);
}
