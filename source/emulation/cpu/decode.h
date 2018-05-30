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

// ADD Eb,Gb
void decode000(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = addr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("ADD", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adde8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("ADD", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = adde8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("ADD", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// ADD Ew,Gw
void decode001(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = addr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("ADD", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adde16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("ADD", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = adde16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("ADD", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// ADD Ed,Gd
void decode201(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = addr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("ADD", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adde32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("ADD", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = adde32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("ADD", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// ADD Gb,Eb
void decode002(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = addr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("ADD", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = addr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("ADD", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = addr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("ADD", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// ADD Gw,Ew
void decode003(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = addr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("ADD", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = addr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("ADD", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = addr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("ADD", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// ADD Gd,Ed
void decode203(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = addr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("ADD", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = addr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("ADD", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = addr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("ADD", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// ADD Al,Ib
void decode004(struct DecodeData* data) {
    data->op->func = add8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("ADD", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// ADD Ax,Iw
void decode005(struct DecodeData* data) {
    data->op->func = add16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("ADD", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// ADD Eax,Id
void decode205(struct DecodeData* data) {
    data->op->func = add32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("ADD", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// OR Eb,Gb
void decode008(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = orr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("OR", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = ore8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("OR", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = ore8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("OR", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// OR Ew,Gw
void decode009(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = orr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("OR", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = ore16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("OR", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = ore16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("OR", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// OR Ed,Gd
void decode209(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = orr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("OR", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = ore32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("OR", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = ore32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("OR", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// OR Gb,Eb
void decode00a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = orr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("OR", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = orr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("OR", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = orr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("OR", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// OR Gw,Ew
void decode00b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = orr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("OR", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = orr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("OR", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = orr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("OR", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// OR Gd,Ed
void decode20b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = orr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("OR", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = orr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("OR", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = orr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("OR", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// OR Al,Ib
void decode00c(struct DecodeData* data) {
    data->op->func = or8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("OR", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// OR Ax,Iw
void decode00d(struct DecodeData* data) {
    data->op->func = or16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("OR", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// OR Eax,Id
void decode20d(struct DecodeData* data) {
    data->op->func = or32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("OR", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// ADC Eb,Gb
void decode010(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = adcr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("ADC", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adce8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("ADC", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = adce8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("ADC", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// ADC Ew,Gw
void decode011(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = adcr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("ADC", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adce16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("ADC", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = adce16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("ADC", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// ADC Ed,Gd
void decode211(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = adcr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("ADC", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adce32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("ADC", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = adce32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("ADC", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// ADC Gb,Eb
void decode012(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = adcr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("ADC", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adcr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("ADC", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = adcr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("ADC", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// ADC Gw,Ew
void decode013(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = adcr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("ADC", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adcr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("ADC", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = adcr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("ADC", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// ADC Gd,Ed
void decode213(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = adcr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("ADC", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = adcr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("ADC", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = adcr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("ADC", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// ADC Al,Ib
void decode014(struct DecodeData* data) {
    data->op->func = adc8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("ADC", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// ADC Ax,Iw
void decode015(struct DecodeData* data) {
    data->op->func = adc16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("ADC", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// ADC Eax,Id
void decode215(struct DecodeData* data) {
    data->op->func = adc32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("ADC", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// SBB Eb,Gb
void decode018(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = sbbr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("SBB", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sbbe8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("SBB", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = sbbe8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("SBB", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// SBB Ew,Gw
void decode019(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = sbbr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("SBB", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sbbe16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("SBB", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = sbbe16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("SBB", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// SBB Ed,Gd
void decode219(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = sbbr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("SBB", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sbbe32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("SBB", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = sbbe32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("SBB", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// SBB Gb,Eb
void decode01a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = sbbr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("SBB", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sbbr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("SBB", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = sbbr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("SBB", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// SBB Gw,Ew
void decode01b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = sbbr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("SBB", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sbbr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("SBB", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = sbbr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("SBB", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// SBB Gd,Ed
void decode21b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = sbbr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("SBB", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sbbr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("SBB", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = sbbr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("SBB", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// SBB Al,Ib
void decode01c(struct DecodeData* data) {
    data->op->func = sbb8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("SBB", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// SBB Ax,Iw
void decode01d(struct DecodeData* data) {
    data->op->func = sbb16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("SBB", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// SBB Eax,Id
void decode21d(struct DecodeData* data) {
    data->op->func = sbb32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("SBB", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// AND Eb,Gb
void decode020(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = andr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("AND", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = ande8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("AND", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = ande8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("AND", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// AND Ew,Gw
void decode021(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = andr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("AND", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = ande16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("AND", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = ande16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("AND", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// AND Ed,Gd
void decode221(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = andr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("AND", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = ande32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("AND", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = ande32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("AND", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// AND Gb,Eb
void decode022(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = andr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("AND", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = andr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("AND", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = andr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("AND", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// AND Gw,Ew
void decode023(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = andr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("AND", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = andr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("AND", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = andr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("AND", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// AND Gd,Ed
void decode223(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = andr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("AND", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = andr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("AND", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = andr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("AND", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// AND Al,Ib
void decode024(struct DecodeData* data) {
    data->op->func = and8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("AND", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// AND Ax,Iw
void decode025(struct DecodeData* data) {
    data->op->func = and16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("AND", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// AND Eax,Id
void decode225(struct DecodeData* data) {
    data->op->func = and32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("AND", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// SUB Eb,Gb
void decode028(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = subr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("SUB", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sube8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("SUB", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = sube8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("SUB", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// SUB Ew,Gw
void decode029(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = subr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("SUB", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sube16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("SUB", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = sube16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("SUB", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// SUB Ed,Gd
void decode229(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = subr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("SUB", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = sube32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("SUB", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = sube32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("SUB", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// SUB Gb,Eb
void decode02a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = subr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("SUB", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = subr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("SUB", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = subr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("SUB", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// SUB Gw,Ew
void decode02b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = subr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("SUB", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = subr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("SUB", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = subr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("SUB", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// SUB Gd,Ed
void decode22b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = subr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("SUB", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = subr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("SUB", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = subr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("SUB", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// SUB Al,Ib
void decode02c(struct DecodeData* data) {
    data->op->func = sub8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("SUB", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// SUB Ax,Iw
void decode02d(struct DecodeData* data) {
    data->op->func = sub16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("SUB", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// SUB Eax,Id
void decode22d(struct DecodeData* data) {
    data->op->func = sub32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("SUB", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// XOR Eb,Gb
void decode030(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xorr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("XOR", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xore8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("XOR", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = xore8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("XOR", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// XOR Ew,Gw
void decode031(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xorr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("XOR", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xore16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("XOR", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = xore16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("XOR", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// XOR Ed,Gd
void decode231(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xorr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("XOR", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xore32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("XOR", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = xore32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("XOR", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// XOR Gb,Eb
void decode032(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xorr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("XOR", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xorr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("XOR", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = xorr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("XOR", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// XOR Gw,Ew
void decode033(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xorr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("XOR", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xorr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("XOR", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = xorr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("XOR", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// XOR Gd,Ed
void decode233(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xorr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("XOR", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xorr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("XOR", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = xorr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("XOR", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// XOR Al,Ib
void decode034(struct DecodeData* data) {
    data->op->func = xor8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("XOR", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// XOR Ax,Iw
void decode035(struct DecodeData* data) {
    data->op->func = xor16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("XOR", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// XOR Eax,Id
void decode235(struct DecodeData* data) {
    data->op->func = xor32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("XOR", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// CMP Eb,Gb
void decode038(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("CMP", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpe8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("CMP", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = cmpe8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("CMP", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// CMP Ew,Gw
void decode039(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("CMP", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpe16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("CMP", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = cmpe16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("CMP", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// CMP Ed,Gd
void decode239(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("CMP", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpe32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("CMP", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = cmpe32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("CMP", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// CMP Gb,Eb
void decode03a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMP", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMP", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = cmpr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMP", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMP Gw,Ew
void decode03b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMP", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMP", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = cmpr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMP", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMP Gd,Ed
void decode23b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMP", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMP", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = cmpr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMP", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMP Al,Ib
void decode03c(struct DecodeData* data) {
    data->op->func = cmp8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("CMP", R8(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// CMP Ax,Iw
void decode03d(struct DecodeData* data) {
    data->op->func = cmp16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("CMP", R16(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// CMP Eax,Id
void decode23d(struct DecodeData* data) {
    data->op->func = cmp32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("CMP", R32(0), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// INC AX
void decode040(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 0;
    LOG_OP("INC AX");
    NEXT_OP(data);
}
// INC EAX
void decode240(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 0;
    LOG_OP("INC EAX");
    NEXT_OP(data);
}
// INC CX
void decode041(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 1;
    LOG_OP("INC CX");
    NEXT_OP(data);
}
// INC ECX
void decode241(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 1;
    LOG_OP("INC ECX");
    NEXT_OP(data);
}
// INC DX
void decode042(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 2;
    LOG_OP("INC DX");
    NEXT_OP(data);
}
// INC EDX
void decode242(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 2;
    LOG_OP("INC EDX");
    NEXT_OP(data);
}
// INC BX
void decode043(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 3;
    LOG_OP("INC BX");
    NEXT_OP(data);
}
// INC EBX
void decode243(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 3;
    LOG_OP("INC EBX");
    NEXT_OP(data);
}
// INC SP
void decode044(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 4;
    LOG_OP("INC SP");
    NEXT_OP(data);
}
// INC ESP
void decode244(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 4;
    LOG_OP("INC ESP");
    NEXT_OP(data);
}
// INC BP
void decode045(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 5;
    LOG_OP("INC BP");
    NEXT_OP(data);
}
// INC EBP
void decode245(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 5;
    LOG_OP("INC EBP");
    NEXT_OP(data);
}
// INC SI
void decode046(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 6;
    LOG_OP("INC SI");
    NEXT_OP(data);
}
// INC ESI
void decode246(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 6;
    LOG_OP("INC ESI");
    NEXT_OP(data);
}
// INC DI
void decode047(struct DecodeData* data) {
    data->op->func = inc16_reg;
    data->op->r1 = 7;
    LOG_OP("INC DI");
    NEXT_OP(data);
}
// INC EDI
void decode247(struct DecodeData* data) {
    data->op->func = inc32_reg;
    data->op->r1 = 7;
    LOG_OP("INC EDI");
    NEXT_OP(data);
}
// DEC AX
void decode048(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 0;
    LOG_OP("DEC AX");
    NEXT_OP(data);
}
// DEC EAX
void decode248(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 0;
    LOG_OP("DEC EAX");
    NEXT_OP(data);
}
// DEC CX
void decode049(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 1;
    LOG_OP("DEC CX");
    NEXT_OP(data);
}
// DEC ECX
void decode249(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 1;
    LOG_OP("DEC ECX");
    NEXT_OP(data);
}
// DEC DX
void decode04a(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 2;
    LOG_OP("DEC DX");
    NEXT_OP(data);
}
// DEC EDX
void decode24a(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 2;
    LOG_OP("DEC EDX");
    NEXT_OP(data);
}
// DEC BX
void decode04b(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 3;
    LOG_OP("DEC BX");
    NEXT_OP(data);
}
// DEC EBX
void decode24b(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 3;
    LOG_OP("DEC EBX");
    NEXT_OP(data);
}
// DEC SP
void decode04c(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 4;
    LOG_OP("DEC SP");
    NEXT_OP(data);
}
// DEC ESP
void decode24c(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 4;
    LOG_OP("DEC ESP");
    NEXT_OP(data);
}
// DEC BP
void decode04d(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 5;
    LOG_OP("DEC BP");
    NEXT_OP(data);
}
// DEC EBP
void decode24d(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 5;
    LOG_OP("DEC EBP");
    NEXT_OP(data);
}
// DEC SI
void decode04e(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 6;
    LOG_OP("DEC SI");
    NEXT_OP(data);
}
// DEC ESI
void decode24e(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 6;
    LOG_OP("DEC ESI");
    NEXT_OP(data);
}
// DEC DI
void decode04f(struct DecodeData* data) {
    data->op->func = dec16_reg;
    data->op->r1 = 7;
    LOG_OP("DEC DI");
    NEXT_OP(data);
}
// DEC EDI
void decode24f(struct DecodeData* data) {
    data->op->func = dec32_reg;
    data->op->r1 = 7;
    LOG_OP("DEC EDI");
    NEXT_OP(data);
}
// PUSH AX
void decode050(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 0;
    LOG_OP("PUSH AX");
    NEXT_OP(data);
}
// PUSH EAX
void decode250(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 0;
    LOG_OP("PUSH EAX");
    NEXT_OP(data);
}
// PUSH CX
void decode051(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 1;
    LOG_OP("PUSH CX");
    NEXT_OP(data);
}
// PUSH ECX
void decode251(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 1;
    LOG_OP("PUSH ECX");
    NEXT_OP(data);
}
// PUSH DX
void decode052(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 2;
    LOG_OP("PUSH DX");
    NEXT_OP(data);
}
// PUSH EDX
void decode252(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 2;
    LOG_OP("PUSH EDX");
    NEXT_OP(data);
}
// PUSH BX
void decode053(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 3;
    LOG_OP("PUSH BX");
    NEXT_OP(data);
}
// PUSH EBX
void decode253(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 3;
    LOG_OP("PUSH EBX");
    NEXT_OP(data);
}
// PUSH SP
void decode054(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 4;
    LOG_OP("PUSH SP");
    NEXT_OP(data);
}
// PUSH ESP
void decode254(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 4;
    LOG_OP("PUSH ESP");
    NEXT_OP(data);
}
// PUSH BP
void decode055(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 5;
    LOG_OP("PUSH BP");
    NEXT_OP(data);
}
// PUSH EBP
void decode255(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 5;
    LOG_OP("PUSH EBP");
    NEXT_OP(data);
}
// PUSH SI
void decode056(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 6;
    LOG_OP("PUSH SI");
    NEXT_OP(data);
}
// PUSH ESI
void decode256(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 6;
    LOG_OP("PUSH ESI");
    NEXT_OP(data);
}
// PUSH DI
void decode057(struct DecodeData* data) {
    data->op->func = pushReg16;
    data->op->r1 = 7;
    LOG_OP("PUSH DI");
    NEXT_OP(data);
}
// PUSH EDI
void decode257(struct DecodeData* data) {
    data->op->func = pushReg32;
    data->op->r1 = 7;
    LOG_OP("PUSH EDI");
    NEXT_OP(data);
}
// POP AX
void decode058(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 0;
    LOG_OP("POP AX");
    NEXT_OP(data);
}
// POP EAX
void decode258(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 0;
    LOG_OP("POP EAX");
    NEXT_OP(data);
}
// POP CX
void decode059(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 1;
    LOG_OP("POP CX");
    NEXT_OP(data);
}
// POP ECX
void decode259(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 1;
    LOG_OP("POP ECX");
    NEXT_OP(data);
}
// POP DX
void decode05a(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 2;
    LOG_OP("POP DX");
    NEXT_OP(data);
}
// POP EDX
void decode25a(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 2;
    LOG_OP("POP EDX");
    NEXT_OP(data);
}
// POP BX
void decode05b(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 3;
    LOG_OP("POP BX");
    NEXT_OP(data);
}
// POP EBX
void decode25b(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 3;
    LOG_OP("POP EBX");
    NEXT_OP(data);
}
// POP SP
void decode05c(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 4;
    LOG_OP("POP SP");
    NEXT_OP(data);
}
// POP ESP
void decode25c(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 4;
    LOG_OP("POP ESP");
    NEXT_OP(data);
}
// POP BP
void decode05d(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 5;
    LOG_OP("POP BP");
    NEXT_OP(data);
}
// POP EBP
void decode25d(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 5;
    LOG_OP("POP EBP");
    NEXT_OP(data);
}
// POP SI
void decode05e(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 6;
    LOG_OP("POP SI");
    NEXT_OP(data);
}
// POP ESI
void decode25e(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 6;
    LOG_OP("POP ESI");
    NEXT_OP(data);
}
// POP DI
void decode05f(struct DecodeData* data) {
    data->op->func = popReg16;
    data->op->r1 = 7;
    LOG_OP("POP DI");
    NEXT_OP(data);
}
// POP EDI
void decode25f(struct DecodeData* data) {
    data->op->func = popReg32;
    data->op->r1 = 7;
    LOG_OP("POP EDI");
    NEXT_OP(data);
}
// PUSHA
void decode060(struct DecodeData* data) {
    data->op->func = pusha;
    LOG_OP("PUSHA");
    NEXT_OP(data);
}
// PUSHAD
void decode260(struct DecodeData* data) {
    data->op->func = pushad;
    LOG_OP("PUSHA");
    NEXT_OP(data);
}
// POPA
void decode061(struct DecodeData* data) {
    data->op->func = popa;
    LOG_OP("POPA");
    NEXT_OP(data);
}
// POPAD
void decode261(struct DecodeData* data) {
    data->op->func = popad;
    LOG_OP("POPA");
    NEXT_OP(data);
}
// MOVSB
void decode0a4(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = movsb16_r_op;
        } else {
            data->op->func = movsb16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = movsb32_r_op;
        } else {
            data->op->func = movsb32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// MOVSW
void decode0a5(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = movsw16_r_op;
        } else {
            data->op->func = movsw16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = movsw32_r_op;
        } else {
            data->op->func = movsw32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// MOVSD
void decode2a5(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = movsd16_r_op;
        } else {
            data->op->func = movsd16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = movsd32_r_op;
        } else {
            data->op->func = movsd32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// CMPSB
void decode0a6(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = cmpsb16_r_op;
        } else {
            data->op->func = cmpsb16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = cmpsb32_r_op;
        } else {
            data->op->func = cmpsb32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// CMPSW
void decode0a7(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = cmpsw16_r_op;
        } else {
            data->op->func = cmpsw16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = cmpsw32_r_op;
        } else {
            data->op->func = cmpsw32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// CMPSD
void decode2a7(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = cmpsd16_r_op;
        } else {
            data->op->func = cmpsd16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = cmpsd32_r_op;
        } else {
            data->op->func = cmpsd32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// STOSB
void decode0aa(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = stosb16_r_op;
        } else {
            data->op->func = stosb16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = stosb32_r_op;
        } else {
            data->op->func = stosb32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// STOSW
void decode0ab(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = stosw16_r_op;
        } else {
            data->op->func = stosw16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = stosw32_r_op;
        } else {
            data->op->func = stosw32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// STOSD
void decode2ab(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = stosd16_r_op;
        } else {
            data->op->func = stosd16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = stosd32_r_op;
        } else {
            data->op->func = stosd32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// LODSB
void decode0ac(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = lodsb16_r_op;
        } else {
            data->op->func = lodsb16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = lodsb32_r_op;
        } else {
            data->op->func = lodsb32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// LODSW
void decode0ad(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = lodsw16_r_op;
        } else {
            data->op->func = lodsw16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = lodsw32_r_op;
        } else {
            data->op->func = lodsw32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// LODSD
void decode2ad(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = lodsd16_r_op;
        } else {
            data->op->func = lodsd16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = lodsd32_r_op;
        } else {
            data->op->func = lodsd32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// SCASB
void decode0ae(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = scasb16_r_op;
        } else {
            data->op->func = scasb16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = scasb32_r_op;
        } else {
            data->op->func = scasb32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// SCASW
void decode0af(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = scasw16_r_op;
        } else {
            data->op->func = scasw16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = scasw32_r_op;
        } else {
            data->op->func = scasw32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// SCASD
void decode2af(struct DecodeData* data) {
    if (data->ea16) {
        if (data->rep) {
            data->op->func = scasd16_r_op;
        } else {
            data->op->func = scasd16_op;
        }
    } else {
        if (data->rep) {
            data->op->func = scasd32_r_op;
        } else {
            data->op->func = scasd32_op;
        }
    }
    data->op->data1 = data->rep_zero;
    data->op->base = data->ds;
    NEXT_OP(data);
}
// CMOVO
void decode140(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovO_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovO_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovO_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVO
void decode340(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovO_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovO_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovO_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNO
void decode141(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNO_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNO_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNO_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNO
void decode341(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNO_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNO_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNO_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVB
void decode142(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovB_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovB_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovB_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVB
void decode342(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovB_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovB_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovB_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNB
void decode143(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNB_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNB_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNB_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNB
void decode343(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNB_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNB_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNB_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVZ
void decode144(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovZ_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovZ_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovZ_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVZ
void decode344(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovZ_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovZ_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovZ_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNZ
void decode145(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNZ_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNZ_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNZ_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNZ
void decode345(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNZ_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNZ_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNZ_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVBE
void decode146(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovBE_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovBE_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovBE_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVBE
void decode346(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovBE_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovBE_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovBE_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNBE
void decode147(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNBE_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNBE_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNBE_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNBE
void decode347(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNBE_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNBE_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNBE_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVS
void decode148(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovS_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovS_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovS_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVS
void decode348(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovS_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovS_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovS_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNS
void decode149(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNS_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNS_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNS_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNS
void decode349(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNS_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNS_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNS_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVP
void decode14a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovP_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovP_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovP_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVP
void decode34a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovP_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovP_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovP_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNP
void decode14b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNP_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNP_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNP_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNP
void decode34b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNP_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNP_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNP_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVL
void decode14c(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovL_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovL_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovL_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVL
void decode34c(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovL_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovL_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovL_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNL
void decode14d(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNL_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNL_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNL_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNL
void decode34d(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNL_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNL_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNL_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVLE
void decode14e(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovLE_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovLE_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovLE_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVLE
void decode34e(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovLE_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovLE_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovLE_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNLE
void decode14f(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNLE_16_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R16(data->op->r1), R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNLE_16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = cmovNLE_16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R16(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMOVNLE
void decode34f(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmovNLE_32_reg;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("CMOV", R32(data->op->r1), R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmovNLE_32_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    } else {
        data->op->func = cmovNLE_32_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("CMOV", R32(data->op->r1), M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// PUSH ES
void decode006(struct DecodeData* data) {
    data->op->func = pushSeg16;
    data->op->r1 = ES;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// PUSH ES
void decode206(struct DecodeData* data) {
    data->op->func = pushSeg32;
    data->op->r1 = ES;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// POP ES
void decode007(struct DecodeData* data) {
    data->op->func = popSeg16;
    data->op->r1 = ES;
    LOG_OP1("POP", EABASE(data->op->r1));
    NEXT_OP(data);
}
// POP ES
void decode207(struct DecodeData* data) {
    data->op->func = popSeg32;
    data->op->r1 = ES;
    LOG_OP1("POP", EABASE(data->op->r1));
    NEXT_OP(data);
}
// PUSH CS
void decode00e(struct DecodeData* data) {
    data->op->func = pushSeg16;
    data->op->r1 = CS;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// PUSH CS
void decode20e(struct DecodeData* data) {
    data->op->func = pushSeg32;
    data->op->r1 = CS;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// PUSH SS
void decode016(struct DecodeData* data) {
    data->op->func = pushSeg16;
    data->op->r1 = SS;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// PUSH SS
void decode216(struct DecodeData* data) {
    data->op->func = pushSeg32;
    data->op->r1 = SS;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// POP SS
void decode017(struct DecodeData* data) {
    data->op->func = popSeg16;
    data->op->r1 = SS;
    LOG_OP1("POP", EABASE(data->op->r1));
    NEXT_OP(data);
}
// POP SS
void decode217(struct DecodeData* data) {
    data->op->func = popSeg32;
    data->op->r1 = SS;
    LOG_OP1("POP", EABASE(data->op->r1));
    NEXT_OP(data);
}
// PUSH DS
void decode01e(struct DecodeData* data) {
    data->op->func = pushSeg16;
    data->op->r1 = DS;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// PUSH DS
void decode21e(struct DecodeData* data) {
    data->op->func = pushSeg32;
    data->op->r1 = DS;
    LOG_OP1("PUSH", EABASE(data->op->r1));
    NEXT_OP(data);
}
// POP DS
void decode01f(struct DecodeData* data) {
    data->op->func = popSeg16;
    data->op->r1 = DS;
    LOG_OP1("POP", EABASE(data->op->r1));
    NEXT_OP(data);
}
// POP DS
void decode21f(struct DecodeData* data) {
    data->op->func = popSeg32;
    data->op->r1 = DS;
    LOG_OP1("POP", EABASE(data->op->r1));
    NEXT_OP(data);
}
// SEG ES
void decode026(struct DecodeData* data) {
    data->ds = ES;
    data->ss = ES;
    RESTART_OP(data);
}
// DAA
void decode027(struct DecodeData* data) {
    data->op->func = daa;
    LOG_OP("DAA");
    NEXT_OP(data);
}
// SEG CS
void decode02e(struct DecodeData* data) {
    data->ds = CS;
    data->ss = CS;
    RESTART_OP(data);
}
// DAS
void decode02f(struct DecodeData* data) {
    data->op->func = das;
    LOG_OP("DAS");
    NEXT_OP(data);
}
// SEG SS
void decode036(struct DecodeData* data) {
    data->ds = SS;
    data->ss = SS;
    RESTART_OP(data);
}
// AAA
void decode037(struct DecodeData* data) {
    data->op->func = aaa;
    LOG_OP("AAA");
    NEXT_OP(data);
}
// SEG DS
void decode03e(struct DecodeData* data) {
    data->ds = DS;
    data->ss = DS;
    RESTART_OP(data);
}
// AAS
void decode03f(struct DecodeData* data) {
    data->op->func = aas;
    LOG_OP("AAS");
    NEXT_OP(data);
}
// SEG FS
void decode064(struct DecodeData* data) {
    data->ds = FS;
    data->ss = FS;
    RESTART_OP(data);
}
// SEG GS
void decode065(struct DecodeData* data) {
    data->ds = GS;
    data->ss = GS;
    RESTART_OP(data);
}
// PUSH Iw
void decode068(struct DecodeData* data) {
    data->op->func = push16data;
    data->op->data1 = FETCH16(data);
    LOG_OP1("PUSH", itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// PUSH Id
void decode268(struct DecodeData* data) {
    data->op->func = push32data;
    data->op->data1 = FETCH32(data);
    LOG_OP1("PUSH", itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// IMUL Gw,Ew,Iw
void decode069(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dimulcr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("DIMULC", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dimulcr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("DIMULC", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = dimulcr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("DIMULC", R16(data->op->r1),M16(data, rm, data->op));
    }
    data->op->data1 = FETCH_S16(data);
    NEXT_OP(data);
}
// IMUL Gd,Ed,Id
void decode269(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dimulcr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("DIMULC", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dimulcr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("DIMULC", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = dimulcr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("DIMULC", R32(data->op->r1),M32(data, rm, data->op));
    }
    data->op->data1 = FETCH32(data);
    NEXT_OP(data);
}
// PUSH Ib
void decode06a(struct DecodeData* data) {
    data->op->func = push16data;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("PUSH", itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// PUSH Ib
void decode26a(struct DecodeData* data) {
    data->op->func = push32data;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("PUSH", itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// IMUL Gw,Ew,Ib
void decode06b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dimulcr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("DIMULC", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dimulcr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("DIMULC", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = dimulcr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("DIMULC", R16(data->op->r1),M16(data, rm, data->op));
    }
    data->op->data1 = FETCH_S8(data);
    NEXT_OP(data);
}
// IMUL Gd,Ed,Ib
void decode26b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dimulcr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("DIMULC", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dimulcr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("DIMULC", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = dimulcr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("DIMULC", R32(data->op->r1),M32(data, rm, data->op));
    }
    data->op->data1 = FETCH_S8(data);
    NEXT_OP(data);
}
// JO
void decode070(struct DecodeData* data) {
    data->op->func = jumpO;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JO", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNO
void decode071(struct DecodeData* data) {
    data->op->func = jumpNO;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNO", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JB
void decode072(struct DecodeData* data) {
    data->op->func = jumpB;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JB", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNB
void decode073(struct DecodeData* data) {
    data->op->func = jumpNB;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNB", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JZ
void decode074(struct DecodeData* data) {
    data->op->func = jumpZ;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JZ", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNZ
void decode075(struct DecodeData* data) {
    data->op->func = jumpNZ;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNZ", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JBE
void decode076(struct DecodeData* data) {
    data->op->func = jumpBE;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JBE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNBE
void decode077(struct DecodeData* data) {
    data->op->func = jumpNBE;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNBE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JS
void decode078(struct DecodeData* data) {
    data->op->func = jumpS;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JS", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNS
void decode079(struct DecodeData* data) {
    data->op->func = jumpNS;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNS", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JP
void decode07a(struct DecodeData* data) {
    data->op->func = jumpP;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JP", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNP
void decode07b(struct DecodeData* data) {
    data->op->func = jumpNP;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNP", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JL
void decode07c(struct DecodeData* data) {
    data->op->func = jumpL;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JL", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNL
void decode07d(struct DecodeData* data) {
    data->op->func = jumpNL;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNL", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JLE
void decode07e(struct DecodeData* data) {
    data->op->func = jumpLE;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JLE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNLE
void decode07f(struct DecodeData* data) {
    data->op->func = jumpNLE;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JNLE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// TEST Eb,Gb
void decode084(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = testr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("TEST", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = teste8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("TEST", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = teste8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("TEST", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// TEST Ew,Gw
void decode085(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = testr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("TEST", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = teste16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("TEST", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = teste16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("TEST", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// TEST Ed,Gd
void decode285(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = testr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("TEST", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = teste32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("TEST", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = teste32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("TEST", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// XCHG Eb,Gb
void decode086(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xchgr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("XCHG", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xchge8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("XCHG", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = xchge8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("XCHG", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// XCHG Ew,Gw
void decode087(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xchgr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("XCHG", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xchge16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("XCHG", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = xchge16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("XCHG", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// XCHG Ed,Gd
void decode287(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xchgr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("XCHG", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xchge32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("XCHG", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = xchge32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("XCHG", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// MOV Eb,Gb
void decode088(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr8r8;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = move8r8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("MOV", M8(data, rm, data->op),R8(data->op->r1));
    } else {
        data->op->func = move8r8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_1(data->op);
        LOG_OP2("MOV", M8(data, rm, data->op),R8(data->op->r1));
    }
    NEXT_OP(data);
}
// MOV Ew,Gw
void decode089(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = move16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("MOV", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = move16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("MOV", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// MOV Ed,Gd
void decode289(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = move32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("MOV", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = move32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("MOV", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// MOV Gb,Eb
void decode08a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr8r8;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOV", R8(data->op->r1),R8(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movr8e8_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", R8(data->op->r1),M8(data, rm, data->op));
    } else {
        data->op->func = movr8e8_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", R8(data->op->r1),M8(data, rm, data->op));
    }
    NEXT_OP(data);
}
// MOV Gw,Ew
void decode08b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOV", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = movr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// MOV Gd,Ed
void decode28b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOV", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = movr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// XCHG CX,AX
void decode091(struct DecodeData* data) {
    data->op->func = xchgr16r16;
    data->op->r1 = 0;
    data->op->r2 = 1;
    NEXT_OP(data);
}
// XCHG ECX,EAX
void decode291(struct DecodeData* data) {
    data->op->func = xchgr32r32;
    data->op->r1 = 0;
    data->op->r2 = 1;
    NEXT_OP(data);
}
// XCHG DX,AX
void decode092(struct DecodeData* data) {
    data->op->func = xchgr16r16;
    data->op->r1 = 0;
    data->op->r2 = 2;
    NEXT_OP(data);
}
// XCHG EDX,EAX
void decode292(struct DecodeData* data) {
    data->op->func = xchgr32r32;
    data->op->r1 = 0;
    data->op->r2 = 2;
    NEXT_OP(data);
}
// XCHG BX,AX
void decode093(struct DecodeData* data) {
    data->op->func = xchgr16r16;
    data->op->r1 = 0;
    data->op->r2 = 3;
    NEXT_OP(data);
}
// XCHG EBX,EAX
void decode293(struct DecodeData* data) {
    data->op->func = xchgr32r32;
    data->op->r1 = 0;
    data->op->r2 = 3;
    NEXT_OP(data);
}
// XCHG SP,AX
void decode094(struct DecodeData* data) {
    data->op->func = xchgr16r16;
    data->op->r1 = 0;
    data->op->r2 = 4;
    NEXT_OP(data);
}
// XCHG ESP,EAX
void decode294(struct DecodeData* data) {
    data->op->func = xchgr32r32;
    data->op->r1 = 0;
    data->op->r2 = 4;
    NEXT_OP(data);
}
// XCHG BP,AX
void decode095(struct DecodeData* data) {
    data->op->func = xchgr16r16;
    data->op->r1 = 0;
    data->op->r2 = 5;
    NEXT_OP(data);
}
// XCHG EBP,EAX
void decode295(struct DecodeData* data) {
    data->op->func = xchgr32r32;
    data->op->r1 = 0;
    data->op->r2 = 5;
    NEXT_OP(data);
}
// XCHG SI,AX
void decode096(struct DecodeData* data) {
    data->op->func = xchgr16r16;
    data->op->r1 = 0;
    data->op->r2 = 6;
    NEXT_OP(data);
}
// XCHG ESI,EAX
void decode296(struct DecodeData* data) {
    data->op->func = xchgr32r32;
    data->op->r1 = 0;
    data->op->r2 = 6;
    NEXT_OP(data);
}
// XCHG DI,AX
void decode097(struct DecodeData* data) {
    data->op->func = xchgr16r16;
    data->op->r1 = 0;
    data->op->r2 = 7;
    NEXT_OP(data);
}
// XCHG EDI,EAX
void decode297(struct DecodeData* data) {
    data->op->func = xchgr32r32;
    data->op->r1 = 0;
    data->op->r2 = 7;
    NEXT_OP(data);
}
// CBW
void decode098(struct DecodeData* data) {
    data->op->func = cbw;
    LOG_OP("CBW");
    NEXT_OP(data);
}
// CBWE
void decode298(struct DecodeData* data) {
    data->op->func = cbwe;
    LOG_OP("CBWE");
    NEXT_OP(data);
}
// CWD
void decode099(struct DecodeData* data) {
    data->op->func = cwd;
    LOG_OP("CWD");
    NEXT_OP(data);
}
// CWQ
void decode299(struct DecodeData* data) {
    data->op->func = cwq;
    LOG_OP("CWQ");
    NEXT_OP(data);
}
// PUSHF16
void decode09c(struct DecodeData* data) {
    data->op->func = pushf16;
    LOG_OP("PUSHF");
    NEXT_OP(data);
}
// PUSHF32
void decode29c(struct DecodeData* data) {
    data->op->func = pushf32;
    LOG_OP("PUSHF");
    NEXT_OP(data);
}
// POPF16
void decode09d(struct DecodeData* data) {
    data->op->func = popf16;
    LOG_OP("POPF");
    NEXT_OP(data);
}
// POPF32
void decode29d(struct DecodeData* data) {
    data->op->func = popf32;
    LOG_OP("POPF");
    NEXT_OP(data);
}
// SAHF
void decode09e(struct DecodeData* data) {
    data->op->func = sahf;
    LOG_OP("SAHF");
    NEXT_OP(data);
}
// LAHF
void decode09f(struct DecodeData* data) {
    data->op->func = lahf;
    LOG_OP("LAHF");
    NEXT_OP(data);
}
// MOV AL,Ob
void decode0a0(struct DecodeData* data) {
    data->op->func = movAl;
    if (data->ea16) {
        data->op->data1 = FETCH16(data);
    } else {
        data->op->data1 = FETCH32(data);
    }
    data->op->base = data->ds;
    LOG_OP2("MOV", "AL", O8(data, data->op));
    NEXT_OP(data);
}
// MOV AX,Ow
void decode0a1(struct DecodeData* data) {
    data->op->func = movAx;
    if (data->ea16) {
        data->op->data1 = FETCH16(data);
    } else {
        data->op->data1 = FETCH32(data);
    }
    data->op->base = data->ds;
    LOG_OP2("MOV", "AX", O16(data, data->op));
    NEXT_OP(data);
}
// MOV EAX,Od
void decode2a1(struct DecodeData* data) {
    data->op->func = movEax;
    if (data->ea16) {
        data->op->data1 = FETCH16(data);
    } else {
        data->op->data1 = FETCH32(data);
    }
    data->op->base = data->ds;
    LOG_OP2("MOV", "EAX", O32(data, data->op));
    NEXT_OP(data);
}
// MOV Ob,Al
void decode0a2(struct DecodeData* data) {
    data->op->func = movDirectAl;
    if (data->ea16) {
        data->op->data1 = FETCH16(data);
    } else {
        data->op->data1 = FETCH32(data);
    }
    WRITE_ADDRESS_DIRECT(data->op);
    WRITE_BYTES_1(data->op);
    data->op->base = data->ds;
    LOG_OP2("MOV", O8(data, data->op), "AL");
    NEXT_OP(data);
}
// MOV Ow,Ax
void decode0a3(struct DecodeData* data) {
    data->op->func = movDirectAx;
    if (data->ea16) {
        data->op->data1 = FETCH16(data);
    } else {
        data->op->data1 = FETCH32(data);
    }
    WRITE_ADDRESS_DIRECT(data->op);
    WRITE_BYTES_2(data->op);
    data->op->base = data->ds;
    LOG_OP2("MOV", O16(data, data->op), "AX");
    NEXT_OP(data);
}
// MOV Od,Eax
void decode2a3(struct DecodeData* data) {
    data->op->func = movDirectEax;
    if (data->ea16) {
        data->op->data1 = FETCH16(data);
    } else {
        data->op->data1 = FETCH32(data);
    }
    WRITE_ADDRESS_DIRECT(data->op);
    WRITE_BYTES_4(data->op);
    data->op->base = data->ds;
    LOG_OP2("MOV", O32(data, data->op), "EAX");
    NEXT_OP(data);
}
// TEST AL,Ib
void decode0a8(struct DecodeData* data) {
    data->op->func = test8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("TEST", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// TEST AX,Iw
void decode0a9(struct DecodeData* data) {
    data->op->func = test16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("TEST", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// TEST EAX,Id
void decode2a9(struct DecodeData* data) {
    data->op->func = test32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("TEST", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV AL,Ib
void decode0b0(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV CL,Ib
void decode0b1(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 1;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV DL,Ib
void decode0b2(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 2;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV BL,Ib
void decode0b3(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 3;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV AH,Ib
void decode0b4(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 4;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV CH,Ib
void decode0b5(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 5;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV DH,Ib
void decode0b6(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 6;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV BH,Ib
void decode0b7(struct DecodeData* data) {
    data->op->func = mov8_reg;
    data->op->r1 = 7;
    data->op->data1 = FETCH8(data);
    LOG_OP2("MOV", R8(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV AX,Iw
void decode0b8(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV EAX,Id
void decode2b8(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 0;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV CX,Iw
void decode0b9(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 1;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV ECX,Id
void decode2b9(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 1;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV DX,Iw
void decode0ba(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 2;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV EDX,Id
void decode2ba(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 2;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV BX,Iw
void decode0bb(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 3;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV EBX,Id
void decode2bb(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 3;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV SP,Iw
void decode0bc(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 4;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV ESP,Id
void decode2bc(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 4;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV BP,Iw
void decode0bd(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 5;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV EBP,Id
void decode2bd(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 5;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV SI,Iw
void decode0be(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 6;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV ESI,Id
void decode2be(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 6;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV DI,Iw
void decode0bf(struct DecodeData* data) {
    data->op->func = mov16_reg;
    data->op->r1 = 7;
    data->op->data1 = FETCH16(data);
    LOG_OP2("MOV", R16(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// MOV EDI,Id
void decode2bf(struct DecodeData* data) {
    data->op->func = mov32_reg;
    data->op->r1 = 7;
    data->op->data1 = FETCH32(data);
    LOG_OP2("MOV", R32(data->op->r1), itoa(data->op->data1, data->tmp,16));
    NEXT_OP(data);
}
// RETN Iw
void decode0c2(struct DecodeData* data) {
    data->op->func = retnIw16;
    data->op->data1 = FETCH16(data);
    LOG_OP1("RETN", itoa(data->op->data1, data->tmp,16));
    FINISH_OP(data);
}
// RETN Iw
void decode2c2(struct DecodeData* data) {
    data->op->func = retnIw32;
    data->op->data1 = FETCH16(data);
    LOG_OP1("RETN", itoa(data->op->data1, data->tmp,16));
    FINISH_OP(data);
}
// RETN16
void decode0c3(struct DecodeData* data) {
    data->op->func = retn16;
    LOG_OP("RETN");
    FINISH_OP(data);
}
// RETN32
void decode2c3(struct DecodeData* data) {
    data->op->func = retn32;
    LOG_OP("RETN");
    FINISH_OP(data);
}
// MOV EB,IB
void decode0c6(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(mov8_reg, mov8_mem16, mov8_mem32);
    WRITE_BYTES_1(data->op);
    data->op->data1 = FETCH8(data);
    LOG_E8C("MOV", rm, data);
    NEXT_OP(data);
}
// MOV EW,IW
void decode0c7(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(mov16_reg, mov16_mem16, mov16_mem32);
    WRITE_BYTES_2(data->op);
    data->op->data1 = FETCH16(data);
    LOG_E16C("MOV", rm, data);
    NEXT_OP(data);
}
// MOV ED,ID
void decode2c7(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(mov32_reg, mov32_mem16, mov32_mem32);
    WRITE_BYTES_4(data->op);
    data->op->data1 = FETCH32(data);
    LOG_E32C("MOV", rm, data);
    NEXT_OP(data);
}
// LEAVE16
void decode0c9(struct DecodeData* data) {
    data->op->func = leave16;
    LOG_OP("LEAVE");
    NEXT_OP(data);
}
// LEAVE32
void decode2c9(struct DecodeData* data) {
    data->op->func = leave32;
    LOG_OP("LEAVE");
    NEXT_OP(data);
}
// SALC
void decode0d6(struct DecodeData* data) {
    data->op->func = salc;
    LOG_OP("SALC");
    NEXT_OP(data);
}
// CALL Jw 
void decode0e8(struct DecodeData* data) {
    data->op->func = callJw;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("CALL", itoa(data->op->data1, data->tmp,16));
    FINISH_OP(data);
}
// CALL Jd 
void decode2e8(struct DecodeData* data) {
    data->op->func = callJd;
    data->op->data1 = FETCH32(data);
    LOG_OP1("CALL", itoa(data->op->data1, data->tmp,16));
    FINISH_OP(data);
}
// JMP Jw 
void decode0e9(struct DecodeData* data) {
    data->op->func = jump;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JMP", itoa(data->op->data1, data->tmp,16));
    FINISH_OP(data);
}
// JMP Jd 
void decode2e9(struct DecodeData* data) {
    data->op->func = jump;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JMP", itoa(data->op->data1, data->tmp,16));
    FINISH_OP(data);
}
// JMP Jb 
void decode0eb(struct DecodeData* data) {
    data->op->func = jump;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JMP", itoa(data->op->data1, data->tmp,16));
    FINISH_OP(data);
}
// CMC
void decode0f5(struct DecodeData* data) {
    data->op->func = cmc;
    LOG_OP("CMC");
    NEXT_OP(data);
}
// GRP3 Eb(,Ib)
void decode0f6(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0x00:
    case 0x01:
        DECODE_E(test8_reg, test8_mem16, test8_mem32);
        data->op->data1 = FETCH8(data);
        LOG_E8C("TEST", rm, data);
        break;
    case 0x02:
        DECODE_E(not8_reg, not8_mem16, not8_mem32);
        LOG_E8("NOT", rm, data);
        break;
    case 0x03:
        DECODE_E(neg8_reg, neg8_mem16, neg8_mem32);
        LOG_E8("NEG", rm, data);
        break;
    case 0x04:
        DECODE_E(mul8_reg, mul8_mem16, mul8_mem32);
        LOG_E8("MUL", rm, data);
        break;
    case 0x05:
        DECODE_E(imul8_reg, imul8_mem16, imul8_mem32);
        LOG_E8("IMUL", rm, data);
        break;
    case 0x06:
        DECODE_E(div8_reg, div8_mem16, div8_mem32);
        LOG_E8("DIV", rm, data);
        break;
    case 0x07:
        DECODE_E(idiv8_reg, idiv8_mem16, idiv8_mem32);
        LOG_E8("IDIV", rm, data);
        break;
    }
    WRITE_BYTES_1(data->op);
    NEXT_OP(data);
}
// GRP3 Ew(,Iw)
void decode0f7(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0x00:
    case 0x01:
        DECODE_E(test16_reg, test16_mem16, test16_mem32);
        data->op->data1 = FETCH16(data);
        LOG_E16C("TEST", rm, data);
        break;
    case 0x02:
        DECODE_E(not16_reg, not16_mem16, not16_mem32);
        LOG_E16("NOT", rm, data);
        break;
    case 0x03:
        DECODE_E(neg16_reg, neg16_mem16, neg16_mem32);
        LOG_E16("NEG", rm, data);
        break;
    case 0x04:
        DECODE_E(mul16_reg, mul16_mem16, mul16_mem32);
        LOG_E16("MUL", rm, data);
        break;
    case 0x05:
        DECODE_E(imul16_reg, imul16_mem16, imul16_mem32);
        LOG_E16("IMUL", rm, data);
        break;
    case 0x06:
        DECODE_E(div16_reg, div16_mem16, div16_mem32);
        LOG_E16("DIV", rm, data);
        break;
    case 0x07:
        DECODE_E(idiv16_reg, idiv16_mem16, idiv16_mem32);
        LOG_E16("IDIV", rm, data);
        break;
    }
    WRITE_BYTES_2(data->op);
    NEXT_OP(data);
}
// GRP3 Ed(,Id)
void decode2f7(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0x00:
    case 0x01:
        DECODE_E(test32_reg, test32_mem16, test32_mem32);
        data->op->data1 = FETCH32(data);
        LOG_E32C("TEST", rm, data);
        break;
    case 0x02:
        DECODE_E(not32_reg, not32_mem16, not32_mem32);
        LOG_E32("NOT", rm, data);
        break;
    case 0x03:
        DECODE_E(neg32_reg, neg32_mem16, neg32_mem32);
        LOG_E32("NEG", rm, data);
        break;
    case 0x04:
        DECODE_E(mul32_reg, mul32_mem16, mul32_mem32);
        LOG_E32("MUL", rm, data);
        break;
    case 0x05:
        DECODE_E(imul32_reg, imul32_mem16, imul32_mem32);
        LOG_E32("IMUL", rm, data);
        break;
    case 0x06:
        DECODE_E(div32_reg, div32_mem16, div32_mem32);
        LOG_E32("DIV", rm, data);
        break;
    case 0x07:
        DECODE_E(idiv32_reg, idiv32_mem16, idiv32_mem32);
        LOG_E32("IDIV", rm, data);
        break;
    }
    WRITE_BYTES_4(data->op);
    NEXT_OP(data);
}
// CLC
void decode0f8(struct DecodeData* data) {
    data->op->func = clc;
    LOG_OP("CLC");
    NEXT_OP(data);
}
// STC
void decode0f9(struct DecodeData* data) {
    data->op->func = stc;
    LOG_OP("STC");
    NEXT_OP(data);
}
// CLD
void decode0fc(struct DecodeData* data) {
    data->op->func = cld;
    LOG_OP("CLD");
    NEXT_OP(data);
}
// STD
void decode0fd(struct DecodeData* data) {
    data->op->func = std;
    LOG_OP("STD");
    NEXT_OP(data);
}
// RDTSC
void decode131(struct DecodeData* data) {
    data->op->func = rdtsc;
    LOG_OP("RDTSC");
    NEXT_OP(data);
}
// JO
void decode180(struct DecodeData* data) {
    data->op->func = jumpO;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JO", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNO
void decode181(struct DecodeData* data) {
    data->op->func = jumpNO;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNO", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JB
void decode182(struct DecodeData* data) {
    data->op->func = jumpB;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JB", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNB
void decode183(struct DecodeData* data) {
    data->op->func = jumpNB;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNB", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JZ
void decode184(struct DecodeData* data) {
    data->op->func = jumpZ;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JZ", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNZ
void decode185(struct DecodeData* data) {
    data->op->func = jumpNZ;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNZ", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JBE
void decode186(struct DecodeData* data) {
    data->op->func = jumpBE;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JBE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNBE
void decode187(struct DecodeData* data) {
    data->op->func = jumpNBE;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNBE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JS
void decode188(struct DecodeData* data) {
    data->op->func = jumpS;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JS", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNS
void decode189(struct DecodeData* data) {
    data->op->func = jumpNS;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNS", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JP
void decode18a(struct DecodeData* data) {
    data->op->func = jumpP;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JP", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNP
void decode18b(struct DecodeData* data) {
    data->op->func = jumpNP;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNP", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JL
void decode18c(struct DecodeData* data) {
    data->op->func = jumpL;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JL", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNL
void decode18d(struct DecodeData* data) {
    data->op->func = jumpNL;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNL", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JLE
void decode18e(struct DecodeData* data) {
    data->op->func = jumpLE;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JLE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNLE
void decode18f(struct DecodeData* data) {
    data->op->func = jumpNLE;
    data->op->data1 = FETCH_S16(data);
    LOG_OP1("JNLE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JO
void decode380(struct DecodeData* data) {
    data->op->func = jumpO;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JO", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNO
void decode381(struct DecodeData* data) {
    data->op->func = jumpNO;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNO", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JB
void decode382(struct DecodeData* data) {
    data->op->func = jumpB;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JB", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNB
void decode383(struct DecodeData* data) {
    data->op->func = jumpNB;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNB", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JZ
void decode384(struct DecodeData* data) {
    data->op->func = jumpZ;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JZ", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNZ
void decode385(struct DecodeData* data) {
    data->op->func = jumpNZ;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNZ", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JBE
void decode386(struct DecodeData* data) {
    data->op->func = jumpBE;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JBE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNBE
void decode387(struct DecodeData* data) {
    data->op->func = jumpNBE;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNBE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JS
void decode388(struct DecodeData* data) {
    data->op->func = jumpS;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JS", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNS
void decode389(struct DecodeData* data) {
    data->op->func = jumpNS;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNS", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JP
void decode38a(struct DecodeData* data) {
    data->op->func = jumpP;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JP", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNP
void decode38b(struct DecodeData* data) {
    data->op->func = jumpNP;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNP", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JL
void decode38c(struct DecodeData* data) {
    data->op->func = jumpL;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JL", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNL
void decode38d(struct DecodeData* data) {
    data->op->func = jumpNL;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNL", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JLE
void decode38e(struct DecodeData* data) {
    data->op->func = jumpLE;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JLE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// JNLE
void decode38f(struct DecodeData* data) {
    data->op->func = jumpNLE;
    data->op->data1 = FETCH32(data);
    LOG_OP1("JNLE", itoa((int)data->op->data1, data->tmp,10));
    FINISH_OP(data);
}
// SETO
void decode190(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setO_reg, setO_mem16, setO_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETO", rm, data);
    NEXT_OP(data);
}
// SETNO
void decode191(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNO_reg, setNO_mem16, setNO_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNO", rm, data);
    NEXT_OP(data);
}
// SETB
void decode192(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setB_reg, setB_mem16, setB_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETB", rm, data);
    NEXT_OP(data);
}
// SETNB
void decode193(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNB_reg, setNB_mem16, setNB_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNB", rm, data);
    NEXT_OP(data);
}
// SETZ
void decode194(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setZ_reg, setZ_mem16, setZ_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETZ", rm, data);
    NEXT_OP(data);
}
// SETNZ
void decode195(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNZ_reg, setNZ_mem16, setNZ_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNZ", rm, data);
    NEXT_OP(data);
}
// SETBE
void decode196(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setBE_reg, setBE_mem16, setBE_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETBE", rm, data);
    NEXT_OP(data);
}
// SETNBE
void decode197(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNBE_reg, setNBE_mem16, setNBE_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNBE", rm, data);
    NEXT_OP(data);
}
// SETS
void decode198(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setS_reg, setS_mem16, setS_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETS", rm, data);
    NEXT_OP(data);
}
// SETNS
void decode199(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNS_reg, setNS_mem16, setNS_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNS", rm, data);
    NEXT_OP(data);
}
// SETP
void decode19a(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setP_reg, setP_mem16, setP_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETP", rm, data);
    NEXT_OP(data);
}
// SETNP
void decode19b(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNP_reg, setNP_mem16, setNP_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNP", rm, data);
    NEXT_OP(data);
}
// SETL
void decode19c(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setL_reg, setL_mem16, setL_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETL", rm, data);
    NEXT_OP(data);
}
// SETNL
void decode19d(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNL_reg, setNL_mem16, setNL_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNL", rm, data);
    NEXT_OP(data);
}
// SETLE
void decode19e(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setLE_reg, setLE_mem16, setLE_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETLE", rm, data);
    NEXT_OP(data);
}
// SETNLE
void decode19f(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    DECODE_E(setNLE_reg, setNLE_mem16, setNLE_mem32);
    WRITE_BYTES_1(data->op);
    LOG_E8("SETNLE", rm, data);
    NEXT_OP(data);
}
// PUSH FS
void decode1a0(struct DecodeData* data) {
    data->op->func = pushSeg16;
    data->op->r1 = FS;
    LOG_OP("PUSH FS");
    NEXT_OP(data);
}
// PUSH FS
void decode3a0(struct DecodeData* data) {
    data->op->func = pushSeg32;
    data->op->r1 = FS;
    LOG_OP("PUSH FS");
    NEXT_OP(data);
}
// POP FS
void decode1a1(struct DecodeData* data) {
    data->op->func = popSeg16;
    data->op->r1 = FS;
    LOG_OP("POP FS");
    NEXT_OP(data);
}
// POP FS
void decode3a1(struct DecodeData* data) {
    data->op->func = popSeg32;
    data->op->r1 = FS;
    LOG_OP("POP FS");
    NEXT_OP(data);
}
// CPUID_OP
void decode1a2(struct DecodeData* data) {
    data->op->func = cpuid_op;
    LOG_OP("CPUID");
    NEXT_OP(data);
}
// BT Ew,Gw
void decode1a3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = btr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("BT", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = bte16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("BT", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = bte16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("BT", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// BT Ed,Gd
void decode3a3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = btr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("BT", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = bte32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BT", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = bte32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BT", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// DSHL Ew,Gw
void decode1a4(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshlr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHL", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshle16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHL", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = dshle16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHL", M16(data, rm, data->op),R16(data->op->r1));
    }
    data->op->data1 = FETCH8(data);
    data->op->data1 &= 0x1f;
    if (data->op->data1 == 0) {
        RESTART(data);
    }
    NEXT_OP(data);
}
// DSHL Ed,Gd
void decode3a4(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshlr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHL", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshle32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHL", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = dshle32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHL", M32(data, rm, data->op),R32(data->op->r1));
    }
    data->op->data1 = FETCH8(data);
    data->op->data1 &= 0x1f;
    if (data->op->data1 == 0) {
        RESTART(data);
    }
    NEXT_OP(data);
}
// DSHLCL Ew,Gw
void decode1a5(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshlclr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHLCL", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshlcle16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHLCL", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = dshlcle16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHLCL", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// DSHLCL Ed,Gd
void decode3a5(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshlclr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHLCL", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshlcle32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHLCL", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = dshlcle32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHLCL", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// PUSH GS
void decode1a8(struct DecodeData* data) {
    data->op->func = pushSeg16;
    data->op->r1 = GS;
    LOG_OP("PUSH GS");
    NEXT_OP(data);
}
// PUSH GS
void decode3a8(struct DecodeData* data) {
    data->op->func = pushSeg32;
    data->op->r1 = GS;
    LOG_OP("PUSH GS");
    NEXT_OP(data);
}
// POP GS
void decode1a9(struct DecodeData* data) {
    data->op->func = popSeg16;
    data->op->r1 = GS;
    LOG_OP("POP GS");
    NEXT_OP(data);
}
// POP GS
void decode3a9(struct DecodeData* data) {
    data->op->func = popSeg32;
    data->op->r1 = GS;
    LOG_OP("POP GS");
    NEXT_OP(data);
}
// BTS Ew,Gw
void decode1ab(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = btsr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("BTS", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = btse16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("BTS", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = btse16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("BTS", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// BTS Ed,Gd
void decode3ab(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = btsr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("BTS", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = btse32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BTS", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = btse32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BTS", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// DSHR Ew,Gw
void decode1ac(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshrr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHR", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshre16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHR", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = dshre16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHR", M16(data, rm, data->op),R16(data->op->r1));
    }
    data->op->data1 = FETCH8(data);
    data->op->data1 &= 0x1f;
    if (data->op->data1 == 0) {
        RESTART(data);
    }
    NEXT_OP(data);
}
// DSHR Ed,Gd
void decode3ac(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshrr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHR", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshre32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHR", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = dshre32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHR", M32(data, rm, data->op),R32(data->op->r1));
    }
    data->op->data1 = FETCH8(data);
    data->op->data1 &= 0x1f;
    if (data->op->data1 == 0) {
        RESTART(data);
    }
    NEXT_OP(data);
}
// DSHRCL Ew,Gw
void decode1ad(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshrclr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHRCL", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshrcle16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHRCL", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = dshrcle16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("DSHRCL", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// DSHRCL Ed,Gd
void decode3ad(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dshrclr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("DSHRCL", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dshrcle32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHRCL", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = dshrcle32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("DSHRCL", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// DIMUL Gw,Ew
void decode1af(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dimulr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("DIMUL", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dimulr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("DIMUL", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = dimulr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("DIMUL", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// DIMUL Gd,Ed
void decode3af(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = dimulr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("DIMUL", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = dimulr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("DIMUL", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = dimulr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("DIMUL", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// CMPXCHG Ew,Gw
void decode1b1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpxchgr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("CMPXCHG", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpxchge16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("CMPXCHG", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = cmpxchge16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("CMPXCHG", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// CMPXCHG Ed,Gd
void decode3b1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = cmpxchgr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("CMPXCHG", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = cmpxchge32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("CMPXCHG", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = cmpxchge32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("CMPXCHG", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// BTR Ew,Gw
void decode1b3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = btrr16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("BTR", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = btre16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("BTR", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = btre16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("BTR", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}
// BTR Ed,Gd
void decode3b3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = btrr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("BTR", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = btre32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BTR", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = btre32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BTR", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// MOVXZ8 Gw,Ew
void decode1b6(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movxz8r16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOVXZ8", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movxz8r16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOVXZ8", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = movxz8r16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOVXZ8", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// MOVXZ8 Gd,Ed
void decode3b6(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movxz8r32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOVXZ8", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movxz8r32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOVXZ8", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = movxz8r32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOVXZ8", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// MOVXZ16 Gd,Ed
void decode3b7(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movxz16r32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOVXZ16", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movxz16r32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOVXZ16", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = movxz16r32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOVXZ16", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// BTC Ed,Gd
void decode3bb(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = btcr32r32;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("BTC", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = btce32r32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BTC", M32(data, rm, data->op),R32(data->op->r1));
    } else {
        data->op->func = btce32r32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP2("BTC", M32(data, rm, data->op),R32(data->op->r1));
    }
    NEXT_OP(data);
}
// BSF Gw,Ew
void decode1bc(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = bsfr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("BSF", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = bsfr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("BSF", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = bsfr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("BSF", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// BSF Gd,Ed
void decode3bc(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = bsfr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("BSF", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = bsfr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("BSF", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = bsfr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("BSF", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// BSR Gw,Ew
void decode1bd(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = bsrr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("BSR", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = bsrr16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("BSR", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = bsrr16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("BSR", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// BSR Gd,Ed
void decode3bd(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = bsrr32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("BSR", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = bsrr32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("BSR", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = bsrr32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("BSR", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// MOVSX8 Gw,Ew
void decode1be(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movsx8r16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOVSX8", R16(data->op->r1),R16(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movsx8r16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOVSX8", R16(data->op->r1),M16(data, rm, data->op));
    } else {
        data->op->func = movsx8r16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOVSX8", R16(data->op->r1),M16(data, rm, data->op));
    }
    NEXT_OP(data);
}
// MOVSX8 Gd,Ed
void decode3be(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movsx8r32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOVSX8", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movsx8r32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOVSX8", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = movsx8r32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOVSX8", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// MOVSX16 Gd,Ed
void decode3bf(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movsx16r32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("MOVSX16", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = movsx16r32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOVSX16", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = movsx16r32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOVSX16", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// XADD
void decode3c1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = xadd32r32r32;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("XADD32", R32(data->op->r1),R32(data->op->r2));
    } else if (data->ea16) {
        data->op->func = xadd32r32e32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("XADD32", R32(data->op->r1),M32(data, rm, data->op));
    } else {
        data->op->func = xadd32r32e32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("XADD32", R32(data->op->r1),M32(data, rm, data->op));
    }
    NEXT_OP(data);
}
// BSWAP
void decode3c8(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 0;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
// BSWAP
void decode3c9(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 1;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
// BSWAP
void decode3ca(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 2;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
// BSWAP
void decode3cb(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 3;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
// BSWAP
void decode3cc(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 4;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
// BSWAP
void decode3cd(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 5;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
// BSWAP
void decode3ce(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 6;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
// BSWAP
void decode3cf(struct DecodeData* data) {
    data->op->func = bswap32;
    data->op->r1 = 7;
    LOG_OP("BSWAP");
    NEXT_OP(data);
}
