package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/27/2018.
 */
public class Bit extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_bit.h");
            fos.write(header.getBytes());
            generateAll(fos, fos_init);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void bitR16(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String op, boolean imm) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        if (imm) {
            out(fos, "    U16 mask = (U16)op->imm;");
        } else {
            out(fos, "    U16 mask=1 << (cpu->reg[op->rm].u16 & 15);");
        }
        out(fos, "    cpu->fillFlagsNoCF();");
        out(fos, "    cpu->setCF(cpu->reg[op->reg].u16 & mask);");
        if (op.length()>0) {
            out(fos, "    "+op);
        }
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        if (!imm) {
            bitR16(fos, fos_init, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bitE16(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String op, boolean imm) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        if (imm) {
            out(fos, "    U16 mask = (U16)op->imm;");
        } else {
            out(fos, "    U16 mask=1 << (cpu->reg[op->reg].u16 & 15);");
        }
        out(fos, "    U32 address = eaa(cpu, op);");
        out(fos, "    U16 value;");
        out(fos, "    cpu->fillFlagsNoCF();");
        if (!imm) {
            out(fos, "    address+=(((S16)cpu->reg[op->reg].u16)>>4)*2;");
        }
        out(fos, "    value = readw(address);");
        out(fos, "    cpu->setCF(value & mask);");
        if (op.length()>0) {
            out(fos, "    "+op);
        }
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        if (!imm) {
            bitE16(fos, fos_init, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bitR32(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String op, boolean imm) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        if (imm) {
            out(fos, "    U32 mask = op->imm;");
        } else {
            out(fos, "    U32 mask=1 << (cpu->reg[op->rm].u32 & 31);");
        }
        out(fos, "    cpu->fillFlagsNoCF();");
        out(fos, "    cpu->setCF(cpu->reg[op->reg].u32 & mask);");
        if (op.length()>0) {
            out(fos, "    "+op);
        }
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        if (!imm) {
            bitR32(fos, fos_init, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bitE32(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String op, boolean imm) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        if (imm) {
            out(fos, "    U32 mask = op->imm;");
        } else {
            out(fos, "    U32 mask=1 << (cpu->reg[op->reg].u32 & 31);");
        }
        out(fos, "    U32 address = eaa(cpu, op);");
        out(fos, "    U32 value;");
        out(fos, "    cpu->fillFlagsNoCF();");
        if (!imm) {
            out(fos, "    address+=(((S32)cpu->reg[op->reg].u32)>>5)*4;");
        }
        out(fos, "    value = readd(address);");
        out(fos, "    cpu->setCF(value & mask);");
        if (op.length()>0) {
            out(fos, "    "+op);
        }
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");

        if (!imm) {
            bitE32(fos, fos_init, functionName.substring(0, functionName.length()-3), enumName.substring(0, enumName.length()-3), op, true);
        }
    }

    public void bsf(FileOutputStream fos, FileOutputStream fos_init, String bits, String functionName, String enumName, String load) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    U"+bits+" value="+load+";");
        out(fos, "    cpu->fillFlagsNoZF();");
        out(fos, "    if (value==0) {");
        out(fos, "        cpu->addZF();");
        out(fos, "    } else {");
        out(fos, "        U"+bits+" result = 0;");
        out(fos, "        while ((value & 0x01)==0) { result++; value>>=1; }");
        out(fos, "        cpu->removeZF();");
        out(fos, "        cpu->reg[op->reg].u"+bits+"=result;");
        out(fos, "    }");
        out(fos, "}");
        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }

    public void bsr(FileOutputStream fos, FileOutputStream fos_init, String bits, String functionName, String enumName, String load) throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    U"+bits+" value="+load+";");
        out(fos, "    cpu->fillFlagsNoZF();");
        out(fos, "    if (value==0) {");
        out(fos, "        cpu->addZF();");
        out(fos, "    } else {");
        if (bits.equals("16")) {
            out(fos, "        U16 result = 15;");
            out(fos, "        while ((value & 0x8000)==0) { result--; value<<=1; }");
        } else {
            out(fos, "        U32 result = 31;");
            out(fos, "        while ((value & 0x80000000)==0) { result--; value<<=1; }");
        }
        out(fos, "        cpu->removeZF();");
        out(fos, "        cpu->reg[op->reg].u"+bits+"=result;");
        out(fos, "    }");
        out(fos, "}");
        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        bitR16(fos, fos_init, "btr16r16", "BtR16R16","", false);
        bitE16(fos, fos_init, "bte16r16", "BtE16R16","", false);
        bitR32(fos, fos_init, "btr32r32", "BtR32R32","", false);
        bitE32(fos, fos_init, "bte32r32", "BtE32R32","", false);

        bitR16(fos, fos_init, "btsr16r16", "BtsR16R16","cpu->reg[op->reg].u16 |= mask;", false);
        bitE16(fos, fos_init, "btse16r16", "BtsE16R16","writew(address, value | mask);", false);
        bitR32(fos, fos_init, "btsr32r32", "BtsR32R32","cpu->reg[op->reg].u32 |= mask;", false);
        bitE32(fos, fos_init, "btse32r32", "BtsE32R32","writed(address, value | mask);", false);

        bitR16(fos, fos_init, "btrr16r16", "BtrR16R16","cpu->reg[op->reg].u16 &= ~mask;", false);
        bitE16(fos, fos_init, "btre16r16", "BtrE16R16","writew(address, value & ~mask);", false);
        bitR32(fos, fos_init, "btrr32r32", "BtrR32R32","cpu->reg[op->reg].u32 &= ~mask;", false);
        bitE32(fos, fos_init, "btre32r32", "BtrE32R32","writed(address, value & ~mask);", false);

        bitR16(fos, fos_init, "btcr16r16", "BtcR16R16","cpu->reg[op->reg].u16 ^= mask;", false);
        bitE16(fos, fos_init, "btce16r16", "BtcE16R16","writew(address, value ^ mask);", false);
        bitR32(fos, fos_init, "btcr32r32", "BtcR32R32","cpu->reg[op->reg].u32 ^= mask;", false);
        bitE32(fos, fos_init, "btce32r32", "BtcE32R32","writed(address, value ^ mask);", false);

        bsf(fos, fos_init, "16", "bsfr16r16", "BsfR16R16", "cpu->reg[op->rm].u16");
        bsf(fos, fos_init, "16", "bsfr16e16", "BsfR16E16", "readw(eaa(cpu, op))");
        bsf(fos, fos_init, "32", "bsfr32r32", "BsfR32R32", "cpu->reg[op->rm].u32");
        bsf(fos, fos_init, "32", "bsfr32e32", "BsfR32E32", "readd(eaa(cpu, op))");

        bsr(fos, fos_init, "16", "bsrr16r16", "BsrR16R16", "cpu->reg[op->rm].u16");
        bsr(fos, fos_init, "16", "bsrr16e16", "BsrR16E16", "readw(eaa(cpu, op))");
        bsr(fos, fos_init, "32", "bsrr32r32", "BsrR32R32", "cpu->reg[op->rm].u32");
        bsr(fos, fos_init, "32", "bsrr32e32", "BsrR32E32", "readd(eaa(cpu, op))");
    }
}
