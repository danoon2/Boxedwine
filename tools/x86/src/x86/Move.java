package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/30/2018.
 */
public class Move extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_move.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_mov.h");
            fos.write(header.getBytes());
            generateAll(fos, fos_init, fos32);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void mov(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String assign, FileOutputStream fos32, String x32) throws IOException {
        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    "+assign+";");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void OPCALL dynamic_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    "+x32);
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void movs(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String assign, FileOutputStream fos32, String x32) throws IOException {
        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    "+assign+";");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        out(fos32, "void OPCALL dynamic_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    "+x32+"");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        mov(fos, fos_init, "movr8r8", "MovR8R8", "*cpu->reg8[op->reg] = *cpu->reg8[op->rm]", fos32, "movToCpuFromCpu(OFFSET_REG8(op->reg), OFFSET_REG8(op->rm), DYN_8bit, DYN_ANY, true);");
        mov(fos, fos_init, "move8r8", "MovE8R8", "writeb(eaa(cpu, op), *cpu->reg8[op->reg])", fos32, "calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->reg), DYN_8bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);");
        mov(fos, fos_init, "movr8e8", "MovR8E8", "*cpu->reg8[op->reg] = readb(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(OFFSET_REG8(op->reg), DYN_8bit, DYN_ADDRESS, true, true);");
        mov(fos, fos_init, "movr8", "MovR8I8", "*cpu->reg8[op->reg] = (U8)op->imm;", fos32, "movToCpu(OFFSET_REG8(op->reg), DYN_8bit, op->imm);");
        mov(fos, fos_init, "move8", "MovE8I8", "writeb(eaa(cpu, op), (U8)op->imm)", fos32, "calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_8bit, op->imm, true);");

        mov(fos, fos_init, "movr16r16", "MovR16R16", "cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16", fos32, "movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_ANY, true);");
        mov(fos, fos_init, "move16r16", "MovE16R16", "writew(eaa(cpu, op), cpu->reg[op->reg].u16)", fos32, "calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);");
        mov(fos, fos_init, "movr16e16", "MovR16E16", "cpu->reg[op->reg].u16 = readw(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);");
        mov(fos, fos_init, "movr16", "MovR16I16", "cpu->reg[op->reg].u16 = (U16)op->imm", fos32, "movToCpu(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, op->imm);");
        mov(fos, fos_init, "move16", "MovE16I16", "writew(eaa(cpu, op), (U16)op->imm)", fos32, "calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_16bit, op->imm, true);");

        mov(fos, fos_init, "movr32r32", "MovR32R32", "cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32", fos32, "movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_ANY, true);");
        mov(fos, fos_init, "move32r32", "MovE32R32", "writed(eaa(cpu, op), cpu->reg[op->reg].u32)", fos32, "calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);");
        mov(fos, fos_init, "movr32e32", "MovR32E32", "cpu->reg[op->reg].u32 = readd(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);");
        mov(fos, fos_init, "movr32", "MovR32I32", "cpu->reg[op->reg].u32 = op->imm", fos32, "movToCpu(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, op->imm);");
        mov(fos, fos_init, "move32", "MovE32I32", "writed(eaa(cpu, op), op->imm)", fos32, "calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, true);");

        mov(fos, fos_init, "movr16s16", "MovR16S16", "cpu->reg[op->reg].u16 = cpu->seg[op->rm].value;", fos32, "movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(seg[op->rm].value), DYN_32bit);\r\n    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false);\r\n    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_SRC, DYN_16bit, true);");
        mov(fos, fos_init, "movr32s16", "MovR32S16", "cpu->reg[op->reg].u32 = cpu->seg[op->rm].value", fos32, "movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(seg[op->rm].value), DYN_32bit, DYN_ANY, true);");
        mov(fos, fos_init, "move16s16", "MovE16S16", "writew(eaa(cpu, op), cpu->seg[op->reg].value)", fos32, "calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(seg[op->reg].value), DYN_32bit); movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);");
        movs(fos, fos_init, "movs16e16", "MovS16E16", "if (cpu->setSegment(op->reg, readw(eaa(cpu, op)))) {\r\n        NEXT();\r\n    } else {\r\n        NEXT_DONE();\r\n    }", fos32, "calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS, true);\r\n    callHostFunction(blockDone, common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);");
        movs(fos, fos_init, "movs16r16", "MovS16R16", "if (cpu->setSegment(op->rm, cpu->reg[op->reg].u16)) {\r\n        NEXT();\r\n    } else {\r\n        NEXT_DONE();\r\n    }", fos32, "callHostFunction(blockDone, common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, CPU_OFFSET_OF(reg[op->reg].u16), DYN_PARAM_CPU_ADDRESS_16, false);");

        mov(fos, fos_init, "movAlOb", "MovAlOb", "AL = readb(cpu->seg[op->base].address+op->disp)", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(seg[op->base].address), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);\r\n    movFromMem(DYN_8bit, DYN_DEST, true);\r\n    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u8), DYN_CALL_RESULT, DYN_8bit, true);");
        mov(fos, fos_init, "movAxOw", "MovAxOw", "AX = readw(cpu->seg[op->base].address+op->disp)", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(seg[op->base].address), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);\r\n    movFromMem(DYN_16bit, DYN_DEST, true);\r\n    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u16), DYN_CALL_RESULT, DYN_16bit, true);");
        mov(fos, fos_init, "movEaxOd", "MovEaxOd", "EAX = readd(cpu->seg[op->base].address+op->disp)", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(seg[op->base].address), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);\r\n    movFromMem(DYN_32bit, DYN_DEST, true);\r\n    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u32), DYN_CALL_RESULT, DYN_32bit, true);");

        mov(fos, fos_init, "movObAl", "MovObAl", "writeb(cpu->seg[op->base].address+op->disp, AL)", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(seg[op->base].address), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);\r\n    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u8), DYN_8bit);\r\n    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_8bit, true, true);");
        mov(fos, fos_init, "movOwAx", "MovOwAx", "writew(cpu->seg[op->base].address+op->disp, AX)", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(seg[op->base].address), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);\r\n    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u16), DYN_16bit);\r\n    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_16bit, true, true);");
        mov(fos, fos_init, "movOdEax", "MovOdEax", "writed(cpu->seg[op->base].address+op->disp, EAX)", fos32, "movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(seg[op->base].address), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);\r\n    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u32), DYN_32bit);\r\n    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_32bit, true, true);");

        mov(fos, fos_init, "movGwXzR8", "MovGwXzR8", "cpu->reg[op->reg].u16 = *cpu->reg8[op->rm]", fos32, "movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_SRC, DYN_16bit, true);");
        mov(fos, fos_init, "movGwXzE8", "MovGwXzE8", "cpu->reg[op->reg].u16 = readb(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_CALL_RESULT, DYN_16bit, true);");
        mov(fos, fos_init, "movGwSxR8", "MovGwSxR8", "cpu->reg[op->reg].u16 = (S8)*cpu->reg8[op->rm]", fos32, "movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromRegSignExtend(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_SRC, DYN_16bit, true);");
        mov(fos, fos_init, "movGwSxE8", "MovGwSxE8", "cpu->reg[op->reg].u16 = (S8)readb(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_CALL_RESULT, DYN_16bit, true);");

        mov(fos, fos_init, "movGdXzR8", "MovGdXzR8", "cpu->reg[op->reg].u32 = *cpu->reg8[op->rm]", fos32, "movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_SRC, DYN_32bit, true);");
        mov(fos, fos_init, "movGdXzE8", "MovGdXzE8", "cpu->reg[op->reg].u32 = readb(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_CALL_RESULT, DYN_32bit, true);");
        mov(fos, fos_init, "movGdSxR8", "MovGdSxR8", "cpu->reg[op->reg].u32 = (S8)(*cpu->reg8[op->rm])", fos32, "movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_SRC, DYN_32bit, true);");
        mov(fos, fos_init, "movGdSxE8", "MovGdSxE8", "cpu->reg[op->reg].u32 = (S8)readb(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_CALL_RESULT, DYN_32bit, true);");

        mov(fos, fos_init, "movGdXzR16", "MovGdXzR16", "cpu->reg[op->reg].u32 = cpu->reg[op->rm].u16", fos32, "movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit); movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_SRC, DYN_32bit, true);");
        mov(fos, fos_init, "movGdXzE16", "MovGdXzE16", "cpu->reg[op->reg].u32 = readw(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);  movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_CALL_RESULT, DYN_32bit, true);");
        mov(fos, fos_init, "movGdSxR16", "MovGdSxR16", "cpu->reg[op->reg].u32 = (S16)cpu->reg[op->rm].u16", fos32, "movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit); movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_SRC, DYN_32bit, true);");
        mov(fos, fos_init, "movGdSxE16", "MovGdSxE16", "cpu->reg[op->reg].u32 = (S16)readw(eaa(cpu, op))", fos32, "calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);  movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_CALL_RESULT, DYN_32bit, true);");

        mov(fos, fos_init, "leaR16", "LeaR16", "cpu->reg[op->reg].u16 = eaa(cpu, op)", fos32, "calculateEaa(op, DYN_ADDRESS); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_ADDRESS, DYN_16bit, true);");
        mov(fos, fos_init, "leaR32", "LeaR32", "cpu->reg[op->reg].u32 = eaa(cpu, op)", fos32, "calculateEaa(op, DYN_ADDRESS); movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_ADDRESS, DYN_32bit, true);");
    }
}
