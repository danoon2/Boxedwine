package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/30/2018.
 */
public class Jump extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_jump.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_jump.h");
            fos.write(header.getBytes());
            generateAll(fos, fos_init, fos32);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32) throws IOException {
        jump(fos, fos_init, fos32, "O", "cpu->getOF()");
        jump(fos, fos_init, fos32, "NO", "!cpu->getOF()");
        jump(fos, fos_init, fos32, "B", "cpu->getCF()");
        jump(fos, fos_init, fos32, "NB", "!cpu->getCF()");
        jump(fos, fos_init, fos32, "Z", "cpu->getZF()");
        jump(fos, fos_init, fos32, "NZ", "!cpu->getZF()");
        jump(fos, fos_init, fos32, "BE", "cpu->getZF() || cpu->getCF()");
        jump(fos, fos_init, fos32, "NBE", "!cpu->getZF() && !cpu->getCF()");
        jump(fos, fos_init, fos32, "S", "cpu->getSF()");
        jump(fos, fos_init, fos32, "NS", "!cpu->getSF()");
        jump(fos, fos_init, fos32, "P", "cpu->getPF()");
        jump(fos, fos_init, fos32, "NP", "!cpu->getPF()");
        jump(fos, fos_init, fos32, "L", "cpu->getSF()!=cpu->getOF()");
        jump(fos, fos_init, fos32, "NL", "cpu->getSF()==cpu->getOF()");
        jump(fos, fos_init, fos32, "LE", "cpu->getZF() || cpu->getSF()!=cpu->getOF()");
        jump(fos, fos_init, fos32, "NLE", "!cpu->getZF() && cpu->getSF()==cpu->getOF()");
    }

    public void jump(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String condition) throws IOException {
        out(fos, "void OPCALL normal_jump"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        out(fos, "    if ("+condition+") {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}");
        out(fos, "}");

        out(fos_init, "INIT_CPU(Jump"+name+", jump"+name+")");

        out(fos32, "void OPCALL dynamic_jump"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "    callHostFunction(blockNext2, common_condition_"+name.toLowerCase()+", true, false, true, 1, 0, DYN_PARAM_CPU);");
        out(fos32, "    INCREMENT_EIP(op->imm);");
        out(fos32, "    blockNext1();");
        out(fos32, "}");
    }
}
