package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class IncDec extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_incdec.h");
            FileOutputStream fos32 = new FileOutputStream("../dynamic/dynamic_incdec.h");
            fos.write(header.getBytes());
            incDec(fos, fos_init, fos32, "inc", "+");
            incDec(fos, fos_init, fos32, "dec", "-");
            fos.close();
        } catch (IOException e) {

        }
    }

    public void incDec(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op) throws IOException {
        incDec8(fos, fos_init, fos32, name, op);
        incDec16(fos, fos_init, fos32, name, op);
        incDec32(fos, fos_init, fos32, name, op);
    }

    public void incDec8(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op) throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        incDecBase(fos, fos_init, fos32, mixed+"R8", name+"8_reg", "FLAGS_"+name.toUpperCase()+"8", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, false, "8", "OFFSET_REG8(op->reg)");
        incDecBase(fos, fos_init, fos32, mixed+"E8", name+"8_mem32", "FLAGS_"+name.toUpperCase()+"8", "readb(eaa)", "writeb(eaa, ", ")", op, true, "8", "");
    }

    public void incDec16(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op) throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        incDecBase(fos, fos_init, fos32, mixed+"R16", name+"16_reg", "FLAGS_"+name.toUpperCase()+"16", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, false, "16", "CPU_OFFSET_OF(reg[op->reg].u16)");
        incDecBase(fos, fos_init, fos32, mixed+"E16", name+"16_mem32", "FLAGS_"+name.toUpperCase()+"16", "readw(eaa)", "writew(eaa, ", ")", op, true, "16", "");
    }

    public void incDec32(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String name, String op) throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        incDecBase(fos, fos_init, fos32, mixed+"R32", name+"32_reg", "FLAGS_"+name.toUpperCase()+"32", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, false, "32", "CPU_OFFSET_OF(reg[op->reg].u32)");
        incDecBase(fos, fos_init, fos32, mixed+"E32", name+"32_mem32", "FLAGS_"+name.toUpperCase()+"32", "readd(eaa)", "writed(eaa, ", ")", op, true, "32", "");
    }

    public void incDecBase(FileOutputStream fos, FileOutputStream fos_init, FileOutputStream fos32, String enumName, String name, String flagName, String destLoad, String destSave1, String destSave2, String op, boolean eaa, String bits, String x32Dest) throws IOException {
        out(fos, "void OPCALL normal_"+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    START_OP(cpu, op);");
        if (eaa)
            out(fos, "    U32 eaa = eaa(cpu, op);");
        out(fos, "    cpu->oldCF=cpu->getCF();");
        out(fos, "    cpu->dst.u" + bits + "=" + destLoad + ";");
        out(fos, "    cpu->result.u" + bits + "=cpu->dst.u" + bits + " " + op + " 1;");
        out(fos, "    cpu->lazyFlags = " + flagName + ";");
        out(fos, "    " + destSave1 + "cpu->result.u" + bits + destSave2 + ";");
        out(fos, "    NEXT();");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+name+")");

        out(fos32, "void dynamic_"+name+"(DynamicData* data, DecodedOp* op) {");
        if (eaa) {
            out(fos32, "    calculateEaa(op, DYN_ADDRESS);");
        }
        out(fos32, "    if (!op->needsToSetFlags()) {");
        if (eaa) {
            out(fos32, "        instMemImm('"+op+"', DYN_ADDRESS, DYN_"+bits+"bit, 1, true);");
        } else {
            out(fos32, "        instCPUImm('"+op+"', "+x32Dest+", DYN_"+bits+"bit, 1);");
        }
        out(fos32, "    } else {");
        out(fos32, "        dynamic_getCF(data);");
        out(fos32, "        movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);");
        if (eaa) {
            out(fos32, "        movToCpuFromMem(CPU_OFFSET_OF(dst.u"+bits+"), DYN_"+bits+"bit, DYN_ADDRESS, false, false);");
            out(fos32, "        instRegImm('"+op+"', DYN_CALL_RESULT, DYN_"+bits+"bit, 1);");
            out(fos32, "        movToCpuFromReg(CPU_OFFSET_OF(result.u"+bits+"), DYN_CALL_RESULT, DYN_"+bits+"bit, false);");
            out(fos32, "        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_"+bits+"bit, true, true);");
        } else {
            out(fos32, "        movToCpuFromCpu(CPU_OFFSET_OF(dst.u" + bits + "), " + x32Dest + ", DYN_" + bits + "bit, DYN_DEST, false);");
            out(fos32, "        instRegImm('"+op+"', DYN_DEST, DYN_"+bits+"bit, 1);");
            out(fos32, "        movToCpuFromReg(CPU_OFFSET_OF(result.u"+bits+"), DYN_DEST, DYN_"+bits+"bit, false);");
            out(fos32, "        movToCpuFromReg("+x32Dest+", DYN_DEST, DYN_"+bits+"bit, true);");
        }
        out(fos32, "        movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)" + flagName + ");");
        out(fos32, "        data->currentLazyFlags="+flagName+";");
        out(fos32, "    }");
        out(fos32, "    INCREMENT_EIP(op->len);");
        out(fos32, "}");
    }
}
