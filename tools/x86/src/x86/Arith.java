package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class Arith extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_arith.h");
            out(fos, "#include \"../common/common_arith.h\"");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_arith.h");
            FileOutputStream fosOps_h = new FileOutputStream("../common/common_arith.h");
            FileOutputStream fosOps_cpp = new FileOutputStream("../common/common_arith.cpp");
            out(fosOps_cpp, "#include \"boxedwine.h\"");
            fos.write(header.getBytes());
            generateAll(fos, fos_init, fosOps_cpp, fosOps_h, fos32);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fosOps_cpp, FileOutputStream fosOps_h, FileOutputStream fos32) throws IOException {
        arith(fos, fos_init, fos32, "add", "+", false, true, true, true, true);
        arith(fos, fos_init, fos32, "or", "|", false, true, true, true, true);
        arith(fos, fos_init, fos32, "adc", "+", true, true, true, true, true);
        arith(fos, fos_init, fos32, "sbb", "-", true, true, true, true, true);
        arith(fos, fos_init, fos32, "and", "&", false, true, true, true, true);
        arith(fos, fos_init, fos32, "sub", "-", false, true, true, true, true);
        arith(fos, fos_init, fos32, "xor", "^", false, true, true, true, true);
        arith(fos, fos_init, fos32, "cmp", "-", false, false, true, true, true);
        arith(fos, fos_init, fos32, "test", "&", false, false, true, true, false);
        arithSingle(fos, fos_init, fos32, "not", "~", false, true, false);
        arithSingle(fos, fos_init, fos32, "neg", "-", false, true, true);

        mul8(fos, fos_init, fos32);
        mul16(fos, fos_init, fos32);
        mul32(fos, fos_init, fos32);

        div8(fos, fos_init, fos32);
        div16(fos, fos_init, fos32);
        div32(fos, fos_init, fos32);

        out(fosOps_h, "void common_dimul16(CPU* cpu, U32 arg1, U32 arg2, U32 regResult);");
        out(fosOps_cpp, "void common_dimul16(CPU* cpu, U32 arg1, U32 arg2, U32 regResult) {");
        out(fosOps_cpp, "    S32 res=(S16)arg1 * (S32)((S16)arg2);");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    if ((res >= -32767) && (res <= 32767)) {");
        out(fosOps_cpp, "        cpu->removeFlag(CF|OF);");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->addFlag(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "    cpu->reg[regResult].u16 = (U16)res;");
        out(fosOps_cpp, "}");

        out(fosOps_h, "void common_dimul32(CPU* cpu, U32 arg1, U32 arg2, U32 regResult);");
        out(fosOps_cpp, "void common_dimul32(CPU* cpu, U32 arg1, U32 arg2, U32 regResult) {");
        out(fosOps_cpp, "    S64 res=(S32)(arg1) * (S64)((S32)arg2);");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    if (res>=-2147483647l && res<=2147483647l) {");
        out(fosOps_cpp, "        cpu->removeFlag(CF|OF);");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->addFlag(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "    cpu->reg[regResult].u32 = (U32)res;");
        out(fosOps_cpp, "}");

        out(fosOps_h, "void common_imul8(CPU* cpu, U8 src);");
        out(fosOps_cpp, "void common_imul8(CPU* cpu, U8 src) {");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    AX = (S16)((S8)AL) * (S8)(src);");
        out(fosOps_cpp, "    if ((S16)AX<-128 || (S16)AX>127) {");
        out(fosOps_cpp, "        cpu->flags|=CF|OF;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->flags&=~(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fosOps_h, "void common_mul8(CPU* cpu, U8 src);");
        out(fosOps_cpp, "void common_mul8(CPU* cpu, U8 src) {");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    AX = AL * src;");
        out(fosOps_cpp, "    if (AH) {");
        out(fosOps_cpp, "        cpu->flags|=CF|OF;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->flags&=~(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fosOps_h, "void common_imul16(CPU* cpu, U16 src);");
        out(fosOps_cpp, "void common_imul16(CPU* cpu, U16 src) {");
        out(fosOps_cpp, "    S32 result = (S32)((S16)AX) * (S16)src;");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    AX = (U16)result;");
        out(fosOps_cpp, "    DX = (U16)(result >> 16);");
        out(fosOps_cpp, "    if (result>32767 || result<-32768) {");
        out(fosOps_cpp, "        cpu->flags|=CF|OF;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->flags&=~(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fosOps_h, "void common_mul16(CPU* cpu, U16 src);");
        out(fosOps_cpp, "void common_mul16(CPU* cpu, U16 src) {");
        out(fosOps_cpp, "    U32 result = (U32)AX * src;");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    AX = (U16)result;");
        out(fosOps_cpp, "    DX = (U16)(result >> 16);");
        out(fosOps_cpp, "    if (DX) {");
        out(fosOps_cpp, "        cpu->flags|=CF|OF;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->flags&=~(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fosOps_h, "void common_imul32(CPU* cpu, U32 src);");
        out(fosOps_cpp, "void common_imul32(CPU* cpu, U32 src) {");
        out(fosOps_cpp, "    S64 result = (S64)((S32)EAX) * ((S32)(src));");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    EAX = (U32)result;");
        out(fosOps_cpp, "    EDX = (U32)(result >> 32);");
        out(fosOps_cpp, "    if (result>0x7fffffffl || result<-0x7fffffffl) {");
        out(fosOps_cpp, "        cpu->flags|=CF|OF;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->flags&=~(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fosOps_h, "void common_mul32(CPU* cpu, U32 src);");
        out(fosOps_cpp, "void common_mul32(CPU* cpu, U32 src) {");
        out(fosOps_cpp, "    U64 result = (U64)EAX * src;");
        out(fosOps_cpp, "    cpu->fillFlagsNoCFOF();");
        out(fosOps_cpp, "    EAX = (U32)result;");
        out(fosOps_cpp, "    EDX = (U32)(result >> 32);");
        out(fosOps_cpp, "    if (EDX) {");
        out(fosOps_cpp, "        cpu->flags|=CF|OF;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        cpu->flags&=~(CF|OF);");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        dmul16(fos, fos_init, "dimulcr16r16", "ImulR16R16", "cpu->reg[op->rm].u16", "op->imm", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit);", "DYN_DEST", "DYN_PARAM_REG_16", "", "op->imm", "DYN_PARAM_CONST_16");
        dmul16(fos, fos_init, "dimulcr16e16", "ImulR16E16", "readw(eaa(cpu, op))", "op->imm", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_16", "", "op->imm", "DYN_PARAM_CONST_16");
        dmul32(fos, fos_init, "dimulcr32r32", "ImulR32R32", "cpu->reg[op->rm].u32", "op->imm", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit);", "DYN_DEST", "DYN_PARAM_REG_32", "", "op->imm", "DYN_PARAM_CONST_32");
        dmul32(fos, fos_init, "dimulcr32e32", "ImulR32E32", "readd(eaa(cpu, op))", "op->imm", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_32", "", "op->imm", "DYN_PARAM_CONST_32");

        dmul16(fos, fos_init, "dimulr16r16", "DimulR16R16", "cpu->reg[op->rm].u16", "cpu->reg[op->reg].u16", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit);", "DYN_DEST", "DYN_PARAM_REG_16", "movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);", "DYN_SRC", "DYN_PARAM_REG_16");
        dmul16(fos, fos_init, "dimulr16e16", "DimulR16E16", "readw(eaa(cpu, op))", "cpu->reg[op->reg].u16", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_16", "movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);", "DYN_SRC", "DYN_PARAM_REG_16");
        dmul32(fos, fos_init, "dimulr32r32", "DimulR32R32", "cpu->reg[op->rm].u32", "cpu->reg[op->reg].u32", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit);", "DYN_DEST", "DYN_PARAM_REG_32", "movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);", "DYN_SRC", "DYN_PARAM_REG_32");
        dmul32(fos, fos_init, "dimulr32e32", "DimulR32E32", "readd(eaa(cpu, op))", "cpu->reg[op->reg].u32", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_32", "movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);", "DYN_SRC", "DYN_PARAM_REG_32");
    }

    public void dmul16(FileOutputStream fos, FileOutputStream fos_init, String name, String ename, String arg1, String arg2, FileOutputStream fos32, String x32LoadArg1, String x32Arg1, String x32Arg1Type, String x32LoadArg2, String x32Arg2, String x32Arg2Type) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_dimul16(cpu, "+arg1+", "+arg2+", op->reg);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");

        out(fos32, "void dynamic_"+name+"(DynamicData* data, DecodedOp* op) {");
        out(fos32, "    "+x32LoadArg1);
        if (x32LoadArg2.length()!=0) {
            out(fos32, "    " + x32LoadArg2);
        }
        out(fos32, "    callHostFunction(common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, "+x32Arg1+", "+x32Arg1Type+", "+(x32Arg1Type.contains("REG")?"true":"false")+", "+x32Arg2+", "+x32Arg2Type+", "+(x32Arg2Type.contains("REG")?"true":"false")+", op->reg, DYN_PARAM_CONST_32, false);");
        out(fos32, "    data->currentLazyFlags=FLAGS_NONE;");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void dmul32(FileOutputStream fos, FileOutputStream fos_init, String name, String ename, String arg1, String arg2, FileOutputStream fos32, String x32LoadArg1, String x32Arg1, String x32Arg1Type, String x32LoadArg2, String x32Arg2, String x32Arg2Type) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_dimul32(cpu, "+arg1+", "+arg2+", op->reg);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");

        out(fos32, "void dynamic_"+name+"(DynamicData* data, DecodedOp* op) {");
        out(fos32, "    "+x32LoadArg1);
        if (x32LoadArg2.length()!=0) {
            out(fos32, "    " + x32LoadArg2);
        }
        out(fos32, "    callHostFunction(common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, "+x32Arg1+", "+x32Arg1Type+", "+(x32Arg1Type.contains("REG")?"true":"false")+", "+x32Arg2+", "+x32Arg2Type+", "+(x32Arg2Type.contains("REG")?"true":"false")+", op->reg, DYN_PARAM_CONST_32, false);");
        out(fos32, "    data->currentLazyFlags=FLAGS_NONE;");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void divInternal(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String func, String name, String ename, String source, String x32Load, String x32Arg, String x32ArgType) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+func+"(cpu, "+source+")) {NEXT();} else {NEXT_DONE();}");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");

        out(fos32, "void dynamic_"+name+"(DynamicData* data, DecodedOp* op) {");
        if (x32Load.length()!=0) {
            out(fos32, "    "+x32Load);
        }
        out(fos32, "    callHostFunction("+func+", true, 2, 0, DYN_PARAM_CPU, false, "+x32Arg+", "+x32ArgType+", "+(x32ArgType.contains("REG")?"true":"false")+");");
        out(fos32, "    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);");
        out(fos32, "    blockDone();");
        out(fos32, "    endIf();");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void div8(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        divInternal(fos, fos_init, fos32, "div8", "divR8", "DivR8", "*cpu->reg8[op->reg]", "", "OFFSET_REG8(op->reg)", "DYN_PARAM_CPU_ADDRESS_8");
        divInternal(fos, fos_init, fos32, "div8", "divE8", "DivE8", "readb(eaa(cpu, op))", "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_8");
        divInternal(fos, fos_init, fos32, "idiv8", "idivR8", "IDivR8", "*cpu->reg8[op->reg]", "", "OFFSET_REG8(op->reg)", "DYN_PARAM_CPU_ADDRESS_8");
        divInternal(fos, fos_init, fos32, "idiv8", "idivE8", "IDivE8", "readb(eaa(cpu, op))", "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_8");
    }

    public void div16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        divInternal(fos, fos_init, fos32, "div16", "divR16", "DivR16", "cpu->reg[op->reg].u16", "", "CPU_OFFSET_OF(reg[op->reg].u16)", "DYN_PARAM_CPU_ADDRESS_16");
        divInternal(fos, fos_init, fos32, "div16", "divE16", "DivE16", "readw(eaa(cpu, op))", "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_16");
        divInternal(fos, fos_init, fos32, "idiv16", "idivR16", "IDivR16", "cpu->reg[op->reg].u16", "", "CPU_OFFSET_OF(reg[op->reg].u16)", "DYN_PARAM_CPU_ADDRESS_16");
        divInternal(fos, fos_init, fos32, "idiv16", "idivE16", "IDivE16", "readw(eaa(cpu, op))", "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_16");
    }

    public void div32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        divInternal(fos, fos_init, fos32, "div32", "divR32", "DivR32", "cpu->reg[op->reg].u32", "", "CPU_OFFSET_OF(reg[op->reg].u32)", "DYN_PARAM_CPU_ADDRESS_32");
        divInternal(fos, fos_init, fos32, "div32", "divE32", "DivE32", "readd(eaa(cpu, op))", "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_32");
        divInternal(fos, fos_init, fos32, "idiv32", "idivR32", "IDivR32", "cpu->reg[op->reg].u32", "", "CPU_OFFSET_OF(reg[op->reg].u32)", "DYN_PARAM_CPU_ADDRESS_32");
        divInternal(fos, fos_init, fos32, "idiv32", "idivE32", "IDivE32", "readd(eaa(cpu, op))", "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_32");
    }

    public void mul8internal(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String source, boolean signed, String x32Load, String x32Arg, String x32ArgType) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (signed) {
            out(fos, "    common_imul8(cpu, "+source+");");
        } else {
            out(fos, "    common_mul8(cpu, "+source+");");
        }
        out(fos, "    NEXT();");
        out(fos, "}");

        String mixed;
        if (signed) {
            mixed = name.substring(0, 2).toUpperCase() + name.substring(2);
        } else {
            mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        }
        out(fos_init, "INIT_CPU(" + mixed + ", " + name + ")");

        out(fos32, "void dynamic_"+name+"(DynamicData* data, DecodedOp* op) {");
        if (x32Load.length()!=0) {
            out(fos32, "    "+x32Load);
        }
        out(fos32, "    callHostFunction("+(signed?"common_imul8":"common_mul8")+", false, 2, 0, DYN_PARAM_CPU, false, "+x32Arg+", "+x32ArgType+", "+(x32ArgType.contains("REG")?"true":"false")+");");
        out(fos32, "    data->currentLazyFlags=FLAGS_NONE;");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void mul8(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        mul8internal(fos, fos_init, fos32, "mulR8", "(*cpu->reg8[op->reg])", false, "", "OFFSET_REG8(op->reg)", "DYN_PARAM_CPU_ADDRESS_8");
        mul8internal(fos, fos_init, fos32, "mulE8", "readb(eaa(cpu, op))", false, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_8");
        mul8internal(fos, fos_init, fos32, "imulR8", "(*cpu->reg8[op->reg])", true, "", "OFFSET_REG8(op->reg)", "DYN_PARAM_CPU_ADDRESS_8");
        mul8internal(fos, fos_init, fos32, "imulE8", "readb(eaa(cpu, op))", true, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_8");
    }

    public void mul16internal(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String source, boolean signed, String x32Load, String x32Arg, String x32ArgType) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (signed) {
            out(fos, "    common_imul16(cpu, "+source+");");
        } else {
            out(fos, "    common_mul16(cpu, "+source+");");
        }
        out(fos, "    NEXT();");
        out(fos, "}");

        String mixed;
        if (signed) {
            mixed = name.substring(0, 2).toUpperCase() + name.substring(2);
        } else {
            mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        }
        out(fos_init, "INIT_CPU(" + mixed + ", " + name + ")");

        out(fos32, "void dynamic_"+name+"(DynamicData* data, DecodedOp* op) {");
        if (x32Load.length()!=0) {
            out(fos32, "    "+x32Load);
        }
        out(fos32, "    callHostFunction("+(signed?"common_imul16":"common_mul16")+", false, 2, 0, DYN_PARAM_CPU, false, "+x32Arg+", "+x32ArgType+", "+(x32ArgType.contains("REG")?"true":"false")+");");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void mul16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        mul16internal(fos, fos_init, fos32, "mulR16", "cpu->reg[op->reg].u16", false, "", "CPU_OFFSET_OF(reg[op->reg].u16)", "DYN_PARAM_CPU_ADDRESS_16");
        mul16internal(fos, fos_init, fos32, "mulE16", "readw(eaa(cpu, op))", false, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_16");
        mul16internal(fos, fos_init, fos32, "imulR16", "cpu->reg[op->reg].u16", true, "", "CPU_OFFSET_OF(reg[op->reg].u16)", "DYN_PARAM_CPU_ADDRESS_16");
        mul16internal(fos, fos_init, fos32, "imulE16", "readw(eaa(cpu, op))", true, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_16");
    }

    public void mul32internal(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String source, boolean signed, String x32Load, String x32Arg, String x32ArgType) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (signed) {
            out(fos, "    common_imul32(cpu, "+source+");");
        } else {
            out(fos, "    common_mul32(cpu, "+source+");");
        }
        out(fos, "    NEXT();");
        out(fos, "}");

        String mixed;
        if (signed) {
            mixed = name.substring(0, 2).toUpperCase() + name.substring(2);
        } else {
            mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        }
        out(fos_init, "INIT_CPU(" + mixed + ", " + name + ")");

        out(fos32, "void dynamic_"+name+"(DynamicData* data, DecodedOp* op) {");
        if (x32Load.length()!=0) {
            out(fos32, "    "+x32Load);
        }
        out(fos32, "    callHostFunction("+(signed?"common_imul32":"common_mul32")+", false, 2, 0, DYN_PARAM_CPU, false, "+x32Arg+", "+x32ArgType+", "+(x32ArgType.contains("REG")?"true":"false")+");");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void mul32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        mul32internal(fos, fos_init, fos32, "mulR32", "cpu->reg[op->reg].u32", false, "", "CPU_OFFSET_OF(reg[op->reg].u32)", "DYN_PARAM_CPU_ADDRESS_32");
        mul32internal(fos, fos_init, fos32, "mulE32", "readd(eaa(cpu, op))", false, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_32");
        mul32internal(fos, fos_init, fos32, "imulR32", "cpu->reg[op->reg].u32", true, "", "CPU_OFFSET_OF(reg[op->reg].u32)", "DYN_PARAM_CPU_ADDRESS_32");
        mul32internal(fos, fos_init, fos32, "imulE32", "readd(eaa(cpu, op))", true, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);", "DYN_CALL_RESULT", "DYN_PARAM_REG_32");
    }

    public void arith(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, boolean flags, boolean iform, boolean hasRE)  throws IOException {
        arith8(fos, fos_init, fos32, name, op, cf, result, "8", flags, iform, hasRE);
        arith16(fos, fos_init, fos32, name, op, cf, result, "16", flags, iform, hasRE);
        arith32(fos, fos_init, fos32, name, op, cf, result, "32", flags, iform, hasRE);
    }

    public String arithDynRR(String op, boolean result, boolean cf, boolean flags, String bits, String name) {
        return "    dynamic_arith(data, op, DYN_Reg, DYN_Reg, DYN_"+bits+"bit, '"+op+"', "+(cf?"true":"false")+", "+(result?"true":"false")+", FLAGS_"+name.toUpperCase()+bits+");";
    }

    public String arithDynER(String op, boolean result, boolean cf, boolean flags, String bits, String name) {
        return "    dynamic_arith(data, op, DYN_Reg, DYN_Mem, DYN_"+bits+"bit, '"+op+"', "+(cf?"true":"false")+", "+(result?"true":"false")+", FLAGS_"+name.toUpperCase()+bits+");";
    }

    public String arithDynRE(String op, boolean result, boolean cf, boolean flags, String bits, String name) {
        return "    dynamic_arith(data, op, DYN_Mem, DYN_Reg, DYN_"+bits+"bit, '"+op+"', "+(cf?"true":"false")+", "+(result?"true":"false")+", FLAGS_"+name.toUpperCase()+bits+");";
    }
    public String arithDynRI(String op, boolean result, boolean cf, boolean flags, String bits, String name) {
        return "    dynamic_arith(data, op, DYN_Const, DYN_Reg, DYN_"+bits+"bit, '"+op+"', "+(cf?"true":"false")+", "+(result?"true":"false")+", FLAGS_"+name.toUpperCase()+bits+");";
    }

    public String arithDynEI(String op, boolean result, boolean cf, boolean flags, String bits, String name) {
        return "    dynamic_arith(data, op, DYN_Const, DYN_Mem, DYN_"+bits+"bit, '"+op+"', "+(cf?"true":"false")+", "+(result?"true":"false")+", FLAGS_"+name.toUpperCase()+bits+");";
    }

    public void arith32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, String bits, boolean flags, boolean iform, boolean hasRE)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, fos32, mixed + "R32R32", name + "r32r32", "FLAGS_" + name.toUpperCase() + bits, "cpu->reg[op->rm].u32", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags, arithDynRR(op, result, cf, flags, bits, name), "", false, false);
        arithbase(fos, fos_init, fos32, mixed + "E32R32", name + "e32r32", "FLAGS_" + name.toUpperCase() + bits, "cpu->reg[op->reg].u32", "readd(eaa)", "writed(eaa, ", ")", op, cf, result, true, bits, flags, arithDynER(op, result, cf, flags, bits, name), "", false, false);
        if (hasRE) {
            arithbase(fos, fos_init, fos32, mixed + "R32E32", name + "r32e32", "FLAGS_" + name.toUpperCase() + bits, "readd(eaa(cpu, op))", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags, arithDynRE(op, result, cf, flags, bits, name), "", false, false);
        }
        if (iform) {
            arithbase(fos, fos_init, fos32, mixed + "R32I32", name + "32_reg", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags, arithDynRI(op, result, cf, flags, bits, name), "", false, false);
            arithbase(fos, fos_init, fos32, mixed + "E32I32", name + "32_mem", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "readd(eaa)", "writed(eaa, ", ")", op, cf, result, true, bits, flags, arithDynEI(op, result, cf, flags, bits, name), "", false, false);
        }
    }

    public void arith16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, String bits, boolean flags, boolean iform, boolean hasRE)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, fos32, mixed + "R16R16", name+"r16r16", "FLAGS_"+name.toUpperCase()+bits, "cpu->reg[op->rm].u16", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags, arithDynRR(op, result, cf, flags, bits, name), "", false, false);
        arithbase(fos, fos_init, fos32, mixed + "E16R16", name+"e16r16", "FLAGS_"+name.toUpperCase()+bits, "cpu->reg[op->reg].u16", "readw(eaa)", "writew(eaa, ", ")", op, cf, result, true, bits, flags, arithDynER(op, result, cf, flags, bits, name), "", false, false);
        if (hasRE) {
            arithbase(fos, fos_init, fos32, mixed + "R16E16", name + "r16e16", "FLAGS_" + name.toUpperCase() + bits, "readw(eaa(cpu, op))", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags, arithDynRE(op, result, cf, flags, bits, name), "", false, false);
        }
        if (iform) {
            arithbase(fos, fos_init, fos32, mixed + "R16I16", name + "16_reg", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags, arithDynRI(op, result, cf, flags, bits, name), "", false, false);
            arithbase(fos, fos_init, fos32, mixed + "E16I16", name + "16_mem", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "readw(eaa)", "writew(eaa, ", ")", op, cf, result, true, bits, flags, arithDynEI(op, result, cf, flags, bits, name),"", false, false);
        }
    }

    public void arith8(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, String bits, boolean flags, boolean iform, boolean hasRE)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, fos32, mixed + "R8R8", name+"r8r8", "FLAGS_"+name.toUpperCase()+bits, "*cpu->reg8[op->rm]", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags, arithDynRR(op, result, cf, flags, bits, name),"", false, false);
        arithbase(fos, fos_init, fos32, mixed + "E8R8", name+"e8r8", "FLAGS_"+name.toUpperCase()+bits, "*cpu->reg8[op->reg]", "readb(eaa)", "writeb(eaa, ", ")", op, cf, result, true, bits, flags, arithDynER(op, result, cf, flags, bits, name),"", false, false);
        if (hasRE) {
            arithbase(fos, fos_init, fos32, mixed + "R8E8", name + "r8e8", "FLAGS_" + name.toUpperCase() + bits, "readb(eaa(cpu, op))", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags, arithDynRE(op, result, cf, flags, bits, name), "", false, false);
        }
        if (iform) {
            arithbase(fos, fos_init, fos32, mixed + "R8I8", name + "8_reg", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags, arithDynRI(op, result, cf, flags, bits, name),"", false, false);
            arithbase(fos, fos_init, fos32, mixed + "E8I8", name + "8_mem", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "readb(eaa)", "writeb(eaa, ", ")", op, cf, result, true, bits, flags, arithDynEI(op, result, cf, flags, bits, name),"", false, false);
        }
    }

    public void arithSingle(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, boolean flags)  throws IOException {
        arithSingle8(fos, fos_init, fos32, name, op, cf, result, "8", flags);
        arithSingle16(fos, fos_init, fos32, name, op, cf, result, "16", flags);
        arithSingle32(fos, fos_init, fos32, name, op, cf, result, "32", flags);
    }

    public void arithSingle32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, String bits, boolean flags)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        String dyn;

        if (flags) {
            dyn =   "    if (!op->needsToSetFlags()) {\r\n"+
                    "        instCPU('"+op+"', CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);\r\n"+
                    "    } else {\r\n"+
                    "        movToCpuFromCpu(CPU_OFFSET_OF(src.u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_DEST, false);\r\n"+
                    "        instReg('"+op+"', DYN_DEST, DYN_32bit);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_DEST, DYN_32bit, false);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);\r\n"+
                    "        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)" + "FLAGS_"+name.toUpperCase()+bits + ");\r\n"+
                    "        data->currentLazyFlags=FLAGS_"+name.toUpperCase()+bits + ";\r\n"+
                    "    }";
        } else {
            dyn = "instCPU('"+op+"', CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);";
        }
        arithbase(fos, fos_init, fos32, mixed + "R32", name + "r32", "FLAGS_" + name.toUpperCase() + bits, flags?"0":"", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags, dyn,"", flags, true);

        if (flags) {
            dyn =   "    calculateEaa(op, DYN_ADDRESS);\r\n"+
                    "    if (!op->needsToSetFlags()) {\r\n"+
                    "        instMem('" + op + "', DYN_ADDRESS, DYN_32bit, true);\r\n"+
                    "    } else {\r\n"+
                    "        movToCpuFromMem(CPU_OFFSET_OF(src.u32), DYN_32bit, DYN_ADDRESS, false, false);\r\n"+
                    "        instReg('"+op+"', DYN_CALL_RESULT, DYN_32bit);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_CALL_RESULT, DYN_32bit, false);\r\n"+
                    "        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);\r\n"+
                    "        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)" + "FLAGS_"+name.toUpperCase()+bits + ");\r\n"+
                    "        data->currentLazyFlags=FLAGS_"+name.toUpperCase()+bits + ";\r\n"+
                    "    }";
        } else {
            dyn =   "    calculateEaa(op, DYN_ADDRESS);\r\n"+
                    "    instMem('"+op+"', DYN_ADDRESS, DYN_32bit, true);";
        }
        arithbase(fos, fos_init, fos32, mixed + "E32", name + "e32", "FLAGS_" + name.toUpperCase() + bits, flags?"0":"", "readd(eaa)", "writed(eaa, ", ")", op, cf, result, true, bits, flags, dyn,"", flags, true);
    }

    public void arithSingle16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, String bits, boolean flags)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        String dyn;

        if (flags) {
            dyn =   "    if (!op->needsToSetFlags()) {\r\n"+
                    "        instCPU('"+op+"', CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);\r\n"+
                    "    } else {\r\n"+
                    "        movToCpuFromCpu(CPU_OFFSET_OF(src.u16), CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_DEST, false);\r\n"+
                    "        instReg('"+op+"', DYN_DEST, DYN_16bit);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_DEST, DYN_16bit, false);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);\r\n"+
                    "        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)" + "FLAGS_"+name.toUpperCase()+bits + ");\r\n"+
                    "        data->currentLazyFlags=FLAGS_"+name.toUpperCase()+bits + ";\r\n"+
                    "    }";
        } else {
            dyn = "instCPU('"+op+"', CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);";
        }
        arithbase(fos, fos_init, fos32, mixed + "R16", name+"r16", "FLAGS_"+name.toUpperCase()+bits, flags?"0":"", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags, dyn, "", flags, true);

        if (flags) {
            dyn =   "    calculateEaa(op, DYN_ADDRESS);\r\n"+
                    "    if (!op->needsToSetFlags()) {\r\n"+
                    "        instMem('" + op + "', DYN_ADDRESS, DYN_16bit, true);\r\n"+
                    "    } else {\r\n"+
                    "        movToCpuFromMem(CPU_OFFSET_OF(src.u16), DYN_16bit, DYN_ADDRESS, false, false);\r\n"+
                    "        instReg('"+op+"', DYN_CALL_RESULT, DYN_16bit);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_CALL_RESULT, DYN_16bit, false);\r\n"+
                    "        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);\r\n"+
                    "        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)" + "FLAGS_"+name.toUpperCase()+bits + ");\r\n"+
                    "        data->currentLazyFlags=FLAGS_"+name.toUpperCase()+bits + ";\r\n"+
                    "    }";
        } else {
            dyn =   "    calculateEaa(op, DYN_ADDRESS);\r\n"+
                    "    instMem('"+op+"', DYN_ADDRESS, DYN_16bit, true);";
        }
        arithbase(fos, fos_init, fos32, mixed + "E16", name+"e16", "FLAGS_"+name.toUpperCase()+bits, flags?"0":"", "readw(eaa)", "writew(eaa, ", ")", op, cf, result, true, bits, flags, dyn,"", flags, true);
    }

    public void arithSingle8(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op, boolean cf, boolean result, String bits, boolean flags)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        String dyn;

        if (flags) {
            dyn =   "    if (!op->needsToSetFlags()) {\r\n"+
                    "        instCPU('"+op+"', OFFSET_REG8(op->reg), DYN_8bit);\r\n"+
                    "    } else {\r\n"+
                    "        movToCpuFromCpu(CPU_OFFSET_OF(src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST, false);\r\n"+
                    "        instReg('"+op+"', DYN_DEST, DYN_8bit);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_DEST, DYN_8bit, false);\r\n"+
                    "        movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);\r\n"+
                    "        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)" + "FLAGS_"+name.toUpperCase()+bits + ");\r\n"+
                    "        data->currentLazyFlags=FLAGS_"+name.toUpperCase()+bits + ";\r\n"+
                    "    }";
        } else {
            dyn = "instCPU('"+op+"', OFFSET_REG8(op->reg), DYN_8bit);";
        }
        arithbase(fos, fos_init, fos32, mixed + "R8", name+"r8", "FLAGS_"+name.toUpperCase()+bits, flags?"0":"", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags, dyn,"", flags, true);

        if (flags) {
            dyn =   "    calculateEaa(op, DYN_ADDRESS);\r\n"+
                    "    if (!op->needsToSetFlags()) {\r\n"+
                    "        instMem('" + op + "', DYN_ADDRESS, DYN_8bit, true);\r\n"+
                    "    } else {\r\n"+
                    "        movToCpuFromMem(CPU_OFFSET_OF(src.u8), DYN_8bit, DYN_ADDRESS, false, false);\r\n"+
                    "        instReg('"+op+"', DYN_CALL_RESULT, DYN_8bit);\r\n"+
                    "        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_CALL_RESULT, DYN_8bit, false);\r\n"+
                    "        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true);\r\n"+
                    "        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)" + "FLAGS_"+name.toUpperCase()+bits + ");\r\n"+
                    "        data->currentLazyFlags=FLAGS_"+name.toUpperCase()+bits + ";\r\n"+
                    "    }";
        } else {
            dyn =   "    calculateEaa(op, DYN_ADDRESS);\r\n"+
                    "    instMem(\'"+op+"\', DYN_ADDRESS, DYN_8bit, true);";
        }
        arithbase(fos, fos_init, fos32, mixed + "E8", name+"e8", "FLAGS_"+name.toUpperCase()+bits, flags?"0":"", "readb(eaa)", "writeb(eaa, ", ")", op, cf, result, true, bits, flags, dyn,"", flags, true);
    }

    public void arithbase(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String enumName, String functionName, String flagName, String source, String loadDest, String saveDest1, String saveDest2, String op, boolean cf, boolean result, boolean eaa, String bits, boolean flags, String x32withFlags, String x32noFlags, boolean reverse, boolean dynEip)  throws IOException {
        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (eaa)
            out(fos, "    U32 eaa = eaa(cpu, op);");
        if (cf)
            out(fos, "    cpu->oldCF = cpu->getCF();");
        if (flags) {
            out(fos, "    cpu->dst.u" + bits + " = " + (reverse?source:loadDest) + ";");
            out(fos, "    cpu->src.u" + bits + " = " + (reverse?loadDest:source) + ";");
            out(fos, "    cpu->result.u" + bits + " = cpu->dst.u" + bits + " " + op + " cpu->src.u" + bits + (cf ? " " + op + " cpu->oldCF;" : ";"));
            out(fos, "    cpu->lazyFlags = " + flagName + ";");
            if (result)
                out(fos, "    " + saveDest1 + " cpu->result.u" + bits + saveDest2 + ";");
        } else {
            if (source.length()==0)
                out(fos, "    "+saveDest1+op+loadDest+saveDest2+";");
            else
                out(fos, "    "+saveDest1+loadDest+op+source+saveDest2+";");
        }
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (result && flags && x32noFlags.length()!=0) {
            out(fos32, "    if (op->needsToSetFlags() {");
        }

        out(fos32, x32withFlags);
        if (dynEip)
            out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }
}
