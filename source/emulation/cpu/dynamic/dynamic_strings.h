/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#include "../normal/normal_strings.h"
void dynamic_movsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)movsb16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)movsb16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)movsb32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)movsb32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_movsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)movsw16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)movsw16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)movsw32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)movsw32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_movsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)movsd16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)movsd16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)movsd32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)movsd32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)cmpsb16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)cmpsb16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)cmpsb32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)cmpsb32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    }    
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)cmpsw16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)cmpsw16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)cmpsw32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)cmpsw32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_cmpsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)cmpsd16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)cmpsd16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)cmpsd32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)cmpsd32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)stosb16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(data, (void*)stosb16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)stosb32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(data, (void*)stosb32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)stosw16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(data, (void*)stosw16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)stosw32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(data, (void*)stosw32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_stosd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)stosd16r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(data, (void*)stosd16, false, 1, 0, DYN_PARAM_CPU, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)stosd32r, false, 1, 0, DYN_PARAM_CPU, false);
        } else { 
            callHostFunction(data, (void*)stosd32, false, 1, 0, DYN_PARAM_CPU, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)lodsb16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)lodsb16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)lodsb32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)lodsb32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)lodsw16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)lodsw16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)lodsw32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)lodsw32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_lodsd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)lodsd16r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)lodsd16, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)lodsd32r, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        } else { 
            callHostFunction(data, (void*)lodsd32, false, 2, 0, DYN_PARAM_CPU, false, op->base, DYN_PARAM_CONST_32, false);
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasb_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)scasb16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)scasb16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)scasb32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)scasb32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB8;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasw_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)scasw16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)scasw16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)scasw32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)scasw32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB16;
        }
    }
    INCREMENT_EIP(data, op);
}
void dynamic_scasd_op(DynamicData* data, DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)scasd16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)scasd16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction(data, (void*)scasd32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction(data, (void*)scasd32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            data->currentLazyFlags=FLAGS_SUB32;
        }
    }
    INCREMENT_EIP(data, op);
}
