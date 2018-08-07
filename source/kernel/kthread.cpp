/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"

#include "kscheduler.h"
#include "ksignal.h"
#include "kscheduler.h"
#include "ksignal.h"
#include "../emulation/cpu/normal/normalCPU.h"

#include <string.h>
#include <setjmp.h>

KThread* KThread::runningThread;

bool KThreadTimer::run() {
    wakeThread(this->thread);
    return true;
}

void threadClearFutexes(KThread* thread);

KThread::~KThread() {    
    this->cleanup();
}

void KThread::cleanup() {
    if (this->waitingForSignalToEnd) {
        wakeThread(this->waitingForSignalToEnd);
        this->waitingForSignalToEnd = 0;
    }    
    if (this->clear_child_tid && this->process && this->process->memory->isValidWriteAddress(this->clear_child_tid, 4)) {
        writed(this->clear_child_tid, 0);
        this->futex(this->clear_child_tid, 1, 1, 0);
        this->clear_child_tid = 0;
    }
    if (this->waiting) {
        wakeThread(this); // remove there from any waiting queues
    }
    threadClearFutexes(this);
    unscheduleThread(this);
    if (this->process)
        this->process->removeThread(this);
}

void KThread::reset() {
    threadClearFutexes(this);
    this->cpu->reset();
    this->alternateStack = 0;
    this->alternateStackSize = 0;
    this->setupStack();    
}

void KThread::setupStack() {
    U32 page = 0;
    U32 pageCount = MAX_STACK_SIZE >> K_PAGE_SHIFT; // 1MB for max stack
    pageCount+=2; // guard pages
    if (!this->memory->findFirstAvailablePage(ADDRESS_PROCESS_STACK_START, pageCount, &page, false))
		if (!this->memory->findFirstAvailablePage(0xC0000, pageCount, &page, false))
			if (!this->memory->findFirstAvailablePage(0x80000, pageCount, &page, false))
				kpanic("Failed to allocate stack for thread");
    this->memory->allocPages(page+1, pageCount-2, PAGE_READ|PAGE_WRITE, 0, 0, 0);
    // 1 page above (catch stack underrun)
    this->memory->allocPages(page+pageCount-1, 1, 0, 0, 0, 0);
    // 1 page below (catch stack overrun)
    this->memory->allocPages(page, 1, 0, 0, 0, 0);
    this->stackPageCount = pageCount;
    this->stackPageStart = page;
    this->cpu->reg[4].u32 = (this->stackPageStart + this->stackPageCount - 1) << K_PAGE_SHIFT; // one page away from the top    
}

KThread::KThread(U32 id, KProcess* process) : 
    id(id),   
    sigMask(0),
    inSigMask(0),
    alternateStack(0),
    alternateStackSize(0),
    cpu(NULL),
    stackPageStart(0),
    stackPageCount(0),
    process(process),
    memory(0),
    interrupted(false),
    inSignal(0),
    clear_child_tid(0),
    userTime(0),
    kernelTime(0),
    inSysCall(0),
    waitingForSignalToEnd(NULL),
    waitingForSignalToEndMaskToRestore(0),
    pendingSignals(0),
    waiting(false),
    waitStartTime(0),
    waitType(0),
    timer(this),
    glContext(0),
    currentContext(0),
    log(false),
    scheduledThreadNode(this),
    waitThreadNode(this),
    waitNode(this)
    {
    int i;

    this->sigMask = 0;
    for (i=0;i<TLS_ENTRIES;i++) {
        this->tls[i].seg_not_present = 1;
        this->tls[i].read_exec_only = 1;
    }
    this->cpu = new NormalCPU();
    this->cpu->thread = this;
    this->memory = process->memory;
    if (process->name=="services.exe") {
        this->log=true;
    }
}

bool KThread::isLdtEmpty(struct user_desc* desc) {
    return (!desc || (desc->seg_not_present==1 && desc->read_exec_only==1));
}

struct user_desc* KThread::getLDT(U32 index) {
    if (index>=TLS_ENTRY_START_INDEX && index<TLS_ENTRIES+TLS_ENTRY_START_INDEX) {
        return &this->tls[index-TLS_ENTRY_START_INDEX];
    } else if (index<LDT_ENTRIES) {
        return this->process->getLDT(index);
    }
    return NULL;
}

U32 KThread::signal(U32 signal, bool wait) {
    if (signal==0)
        return 0;

    memset(process->sigActions[signal].sigInfo, 0, sizeof(process->sigActions[signal].sigInfo));
    process->sigActions[signal].sigInfo[0] = signal;
    process->sigActions[signal].sigInfo[2] = K_SI_USER;
    process->sigActions[signal].sigInfo[3] = process->id;
    process->sigActions[signal].sigInfo[4] = process->userId;

    if (((U64)1 << (signal-1)) & ~(this->inSignal?this->inSigMask:this->sigMask)) {
        // don't return -K_WAIT, we don't want to re-enter tgkill, instead we will return 0 once the thread wakes up

        // must set CPU state before runSignal since it will be stored
        this->cpu->reg[0].u32 = 0; 
        this->cpu->eip.u32+=2;

        this->runSignal(signal, -1, 0);
        if (wait) {
            this->waitingForSignalToEnd = KThread::currentThread();       
            waitThread(KThread::currentThread());			
            return -K_CONTINUE;
        }        
    } else {
        this->pendingSignals |= ((U64)1 << (signal-1));
    }
    return 0;
}

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define FUTEX_WAIT_PRIVATE 128
#define FUTEX_WAKE_PRIVATE 129

struct futex {
    KThread* thread;
    U8* address;  
    U32 expireTimeInMillies;
    bool wake;
};

#define MAX_FUTEXES 128

struct futex system_futex[MAX_FUTEXES];
#ifdef BOXEDWINE_VM
SDL_mutex* mutexFutex;
#endif

struct futex* getFutex(KThread* thread, U8* address) {
    int i=0;

    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].address == address && system_futex[i].thread==thread)
            return &system_futex[i];
    }
    return 0;
}

struct futex* allocFutex(KThread* thread, U8* address, U32 millies) {
    int i=0;

    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].thread==0) {
            system_futex[i].thread = thread;
            system_futex[i].address = address;
#ifdef BOXEDWINE_VM
            if (!system_futex[i].cond) {
                system_futex[i].cond = SDL_CreateCond();
            }
#else
            system_futex[i].expireTimeInMillies = millies;
            system_futex[i].wake = false;
#endif
            return &system_futex[i];
        }
    }
    kpanic("ran out of futexes");
    return 0;
}

void freeFutex(struct futex* f) {
    f->thread = 0;
    f->address = 0;
#ifdef BOXEDWINE_VM
    f->pendingWakeUp = 0;
#endif
}

void threadClearFutexes(KThread* thread) {
    U32 i;

    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].thread == thread) {
            freeFutex(&system_futex[i]);
        }
    }
}

U32 KThread::futex(U32 addr, U32 op, U32 value, U32 pTime) {
    U8* ramAddress = getPhysicalAddress(addr);

    if (ramAddress==0) {
        kpanic("Could not find futex address: %0.8X", addr);
    }
    if (op==FUTEX_WAIT || op==FUTEX_WAIT_PRIVATE) {
        struct futex* f=getFutex(this, ramAddress);
        U32 millies;

        if (f) {
            if (f->wake) {
                freeFutex(f);
                return 0;
            }
            if (f->expireTimeInMillies<=getMilliesSinceStart()) {
                freeFutex(f);
                return -K_ETIMEDOUT;
            }
            this->timer.millies = f->expireTimeInMillies;
            if (f->expireTimeInMillies<0xF0000000)
                this->timer.millies+=this->waitStartTime;
            addTimer(&this->timer);
            return -K_WAIT;
        }
        if (pTime == 0) {
            millies = 0xFFFFFFFF;
        } else {
            U32 seconds = readd(pTime);
            U32 nano = readd(pTime + 4);
            millies = seconds * 1000 + nano / 1000000 + getMilliesSinceStart();
        }
        if (readd(addr) != value) {
            return -K_EWOULDBLOCK;
        }
        f = allocFutex(this, ramAddress, millies);
        this->waitStartTime = getMilliesSinceStart();			
        this->timer.millies = f->expireTimeInMillies;
        if (f->expireTimeInMillies<0xF0000000)
            this->timer.millies+=this->waitStartTime;
        addTimer(&this->timer);
        return -K_WAIT;
    } else if (op==FUTEX_WAKE_PRIVATE || op==FUTEX_WAKE) {
        int i;
        U32 count = 0;
        for (i=0;i<MAX_FUTEXES && count<value;i++) {
            if (system_futex[i].address==ramAddress && !system_futex[i].wake) {
                system_futex[i].wake = true;
                wakeThread(system_futex[i].thread);				
                count++;
            }
        }
        return count;
    } else {
        kwarn("syscall __NR_futex op %d not implemented", op);
        return -1;
    }
}

void KThread::signalIllegalInstruction(int code) {
    memset(this->process->sigActions[K_SIGILL].sigInfo, 0, sizeof(this->process->sigActions[K_SIGILL].sigInfo));
    this->process->sigActions[K_SIGILL].sigInfo[0] = K_SIGILL;
    this->process->sigActions[K_SIGILL].sigInfo[2] = code;
    this->process->sigActions[K_SIGILL].sigInfo[3] = this->process->id;
    this->process->sigActions[K_SIGILL].sigInfo[4] = this->process->userId;
    this->runSignal(K_SIGILL, -1, 0);
}

bool KThread::runSignals() {
    U64 todo = this->process->pendingSignals & ~(this->inSignal?this->inSigMask:this->sigMask);
    todo |= this->pendingSignals & ~(this->inSignal?this->inSigMask:this->sigMask);

    if (todo!=0) {
        U32 i;

        for (i=0;i<32;i++) {
            if ((todo & ((U64)1 << i))!=0) {
                this->runSignal(i+1, -1, 0);
                return true;
            }
        }
    }	
    return false;
}
/*
typedef union compat_sigval {
        S32    sival_int;
        U32    sival_ptr;
} compat_sigval_t;

typedef struct compat_siginfo {
        S32 si_signo;
        S32 si_errno;
        S32 si_code;

        union {
                S32 _pad[29];

                // kill() 
                struct {
                        U32 _pid;      // sender's pid
                        U32 _uid;      // sender's uid
                } _kill;

                // POSIX.1b timers 
                struct {
                        S32 _tid;    // timer id 
                        S32 _overrun;           // overrun count 
                        compat_sigval_t _sigval;        // same as below 
                        S32 _sys_private;       // not to be passed to user 
                        S32 _overrun_incr;      // amount to add to overrun 
                } _timer;

                // POSIX.1b signals 
                struct {
                        U32 _pid;      // sender's pid 
                        U32 _uid;      // sender's uid 
                        compat_sigval_t _sigval;
                } _rt;

                // SIGCHLD 
                struct {
                        U32 _pid;      // which child 
                        U32 _uid;      // sender's uid
                        S32 _status;   // exit code 
                        S32 _utime;
                        S32 _stime;
                } _sigchld;

                // SIGCHLD (x32 version) 
                struct {
                        U32 _pid;      // which child
                        U32 _uid;      // sender's uid
                        S32 _status;   // exit code 
                        S64 _utime;
                        S64 _stime;
                } _sigchld_x32;

                // SIGILL, SIGFPE, SIGSEGV, SIGBUS 
                struct {
                        U32 _addr;     // faulting insn/memory ref. 
                } _sigfault;

                // SIGPOLL 
                struct {
                        S32 _band;      // POLL_IN, POLL_OUT, POLL_MSG 
                        S32 _fd;
                } _sigpoll;

                struct {
                        U32 _call_addr; // calling insn 
                        S32 _syscall;   // triggering system call number 
                        U32 _arch;     // AUDIT_ARCH_* of syscall 
                } _sigsys;
        } _sifields;
} compat_siginfo_t;
*/
typedef struct fpregset
  {
    union
      {
        struct fpchip_state
          {
            int state[27];
            int status;
          } fpchip_state;

        struct fp_emul_space
          {
            char fp_emul[246];
            char fp_epad[2];
          } fp_emul_space;

        int f_fpregs[62];
      } fp_reg_set;

    long int f_wregs[33];
  } fpregset_t;


// Number of general registers. 
#define NGREG   19

enum
{
  REG_GS = 0,
#define REG_GS  REG_GS
  REG_FS,
#define REG_FS  REG_FS
  REG_ES,
#define REG_ES  REG_ES
  REG_DS,
#define REG_DS  REG_DS
  REG_EDI,
#define REG_EDI REG_EDI
  REG_ESI,
#define REG_ESI REG_ESI
  REG_EBP,
#define REG_EBP REG_EBP
  REG_ESP,
#define REG_ESP REG_ESP
  REG_EBX,
#define REG_EBX REG_EBX
  REG_EDX,
#define REG_EDX REG_EDX
  REG_ECX,
#define REG_ECX REG_ECX
  REG_EAX,
#define REG_EAX REG_EAX
  REG_TRAPNO,
#define REG_TRAPNO      REG_TRAPNO
  REG_ERR,
#define REG_ERR REG_ERR
  REG_EIP,
#define REG_EIP REG_EIP
  REG_CS,
#define REG_CS  REG_CS
  REG_EFL,
#define REG_EFL REG_EFL
  REG_UESP,
#define REG_UESP        REG_UESP
  REG_SS
#define REG_SS  REG_SS
};

// Container for all general registers. 
typedef S32 gregset_t[NGREG];

// Context to describe whole processor state. 
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
  } mcontext_tt;

typedef struct sigaltstack {
        void *ss_sp;
        int ss_flags;
        S32 ss_size;
} stack_tt;

# define K_SIGSET_NWORDS (1024 / 32)
typedef struct
{
unsigned long int __val[K_SIGSET_NWORDS];
} k__sigset_t;


// Userlevel context. 
struct ucontext_ia32 {
        unsigned int      uc_flags;        // 0
        unsigned int      uc_link;         // 4
        stack_tt           uc_stack;        // 8
        mcontext_tt uc_mcontext;			   // 20
        k__sigset_t   uc_sigmask;   /* mask last for extensibility */
};


  
#define INFO_SIZE 128
#define CONTEXT_SIZE 128

void writeToContext(KThread* thread, U32 stack, U32 context, bool altStack, U32 trapNo, U32 errorNo) {	
    CPU* cpu = thread->cpu;

    if (altStack) {
        writed(context+0x8, thread->alternateStack);
        writed(context+0xC, K_SS_ONSTACK);
        writed(context+0x10, thread->alternateStackSize);
    } else {
        writed(context+0x8, thread->alternateStack);
        writed(context+0xC, K_SS_DISABLE);
        writed(context+0x10, 0);
    }
    writed(context+0x14, cpu->seg[GS].value);
    writed(context+0x18, cpu->seg[FS].value);
    writed(context+0x1C, cpu->seg[ES].value);
    writed(context+0x20, cpu->seg[DS].value);
    writed(context+0x24, cpu->reg[7].u32); // EDI
    writed(context+0x28, cpu->reg[6].u32); // ESI
    writed(context+0x2C, cpu->reg[5].u32); // EBP
    writed(context+0x30, stack); // ESP
    writed(context+0x34, cpu->reg[3].u32); // EBX
    writed(context+0x38, cpu->reg[2].u32); // EDX
    writed(context+0x3C, cpu->reg[1].u32); // ECX
    writed(context+0x40, cpu->reg[0].u32); // EAX
    writed(context+0x44, trapNo); // REG_TRAPNO
    writed( context+0x48, errorNo); // REG_ERR
    writed(context+0x4C, cpu->eip.u32);
    writed(context+0x50, cpu->seg[CS].value);
    writed(context+0x54, cpu->flags);
    writed(context+0x58, 0); // REG_UESP
    writed(context+0x5C, cpu->seg[SS].value);	
    writed(context+0x60, 0); // fpu save state
}

void readFromContext(CPU* cpu, U32 context) {
    cpu->setSegment(GS, readd(context+0x14));
    cpu->setSegment(FS, readd(context+0x18));
    cpu->setSegment(ES, readd(context+0x1C));
    cpu->setSegment(DS, readd(context+0x20));

    cpu->reg[7].u32 = readd(context+0x24); // EDI
    cpu->reg[6].u32 = readd(context+0x28); // ESI
    cpu->reg[5].u32 = readd(context+0x2C); // EBP
    cpu->reg[4].u32 = readd(context+0x30); // ESP

    cpu->reg[3].u32 = readd(context+0x34); // EBX
    cpu->reg[2].u32 = readd(context+0x38); // EDX
    cpu->reg[1].u32 = readd(context+0x3C); // ECX
    cpu->reg[0].u32 = readd(context+0x40); // EAX
    
    cpu->eip.u32 = readd(context+0x4C);
    cpu->setSegment(CS, readd(context+0x50));
    cpu->flags = readd(context+0x54);
    cpu->setSegment(SS, readd(context+0x5C));
}

U32 KThread::sigreturn() {
    memcopyToNative(this->cpu->reg[4].u32, (char*)&this->cpu, sizeof(CPU));
    //klog("signal return (threadId=%d)", thread->id);
    return -K_CONTINUE;
}

void OPCALL onExitSignal(CPU* cpu, DecodedOp* op) {
    U32 context;	
    U64 count = cpu->instructionCount;

    cpu->pop32(); // signal
    cpu->pop32(); // address
    context = cpu->pop32();
    cpu->thread->waitStartTime = cpu->pop32();
    cpu->thread->interrupted = cpu->pop32()!=0;

#ifdef LOG_OPS
    //klog("onExitSignal signal=%d info=%X context=%X stack=%X interrupted=%d", signal, address, context, cpu->reg[4].u32, cpu->thread->interrupted);
    //klog("    before context %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[4], cpu->segAddress[4], cpu->segAddress[4]?readd(cpu->memory, cpu->segAddress[4]+0x18):0);
#endif
    readFromContext(cpu, context);
#ifdef LOG_OPS
    klog("    after  context %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[4], cpu->segAddress[4], cpu->segAddress[4] ? readd(cpu->segAddress[4] + 0x18) : 0);
#endif
    cpu->instructionCount = count;
    cpu->thread->inSignal--;
    
    if (cpu->thread->waitingForSignalToEnd) {
        wakeThread(cpu->thread->waitingForSignalToEnd);
        cpu->thread->waitingForSignalToEnd = NULL;		
    }
    if (cpu->thread->waitingForSignalToEndMaskToRestore & RESTORE_SIGNAL_MASK) {
        cpu->thread->sigMask = cpu->thread->waitingForSignalToEndMaskToRestore & RESTORE_SIGNAL_MASK;
        cpu->thread->waitingForSignalToEndMaskToRestore = SIGSUSPEND_RETURN;
    }

    //cpu->nextBlock = getBlock(cpu, cpu->eip.u32);

    /*
    if (action->flags & K_SA_RESTORER) {
        push32(&thread->cpu, thread->cpu->eip.u32);
        thread->cpu->eip.u32 = action->restorer;
        while (thread->cpu->eip.u32!=savedState.eip.u32) {
            runCPU(&thread->cpu);
        }
    }
    */
}

// interrupted and waitStartTime are pushed because syscall's during the signal will clobber them
void KThread::runSignal(U32 signal, U32 trapNo, U32 errorNo) {
    KSigAction* action = &this->process->sigActions[signal];
    if (action->handlerAndSigAction==K_SIG_DFL) {

    } else if (action->handlerAndSigAction != K_SIG_IGN) {
        U32 context;
        U32 address = 0;
        U32 stack = this->cpu->reg[4].u32;
        U32 interrupted = 0;
        bool altStack = (action->flags & K_SA_ONSTACK) != 0;
        ChangeThread c(this);

        cpu->fillFlags();        

#ifdef LOG_OPS
        klog("runSignal %d", signal);
        klog("    before signal %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->seg[4].value, cpu->seg[4].address, cpu->seg[4].address?readd(cpu->seg[4].address+0x18):0);
#endif
        this->inSigMask=action->mask | this->sigMask;
        if (action->flags & K_SA_RESETHAND) {
            action->handlerAndSigAction=K_SIG_DFL;
        } else if (!(action->flags & K_SA_NODEFER)) {
            this->inSigMask|= (U64)1 << (signal-1);
        }
        if (this->waiting) {
            if (!(action->flags & K_SA_RESTART))
                interrupted = 1;
            wakeThread(this);
        }		
		if (altStack)
			context = this->alternateStack + this->alternateStackSize - CONTEXT_SIZE;
		else
	        context = this->cpu->seg[SS].address + (ESP & this->cpu->stackMask) - CONTEXT_SIZE;        
        writeToContext(this, stack, context, altStack, trapNo, errorNo);
        
        this->cpu->stackMask = 0xFFFFFFFF;
        this->cpu->stackNotMask = 0;
        this->cpu->seg[SS].address = 0;
        this->cpu->reg[4].u32 = context;

        this->cpu->reg[4].u32 &= ~15;
        if (action->flags & K_SA_SIGINFO) {
            U32 i;
            
            this->cpu->reg[4].u32-=INFO_SIZE;
            address = this->cpu->reg[4].u32;
            for (i=0;i<K_SIG_INFO_SIZE;i++) {
                writed(address+i*4, this->process->sigActions[signal].sigInfo[i]);
            }
                        
            this->cpu->push32(interrupted);
            this->cpu->push32(this->waitStartTime);
            this->cpu->push32(context);
            this->cpu->push32(address);			
            this->cpu->push32(signal);
            this->cpu->reg[0].u32 = signal;
            this->cpu->reg[1].u32 = address;
            this->cpu->reg[2].u32 = context;	
        } else {
            this->cpu->reg[0].u32 = signal;
            this->cpu->reg[1].u32 = 0;
            this->cpu->reg[2].u32 = 0;	
            this->cpu->push32(interrupted);
            this->cpu->push32(this->waitStartTime);
            this->cpu->push32(context);
            this->cpu->push32(0);			
            this->cpu->push32(signal);
        }
#ifdef LOG_OPS
        klog("    context %X interrupted %d", context, interrupted);
#endif
        this->cpu->push32(SIG_RETURN_ADDRESS);
        this->cpu->eip.u32 = action->handlerAndSigAction;

        this->inSignal++;				

        this->cpu->setSegment(CS, 0xf);        
        this->cpu->setSegment(SS, 0x17);
        this->cpu->setSegment(DS, 0x17);
        this->cpu->setSegment(ES, 0x17);
        this->cpu->big = 1;
    }    
    this->process->pendingSignals &= ~(1 << (signal - 1));
    this->pendingSignals &= ~(1 << (signal - 1));
}

extern jmp_buf runBlockJump;

// bit 0 - 0 = no page found, 1 = protection fault
// bit 1 - 0 = read access, 1 = write access
// bit 2 - 0 = kernel-mode access, 1 = user mode access
// bit 3 - 0 = n/a, 1 = use of reserved bit detected
// bit 4 - 0 = n/a, 1 = fault was an instruction fetch

void KThread::seg_mapper(U32 address, bool readFault, bool writeFault) {
    Memory* memory = this->process->memory;

    if (memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {
        U32 eip = this->cpu->eip.u32;

        memory->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        memory->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        memory->process->sigActions[K_SIGSEGV].sigInfo[2] = 1; // SEGV_MAPERR
        memory->process->sigActions[K_SIGSEGV].sigInfo[3] = address;
        this->runSignal(K_SIGSEGV, EXCEPTION_PAGE_FAULT, (writeFault?2:0));
#ifdef BOXEDWINE_HAS_SETJMP
        longjmp(runBlockJump, 1);		
#else
        kpanic("setjmp is required for this app but it was compiled into boxedwine");
#endif
    } else {
        this->memory->log_pf(this, address);
    }
}

void KThread::seg_access(U32 address, bool readFault, bool writeFault) {
    Memory* memory = this->process->memory;

    if (memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {
        U32 eip = this->cpu->eip.u32;

        memory->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        memory->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        memory->process->sigActions[K_SIGSEGV].sigInfo[2] = 2; // SEGV_ACCERR
        memory->process->sigActions[K_SIGSEGV].sigInfo[3] = address;        
        this->runSignal(K_SIGSEGV, EXCEPTION_PAGE_FAULT, 1 | (writeFault?2:0)); 
        printf("seg fault %X\n", address);
#ifdef BOXEDWINE_HAS_SETJMP
        longjmp(runBlockJump, 1);		
#else 
        kpanic("setjmp is required for this app but it was compiled into boxedwine");
#endif
    } else {
        this->memory->log_pf(this, address);
    }
}

void KThread::clone(KThread* from) {    
    this->sigMask = from->sigMask;
    this->stackPageStart = from->stackPageStart;
    this->stackPageCount = from->stackPageCount;
    this->waitingForSignalToEndMaskToRestore = from->waitingForSignalToEndMaskToRestore;
    this->cpu->clone(from->cpu);
    this->cpu->thread = this;
}

U32 KThread::modify_ldt(U32 func, U32 ptr, U32 count) {
    if (func == 1 || func == 0x11) {
        int index = readd(ptr);
        U32 address = readd(ptr + 4);
        U32 limit = readd(ptr + 8);
        U32 flags = readd(ptr + 12);

        if (index>=0 && index<LDT_ENTRIES) {
            struct user_desc* ldt = this->getLDT(index);;            
            
            ldt->entry_number = index;
            ldt->limit = limit;
            ldt->base_addr = address;
            ldt->flags = flags;
        } else {
            kpanic("syscall_modify_ldt invalid index: %d", index);
        }
        return 0;
    } else if (func == 0) {
        int index = readd(ptr);
        if (index>=0 && index<LDT_ENTRIES) {
            struct user_desc* ldt = this->getLDT(index);

            writed(ptr + 4, ldt->base_addr);
            writed(ptr + 8, ldt->limit);
            writed(ptr + 12, ldt->flags);
        } else {
            kpanic("syscall_modify_ldt invalid index: %d", index);
        }
        return 16;
    } else {
        kpanic("syscall_modify_ldt unknown func: %d", func);
        return -1;
    }
}

U32 KThread::sleep(U32 ms) {
    if (this->waitStartTime) {
        // if we were interupted, perhaps we ran a signal, now we need to continue sleeping
        U32 diff = getMilliesSinceStart()-this->waitStartTime;
        if (diff >= ms) {
            this->waitStartTime = 0;
            return 0;
        } else {
            addTimer(&this->timer);
            return -K_WAIT;
        }
    } else {
        this->waitStartTime = getMilliesSinceStart();
        this->timer.millies = this->waitStartTime+ms;
        addTimer(&this->timer);
        return -K_WAIT;
    }
}

U32 KThread::sigprocmask(U32 how, U32 set, U32 oset, U32 sigsetSize) {
    if (oset!=0) {
        if (sigsetSize==4)
            writed(oset, (U32)this->sigMask);
        else if (sigsetSize==8)
            writeq(oset, this->sigMask);
        else
            klog("sigprocmask: can't handle sigsetSize=%d", sigsetSize);
        //klog("syscall_sigprocmask oset=%X", thread->sigMask);
    }
    if (set!=0) {
        U64 mask;
        
        if (sigsetSize==4)
            mask = readd(set);
        else if (sigsetSize==8)
            mask = readq(set);
        else {
            klog("sigprocmask: can't handle sigsetSize=%d", sigsetSize);
            mask = 0; // removes warning
        }
        if (how == K_SIG_BLOCK) {
            this->sigMask|=mask;
            //klog("syscall_sigprocmask block %X(%X)", set, thread->sigMask);
        } else if (how == K_SIG_UNBLOCK) {
            this->sigMask&=~mask;
            //klog("syscall_sigprocmask unblock %X(%X)", set, thread->sigMask);
        } else if (how == K_SIG_SETMASK) {
            this->sigMask = mask;
            //klog("syscall_sigprocmask set %X(%X)", set, thread->sigMask);
        } else {
            kpanic("sigprocmask how %d unsupported", how);
        }
    }
    return 0;
}

U32 KThread::sigsuspend(U32 mask, U32 sigsetSize) {
    if (this->waitingForSignalToEndMaskToRestore==SIGSUSPEND_RETURN) {
        this->waitingForSignalToEndMaskToRestore = 0;
        return -K_EINTR;
    }
    this->waitingForSignalToEndMaskToRestore = this->sigMask | RESTORE_SIGNAL_MASK;
    if (sigsetSize==4)
        this->sigMask = readd(mask);
    else if (sigsetSize==8)
        this->sigMask = readq(mask);
    else
        klog("sigsuspend: can't handle sigsetSize=%d", sigsetSize);
    waitThread(this);			
    return -K_CONTINUE;
}

U32 KThread::signalstack(U32 ss, U32 oss) {
    if (oss!=0) {
        if (!this->memory->isValidWriteAddress(oss, 12)) {
            return -K_EFAULT;
        }
        writed(oss, this->alternateStack);
        writed(oss + 4, (this->alternateStack && this->inSignal) ? K_SS_ONSTACK : K_SS_DISABLE);
        writed(oss + 8, this->alternateStackSize);
    }
    if (ss!=0) {
        if (!this->memory->isValidReadAddress(ss, 12)) {
            return -K_EFAULT;
        }
        if (this->alternateStack && this->inSignal) {
            return -K_EPERM;
        }
        U32 flags = readd(ss + 4);
        if (flags & K_SS_DISABLE) {
            this->alternateStack = 0;
            this->alternateStackSize = 0;
        } else {
            this->alternateStack = readd(ss);
            this->alternateStackSize = readd(ss + 8);
        }
    }
    return 0;
}

KListNode<KThread*>* KThread::getWaitNofiyNode() {
    // this one node will handle all cases, except for syscalls that can wait on multiple objects, like select and poll
    if (!this->waitNode.isInList())
        return &this->waitNode;
    BoxedPtr<KListNode<KThread*> > p = new KListNode<KThread*>(this);
    this->extraWaitNodes.add(p);
    return p.get();
}

void KThread::clearWaitNofifyNodes() {
    this->waitNode.remove();
    this->extraWaitNodes.for_each([](BoxedPtr<KListNode<KThread*> > & node) {
        node->remove();
    });
    this->extraWaitNodes.removeAll();
}
