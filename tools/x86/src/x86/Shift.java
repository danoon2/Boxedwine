package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class Shift extends Base {

    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos_c = new FileOutputStream("normal_shift.cpp");
            FileOutputStream fos_h = new FileOutputStream("normal_shift.h");
            FileOutputStream fos_op = new FileOutputStream("normal_shift_op.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_shift.h");

            out(fos32, "#include \"../normal/normal_shift.h\"");

            fos_c.write(header.getBytes());
            fos_h.write(header.getBytes());
            fos_op.write(header.getBytes());
            out(fos_c, "#include \"boxedwine.h\"");
            out(fos_h, "#ifndef __SHIFT_OP_H__");
            out(fos_h, "#define __SHIFT_OP_H__");
            out(fos_h, "#include \"../common/cpu.h\"");
            out(fos_op, "#include \"normal_shift.h\"");
            
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "rol8", false, "1", "3", rol8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "rol8cl", true, "4", "4", rol8, 7, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "rol16", false, "1", "3", rol16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "rol16cl", true, "4", "4", rol16, 0xf, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "rol32", false, "1", "3", rol32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "rol32cl", true, "4", "4", rol32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "ror8", false, "1", "3", ror8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "ror8cl", true, "4", "4", ror8, 7, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "ror16", false, "1", "3", ror16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "ror16cl", true, "4", "4", ror16, 0xf, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "ror32", false, "1", "3", ror32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "ror32cl", true, "4", "4", ror32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "rcl8", false, "8", "10", rcl8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "rcl8cl", true, "7", "9", rcl8, 0, 9, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "rcl16", false, "8", "10", rcl16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "rcl16cl", true, "7", "9", rcl16, 0, 17, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "rcl32", false, "8", "10", rcl32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "rcl32cl", true, "7", "9", rcl32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "rcr8", false, "8", "10", rcr8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "rcr8cl", true, "7", "9", rcr8, 0, 9, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "rcr16", false, "8", "10", rcr16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "rcr16cl", true, "7", "9", rcr16, 0, 17, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "rcr32", false, "8", "10", rcr32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "rcr32cl", true, "7", "9", rcr32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "shl8", false, "1", "3", shl8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "shl8cl", true, "4", "4", shl8, 0, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "shl16", false, "1", "3", shl16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "shl16cl", true, "4", "4", shl16, 0, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "shl32", false, "1", "3", shl32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "shl32cl", true, "4", "4", shl32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "shr8", false, "1", "3", shr8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "shr8cl", true, "4", "4", shr8, 0, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "shr16", false, "1", "3", shr16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "shr16cl", true, "4", "4", shr16, 0, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "shr32", false, "1", "3", shr32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "shr32cl", true, "4", "4", shr32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "sar8", false, "1", "3", sar8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, fos32, "sar8cl", true, "4", "4", sar8, 0, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "sar16", false, "1", "3", sar16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, fos32, "sar16cl", true, "4", "4", sar16, 0, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "sar32", false, "1", "3", sar32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, fos32, "sar32cl", true, "4", "4", sar32, 0, 0, true);

            dshift(fos_op, fos_init, fos32, "dshlr16r16", "dshlr16r16", "DshlR16R16", false, false);
            dshift(fos_op, fos_init, fos32, "dshle16r16", "dshle16r16", "DshlE16R16", true, false);
            dshift(fos_op, fos_init, fos32, "dshlr32r32", "dshlr32r32", "DshlR32R32", false, false);
            dshift(fos_op, fos_init, fos32, "dshle32r32", "dshle32r32", "DshlE32R32", true, false);
            dshift(fos_op, fos_init, fos32, "dshlclr16r16", "dshlclr16r16", "DshlClR16R16", false, true);
            dshift(fos_op, fos_init, fos32, "dshlcle16r16", "dshlcle16r16", "DshlClE16R16", true, true);
            dshift(fos_op, fos_init, fos32, "dshlclr32r32", "dshlclr32r32", "DshlClR32R32", false, true);
            dshift(fos_op, fos_init, fos32, "dshlcle32r32", "dshlcle32r32", "DshlClE32R32", true, true);

            dshift(fos_op, fos_init, fos32, "dshrr16r16", "dshrr16r16", "DshrR16R16", false, false);
            dshift(fos_op, fos_init, fos32, "dshre16r16", "dshre16r16", "DshrE16R16", true, false);
            dshift(fos_op, fos_init, fos32, "dshrr32r32", "dshrr32r32", "DshrR32R32", false, false);
            dshift(fos_op, fos_init, fos32, "dshre32r32", "dshre32r32", "DshrE32R32", true, false);
            dshift(fos_op, fos_init, fos32, "dshrclr16r16", "dshrclr16r16", "DshrClR16R16", false, true);
            dshift(fos_op, fos_init, fos32, "dshrcle16r16", "dshrcle16r16", "DshrClE16R16", true, true);
            dshift(fos_op, fos_init, fos32, "dshrclr32r32", "dshrclr32r32", "DshrClR32R32", false, true);
            dshift(fos_op, fos_init, fos32, "dshrcle32r32", "dshrcle32r32", "DshrClE32R32", true, true);

            fos_c.close();
            out(fos_h, "#endif");
            fos_h.close();
            fos_op.close();
        } catch (IOException e) {

        }
    }

    public void dshift(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String func, String name, String ename, boolean eaa, boolean cl) throws IOException {
        String source;

        if (eaa) {
            if (cl) {
                source = "op->reg, eaa(cpu, op)";
            } else {
                source = "op->reg, eaa(cpu, op), op->imm";
            }
        } else {
            if (cl) {
                source = "op->reg, op->rm";
            } else {
                source = "op->reg, op->rm, op->imm";
            }
        }
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    "+func+"(cpu, "+source+");");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");

        out(fos32, "void OPCALL dynamic_"+name+"(CPU* cpu, DecodedOp* op) {");
        if (eaa) {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            if (cl) {
                out(fos32, "    callHostFunction(NULL, " + name + ", false, false, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);");
            } else {
                out(fos32, "    callHostFunction(NULL, " + name + ", false, false, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);");
            }
        } else {
            if (cl) {
                out(fos32, "    callHostFunction(NULL, " + name + ", false, false, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);");
            } else {
                out(fos32, "    callHostFunction(NULL, " + name + ", false, false, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);");
            }
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    static public String rol8 = "cpu->fillFlagsNoCFOF();\r\n    result = (var1 << var2) | (var1 >> (8 - var2));\r\n    cpu->setCF(result & 1);\r\n    cpu->setOF((result & 1) ^ (result >> 7));";
    static public String rol16 = "cpu->fillFlagsNoCFOF();\r\n    result = (var1 << var2) | (var1 >> (16 - var2));\r\n    cpu->setCF(result & 1);\r\n    cpu->setOF((result & 1) ^ (result >> 15));";
    static public String rol32 = "cpu->fillFlagsNoCFOF();\r\n    result = (var1 << var2) | (var1 >> (32 - var2));\r\n    cpu->setCF(result & 1);\r\n    cpu->setOF((result & 1) ^ (result >> 31));";
    static public String ror8 = "cpu->fillFlagsNoCFOF();\r\n    result = (var1 >> var2) | (var1 << (8 - var2));\r\n    cpu->setCF(result & 0x80);\r\n    cpu->setOF((result ^ (result<<1)) & 0x80);";
    static public String ror16 = "cpu->fillFlagsNoCFOF();\r\n    result = (var1 >> var2) | (var1 << (16 - var2));\r\n    cpu->setCF(result & 0x8000);\r\n    cpu->setOF((result ^ (result<<1)) & 0x8000);";
    static public String ror32 = "cpu->fillFlagsNoCFOF();\r\n    result = (var1 >> var2) | (var1 << (32 - var2));\r\n    cpu->setCF(result & 0x80000000);\r\n    cpu->setOF((result ^ (result<<1)) & 0x80000000);";
    static public String rcl8 = "cpu->fillFlagsNoOF();\r\n    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));\r\n    cpu->setCF(((var1 >> (8-var2)) & 1));\r\n    cpu->setOF((cpu->flags & CF) ^ (result >> 7));";
    static public String rcl16 = "cpu->fillFlagsNoOF();\r\n    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));\r\n    cpu->setCF(((var1 >> (16-var2)) & 1));\r\n    cpu->setOF((cpu->flags & CF) ^ (result >> 15));";
    static public String rcl32 = "cpu->fillFlagsNoOF();\r\n    if (var2==1) {\r\n        result = (var1 << var2) | (cpu->flags & CF);\r\n    } else {\r\n        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));\r\n    }\r\n    cpu->setCF(((var1 >> (32-var2)) & 1));\r\n    cpu->setOF((cpu->flags & CF) ^ (result >> 31));";
    static public String rcr8 = "cpu->fillFlagsNoOF();\r\n    result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));\r\n    cpu->setCF((var1 >> (var2 - 1)) & 1);\r\n    cpu->setOF((result ^ (result<<1)) & 0x80);";
    static public String rcr16 = "cpu->fillFlagsNoOF();\r\n    result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));\r\n    cpu->setCF((var1 >> (var2 - 1)) & 1);\r\n    cpu->setOF((result ^ (result<<1)) & 0x8000);";
    static public String rcr32 = "cpu->fillFlagsNoOF();\r\n    if (var2==1) {\r\n        result = (var1 >> var2) | ((cpu->flags & CF) << 31);\r\n    } else {\r\n        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));\r\n    }\r\n    cpu->setCF((var1 >> (var2 - 1)) & 1);\r\n    cpu->setOF((result ^ (result<<1)) & 0x80000000);";
    static public String shl8 = "result = var1 << var2;\r\n    cpu->lazyFlags = FLAGS_SHL8;\r\n    cpu->result.u8 = result;\r\n    cpu->src.u8=var2;\r\n    cpu->dst.u8 = var1;";
    static public String shl16 = "result = var1 << var2;\r\n    cpu->lazyFlags = FLAGS_SHL16;\r\n    cpu->result.u16 = result;\r\n    cpu->src.u16=var2;\r\n    cpu->dst.u16 = var1;";
    static public String shl32 = "result = var1 << var2;\r\n    cpu->lazyFlags = FLAGS_SHL32;\r\n    cpu->result.u32 = result;\r\n    cpu->src.u32=var2;\r\n    cpu->dst.u32 = var1;";
    static public String shr8 = "result = var1 >> var2;\r\n    cpu->lazyFlags = FLAGS_SHR8;\r\n    cpu->result.u8 = result;\r\n    cpu->src.u8=var2;\r\n    cpu->dst.u8 = var1;";
    static public String shr16 = "result = var1 >> var2;\r\n    cpu->lazyFlags = FLAGS_SHR16;\r\n    cpu->result.u16 = result;\r\n    cpu->src.u16=var2;\r\n    cpu->dst.u16 = var1;";
    static public String shr32 = "result = var1 >> var2;\r\n    cpu->lazyFlags = FLAGS_SHR32;\r\n    cpu->result.u32 = result;\r\n    cpu->src.u32=var2;\r\n    cpu->dst.u32 = var1;";
    static public String sar8 = "result = (S8)var1 >> var2;\r\n    cpu->lazyFlags = FLAGS_SAR8;\r\n    cpu->result.u8 = result;\r\n    cpu->src.u8=var2;\r\n    cpu->dst.u8 = var1;";
    static public String sar16 = "result = (S16)var1 >> var2;\r\n    cpu->lazyFlags = FLAGS_SAR16;\r\n    cpu->result.u16 = result;\r\n    cpu->src.u16=var2;\r\n    cpu->dst.u16 = var1;";
    static public String sar32 = "result = (S32)var1 >> var2;\r\n    cpu->lazyFlags = FLAGS_SAR32;\r\n    cpu->result.u32 = result;\r\n    cpu->src.u32=var2;\r\n    cpu->dst.u32 = var1;";

    public void shiftInst8(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, FileOutputStream fos32, String name, boolean cl, String rcycles, String mcycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        shiftBase(fos, fos_h, fos_op, fos_init, fos32, name+"_reg", false, "8", cl, "*cpu->reg8[reg]", "*cpu->reg8[reg] = ", "", rcycles, inst, mask, mod, checkForZero);
        shiftBase(fos, fos_h, fos_op, fos_init, fos32, name+"_mem", true, "8", cl, "readb(eaa)", "writeb(eaa, ", ")", mcycles, inst, mask, mod, checkForZero);
    }

    public void shiftInst16(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, FileOutputStream fos32, String name, boolean cl, String rcycles, String mcycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        shiftBase(fos, fos_h, fos_op, fos_init, fos32, name+"_reg", false, "16", cl, "cpu->reg[reg].u16", "cpu->reg[reg].u16 = ", "", rcycles, inst, mask, mod, checkForZero);
        shiftBase(fos, fos_h, fos_op, fos_init, fos32, name+"_mem", true, "16", cl, "readw(eaa)", "writew(eaa, ", ")", mcycles, inst, mask, mod, checkForZero);
    }

    public void shiftInst32(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, FileOutputStream fos32, String name, boolean cl, String rcycles, String mcycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        shiftBase(fos, fos_h, fos_op, fos_init, fos32, name+"_reg", false, "32", cl, "cpu->reg[reg].u32", "cpu->reg[reg].u32 = ", "", rcycles, inst, mask, mod, checkForZero);
        shiftBase(fos, fos_h, fos_op, fos_init, fos32, name+"_mem", true, "32", cl, "readd(eaa)", "writed(eaa, ", ")", mcycles, inst, mask, mod, checkForZero);
    }

    public void shiftBase(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, FileOutputStream fos32, String name, boolean eaa, String bits, boolean useCL, String source, String destSave1, String destSave2, String cycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        if (fos_h!=null) {
            if (eaa)
                out(fos_h, "void " + name + "(CPU* cpu, U32 eaa, U32 var2);");
            else
                out(fos_h, "void " + name + "(CPU* cpu, U32 reg, U32 var2);");
        }
        if (fos_op!=null) {
            out(fos_op, "void OPCALL normal_" + name + "_op(CPU* cpu, DecodedOp* op) {");
            out(fos_op, "    START_OP(cpu, op);");
            String shiftSource = (useCL?"CL & 0x1F":"op->imm");
            if (eaa)
                out(fos_op, "    " + name + "(cpu, eaa(cpu, op), " + shiftSource + ");");
            else
                out(fos_op, "    " + name + "(cpu, op->reg, " + shiftSource + ");");
            out(fos_op, "    NEXT();");
            out(fos_op, "}");

            out(fos32, "void OPCALL dynamic_"+name+"_op(CPU* cpu, DecodedOp* op) {");
            String param;
            String paramType;

            if (useCL) {
                // reg = CL & 0x1F;
                out(fos32, "    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);");
                out(fos32, "    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);");
                out(fos32, "    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);");
                param = "DYN_SRC";
                paramType = "DYN_PARAM_REG_32";
            } else {
                // op->imm
                param = "op->imm";
                paramType = "DYN_PARAM_CONST_32";
            }
            if (eaa) {
                out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
                out(fos32, "    callHostFunction(NULL, " + name + ", false, false, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, "+param+", "+paramType+", "+(useCL?"true":"false")+");");
            } else {
                out(fos32, "    callHostFunction(NULL, " + name + ", false, false, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, "+param+", "+paramType+", "+(useCL?"true":"false")+");");
            }
            out(fos32, "    INCREMENT_EIP(op->len);");
            out(fos32, "}");
        }

        if (eaa)
            out(fos, "void " + name + "(CPU* cpu, U32 eaa, U32 var2) {");
        else
            out(fos, "void " + name + "(CPU* cpu, U32 reg, U32 var2) {");
        out(fos, "    U"+bits+" result;");
        out(fos, "    U"+bits+" var1"+((mask==0 && mod==0)?"="+source:"")+";");

        if (checkForZero) {
            out(fos, "    if (var2) {");
        }
        if (mask!=0 || mod!=0) {

            if (mask!=0)
                out(fos, "    var2&="+mask+";");
            if (mod!=0)
                out(fos, "    var2=var2 % "+mod+";");
            out(fos, "    var1 = "+source+";");
        }
        out(fos, "    "+inst);
        out(fos, "    "+destSave1+"result"+destSave2+";");
        if (checkForZero) {
            out(fos, "    }");
        }
        out(fos, "}");

        String namePart = name.substring(0, name.indexOf("_"));
        String mixed = namePart.substring(0, 1).toUpperCase() + namePart.substring(1);
        boolean cl = mixed.endsWith("cl");
        if (cl) {
            mixed = mixed.substring(0, mixed.length()-2);
        }
        boolean reg = name.contains("_reg");
        String enumName;

        if (mixed.endsWith("8")) {
            enumName = mixed.substring(0, mixed.length()-1);
            if (reg)
                enumName+="R8";
            else
                enumName+="E8";
        } else if (mixed.endsWith("16")) {
            enumName = mixed.substring(0, mixed.length()-2);
            if (reg)
                enumName+="R16";
            else
                enumName+="E16";
        } else {
            enumName = mixed.substring(0, mixed.length()-2);
            if (reg)
                enumName+="R32";
            else
                enumName+="E32";
        }
        if (cl) {
            enumName+="Cl";
        } else {
            enumName+="I8";
        }
        out(fos_init, "INIT_CPU("+enumName+", "+name+"_op)");
    }
}
