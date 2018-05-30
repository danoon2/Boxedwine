package x86;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by James on 3/30/2018.
 */
public class Other extends Base {
    public void generate(FileOutputStream fos_init) {
        try {
            FileOutputStream fos = new FileOutputStream("normal_other.h");
            fos.write(header.getBytes());
            generateAll(fos, fos_init);
            fos.close();
        } catch (IOException e) {

        }
    }

    public void generateAll(FileOutputStream fos, FileOutputStream fos_init) throws IOException {
        bound(fos, fos_init, "16", "w", "2");
        bound(fos, fos_init, "32", "d", "4");

        inst(fos, fos_init, "normal_daa", "Daa", "daa(cpu);");
        inst(fos, fos_init, "normal_das", "Das", "das(cpu);");
        inst(fos, fos_init, "normal_aaa", "Aaa", "aaa(cpu);");
        inst(fos, fos_init, "normal_aas", "Aas", "aas(cpu);");
        inst(fos, fos_init, "normal_aad", "Aad", "aad(cpu, op->imm);");
        inst(fos, fos_init, "normal_aam", "Aam", "if (aam(cpu, op->imm)) {cpu->eip.u32+=op->len;}");

        inst(fos, fos_init, "normal_nop", "Nop", "");
        inst(fos, fos_init, "normal_wait", "Wait", "");
        inst(fos, fos_init, "normal_cwd", "Cwd", "if (((S16)AX) < 0) DX = 0xFFFF; else  DX = 0;");
        inst(fos, fos_init, "normal_cwq", "Cwq", "if (((S32)EAX) < 0) EDX = 0xFFFFFFFF; else EDX = 0;");

        inst(fos, fos_init, "normal_callAp", "CallAp", "cpu->call(0, op->disp, op->imm, cpu->eip.u32+op->len);");
        inst(fos, fos_init, "normal_callFar", "CallFar", "cpu->call(1, op->disp, op->imm, cpu->eip.u32+op->len);");
        inst(fos, fos_init, "normal_jmpAp", "JmpAp", "cpu->jmp(0, op->disp, op->imm, cpu->eip.u32+op->len);");
        inst(fos, fos_init, "normal_jmpFar", "JmpFar", "cpu->jmp(1, op->disp, op->imm, cpu->eip.u32+op->len);");
        inst(fos, fos_init, "normal_retf16", "Retf16", "cpu->eip.u32+=op->len; cpu->ret(0, op->imm);");
        inst(fos, fos_init, "normal_retf32", "Retf32", "cpu->eip.u32+=op->len; cpu->ret(1, op->imm);");
        inst(fos, fos_init, "normal_iret", "Iret", "cpu->iret(0, cpu->eip.u32+op->len);");
        inst(fos, fos_init, "normal_iret32", "Iret32", "cpu->iret(1, cpu->eip.u32+op->len);");

        inst(fos, fos_init, "normal_sahf", "Sahf", "cpu->fillFlags(); cpu->setFlags(AH, FMASK_ALL & 0xFF);");
        inst(fos, fos_init, "normal_lahf", "Lahf", "cpu->fillFlags(); AH = (cpu->flags & (SF|ZF|AF|PF|CF)) | 2;");
        inst(fos, fos_init, "normal_salc", "Salc", "if (cpu->getCF()) AL = 0xFF; else AL = 0;");

        inst(fos, fos_init, "normal_retn16Iw", "Retn16Iw", "U16 eip = cpu->pop16();\r\n    SP = SP+op->imm;\r\n    cpu->eip.u32 = eip;");
        inst(fos, fos_init, "normal_retn32Iw", "Retn32Iw", "U32 eip = cpu->pop32();\r\n    ESP = ESP+op->imm;\r\n    cpu->eip.u32 = eip;");

        inst(fos, fos_init, "normal_retn16", "Retn16", "cpu->eip.u32 = cpu->pop16();");
        inst(fos, fos_init, "normal_retn32", "Retn32", "cpu->eip.u32 = cpu->pop32();");

        inst(fos, fos_init, "normal_invalid", "Invalid", "kpanic(\"Invalid instruction %x\\n\", op->inst);");

        inst(fos, fos_init, "normal_int80", "Int80", "ksyscall(cpu, op->len);");
        inst(fos, fos_init, "normal_int98", "Int98", "U32 index = cpu->peek32(0); if (index<wine_callbackSize && wine_callback[index]) {wine_callback[index](cpu);} else {kpanic(\"Uknown int 98 call: %d\", index);}");
        inst(fos, fos_init, "normal_int99", "Int99", "U32 index = cpu->peek32(0); if (index<int99CallbackSize && int99Callback[index]) {int99Callback[index](cpu);} else {kpanic(\"Uknown int 99 call: %d\", index);}");

        inst(fos, fos_init, "normal_xlat", "Xlat", "if (op->ea16) {AL = readb(cpu->seg[op->base].address + (U16)(BX + AL));} else {AL = readb(cpu->seg[op->base].address + EBX + AL);}");
        inst(fos, fos_init, "normal_hlt", "Hlt", "kpanic(\"Hlt\");");

        inst(fos, fos_init, "normal_cmc", "Cmc", "cpu->fillFlags();cpu->setCF(!cpu->getCF());");
        inst(fos, fos_init, "normal_clc", "Clc", "cpu->fillFlags();cpu->removeCF();");
        inst(fos, fos_init, "normal_stc", "Stc", "cpu->fillFlags();cpu->addCF();");

        inst(fos, fos_init, "normal_cli", "Cli", "cpu->fillFlags();cpu->removeFlag(IF);");
        inst(fos, fos_init, "normal_sti", "Sti", "cpu->fillFlags();cpu->addFlag(IF);");

        inst(fos, fos_init, "normal_cld", "Cld", "cpu->fillFlags();cpu->removeFlag(DF);cpu->df=1;");
        inst(fos, fos_init, "normal_std", "Std", "cpu->fillFlags();cpu->addFlag(DF);cpu->df=-1;");

        inst(fos, fos_init, "normal_rdtsc", "Rdtsc", "U64 t = cpu->instructionCount+op->imm; EAX = (U32)t; EDX = (U32)(t >> 32);");
        inst(fos, fos_init, "normal_cpuid", "CPUID", "cpu->cpuid();");

        inst(fos, fos_init, "normal_enter16", "Enter16", "cpu->enter(0, op->imm, op->reg);");
        inst(fos, fos_init, "normal_enter32", "Enter32", "cpu->enter(1, op->imm, op->reg);");

        inst(fos, fos_init, "normal_leave16", "Leave16", "SP = BP; BP = cpu->pop16();");
        inst(fos, fos_init, "normal_leave32", "Leave32", "ESP = EBP; EBP = cpu->pop32();");

        inst(fos, fos_init, "normal_loopnz", "LoopNZ", "if (op->ea16){CX--;cpu->eip.u32+=op->len;if (CX!=0 && !cpu->getZF()) {cpu->eip.u32+=op->imm;}} else {ECX--;cpu->eip.u32+=op->len;if (ECX!=0 && !cpu->getZF()) {cpu->eip.u32+=op->imm;}}");
        inst(fos, fos_init, "normal_loopz", "LoopZ", "if (op->ea16){CX--;cpu->eip.u32+=op->len;if (CX!=0 && cpu->getZF()) {cpu->eip.u32+=op->imm;}} else {ECX--;cpu->eip.u32+=op->len;if (ECX!=0 && cpu->getZF()) {cpu->eip.u32+=op->imm;}}");

        inst(fos, fos_init, "normal_loop", "Loop", "if (op->ea16){CX--;cpu->eip.u32+=op->len;if (CX!=0) {cpu->eip.u32+=op->imm;}} else {ECX--;cpu->eip.u32+=op->len;if (ECX!=0) {cpu->eip.u32+=op->imm;}}");

        inst(fos, fos_init, "normal_jcxz", "Jcxz", "cpu->eip.u32+=op->len;if ((op->ea16?CX:ECX)==0) {cpu->eip.u32+=op->imm;}");

        inst(fos, fos_init, "normal_InAlIb", "InAlIb", "AL=0xFF;");
        inst(fos, fos_init, "normal_InAxIb", "InAxIb", "AX=0xFFFF;");
        inst(fos, fos_init, "normal_InEaxIb", "InEaxIb", "EAX=0xFFFFFFFF;");

        inst(fos, fos_init, "normal_OutIbAl", "OutIbAl", "");
        inst(fos, fos_init, "normal_OutIbAx", "InAxIb", "");
        inst(fos, fos_init, "normal_OutIbEax", "InEaxIb", "");

        inst(fos, fos_init, "normal_InAlDx", "InAlDx", "AL=0xFF;");
        inst(fos, fos_init, "normal_InAxDx", "InAxDx", "AX=0xFFFF;");
        inst(fos, fos_init, "normal_InEaxDx", "InEaxDx", "EAX=0xFFFFFFFF;");

        inst(fos, fos_init, "normal_OutDxAl", "OutDxAl", "");
        inst(fos, fos_init, "normal_OutDxAx", "OutDxAx", "");
        inst(fos, fos_init, "normal_OutDxEax", "OutDxEax", "");

        inst(fos, fos_init, "normal_callJw", "CallJw", "cpu->push16(cpu->eip.u32 + op->len); cpu->eip.u32 += op->len + (S16)op->imm;");
        inst(fos, fos_init, "normal_callJd", "CallJd", "cpu->push32(cpu->eip.u32 + op->len); cpu->eip.u32 += op->len + (S32)op->imm;");

        inst(fos, fos_init, "normal_jmp8", "JmpJb", "cpu->eip.u32 += op->len + (S8)op->imm;");
        inst(fos, fos_init, "normal_jmp16", "JmpJw", "cpu->eip.u32 += op->len + (S16)op->imm;");
        inst(fos, fos_init, "normal_jmp32", "JmpJd", "cpu->eip.u32 += op->len + (S32)op->imm;");

        inst(fos, fos_init, "normal_callR16", "CallR16", "cpu->push16(cpu->eip.u32+op->len); cpu->eip.u32 = cpu->reg[op->reg].u16;");
        inst(fos, fos_init, "normal_callR32", "CallR32", "cpu->push32(cpu->eip.u32+op->len);cpu->eip.u32 = cpu->reg[op->reg].u32;");
        inst(fos, fos_init, "normal_callE16", "CallE16", "U32 neweip = readw(eaa(cpu, op)); cpu->push16(cpu->eip.u32+op->len); cpu->eip.u32 = neweip;");
        inst(fos, fos_init, "normal_callE32", "CallE32", "U32 neweip = readd(eaa(cpu, op));cpu->push32(cpu->eip.u32+op->len);cpu->eip.u32 = neweip;");

        inst(fos, fos_init, "normal_jmpR16", "JmpR16", "cpu->eip.u32 = cpu->reg[op->reg].u16;");
        inst(fos, fos_init, "normal_jmpR32", "JmpR32", "cpu->eip.u32 = cpu->reg[op->reg].u32;");
        inst(fos, fos_init, "normal_jmpE16", "JmpE16", "U32 neweip = readw(eaa(cpu, op)); cpu->eip.u32 = neweip;");
        inst(fos, fos_init, "normal_jmpE32", "JmpE32", "U32 neweip = readd(eaa(cpu, op)); cpu->eip.u32 = neweip;");

        inst(fos, fos_init, "normal_callFarE16", "CallFarE16", "U32 eaa = eaa(cpu, op); U16 newip = readw(eaa); U16 newcs = readw(eaa+2); cpu->call(0, newcs, newip, cpu->eip.u32 + op->len);");
        inst(fos, fos_init, "normal_callFarE32", "CallFarE32", "U32 eaa = eaa(cpu, op); U32 newip = readd(eaa); U16 newcs = readw(eaa+4); cpu->call(1, newcs, newip, cpu->eip.u32 + op->len);");

        inst(fos, fos_init, "normal_jmpFarE16", "JmpFarE16", "U32 eaa = eaa(cpu, op); U16 newip = readw(eaa); U16 newcs = readw(eaa+2); cpu->jmp(0, newcs, newip, cpu->eip.u32 + op->len);");
        inst(fos, fos_init, "normal_jmpFarE32", "JmpFarE32", "U32 eaa = eaa(cpu, op); U32 newip = readd(eaa); U16 newcs = readw(eaa+4); cpu->jmp(1, newcs, newip, cpu->eip.u32 + op->len);");

        inst(fos, fos_init, "normal_larr16r16", "LarR16R16", "cpu->reg[op->reg].u16 = cpu->lar(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);");
        inst(fos, fos_init, "normal_larr16e16", "LarR16E16", "cpu->reg[op->reg].u16 = cpu->lar(readw(eaa(cpu, op)), cpu->reg[op->reg].u16);");
        inst(fos, fos_init, "normal_lslr16r16", "LslR16R16", "cpu->reg[op->reg].u16 = cpu->lsl(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);");
        inst(fos, fos_init, "normal_lslr16e16", "LslR16E16", "cpu->reg[op->reg].u16 = cpu->lsl(readw(eaa(cpu, op)), cpu->reg[op->reg].u16);");

        inst(fos, fos_init, "normal_xaddr32r32", "XaddR32R32", "cpu->src.u32 = cpu->reg[op->reg].u32; cpu->dst.u32 = cpu->reg[op->rm].u32; cpu->result.u32 = cpu->dst.u32 + cpu->src.u32; cpu->lazyFlags = FLAGS_ADD32; cpu->reg[op->reg].u32 = cpu->dst.u32; cpu->reg[op->rm].u32 =  cpu->result.u32;");
        inst(fos, fos_init, "normal_xaddr32e32", "XaddR32E32", "U32 address = eaa(cpu, op); cpu->src.u32 = cpu->reg[op->reg].u32; cpu->dst.u32 = readd(address); cpu->result.u32 = cpu->dst.u32 + cpu->src.u32; cpu->lazyFlags = FLAGS_ADD32;cpu->reg[op->reg].u32 = cpu->dst.u32; writed(address, cpu->result.u32);");

        inst(fos, fos_init, "normal_bswap32", "Bswap32", "U32 val = cpu->reg[op->reg].u32; cpu->reg[op->reg].u32 = (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >>  8) | ((val & 0x0000ff00) <<  8) | ((val & 0x000000ff) << 24));");
        inst(fos, fos_init, "normal_cmpxchgg8b", "CmpXchg8b", "U32 address = eaa(cpu, op); U64 value1 = ((U64)EDX) << 32 | EAX; U64 value2 = readq(address); cpu->fillFlags();if (value1 == value2) {cpu->addZF(); writed(address, EBX); writed(address + 4, ECX);} else {cpu->removeZF();EDX = (U32)(value2 >> 32);EAX = (U32)value2;}");

        loadSeg(fos, fos_init, "loadSegment16", "LoadSegment16", "16", "w", "2");
        loadSeg(fos, fos_init, "loadSegment32", "LoadSegment32", "32", "d", "4");
    }

    public void bound(FileOutputStream fos, FileOutputStream fos_init, String bits, String width, String offset) throws IOException {
        out(fos, "void OPCALL bound" + bits + "(CPU* cpu, DecodedOp* op){");
        out(fos, "    U32 eaa = eaa(cpu, op);");
        out(fos, "    if (cpu->reg[op->reg].u"+bits+"<read"+width+"(eaa) || cpu->reg[op->reg].u"+bits+">read"+width+"(eaa+"+offset+")) {cpu->prepareException(EXCEPTION_BOUND, 0);} else { cpu->eip.u32+=op->len;}");
        out(fos, "}");

        out(fos_init, "INIT_CPU(Bound" + bits + ", bound" + bits + ")");
    }

    public void inst(FileOutputStream fos, FileOutputStream fos_init, String name, String ename, String func) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    "+func);
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");
    }

    public void loadSeg(FileOutputStream fos, FileOutputStream fos_init, String name, String ename, String bits, String width, String offset) throws IOException {
        out(fos, "void OPCALL "+name+"(CPU* cpu, DecodedOp* op) {");
        out(fos, "    U32 eaa = eaa(cpu, op);");
        out(fos, "    U"+bits+" val = read"+width+"(eaa);");
        out(fos, "    U32 selector = readw(eaa+"+offset+");"); // yes, only 2 bytes
        out(fos, "    if (cpu->setSegment(op->imm, selector)) {cpu->reg[op->reg].u"+bits+" = val; cpu->eip.u32+=op->len;}");
        out(fos, "}");

        out(fos_init, "INIT_CPU("+ename+", "+name+")");
    }
}
