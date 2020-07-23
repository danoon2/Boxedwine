#include "../normal/normal_strings.h"
void dynamic_movsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)movsb16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)movsb16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)movsb32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)movsb32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_movsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)movsw16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)movsw16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)movsw32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)movsw32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_movsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)movsd16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)movsd16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)movsd32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)movsd32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsb16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)cmpsb16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsb32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)cmpsb32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsw16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)cmpsw16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsw32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)cmpsw32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsd16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)cmpsd16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsd32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)cmpsd32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)stosb16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction((void*)stosb16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)stosb32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction((void*)stosb32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)stosw16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction((void*)stosw16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)stosw32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction((void*)stosw32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)stosd16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction((void*)stosd16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)stosd32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction((void*)stosd32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)lodsb16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)lodsb16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)lodsb32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)lodsb32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)lodsw16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)lodsw16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)lodsw32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)lodsw32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)lodsd16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)lodsd16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)lodsd32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction((void*)lodsd32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasb16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)scasb16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasb32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB8) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)scasb32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasw16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)scasw16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasw32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB16) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)scasw32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasd16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)scasd16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasd32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            if (data->currentLazyFlags!=FLAGS_SUB32) data->currentLazyFlags=0;
        } else { 
            callHostFunction((void*)scasd32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    }
    INCREMENT_EIP(data, op);
}
