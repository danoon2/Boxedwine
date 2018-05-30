package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/27/2018.
 */
public class Xchg extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_xchg.h");
            fos.write(header.getBytes());
            generateAll(fos, fos_init);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void xchg8(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String load, String store, String storeEnd) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    U8 tmp = "+load+";");
        out(fos, "    "+store+"*cpu->reg8[op->reg]"+storeEnd+";");
        out(fos, "    *cpu->reg8[op->reg] = tmp;");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }

    public void xchg16(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String load, String store, String storeEnd) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    U16 tmp = "+load+";");
        out(fos, "    "+store+"cpu->reg[op->reg].u16"+storeEnd+";");
        out(fos, "    cpu->reg[op->reg].u16 = tmp;");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }

    public void xchg32(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String load, String store, String storeEnd) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    U32 tmp = "+load+";");
        out(fos, "    "+store+"cpu->reg[op->reg].u32"+storeEnd+";");
        out(fos, "    cpu->reg[op->reg].u32 = tmp;");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }

    public void cmpxchg16(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String local, String load, String store) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        if (local!=null)
            out(fos, "    "+local);
        out(fos, "    cpu->dst.u16 = AX;");
        out(fos, "    cpu->src.u16 = "+load+";");
        out(fos, "    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;");
        out(fos, "    cpu->lazyFlags = FLAGS_CMP16;");
        out(fos, "    if (AX == cpu->src.u16) {");
        out(fos, "        "+store+";");
        out(fos, "    } else {");
        out(fos, "        AX = cpu->src.u16;");
        out(fos, "    }");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }

    public void cmpxchg32(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String local, String load, String store) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        if (local!=null)
            out(fos, "    "+local);
        out(fos, "    cpu->dst.u32 = EAX;");
        out(fos, "    cpu->src.u32 = "+load+";");
        out(fos, "    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;");
        out(fos, "    cpu->lazyFlags = FLAGS_CMP32;");
        out(fos, "    if (EAX == cpu->src.u32) {");
        out(fos, "        "+store+";");
        out(fos, "    } else {");
        out(fos, "        EAX = cpu->src.u32;");
        out(fos, "    }");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        xchg8(fos, fos_init, "xchgr8r8", "XchgR8R8","*cpu->reg8[op->rm]", "*cpu->reg8[op->rm] = ", "");
        xchg8(fos, fos_init, "xchge8r8", "XchgE8R8", "readb(eaa(cpu, op))", "writeb(eaa(cpu, op), ", ")");
        xchg16(fos, fos_init, "xchgr16r16", "XchgR16R16", "cpu->reg[op->rm].u16", "cpu->reg[op->rm].u16 = ", "");
        xchg16(fos, fos_init, "xchge16r16", "XchgE16R16", "readw(eaa(cpu, op))", "writew(eaa(cpu, op), ", ")");
        xchg32(fos, fos_init, "xchgr32r32", "XchgR32R32", "cpu->reg[op->rm].u32", "cpu->reg[op->rm].u32 = ", "");
        xchg32(fos, fos_init, "xchge32r32", "XchgE32R32", "readd(eaa(cpu, op))", "writed(eaa(cpu, op), ", ")");

        cmpxchg16(fos, fos_init, "cmpxchgr16r16", "CmpXchgR16R16", null, "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16");
        cmpxchg16(fos, fos_init, "cmpxchge16r16", "CmpXchgE16R16", "U32 address = eaa(cpu, op);", "readw(address)", "writew(address, cpu->reg[op->reg].u16)");
        cmpxchg32(fos, fos_init, "cmpxchgr32r32", "CmpXchgR32R32", null, "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32");
        cmpxchg32(fos, fos_init, "cmpxchge32r32", "CmpXchgE32R32", "U32 address = eaa(cpu, op);", "readd(address)", "writed(address, cpu->reg[op->reg].u32)");
    }
}
