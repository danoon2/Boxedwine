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
void DynamicData::dynamic_movsb_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_movsw_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_movsd_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsb16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction((void*)cmpsb16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsb32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction((void*)cmpsb32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB8;
        }
    }    
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsw16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction((void*)cmpsw16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsw32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction((void*)cmpsw32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB16;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsd16r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction((void*)cmpsd16, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)cmpsd32r, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction((void*)cmpsd32, false, 3, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false, op->base, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB32;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_stosb_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_stosw_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_stosd_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_lodsb_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_lodsw_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_lodsd_op(DecodedOp* op) {
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
    incrementEip(op->len);
}
void DynamicData::dynamic_scasb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasb16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction((void*)scasb16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasb32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            callHostFunction((void*)scasb32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB8;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_scasw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasw16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction((void*)scasw16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasw32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            callHostFunction((void*)scasw32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB16;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_scasd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasd16r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction((void*)scasd16, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            callHostFunction((void*)scasd32r, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            callHostFunction((void*)scasd32, false, 2, 0, DYN_PARAM_CPU, false, op->repZero, DYN_PARAM_CONST_32, false);
            currentLazyFlags=FLAGS_SUB32;
        }
    }
    incrementEip(op->len);
}
