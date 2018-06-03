package x86;

import java.io.FileOutputStream;
import java.io.IOException;

public class Strings extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos_c = new FileOutputStream("normal_strings.cpp");
            FileOutputStream fos_h = new FileOutputStream("normal_strings.h");
            FileOutputStream fos_op = new FileOutputStream("normal_strings_op.h");

            fos_c.write(header.getBytes());
            fos_h.write(header.getBytes());
            fos_op.write(header.getBytes());

            out(fos_h, "#ifndef __STRINGS_OP_H__");
            out(fos_h, "#define __STRINGS_OP_H__");
            out(fos_h, "#include \"../common/cpu.h\"");
            out(fos_c, "#include \"boxedwine.h\"");

            movs(fos_c, fos_h, fos_op, fos_init, "movsb", "b", "SI", "DI", "CX", 0);
            movs(fos_c, fos_h, fos_op, fos_init, "movsw", "w", "SI", "DI", "CX", 1);
            movs(fos_c, fos_h, fos_op, fos_init, "movsd", "d", "SI", "DI", "CX", 2);

            cmps(fos_c, fos_h, fos_op, fos_init, "cmpsb", "b", "8", "SI", "DI", "CX", 0);
            cmps(fos_c, fos_h, fos_op, fos_init, "cmpsw", "w", "16", "SI", "DI", "CX", 1);
            cmps(fos_c, fos_h, fos_op, fos_init, "cmpsd", "d", "32", "SI", "DI", "CX", 2);

            stos(fos_c, fos_h, fos_op, fos_init, "stosb", "b", "DI", "CX", "AL", 0);
            stos(fos_c, fos_h, fos_op, fos_init, "stosw", "w", "DI", "CX", "AX", 1);
            stos(fos_c, fos_h, fos_op, fos_init, "stosd", "d", "DI", "CX", "EAX", 2);

            lods(fos_c, fos_h, fos_op, fos_init, "lodsb", "b", "SI", "CX", "AL", 0);
            lods(fos_c, fos_h, fos_op, fos_init, "lodsw", "w", "SI", "CX", "AX", 1);
            lods(fos_c, fos_h, fos_op, fos_init, "lodsd", "d", "SI", "CX", "EAX", 2);

            scas(fos_c, fos_h, fos_op, fos_init, "scasb", "b", "8", "AL", "DI", "CX", 0);
            scas(fos_c, fos_h, fos_op, fos_init, "scasw", "w", "16", "AX", "DI", "CX", 1);
            scas(fos_c, fos_h, fos_op, fos_init, "scasd", "d", "32", "EAX", "DI", "CX", 2);

            fos_c.close();
            out(fos_h, "#endif");
            fos_h.close();
            fos_op.close();
        } catch (IOException e) {

        }
    }

    public void movsBody(FileOutputStream fos, String name, String bits, String SI, String DI, String CX, int inc, boolean repeat) throws IOException {
        out(fos, "void "+name+"(CPU* cpu, U32 base) {");
        out(fos, "    U32 dBase = cpu->seg[ES].address;");
        out(fos, "    U32 sBase = cpu->seg[base].address;");
        out(fos, "    S32 inc = cpu->df"+(inc>0?" << "+String.valueOf(inc):"")+";");
        if (repeat) {
            out(fos, "    U32 count = " + CX + ";");
            out(fos, "    U32 i;");
            out(fos, "    for (i=0;i<count;i++) {");
        }
        out(fos, "        write"+bits+"(dBase+"+DI+", read"+bits+"(sBase+"+SI+"));");
        out(fos, "        "+DI+"+=inc;");
        out(fos, "        "+SI+"+=inc;");
        if (repeat) {
            out(fos, "    }");
            out(fos, "    " + CX + "=0;");
        }
        out(fos, "}");
    }
    public void movs(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String bits, String SI, String DI, String CX, int inc) throws  IOException {
        out(fos_h, "void "+name+"16(CPU* cpu, U32 base);");
        out(fos_h, "void "+name+"16r(CPU* cpu, U32 base);");
        out(fos_h, "void "+name+"32(CPU* cpu, U32 base);");
        out(fos_h, "void "+name+"32r(CPU* cpu, U32 base);");

        out(fos_op, "void OPCALL "+name+"_op(CPU* cpu, DecodedOp* op) {");
        out(fos_op, "    START_OP(cpu, op);");
        out(fos_op, "    if (op->ea16) {");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"16r(cpu, op->base);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"16(cpu, op->base);");
        out(fos_op, "        }");
        out(fos_op, "    } else { ");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"32r(cpu, op->base);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"32(cpu, op->base);");
        out(fos_op, "        }");
        out(fos_op, "    }");
        out(fos_op, "    NEXT();");
        out(fos_op, "}");

        movsBody(fos, name+"16", bits, SI, DI, CX, inc, false);
        movsBody(fos, name+"16r", bits, SI, DI, CX, inc, true);
        movsBody(fos, name+"32", bits, "E"+SI, "E"+DI, "E"+CX, inc, false);
        movsBody(fos, name+"32r", bits, "E"+SI, "E"+DI, "E"+CX, inc, true);

        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        out(fos_init, "INIT_CPU("+mixed+", "+name+"_op)");
    }

    public void cmpsBody(FileOutputStream fos, String name, String width, String bits, String SI, String DI, String CX, int inc, boolean repeat) throws IOException {
        out(fos, "void "+name+"(CPU* cpu, U32 rep_zero, U32 base) {");
        out(fos, "    U32 dBase = cpu->seg[ES].address;");
        out(fos, "    U32 sBase = cpu->seg[base].address;");
        out(fos, "    S32 inc = cpu->df"+(inc>0?" << "+String.valueOf(inc):"")+";");
        out(fos, "    U"+bits+" v1;");
        out(fos, "    U"+bits+" v2;");
        if (repeat) {
            out(fos, "    U32 count = " + CX + ";");
            out(fos, "    U32 i;");
            out(fos, "    if (count) {");
            out(fos, "        for (i=0;i<count;i++) {");
        }
        out(fos, "            v1 = read"+width+"(dBase+"+DI+");");
        out(fos, "            v2 = read"+width+"(sBase+"+SI+");");
        out(fos, "            "+DI+"+=inc;");
        out(fos, "            "+SI+"+=inc;");
        if (repeat) {
            out(fos, "            "+CX+"--;");
            out(fos, "            if ((v1==v2)!=rep_zero) break;");
            out(fos, "        }");
        }
        out(fos, "        cpu->dst.u"+bits+" = v2;");
        out(fos, "        cpu->src.u"+bits+" = v1;");
        out(fos, "        cpu->result.u"+bits+" = cpu->dst.u"+bits+" - cpu->src.u"+bits+";");
        out(fos, "        cpu->lazyFlags = FLAGS_SUB"+bits+";");
        if (repeat) {
            out(fos, "    }");
        }
        out(fos, "}");
    }

    public void cmps(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String width, String bits, String SI, String DI, String CX, int inc) throws  IOException {
        out(fos_h, "void "+name+"16(CPU* cpu, U32 rep_zero, U32 base);");
        out(fos_h, "void "+name+"16r(CPU* cpu, U32 rep_zero, U32 base);");
        out(fos_h, "void "+name+"32(CPU* cpu, U32 rep_zero, U32 base);");
        out(fos_h, "void "+name+"32r(CPU* cpu, U32 rep_zero, U32 base);");

        out(fos_op, "void OPCALL "+name+"_op(CPU* cpu, DecodedOp* op) {");
        out(fos_op, "    START_OP(cpu, op);");
        out(fos_op, "    if (op->ea16) {");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"16r(cpu, op->repZero, op->base);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"16(cpu, op->repZero, op->base);");
        out(fos_op, "        }");
        out(fos_op, "    } else { ");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"32r(cpu, op->repZero, op->base);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"32(cpu, op->repZero, op->base);");
        out(fos_op, "        }");
        out(fos_op, "    }");
        out(fos_op, "    NEXT();");
        out(fos_op, "}");

        cmpsBody(fos, name+"16", width, bits, SI, DI, CX, inc, false);
        cmpsBody(fos, name+"16r", width, bits, SI, DI, CX, inc, true);
        cmpsBody(fos, name+"32", width, bits, "E"+SI, "E"+DI, "E"+CX, inc, false);
        cmpsBody(fos, name+"32r", width, bits, "E"+SI, "E"+DI, "E"+CX, inc, true);

        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        out(fos_init, "INIT_CPU("+mixed+", "+name+"_op)");
    }

    public void scasBody(FileOutputStream fos, String name, String width, String bits, String AX, String DI, String CX, int inc, boolean repeat) throws IOException {
        out(fos, "void "+name+"(CPU* cpu, U32 rep_zero) {");
        out(fos, "    U32 dBase = cpu->seg[ES].address;");
        out(fos, "    S32 inc = cpu->df"+(inc>0?" << "+String.valueOf(inc):"")+";");
        out(fos, "    U"+bits+" v1;");
        if (repeat) {
            out(fos, "    U32 count = " + CX + ";");
            out(fos, "    U32 i;");
            out(fos, "    if (count) {");
            out(fos, "        for (i=0;i<count;i++) {");
        }
        out(fos, "            v1 = read"+width+"(dBase+"+DI+");");
        out(fos, "            "+DI+"+=inc;");
        if (repeat) {
            out(fos, "            "+CX+"--;");
            out(fos, "            if (("+AX+"==v1)!=rep_zero) break;");
            out(fos, "        }");
        }
        out(fos, "        cpu->dst.u"+bits+" = "+AX+";");
        out(fos, "        cpu->src.u"+bits+" = v1;");
        out(fos, "        cpu->result.u"+bits+" = "+AX+" - v1;");
        out(fos, "        cpu->lazyFlags = FLAGS_SUB"+bits+";");
        if (repeat) {
            out(fos, "    }");
        }
        out(fos, "}");
    }
    public void scas(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String width, String bits, String AX, String DI, String CX, int inc) throws  IOException {
        out(fos_h, "void "+name+"16(CPU* cpu, U32 rep_zero);");
        out(fos_h, "void "+name+"16r(CPU* cpu, U32 rep_zero);");
        out(fos_h, "void "+name+"32(CPU* cpu, U32 rep_zero);");
        out(fos_h, "void "+name+"32r(CPU* cpu, U32 rep_zero);");
        out(fos_op, "void OPCALL "+name+"_op(CPU* cpu, DecodedOp* op) {");
        out(fos_op, "    START_OP(cpu, op);");
        out(fos_op, "    if (op->ea16) {");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"16r(cpu, op->repZero);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"16(cpu, op->repZero);");
        out(fos_op, "        }");
        out(fos_op, "    } else { ");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"32r(cpu, op->repZero);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"32(cpu, op->repZero);");
        out(fos_op, "        }");
        out(fos_op, "    }");
        out(fos_op, "    NEXT();");
        out(fos_op, "}");

        scasBody(fos, name+"16", width, bits, AX, DI, CX, inc, false);
        scasBody(fos, name+"16r", width, bits, AX, DI, CX, inc, true);
        scasBody(fos, name+"32", width, bits, AX, "E"+DI, "E"+CX, inc, false);
        scasBody(fos, name+"32r", width, bits, AX, "E"+DI, "E"+CX, inc, true);

        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        out(fos_init, "INIT_CPU("+mixed+", "+name+"_op)");
    }

    public void lodsBody(FileOutputStream fos, String name, String bits, String SI, String CX, String AX, int inc, boolean repeat) throws IOException {
        out(fos, "void "+name+"(CPU* cpu, U32 base) {");
        if (repeat) {
            out(fos, "    U32 sBase = cpu->seg[base].address;");
            out(fos, "    S32 inc = cpu->df"+(inc>0?" << "+String.valueOf(inc):"")+";");
            out(fos, "    U32 count = " + CX + ";");
            out(fos, "    U32 i;");
            out(fos, "    for (i=0;i<count;i++) {");
            out(fos, "        "+AX+" = read"+bits+"(sBase+"+SI+");");
            out(fos, "        "+SI+"+=inc;");
            out(fos, "    }");
            out(fos, "    " + CX + "=0;");
        } else {
            out(fos, "    "+AX+" = read"+bits+"(cpu->seg[base].address+"+SI+");");
            out(fos, "    "+SI+"+=cpu->df"+(inc>0?" << "+String.valueOf(inc):"")+";");
        }
        out(fos, "}");
    }
    public void lods(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String bits, String SI, String CX, String AX, int inc) throws  IOException {
        out(fos_h, "void "+name+"16(CPU* cpu, U32 base);");
        out(fos_h, "void "+name+"16r(CPU* cpu, U32 base);");
        out(fos_h, "void "+name+"32(CPU* cpu, U32 base);");
        out(fos_h, "void "+name+"32r(CPU* cpu, U32 base);");
        out(fos_op, "void OPCALL "+name+"_op(CPU* cpu, DecodedOp* op) {");
        out(fos_op, "    START_OP(cpu, op);");
        out(fos_op, "    if (op->ea16) {");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"16r(cpu, op->base);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"16(cpu, op->base);");
        out(fos_op, "        }");
        out(fos_op, "    } else { ");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"32r(cpu, op->base);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"32(cpu, op->base);");
        out(fos_op, "        }");
        out(fos_op, "    }");
        out(fos_op, "    NEXT();");
        out(fos_op, "}");

        lodsBody(fos, name+"16", bits, SI, CX, AX, inc, false);
        lodsBody(fos, name+"16r", bits, SI, CX, AX, inc, true);
        lodsBody(fos, name+"32", bits, "E"+SI, "E"+CX, AX, inc, false);
        lodsBody(fos, name+"32r", bits, "E"+SI, "E"+CX, AX, inc, true);

        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        out(fos_init, "INIT_CPU("+mixed+", "+name+"_op)");
    }

    public void stosBody(FileOutputStream fos, String name, String bits, String DI, String CX, String AX, int inc, boolean repeat) throws IOException {
        out(fos, "void "+name+"(CPU* cpu) {");
        if (repeat) {
            out(fos, "    U32 dBase = cpu->seg[ES].address;");
            out(fos, "    S32 inc = cpu->df"+(inc>0?" << "+String.valueOf(inc):"")+";");
            out(fos, "    U32 count = " + CX + ";");
            out(fos, "    U32 i;");
            out(fos, "    for (i=0;i<count;i++) {");
            out(fos, "        write"+bits+"(dBase+"+DI+", "+AX+");");
            out(fos, "        "+DI+"+=inc;");
            out(fos, "    }");
            out(fos, "    " + CX + "=0;");
        } else {
            out(fos, "    write"+bits+"(cpu->seg[ES].address+"+DI+", "+AX+");");
            out(fos, "    "+DI+"+=cpu->df"+(inc>0?" << "+String.valueOf(inc):"")+";");
        }
        out(fos, "}");
    }
    public void stos(FileOutputStream fos, FileOutputStream fos_h, FileOutputStream fos_op, FileOutputStream fos_init, String name, String bits, String DI, String CX, String AX, int inc) throws  IOException {
        out(fos_h, "void "+name+"16(CPU* cpu);");
        out(fos_h, "void "+name+"16r(CPU* cpu);");
        out(fos_h, "void "+name+"32(CPU* cpu);");
        out(fos_h, "void "+name+"32r(CPU* cpu);");
        out(fos_op, "void OPCALL "+name+"_op(CPU* cpu, DecodedOp* op) {");
        out(fos_op, "    START_OP(cpu, op);");
        out(fos_op, "    if (op->ea16) {");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"16r(cpu);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"16(cpu);");
        out(fos_op, "        }");
        out(fos_op, "    } else { ");
        out(fos_op, "        if (op->repZero || op->repNotZero) {");
        out(fos_op, "            "+name+"32r(cpu);");
        out(fos_op, "        } else { ");
        out(fos_op, "            "+name+"32(cpu);");
        out(fos_op, "        }");
        out(fos_op, "    }");
        out(fos_op, "    NEXT();");
        out(fos_op, "}");

        stosBody(fos, name+"16", bits, DI, CX, AX, inc, false);
        stosBody(fos, name+"16r", bits, DI, CX, AX, inc, true);
        stosBody(fos, name+"32", bits, "E"+DI, "E"+CX, AX, inc, false);
        stosBody(fos, name+"32r", bits, "E"+DI, "E"+CX, AX, inc, true);

        String mixed = name.substring(0, 1).toUpperCase() + name.substring(1);
        out(fos_init, "INIT_CPU("+mixed+", "+name+"_op)");
    }
}
