package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class SetCC extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_setcc.h");
            fos.write(header.getBytes());
            cc(fos, fos_init, "setO", "cpu->getOF()");
            cc(fos, fos_init, "setNO", "!cpu->getOF()");
            cc(fos, fos_init, "setB", "cpu->getCF()");
            cc(fos, fos_init, "setNB", "!cpu->getCF()");
            cc(fos, fos_init, "setZ", "cpu->getZF()");
            cc(fos, fos_init, "setNZ", "!cpu->getZF()");
            cc(fos, fos_init, "setBE", "cpu->getZF() || cpu->getCF()");
            cc(fos, fos_init, "setNBE", "!cpu->getZF() && !cpu->getCF()");
            cc(fos, fos_init, "setS", "cpu->getSF()");
            cc(fos, fos_init, "setNS", "!cpu->getSF()");
            cc(fos, fos_init, "setP", "cpu->getPF()");
            cc(fos, fos_init, "setNP", "!cpu->getPF()");
            cc(fos, fos_init, "setL", "cpu->getSF()!=cpu->getOF()");
            cc(fos, fos_init, "setNL", "cpu->getSF()==cpu->getOF()");
            cc(fos, fos_init, "setLE", "cpu->getZF() || cpu->getSF()!=cpu->getOF()");
            cc(fos, fos_init, "setNLE", "!cpu->getZF() && cpu->getSF()==cpu->getOF()");
            fos.close();
        } catch (IOException e) {

        }
    }

    public void cc(FileOutputStream fos, FileOutputStream fos_init, String name, String condition) throws IOException {
        out(fos, "void OPCALL "+name+"_reg(CPU* cpu, DecodedOp* op) {");
        out(fos, "    if ("+condition+") {");
        out(fos, "        *cpu->reg8[op->reg] = 1;");
        out(fos, "    } else {");
        out(fos, "        *cpu->reg8[op->reg] = 0;");
        out(fos, "    }");
        out(fos, "}");
        out(fos, "void OPCALL "+name+"_mem(CPU* cpu, DecodedOp* op) {");
        out(fos, "    if ("+condition+") {");
        out(fos, "        writeb(eaa(cpu, op), 1);");
        out(fos, "    } else {");
        out(fos, "        writeb(eaa(cpu, op), 0);");
        out(fos, "    }");
        out(fos, "}");

        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        out(fos_init, "INIT_CPU("+mixed+"_R8, "+name+"_reg)");
        out(fos_init, "INIT_CPU("+mixed+"_E8, "+name+"_mem)");
    }
}
