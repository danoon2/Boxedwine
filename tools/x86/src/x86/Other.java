package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/30/2018.
 */
public class Other extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_other.h");
            out(fos, "#include \"../common/common_other.h\"");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_other.h");
            out(fos32, "#include \"../common/common_other.h\"");

            FileOutputStream fosOps_h = new FileOutputStream("../common/common_other.h");
            FileOutputStream fosOps_cpp = new FileOutputStream("../common/common_other.cpp");

            out(fosOps_cpp, "#include \"boxedwine.h\"");

            fos.write(header.getBytes());
            generateAll(fos, fos_init, fos32, fosOps_h, fosOps_cpp);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp) throws IOException {
        bound(fos, fos_init, "16", "w", "2", fos32, fosOps_h, fosOps_cpp);
        bound(fos, fos_init, "32", "d", "4", fos32, fosOps_h, fosOps_cpp);

        inst(fos, fos_init, "daa", "Daa", "daa(cpu);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(daa, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "das", "Das", "das(cpu);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(das, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "aaa", "Aaa", "aaa(cpu);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(aaa, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "aas", "Aas", "aas(cpu);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(aas, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "aad", "Aad", "aad(cpu, op->imm);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(aad, false, false, false, 2, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32);", true, false);
        inst(fos, fos_init, "aam", "Aam", "if (aam(cpu, op->imm)) {\r\n        NEXT();\r\n    } else {\r\n        NEXT_DONE();\r\n    }", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(aam, true, false, true, 2, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32);", true, false);

        inst(fos, fos_init, "nop", "Nop", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// Nop", true, false);
        // fake instuction to prematuring end a block (like when the block is more than 4k in size)
        inst(fos, fos_init, "done", "Done", "NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "// do nothing", true, false);
        inst(fos, fos_init, "wait", "Wait", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// Wait", true, false);
        inst(fos, fos_init, "cwd", "Cwd", "if (((S16)AX) < 0)\r\n        DX = 0xFFFF;\r\n    else\r\n        DX = 0;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "if (((S16)AX) < 0) {\r\n        DX = 0xFFFF;\r\n    } else {\r\n        DX = 0;\r\n    }", "", "callHostFunction(common_cwd, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "cwq", "Cwq", "if (((S32)EAX) < 0)\r\n        EDX = 0xFFFFFFFF;\r\n    else\r\n        EDX = 0;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "if (((S32)EAX) < 0) {\r\n        EDX = 0xFFFFFFFF;\r\n    } else {\r\n        EDX = 0;\r\n    }", "", "callHostFunction(common_cwq, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);

        inst(fos, fos_init, "callAp", "CallAp", "cpu->call(0, op->imm, op->disp, cpu->eip.u32+op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);", false, false);
        inst(fos, fos_init, "callFar", "CallFar", "cpu->call(1, op->imm, op->disp, cpu->eip.u32+op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);", false, false);
        inst(fos, fos_init, "jmpAp", "JmpAp", "cpu->jmp(0, op->imm, op->disp, cpu->eip.u32+op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);", false, false);
        inst(fos, fos_init, "jmpFar", "JmpFar", "cpu->jmp(1, op->imm, op->disp, cpu->eip.u32+op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n     callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);", false, false);
        inst(fos, fos_init, "retf16", "Retf16", "cpu->eip.u32+=op->len; cpu->ret(0, op->imm);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);\r\n    callHostFunction(common_ret, false, false, false, 3, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);", false, false);
        inst(fos, fos_init, "retf32", "Retf32", "cpu->eip.u32+=op->len; cpu->ret(1, op->imm);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);\r\n    callHostFunction(common_ret, false, false, false, 3, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);", false, false);
        inst(fos, fos_init, "iret", "Iret", "cpu->iret(0, cpu->eip.u32+op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_iret, false, false, false, 3, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);", false, false);
        inst(fos, fos_init, "iret32", "Iret32", "cpu->iret(1, cpu->eip.u32+op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_iret, false, false, false, 3, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);", false, false);

        inst(fos, fos_init, "sahf", "Sahf", "cpu->fillFlags(); cpu->setFlags(AH, FMASK_ALL & 0xFF);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_fillFlags, false, false, false, 1, 0, DYN_PARAM_CPU);\r\n    callHostFunction(common_setFlags, false, false, false, 3, 0, DYN_PARAM_CPU, offsetof(CPU, reg[0].h8), DYN_PARAM_CPU_ADDRESS_8, FMASK_ALL & 0xFF, DYN_PARAM_CONST_32);", true, false);
        inst(fos, fos_init, "lahf", "Lahf", "cpu->fillFlags(); AH = (cpu->flags & (SF|ZF|AF|PF|CF)) | 2;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_fillFlags, false, false, false, 1, 0, DYN_PARAM_CPU);\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, flags), DYN_32bit);\r\n    instRegImm('&', DYN_SRC, DYN_32bit, SF|ZF|AF|PF|CF);\r\n    instRegImm('|', DYN_SRC, DYN_32bit, 2);\r\n    movToCpuFromReg(offsetof(CPU, reg[0].h8), DYN_SRC, DYN_8bit);", true, false);
        inst(fos, fos_init, "salc", "Salc", "if (cpu->getCF()) AL = 0xFF; else AL = 0;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "if (cpu->getCF()) AL = 0xFF; else AL = 0;", "", "callHostFunction(common_salc, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);

        inst(fos, fos_init, "retn16Iw", "Retn16Iw", "U16 eip = cpu->pop16();\r\n    SP = SP+op->imm;\r\n    cpu->eip.u32 = eip;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_pop16, true, false, false, 1, 0, DYN_PARAM_CPU);\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[4].u16), DYN_16bit);\r\n    instRegImm('+', DYN_SRC, DYN_16bit, op->imm);\r\n    movToCpuFromReg(offsetof(CPU, reg[4].u16), DYN_SRC, DYN_16bit);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);", false, false);
        inst(fos, fos_init, "retn32Iw", "Retn32Iw", "U32 eip = cpu->pop32();\r\n    ESP = ESP+op->imm;\r\n    cpu->eip.u32 = eip;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_pop32, true, false, false, 1, 0, DYN_PARAM_CPU);\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[4].u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->imm);\r\n    movToCpuFromReg(offsetof(CPU, reg[4].u32), DYN_SRC, DYN_32bit);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);", false, false);

        inst(fos, fos_init, "retn16", "Retn16", "cpu->eip.u32 = cpu->pop16();\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_pop16, true, false, false, 1, 0, DYN_PARAM_CPU);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);", false, false);
        inst(fos, fos_init, "retn32", "Retn32", "cpu->eip.u32 = cpu->pop32();\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_pop32, true, false, false, 1, 0, DYN_PARAM_CPU);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);", false, false);;

        inst(fos, fos_init, "invalid", "Invalid", "kpanic(\"Invalid instruction %x\\n\", op->inst);", fos32, fosOps_h, fosOps_cpp, "", "", "kpanic(\"Dyn:Invalid instruction %x\\n\", op->inst);", false, false);

        inst(fos, fos_init, "int80", "Int80", "ksyscall(cpu, op->len);\r\n NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(ksyscall, false, false, false, 2, 0, DYN_PARAM_CPU, op->len, DYN_PARAM_CONST_32);", false, false);
        inst(fos, fos_init, "int98", "Int98", "U32 index = cpu->peek32(0);\r\n    if (index<wine_callbackSize && wine_callback[index]) {\r\n        wine_callback[index](cpu);\r\n    } else {\r\n        kpanic(\"Uknown int 98 call: %d\", index);\r\n    }\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "U32 index = cpu->peek32(0);\r\n    if (index<wine_callbackSize && wine_callback[index]) {\r\n        wine_callback[index](cpu);\r\n    } else {\r\n        kpanic(\"Uknown int 98 call: %d\", index);\r\n    }", "", "callHostFunction(common_int98, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "int99", "Int99", "U32 index = cpu->peek32(0);\r\n    if (index<int99CallbackSize && int99Callback[index]) {\r\n        int99Callback[index](cpu);\r\n    } else {\r\n        kpanic(\"Uknown int 99 call: %d\", index);\r\n    }\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "U32 index = cpu->peek32(0);\r\n    if (index<int99CallbackSize && int99Callback[index]) {\r\n        int99Callback[index](cpu);\r\n    } else {\r\n        kpanic(\"Uknown int 99 call: %d\", index);\r\n    }", "", "callHostFunction(common_int99, false, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "intIb", "IntIb", "cpu->thread->signalIllegalInstruction(5);// 5=ILL_PRVOPC  // :TODO: just a guess\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "cpu->thread->signalIllegalInstruction(5);// 5=ILL_PRVOPC  // :TODO: just a guess", "", "callHostFunction(common_intIb, false, false, false, 1, 0, DYN_PARAM_CPU);", false, false);

        inst(fos, fos_init, "xlat", "Xlat", "if (op->ea16) {\r\n        AL = readb(cpu->seg[op->base].address + (U16)(BX + AL));\r\n    } else {\r\n        AL = readb(cpu->seg[op->base].address + EBX + AL);\r\n    }\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[0].u8), DYN_8bit);\r\n    if (op->ea16) {\r\n        movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[3].u16), DYN_16bit);\r\n        movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit);\r\n        instRegReg('+', DYN_DEST, DYN_SRC, DYN_16bit);\r\n        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit);\r\n    } else {\r\n        movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[3].u32), DYN_32bit);\r\n        movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);\r\n        instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit);\r\n    }\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, seg[op->base].address), DYN_32bit);\r\n    instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit);\r\n    movFromMem(DYN_8bit, DYN_DEST);\r\n    movToCpuFromReg(offsetof(CPU, reg[0].u8), DYN_READ_RESULT, DYN_8bit);", true, false);
        inst(fos, fos_init, "hlt", "Hlt", "kpanic(\"Hlt\");", fos32, fosOps_h, fosOps_cpp, "", "", "kpanic(\"Dyn:Hlt\");", false, false);

        inst(fos, fos_init, "cmc", "Cmc", "cpu->fillFlags();\r\n    cpu->setCF(!cpu->getCF());\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->fillFlags();\r\n    cpu->setCF(!cpu->getCF());", "", "callHostFunction(common_cmc, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "clc", "Clc", "cpu->fillFlags();\r\n    cpu->removeCF();\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->fillFlags();\r\n    cpu->removeCF();", "", "callHostFunction(common_clc, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "stc", "Stc", "cpu->fillFlags();\r\n    cpu->addCF();\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->fillFlags();\r\n    cpu->addCF();", "", "callHostFunction(common_stc, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);

        inst(fos, fos_init, "cli", "Cli", "cpu->removeFlag(IF);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->removeFlag(IF);", "", "callHostFunction(common_cli, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "sti", "Sti", "cpu->addFlag(IF);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->addFlag(IF);", "", "callHostFunction(common_sti, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);

        inst(fos, fos_init, "cld", "Cld", "cpu->removeFlag(DF);\r\n    cpu->df=1;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->removeFlag(DF);\r\n    cpu->df=1;", "", "callHostFunction(common_cld, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);
        inst(fos, fos_init, "std", "Std", "cpu->addFlag(DF);\r\n    cpu->df=-1;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->addFlag(DF);\r\n    cpu->df=-1;", "", "callHostFunction(common_std, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);

        inst(fos, fos_init, "rdtsc", "Rdtsc", "U64 t = cpu->instructionCount+op->imm;\r\n    EAX = (U32)t;\r\n    EDX = (U32)(t >> 32);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_rdtsc, true, false, false, 2, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32);", true, false);
        inst(fos, fos_init, "cpuid", "CPUID", "cpu->cpuid();\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->cpuid();", "", "callHostFunction(common_cpuid, true, false, false, 1, 0, DYN_PARAM_CPU);", true, false);

        inst(fos, fos_init, "enter16", "Enter16", "cpu->enter(0, op->imm, op->reg);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_enter, true, false, false, 4, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);", true, false);
        inst(fos, fos_init, "enter32", "Enter32", "cpu->enter(1, op->imm, op->reg);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "callHostFunction(common_enter, true, false, false, 4, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);", true, false);

        inst(fos, fos_init, "leave16", "Leave16", "SP = BP;\r\n    BP = cpu->pop16();\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpuFromCpu(offsetof(CPU, reg[4].u16), offsetof(CPU, reg[5].u16), DYN_16bit, DYN_SRC);\r\n    callHostFunction(common_pop16, true, false, false, 1, 0, DYN_PARAM_CPU);\r\n    movToCpuFromReg(offsetof(CPU, reg[5].u16), DYN_CALL_RESULT, DYN_16bit);", true, false);
        inst(fos, fos_init, "leave32", "Leave32", "ESP = EBP;\r\n    EBP = cpu->pop32();\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpuFromCpu(offsetof(CPU, reg[4].u32), offsetof(CPU, reg[5].u32), DYN_32bit, DYN_SRC);\r\n    callHostFunction(common_pop32, true, false, false, 1, 0, DYN_PARAM_CPU);\r\n    movToCpuFromReg(offsetof(CPU, reg[5].u32), DYN_CALL_RESULT, DYN_32bit);", true, false);

        out(fosOps_h, "void common_loopnz32(CPU* cpu, U32 offset1, U32 offset2);");
        out(fosOps_cpp, "void common_loopnz32(CPU* cpu, U32 offset1, U32 offset2) {\r\n    ECX--;\r\n    if (ECX!=0 && !cpu->getZF()) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }\r\n}");

        inst(fos, fos_init, "loopnz", "LoopNZ", "if (op->ea16){\r\n        CX--;\r\n        if (CX!=0 && !cpu->getZF()) {\r\n            cpu->eip.u32+=op->imm;\r\n            NEXT_BRANCH1();\r\n        } else {\r\n            NEXT_BRANCH2();\r\n        }\r\n    } else {\r\n        ECX--;\r\n        if (ECX!=0 && !cpu->getZF()) {\r\n            cpu->eip.u32+=op->imm;\r\n            NEXT_BRANCH1();\r\n        } else {\r\n            NEXT_BRANCH2();\r\n        }\r\n    }", fos32, fosOps_h, fosOps_cpp, "CX--;\r\n    if (CX!=0 && !cpu->getZF()) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }", ", U32 offset1, U32 offset2", "if (op->ea16) {\r\n        callHostFunction(common_loopnz, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    } else {\r\n        callHostFunction(common_loopnz32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    }", false, false);

        out(fosOps_h, "void common_loopz32(CPU* cpu, U32 offset1, U32 offset2);");
        out(fosOps_cpp, "void common_loopz32(CPU* cpu, U32 offset1, U32 offset2) {\r\n    ECX--;\r\n    if (ECX!=0 && cpu->getZF()) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }\r\n}");

        inst(fos, fos_init, "loopz", "LoopZ", "if (op->ea16){\r\n        CX--;\r\n        if (CX!=0 && cpu->getZF()) {\r\n            cpu->eip.u32+=op->imm;\r\n            NEXT_BRANCH1();\r\n        } else {\r\n            NEXT_BRANCH2();\r\n        }\r\n    } else {\r\n        ECX--;\r\n        if (ECX!=0 && cpu->getZF()) {\r\n            cpu->eip.u32+=op->imm;\r\n            NEXT_BRANCH1();\r\n        } else {\r\n            NEXT_BRANCH2();\r\n        }\r\n    }", fos32, fosOps_h, fosOps_cpp, "CX--;\r\n    if (CX!=0 && !cpu->getZF()) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }", ", U32 offset1, U32 offset2", "if (op->ea16) {\r\n        callHostFunction(common_loopz, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    } else {\r\n        callHostFunction(common_loopz32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    }", false, false);

        out(fosOps_h, "void common_loop32(CPU* cpu, U32 offset1, U32 offset2);");
        out(fosOps_cpp, "void common_loop32(CPU* cpu, U32 offset1, U32 offset2) {\r\n    ECX--;\r\n    if (ECX!=0) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }\r\n}");

        inst(fos, fos_init, "loop", "Loop", "if (op->ea16){\r\n        CX--;\r\n        if (CX!=0) {\r\n            cpu->eip.u32+=op->imm;\r\n            NEXT_BRANCH1();\r\n        } else {\r\n            NEXT_BRANCH2();\r\n        }\r\n    } else {\r\n        ECX--;\r\n        if (ECX!=0) {\r\n            cpu->eip.u32+=op->imm;\r\n            NEXT_BRANCH1();\r\n        } else {\r\n            NEXT_BRANCH2();\r\n        }\r\n    }", fos32, fosOps_h, fosOps_cpp, "CX--;\r\n    if (CX!=0) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }", ", U32 offset1, U32 offset2", "if (op->ea16) {\r\n        callHostFunction(common_loop, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    } else {\r\n        callHostFunction(common_loop32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    }", false, false);

        out(fosOps_h, "void common_jcxz32(CPU* cpu, U32 offset1, U32 offset2);");
        out(fosOps_cpp, "void common_jcxz32(CPU* cpu, U32 offset1, U32 offset2) {\r\n    if (ECX==0) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }\r\n}");

        inst(fos, fos_init, "jcxz", "Jcxz", "if ((op->ea16?CX:ECX)==0) {\r\n        cpu->eip.u32+=op->imm;\r\n        NEXT_BRANCH1();\r\n    } else {\r\n        NEXT_BRANCH2();\r\n    }", fos32, fosOps_h, fosOps_cpp, "if (CX==0) {\r\n        cpu->eip.u32+=offset1;\r\n    } else {\r\n        cpu->eip.u32+=offset2;\r\n    }", ", U32 offset1, U32 offset2", "if (op->ea16) {\r\n        callHostFunction(common_jcxz, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    } else {\r\n        callHostFunction(common_jcxz32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);\r\n    }", false, false);

        inst(fos, fos_init, "InAlIb", "InAlIb", "AL=0xFF;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpu(offsetof(CPU, reg[0].u8), DYN_8bit, 0xFF);", true, false);
        inst(fos, fos_init, "InAxIb", "InAxIb", "AX=0xFFFF;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpu(offsetof(CPU, reg[0].u16), DYN_16bit, 0xFFFF);", true, false);
        inst(fos, fos_init, "InEaxIb", "InEaxIb", "EAX=0xFFFFFFFF;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpu(offsetof(CPU, reg[0].u32), DYN_32bit, 0xFFFFFFFF);", true, false);

        inst(fos, fos_init, "OutIbAl", "OutIbAl", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// do nothing", true, false);
        inst(fos, fos_init, "OutIbAx", "OutIbAx", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// do nothing", true, false);
        inst(fos, fos_init, "OutIbEax", "OutIbEax", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// do nothing", true, false);

        inst(fos, fos_init, "InAlDx", "InAlDx", "AL=0xFF;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpu(offsetof(CPU, reg[0].u8), DYN_8bit, 0xFF);", true, false);
        inst(fos, fos_init, "InAxDx", "InAxDx", "AX=0xFFFF;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpu(offsetof(CPU, reg[0].u16), DYN_16bit, 0xFFFF);", true, false);
        inst(fos, fos_init, "InEaxDx", "InEaxDx", "EAX=0xFFFFFFFF;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpu(offsetof(CPU, reg[0].u32), DYN_32bit, 0xFFFFFFFF);", true, false);

        inst(fos, fos_init, "OutDxAl", "OutDxAl", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// do nothing", true, false);
        inst(fos, fos_init, "OutDxAx", "OutDxAx", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// do nothing", true, false);
        inst(fos, fos_init, "OutDxEax", "OutDxEax", "NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "// do nothing", true, false);

        inst(fos, fos_init, "callJw", "CallJw", "cpu->push16(cpu->eip.u32 + op->len);\r\n    cpu->eip.u32 += (S16)op->imm;\r\n    NEXT_BRANCH1();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_push16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);\r\n    INCREMENT_EIP(op->len+(S32)((S16)op->imm));", false, false);
        inst(fos, fos_init, "callJd", "CallJd", "cpu->push32(cpu->eip.u32 + op->len);\r\n    cpu->eip.u32 += (S32)op->imm;\r\n    NEXT_BRANCH1();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_push32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);\r\n    INCREMENT_EIP(op->len+(S32)op->imm);", false, false);

        inst(fos, fos_init, "jmp8", "JmpJb", "cpu->eip.u32 += (S8)op->imm;\r\n    NEXT_BRANCH1();", fos32, fosOps_h, fosOps_cpp, "", "", "INCREMENT_EIP(op->len+(S32)((S8)op->imm));", false, false);
        inst(fos, fos_init, "jmp16", "JmpJw", "cpu->eip.u32 += (S16)op->imm;\r\n    NEXT_BRANCH1();", fos32, fosOps_h, fosOps_cpp, "", "", "INCREMENT_EIP(op->len+(S32)((S16)op->imm));", false, false);
        inst(fos, fos_init, "jmp32", "JmpJd", "cpu->eip.u32 += (S32)op->imm;\r\n    NEXT_BRANCH1();", fos32, fosOps_h, fosOps_cpp, "", "", "INCREMENT_EIP(op->len+(S32)op->imm);", false, false);

        inst(fos, fos_init, "callR16", "CallR16", "cpu->push16(cpu->eip.u32+op->len);\r\n    cpu->eip.u32 = cpu->reg[op->reg].u16;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_push16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u16), DYN_16bit);\r\n    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit); movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);", false, false);
        inst(fos, fos_init, "callR32", "CallR32", "cpu->push32(cpu->eip.u32+op->len);\r\n    cpu->eip.u32 = cpu->reg[op->reg].u32;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_push32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);\r\n    movToCpuFromCpu(offsetof(CPU, eip.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);", false, false);
        inst(fos, fos_init, "callE16", "CallE16", "U32 neweip = readw(eaa(cpu, op));\r\n    cpu->push16(cpu->eip.u32+op->len);\r\n    cpu->eip.u32 = neweip;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_push16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_16);\r\n    movToRegFromReg(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_16bit);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);", false, false);
        inst(fos, fos_init, "callE32", "CallE32", "U32 neweip = readd(eaa(cpu, op));\r\n    cpu->push32(cpu->eip.u32+op->len);\r\n    cpu->eip.u32 = neweip;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_32bit, DYN_ADDRESS);\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_SRC, DYN_32bit, op->len);\r\n    callHostFunction(common_push32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);", false, false);

        inst(fos, fos_init, "jmpR16", "JmpR16", "cpu->eip.u32 = cpu->reg[op->reg].u16;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u16), DYN_16bit);\r\n    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);", false, false);
        inst(fos, fos_init, "jmpR32", "JmpR32", "cpu->eip.u32 = cpu->reg[op->reg].u32;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpuFromCpu(offsetof(CPU, eip.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);", false, false);
        inst(fos, fos_init, "jmpE16", "JmpE16", "U32 neweip = readw(eaa(cpu, op));\r\n    cpu->eip.u32 = neweip;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    movToRegFromReg(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_16bit);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);", false, false);
        inst(fos, fos_init, "jmpE32", "JmpE32", "U32 neweip = readd(eaa(cpu, op));\r\n    cpu->eip.u32 = neweip;\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_32bit, DYN_ADDRESS);\r\n    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);", false, false);

        inst(fos, fos_init, "callFarE16", "CallFarE16", "U32 eaa = eaa(cpu, op);\r\n    U16 newip = readw(eaa);\r\n    U16 newcs = readw(eaa+2);\r\n    cpu->call(0, newcs, newip, cpu->eip.u32 + op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->len);\r\n    calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_READ_RESULT, DYN_16bit);\r\n    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_16, DYN_DEST, DYN_PARAM_REG_32);", false, false);
        inst(fos, fos_init, "callFarE32", "CallFarE32", "U32 eaa = eaa(cpu, op);\r\n    U32 newip = readd(eaa);\r\n    U16 newcs = readw(eaa+4);\r\n    cpu->call(1, newcs, newip, cpu->eip.u32 + op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->len);\r\n    calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_32bit, DYN_ADDRESS);\r\n    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_READ_RESULT, DYN_32bit);\r\n    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_32, DYN_DEST, DYN_PARAM_REG_32);", false, false);

        inst(fos, fos_init, "jmpFarE16", "JmpFarE16", "U32 eaa = eaa(cpu, op);\r\n    U16 newip = readw(eaa);\r\n    U16 newcs = readw(eaa+2);\r\n    cpu->jmp(0, newcs, newip, cpu->eip.u32 + op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->len);\r\n    calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_READ_RESULT, DYN_16bit);\r\n    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_16, DYN_DEST, DYN_PARAM_REG_32);", false, false);
        inst(fos, fos_init, "jmpFarE32", "JmpFarE32", "U32 eaa = eaa(cpu, op);\r\n    U32 newip = readd(eaa);\r\n    U16 newcs = readw(eaa+4);\r\n    cpu->jmp(1, newcs, newip, cpu->eip.u32 + op->len);\r\n    NEXT_DONE();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);\r\n    instRegImm('+', DYN_DEST, DYN_32bit, op->len);\r\n    calculateEaa(op, DYN_ADDRESS);\r\n    movFromMem(DYN_32bit, DYN_ADDRESS);\r\n    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_READ_RESULT, DYN_32bit);\r\n    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);\r\n    movFromMem(DYN_16bit, DYN_ADDRESS);\r\n    callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_32, DYN_DEST, DYN_PARAM_REG_32);", false, false);

        inst(fos, fos_init, "larr16r16", "LarR16R16", "cpu->reg[op->reg].u16 = cpu->lar(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->reg[dstReg].u16 = cpu->lar(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);", ", U32 dstReg, U32 srcReg", "callHostFunction(common_larr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);", true, false);
        inst(fos, fos_init, "larr16e16", "LarR16E16", "cpu->reg[op->reg].u16 = cpu->lar(readw(eaa(cpu, op)), cpu->reg[op->reg].u16);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->reg[reg].u16 = cpu->lar(readw(address), cpu->reg[reg].u16);", ", U32 reg, U32 address", "calculateEaa(op, DYN_ADDRESS);\r\n    callHostFunction(common_larr16e16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);", true, false);
        inst(fos, fos_init, "lslr16r16", "LslR16R16", "cpu->reg[op->reg].u16 = cpu->lsl(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->reg[dstReg].u16 = cpu->lsl(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);", ", U32 dstReg, U32 srcReg", "callHostFunction(common_lslr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);", true, false);
        inst(fos, fos_init, "lslr16e16", "LslR16E16", "cpu->reg[op->reg].u16 = cpu->lsl(readw(eaa(cpu, op)), cpu->reg[op->reg].u16);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "cpu->reg[reg].u16 = cpu->lsl(readw(address), cpu->reg[reg].u16);", ", U32 reg, U32 address", "calculateEaa(op, DYN_ADDRESS);\r\n    callHostFunction(common_lslr16e16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);", true, false);

        inst(fos, fos_init, "xaddr32r32", "XaddR32R32", "cpu->src.u32 = cpu->reg[op->reg].u32;\r\n    cpu->dst.u32 = cpu->reg[op->rm].u32;\r\n    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;\r\n    cpu->lazyFlags = FLAGS_ADD32;\r\n    cpu->reg[op->reg].u32 = cpu->dst.u32;\r\n    cpu->reg[op->rm].u32 =  cpu->result.u32;\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);\r\n    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_DEST);\r\n    instRegReg('+', DYN_SRC, DYN_DEST, DYN_32bit);\r\n    movToCpuFromReg(offsetof(CPU, result.u32), DYN_SRC, DYN_32bit);\r\n    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);\r\n    movToCpuFromReg(offsetof(CPU, reg[op->rm].u32), DYN_SRC, DYN_32bit);\r\n    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);", true, false);
        inst(fos, fos_init, "xaddr32e32", "XaddR32E32", "U32 address = eaa(cpu, op);\r\n    cpu->src.u32 = cpu->reg[op->reg].u32;\r\n    cpu->dst.u32 = readd(address);\r\n    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;\r\n    cpu->lazyFlags = FLAGS_ADD32;\r\n    cpu->reg[op->reg].u32 = cpu->dst.u32;\r\n    writed(address, cpu->result.u32);\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "calculateEaa(op, DYN_ADDRESS);\r\n    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);\r\n    movFromMem(DYN_32bit, DYN_ADDRESS);\r\n    movToCpuFromReg(offsetof(CPU, dst.u32), DYN_READ_RESULT, DYN_32bit);\r\n    instRegReg('+', DYN_SRC, DYN_READ_RESULT, DYN_32bit);\r\n    movToCpuFromReg(offsetof(CPU, result.u32), DYN_SRC, DYN_32bit);\r\n    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_READ_RESULT, DYN_32bit);\r\n    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit);\r\n    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);", true, false);

        inst(fos, fos_init, "bswap32", "Bswap32", "U32 val = cpu->reg[op->reg].u32;\r\n    cpu->reg[op->reg].u32 = (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >>  8) | ((val & 0x0000ff00) <<  8) | ((val & 0x000000ff) << 24));\r\nNEXT();", fos32, fosOps_h, fosOps_cpp, "", "", "movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].byte[0]), DYN_8bit);\r\n    movToCpuFromCpu(offsetof(CPU, reg[op->reg].byte[0]), offsetof(CPU, reg[op->reg].byte[3]), DYN_8bit, DYN_DEST);\r\n    movToCpuFromReg(offsetof(CPU, reg[op->reg].byte[3]), DYN_SRC, DYN_8bit);\r\n    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].byte[1]), DYN_8bit);\r\n    movToCpuFromCpu(offsetof(CPU, reg[op->reg].byte[1]), offsetof(CPU, reg[op->reg].byte[2]), DYN_8bit, DYN_DEST);\r\n    movToCpuFromReg(offsetof(CPU, reg[op->reg].byte[2]), DYN_SRC, DYN_8bit);", true, false);
        inst(fos, fos_init, "cmpxchgg8b", "CmpXchg8b", "common_cmpxchgg8b(cpu, eaa(cpu, op));\r\n    NEXT();", fos32, fosOps_h, fosOps_cpp, "U64 value1 = ((U64)EDX) << 32 | EAX;\r\n    U64 value2 = readq(address);\r\n    cpu->fillFlags();\r\n    if (value1 == value2) {\r\n        cpu->addZF();\r\n        writed(address, EBX);\r\n        writed(address + 4, ECX);\r\n    } else {\r\n        cpu->removeZF();\r\n        EDX = (U32)(value2 >> 32);\r\n        EAX = (U32)value2;\r\n    }", ", U32 address", "calculateEaa(op, DYN_ADDRESS);\r\n    callHostFunction(common_cmpxchgg8b, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32);", true, false);

        loadSeg(fos, fos_init, fos32, "loadSegment16", "LoadSegment16", "16", "w", "2");
        loadSeg(fos, fos_init, fos32, "loadSegment32", "LoadSegment32", "32", "d", "4");
    }

    public void bound(FileOutputStream fos, FileOutputStream fos_init, String bits, String width, String offset, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp) throws IOException {
        out(fosOps_h, "U32 common_bound" + bits + "(CPU* cpu, U32 reg, U32 address);");
        out(fosOps_cpp, "U32 common_bound" + bits + "(CPU* cpu, U32 reg, U32 address){");
        out(fosOps_cpp, "    if (cpu->reg[reg].u"+bits+"<read"+width+"(address) || cpu->reg[reg].u"+bits+">read"+width+"(address+"+offset+")) {");
        out(fosOps_cpp, "        cpu->prepareException(EXCEPTION_BOUND, 0);");
        out(fosOps_cpp, "        return 0;");
        out(fosOps_cpp, "    } else { ");
        out(fosOps_cpp, "        return 1;");
        out(fosOps_cpp, "    }");
        out(fosOps_cpp, "}");

        out(fos, "void OPCALL normal_bound" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if (common_bound"+bits+"(cpu, op->reg, eaa(cpu, op))) {");
        out(fos, "        NEXT();");
        out(fos, "    } else { ");
        out(fos, "        NEXT_DONE();");
        out(fos, "    }");
        out(fos, "}");

        out(fos_init, "INIT_CPU(Bound" + bits + ", bound" + bits + ")");

        out(fos32, "void OPCALL dynamic_bound"+bits+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        out(fos32, "    callHostFunction(common_bound"+bits+", true, false, true, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }

    public void inst(FileOutputStream fos, FileOutputStream fos_init, String name, String ename, String func, FileOutputStream fos32, FileOutputStream fosOps_h, FileOutputStream fosOps_cpp, String body, String bodyParams, String x32, boolean x32Eip, boolean bodyReturns) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    "+func);
        out(fos, "}");

        if (body.length()>0) {
            out(fosOps_h, (bodyReturns?"U32":"void")+" common_" + name + "(CPU* cpu"+bodyParams+");");
            out(fosOps_cpp, (bodyReturns?"U32":"void")+" common_" + name + "(CPU* cpu"+bodyParams+"){");
            out(fosOps_cpp, "    "+body);
            out(fosOps_cpp, "}");
        }
        out(fos_init, "INIT_CPU("+ename+", "+name+")");
        
        if (x32.length()>0) {
            out(fos32, "void OPCALL dynamic_"+name+"(CPU* cpu, DecodedOp* op) {");
            out(fos32, "    "+x32);
            if (x32Eip) {
                out(fos32, "    INCREMENT_EIP(op->len);");
            }
            out(fos32, "}");
        }
    }

    public void loadSeg(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String ename, String bits, String width, String offset) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    U32 eaa = eaa(cpu, op);");
        out(fos, "    U"+bits+" val = read"+width+"(eaa);");
        out(fos, "    U32 selector = readw(eaa+"+offset+");"); // yes, only 2 bytes
        out(fos, "    if (cpu->setSegment(op->imm, selector)) {");
        out(fos, "        cpu->reg[op->reg].u"+bits+" = val;");
        out(fos, "        NEXT();");
        out(fos, "    } else {");
        out(fos, "        NEXT_DONE();");
        out(fos, "    }");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");

        out(fos32, "void OPCALL dynamic_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        out(fos32, "    movFromMem(DYN_"+bits+"bit, DYN_ADDRESS);");
        out(fos32, "    movToRegFromReg(DYN_DEST, DYN_"+bits+"bit, DYN_READ_RESULT, DYN_"+bits+"bit);");
        out(fos32, "    instRegImm('+', DYN_ADDRESS, DYN_32bit, "+offset+");");
        out(fos32, "    movFromMem(DYN_16bit, DYN_ADDRESS);");
        out(fos32, "    callHostFunction(common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32,  DYN_READ_RESULT, DYN_PARAM_REG_16);");
        out(fos32, "    movToCpuFromReg(offsetof(CPU, reg[op->reg].u"+bits+"), DYN_DEST, DYN_"+bits+"bit);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }
}
