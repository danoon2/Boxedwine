package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class Conditions extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_conditions.h");
            fos.write(header.getBytes());
            condition(fos, fos_init, "cmovO", "cpu->getOF()");
            condition(fos, fos_init, "cmovNO", "!cpu->getOF()");
            condition(fos, fos_init, "cmovB", "cpu->getCF()");
            condition(fos, fos_init, "cmovNB", "!cpu->getCF()");
            condition(fos, fos_init, "cmovZ", "cpu->getZF()");
            condition(fos, fos_init, "cmovNZ", "!cpu->getZF()");
            condition(fos, fos_init, "cmovBE", "cpu->getZF() || cpu->getCF()");
            condition(fos, fos_init, "cmovNBE", "!cpu->getZF() && !cpu->getCF()");
            condition(fos, fos_init, "cmovS", "cpu->getSF()");
            condition(fos, fos_init, "cmovNS", "!cpu->getSF()");
            condition(fos, fos_init, "cmovP", "cpu->getPF()");
            condition(fos, fos_init, "cmovNP", "!cpu->getPF()");
            condition(fos, fos_init, "cmovL", "cpu->getSF()!=cpu->getOF()");
            condition(fos, fos_init, "cmovNL", "cpu->getSF()==cpu->getOF()");
            condition(fos, fos_init, "cmovLE", "cpu->getZF() || cpu->getSF()!=cpu->getOF()");
            condition(fos, fos_init, "cmovNLE", "!cpu->getZF() && cpu->getSF()==cpu->getOF()");
            fos.close();
            fos = null;
        } catch (IOException e) {

        }
    }

    public void condition(FileOutputStream fos, FileOutputStream fos_init, String name, String condition) throws IOException {
        out(fos, "void OPCALL "+name+"_16_reg(CPU* cpu, DecodedOp* op) {");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;");
        out(fos, "    }");
        out(fos, "}");
        out(fos, "void OPCALL "+name+"_16_mem(CPU* cpu, DecodedOp* op) {");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));");
        out(fos, "    }");
        out(fos, "}");
        out(fos, "void OPCALL "+name+"_32_reg(CPU* cpu, DecodedOp* op) {");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;");
        out(fos, "    }");
        out(fos, "}");
        out(fos, "void OPCALL "+name+"_32_mem(CPU* cpu, DecodedOp* op) {");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));");
        out(fos, "    }");
        out(fos, "}");

        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        out(fos_init, "INIT_CPU("+mixed+"_R32R32, "+name+"_32_reg)");
        out(fos_init, "INIT_CPU("+mixed+"_R32E32, "+name+"_32_mem)");
        out(fos_init, "INIT_CPU("+mixed+"_R16R16, "+name+"_16_reg)");
        out(fos_init, "INIT_CPU("+mixed+"_R16E16, "+name+"_16_mem)");
    }
}
