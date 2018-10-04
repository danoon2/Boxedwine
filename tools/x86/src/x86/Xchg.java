package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/27/2018.
 */
public class Xchg extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_xchg.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_xchg.h");

            out(fos, "#include \"../common/common_xchg.h\"");
            out(fos32, "#include \"../common/common_xchg.h\"");

            FileOutputStream fosOps_h = new FileOutputStream("../common/common_xchg.h");
            FileOutputStream fosOps_cpp = new FileOutputStream("../common/common_xchg.cpp");

            out(fosOps_cpp, "#include \"boxedwine.h\"");

            fos.write(header.getBytes());
            generateAll(fos, fos_init, fosOps_h, fosOps_cpp, fos32);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void xchg8(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String functionName, String enumName, boolean reg) throws IOException {
        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (reg) {
            out(fos, "    U8 tmp = *cpu->reg8[op->rm];");
            out(fos, "    *cpu->reg8[op->rm] = *cpu->reg8[op->reg];");
        } else {
            out(fos, "    U32 address = eaa(cpu, op);");
            out(fos, "    U8 tmp = readb(address);");
            out(fos, "    writeb(address, *cpu->reg8[op->reg]);");
        }
        out(fos, "    *cpu->reg8[op->reg] = tmp;");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (reg) {
            out(fos32, "    movToRegFromCpu(DYN_DEST, OFFSET_REG8(op->rm), DYN_8bit);");
            out(fos32, "    movToCpuFromCpu(OFFSET_REG8(op->rm), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC, true);");
            out(fos32, "    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);");
        } else {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            out(fos32, "    movFromMem(DYN_8bit, DYN_ADDRESS, false);");
            out(fos32, "    movToRegFromCpu(DYN_DEST, OFFSET_REG8(op->reg), DYN_8bit);");
            out(fos32, "    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true);");
            out(fos32, "    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_CALL_RESULT, DYN_8bit, true);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void xchg16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String functionName, String enumName, boolean reg) throws IOException {
        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (reg) {
            out(fos, "    U16 tmp = cpu->reg[op->rm].u16;");
            out(fos, "    cpu->reg[op->rm].u16 = cpu->reg[op->reg].u16;");
        } else {
            out(fos, "    U32 address = eaa(cpu, op);");
            out(fos, "    U16 tmp = readw(address);");
            out(fos, "    writew(address, cpu->reg[op->reg].u16);");
        }
        out(fos, "    cpu->reg[op->reg].u16 = tmp;");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (reg) {
            out(fos32, "    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit);");
            out(fos32, "    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->rm].u16), CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_SRC, true);");
            out(fos32, "    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);");
        } else {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            out(fos32, "    movFromMem(DYN_16bit, DYN_ADDRESS, false);");
            out(fos32, "    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);");
            out(fos32, "    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true);");
            out(fos32, "    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_CALL_RESULT, DYN_16bit, true);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void xchg32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String functionName, String enumName, boolean reg) throws IOException {
        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (reg) {
            out(fos, "    U32 tmp = cpu->reg[op->rm].u32;");
            out(fos, "    cpu->reg[op->rm].u32 = cpu->reg[op->reg].u32;");
        } else {
            out(fos, "    U32 address = eaa(cpu, op);");
            out(fos, "    U32 tmp = readd(address);");
            out(fos, "    writed(address, cpu->reg[op->reg].u32);");
        }
        out(fos, "    cpu->reg[op->reg].u32 = tmp;");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (reg) {
            out(fos32, "    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit);");
            out(fos32, "    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->rm].u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_SRC, true);");
            out(fos32, "    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);");
        } else {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            out(fos32, "    movFromMem(DYN_32bit, DYN_ADDRESS, false);");
            out(fos32, "    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);");
            out(fos32, "    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_CALL_RESULT, DYN_32bit, true);");
            out(fos32, "    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void cmpxchg16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, FileOutputStream fos32, String functionName, String enumName, boolean address) throws IOException {
        out(fosOps_h, "void common_"+functionName+"(CPU* cpu, "+(address?"U32 address":"U32 dstReg")+", U32 srcReg);");
        out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, "+(address?"U32 address":"U32 dstReg")+", U32 srcReg){");
        out(fosOps_cpp, "    cpu->dst.u16 = AX;");
        out(fosOps_cpp, "    cpu->src.u16 = "+(address?"readw(address)":"cpu->reg[dstReg].u16")+";");
        out(fosOps_cpp, "    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;");
        out(fosOps_cpp, "    cpu->lazyFlags = FLAGS_CMP16;");
        out(fosOps_cpp, "    if (AX == cpu->src.u16) {");
        if (address)
            out(fosOps_cpp, "        writew(address, cpu->reg[srcReg].u16);");
        else
            out(fosOps_cpp, "        cpu->reg[dstReg].u16 = cpu->reg[srcReg].u16;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        AX = cpu->src.u16;");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (address)
            out(fos, "    common_"+functionName+"(cpu, eaa(cpu, op), op->reg);");
        else
            out(fos, "    common_"+functionName+"(cpu, op->reg, op->rm);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (address) {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void cmpxchg32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp,  FileOutputStream fos32, String functionName, String enumName, boolean address) throws IOException {
        out(fosOps_h, "void common_"+functionName+"(CPU* cpu, "+(address?"U32 address":"U32 dstReg")+", U32 srcReg);");
        out(fosOps_cpp, "void common_"+functionName+"(CPU* cpu, "+(address?"U32 address":"U32 dstReg")+", U32 srcReg){");
        out(fosOps_cpp, "    cpu->dst.u32 = EAX;");
        out(fosOps_cpp, "    cpu->src.u32 = "+(address?"readd(address)":"cpu->reg[dstReg].u32")+";");
        out(fosOps_cpp, "    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;");
        out(fosOps_cpp, "    cpu->lazyFlags = FLAGS_CMP32;");
        out(fosOps_cpp, "    if (EAX == cpu->src.u32) {");
        if (address)
            out(fosOps_cpp, "        writed(address, cpu->reg[srcReg].u32);");
        else
            out(fosOps_cpp, "        cpu->reg[dstReg].u32 = cpu->reg[srcReg].u32;");
        out(fosOps_cpp, "    } else {");
        out(fosOps_cpp, "        EAX = cpu->src.u32;");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (address)
            out(fos, "    common_"+functionName+"(cpu, eaa(cpu, op), op->reg);");
        else
            out(fos, "    common_"+functionName+"(cpu, op->reg, op->rm);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void dynamic_"+functionName+"(DynamicData* data, DecodedOp* op) {");
        if (address) {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);");
        } else {
            out(fos32, "    callHostFunction(common_" + functionName + ", false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);");
        }
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp,  FileOutputStream fos32) throws IOException {
        xchg8(fos, fos_init, fos32, "xchgr8r8", "XchgR8R8", true);
        xchg8(fos, fos_init, fos32, "xchge8r8", "XchgE8R8", false);
        xchg16(fos, fos_init, fos32, "xchgr16r16", "XchgR16R16", true);
        xchg16(fos, fos_init, fos32, "xchge16r16", "XchgE16R16", false);
        xchg32(fos, fos_init, fos32, "xchgr32r32", "XchgR32R32", true);
        xchg32(fos, fos_init, fos32, "xchge32r32", "XchgE32R32", false);

        cmpxchg16(fos, fos_init, fosOps_h, fosOps_cpp, fos32, "cmpxchgr16r16", "CmpXchgR16R16", false);
        cmpxchg16(fos, fos_init, fosOps_h, fosOps_cpp, fos32, "cmpxchge16r16", "CmpXchgE16R16", true);
        cmpxchg32(fos, fos_init, fosOps_h, fosOps_cpp, fos32, "cmpxchgr32r32", "CmpXchgR32R32", false);
        cmpxchg32(fos, fos_init, fosOps_h, fosOps_cpp, fos32, "cmpxchge32r32", "CmpXchgE32R32", true);
    }
}
