#include "boxedwine.h"

#include "ksignal.h"
#include "bufferaccess.h"
#include "kstat.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"

#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
void wasmJitMtLeaveCpu(CPU* cpu);
void wasmJitMtUnregisterCpu(CPU* cpu);
#endif

CPU* CPU::allocCPU(KMemory* memory) {
    return new NormalCPU(memory);
}

CPU::~CPU() {
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
    wasmJitMtUnregisterCpu(this);
#endif
}

U32 CPU_CHECK_COND(CPU* cpu, U32 cond, const char* msg, int exc, int sel) {
    if (cond) {
        kwarn(msg);
        cpu->prepareException(exc, sel);
        return 1;
    }
    return 0;
}

CPU::CPU(KMemory* memory) : memory(memory) {    
    this->reg8[0] = &this->reg[0].u8;
    this->reg8[1] = &this->reg[1].u8;
    this->reg8[2] = &this->reg[2].u8;
    this->reg8[3] = &this->reg[3].u8;
    this->reg8[4] = &this->reg[0].h8;
    this->reg8[5] = &this->reg[1].h8;
    this->reg8[6] = &this->reg[2].h8;
    this->reg8[7] = &this->reg[3].h8;  
    this->reg8[8] = &this->reg[8].u8;  

    this->reset();
    this->fpu.reset();

    opCache = (DecodedOp***)&getMemData(memory)->opCache.pageData[0];
#ifdef BOXEDWINE_JIT_ARMV8
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE].pd.f64[0] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE].pd.f64[1] = 2147483648.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE].pd.f64[0] = -2147483649.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE].pd.f64[1] = -2147483649.0;

    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[0] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[1] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[2] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[3] = 2147483648.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[0] = -2147483649.0f;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[1] = -2147483649.0f;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[2] = -2147483649.0f;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[3] = -2147483649.0f;

    sseConstants[SSE_INT32_BIT_MASK].ps.u32[0] = 1;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[1] = 2;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[2] = 4;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[3] = 8;
#undef u8
    sseConstants[SSE_BYTE8_BIT_MASK].pi.u8[0] = 1;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[1] = 2;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[2] = 4;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[3] = 8;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[4] = 16;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[5] = 32;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[6] = 64;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[7] = 128;
    sseConstants[SSE_BYTE8_BIT_MASK].pi.u8[8] = 1;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[9] = 2;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[10] = 4;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[11] = 8;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[12] = 16;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[13] = 32;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[14] = 64;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[15] = 128;
#define u8 byte[0]
#endif
}

void CPU::setSeg(U32 index, U32 address, U32 value) {
    this->seg[index].address = address;
    this->seg[index].value = value;
    if (this->seg[index].address) {
        this->thread->process->hasSetSeg[index] = true;
    }
}

void CPU::setIsBig(U32 value) {
    this->big = value;
}

void CPU::reset() {
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
    // exec can reset the CPU from a syscall helper while the old compiled
    // frame is still unwinding. Preserve its owner hazard until the outer
    // wasmStartJITOp call returns; ordinary resets are already quiescent.
    bool resetInsideWasmJitCall = this->wasmJitInCompiledCall != 0;
    if (!resetInsideWasmJitCall) {
        wasmJitMtLeaveCpu(this);
    }
#endif
    this->flags = ID;
    this->eip.u32 = 0;
    this->instructionCount = 0;
    for (int i=0;i<7;i++) {
        this->setSeg(i, 0, 0);       
    }
    for (int i=0;i<9;i++) {
        this->reg[i].u32 = 0;
    }
    for (int i=0;i<8;i++) {
        this->xmm[i].pi.u64[0] = 0;
        this->xmm[i].pi.u64[1] = 0;
    }
    this->lazyFlagType = FLAGS_NONE;
    this->setIsBig(1);
    this->seg[CS].value = 0xF; // index 1, LDT, rpl=3
    this->seg[SS].value = 0x17; // index 2, LDT, rpl=3
    this->seg[DS].value = 0x17; // index 2, LDT, rpl=3
    this->seg[ES].value = 0x17; // index 2, LDT, rpl=3
    this->cpl = 3; // user mode
    this->cr0 = CR0_PROTECTION | CR0_FPUPRESENT | CR0_PAGING;
    this->flags|=IF;
    this->fpu.FINIT();
    this->stackNotMask = 0;
    this->stackMask = 0xFFFFFFFF;
    this->nextOp = nullptr;
#ifdef BOXEDWINE_MULTI_THREADED
    this->tmpLockAddress = 0;
#endif
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
    if (!resetInsideWasmJitCall) {
        this->wasmJitActiveTableIndex = 0;
        this->wasmJitActiveTableIndexLocal = 0;
        this->wasmJitCallsUntilQuiescence = 0;
        this->wasmJitInCompiledCall = 0;
        this->wasmJitReapRetiredOnExit = 0;
    }
#endif
#ifdef BOXEDWINE_JIT
    memset(calculateCF, 0, sizeof(calculateCF));
#endif
}

void CPU::call(U32 big, U32 selector, U32 offset, U32 oldEip) {
     if (this->flags & VM) {
        U32 esp = THIS_ESP; //  // don't set ESP until we are done with memory Writes / push so that we are reentrant
        if (big) {
            esp = push32_r(esp, this->seg[CS].value);
            esp = push32_r(esp, oldEip);
            this->eip.u32 = offset;
        } else {
            esp = push16_r(esp, this->seg[CS].value);
            esp = push16_r(esp, oldEip & 0xFFFF);
            this->eip.u32 = offset & 0xffff;
        } 
        THIS_ESP = esp;
        this->setIsBig(0);
        this->setSeg(CS, selector << 4, selector);
    } else {
        //U32 rpl=selector & 3;
        U32 index = selector >> 3;

        if (CPU_CHECK_COND(this, (selector & 0xfffc)==0, "CALL:CS selector zero", EXCEPTION_GP,0))
            return;
            
        if (index>=LDT_ENTRIES) {
            CPU_CHECK_COND(this, 0, "CALL:CS beyond limits", EXCEPTION_GP,selector & 0xfffc);
            return;
        }
        struct user_desc* ldt = this->thread->getLDT(index);

        if (this->thread->isLdtEmpty(ldt)) {
            prepareException(EXCEPTION_NP,selector & 0xfffc);
            return;
        }
       
        U32 esp = THIS_ESP;
        // commit point
        if (big) {
            esp = push32_r(esp, this->seg[CS].value);
            esp = push32_r(esp, oldEip);
            this->eip.u32=offset;
        } else {
            esp = push16_r(esp, this->seg[CS].value);
            esp = push16_r(esp, oldEip);
            this->eip.u32=offset & 0xffff;
        }
        THIS_ESP = esp; // don't set ESP until we are done with Memory Writes / CPU_Push so that we are reentrant
        this->setIsBig(ldt->seg_32bit);
        this->setSeg(CS, ldt->base_addr, (selector & 0xfffc) | this->cpl);
    }
}

void CPU::jmp(U32 big, U32 selector, U32 offset, U32 oldEip) {
    if (this->flags & VM) {
        if (!big) {
            this->eip.u32 = offset & 0xffff;
        } else {
            this->eip.u32 = offset;
        }
        this->setSeg(CS, selector << 4, selector);
        this->setIsBig(0);
    } else {
        //U32 rpl=selector & 3;
        U32 index = selector >> 3;

        if (CPU_CHECK_COND(this, (selector & 0xfffc)==0, "JMP:CS selector zero", EXCEPTION_GP,0))
            return;
            
        if (index>=LDT_ENTRIES) {
            if (CPU_CHECK_COND(this, 0, "JMP:CS beyond limits", EXCEPTION_GP,selector & 0xfffc))
                return;
        }
        struct user_desc* ldt = this->thread->getLDT(index);

        if (this->thread->isLdtEmpty(ldt)) {
            prepareException(EXCEPTION_NP,selector & 0xfffc);
            return;
        }

        this->setIsBig(ldt->seg_32bit);
        this->setSeg(CS, ldt->base_addr, (selector & 0xfffc) | this->cpl);
        if (!big) {
            this->eip.u32 = offset & 0xffff;
        } else {
            this->eip.u32 = offset;
        }
    }
}

void CPU::prepareFpuException(int code, int error) {
    const KProcessPtr& process = this->thread->process;

    // blocking signals, signalfd can't handle these
    if (process->sigActions[K_SIGSEGV].handlerAndSigAction != K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction != K_SIG_DFL) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGFPE;
        process->sigActions[K_SIGSEGV].sigInfo[1] = error; // always 0?
        process->sigActions[K_SIGSEGV].sigInfo[2] = code;
        process->sigActions[K_SIGSEGV].sigInfo[3] = 0; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 16; // trap #, TRAP_x86_ARITHTRAP
        this->thread->runSignal(K_SIGSEGV, 13, error);
    } else {
        CPU* cpu = this;
        this->walkStack(this->eip.u32, EBP, 2);
        kpanic_fmt("unhandled exception: code=%d error=%d", code, error);
    }
}

void CPU::prepareException(int code, int error) {
    const KProcessPtr& process = this->thread->process;

     // blocking signals, signalfd can't handle these
    if (code==EXCEPTION_GP && (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = error;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = 0; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 13; // trap #, TRAP_x86_PROTFLT
        this->thread->runSignal(K_SIGSEGV, 13, error);
    } else if (code==EXCEPTION_DIVIDE && error == 0 && (process->sigActions[K_SIGFPE].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGFPE].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGFPE].sigInfo[0] = K_SIGFPE;		
        process->sigActions[K_SIGFPE].sigInfo[1] = error;
        process->sigActions[K_SIGFPE].sigInfo[2] = K_FPE_INTDIV;
        process->sigActions[K_SIGFPE].sigInfo[3] = this->eip.u32; // address
        process->sigActions[K_SIGFPE].sigInfo[4] = 0; // trap #, TRAP_x86_DIVIDE
        this->thread->runSignal(K_SIGFPE, 0, error);
    } else if (code==EXCEPTION_DIVIDE && error == 1 && (process->sigActions[K_SIGFPE].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGFPE].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGFPE].sigInfo[0] = K_SIGFPE;
        process->sigActions[K_SIGFPE].sigInfo[1] = 0;
        process->sigActions[K_SIGFPE].sigInfo[2] = K_FPE_INTOVF;
        process->sigActions[K_SIGFPE].sigInfo[3] = this->eip.u32; // address
        process->sigActions[K_SIGFPE].sigInfo[4] = 4; // trap #, TRAP_x86_OFLOW
        this->thread->runSignal(K_SIGFPE, 4, error);
    } else if (code==EXCEPTION_NP &&  (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = this->eip.u32; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 11; // trap #, TRAP_x86_SEGNPFLT
        this->thread->runSignal(K_SIGSEGV, 11, error);
    } else if (code==EXCEPTION_SS &&  (process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL)) {
        process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[2] = 0;
        process->sigActions[K_SIGSEGV].sigInfo[3] = this->eip.u32; // address
        process->sigActions[K_SIGSEGV].sigInfo[4] = 12; // trap #, TRAP_x86_SEGNPFLT
        this->thread->runSignal(K_SIGSEGV, 12, error);
    } else {        
        CPU* cpu = this;
        this->walkStack(this->eip.u32, EBP, 2);
        kpanic_fmt("unhandled exception: code=%d error=%d", code, error);
    }
}

class TTYBufferAccess : public BufferAccess {
public:
    TTYBufferAccess(const std::shared_ptr<FsNode>& node, U32 flags, BString buffer) : BufferAccess(node, flags, buffer) {}

    // from FsOpenNode
    U32 writeNative(U8* buffer, U32 len) override;
};

static BString tty9Buffer;

U32 TTYBufferAccess::writeNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION;
    tty9Buffer.append((char*)buffer, len);
    return len;
}

FsOpenNode* openTTY9(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new TTYBufferAccess(node, flags, B(""));
}

BString getFunctionName(BString name, U32 moduleEip) {    
    KProcessPtr process = KProcess::create();
    std::vector<BString> args;
    std::vector<BString> env;
    KFileDescriptorPtr fd;

    if (!name.length())
        return B("Unknown");
    args.push_back(B("/usr/local/bin/addr2line"));
    args.push_back(B("-e"));
    args.push_back(name);
    args.push_back(B("-f"));
    args.push_back(BString::valueOf(moduleEip, 16));
    KThread* thread = process->startProcess(B("/usr/local/bin"), args, env, 0, 0, 0, 0);
    if (!thread)
        return B("");

    KThread* tmpThread = new KThread(0, process);
    {
        ChangeThread c(tmpThread);
        tty9Buffer = "";
        std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), B("/dev"), true);
        std::shared_ptr<FsNode> node = Fs::addVirtualFile(B("/dev/tty9"), openTTY9, K__S_IWRITE, (4 << 8) | 9, parent);
        process = tmpThread->process;
        process->openFile(B(""), B("/dev/tty9"), K_O_WRONLY, fd);
        if (fd) {
            tmpThread->log = false;
            tmpThread->process->dup2(fd->handle, 1); // replace stdout with tty9
            waitForProcessToFinish(tmpThread->process, tmpThread);
        }
    }
    delete tmpThread;
    KSystem::eraseProcess(process->id);
    int pos = tty9Buffer.indexOf("\r\n");
    if (pos>=0) {
        return tty9Buffer.substr(0, pos-1);
    }
    return tty9Buffer;
}

void CPU::walkStack(U32 eip, U32 ebp, U32 indent) {
    U32 moduleEip = this->thread->process->getModuleEip(this->seg[CS].address+eip);
    BString name = this->thread->process->getModuleName(this->seg[CS].address+eip);
    BString functionName;
    
    if (name != "Unknown") {
        //functionName = getFunctionName(name, moduleEip);
    }
    name = Fs::getFileNameFromPath(name);

    std::vector<BString> parts;
    functionName.split("\n", parts);
    if (parts.size()) {
        functionName = parts[0];
    }
    klog_fmt("%*s %-20s %-40s %08x / %08x", indent, "", name.length()?name.c_str():"Unknown", functionName.c_str(), eip, moduleEip);

    ChangeThread c(this->thread);
    if (this->memory->canRead(ebp, 8)) {
        U32 prevEbp = memory->readd(ebp); 
        U32 returnEip = memory->readd(ebp+4);
        if (prevEbp==0)
            return;
        walkStack(returnEip, prevEbp, indent);
    }
}

void CPU::cpuid() {
    CPU* cpu = this;
    switch (EAX) {
        case 0:	/* Vendor ID String and maximum level? */
            EAX=2;  /* Maximum level */
            EBX='G' | ('e' << 8) | ('n' << 16) | ('u'<< 24);
            EDX='i' | ('n' << 8) | ('e' << 16) | ('I'<< 24);
            ECX='n' | ('t' << 8) | ('e' << 16) | ('l'<< 24);
            break;
        case 1:	/* get processor type/family/model/stepping and feature flags */
            EBX=0;			/* Not Supported */
            ECX=0;			/* No features */
            EDX=1;	        /* FPU */
            if (KSystem::pentiumLevel==2) {
                EAX=0x633;		/* intel pentium 2 */
            } else if (KSystem::pentiumLevel==3) {
                EAX=0x686;		/* Intel Pentium III 733 MHz */
                EDX|= (1<<24);    // FXSAVE, FXRESTOR
                EDX|= (1<<25);    // SSE
            } else if (KSystem::pentiumLevel==4) {
                EAX=0xF07;		/* Intel Pentium 4 1.4 GHz */
                EDX|= (1<<19);    // CLFLUSH
                EDX|= (1<<24);    // FXSAVE, FXRESTOR
                EDX|= (1<<25);    // SSE
                EDX|= (1<<26);    // SSE2
            }          
            EDX|= (1<<4);     /* RDTSC */
            EDX|= (1<<5);     /* MSR */
            EDX|= (1<<8);     /* CMPXCHG8B instruction */
            EDX|= (1<<15);    /* support CMOV instructions */
            EDX|= (1<<13);    /* PTE Global Flag */            
            EDX|= (1<<23);    // MMX
            break;
        case 2: // TLB and cache
            EAX=0x3020101;
            EBX=0;
            ECX=0;
            EDX=0x0C040843;
            break;
        case 0x80000000:
            EAX = 0x80000004;
            break;
        case 0x80000001:
            EAX = 0;
            EBX = 0;
            ECX = 0;
            EDX = 0;
            break;
        case 0x80000002:
            // "Boxedwine"
            EAX = 0x65786F42;
            EBX = 0x6E697764;
            ECX = 0x00000065;
            EDX = 0;
            break;
        case 0x80000003:
        case 0x80000004:
            EAX = 0;
            EBX = 0;
            ECX = 0;
            EDX = 0;
            break;
        default:
            kwarn_fmt("Unhandled CPUID Function %X", EAX);
            EAX=0;
            EBX=0;
            ECX=0;
            EDX=0;
            break;
    }
}

void CPU::enter(U32 big, U32 bytes, U32 level) {
    CPU* cpu = this;
    if (big) {
        U32 sp_index=ESP & cpu->stackMask;
        U32 bp_index=EBP & cpu->stackMask;

        sp_index-=4;
        memory->writed(cpu->seg[SS].address + sp_index, EBP);
        EBP = ESP - 4;
        if (level!=0) {
            U32 i;

            for (i=1;i<level;i++) {
                sp_index-=4;
                bp_index-=4;
                memory->writed(cpu->seg[SS].address + sp_index, memory->readd(cpu->seg[SS].address + bp_index));
            }
            sp_index-=4;
            memory->writed(cpu->seg[SS].address + sp_index, EBP);
        }
        sp_index-=bytes;
        ESP = (ESP & cpu->stackNotMask) | (sp_index & cpu->stackMask);
    } else {
        U32 sp_index=ESP & cpu->stackMask;
        U32 bp_index=EBP & cpu->stackMask;

        sp_index-=2;
        memory->writew(cpu->seg[SS].address + sp_index, BP);
        BP = SP - 2;
        if (level!=0) {
            U32 i;

            for (i=1;i<level;i++) {
                sp_index-=2;bp_index-=2;
                memory->writew(cpu->seg[SS].address + sp_index, memory->readw(cpu->seg[SS].address + bp_index));
            }
            sp_index-=2;
            memory->writew(cpu->seg[SS].address + sp_index, BP);
        }

        sp_index-=bytes;
        ESP = (ESP & cpu->stackNotMask) | (sp_index & cpu->stackMask);
    }
}

U32 CPU::lar(U32 selector, U32 ar) {
    this->fillFlags();
	U32 selectorIndex = selector >> 3;

    if (selectorIndex == 0 || selectorIndex >= LDT_ENTRIES) {
        this->flags &=~ZF;
        return ar;
    }    
    struct user_desc* ldt = this->thread->getLDT(selectorIndex);
    this->flags |= ZF;
    ar = 0;
    if (!ldt->seg_not_present)
        ar|=0x0100;
    ar|=0x0600; // ring 3 (dpl)
    ar|=0x0800; // not system
    // bits 5,6,7 = type, hopefully not used
    ar|=0x8000; // accessed;
    return ar;
}

U32 CPU::lsl(U32 selector, U32 limit) {
    this->fillFlags();
    U32 selectorIndex = selector >> 3;

    if (selectorIndex == 0 || selectorIndex >= LDT_ENTRIES) {
        this->removeZF();
        return limit;
    }    
    struct user_desc* ldt = this->thread->getLDT(selectorIndex);
    if (!ldt) {
        this->removeZF();
        return limit;
    }
    this->addZF();
    return ldt->limit;
}

U32 CPU::setSegment(U32 seg, U32 value) {
    value &= 0xffff;
    if (seg>=6) {
        kpanic_fmt("CPU::setSegment invalid segment: %d", seg);
    }
    if (this->flags & VM) {
        this->setSeg(seg, value << 4, value);
    } else  if ((value & 0xfffc)==0) {
        this->setSeg(seg, 0, value);
    } else {
        U32 index = value >> 3;
        struct user_desc* ldt = this->thread->getLDT(index);

        if (!ldt) {
            this->prepareException(EXCEPTION_GP,value & 0xfffc);
            return 0;
        }
        if (ldt->seg_not_present) {
            if (seg==SS)
                this->prepareException(EXCEPTION_SS,value & 0xfffc);
            else
                this->prepareException(EXCEPTION_NP,value & 0xfffc);
            return 0;
        }
        this->setSeg(seg, ldt->base_addr, value);        
        if (seg == SS) {
            if (ldt->seg_32bit) {
                this->stackMask = 0xffffffff;
                this->stackNotMask = 0;
            } else {
                this->stackMask = 0xffff;
                this->stackNotMask = 0xffff0000;
                this->thread->process->hasSetStackMask = true;
            }
        }
    }
    return 1;
}

void CPU::ret(U32 big, U32 bytes) {
    if (this->flags & VM) {
        U32 new_ip = 0;
        U32 new_cs = 0;
        if (big) {
            new_ip = pop32();
            new_cs = pop32() & 0xffff;            
        } else {
            new_ip = pop16();
            new_cs = pop16();
        }
        THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + bytes ) & this->stackMask);
        this->setSegment(CS, new_cs);
        this->eip.u32 = new_ip;
        this->setIsBig(0);
    } else {
        U32 selector = 0;

        if (big) 
            selector = peek32(1);
        else 
            selector = peek16(1);

        U32 rpl=selector & 3; // requested privilege level
        if(rpl < this->cpl) {
            this->prepareException(EXCEPTION_GP, selector & 0xfffc);
            return;
        }        
        U32 index = selector >> 3;

        if (CPU_CHECK_COND(this, (selector & 0xfffc)==0, "RET:CS selector zero", EXCEPTION_GP,0))
            return;

        if (index>=LDT_ENTRIES) {
            CPU_CHECK_COND(this, 0, "RET:CS beyond limits", EXCEPTION_GP,selector & 0xfffc);
            return;
        }
        struct user_desc* ldt = this->thread->getLDT(index);
        if (this->thread->isLdtEmpty(ldt)) {
            this->prepareException(EXCEPTION_NP, selector & 0xfffc);
            return;
        }
        if (this->cpl==rpl) {
            // Return to same level             
            U32 offset = 0;
            if (big) {
                offset = pop32();
                selector = pop32() & 0xffff;
            } else {
                offset = pop16();
                selector = pop16();
            }
            this->setSeg(CS, ldt->base_addr, selector);
            this->setIsBig(ldt->seg_32bit);
            this->eip.u32 = offset;
            THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + bytes ) & this->stackMask);
        } else {
            // Return to outer level
            U32 n_esp = 0;
            U32 n_ss = 0;
            U32 offset = 0;;

            if (big) {
                offset = pop32();
                selector = pop32() & 0xffff;
                THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + bytes ) & this->stackMask);
                n_esp = pop32();
                n_ss = pop32();
            } else {
                offset = pop16();
                selector = pop16();
                THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + bytes ) & this->stackMask);
                n_esp = pop16();
                n_ss = pop16();
            }
            U32 ssIndex = n_ss >> 3;
            if (CPU_CHECK_COND(this, (n_ss & 0xfffc)==0, "RET to outer level with SS selector zero", EXCEPTION_GP,0))
                return;
            
            if (ssIndex>=LDT_ENTRIES) {
                CPU_CHECK_COND(this, 0, "RET:SS beyond limits", EXCEPTION_GP, n_ss & 0xfffc);
                return;
            }
            struct user_desc* ssLdt = this->thread->getLDT(ssIndex);

            if (CPU_CHECK_COND(this, (n_ss & 3)!=rpl, "RET to outer segment with invalid SS privileges", EXCEPTION_GP,n_ss & 0xfffc))
                return;

            if (CPU_CHECK_COND(this, this->thread->isLdtEmpty(ldt), "RET:Stack segment not present", EXCEPTION_SS,n_ss & 0xfffc))
                return;

            this->cpl = rpl; // don't think paging tables need to be messed with, this isn't 100% cpu emulator since we are assuming a user space program

            this->setSeg(CS, ldt->base_addr, (selector & 0xfffc) | this->cpl);
            this->setIsBig(ldt->seg_32bit);
            this->eip.u32 = offset;

            this->setSeg(SS, ssLdt->base_addr, n_ss);                

            if (ssLdt->seg_32bit) {
                this->stackMask = 0xFFFFFFFF;
                this->stackNotMask = 0;
                THIS_ESP=n_esp+bytes;
            } else {
                this->stackMask = 0x0000FFFF;
                this->stackNotMask = 0xFFFF0000;
                this->thread->process->hasSetStackMask = true;
                THIS_SP=n_esp+bytes;
            }
        }
    }
}

void CPU::iret(U32 big, U32 oldeip) {
    CPU* cpu = this;

    if (this->flags & VM) {
        if ((this->flags & IOPL)!=IOPL) {
            this->prepareException(EXCEPTION_GP, 0);
            return;
        } else {
            if (big) {
                U32 new_eip = this->peek32(0);
                U32 new_cs = this->peek32(1);
                U32 new_flags = this->peek32(2);

                ESP = (ESP & this->stackNotMask) | ((ESP + 12) & this->stackMask);

                this->eip.u32 = new_eip;
                this->setSegment(CS, new_cs & 0xFFFF);

                /* IOPL can not be modified in v86 mode by IRET */
                this->setFlags(new_flags, FMASK_NORMAL | NT);
            } else {
                U32 new_eip = this->peek16(0);
                U32 new_cs = this->peek16(1);
                U32 new_flags = this->peek16(2);

                ESP = (ESP & this->stackNotMask) | ((ESP + 6) & this->stackMask);

                cpu->eip.u32 = new_eip;
                this->setSegment(CS, new_cs);
                /* IOPL can not be modified in v86 mode by IRET */
                cpu->setFlags(new_flags, FMASK_NORMAL | NT);
            }
            this->setIsBig(0);
            this->lazyFlagType = FLAGS_NONE;
            return;
        }
    }
    /* Check if this is task IRET */
    if (this->flags & NT) {
        kpanic("cpu tasks not implemented");
        return;
    } else {
        U32 n_cs_sel = 0;
        U32 n_flags = 0;
        U32 n_eip = 0;

        if (big) {
            n_eip = this->peek32(0);
            n_cs_sel = this->peek32(1) & 0xffff;
            n_flags = this->peek32(2);

            if ((n_flags & VM) && (this->cpl==0)) {
                // commit point
                ESP = (ESP & this->stackNotMask) | ((ESP + 12) & this->stackMask);
                this->eip.u32 = n_eip & 0xffff;
                U32 n_esp=this->pop32();
                U32 n_ss=this->pop32() & 0xffff;
                U32 n_es=this->pop32() & 0xffff;
                U32 n_ds=this->pop32() & 0xffff;
                U32 n_fs=this->pop32() & 0xffff;
                U32 n_gs=this->pop32() & 0xffff;

                this->setFlags(n_flags, FMASK_NORMAL | VM);
                this->lazyFlagType = FLAGS_NONE;
                this->cpl = 3;

                this->setSegment(SS, n_ss);
                this->setSegment(ES, n_es);
                this->setSegment(DS, n_ds);
                this->setSegment(FS, n_fs);
                this->setSegment(GS, n_gs);
                ESP = n_esp;
                this->setIsBig(0);
                this->setSegment(CS, n_cs_sel);
                return;
            }
            if (n_flags & VM) kpanic("IRET from pmode to v86 with CPL!=0");
        } else {
            n_eip = this->peek16(0);
            n_cs_sel = this->peek16(1);
            n_flags = this->peek16(2);

            n_flags |= (this->flags & 0xffff0000);
            if (n_flags & VM) kpanic("VM Flag in 16-bit iret");
        }
        if (CPU_CHECK_COND(this, (n_cs_sel & 0xfffc)==0, "IRET:CS selector zero", EXCEPTION_GP, 0))
            return;
        U32 n_cs_rpl=n_cs_sel & 3;
        U32 csIndex = n_cs_sel >> 3;

        if (CPU_CHECK_COND(this, csIndex>=LDT_ENTRIES, "IRET:CS selector beyond limits", EXCEPTION_GP,(n_cs_sel & 0xfffc))) {
            return;
        }
        if (CPU_CHECK_COND(this, n_cs_rpl<this->cpl, "IRET to lower privilege", EXCEPTION_GP,(n_cs_sel & 0xfffc))) {
            return;
        }
        struct user_desc* ldt = this->thread->getLDT(csIndex);

        if (CPU_CHECK_COND(this, this->thread->isLdtEmpty(ldt), "IRET with nonpresent code segment",EXCEPTION_NP,(n_cs_sel & 0xfffc)))
            return;

        /* Return to same level */
        if (n_cs_rpl==this->cpl) {
            // commit point
            ESP = (ESP & this->stackNotMask) | ((ESP + (big?12:6)) & this->stackMask);
            this->setSeg(CS, ldt->base_addr, n_cs_sel);
            this->setIsBig(ldt->seg_32bit);
            this->eip.u32 = n_eip;     
            U32 mask = this->cpl !=0 ? (FMASK_NORMAL | NT) : FMASK_ALL;
            if (((this->flags & IOPL) >> 12) < this->cpl) mask &= ~IF;
            this->lazyFlagType = FLAGS_NONE;
            this->setFlags(n_flags,mask);            
        } else {
            /* Return to outer level */
            U32 n_ss = 0;
            U32 n_esp = 0;

            if (big) {
                n_esp = this->peek32(3);
                n_ss = this->peek32(4);
            } else {
                n_esp = this->peek16(3);
                n_ss = this->peek16(4);
            }
            if (CPU_CHECK_COND(this, (n_ss & 0xfffc)==0, "IRET:Outer level:SS selector zero", EXCEPTION_GP,0))
                return;
            if (CPU_CHECK_COND(this, (n_ss & 3)!=n_cs_rpl, "IRET:Outer level:SS rpl!=CS rpl", EXCEPTION_GP,n_ss & 0xfffc))
                return;

            U32 ssIndex = n_ss >> 3;
            if (CPU_CHECK_COND(this, ssIndex>=LDT_ENTRIES, "IRET:Outer level:SS beyond limit", EXCEPTION_GP,n_ss & 0xfffc))
                return;
            //if (CPU_CHECK_COND(n_ss_desc_2.DPL()!=n_cs_rpl, "IRET:Outer level:SS dpl!=CS rpl", EXCEPTION_GP,n_ss & 0xfffc))
            //    return;

            struct user_desc* ssLdt = this->thread->getLDT(ssIndex);

            if (CPU_CHECK_COND(this, this->thread->isLdtEmpty(ssLdt), "IRET:Outer level:Stack segment not present", EXCEPTION_NP,n_ss & 0xfffc))
                return;

            // commit point
            this->setSeg(CS, ldt->base_addr, n_cs_sel);
            this->setIsBig(ldt->seg_32bit);
            U32 mask = this->cpl !=0 ? (FMASK_NORMAL | NT) : FMASK_ALL;
            if (((this->flags & IOPL) >> 12) < this->cpl) mask &= ~IF;
            this->setFlags(n_flags, mask);
            this->lazyFlagType = FLAGS_NONE;

            this->cpl = n_cs_rpl;
            this->eip.u32 = n_eip;

            this->setSeg(SS, ssLdt->base_addr, n_ss);

            if (ssLdt->seg_32bit) {
                this->stackMask = 0xffffffff;
                this->stackNotMask = 0;
                ESP = n_esp;
            } else {
                this->stackMask = 0xffff;
                this->stackNotMask = 0xffff0000;
                this->thread->process->hasSetStackMask = true;
                SP = (U16)n_esp;
            }
        }
    }
} 

void CPU::lazyFlagsCFOF(bool fillCF) {
    if (lazyFlagType != FLAGS_CFOF) {
        if (fillCF) {
            bool cf = getCF();
            lazyFlagTypePrev = lazyFlagType;
            lazyFlagType = FLAGS_CFOF;
            setCF(cf ? 1 : 0);
        } else {
            lazyFlagTypePrev = lazyFlagType;
            lazyFlagType = FLAGS_CFOF;
        }
    }
}

void CPU::fillFlags() {
    if (this->lazyFlagType !=FLAGS_NONE) {
        int newFlags = this->flags & ~(CF|AF|OF|SF|ZF|PF);
        const LazyFlags* lazyFlag = lazyFlags[this->lazyFlagType];

        if (lazyFlag->getAF(this)) newFlags |= AF;
        if (lazyFlag->getZF(this)) newFlags |= ZF;
        if (lazyFlag->getPF(this)) newFlags |= PF;
        if (lazyFlag->getSF(this)) newFlags |= SF;
        if (lazyFlag->getCF(this)) newFlags |= CF;
        if (lazyFlag->getOF(this)) newFlags |= OF;
        this->flags = newFlags;
        this->lazyFlagType = FLAGS_NONE;
    }
}

void CPU::fillFlagsNoCF() {
    if (this->lazyFlagType !=FLAGS_NONE) {
        int newFlags = this->flags & ~(CF|AF|OF|SF|ZF|PF);
        const LazyFlags* lazyFlag = lazyFlags[this->lazyFlagType];

        if (lazyFlag->getAF(this)) newFlags |= AF;
        if (lazyFlag->getZF(this)) newFlags |= ZF;
        if (lazyFlag->getPF(this)) newFlags |= PF;
        if (lazyFlag->getSF(this)) newFlags |= SF;
        if (lazyFlag->getOF(this)) newFlags |= OF;
        this->flags = newFlags;
        this->lazyFlagType = FLAGS_NONE;
    }
}

void CPU::fillFlagsNoZF() {
    if (this->lazyFlagType !=FLAGS_NONE) {
        int newFlags = this->flags & ~(CF|AF|OF|SF|ZF|PF);
        const LazyFlags* lazyFlag = lazyFlags[this->lazyFlagType];

        if (lazyFlag->getAF(this)) newFlags |= AF;
        if (lazyFlag->getCF(this)) newFlags |= CF;
        if (lazyFlag->getPF(this)) newFlags |= PF;
        if (lazyFlag->getSF(this)) newFlags |= SF;
        if (lazyFlag->getOF(this)) newFlags |= OF;
        this->flags = newFlags;
        this->lazyFlagType = FLAGS_NONE;
    }
}

bool CPU::getCF() {
    return lazyFlags[this->lazyFlagType]->getCF(this) != 0;
}

bool CPU::getSF() {
    return lazyFlags[this->lazyFlagType]->getSF(this) != 0;
}

bool CPU::getZF() {
    return lazyFlags[this->lazyFlagType]->getZF(this) != 0;
}

bool CPU::getOF() {
    return lazyFlags[this->lazyFlagType]->getOF(this) != 0;
}

bool CPU::getAF() {
    return lazyFlags[this->lazyFlagType]->getAF(this) != 0;
}

bool CPU::getPF() {
    return lazyFlags[this->lazyFlagType]->getPF(this) != 0;
}

void CPU::setCF(U32 value) {
#ifdef _DEBUG
    if (this->lazyFlagType != FLAGS_NONE && this->lazyFlagType != FLAGS_CFOF) {
        kpanic("CPU::fillFlags must be called before CPU::setCF");
    }
#endif
    this->flags&=~CF;
    if (value)
        this->flags|=CF;
}

void CPU::setOF(U32 value) {
#ifdef _DEBUG
    if (this->lazyFlagType != FLAGS_NONE && this->lazyFlagType != FLAGS_CFOF) {
        kpanic("CPU::fillFlags must be called before CPU::setOF");
    }
#endif
    this->flags&=~OF;
    if (value)
        this->flags|=OF;
}

void CPU::setSF(U32 value) {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::setSF");
    }
#endif
    this->flags&=~SF;
    if (value)
        this->flags|=SF;
}

void CPU::setZF(U32 value) {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::setZF");
    }
#endif
    this->flags&=~ZF;
    if (value)
        this->flags|=ZF;
}

extern U8 parity_lookup[256] ;
void CPU::setPFonValue(U32 value) {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::setPFonValue");
    }
#endif
    this->flags&=~PF;
    if (parity_lookup[value & 0xFF])
        this->flags|=PF;
}

void CPU::setFlags(U32 flags, U32 mask) {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::setFlags");
    }
#endif
    this->flags=(this->flags & ~mask)|(flags & mask)|2;
}

void CPU::addFlag(U32 flags) {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE && (flags & (CF|AF|OF|SF|ZF|PF))) {
        kpanic("CPU::fillFlags must be called before CPU::addFlag");
    }
#endif
    this->flags|=flags;
}

void CPU::removeFlag(U32 flags) {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE && (flags & (CF|AF|OF|SF|ZF|PF))) {
        kpanic("CPU::fillFlags must be called before CPU::removeFlag");
    }
#endif
    this->flags&=~flags;
}

void CPU::addZF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::addZF");
    }
#endif
    this->flags|=ZF;
}

void CPU::removeZF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::removeZF");
    }
#endif
    this->flags&=~ZF;
}

void CPU::addCF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::addCF");
    }
#endif
    this->flags|=CF;
}

void CPU::removeCF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::removeCF");
    }
#endif
    this->flags&=~CF;
}

void CPU::addAF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::addAF");
    }
#endif
    this->flags|=AF;
}

void CPU::removeAF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::removeAF");
    }
#endif
    this->flags&=~AF;
}

void CPU::addOF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::addOF");
    }
#endif
    this->flags|=OF;
}

void CPU::removeOF() {
#ifdef _DEBUG
    if (this->lazyFlagType !=FLAGS_NONE) {
        kpanic("CPU::fillFlags must be called before CPU::removeOF");
    }
#endif
    this->flags&=~OF;
}

U32 CPU::pop32() {
    U32 val = memory->readd(this->seg[SS].address + (THIS_ESP & this->stackMask));
    THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + 4 ) & this->stackMask);
    return val;
}

U16 CPU::pop16() {
    U16 val = memory->readw(this->seg[SS].address + (THIS_ESP & this->stackMask));
    THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + 2 ) & this->stackMask);
    return val;
}

U32 CPU::peek32(U32 index) {
    return memory->readd(this->seg[SS].address + ((THIS_ESP+index*4) & this->stackMask));
}

U16 CPU::peek16(U32 index) {
    return memory->readw(this->seg[SS].address+ ((THIS_ESP+index*2) & this->stackMask));
}

void CPU::push16(U16 value) {
    U32 new_esp=(THIS_ESP & this->stackNotMask) | ((THIS_ESP - 2) & this->stackMask);
    memory->writew(this->seg[SS].address + (new_esp & this->stackMask) ,value);
    THIS_ESP = new_esp;
}

U32 CPU::push16_r(U32 esp, U16 value) {
    U32 new_esp=(esp & this->stackNotMask) | ((esp - 2) & this->stackMask);
    U32 address = this->seg[SS].address + (new_esp & this->stackMask);
    memory->writew(address ,value);
    return new_esp;
}

void CPU::push32(U32 value) {
    U32 new_esp=(THIS_ESP & this->stackNotMask) | ((THIS_ESP - 4) & this->stackMask);
    memory->writed(this->seg[SS].address + (new_esp & this->stackMask) ,value);
    THIS_ESP = new_esp;
}

U32 CPU::push32_r(U32 esp, U32 value) {
    U32 new_esp=(esp & this->stackNotMask) | ((esp - 4) & this->stackMask);
    U32 address = this->seg[SS].address + (new_esp & this->stackMask);

    memory->writed(address ,value);
    return new_esp;
}

void CPU::clone(CPU* from) {
    for (int i=0;i<9;i++)
        this->reg[i] = from->reg[i];
    for (int i=0;i<7;i++)
        this->seg[i] = from->seg[i];
    this->flags = from->flags;
    this->eip = from->eip;
    this->setIsBig(from->isBig());

    this->src = from->src;
    this->dst = from->dst;
    this->result = from->result;
    this->lazyFlagType = from->lazyFlagType;
    this->oldCF = from->oldCF;
    this->fpu = from->fpu;
    //U64		    instructionCount;
    //U32         blockInstructionCount;
    //bool        yield;
    this->cpl = from->cpl;    
    this->cr0 = from->cr0;
    this->stackNotMask = from->stackNotMask;
    this->stackMask = from->stackMask;

    //KThread* thread;
    //Memory* memory;
    //void* logFile;
}

void CPU::verr(U32 selector) {
    this->fillFlags();
    if (selector == 0) {
        this->removeZF();
        return;
    }
    U32 index = selector >> 3;
    struct user_desc* ldt = this->thread->getLDT(index);

    if (!ldt || ldt->seg_not_present) {
        this->removeZF();
    } else {
        this->addZF();
    }
}

void CPU::verw(U32 selector) {
    this->fillFlags();
    if (selector == 0) {
        this->removeZF();
        return;
    }
    U32 index = selector >> 3;
    struct user_desc* ldt = this->thread->getLDT(index);

    if (!ldt || ldt->seg_not_present || ldt->read_exec_only) {
        this->removeZF();
    } else {
        this->addZF();
    }
}

U32 CPU::getEipAddress() {
    if (this->isBig())
        return this->eip.u32+this->seg[CS].address;
    return (this->eip.u32 & 0xFFFF) + this->seg[CS].address;
}

U32 CPU::readCrx(U32 which, U32 reg) {
    //this->prepareException(EXCEPTION_GP, 0);
    this->reg[reg].u32 = 0;
    return 1;
}

U32 CPU::writeCrx(U32 which, U32 value) {
    //this->prepareException(EXCEPTION_GP, 0);
    return 1;
}

bool CPU::shouldContinue(U32 eip) {
    return this->memory->getDecodedOp(eip) == nullptr;
}

DecodedOp** CPU::getOpLocation(U32 eip) {
    return this->memory->getDecodedOpLocation(eip);
}

U8 CPU::fetchByte(U32* eip) {
    if (*eip - this->seg[CS].address == 0xFFFF && !this->isBig()) {
        kpanic("eip wrapped around.");
    }
    return this->memory->readb((*eip)++);
}

DecodedOp* CPU::getNextOp(U32 jumpTargetFlags) {
    return getOp(getEipAddress(), jumpTargetFlags);
}

#ifdef BOXEDWINE_HOST_EXCEPTIONS
void* CPU::startException(U32 address, bool readAddress) {
    if (this->thread->terminating) {
        return this->thread->process->blockExit;
    }
    if (this->inException) {
        this->thread->seg_mapper(address, readAddress, !readAddress, false);
        this->nextOp = getNextOp();
    }
    return 0;
}

void* CPU::handleAccessException(DecodedOp* op) {
    if (op->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
        op->exceptionCount++;
    } else if (op->blockStart) {
        U32 eipDistance = 0;
        DecodedOp* nextOp = op->blockStart;
        U32 count = 1;

        while (nextOp && nextOp != op && count < MAX_OP_COUNT_PER_BLOCK) {
            eipDistance += nextOp->len;
            nextOp = nextOp->next;
            count++;
        }        
        if (nextOp == op) {
            memory->removeCodeBlock(this->eip.u32 - eipDistance, op->blockStart, false);
        }
    }
    runNextSingleOp();
    return this->thread->process->blockExit;
}
#endif

static DecodedOp lastOp;

void OPCALL onLastOp(CPU* cpu, DecodedOp* op) {
}

void CPU::runNextSingleOp() {
    DecodedOp* op = nullptr;
    try {
        op = getNextOp();
        if (!op) {
            this->thread->seg_mapper(getEipAddress(), true, false, false);
            this->nextOp = getNextOp();
        }
    } catch (...) {
        // at this point the previous getNextOp threw an exception and the eip is now pointing to the signal handler
        op = getNextOp();
    }

    if (!op) {
        kpanic("jitRunSingleOp oops");
    }
    try {
#ifdef BOXEDWINE_JIT
        if (op->flags2 & OP_FLAG2_TRACED_STUB) {
            op->flags2 &= ~OP_FLAG2_TRACED_STUB;
            op->runCount = 1; // so that tracing won't stub it out again
            memory->removeCodeBlock(this->eip.u32, op, false);
        }
#endif
        DecodedOp o = *op;
        lastOp.pfn = onLastOp;
        lastOp.inst = Custom1; // Custom1 has no normalDispatch case, so inst-switch chains fall to default: and call pfn
        o.next = &lastOp;
        o.pfn = NormalCPU::getFunctionForOp(op);
        o.pfn(this, &o);
    } catch (...) {
        // motorhead 3dfx will trigger this when pressing enter to start a new game
    }
    this->nextOp = getNextOp();
}

#ifdef BOXEDWINE_JIT
U32 CPU::offsetofReg32(U32 index) {
    switch (index) {
    case 0: return offsetof(CPU, reg[0].u32);
    case 1: return offsetof(CPU, reg[1].u32);
    case 2: return offsetof(CPU, reg[2].u32);
    case 3: return offsetof(CPU, reg[3].u32);
    case 4: return offsetof(CPU, reg[4].u32);
    case 5: return offsetof(CPU, reg[5].u32);
    case 6: return offsetof(CPU, reg[6].u32);
    case 7: return offsetof(CPU, reg[7].u32);
    case 8: return offsetof(CPU, reg[8].u32);
    }
    kpanic_fmt("CPU::offsetofReg32 oops %d", index);
    return 0;
}

U32 CPU::offsetofReg16(U32 index) {
    switch (index) {
    case 0: return offsetof(CPU, reg[0].u16);
    case 1: return offsetof(CPU, reg[1].u16);
    case 2: return offsetof(CPU, reg[2].u16);
    case 3: return offsetof(CPU, reg[3].u16);
    case 4: return offsetof(CPU, reg[4].u16);
    case 5: return offsetof(CPU, reg[5].u16);
    case 6: return offsetof(CPU, reg[6].u16);
    case 7: return offsetof(CPU, reg[7].u16);
    case 8: return offsetof(CPU, reg[8].u16);
    }
    kpanic_fmt("CPU::offsetofReg16 oops %d", index);
    return 0;
}

U32 CPU::offsetofReg8(U32 index) {
    switch (index) {
    case 0: return offsetof(CPU, reg[0].u8);
    case 1: return offsetof(CPU, reg[1].u8);
    case 2: return offsetof(CPU, reg[2].u8);
    case 3: return offsetof(CPU, reg[3].u8);
    case 4: return offsetof(CPU, reg[0].h8);
    case 5: return offsetof(CPU, reg[1].h8);
    case 6: return offsetof(CPU, reg[2].h8);
    case 7: return offsetof(CPU, reg[3].h8);
    }
    kpanic_fmt("CPU::offsetofReg8 oops %d", index);
    return 0;
}

U32 CPU::offsetofSegAddress(U32 index) {
    switch (index) {
    case 0: return offsetof(CPU, seg[0].address);
    case 1: return offsetof(CPU, seg[1].address);
    case 2: return offsetof(CPU, seg[2].address);
    case 3: return offsetof(CPU, seg[3].address);
    case 4: return offsetof(CPU, seg[4].address);
    case 5: return offsetof(CPU, seg[5].address);
    case 6: return offsetof(CPU, seg[6].address);
    }
    kpanic_fmt("CPU::offsetofSegAddress oops %d", index);
    return 0;
}

U32 CPU::offsetofSegValue(U32 index) {
    switch (index) {
    case 0: return offsetof(CPU, seg[0].value);
    case 1: return offsetof(CPU, seg[1].value);
    case 2: return offsetof(CPU, seg[2].value);
    case 3: return offsetof(CPU, seg[3].value);
    case 4: return offsetof(CPU, seg[4].value);
    case 5: return offsetof(CPU, seg[5].value);
    case 6: return offsetof(CPU, seg[6].value);
    }
    kpanic_fmt("CPU::offsetofSegvalue oops %d", index);
    return 0;
}

#endif

void common_prepareException(CPU* cpu, int code, int error) {
    cpu->prepareException(code, error);
}

U32 common_getCF(CPU* cpu) {
    return cpu->getCF() ? 1 : 0;
}

U32 common_condition_o(CPU* cpu) {
    return cpu->getOF() ? 1 : 0;
}

U32 common_condition_no(CPU* cpu) {
    return cpu->getOF() ? 0 : 1;
}

U32 common_condition_b(CPU* cpu) {
    return cpu->getCF() ? 1 : 0;
}

U32 common_condition_nb(CPU* cpu) {
    return cpu->getCF() ? 0 : 1;
}

U32 common_condition_z(CPU* cpu) {
    return cpu->getZF() ? 1 : 0;
}

U32 common_condition_nz(CPU* cpu) {
    return cpu->getZF() ? 0 : 1;
}

U32 common_condition_be(CPU* cpu) {
    return (cpu->getZF() || cpu->getCF()) ? 1 : 0;
}

U32 common_condition_nbe(CPU* cpu) {
    return (!cpu->getZF() && !cpu->getCF()) ? 1 : 0;
}

U32 common_condition_s(CPU* cpu) {
    return cpu->getSF() ? 1 : 0;
}

U32 common_condition_ns(CPU* cpu) {
    return cpu->getSF() ? 0 : 1;
}

U32 common_condition_p(CPU* cpu) {
    return cpu->getPF() ? 1 : 0;
}

U32 common_condition_np(CPU* cpu) {
    return cpu->getPF() ? 0 : 1;
}

U32 common_condition_l(CPU* cpu) {
    return (cpu->getSF()!=cpu->getOF()) ? 1 : 0;
}

U32 common_condition_nl(CPU* cpu) {
    return (cpu->getSF()==cpu->getOF()) ? 1 : 0;
}

U32 common_condition_le(CPU* cpu) {
    return (cpu->getZF() || cpu->getSF()!=cpu->getOF()) ? 1 : 0;
}

U32 common_condition_nle(CPU* cpu) {
    return (!cpu->getZF() && cpu->getSF()==cpu->getOF()) ? 1 : 0;
}

U32 common_pop32(CPU* cpu) {
    return cpu->pop32();
}

U16 common_pop16(CPU* cpu) {
    return cpu->pop16();
}

void common_push16(CPU* cpu, U16 value) {
    cpu->push16(value);
}

void common_push32(CPU* cpu, U32 value) {
    cpu->push32(value);
}

U32 common_peek32(CPU* cpu, U32 index) {
    return cpu->peek32(index);
}

U16 common_peek16(CPU* cpu, U32 index) {
    return cpu->peek16(index);
}

U32 common_setSegment(CPU* cpu, U32 seg, U32 value) {
    return cpu->setSegment(seg, value);
}

void common_setFlags(CPU* cpu, U32 flags, U32 mask) {
    if (cpu->lazyFlagType != FLAGS_NONE) {
        cpu->fillFlags();
    }
    cpu->setFlags(flags, mask);
}

void common_fillFlags(CPU* cpu) {
    cpu->fillFlags();
}

void common_call(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip) {
    cpu->call(big, selector, offset, oldEip);
}

void common_jmp(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip) {
    cpu->jmp(big, selector, offset, oldEip);
}

void common_ret(CPU* cpu, U32 big, U32 bytes) {
    cpu->ret(big, bytes);
}

void common_iret(CPU* cpu, U32 big, U32 oldEip) {
    cpu->iret(big, oldEip);
}

void common_enter(CPU* cpu, U32 big, U32 bytes, U32 level) {
    cpu->enter(big, bytes, level);
}

void common_rdtsc(CPU* cpu, U32 extra) {
    U64 t = cpu->instructionCount+cpu->blockInstructionCount+extra;
    EAX = (U32)t;
    EDX = (U32)(t >> 32);
}

void common_log(CPU* cpu, DecodedOp* op) {
    op->log(cpu);
}

DecodedOp* common_getNextOp(CPU* cpu) {
    if (cpu->thread->terminating) {
        return nullptr;
    }
    return cpu->getNextOp();
}

U32 common_readCrx(CPU* cpu, U32 which, U32 reg) {
    return cpu->readCrx(which, reg);
}

U32 common_writeCrx(CPU* cpu, U32 which, U32 value) {
    return cpu->writeCrx(which, value);
}
