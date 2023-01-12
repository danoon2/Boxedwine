#include "../normal/normal_strings.h"
void dynamic_movsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(movsb16r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(movsb16), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(movsb32r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(movsb32), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_movsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(movsw16r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(movsw16), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(movsw32r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(movsw32), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_movsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(movsd16r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(movsd16), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(movsd32r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(movsd32), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(cmpsb16r), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(cmpsb16), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(cmpsb32r), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(cmpsb32), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(cmpsw16r), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(cmpsw16), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(cmpsw32r), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(cmpsw32), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(cmpsd16r), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(cmpsd16), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(cmpsd32r), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(cmpsd32), false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(stosb16r), false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(DYN_HOST_FN(stosb16), false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(stosb32r), false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(DYN_HOST_FN(stosb32), false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(stosw16r), false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(DYN_HOST_FN(stosw16), false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(stosw32r), false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(DYN_HOST_FN(stosw32), false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(stosd16r), false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(DYN_HOST_FN(stosd16), false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(stosd32r), false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(DYN_HOST_FN(stosd32), false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(lodsb16r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(lodsb16), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(lodsb32r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(lodsb32), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(lodsw16r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(lodsw16), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(lodsw32r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(lodsw32), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(lodsd16r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(lodsd16), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(lodsd32r), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(DYN_HOST_FN(lodsd32), false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(scasb16r), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(scasb16), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(scasb32r), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(scasb32), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(scasw16r), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(scasw16), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(scasw32r), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(scasw32), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(scasd16r), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(scasd16), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(DYN_HOST_FN(scasd32r), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction(DYN_HOST_FN(scasd32), false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    }
    INCREMENT_EIP(data, op);
}
