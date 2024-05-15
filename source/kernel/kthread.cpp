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
#include <string.h>
#include "../io/fsfilenode.h"
#include "bufferaccess.h"
#include "kstat.h"

thread_local KThread* KThread::runningThread;

BOXEDWINE_MUTEX KThread::futexesMutex;

KThread::~KThread() {  
    for (auto& callback : callbacksOnExit) {
        callback(id);
    }
    this->cleanup();
    CPU* cpu = this->cpu;
    this->cpu = nullptr;
    delete cpu;
}

void KThread::cleanup() {
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->waitingForSignalToEndCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->waitingForSignalToEndCond);
    }
    if (!KSystem::shutingDown && this->clear_child_tid && this->process && memory->canWrite(this->clear_child_tid, 4)) {
        memory->writed(this->clear_child_tid, 0);
        this->futex(this->clear_child_tid, 1, 0xffffffff, 0, 0, 0, false);        
    }
	this->clear_child_tid = 0;
#ifndef BOXEDWINE_MULTI_THREADED
    if (this->waitingCond) {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(*this->waitingCond);
        this->waitThreadNode.remove();
        this->waitingCond = nullptr;
    }
#endif
    this->clearFutexes();    
    if (this->process) {
        this->process->removeThread(this);
    }
    unscheduleThread(this);
}

void KThread::reset() {
    this->clearFutexes();
    this->cpu->reset();
    this->alternateStack = 0;
    this->alternateStackSize = 0;
    this->setupStack();    
}

void KThread::setupStack() {  
    U32 stack = memory->mmap(this, 0, MAX_STACK_SIZE, K_PROT_NONE, K_MAP_ANONYMOUS|K_MAP_PRIVATE, -1, 0);
    // will all by on demand
    memory->mprotect(this, stack + K_PAGE_SIZE, MAX_STACK_SIZE - 2 * K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);    
    U32 stackPageCount = MAX_STACK_SIZE >> K_PAGE_SHIFT;
    U32 stackPageStart = stack >> K_PAGE_SHIFT;
    this->cpu->reg[4].u32 = (stackPageStart + stackPageCount - 1) << K_PAGE_SHIFT;  
    // touch the first 16 pages now so that they are ready
    for (int i = 1; i < 17; i++) {
        memory->readd(this->cpu->reg[4].u32 - K_PAGE_SIZE * i);
    }
}

KThread::KThread(U32 id, const std::shared_ptr<KProcess>& process) : 
    id(id),   
    process(process),
    memory(process->memory),    
    waitingForSignalToEndCond(std::make_shared<BoxedWineCondition>(B("KThread::waitingForSignalToEndCond"))),
    sigWaitCond(std::make_shared<BoxedWineCondition>(B("KThread::sigWaitCond"))),
    pollCond(std::make_shared<BoxedWineCondition>(B("KThread::pollCond"))),
#ifndef BOXEDWINE_MULTI_THREADED
    scheduledThreadNode(this),
    waitThreadNode(this),            
#endif
    sleepCond(std::make_shared<BoxedWineCondition>(B("KThread::sleepCond")))
    {

    this->sigMask = 0;
    for (int i=0;i<TLS_ENTRIES;i++) {
        this->tls[i].seg_not_present = 1;
        this->tls[i].read_exec_only = 1;
    }
    this->cpu = CPU::allocCPU(memory);
    this->cpu->thread = this;
    if (process->name=="services.exe") {
        this->log=true;
    }
    this->name = BString::valueOf(id);
    if (process->taskNode) {
        this->threadNode = Fs::addFileNode(process->taskNode->path + B("/") + BString::valueOf(id), B(""), B(""), true, process->taskNode);
        this->commNode = Fs::addVirtualFile(threadNode->path + B("/comm"), [this](const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
            return new BufferAccess(node, flags, &this->name);
            }, K__S_IREAD | K__S_IWRITE, k_mdev(0, 0), threadNode);
    }
    //BString tmp = BString::valueOf(id);
    //tmp += ".txt";
    //if (id==0x1c)
    //this->cpu->logFile = fopen(tmp.c_str(), "w");
}

bool KThread::isLdtEmpty(struct user_desc* desc) {
    return (!desc || (desc->seg_not_present==1 && desc->read_exec_only==1));
}

struct user_desc* KThread::getLDT(U32 index) {
    if (index>=TLS_ENTRY_START_INDEX && index<TLS_ENTRIES+TLS_ENTRY_START_INDEX) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(tlsMutex);
        return &this->tls[index-TLS_ENTRY_START_INDEX];
    } else if (index<LDT_ENTRIES) {
        return this->process->getLDT(index);
    }
    return nullptr;
}

void KThread::setTLS(struct user_desc* desc) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(tlsMutex);
    this->tls[desc->entry_number-TLS_ENTRY_START_INDEX] = *desc;   
}

bool KThread::readyForSignal(U32 signal) {
    return (((U64)1 << (signal - 1)) & ~(this->inSignal ? this->inSigMask : this->sigMask)) != 0;
}

U32 KThread::signal(U32 signal, bool wait) {
    if (signal==0) {
        return 0;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->sigWaitCond);
    if (this->sigWaitMask & signal) {
        this->foundWaitSignal = signal;
        BOXEDWINE_CONDITION_SIGNAL(this->sigWaitCond);        
        return 0;
    }
    memset(process->sigActions[signal].sigInfo, 0, sizeof(process->sigActions[signal].sigInfo));
    process->sigActions[signal].sigInfo[0] = signal;
    process->sigActions[signal].sigInfo[2] = K_SI_USER;
    process->sigActions[signal].sigInfo[3] = process->id;
    process->sigActions[signal].sigInfo[4] = process->userId;

    if (readyForSignal(signal)) {
        // don't return -K_WAIT, we don't want to re-enter tgkill, instead we will return 0 once the thread wakes up

        // must set CPU state before runSignal since it will be stored
        if (this==KThread::currentThread()) {
            this->cpu->reg[0].u32 = 0; 
            this->cpu->eip.u32+=2;
        } 
#ifdef BOXEDWINE_MULTI_THREADED                     
        else {
            // :TODO: how to interrupt the thread (the current approache assumes the thread will yield to the signal)
            {    
                bool handled = false;

                BOXEDWINE_CONDITION cond = waitingCond;
                if (signal == K_SIGQUIT && cond) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(cond);
                    if (waitingCond) {
                        this->startSignal = true;
                        this->runSignal(K_SIGQUIT, -1, 0);
                        BOXEDWINE_CONDITION_SIGNAL(cond);
                        handled = true;
                    }
                }
                if (!handled) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
                    this->pendingSignals |= ((U64)1 << (signal - 1));
                }
            }
            if (wait) {
                BOXEDWINE_CONDITION c = this->waitingCond;
                if (c) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(c);
                    BOXEDWINE_CONDITION_SIGNAL_ALL(c);
                }
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->waitingForSignalToEndCond);
                BOXEDWINE_CONDITION_WAIT(this->waitingForSignalToEndCond);
            }
            return 0;
        }
#endif
        this->runSignal(signal, -1, 0);
        if (wait && KThread::currentThread()!=this) {
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->waitingForSignalToEndCond);
            BOXEDWINE_CONDITION_WAIT(this->waitingForSignalToEndCond);
        }        
    } else {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
        this->pendingSignals |= ((U64)1 << (signal-1));
        this->process->signalFd(this, signal);
    }
    return 0;
}

#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_FD		2
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_WAIT_REQUEUE_PI	11
#define FUTEX_CMP_REQUEUE_PI	12

#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256
#define FUTEX_CMD_MASK		~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)

struct futex {
public:
    futex() : cond(std::make_shared<BoxedWineCondition>(B("futex"))) {}
    KThread* thread = nullptr;
    U64 address = 0;  
    U32 expireTimeInMillies = 0;
    U32 mask = 0;
    bool wake = false;
    BOXEDWINE_CONDITION cond;
};

#define MAX_FUTEXES 128

struct futex system_futex[MAX_FUTEXES];

struct futex* getFutex(KThread* thread, U64 address) {
    int i=0;

    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].address == address && system_futex[i].thread==thread) {
            return &system_futex[i];
        }
    }
    return nullptr;
}

struct futex* allocFutex(KThread* thread, U64 address, U32 millies) {
    BOXEDWINE_CRITICAL_SECTION;
    int i=0;

    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].thread== nullptr) {
            system_futex[i].thread = thread;
            system_futex[i].address = address;
            system_futex[i].expireTimeInMillies = millies;
            system_futex[i].wake = false;
            system_futex[i].mask = 0;
            return &system_futex[i];
        }
    }
    kpanic("ran out of futexes");
    return nullptr;
}

void freeFutex(struct futex* f) {
    f->thread = nullptr;
    f->address = 0;
}

void KThread::clearFutexes() {
    U32 i;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KThread::futexesMutex);
    for (i=0;i<MAX_FUTEXES;i++) {
        if (system_futex[i].thread == this) {
            freeFutex(&system_futex[i]);
        }
    }
}

U32 KThread::futex(U32 addr, U32 op, U32 value, U32 pTime, U32 val2, U32 val3, bool time64) {
    U64 ramAddress = 0;
    U32 cmd = (op & FUTEX_CMD_MASK);
    bool isPrivate = (op & FUTEX_PRIVATE_FLAG) != 0;

    if (isPrivate) {
        ramAddress = addr;
    } else {
        ramAddress = (U64)memory->getPtrForFutex(addr);
    }
    if (ramAddress == 0) {
        kpanic("Could not find futex address: %0.8X", addr);
    }
    /*
    * from kernel source, if I ever implement one of these I need to note pTime actually contains val2
    if (cmd == FUTEX_REQUEUE || cmd == FUTEX_CMP_REQUEUE ||
        cmd == FUTEX_CMP_REQUEUE_PI || cmd == FUTEX_WAKE_OP)
        val2 = (u32)(unsigned long)utime;
        */
    if (cmd ==FUTEX_WAIT || cmd == FUTEX_WAIT_BITSET) {
        //klog("%x/%x futux WAIT addr=%x op=%x val=%x ram=%x", id, process->id, addr, op, value, (U32)ramAddress);
        struct futex* f=getFutex(this, ramAddress);
        U32 expireTime = 0xFFFFFFFF;

        if (pTime != 0) {
            if (time64) {
                U64 seconds = memory->readq(pTime);
                U32 nano = memory->readd(pTime + 8);

                if (cmd == FUTEX_WAIT) {
                    // FUTEX_WAIT timeout is relative
                    expireTime = (U32)(seconds * 1000 + nano / 1000000);
                } else {
                    expireTime = (U32)((seconds * 1000 + nano / 1000000) - KSystem::getSystemTimeAsMicroSeconds() / 1000);
                }
            } else {
                U32 seconds = memory->readd(pTime);
                U32 nano = memory->readd(pTime + 4);

                if (cmd == FUTEX_WAIT) {
                    // FUTEX_WAIT timeout is relative
                    expireTime = seconds * 1000 + nano / 1000000;
                } else {
                    expireTime = (U32)((seconds * 1000 + nano / 1000000) - KSystem::getSystemTimeAsMicroSeconds() / 1000);
                }                                
            }
            expireTime += KSystem::getMilliesSinceStart();
        }
        if (!f) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KThread::futexesMutex);

            U32 currentValue = memory->readd(addr);
            if (currentValue != value) {
                //klog("   %x/%x futux addr=%x op=%x val=%x ram=%x NEW VALUE %x", id, process->id, addr, op, value, (U32)ramAddress, currentValue);
                return -K_EWOULDBLOCK;
            }

            f = allocFutex(this, ramAddress, expireTime);
            if (cmd == FUTEX_WAIT_BITSET) {
                f->mask = val3;
            }
        } else {
            int ii = 0;
        }
        while (true) {                        
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(f->cond);
            if (this->pendingSignals) {
                // I know this is a nested if statement, but it makes setting a break point easier
                if (runSignals()) {
                    //klog("   %x/%x futux addr=%x op=%x val=%x ram=%x RAN SIGNAL", id, process->id, addr, op, value, (U32)ramAddress);
                    return -K_CONTINUE;
                }
            }
            if (f->wake) {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KThread::futexesMutex);
                freeFutex(f);
                return 0;
            }       
            if (f->expireTimeInMillies<0x7FFFFFFF) {
                S32 diff = f->expireTimeInMillies - KSystem::getMilliesSinceStart();
                if (diff<=0) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KThread::futexesMutex);
                    freeFutex(f);
                    return -K_ETIMEDOUT;
                }
                //klog("   %x/%x futux SLEEPING %x addr=%x op=%x val=%x ram=%x", id, process->id, (U32)diff, addr, op, value, (U32)ramAddress);
                BOXEDWINE_CONDITION_WAIT_TIMEOUT(f->cond, (U32)diff);
                //klog("   %x/%x futux DONE SLEEPING %x addr=%x op=%x val=%x ram=%x", id, process->id, (U32)diff, addr, op, value, (U32)ramAddress);
            } else {
                BOXEDWINE_CONDITION_WAIT(f->cond);
            }
#ifdef BOXEDWINE_MULTI_THREADED
			if (this->terminating) {
				return -K_EINTR;
			}
            if (KThread::currentThread()->startSignal) {
                KThread::currentThread()->startSignal = false;
                return -K_CONTINUE;
            }
#endif
        }
    } else if (cmd ==FUTEX_WAKE || cmd == FUTEX_WAKE_BITSET) {
        U32 count = 0;
        //klog("%x/%x futux wake addr=%x op=%x val=%x ram=%x", id, process->id, addr, op, value, (U32)ramAddress);
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KThread::futexesMutex);            

            for (int i = 0; i < MAX_FUTEXES && count < value; i++) {
                if (!system_futex[i].thread) {
                    continue;
                }
                bool processCheck = (!isPrivate || system_futex[i].thread->process->id == this->process->id);
                bool addressCheck = system_futex[i].address == ramAddress;
                bool maskCheck = ((cmd != FUTEX_WAKE_BITSET) || (system_futex[i].mask & val3));
                if (processCheck && addressCheck && !system_futex[i].wake && maskCheck) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(system_futex[i].cond);
                    system_futex[i].wake = true;
                    BOXEDWINE_CONDITION_SIGNAL(system_futex[i].cond);
                    count++;
                }
            }
        }
        //klog("    %x/%x futux wake finished addr=%x op=%x val=%x ram=%x", id, process->id, addr, op, value, (U32)ramAddress);
        return count;
    } else {
        kwarn("syscall __NR_futex op %d not implemented", op);
        return -1;
    }
}

static U8 fetchByte(void* data, U32* eip) {
    KMemory* memory = (KMemory*)data;
    return memory->readb((*eip)++);
}

void KThread::signalTrap(U32 code) {
    KSigAction* action = &this->process->sigActions[K_SIGTRAP];
    if (action->handlerAndSigAction == K_SIG_DFL) {
        DecodedBlock block;
        decodeBlock(fetchByte, memory, cpu->eip.u32 + cpu->seg[CS].address, cpu->isBig(), 1, K_PAGE_SIZE, 0, &block);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
        kpanic("%s tid=%04X eip=%08X Illegal instruction but no signal handler set up for it: %s: (%X)", process->name.c_str(), cpu->thread->id, cpu->eip.u32, block.op->name(), block.op->originalOp);
#else
        kpanic("%s tid=%04X eip=%08X Illegal instruction but no signal handler set up for it: %s (%X)", process->name.c_str(), cpu->thread->id, cpu->eip.u32, block.op->name(), block.op->inst);
#endif
    }
    memset(this->process->sigActions[K_SIGTRAP].sigInfo, 0, sizeof(this->process->sigActions[K_SIGTRAP].sigInfo));
    this->process->sigActions[K_SIGTRAP].sigInfo[0] = K_SIGTRAP;
    this->process->sigActions[K_SIGTRAP].sigInfo[2] = code;
    this->process->sigActions[K_SIGTRAP].sigInfo[3] = cpu->eip.u32;
    this->runSignal(K_SIGTRAP, 3, 0);
}

void KThread::signalIllegalInstruction(int code) {
    KSigAction* action = &this->process->sigActions[K_SIGILL];
    if (action->handlerAndSigAction == K_SIG_DFL) {
        DecodedBlock block;
        decodeBlock(fetchByte, memory, cpu->eip.u32 + cpu->seg[CS].address, cpu->isBig(), 1, K_PAGE_SIZE, 0, &block);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
        kpanic("%s tid=%04X eip=%08X Illegal instruction but no signal handler set up for it: %s: (%X)", process->name.c_str(), cpu->thread->id, cpu->eip.u32, block.op->name(), block.op->originalOp);
#else
        kpanic("%s tid=%04X eip=%08X Illegal instruction but no signal handler set up for it: %s (%X)", process->name.c_str(), cpu->thread->id, cpu->eip.u32, block.op->name(), block.op->inst);
#endif
    }
    memset(this->process->sigActions[K_SIGILL].sigInfo, 0, sizeof(this->process->sigActions[K_SIGILL].sigInfo));
    this->process->sigActions[K_SIGILL].sigInfo[0] = K_SIGILL;
    this->process->sigActions[K_SIGILL].sigInfo[2] = code;
    this->process->sigActions[K_SIGILL].sigInfo[3] = cpu->eip.u32;
    this->runSignal(K_SIGILL, 13, 0); // blocking signal, signalfd can't handle this
}

bool KThread::runSignals() {
    U64 todoProcess = this->process->pendingSignals & ~(this->inSignal?this->inSigMask:this->sigMask);
    U64 todoThread = this->pendingSignals & ~(this->inSignal?this->inSigMask:this->sigMask);

    if (todoProcess!=0 || todoThread!=0) {
        U32 i;

        for (i=0;i<32;i++) {
            if ((todoProcess & ((U64)1 << i))!=0) {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->process->pendingSignalsMutex);
                if ((this->process->pendingSignals & ((U64)1 << i))!=0 || i + 1 == K_SIGKILL) { // SIGKILL can't be ignored
                    this->process->pendingSignals &= ~(1 << i);
                    this->runSignal(i+1, -1, 0);
                    return true;
                }
            }
            if ((todoThread & ((U64)1 << i))!=0) {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
                if ((this->pendingSignals & ((U64)1 << i))!=0 || i+1 == K_SIGKILL) { // SIGKILL can't be ignored
                    this->pendingSignals &= ~(1 << i);
                    this->runSignal(i+1, -1, 0);
                    return true;
                }
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
        k__sigset_t   uc_sigmask;   // mask last for extensibility 
};

*/
  
#define INFO_SIZE 128
#define CONTEXT_SIZE 128

void writeToContext(KThread* thread, U32 stack, U32 context, bool altStack, U32 trapNo, U32 errorNo) {	
    CPU* cpu = thread->cpu;
    KMemory* memory = thread->memory;

    if (altStack) {
        memory->writed(context+0x8, thread->alternateStack);
        memory->writed(context+0xC, K_SS_ONSTACK);
        memory->writed(context+0x10, thread->alternateStackSize);
    } else {
        memory->writed(context+0x8, thread->alternateStack);
        memory->writed(context+0xC, K_SS_DISABLE);
        memory->writed(context+0x10, 0);
    }
    memory->writed(context+0x14, cpu->seg[GS].value);
    memory->writed(context+0x18, cpu->seg[FS].value);
    memory->writed(context+0x1C, cpu->seg[ES].value);
    memory->writed(context+0x20, cpu->seg[DS].value);
    memory->writed(context+0x24, cpu->reg[7].u32); // EDI
    memory->writed(context+0x28, cpu->reg[6].u32); // ESI
    memory->writed(context+0x2C, cpu->reg[5].u32); // EBP
    memory->writed(context+0x30, stack); // ESP
    memory->writed(context+0x34, cpu->reg[3].u32); // EBX
    memory->writed(context+0x38, cpu->reg[2].u32); // EDX
    memory->writed(context+0x3C, cpu->reg[1].u32); // ECX
    memory->writed(context+0x40, cpu->reg[0].u32); // EAX
    memory->writed(context+0x44, trapNo); // REG_TRAPNO
    memory->writed(context+0x48, errorNo); // REG_ERR
    memory->writed(context+0x4C, cpu->isBig()?cpu->eip.u32:cpu->eip.u16);
    memory->writed(context+0x50, cpu->seg[CS].value);
    memory->writed(context+0x54, cpu->flags);
    memory->writed(context+0x58, 0); // REG_UESP
    memory->writed(context+0x5C, cpu->seg[SS].value);
    memory->writed(context+0x60, 0); // fpu save state
}

void readFromContext(CPU* cpu, U32 context) {
    KMemory* memory = (KMemory*)cpu->memory;

    cpu->setSegment(GS, memory->readd(context+0x14));
    cpu->setSegment(FS, memory->readd(context+0x18));
    cpu->setSegment(ES, memory->readd(context+0x1C));
    cpu->setSegment(DS, memory->readd(context+0x20));

    cpu->reg[7].u32 = memory->readd(context+0x24); // EDI
    cpu->reg[6].u32 = memory->readd(context+0x28); // ESI
    cpu->reg[5].u32 = memory->readd(context+0x2C); // EBP
    cpu->reg[4].u32 = memory->readd(context+0x30); // ESP

    cpu->reg[3].u32 = memory->readd(context+0x34); // EBX
    cpu->reg[2].u32 = memory->readd(context+0x38); // EDX
    cpu->reg[1].u32 = memory->readd(context+0x3C); // ECX
    cpu->reg[0].u32 = memory->readd(context+0x40); // EAX
    
    cpu->eip.u32 = memory->readd(context+0x4C);
    cpu->setSegment(CS, memory->readd(context+0x50));
    cpu->flags = memory->readd(context+0x54);
    cpu->setSegment(SS, memory->readd(context+0x5C));
}

U32 KThread::sigreturn() {
    kpanic("KThread::sigreturn This code seems wrong, we never did a memcpy to memory of CPU, should this be readContext.  Debug this after finding a program that triggers it");
    memory->memcpy(&this->cpu, this->cpu->reg[4].u32, sizeof(CPU));
    //klog("signal return (threadId=%d)", thread->id);
    return -K_CONTINUE;
}

// https://github.com/torvalds/linux/blob/master/kernel/rseq.c
/*
struct rseq {
    U32 cpu_id_start;
    U32 cpu_id;
    U32 rseq_cs;
    U32 padding;
    U32 flags;
};
*/
// I'm not sure if this will every be able to be emulated properly, I might be stuck using Debian 11.  Debian 12 glibc uses this.
U32 KThread::rseq(U32 rseq, U32 rseq_len, U32 flags, U32 sig) {
    memory->writed(rseq, 0);
    memory->writed(rseq+4, 0);
    return 0;
}

void OPCALL onExitSignal(CPU* cpu, DecodedOp* op) {
    U64 count = cpu->instructionCount;

    cpu->pop32(); // signal
    cpu->pop32(); // address
    U32 context = cpu->pop32();
    cpu->thread->condStartWaitTime = cpu->pop32();
    cpu->thread->interrupted = cpu->pop32()!=0;

#ifdef LOG_OPS
    //klog("onExitSignal signal=%d info=%X context=%X stack=%X interrupted=%d", signal, address, context, cpu->reg[4].u32, cpu->thread->interrupted);
    //klog("    before context %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->segValue[4], cpu->segAddress[4], cpu->segAddress[4]?readd(cpu->memory, cpu->segAddress[4]+0x18):0);
#endif
    readFromContext(cpu, context);
#ifdef LOG_OPS
    klog("    after  context %.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X fs=%d(%X) fs18=%X", cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->seg[4].value, cpu->seg[4].address, cpu->seg[4].address ? readd(cpu->seg[4].address + 0x18) : 0);
#endif
    cpu->instructionCount = count;
    cpu->thread->inSignal--;
    
    if (cpu->thread->waitingForSignalToEndMaskToRestore & RESTORE_SIGNAL_MASK) {
        cpu->thread->sigMask = cpu->thread->waitingForSignalToEndMaskToRestore & RESTORE_SIGNAL_MASK;
        cpu->thread->waitingForSignalToEndMaskToRestore = SIGSUSPEND_RETURN;
    }

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(cpu->thread->waitingForSignalToEndCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(cpu->thread->waitingForSignalToEndCond);
    }

#ifndef BOXEDWINE_BINARY_TRANSLATOR
    cpu->nextBlock = cpu->getNextBlock();
#endif
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

// interrupted and condStartWaitTime are pushed because syscall's during the signal will clobber them
void KThread::runSignal(U32 signal, U32 trapNo, U32 errorNo) {
    KSigAction* action = &this->process->sigActions[signal];
    if (action->handlerAndSigAction==K_SIG_DFL) {
        //klog("%x/%x default runSignal %d", this->id, this->process->id, signal);
        if (signal == K_SIGURG || signal == K_SIGCONT || signal == K_SIGCHLD || signal == K_SIGWINCH) {
            // ignored by default
        } else {
            process->signal(K_SIGKILL);
        }
    } else if (action->handlerAndSigAction != K_SIG_IGN) {
        U32 context = 0;
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
#ifndef BOXEDWINE_MULTI_THREADED
        if (this->waitingCond) {
            if (!(action->flags & K_SA_RESTART))
                interrupted = 1;
            this->waitingCond->signalAll(); // this will make sure it gets cleaned up properly
        }		
#endif
        // move to front of the queue
#ifndef BOXEDWINE_MULTI_THREADED
        unscheduleThread(this);
        scheduleThread(this);
#endif
		if (altStack) {
			context = this->alternateStack + this->alternateStackSize - CONTEXT_SIZE;
        } else {
	        context = this->cpu->seg[SS].address + (ESP & this->cpu->stackMask) - CONTEXT_SIZE;        
        }
        writeToContext(this, stack, context, altStack, trapNo, errorNo);
        
        this->cpu->stackMask = 0xFFFFFFFF;
        this->cpu->stackNotMask = 0;
        this->cpu->seg[SS].address = 0;
        this->cpu->reg[4].u32 = context;

        this->cpu->reg[4].u32 &= ~15;
        if (action->flags & K_SA_SIGINFO) {            
            this->cpu->reg[4].u32-=INFO_SIZE;
            address = this->cpu->reg[4].u32;
            for (U32 i=0;i<K_SIG_INFO_SIZE;i++) {
                memory->writed(address+i*4, this->process->sigActions[signal].sigInfo[i]);
            }
                        
            this->cpu->push32(interrupted);
            this->cpu->push32(this->condStartWaitTime);
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
            this->cpu->push32(this->condStartWaitTime);
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
        this->cpu->setIsBig(1);
#ifdef BOXEDWINE_MULTI_THREADED
        if (!this->startSignal) {
            BOXEDWINE_CONDITION cond = this->waitingCond;
            if (cond) {
                this->startSignal = true;
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(cond);
                BOXEDWINE_CONDITION_SIGNAL_ALL(cond);
            }
        }
#endif
    } else {
        //klog("%x/%x ignore runSignal %d", this->id, this->process->id, signal);
    }
}

// bit 0 - 0 = no page found, 1 = protection fault
// bit 1 - 0 = read access, 1 = write access
// bit 2 - 0 = kernel-mode access, 1 = user mode access
// bit 3 - 0 = n/a, 1 = use of reserved bit detected
// bit 4 - 0 = n/a, 1 = fault was an instruction fetch

void KThread::seg_mapper(U32 address, bool readFault, bool writeFault, bool throwException) {
    if (this->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && this->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {
        this->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        this->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        this->process->sigActions[K_SIGSEGV].sigInfo[2] = 1; // SEGV_MAPERR
        this->process->sigActions[K_SIGSEGV].sigInfo[3] = address;
        this->runSignal(K_SIGSEGV, EXCEPTION_PAGE_FAULT, (writeFault?2:0));
        if (throwException) {
            throw 2;
        }
    } else {
        this->memory->logPageFault(this, address);
    }
}

// motorhead demo installer, tomb raider 3 demo will trigger this
void KThread::seg_access(U32 address, bool readFault, bool writeFault, bool throwException) {
    if (this->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && this->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {

        this->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        this->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        this->process->sigActions[K_SIGSEGV].sigInfo[2] = 2; // SEGV_ACCERR
        this->process->sigActions[K_SIGSEGV].sigInfo[3] = address;        
        this->runSignal(K_SIGSEGV, EXCEPTION_PAGE_FAULT, 1 | (writeFault?2:0)); 
        if (throwException) {
            throw 1;
        }
    } else {
        this->memory->logPageFault(this, address);
    }
}

void KThread::clone(KThread* from) {    
    this->sigMask = from->sigMask;
    this->waitingForSignalToEndMaskToRestore = from->waitingForSignalToEndMaskToRestore;
    this->cpu->clone(from->cpu);
    this->cpu->thread = this;
}

U32 KThread::modify_ldt(U32 func, U32 ptr, U32 count) {
    if (func == 1 || func == 0x11) {
        int index = memory->readd(ptr);
        U32 address = memory->readd(ptr + 4);
        U32 limit = memory->readd(ptr + 8);
        U32 flags = memory->readd(ptr + 12);

        if (index>=0 && index<LDT_ENTRIES) {
            struct user_desc* ldt = this->getLDT(index);            
            
            ldt->entry_number = index;
            ldt->limit = limit;
            ldt->base_addr = address;
            ldt->flags = flags;
        } else {
            kpanic("syscall_modify_ldt invalid index: %d", index);
        }
        return 0;
    } else if (func == 0) {
        int index = memory->readd(ptr);
        if (index>=0 && index<LDT_ENTRIES) {
            struct user_desc* ldt = this->getLDT(index);

            memory->writed(ptr + 4, ldt->base_addr);
            memory->writed(ptr + 8, ldt->limit);
            memory->writed(ptr + 12, ldt->flags);
        } else {
            kpanic("syscall_modify_ldt invalid index: %d", index);
        }
        return 16;
    } else {
        kpanic("syscall_modify_ldt unknown func: %d", func);
        return -K_ENOSYS;
    }
}

U32 KThread::nanoSleep(U64 nano) {
    U32 millies = (U32)(nano / 1000000);
    if (millies > NUMBER_OF_MILLIES_TO_SPIN_FOR_WAIT) {
        return sleep(millies);
    }
    return Platform::nanoSleep(nano);
}

U32 KThread::clockNanoSleep(U32 clock, U32 flags, U64 nano, U32 addressRemain) {
    if (flags) {
        kpanic("clockNanoSleep flags (%x) does not equal 0: this has not been implemented yet", flags);
    }
    return Platform::nanoSleep(nano);
}

U32 KThread::sleep(U32 ms) {
    if (ms <= NUMBER_OF_MILLIES_TO_SPIN_FOR_WAIT) {
        return Platform::nanoSleep(((U64)ms) * 1000000l);
    }
    while (true) {
        if (!this->condStartWaitTime) {
            this->condStartWaitTime = KSystem::getMilliesSinceStart();
        } else {
            U32 diff = KSystem::getMilliesSinceStart()-this->condStartWaitTime;
            if (diff>ms) {
                this->condStartWaitTime = 0;
                return 0;
            }
            ms-=diff;
        }

        {
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->sleepCond);
            BOXEDWINE_CONDITION_WAIT_TIMEOUT(this->sleepCond, ms);
        }
#ifdef BOXEDWINE_MULTI_THREADED
		if (this->terminating) {
			return -K_EINTR;
		}
        if (KThread::currentThread()->startSignal) {
            KThread::currentThread()->startSignal = false;
            return -K_CONTINUE;
        }
#endif
    }
}

U32 KThread::sigprocmask(U32 how, U32 set, U32 oset, U32 sigsetSize) {
    if (oset!=0) {
        if (sigsetSize==4) {
            memory->writed(oset, (U32)this->sigMask);
        } else if (sigsetSize==8) {
            memory->writeq(oset, this->sigMask);
        } else {
            klog("sigprocmask: can't handle sigsetSize=%d", sigsetSize);
        }
        //klog("syscall_sigprocmask oset=%X", thread->sigMask);
    }
    if (set!=0) {
        U64 mask = 0;
        
        if (sigsetSize==4) {
            mask = memory->readd(set);
        } else if (sigsetSize==8) {
            mask = memory->readq(set);
        } else {
            klog("sigprocmask: can't handle sigsetSize=%d", sigsetSize);
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

U32 KThread::sigtimedwait(U32 set, U32 info, U32 timeout, U32 sizeofSet, bool time64) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(sigWaitCond);
    U64 mask = 0;
    
    if (sizeofSet == 8) {
        mask = memory->readq(set);
    } else if (sizeofSet == 4) {
        mask = memory->readd(set);
    } else {
        kpanic("KThread::sigtimedwait unhandled sigSetSize %d", sizeofSet);
    }

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
        for (int i = 0; i < 64; i++) {
            if ((mask & (1ll << i)) && (this->pendingSignals & (1ll << i))) {
                this->pendingSignals &= ~(1ll << i);
                return 0;
            }
        }
    }
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->pendingSignalsMutex);
        for (int i = 0; i < 64; i++) {
            if ((mask & (1ll << i)) && (process->pendingSignals & (1ll << i))) {
                this->pendingSignals &= ~(1ll << i);
                return 0;
            }
        }
    }
    this->sigWaitMask = mask;
    if (!timeout) {
        BOXEDWINE_CONDITION_WAIT(sigWaitCond);
#ifdef BOXEDWINE_MULTI_THREADED
        if (this->startSignal) {
            this->startSignal = false;
            this->sigWaitMask = 0;
            return -K_CONTINUE;
        }
#endif
    } else {
        U32 ms = 0;
        U64 startTime = KSystem::getMilliesSinceStart();
        if (time64) {
            U64 seconds = memory->readq(timeout);
            U32 nano = memory->readd(timeout + 8);
            ms = (U32)(seconds * 1000 + nano / 1000000);
        } else {
            U32 seconds = memory->readd(timeout);
            U32 nano = memory->readd(timeout + 4);
            ms = (U32)(seconds * 1000 + nano / 1000000);
        }
        if (ms == 0) {
            this->sigWaitMask = 0;
            return -K_EAGAIN;
        }
        BOXEDWINE_CONDITION_WAIT_TIMEOUT(sigWaitCond, ms);
#ifdef BOXEDWINE_MULTI_THREADED
        if (this->startSignal) {
            this->startSignal = false;
            this->sigWaitMask = 0;
            return -K_CONTINUE;
        }
#endif
        if (!foundWaitSignal && (KSystem::getMilliesSinceStart() - startTime) > ms) {
            this->sigWaitMask = 0;
            return -K_EAGAIN;
        }
    }
    this->sigWaitMask = 0;
    if (foundWaitSignal) {
        return 0;
    }
    return -K_EINTR;
}

U32 KThread::sigsuspend(U32 mask, U32 sigsetSize) {
    if (this->waitingForSignalToEndMaskToRestore==SIGSUSPEND_RETURN) {
        this->waitingForSignalToEndMaskToRestore = 0;
        return -K_EINTR;
    }
    this->waitingForSignalToEndMaskToRestore = this->sigMask | RESTORE_SIGNAL_MASK;
    if (sigsetSize==4) {
        this->sigMask = memory->readd(mask);
    } else if (sigsetSize==8) {
        this->sigMask = memory->readq(mask);
    } else {
        klog("sigsuspend: can't handle sigsetSize=%d", sigsetSize);
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->waitingForSignalToEndCond);
    BOXEDWINE_CONDITION_WAIT(this->waitingForSignalToEndCond);
#ifdef BOXEDWINE_MULTI_THREADED
    this->startSignal = false;
    return -K_CONTINUE; // so that cpu.eip is not incremented by syscall
#endif
}

U32 KThread::signalstack(U32 ss, U32 oss) {
    if (oss!=0) {
        if (!this->memory->canWrite(oss, 12)) {
            return -K_EFAULT;
        }
        memory->writed(oss, this->alternateStack);
        memory->writed(oss + 4, (this->alternateStack && this->inSignal) ? K_SS_ONSTACK : K_SS_DISABLE);
        memory->writed(oss + 8, this->alternateStackSize);
    }
    if (ss!=0) {
        if (!this->memory->canRead(ss, 12)) {
            return -K_EFAULT;
        }
        if (this->alternateStack && this->inSignal) {
            return -K_EPERM;
        }
        U32 flags = memory->readd(ss + 4);
        if (flags & K_SS_DISABLE) {
            this->alternateStack = 0;
            this->alternateStackSize = 0;
        } else {
            this->alternateStack = memory->readd(ss);
            this->alternateStackSize = memory->readd(ss + 8);
        }
    }
    return 0;
}

std::shared_ptr<KThreadGlContext> KThread::getGlContextById(U32 id) {
    return this->glContext[id];
}

void KThread::removeGlContextById(U32 id) {
    this->glContext.remove(id);
}

void KThread::addGlContext(U32 id, void* context) {
    this->glContext.set(id, std::make_shared<KThreadGlContext>(context));
}

void KThread::removeAllGlContexts() {
    this->glContext.clear();
}

ChangeThread::ChangeThread(KThread* thread) {
    this->savedThread = KThread::currentThread();
    KThread::setCurrentThread(thread);
}
ChangeThread::~ChangeThread() {
    KThread::setCurrentThread(savedThread);
}

void common_signalIllegalInstruction(CPU* cpu, int code) {
    cpu->thread->signalIllegalInstruction(code);
}
