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
            fos.write(header.getBytes());
            generateAll(fos, fos_init);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        jump(fos, fos_init, "jumpO", "JumpO", "cpu->getOF()");
        jump(fos, fos_init, "jumpNO", "JumpNO", "!cpu->getOF()");
        jump(fos, fos_init, "jumpB", "JumpB", "cpu->getCF()");
        jump(fos, fos_init, "jumpNB", "JumpNB", "!cpu->getCF()");
        jump(fos, fos_init, "jumpZ", "JumpZ", "cpu->getZF()");
        jump(fos, fos_init, "jumpNZ", "JumpNZ", "!cpu->getZF()");
        jump(fos, fos_init, "jumpBE", "JumpBE", "cpu->getZF() || cpu->getCF()");
        jump(fos, fos_init, "jumpNBE", "JumpNBE", "!cpu->getZF() && !cpu->getCF()");
        jump(fos, fos_init, "jumpS", "JumpS", "cpu->getSF()");
        jump(fos, fos_init, "jumpNS", "JumpNS", "!cpu->getSF()");
        jump(fos, fos_init, "jumpP", "JumpP", "cpu->getPF()");
        jump(fos, fos_init, "jumpNP", "JumpNP", "!cpu->getPF()");
        jump(fos, fos_init, "jumpL", "JumpL", "cpu->getSF()!=cpu->getOF()");
        jump(fos, fos_init, "jumpNL", "JumpNL", "cpu->getSF()==cpu->getOF()");
        jump(fos, fos_init, "jumpLE", "JumpLE", "cpu->getZF() || cpu->getSF()!=cpu->getOF()");
        jump(fos, fos_init, "jumpNLE", "JumpNLE", "!cpu->getZF() && cpu->getSF()==cpu->getOF()");
    }

    public void jump(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String condition) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    cpu->eip.u32+=op->len; if ("+condition+") cpu->eip.u32+=op->imm;");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }
}
