package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class PushPop extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_pushpop.h");
            out(fos, "#include \"../common/common_pushpop.h\"");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_pushpop.h");

            FileOutputStream fosOps_h = new FileOutputStream("../common/common_pushpop.h");
            FileOutputStream fosOps_cpp = new FileOutputStream("../common/common_pushpop.cpp");

            out(fosOps_cpp, "#include \"boxedwine.h\"");

            out(fosOps_h, "void common_pushA32(CPU* cpu);");
            out(fosOps_cpp, "void common_pushA32(CPU* cpu){");
            out(fosOps_cpp, "    U32 sp = ESP;");
            out(fosOps_cpp, "    cpu->push32(EAX);");
            out(fosOps_cpp, "    cpu->push32(ECX);");
            out(fosOps_cpp, "    cpu->push32(EDX);");
            out(fosOps_cpp, "    cpu->push32(EBX);");
            out(fosOps_cpp, "    cpu->push32(sp);");
            out(fosOps_cpp, "    cpu->push32(EBP);");
            out(fosOps_cpp, "    cpu->push32(ESI);");
            out(fosOps_cpp, "    cpu->push32(EDI);");
            out(fosOps_cpp, "}");

            out(fosOps_h, "void common_pushA16(CPU* cpu);");
            out(fosOps_cpp, "void common_pushA16(CPU* cpu){");
            out(fosOps_cpp, "    U16 sp = SP;");
            out(fosOps_cpp, "    cpu->push16(AX);");
            out(fosOps_cpp, "    cpu->push16(CX);");
            out(fosOps_cpp, "    cpu->push16(DX);");
            out(fosOps_cpp, "    cpu->push16(BX);");
            out(fosOps_cpp, "    cpu->push16(sp);");
            out(fosOps_cpp, "    cpu->push16(BP);");
            out(fosOps_cpp, "    cpu->push16(SI);");
            out(fosOps_cpp, "    cpu->push16(DI);");
            out(fosOps_cpp, "}");

            out(fosOps_h, "void common_popA32(CPU* cpu);");
            out(fosOps_cpp, "void common_popA32(CPU* cpu){");
            out(fosOps_cpp, "    EDI = cpu->pop32();");
            out(fosOps_cpp, "    ESI = cpu->pop32();");
            out(fosOps_cpp, "    EBP = cpu->pop32();");
            out(fosOps_cpp, "    cpu->pop32();");
            out(fosOps_cpp, "    EBX = cpu->pop32();");
            out(fosOps_cpp, "    EDX = cpu->pop32();");
            out(fosOps_cpp, "    ECX = cpu->pop32();");
            out(fosOps_cpp, "    EAX = cpu->pop32();");
            out(fosOps_cpp, "}");

            out(fosOps_h, "void common_popA16(CPU* cpu);");
            out(fosOps_cpp, "void common_popA16(CPU* cpu){");
            out(fosOps_cpp, "    DI = cpu->pop16();");
            out(fosOps_cpp, "    SI = cpu->pop16();");
            out(fosOps_cpp, "    BP = cpu->pop16();");
            out(fosOps_cpp, "    cpu->pop16();");
            out(fosOps_cpp, "    BX = cpu->pop16();");
            out(fosOps_cpp, "    DX = cpu->pop16();");
            out(fosOps_cpp, "    CX = cpu->pop16();");
            out(fosOps_cpp, "    AX = cpu->pop16();");
            out(fosOps_cpp, "}");

            fos.write(header.getBytes());
            pushPopReg(fos, fos_init, fos32, "w", "16");
            pushPopReg(fos, fos_init, fos32, "d", "32");
            pushPopSeg(fos, fos_init, fos32, "16");
            pushPopSeg(fos, fos_init, fos32, "32");
            pusha(fos, fos_init, fos32, "16");
            pusha(fos, fos_init, fos32, "32");
            popa(fos, fos_init, fos32, "16");
            popa(fos, fos_init, fos32, "32");
            pushData(fos, fos_init, fos32, "16");
            pushData(fos, fos_init, fos32, "32");
            popf(fos, fos_init, fos32, "16", "0xFFFF");
            popf(fos, fos_init, fos32, "32", "");
            pushf(fos, fos_init, fos32, "16", "");
            pushf(fos, fos_init, fos32, "32", "0xFCFFFF");
            fos.close();
        } catch (IOException e) {

        }
    }

    public void pushPopReg(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String bits) throws IOException {
        out(fos, "void OPCALL normal_pushE"+name+"_reg(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    cpu->push"+bits+"(cpu->reg[op->reg].u"+bits+");");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_popE"+name+"_reg(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    cpu->reg[op->reg].u"+bits+" = cpu->pop"+bits+"();");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_pushE"+name+"_mem(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    cpu->push"+bits+"(read"+name+"(eaa(cpu, op)));");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_popE"+name+"_mem(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    write"+name+"(eaa(cpu, op), cpu->pop"+bits+"());");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushR"+bits+", pushE"+name+"_reg)");
        out(fos_init, "INIT_CPU(PushE"+bits+", pushE"+name+"_mem)");
        out(fos_init, "INIT_CPU(PopR"+bits+", popE"+name+"_reg)");
        out(fos_init, "INIT_CPU(PopE"+bits+", popE"+name+"_mem)");

        out(fos32, "void OPCALL dynamic_pushE"+name+"_reg(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(NULL, common_push"+bits+", false, false, false, 2, 0, DYN_PARAM_CPU, false, CPU_OFFSET_OF(reg[op->reg].u"+bits+"), DYN_PARAM_CPU_ADDRESS_"+bits+", false);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void OPCALL dynamic_popE"+name+"_reg(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(NULL, common_pop"+bits+", true, false, false, 1, 0, DYN_PARAM_CPU, false);");
        out(fos32, "    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u"+bits+"), DYN_CALL_RESULT, DYN_"+bits+"bit, true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void OPCALL dynamic_pushE"+name+"_mem(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        out(fos32, "    movFromMem(DYN_"+bits+"bit, DYN_ADDRESS, true);");
        out(fos32, "    callHostFunction(NULL, common_push"+bits+", false, false, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_"+bits+", true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void OPCALL dynamic_popE"+name+"_mem(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        out(fos32, "    callHostFunction(NULL, common_pop"+bits+", true, false, false, 1, 0, DYN_PARAM_CPU, false);");
        out(fos32, "    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_"+bits+"bit, true, true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void pushPopSeg(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String bits) throws IOException {
        out(fos, "void OPCALL normal_pushSeg" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    cpu->push" + bits + "(cpu->seg[op->reg].value);");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_popSeg" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if (cpu->setSegment(op->reg, cpu->peek" + bits + "(0))) {ESP = (ESP & cpu->stackNotMask) | ((ESP + "+(bits.equals("32")?"4":"2")+" ) & cpu->stackMask);" + " NEXT();} else {NEXT_DONE();}");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushSeg" + bits + ", pushSeg" + bits + ")");
        out(fos_init, "INIT_CPU(PopSeg" + bits + ", popSeg" + bits + ")");

        out(fos32, "void OPCALL dynamic_pushSeg"+bits+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(blockDone, common_push"+bits+", false, false, false, 2, 0, DYN_PARAM_CPU, false, CPU_OFFSET_OF(seg[op->reg].value), DYN_PARAM_CPU_ADDRESS_"+bits+", false);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void OPCALL dynamic_popSeg"+bits+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(NULL, common_peek"+bits+", true, false, false, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);");
        out(fos32, "    callHostFunction(blockDone, common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_"+bits+", true);");
        // ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask);
        out(fos32, "    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackMask), DYN_32bit);");
        out(fos32, "    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);");
        out(fos32, "    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);");
        out(fos32, "    instRegImm('+', DYN_SRC, DYN_32bit, "+(bits.equals("32")?"4":"2")+");");
        out(fos32, "    instRegReg('&', DYN_SRC, DYN_DEST, DYN_32bit, true);");
        out(fos32, "    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackNotMask), DYN_32bit);");
        out(fos32, "    instRegReg('&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);");
        out(fos32, "    instRegReg('|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);");
        out(fos32, "    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void pusha(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String bits) throws IOException {
        out(fos, "void OPCALL normal_pushA" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_pushA"+bits+"(cpu);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushA" + bits + ", pushA" + bits + ")");

        out(fos32, "void OPCALL dynamic_pushA"+bits+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(NULL, common_pushA"+bits+", false, false, false, 1, 0, DYN_PARAM_CPU, false);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void popa(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String bits) throws IOException {
        out(fos, "void OPCALL normal_popA" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    common_popA"+bits+"(cpu);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PopA" + bits + ", popA" + bits + ")");

        out(fos32, "void OPCALL dynamic_popA"+bits+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(NULL, common_popA"+bits+", false, false, false, 1, 0, DYN_PARAM_CPU, false);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void pushData(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String bits) throws IOException {
        out(fos, "void OPCALL normal_push" + bits + "imm(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    cpu->push"+bits+"(op->imm);");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(Push" + bits + ", push" + bits + "imm)");

        out(fos32, "void OPCALL dynamic_push"+bits+"imm(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(NULL, common_push"+bits+", false, false, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_"+bits+", false);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void pushf(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String bits, String mask) throws IOException {
        out(fos, "void OPCALL normal_pushf" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    cpu->fillFlags();");
        out(fos, "    cpu->push"+bits+"((cpu->flags|2)"+(mask.length()==0?"":" & "+mask)+");");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushF" + bits + ", pushf" + bits + ")");

        out(fos32, "void OPCALL dynamic_pushf"+bits+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    callHostFunction(NULL, common_fillFlags, false, false, false, 1, 0, DYN_PARAM_CPU, false);");
        out(fos32, "    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(flags), DYN_32bit);");
        out(fos32, "    instRegImm(\'|\', DYN_SRC, DYN_32bit, 2);");
        if (mask.length()!=0)
            out(fos32, "    instRegImm(\'&\', DYN_SRC, DYN_32bit, "+mask+");");
        out(fos32, "    callHostFunction(NULL, common_push"+bits+", false, false, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void popf(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String bits, String mask) throws IOException {
        out(fos, "void OPCALL normal_popf" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    cpu->lazyFlags = FLAGS_NONE;");
        out(fos, "    cpu->setFlags(cpu->pop"+bits+"(), FMASK_ALL"+(mask.length()!=0?" & "+mask:"")+");");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PopF" + bits + ", popf" + bits + ")");

        out(fos32, "void OPCALL dynamic_popf"+bits+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_NONE);");
        out(fos32, "    callHostFunction(NULL, common_pop"+bits+", true, false, false, 1, 0, DYN_PARAM_CPU, false);");
        out(fos32, "    callHostFunction(NULL, common_setFlags, false, false, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_"+bits+", true, FMASK_ALL"+(mask.length()!=0?" & "+mask:"")+", DYN_PARAM_CONST_"+bits+", false);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }
}
