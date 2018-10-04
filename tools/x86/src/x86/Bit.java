package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/27/2018.
 */
public class Bit extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_bit.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_bit.h");

            out(fos, "#include \"../common/common_bit.h\"");
            out(fos32, "#include \"../common/common_bit.h\"");

            FileOutputStream fosOps_h = new FileOutputStream("../common/common_bit.h");
            FileOutputStream fosOps_cpp = new FileOutputStream("../common/common_bit.cpp");

            out(fosOps_cpp, "#include \"boxedwine.h\"");

            fos.write(header.getBytes());
            generateAll(fos, fos_init, fos32, fosOps_h, fosOps_cpp);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void bitR16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, String functionName, String enumName, String op, boolean imm) throws IOException {

        if (imm) {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U16 mask, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U16 mask, U32 reg) {");
        } else {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U32 maskReg, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U32 maskReg, U32 reg) {");
            out(fosOps_cpp, "    U16 mask=1 << (cpu->reg[maskReg].u16 & 15);");
        }
        out(fosOps_cpp, "    cpu->fillFlagsNoCF();");
        out(fosOps_cpp, "    cpu->setCF(cpu->reg[reg].u16 & mask);");
        if (op.length()>0) {
            out(fosOps_cpp, "    "+op);
        }
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_"+functionName+"(cpu, "+(imm?"op->imm":"op->rm")+", op->reg);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (imm) {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        if (!imm) {
            bitR16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bitE16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, String functionName, String enumName, String op, boolean imm) throws IOException {
        if (imm) {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U16 mask, U32 address, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U16 mask, U32 address, U32 reg) {");
        } else {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U32 address, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U32 address, U32 reg) {");
            out(fosOps_cpp, "    U16 mask=1 << (cpu->reg[reg].u16 & 15);");
        }
        out(fosOps_cpp, "    U16 value;");
        out(fosOps_cpp, "    cpu->fillFlagsNoCF();");
        if (!imm) {
            out(fosOps_cpp, "    address+=(((S16)cpu->reg[reg].u16)>>4)*2;");
        }
        out(fosOps_cpp, "    value = readw(address);");
        out(fosOps_cpp, "    cpu->setCF(value & mask);");
        if (op.length()>0) {
            out(fosOps_cpp, "    "+op);
        }
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_"+functionName+"(cpu, "+(imm?"op->imm, ":"")+"eaa(cpu,op), op->reg);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        if (imm) {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        if (!imm) {
            bitE16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bitR32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, String functionName, String enumName, String op, boolean imm) throws IOException {
        if (imm) {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U32 mask, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U32 mask, U32 reg) {");
        } else {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U32 maskReg, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U32 maskReg, U32 reg) {");
            out(fosOps_cpp, "    U32 mask=1 << (cpu->reg[maskReg].u32 & 31);");
        }
        out(fosOps_cpp, "    cpu->fillFlagsNoCF();");
        out(fosOps_cpp, "    cpu->setCF(cpu->reg[reg].u32 & mask);");
        if (op.length()>0) {
            out(fosOps_cpp, "    "+op);
        }
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_"+functionName+"(cpu, "+(imm?"op->imm":"op->rm")+", op->reg);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (imm) {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        if (!imm) {
            bitR32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bitE32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, String functionName, String enumName, String op, boolean imm) throws IOException {
        if (imm) {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U32 mask, U32 address, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U32 mask, U32 address, U32 reg) {");
        } else {
            out(fosOps_h, "void common_"+functionName+"(CPU* cpu, U32 address, U32 reg);");
            out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, U32 address, U32 reg) {");
            out(fosOps_cpp, "    U32 mask=1 << (cpu->reg[reg].u32 & 31);");
        }
        out(fosOps_cpp, "    U32 value;");
        out(fosOps_cpp, "    cpu->fillFlagsNoCF();");
        if (!imm) {
            out(fosOps_cpp, "    address+=(((S32)cpu->reg[reg].u32)>>5)*4;");
        }
        out(fosOps_cpp, "    value = readd(address);");
        out(fosOps_cpp, "    cpu->setCF(value & mask);");
        if (op.length()>0) {
            out(fosOps_cpp, "    "+op);
        }
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_"+functionName+"(cpu, "+(imm?"op->imm, ":"") +"eaa(cpu,op), op->reg);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        if (imm) {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        if (!imm) {
            bitE32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bsf(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, String bits, String functionName, String enumName, String load, boolean eaa) throws IOException {
        if (eaa) {
            out(fosOps_h, "void common_" + functionName + "(CPU* cpu, U32 address, U32 dstReg);");
            out(fosOps_cpp, "void common_" + functionName + "(CPU* cpu, U32 address, U32 dstReg) {");
        } else {
            out(fosOps_h, "void common_" + functionName + "(CPU* cpu, U32 srcReg, U32 dstReg);");
            out(fosOps_cpp, "void common_" + functionName + "(CPU* cpu, U32 srcReg, U32 dstReg) {");
        }
        out(fosOps_cpp, "    U"+bits+" value="+load+";");
        out(fosOps_cpp, "    cpu->fillFlagsNoZF();");
        out(fosOps_cpp, "    if (value==0) {");
        out(fosOps_cpp, "        cpu->addZF();");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        U"+bits+" result = 0;");
        out(fosOps_cpp, "        while ((value & 0x01)==0) { result++; value>>=1; }");
        out(fosOps_cpp, "        cpu->removeZF();");
        out(fosOps_cpp, "        cpu->reg[dstReg].u"+bits+"=result;");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (eaa) {
            out(fos, "    common_"+functionName+"(cpu, eaa(cpu, op), op->reg);");
        } else {
            out(fos, "    common_"+functionName+"(cpu, op->rm, op->reg);");
        }
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (eaa) {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void bsr(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, String bits, String functionName, String enumName, String load, boolean eaa) throws IOException {
        if (eaa) {
            out(fosOps_h, "void common_" + functionName + "(CPU* cpu, U32 address, U32 dstReg);");
            out(fosOps_cpp, "void common_" + functionName + "(CPU* cpu, U32 address, U32 dstReg) {");
        } else {
            out(fosOps_h, "void common_" + functionName + "(CPU* cpu, U32 srcReg, U32 dstReg);");
            out(fosOps_cpp, "void common_" + functionName + "(CPU* cpu, U32 srcReg, U32 dstReg) {");
        }
        out(fosOps_cpp, "    U"+bits+" value="+load+";");
        out(fosOps_cpp, "    cpu->fillFlagsNoZF();");
        out(fosOps_cpp, "    if (value==0) {");
        out(fosOps_cpp, "        cpu->addZF();");
        out(fosOps_cpp, "    } else {");
        if (bits.equals("16")) {
            out(fosOps_cpp, "        U16 result = 15;");
            out(fosOps_cpp, "        while ((value & 0x8000)==0) { result--; value<<=1; }");
        } else {
            out(fosOps_cpp, "        U32 result = 31;");
            out(fosOps_cpp, "        while ((value & 0x80000000)==0) { result--; value<<=1; }");
        }
        out(fosOps_cpp, "        cpu->removeZF();");
        out(fosOps_cpp, "        cpu->reg[dstReg].u"+bits+"=result;");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (eaa) {
            out(fos, "    common_"+functionName+"(cpu, eaa(cpu, op), op->reg);");
        } else {
            out(fos, "    common_"+functionName+"(cpu, op->rm, op->reg);");
        }
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (eaa) {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp) throws IOException {
        bitR16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btr16r16", "BtR16R16","", false);
        bitE16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "bte16r16", "BtE16R16","", false);
        bitR32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btr32r32", "BtR32R32","", false);
        bitE32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "bte32r32", "BtE32R32","", false);

        bitR16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btsr16r16", "BtsR16R16","cpu->reg[reg].u16 |= mask;", false);
        bitE16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btse16r16", "BtsE16R16","writew(address, value | mask);", false);
        bitR32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btsr32r32", "BtsR32R32","cpu->reg[reg].u32 |= mask;", false);
        bitE32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btse32r32", "BtsE32R32","writed(address, value | mask);", false);

        bitR16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btrr16r16", "BtrR16R16","cpu->reg[reg].u16 &= ~mask;", false);
        bitE16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btre16r16", "BtrE16R16","writew(address, value & ~mask);", false);
        bitR32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btrr32r32", "BtrR32R32","cpu->reg[reg].u32 &= ~mask;", false);
        bitE32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btre32r32", "BtrE32R32","writed(address, value & ~mask);", false);

        bitR16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btcr16r16", "BtcR16R16","cpu->reg[reg].u16 ^= mask;", false);
        bitE16(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btce16r16", "BtcE16R16","writew(address, value ^ mask);", false);
        bitR32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btcr32r32", "BtcR32R32","cpu->reg[reg].u32 ^= mask;", false);
        bitE32(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "btce32r32", "BtcE32R32","writed(address, value ^ mask);", false);

        bsf(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "16", "bsfr16r16", "BsfR16R16", "cpu->reg[srcReg].u16", false);
        bsf(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "16", "bsfr16e16", "BsfR16E16", "readw(address)", true);
        bsf(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "32", "bsfr32r32", "BsfR32R32", "cpu->reg[srcReg].u32", false);
        bsf(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "32", "bsfr32e32", "BsfR32E32", "readd(address)", true);

        bsr(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "16", "bsrr16r16", "BsrR16R16", "cpu->reg[srcReg].u16", false);
        bsr(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "16", "bsrr16e16", "BsrR16E16", "readw(address)", true);
        bsr(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "32", "bsrr32r32", "BsrR32R32", "cpu->reg[srcReg].u32", false);
        bsr(fos, fos_init, fos32, fosOps_h, fosOps_cpp, "32", "bsrr32e32", "BsrR32E32", "readd(address)", true);
    }
}
