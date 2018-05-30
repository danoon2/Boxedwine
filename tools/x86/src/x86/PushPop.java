package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class PushPop extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_pushpop.h");
            fos.write(header.getBytes());
            pushPopReg(fos, fos_init, "w", "16");
            pushPopReg(fos, fos_init, "d", "32");
            pushPopSeg(fos, fos_init, "16");
            pushPopSeg(fos, fos_init, "32");
            pusha(fos, fos_init, "16", "");
            pusha(fos, fos_init, "32", "E");
            popa(fos, fos_init, "16", "");
            popa(fos, fos_init, "32", "E");
            pushData(fos, fos_init, "16");
            pushData(fos, fos_init, "32");
            popf(fos, fos_init, "16", " & 0xFFFF");
            popf(fos, fos_init, "32", "");
            pushf(fos, fos_init, "16", "cpu->flags|2");
            pushf(fos, fos_init, "32", "(cpu->flags|2) & 0xFCFFFF");
            fos.close();
        } catch (IOException e) {

        }
    }

    public void pushPopReg(FileOutputStream fos, FileOutputStream fos_init, String name, String bits) throws IOException {
        out(fos, "void OPCALL pushE"+name+"_reg(CPU* cpu, DecodedOp* op){");
        out(fos, "    cpu->push"+bits+"(cpu->reg[op->reg].u"+bits+");");
        out(fos, "}");
        out(fos, "void OPCALL popE"+name+"_reg(CPU* cpu, DecodedOp* op){");
        out(fos, "    cpu->reg[op->reg].u"+bits+" = cpu->pop"+bits+"();");
        out(fos, "}");
        out(fos, "void OPCALL pushE"+name+"_mem(CPU* cpu, DecodedOp* op){");
        out(fos, "    cpu->push"+bits+"(read"+name+"(eaa(cpu, op)));");
        out(fos, "}");
        out(fos, "void OPCALL popE"+name+"_mem(CPU* cpu, DecodedOp* op){");
        out(fos, "     write"+name+"(eaa(cpu, op), cpu->pop"+bits+"());");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushR"+bits+", pushE"+name+"_reg)");
        out(fos_init, "INIT_CPU(PushE"+bits+", pushE"+name+"_mem)");
        out(fos_init, "INIT_CPU(PopR"+bits+", popE"+name+"_reg)");
        out(fos_init, "INIT_CPU(PopE"+bits+", popE"+name+"_mem)");
    }

    public void pushPopSeg(FileOutputStream fos, FileOutputStream fos_init, String bits) throws IOException {
        out(fos, "void OPCALL pushSeg" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    cpu->push" + bits + "(cpu->seg[op->reg].value);");
        out(fos, "}");
        out(fos, "void OPCALL popSeg" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    if (cpu->setSegment(op->reg, cpu->peek" + bits + "(0))) {cpu->pop" + bits+"(); cpu->eip.u32+=op->len;}");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushSeg" + bits + ", pushSeg" + bits + ")");
        out(fos_init, "INIT_CPU(PopSeg" + bits + ", popSeg" + bits + ")");
    }

    public void pusha(FileOutputStream fos, FileOutputStream fos_init, String bits, String regPrefix) throws IOException {
        out(fos, "void OPCALL pushA" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    U"+bits+" sp = "+regPrefix+"SP;");
        out(fos, "    cpu->push"+bits+"("+regPrefix+"AX);");
        out(fos, "    cpu->push"+bits+"("+regPrefix+"CX);");
        out(fos, "    cpu->push"+bits+"("+regPrefix+"DX);");
        out(fos, "    cpu->push"+bits+"("+regPrefix+"BX);");
        out(fos, "    cpu->push"+bits+"(sp);");
        out(fos, "    cpu->push"+bits+"("+regPrefix+"BP);");
        out(fos, "    cpu->push"+bits+"("+regPrefix+"SI);");
        out(fos, "    cpu->push"+bits+"("+regPrefix+"DI);");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushA" + bits + ", pushA" + bits + ")");
    }

    public void popa(FileOutputStream fos, FileOutputStream fos_init, String bits, String regPrefix) throws IOException {
        out(fos, "void OPCALL popA" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    " + regPrefix + "DI = cpu->pop" + bits + "();");
        out(fos, "    " + regPrefix + "SI = cpu->pop" + bits + "();");
        out(fos, "    " + regPrefix + "BP = cpu->pop" + bits + "();");
        out(fos, "    cpu->pop" + bits + "();");
        out(fos, "    " + regPrefix + "BX = cpu->pop" + bits + "();");
        out(fos, "    " + regPrefix + "DX = cpu->pop" + bits + "();");
        out(fos, "    " + regPrefix + "CX = cpu->pop" + bits + "();");
        out(fos, "    " + regPrefix + "AX = cpu->pop" + bits + "();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PopA" + bits + ", popA" + bits + ")");
    }

    public void pushData(FileOutputStream fos, FileOutputStream fos_init, String bits) throws IOException {
        out(fos, "void OPCALL push" + bits + "imm(CPU* cpu, DecodedOp* op){");
        out(fos, "    cpu->push"+bits+"(op->imm);");
        out(fos, "}");

        out(fos_init, "INIT_CPU(Push" + bits + ", push" + bits + "imm)");
    }

    public void pushf(FileOutputStream fos, FileOutputStream fos_init, String bits, String flags) throws IOException {
        out(fos, "void OPCALL pushf" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    cpu->fillFlags();");
        out(fos, "    cpu->push"+bits+"("+flags+");");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PushF" + bits + ", pushf" + bits + ")");
    }

    public void popf(FileOutputStream fos, FileOutputStream fos_init, String bits, String mask) throws IOException {
        out(fos, "void OPCALL popf" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    cpu->lazyFlags = FLAGS_NONE;");
        out(fos, "    cpu->setFlags(cpu->pop"+bits+"(), FMASK_ALL"+mask+");");
        out(fos, "}");

        out(fos_init, "INIT_CPU(PopF" + bits + ", popf" + bits + ")");
    }
}
