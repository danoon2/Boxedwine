package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class Conditions extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_conditions.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_conditions.h");

            fos.write(header.getBytes());
            condition(fos, fos_init, fos32, "O", "cpu->getOF()");
            condition(fos, fos_init, fos32, "NO", "!cpu->getOF()");
            condition(fos, fos_init, fos32, "B", "cpu->getCF()");
            condition(fos, fos_init, fos32, "NB", "!cpu->getCF()");
            condition(fos, fos_init, fos32, "Z", "cpu->getZF()");
            condition(fos, fos_init, fos32, "NZ", "!cpu->getZF()");
            condition(fos, fos_init, fos32, "BE", "cpu->getZF() || cpu->getCF()");
            condition(fos, fos_init, fos32, "NBE", "!cpu->getZF() && !cpu->getCF()");
            condition(fos, fos_init, fos32, "S", "cpu->getSF()");
            condition(fos, fos_init, fos32, "NS", "!cpu->getSF()");
            condition(fos, fos_init, fos32, "P", "cpu->getPF()");
            condition(fos, fos_init, fos32, "NP", "!cpu->getPF()");
            condition(fos, fos_init, fos32, "L", "cpu->getSF()!=cpu->getOF()");
            condition(fos, fos_init, fos32, "NL", "cpu->getSF()==cpu->getOF()");
            condition(fos, fos_init, fos32, "LE", "cpu->getZF() || cpu->getSF()!=cpu->getOF()");
            condition(fos, fos_init, fos32, "NLE", "!cpu->getZF() && cpu->getSF()==cpu->getOF()");
            fos.close();
            fos = null;
        } catch (IOException e) {

        }
    }

    public void condition(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String condition) throws IOException {
        out(fos, "void OPCALL normal_cmov"+name+"_16_reg(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;");
        out(fos, "    }");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_cmov"+name+"_16_mem(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));");
        out(fos, "    }");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_cmov"+name+"_32_reg(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;");
        out(fos, "    }");
        out(fos, "    NEXT();");
        out(fos, "}");
        out(fos, "void OPCALL normal_cmov"+name+"_32_mem(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+condition+") {");
        out(fos, "        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));");
        out(fos, "    }");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU(Cmov"+name+"_R32R32, cmov"+name+"_32_reg)");
        out(fos_init, "INIT_CPU(Cmov"+name+"_R32E32, cmov"+name+"_32_mem)");
        out(fos_init, "INIT_CPU(Cmov"+name+"_R16R16, cmov"+name+"_16_reg)");
        out(fos_init, "INIT_CPU(Cmov"+name+"_R16E16, cmov"+name+"_16_mem)");

        out(fos32, "void OPCALL dynamic_cmov"+name+"_16_reg(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    movCC(common_condition_"+name.toLowerCase()+", op, DYN_16bit);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void OPCALL dynamic_cmov"+name+"_16_mem(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    movCC(common_condition_"+name.toLowerCase()+", op, DYN_16bit, true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void OPCALL dynamic_cmov"+name+"_32_reg(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    movCC(common_condition_"+name.toLowerCase()+", op, DYN_32bit);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");

        out(fos32, "void OPCALL dynamic_cmov"+name+"_32_mem(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    movCC(common_condition_"+name.toLowerCase()+", op, DYN_32bit, true);");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }
}
