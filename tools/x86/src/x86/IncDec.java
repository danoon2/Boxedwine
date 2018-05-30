package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class IncDec extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_incdec.h");
            fos.write(header.getBytes());
            incDec(fos, fos_init, "inc", "+");
            incDec(fos, fos_init, "dec", "-");
            fos.close();
        } catch (IOException e) {

        }
    }

    public void incDec(FileOutputStream fos, FileOutputStream fos_init, String name, String op) throws IOException {
        incDec8(fos, fos_init, name, op);
        incDec16(fos, fos_init, name, op);
        incDec32(fos, fos_init, name, op);
    }

    public void incDec8(FileOutputStream fos, FileOutputStream fos_init, String name, String op) throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        incDecBase(fos, fos_init, mixed+"R8", name+"8_reg", "FLAGS_"+name.toUpperCase()+"8", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, false, "8");
        incDecBase(fos, fos_init, mixed+"E8", name+"8_mem32", "FLAGS_"+name.toUpperCase()+"8", "readb(eaa)", "writeb(eaa, ", ")", op, true, "8");
    }

    public void incDec16(FileOutputStream fos, FileOutputStream fos_init, String name, String op) throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        incDecBase(fos, fos_init, mixed+"R16", name+"16_reg", "FLAGS_"+name.toUpperCase()+"16", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, false, "16");
        incDecBase(fos, fos_init, mixed+"E16", name+"16_mem32", "FLAGS_"+name.toUpperCase()+"16", "readw(eaa)", "writew(eaa, ", ")", op, true, "16");
    }

    public void incDec32(FileOutputStream fos, FileOutputStream fos_init, String name, String op) throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        incDecBase(fos, fos_init, mixed+"R32", name+"32_reg", "FLAGS_"+name.toUpperCase()+"32", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, false, "32");
        incDecBase(fos, fos_init, mixed+"E32", name+"32_mem32", "FLAGS_"+name.toUpperCase()+"32", "readd(eaa)", "writed(eaa, ", ")", op, true, "32");
    }

    public void incDecBase(FileOutputStream fos, FileOutputStream fos_init, String enumName, String name, String flagName, String destLoad, String destSave1, String destSave2, String op, boolean eaa, String bits) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        if (eaa)
            out(fos, "    U32 eaa = eaa(cpu, op);");
        out(fos, "    cpu->oldCF=cpu->getCF();");
        out(fos, "    cpu->dst.u" + bits + "=" + destLoad + ";");
        out(fos, "    cpu->result.u" + bits + "=cpu->dst.u" + bits + " " + op + " 1;");
        out(fos, "    cpu->lazyFlags = " + flagName + ";");
        out(fos, "    " + destSave1 + "cpu->result.u" + bits + destSave2 + ";");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+name+")");
    }
}
