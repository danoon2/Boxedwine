package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class Arith extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_arith.h");
            fos.write(header.getBytes());
            generateAll(fos, fos_init);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        arith(fos, fos_init, "add", "+", false, true, true, true);
        arith(fos, fos_init, "or", "|", false, true, true, true);
        arith(fos, fos_init, "adc", "+", true, true, true, true);
        arith(fos, fos_init, "sbb", "-", true, true, true, true);
        arith(fos, fos_init, "and", "&", false, true, true, true);
        arith(fos, fos_init, "sub", "-", false, true, true, true);
        arith(fos, fos_init, "xor", "^", false, true, true, true);
        arith(fos, fos_init, "cmp", "-", false, false, true, true);
        arith(fos, fos_init, "test", "&", false, false, true, true);
        arithSingle(fos, fos_init, "not", "~", false, true, false);
        arithSingle(fos, fos_init, "neg", "-", false, true, true);

        mul8(fos, fos_init);
        mul16(fos, fos_init);
        mul32(fos, fos_init);

        div8(fos, fos_init);
        div16(fos, fos_init);
        div32(fos, fos_init);

        dmul16(fos, fos_init, "dimulcr16r16", "ImulR16R16", "cpu->reg[op->rm].u16", "op->imm");
        dmul16(fos, fos_init, "dimulcr16e16", "ImulR16E16", "readw(eaa(cpu, op))", "op->imm");
        dmul32(fos, fos_init, "dimulcr32r32", "ImulR32R32", "cpu->reg[op->rm].u32", "op->imm");
        dmul32(fos, fos_init, "dimulcr32e32", "ImulR32E32", "readd(eaa(cpu, op))", "op->imm");

        dmul16(fos, fos_init, "dimulr16r16", "DimulR16R16", "cpu->reg[op->rm].u16", "cpu->reg[op->reg].u16");
        dmul16(fos, fos_init, "dimulr16e16", "DimulR16E16", "readw(eaa(cpu, op))", "cpu->reg[op->reg].u16");
        dmul32(fos, fos_init, "dimulr32r32", "DimulR32R32", "cpu->reg[op->rm].u32", "cpu->reg[op->reg].u32");
        dmul32(fos, fos_init, "dimulr32e32", "DimulR32E32", "readd(eaa(cpu, op))", "cpu->reg[op->reg].u32");
    }

    public void dmul16(FileOutputStream fos, FileOutputStream fos_init, String name, String ename, String arg1, String arg2) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    S32 res=(S16)("+arg1+") * (S32)((S16)"+arg2+");");
        out(fos, "    cpu->fillFlagsNoCFOF();");
        out(fos, "    if ((res >= -32767) && (res <= 32767)) {");
        out(fos, "        cpu->removeFlag(CF|OF);");
        out(fos, "    } else {");
        out(fos, "        cpu->addFlag(CF|OF);");
        out(fos, "    }");
        out(fos, "    cpu->reg[op->reg].u16 = (U16)res;");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");
    }

    public void dmul32(FileOutputStream fos, FileOutputStream fos_init, String name, String ename, String arg1, String arg2) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    S64 res=(S32)("+arg1+") * (S64)((S32)"+arg2+");");
        out(fos, "    cpu->fillFlagsNoCFOF();");
        out(fos, "    if (res>=-2147483647l && res<=2147483647l) {");
        out(fos, "        cpu->removeFlag(CF|OF);");
        out(fos, "    } else {");
        out(fos, "        cpu->addFlag(CF|OF);");
        out(fos, "    }");
        out(fos, "    cpu->reg[op->reg].u32 = (U32)res;");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");
    }

    public void divInternal(FileOutputStream fos, FileOutputStream fos_init, String func, String name, String ename, String source) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    if ("+func+"(cpu, "+source+")) cpu->eip.u32+=op->len;");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");
    }

    public void div8(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        divInternal(fos, fos_init, "div8", "divR8", "DivR8", "*cpu->reg8[op->reg]");
        divInternal(fos, fos_init, "div8", "divE8", "DivE8", "readb(eaa(cpu, op))");
        divInternal(fos, fos_init, "idiv8", "idivR8", "IDivR8", "*cpu->reg8[op->reg]");
        divInternal(fos, fos_init, "idiv8", "idivE8", "IDivE8", "readb(eaa(cpu, op))");
    }

    public void div16(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        divInternal(fos, fos_init, "div16", "divR16", "DivR16", "cpu->reg[op->reg].u16");
        divInternal(fos, fos_init, "div16", "divE16", "DivE16", "readw(eaa(cpu, op))");
        divInternal(fos, fos_init, "idiv16", "idivR16", "IDivR16", "cpu->reg[op->reg].u16");
        divInternal(fos, fos_init, "idiv16", "idivE16", "IDivE16", "readw(eaa(cpu, op))");
    }

    public void div32(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        divInternal(fos, fos_init, "div32", "divR32", "DivR32", "cpu->reg[op->reg].u32");
        divInternal(fos, fos_init, "div32", "divE32", "DivE32", "readd(eaa(cpu, op))");
        divInternal(fos, fos_init, "idiv32", "idivR32", "IDivR32", "cpu->reg[op->reg].u32");
        divInternal(fos, fos_init, "idiv32", "idivE32", "IDivE32", "readd(eaa(cpu, op))");
    }

    public void mul8internal(FileOutputStream fos, FileOutputStream fos_init, String name, String source, boolean signed) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        if (signed) {
            out(fos, "    AX = (S16)((S8)AL) * (S8)("+source+");");
            out(fos, "    if ((S16)AX<-128 || (S16)AX>127) {");
            out(fos, "        cpu->flags|=CF|OF;");
            out(fos, "    } else {");
            out(fos, "        cpu->flags&=~(CF|OF);");
            out(fos, "    }");
        } else {
            out(fos, "    AX = AL * " + source + ";");
            out(fos, "    if (AH) {");
            out(fos, "        cpu->flags|=CF|OF;");
            out(fos, "    } else {");
            out(fos, "        cpu->flags&=~(CF|OF);");
            out(fos, "    }");
        }
        out(fos, "}");

        String mixed;
        if (signed) {
            mixed = name.substring(0, 2).toUpperCase() + name.substring(2);
        } else {
            mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        }
        out(fos_init, "INIT_CPU(" + mixed + ", " + name + ")");
    }

    public void mul8(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        mul8internal(fos, fos_init, "mulR8", "(*cpu->reg8[op->reg])", false);
        mul8internal(fos, fos_init, "mulE8", "readb(eaa(cpu, op))", false);
        mul8internal(fos, fos_init, "imulR8", "(*cpu->reg8[op->reg])", true);
        mul8internal(fos, fos_init, "imulE8", "readb(eaa(cpu, op))", true);
    }

    public void mul16internal(FileOutputStream fos, FileOutputStream fos_init, String name, String source, boolean signed) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        if (signed) {
            out(fos, "    S32 result = (S32)((S16)AX) * ((S16)("+source+"));");
            out(fos, "    cpu->fillFlagsNoCFOF();");
            out(fos, "    AX = (U16)result;");
            out(fos, "    DX = (U16)(result >> 16);");
            out(fos, "    if (result>32767 || result<-32768) {");
            out(fos, "        cpu->flags|=CF|OF;");
            out(fos, "    } else {");
            out(fos, "        cpu->flags&=~(CF|OF);");
            out(fos, "    }");
        } else {
            out(fos, "    U32 result = (U32)AX * "+source+";");
            out(fos, "    cpu->fillFlagsNoCFOF();");
            out(fos, "    AX = (U16)result;");
            out(fos, "    DX = (U16)(result >> 16);");
            out(fos, "    if (DX) {");
            out(fos, "        cpu->flags|=CF|OF;");
            out(fos, "    } else {");
            out(fos, "        cpu->flags&=~(CF|OF);");
            out(fos, "    }");
        }
        out(fos, "}");

        String mixed;
        if (signed) {
            mixed = name.substring(0, 2).toUpperCase() + name.substring(2);
        } else {
            mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        }
        out(fos_init, "INIT_CPU(" + mixed + ", " + name + ")");
    }

    public void mul16(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        mul16internal(fos, fos_init, "mulR16", "cpu->reg[op->reg].u16", false);
        mul16internal(fos, fos_init, "mulE16", "readw(eaa(cpu, op))", false);
        mul16internal(fos, fos_init, "imulR16", "cpu->reg[op->reg].u16", true);
        mul16internal(fos, fos_init, "imulE16", "readw(eaa(cpu, op))", true);
    }

    public void mul32internal(FileOutputStream fos, FileOutputStream fos_init, String name, String source, boolean signed) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        if (signed) {
            out(fos, "    S64 result = (S64)((S32)EAX) * ((S32)("+source+"));");
            out(fos, "    cpu->fillFlagsNoCFOF();");
            out(fos, "    EAX = (U32)result;");
            out(fos, "    EDX = (U32)(result >> 32);");
            out(fos, "    if (result>0x7fffffffl || result<-0x7fffffffl) {");
            out(fos, "        cpu->flags|=CF|OF;");
            out(fos, "    } else {");
            out(fos, "        cpu->flags&=~(CF|OF);");
            out(fos, "    }");
        } else {
            out(fos, "    U64 result = (U64)EAX * "+source+";");
            out(fos, "    cpu->fillFlagsNoCFOF();");
            out(fos, "    EAX = (U32)result;");
            out(fos, "    EDX = (U32)(result >> 32);");
            out(fos, "    if (EDX) {");
            out(fos, "        cpu->flags|=CF|OF;");
            out(fos, "    } else {");
            out(fos, "        cpu->flags&=~(CF|OF);");
            out(fos, "    }");
        }
        out(fos, "}");

        String mixed;
        if (signed) {
            mixed = name.substring(0, 2).toUpperCase() + name.substring(2);
        } else {
            mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        }
        out(fos_init, "INIT_CPU(" + mixed + ", " + name + ")");
    }

    public void mul32(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        mul32internal(fos, fos_init, "mulR32", "cpu->reg[op->reg].u32", false);
        mul32internal(fos, fos_init, "mulE32", "readd(eaa(cpu, op))", false);
        mul32internal(fos, fos_init, "imulR32", "cpu->reg[op->reg].u32", true);
        mul32internal(fos, fos_init, "imulE32", "readd(eaa(cpu, op))", true);
    }

    public void arith(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, boolean flags, boolean iform)  throws IOException {
        arith8(fos, fos_init, name, op, cf, result, "8", flags, iform);
        arith16(fos, fos_init, name, op, cf, result, "16", flags, iform);
        arith32(fos, fos_init, name, op, cf, result, "32", flags, iform);
    }

    public void arith32(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, String bits, boolean flags, boolean iform)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, mixed + "R32R32", name + "r32r32", "FLAGS_" + name.toUpperCase() + bits, "cpu->reg[op->rm].u32", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags);
        arithbase(fos, fos_init, mixed + "E32R32", name + "e32r32", "FLAGS_" + name.toUpperCase() + bits, "cpu->reg[op->reg].u32", "readd(eaa)", "writed(eaa, ", ")", op, cf, result, true, bits, flags);
        arithbase(fos, fos_init, mixed + "R32E32", name + "r32e32", "FLAGS_" + name.toUpperCase() + bits, "readd(eaa(cpu, op))", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags);
        if (iform) {
            arithbase(fos, fos_init, mixed + "R32I32", name + "32_reg", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "cpu->reg[op->reg].u32", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags);
            arithbase(fos, fos_init, mixed + "E32I32", name + "32_mem", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "readd(eaa)", "writed(eaa, ", ")", op, cf, result, true, bits, flags);
        }
    }

    public void arith16(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, String bits, boolean flags, boolean iform)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, mixed + "R16R16", name+"r16r16", "FLAGS_"+name.toUpperCase()+bits, "cpu->reg[op->rm].u16", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags);
        arithbase(fos, fos_init, mixed + "E16R16", name+"e16r16", "FLAGS_"+name.toUpperCase()+bits, "cpu->reg[op->reg].u16", "readw(eaa)", "writew(eaa, ", ")", op, cf, result, true, bits, flags);
        arithbase(fos, fos_init, mixed + "R16E16", name+"r16e16", "FLAGS_"+name.toUpperCase()+bits, "readw(eaa(cpu, op))", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags);
        if (iform) {
            arithbase(fos, fos_init, mixed + "R16I16", name + "16_reg", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "cpu->reg[op->reg].u16", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags);
            arithbase(fos, fos_init, mixed + "E16I16", name + "16_mem", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "readw(eaa)", "writew(eaa, ", ")", op, cf, result, true, bits, flags);
        }
    }

    public void arith8(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, String bits, boolean flags, boolean iform)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, mixed + "R8R8", name+"r8r8", "FLAGS_"+name.toUpperCase()+bits, "*cpu->reg8[op->rm]", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags);
        arithbase(fos, fos_init, mixed + "E8R8", name+"e8r8", "FLAGS_"+name.toUpperCase()+bits, "*cpu->reg8[op->reg]", "readb(eaa)", "writeb(eaa, ", ")", op, cf, result, true, bits, flags);
        arithbase(fos, fos_init, mixed + "R8E8", name+"r8e8", "FLAGS_"+name.toUpperCase()+bits, "readb(eaa(cpu, op))", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags);
        if (iform) {
            arithbase(fos, fos_init, mixed + "R8I8", name + "8_reg", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "*cpu->reg8[op->reg]", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags);
            arithbase(fos, fos_init, mixed + "E8I8", name + "8_mem", "FLAGS_" + name.toUpperCase() + bits, "op->imm", "readb(eaa)", "writeb(eaa, ", ")", op, cf, result, true, bits, flags);
        }
    }

    public void arithSingle(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, boolean flags)  throws IOException {
        arithSingle8(fos, fos_init, name, op, cf, result, "8", flags);
        arithSingle16(fos, fos_init, name, op, cf, result, "16", flags);
        arithSingle32(fos, fos_init, name, op, cf, result, "32", flags);
    }

    public void arithSingle32(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, String bits, boolean flags)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, mixed + "R32", name + "r32", "FLAGS_" + name.toUpperCase() + bits, "cpu->reg[op->reg].u32", flags?"0":"", "cpu->reg[op->reg].u32 = ", "", op, cf, result, false, bits, flags);
        arithbase(fos, fos_init, mixed + "E32", name + "e32", "FLAGS_" + name.toUpperCase() + bits, "readd(eaa)", flags?"0":"", "writed(eaa, ", ")", op, cf, result, true, bits, flags);
    }

    public void arithSingle16(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, String bits, boolean flags)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, mixed + "R16", name+"r16", "FLAGS_"+name.toUpperCase()+bits, "cpu->reg[op->reg].u16", flags?"0":"", "cpu->reg[op->reg].u16 = ", "", op, cf, result, false, bits, flags);
        arithbase(fos, fos_init, mixed + "E16", name+"e16", "FLAGS_"+name.toUpperCase()+bits, "readw(eaa)", flags?"0":"", "writew(eaa, ", ")", op, cf, result, true, bits, flags);
    }

    public void arithSingle8(FileOutputStream fos, FileOutputStream fos_init, String name, String op, boolean cf, boolean result, String bits, boolean flags)  throws IOException {
        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);

        arithbase(fos, fos_init, mixed + "R8", name+"r8", "FLAGS_"+name.toUpperCase()+bits, "*cpu->reg8[op->reg]", flags?"0":"", "*cpu->reg8[op->reg] = ", "", op, cf, result, false, bits, flags);
        arithbase(fos, fos_init, mixed + "E8", name+"e8", "FLAGS_"+name.toUpperCase()+bits, "readb(eaa)", flags?"0":"", "writeb(eaa, ", ")", op, cf, result, true, bits, flags);
    }

    public void arithbase(FileOutputStream fos, FileOutputStream fos_init, String enumName, String functionName, String flagName, String source, String loadDest, String saveDest1, String saveDest2, String op, boolean cf, boolean result, boolean eaa, String bits, boolean flags)  throws IOException {
        out(fos, "void OPCALL "+functionName+"(CPU* cpu, DecodedOp* op) {");
        if (eaa)
            out(fos, "    U32 eaa = eaa(cpu, op);");
        if (cf)
            out(fos, "    cpu->oldCF = cpu->getCF();");
        if (flags) {
            out(fos, "    cpu->dst.u" + bits + " = " + loadDest + ";");
            out(fos, "    cpu->src.u" + bits + " = " + source + ";");
            out(fos, "    cpu->result.u" + bits + " = cpu->dst.u" + bits + " " + op + " cpu->src.u" + bits + (cf ? " " + op + " cpu->oldCF;" : ";"));
            out(fos, "    cpu->lazyFlags = " + flagName + ";");
            if (result)
                out(fos, "    " + saveDest1 + " cpu->result.u" + bits + saveDest2 + ";");
        } else {
            out(fos, "    "+saveDest1+loadDest+op+source+saveDest2+";");
        }
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", "+functionName+")");
    }
}
