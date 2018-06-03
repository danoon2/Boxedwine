package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class Shift extends Base {

    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos_c = new FileOutputStream("normal_shift.cpp");
            FileOutputStream fos_h = new FileOutputStream("normal_shift.h");
            FileOutputStream fos_op = new FileOutputStream("normal_shift_op.h");

            fos_c.write(header.getBytes());
            fos_h.write(header.getBytes());
            fos_op.write(header.getBytes());
            out(fos_c, "#include \"boxedwine.h\"");
            out(fos_h, "#ifndef __SHIFT_OP_H__");
            out(fos_h, "#define __SHIFT_OP_H__");
            out(fos_h, "#include \"../common/cpu.h\"");
            out(fos_op, "#include \"normal_shift.h\"");
            
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "rol8", "op->imm", "1", "3", rol8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "rol8cl", "CL & 0x1f", "4", "4", rol8, 7, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "rol16", "op->imm", "1", "3", rol16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "rol16cl", "CL & 0x1f", "4", "4", rol16, 0xf, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "rol32", "op->imm", "1", "3", rol32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "rol32cl", "CL & 0x1f", "4", "4", rol32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, "ror8", "op->imm", "1", "3", ror8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "ror8cl", "CL & 0x1f", "4", "4", ror8, 7, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "ror16", "op->imm", "1", "3", ror16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "ror16cl", "CL & 0x1f", "4", "4", ror16, 0xf, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "ror32", "op->imm", "1", "3", ror32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "ror32cl", "CL & 0x1f", "4", "4", ror32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, "rcl8", "op->imm", "8", "10", rcl8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "rcl8cl", "CL & 0x1f", "7", "9", rcl8, 0, 9, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "rcl16", "op->imm", "8", "10", rcl16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "rcl16cl", "CL & 0x1f", "7", "9", rcl16, 0, 17, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "rcl32", "op->imm", "8", "10", rcl32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "rcl32cl", "CL & 0x1f", "7", "9", rcl32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, "rcr8", "op->imm", "8", "10", rcr8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "rcr8cl", "CL & 0x1f", "7", "9", rcr8, 0, 9, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "rcr16", "op->imm", "8", "10", rcr16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "rcr16cl", "CL & 0x1f", "7", "9", rcr16, 0, 17, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "rcr32", "op->imm", "8", "10", rcr32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "rcr32cl", "CL & 0x1f", "7", "9", rcr32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, "shl8", "op->imm", "1", "3", shl8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "shl8cl", "CL & 0x1f", "4", "4", shl8, 0, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "shl16", "op->imm", "1", "3", shl16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "shl16cl", "CL & 0x1f", "4", "4", shl16, 0, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "shl32", "op->imm", "1", "3", shl32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "shl32cl", "CL & 0x1f", "4", "4", shl32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, "shr8", "op->imm", "1", "3", shr8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "shr8cl", "CL & 0x1f", "4", "4", shr8, 0, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "shr16", "op->imm", "1", "3", shr16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "shr16cl", "CL & 0x1f", "4", "4", shr16, 0, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "shr32", "op->imm", "1", "3", shr32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "shr32cl", "CL & 0x1f", "4", "4", shr32, 0, 0, true);

            shiftInst8(fos_c, fos_h, fos_op, fos_init, "sar8", "op->imm", "1", "3", sar8, 0, 0, false);
            shiftInst8(fos_c, fos_h, fos_op, fos_init, "sar8cl", "CL & 0x1f", "4", "4", sar8, 0, 0, true);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "sar16", "op->imm", "1", "3", sar16, 0, 0, false);
            shiftInst16(fos_c, fos_h, fos_op, fos_init, "sar16cl", "CL & 0x1f", "4", "4", sar16, 0, 0, true);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "sar32", "op->imm", "1", "3", sar32, 0, 0, false);
            shiftInst32(fos_c, fos_h, fos_op, fos_init, "sar32cl", "CL & 0x1f", "4", "4", sar32, 0, 0, true);

            dshift(fos_op, fos_init, "dshlr16r16", "normal_dshlr16r16", "DshlR16R16", "op->reg, op->rm, op->imm");
            dshift(fos_op, fos_init, "dshle16r16", "normal_dshle16r16", "DshlE16R16", "op->reg, eaa(cpu, op), op->imm");
            dshift(fos_op, fos_init, "dshlr32r32", "normal_dshlr32r32", "DshlR32R32", "op->reg, op->rm, op->imm");
            dshift(fos_op, fos_init, "dshle32r32", "normal_dshle32r32", "DshlE32R32", "op->reg, eaa(cpu, op), op->imm");
            dshift(fos_op, fos_init, "dshlclr16r16", "normal_dshlclr16r16", "DshlClR16R16", "op->reg, op->rm");
            dshift(fos_op, fos_init, "dshlcle16r16", "normal_dshlcle16r16", "DshlClE16R16", "op->reg, eaa(cpu, op)");
            dshift(fos_op, fos_init, "dshlclr32r32", "normal_dshlclr32r32", "DshlClR32R32", "op->reg, op->rm");
            dshift(fos_op, fos_init, "dshlcle32r32", "normal_dshlcle32r32", "DshlClE32R32", "op->reg, eaa(cpu, op)");

            dshift(fos_op, fos_init, "dshrr16r16", "normal_dshrr16r16", "DshrR16R16", "op->reg, op->rm, op->imm");
            dshift(fos_op, fos_init, "dshre16r16", "normal_dshre16r16", "DshrE16R16", "op->reg, eaa(cpu, op), op->imm");
            dshift(fos_op, fos_init, "dshrr32r32", "normal_dshrr32r32", "DshrR32R32", "op->reg, op->rm, op->imm");
            dshift(fos_op, fos_init, "dshre32r32", "normal_dshre32r32", "DshrE32R32", "op->reg, eaa(cpu, op), op->imm");
            dshift(fos_op, fos_init, "dshrclr16r16", "normal_dshrclr16r16", "DshrClR16R16", "op->reg, op->rm");
            dshift(fos_op, fos_init, "dshrcle16r16", "normal_dshrcle16r16", "DshrClE16R16", "op->reg, eaa(cpu, op)");
            dshift(fos_op, fos_init, "dshrclr32r32", "normal_dshrclr32r32", "DshrClR32R32", "op->reg, op->rm");
            dshift(fos_op, fos_init, "dshrcle32r32", "normal_dshrcle32r32", "DshrClE32R32", "op->reg, eaa(cpu, op)");

            fos_c.close();
            out(fos_h, "#endif");
            fos_h.close();
            fos_op.close();
        } catch (IOException e) {

        }
    }

    public void dshift(FileOutputStream fos, FileOutputStream fos_init, String func, String name, String ename, String source) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    "+func+"(cpu, "+source+");");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");
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

    public void shiftInst8(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String source, String rcycles, String mcycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        shiftBase(fos, fos_h, fos_op, fos_init, name+"_reg", false, "8", source, "*cpu->reg8[reg]", "*cpu->reg8[reg] = ", "", rcycles, inst, mask, mod, checkForZero);
        shiftBase(fos, fos_h, fos_op, fos_init, name+"_mem", true, "8", source, "readb(eaa)", "writeb(eaa, ", ")", mcycles, inst, mask, mod, checkForZero);
    }

    public void shiftInst16(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String source, String rcycles, String mcycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        shiftBase(fos, fos_h, fos_op, fos_init, name+"_reg", false, "16", source, "cpu->reg[reg].u16", "cpu->reg[reg].u16 = ", "", rcycles, inst, mask, mod, checkForZero);
        shiftBase(fos, fos_h, fos_op, fos_init, name+"_mem", true, "16", source, "readw(eaa)", "writew(eaa, ", ")", mcycles, inst, mask, mod, checkForZero);
    }

    public void shiftInst32(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String source, String rcycles, String mcycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        shiftBase(fos, fos_h, fos_op, fos_init, name+"_reg", false, "32", source, "cpu->reg[reg].u32", "cpu->reg[reg].u32 = ", "", rcycles, inst, mask, mod, checkForZero);
        shiftBase(fos, fos_h, fos_op, fos_init, name+"_mem", true, "32", source, "readd(eaa)", "writed(eaa, ", ")", mcycles, inst, mask, mod, checkForZero);
    }

    public void shiftBase(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, boolean eaa, String bits, String shiftSource, String source, String destSave1, String destSave2, String cycles, String inst, int mask, int mod, boolean checkForZero) throws IOException {
        if (fos_h!=null) {
            if (eaa)
                out(fos_h, "void " + name + "(CPU* cpu, U32 eaa, U32 var2);");
            else
                out(fos_h, "void " + name + "(CPU* cpu, U32 reg, U32 var2);");
        }
        if (fos_op!=null) {
            out(fos_op, "void OPCALL " + name + "_op(CPU* cpu, DecodedOp* op) {");
            out(fos_op, "    START_OP(cpu, op);");
            if (eaa)
                out(fos_op, "    " + name + "(cpu, eaa(cpu, op), " + shiftSource + ");");
            else
                out(fos_op, "    " + name + "(cpu, op->reg, " + shiftSource + ");");
            out(fos_op, "    NEXT();");
            out(fos_op, "}");
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
