package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/30/2018.
 */
public class Move extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_move.h");
            fos.write(header.getBytes());
            generateAll(fos, fos_init);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void mov(FileOutputStream fos, FileOutputStream fos_init, String functionName, String enumName, String assign) throws IOException {
        out(fos, "void OPCALL normal_"+functionName+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    "+assign+";");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+enumName+", normal_"+functionName+")");
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        mov(fos, fos_init, "movr8r8", "MovR8R8", "*cpu->reg8[op->reg] = *cpu->reg8[op->rm]");
        mov(fos, fos_init, "move8r8", "MovE8R8", "writeb(eaa(cpu, op), *cpu->reg8[op->reg])");
        mov(fos, fos_init, "movr8e8", "MovR8E8", "*cpu->reg8[op->reg] = readb(eaa(cpu, op))");
        mov(fos, fos_init, "movr8", "MovR8I8", "*cpu->reg8[op->reg] = (U8)op->imm;");
        mov(fos, fos_init, "move8", "MovE8I8", "writeb(eaa(cpu, op), (U8)op->imm)");

        mov(fos, fos_init, "movr16r16", "MovR16R16", "cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16");
        mov(fos, fos_init, "move16r16", "MovE16R16", "writew(eaa(cpu, op), cpu->reg[op->reg].u16)");
        mov(fos, fos_init, "movr16e16", "MovR16E16", "cpu->reg[op->reg].u16 = readw(eaa(cpu, op))");
        mov(fos, fos_init, "movr16", "MovR16I16", "cpu->reg[op->reg].u16 = (U16)op->imm");
        mov(fos, fos_init, "move16", "MovE16I16", "writew(eaa(cpu, op), (U16)op->imm)");

        mov(fos, fos_init, "movr32r32", "MovR32R32", "cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32");
        mov(fos, fos_init, "move32r32", "MovE32R32", "writed(eaa(cpu, op), cpu->reg[op->reg].u32)");
        mov(fos, fos_init, "movr32e32", "MovR32E32", "cpu->reg[op->reg].u32 = readd(eaa(cpu, op))");
        mov(fos, fos_init, "movr32", "MovR32I32", "cpu->reg[op->reg].u32 = op->imm");
        mov(fos, fos_init, "move32", "MovE32I32", "writed(eaa(cpu, op), op->imm)");

        mov(fos, fos_init, "movr16s16", "MovR16S16", "cpu->reg[op->reg].u16 = cpu->seg[op->rm].value;");
        mov(fos, fos_init, "movr32s16", "MovR32S16", "cpu->reg[op->reg].u32 = cpu->seg[op->rm].value");
        mov(fos, fos_init, "move16s16", "MovE16S16", "writew(eaa(cpu, op), cpu->seg[op->reg].value)");
        mov(fos, fos_init, "movs16e16", "MovS16E16", "if (cpu->setSegment(op->reg, readw(eaa(cpu, op)))) {cpu->eip.u32+=op->len;}");
        mov(fos, fos_init, "movs16r16", "MovS16R16", "if (cpu->setSegment(op->rm, cpu->reg[op->reg].u16)) {cpu->eip.u32+=op->len;}");

        mov(fos, fos_init, "movAlOb", "MovAlOb", "AL = readb(cpu->seg[op->base].address+op->disp)");
        mov(fos, fos_init, "movAxOw", "MovAxOw", "AX = readw(cpu->seg[op->base].address+op->disp)");
        mov(fos, fos_init, "movEaxOd", "MovEaxOd", "EAX = readd(cpu->seg[op->base].address+op->disp)");

        mov(fos, fos_init, "movObAl", "MovObAl", "writeb(cpu->seg[op->base].address+op->disp, AL)");
        mov(fos, fos_init, "movOwAx", "MovOwAx", "writew(cpu->seg[op->base].address+op->disp, AX)");
        mov(fos, fos_init, "movOdEax", "MovOdEax", "writed(cpu->seg[op->base].address+op->disp, EAX)");

        mov(fos, fos_init, "movGwXzR8", "MovGwXzR8", "cpu->reg[op->reg].u16 = *cpu->reg8[op->rm]");
        mov(fos, fos_init, "movGwXzE8", "MovGwXzE8", "cpu->reg[op->reg].u16 = readb(eaa(cpu, op))");
        mov(fos, fos_init, "movGwSxR8", "MovGwSxR8", "cpu->reg[op->reg].u16 = (S8)*cpu->reg8[op->rm]");
        mov(fos, fos_init, "movGwSxE8", "MovGwSxE8", "cpu->reg[op->reg].u16 = (S8)readb(eaa(cpu, op))");

        mov(fos, fos_init, "movGdXzR8", "MovGdXzR8", "cpu->reg[op->reg].u32 = *cpu->reg8[op->rm]");
        mov(fos, fos_init, "movGdXzE8", "MovGdXzE8", "cpu->reg[op->reg].u32 = readb(eaa(cpu, op))");
        mov(fos, fos_init, "movGdSxR8", "MovGdSxR8", "cpu->reg[op->reg].u32 = (S8)(*cpu->reg8[op->rm])");
        mov(fos, fos_init, "movGdSxE8", "MovGdSxE8", "cpu->reg[op->reg].u32 = (S8)readb(eaa(cpu, op))");

        mov(fos, fos_init, "movGdXzR16", "MovGdXzR16", "cpu->reg[op->reg].u32 = cpu->reg[op->rm].u16");
        mov(fos, fos_init, "movGdXzE16", "MovGdXzE16", "cpu->reg[op->reg].u32 = readw(eaa(cpu, op))");
        mov(fos, fos_init, "movGdSxR16", "MovGdSxR16", "cpu->reg[op->reg].u32 = (S16)cpu->reg[op->rm].u16");
        mov(fos, fos_init, "movGdSxE16", "MovGdSxE16", "cpu->reg[op->reg].u32 = (S16)readw(eaa(cpu, op))");

        mov(fos, fos_init, "leaR16", "LeaR16", "cpu->reg[op->reg].u16 = eaa(cpu, op)");
        mov(fos, fos_init, "leaR32", "LeaR32", "cpu->reg[op->reg].u32 = eaa(cpu, op)");
    }
}
