void dynamic_addr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_adde8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_addr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_add8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_add8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_addr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_adde16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_addr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_add16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_add16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_addr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_adde32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_addr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_add32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_add32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_orr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_ore8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_orr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_or8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_or8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_orr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_ore16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_orr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_or16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_or16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_orr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_ore32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_orr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_or32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_or32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_adcr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adce8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adcr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adc8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adc8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adcr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adce16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adcr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adc16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adc16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adcr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adce32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adcr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adc32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adc32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_sbbr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbbe8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbbr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbb8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbb8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbbr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbbe16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbbr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbb16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbb16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbbr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbbe32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbbr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbb32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbb32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_andr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_ande8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_andr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_and8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_and8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_andr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_ande16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_andr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_and16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_and16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_andr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_ande32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_andr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_and32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_and32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_subr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_sube8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_subr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_sub8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_sub8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_subr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_sube16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_subr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_sub16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_sub16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_subr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_sube32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_subr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_sub32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_sub32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_xorr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xore8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xorr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xor8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xor8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xorr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xore16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xorr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xor16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xor16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xorr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xore32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xorr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xor32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xor32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_cmpr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmpe8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmpr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmp8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmp8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmpr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmpe16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmpr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmp16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmp16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmpr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmpe32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmpr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmp32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmp32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_testr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_teste8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_test8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_test8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_testr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_teste16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_test16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_test16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_testr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_teste32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_test32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_test32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_notr8(DynamicData* data, DecodedOp* op) {
instCPU('~', CPU::offsetofReg8(op->reg), DYN_8bit);
    INCREMENT_EIP(data, op);
}
void dynamic_note8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    instMem('~', DYN_ADDRESS, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_notr16(DynamicData* data, DecodedOp* op) {
instCPU('~', CPU::offsetofReg16(op->reg), DYN_16bit);
    INCREMENT_EIP(data, op);
}
void dynamic_note16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    instMem('~', DYN_ADDRESS, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_notr32(DynamicData* data, DecodedOp* op) {
instCPU('~', CPU::offsetofReg32(op->reg), DYN_32bit);
    INCREMENT_EIP(data, op);
}
void dynamic_note32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    instMem('~', DYN_ADDRESS, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_negr8(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPU('-', CPU::offsetofReg8(op->reg), DYN_8bit);
    } else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u8), CPU::offsetofReg8(op->reg), DYN_8bit, DYN_DEST, false);
        instReg('-', DYN_DEST, DYN_8bit);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_DEST, DYN_8bit, false);
        movToCpuFromReg(CPU::offsetofReg8(op->reg), DYN_DEST, DYN_8bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NEG8);
        data->currentLazyFlags=FLAGS_NEG8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_nege8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags()) {
        instMem('-', DYN_ADDRESS, DYN_8bit, true);
    } else {
        movToCpuFromMem(CPU_OFFSET_OF(src.u8), DYN_8bit, DYN_ADDRESS, false, false);
        instReg('-', DYN_CALL_RESULT, DYN_8bit);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NEG8);
        data->currentLazyFlags=FLAGS_NEG8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_negr16(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPU('-', CPU::offsetofReg16(op->reg), DYN_16bit);
    } else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u16), CPU::offsetofReg16(op->reg), DYN_16bit, DYN_DEST, false);
        instReg('-', DYN_DEST, DYN_16bit);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_DEST, DYN_16bit, false);
        movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_DEST, DYN_16bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NEG16);
        data->currentLazyFlags=FLAGS_NEG16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_nege16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags()) {
        instMem('-', DYN_ADDRESS, DYN_16bit, true);
    } else {
        movToCpuFromMem(CPU_OFFSET_OF(src.u16), DYN_16bit, DYN_ADDRESS, false, false);
        instReg('-', DYN_CALL_RESULT, DYN_16bit);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NEG16);
        data->currentLazyFlags=FLAGS_NEG16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_negr32(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPU('-', CPU::offsetofReg32(op->reg), DYN_32bit);
    } else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u32), CPU::offsetofReg32(op->reg), DYN_32bit, DYN_DEST, false);
        instReg('-', DYN_DEST, DYN_32bit);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_DEST, DYN_32bit, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_DEST, DYN_32bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NEG32);
        data->currentLazyFlags=FLAGS_NEG32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_nege32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags()) {
        instMem('-', DYN_ADDRESS, DYN_32bit, true);
    } else {
        movToCpuFromMem(CPU_OFFSET_OF(src.u32), DYN_32bit, DYN_ADDRESS, false, false);
        instReg('-', DYN_CALL_RESULT, DYN_32bit);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NEG32);
        data->currentLazyFlags=FLAGS_NEG32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_mulR8(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_mulE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_imulR8(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_imulE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_mulR16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_mulE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    INCREMENT_EIP(data, op);
}
void dynamic_imulR16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_imulE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    INCREMENT_EIP(data, op);
}
void dynamic_mulR32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_mulE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_imulR32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_imulE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_divR8(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)div8, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_divE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    callHostFunction((void*)div8, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_idivR8(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->reg), DYN_8bit);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false); // ARM with -O2 needs this
    callHostFunction((void*)idiv8, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_idivE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false); // ARM with -O2 needs this
    callHostFunction((void*)idiv8, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_divR16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)div16, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_divE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)div16, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_idivR16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->reg), DYN_16bit);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false); // ARM64 with -O2 needs this
    callHostFunction((void*)idiv16, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_idivE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false); // ARM64 with -O2 needs this
    callHostFunction((void*)idiv16, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_divR32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)div32, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_divE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)div32, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_idivR32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_idivE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr16r16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofReg16(op->rm), DYN_16bit);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_DEST, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr32r32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofReg32(op->rm), DYN_32bit);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_DEST, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr16r16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofReg16(op->rm), DYN_16bit);
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->reg), DYN_16bit);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_DEST, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->reg), DYN_16bit);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr32r32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofReg32(op->rm), DYN_32bit);
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_DEST, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
