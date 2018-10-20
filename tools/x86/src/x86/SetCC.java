package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class SetCC extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_setcc.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_setcc.h");

            fos.write(header.getBytes());
            cc(fos, fos_init, fos32, "O", "cpu->getOF()");
            cc(fos, fos_init, fos32, "NO", "!cpu->getOF()");
            cc(fos, fos_init, fos32, "B", "cpu->getCF()");
            cc(fos, fos_init, fos32, "NB", "!cpu->getCF()");
            cc(fos, fos_init, fos32, "Z", "cpu->getZF()");
            cc(fos, fos_init, fos32, "NZ", "!cpu->getZF()");
            cc(fos, fos_init, fos32, "BE", "cpu->getZF() || cpu->getCF()");
            cc(fos, fos_init, fos32, "NBE", "!cpu->getZF() && !cpu->getCF()");
            cc(fos, fos_init, fos32, "S", "cpu->getSF()");
            cc(fos, fos_init, fos32, "NS", "!cpu->getSF()");
            cc(fos, fos_init, fos32, "P", "cpu->getPF()");
            cc(fos, fos_init, fos32, "NP", "!cpu->getPF()");
            cc(fos, fos_init, fos32, "L", "cpu->getSF()!=cpu->getOF()");
            cc(fos, fos_init, fos32, "NL", "cpu->getSF()==cpu->getOF()");
            cc(fos, fos_init, fos32, "LE", "cpu->getZF() || cpu->getSF()!=cpu->getOF()");
            cc(fos, fos_init, fos32, "NLE", "!cpu->getZF() && cpu->getSF()==cpu->getOF()");
            fos.close();
        } catch (IOException e) {

        }
    }

    public void cc(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String condition) throws IOException {
        out(fos, "void OPCALL normal_set"+name+"_reg(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+condition+") {");
        out(fos, "        *cpu->reg8[op->reg] = 1;");
        out(fos, "    } else {");
        out(fos, "        *cpu->reg8[op->reg] = 0;");
        out(fos, "    }");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_set"+name+"_mem(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+condition+") {");
        out(fos, "        writeb(eaa(cpu, op), 1);");
        out(fos, "    } else {");
        out(fos, "        writeb(eaa(cpu, op), 0);");
        out(fos, "    }");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(Set"+name+"_R8, set"+name+"_reg)");
        out(fos_init, "INIT_CPU(Set"+name+"_E8, set"+name+"_mem)");

        out(fos32, "void dynamic_set"+name+"_reg(DynamicData* data, DecodedOp* op) {");
        out(fos32, "    setCPU(data, OFFSET_REG8(op->reg), DYN_8bit, "+name+");");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void dynamic_set"+name+"_mem(DynamicData* data, DecodedOp* op) {");
        out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        out(fos32, "    setMem(data, DYN_ADDRESS, DYN_8bit, "+name+", true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }
}
