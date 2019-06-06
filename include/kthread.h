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

#ifndef __KTHREAD_H__
#define __KTHREAD_H__

#define MAX_POLL_DATA 256

#define TLS_ENTRIES 10
#define TLS_ENTRY_START_INDEX 10

class OpenGLVetexPointer {
public:
    OpenGLVetexPointer() : size(0), type(0), stride(0), count(0), ptr(0), marshal(NULL), marshal_size(0), refreshEachCall(0) {}
    U32 size;
    U32 type;
    U32 stride;
    U32 count; // used by marshalEdgeFlagPointerEXT
    U32 ptr;
    U8* marshal;
    U32 marshal_size;
    U32 refreshEachCall;
};

class KProcess;
class Memory;

class KThread {
public:
    KThread(U32 id, KProcess* process);
    ~KThread();

    void reset();

    struct user_desc* getLDT(U32 index);
    bool isLdtEmpty(struct user_desc* desc);
    U32 signal(U32 signal, bool wait);
    void cleanup();

    void seg_mapper(U32 address, bool readFault, bool writeFault, bool throwException=true);
    void seg_access(U32 address, bool readFault, bool writeFault, bool throwException=true);
    bool runSignals();
    void runSignal(U32 signal, U32 trapNo, U32 errorNo);
    void signalIllegalInstruction(int code);    
    void clone(KThread* from);
    void setupStack();
    void setTLS(struct user_desc* desc);

    // syscalls
    U32 futex(U32 addr, U32 op, U32 value, U32 pTime) ;
    U32 modify_ldt(U32 func, U32 ptr, U32 count);
    U32 signalstack(U32 ss, U32 oss);
    U32 sigprocmask(U32 how, U32 set, U32 oset, U32 sigsetSize);
    U32 sigreturn();
    U32 sigsuspend(U32 mask, U32 sigsetSize);
    U32 sleep(U32 ms);

    U32 id;
    U64 sigMask; // :TODO: what happens when this is changed while in a signal
    U64 inSigMask;
    U32 alternateStack;
    U32 alternateStackSize;
    CPU* cpu;
    U32 stackPageStart;
    U32 stackPageCount;
    KProcess* process;   
    Memory* memory;
    bool interrupted;
    U32 inSignal;    
    bool exiting;
#ifdef BOXEDWINE_MULTI_THREADED
    bool exited;
#endif
    U32 clear_child_tid;
    U64 userTime;
    U64 kernelTime;
    U32 inSysCall;
    BOXEDWINE_CONDITION waitingForSignalToEndCond;
    U64 waitingForSignalToEndMaskToRestore;    
    U64 pendingSignals;
    void* glContext;
    void* currentContext;
    bool log; // syscalls
    OpenGLVetexPointer glVertextPointer;
    OpenGLVetexPointer glNormalPointer;
    OpenGLVetexPointer glFogPointer;
    OpenGLVetexPointer glFogPointerEXT;
    OpenGLVetexPointer glColorPointer;
    OpenGLVetexPointer glSecondaryColorPointer;
    OpenGLVetexPointer glSecondaryColorPointerEXT;
    OpenGLVetexPointer glIndexPointer;
    OpenGLVetexPointer glTexCoordPointer;
    OpenGLVetexPointer glEdgeFlagPointer;
    OpenGLVetexPointer glEdgeFlagPointerEXT;

    inline static KThread* currentThread() {return runningThread;}
    inline static void setCurrentThread(KThread* thread){runningThread = thread; thread->memory->onThreadChanged();}

    BOXEDWINE_CONDITION *waitingCond;
    BOXEDWINE_CONDITION pollCond;
#ifdef BOXEDWINE_MULTI_THREADED
#else
    KListNode<KThread*> scheduledThreadNode;
    KListNode<KThread*> waitThreadNode;
        
    BoxedWineConditionTimer condTimer;
#endif

    U32 condStartWaitTime;
    bool srand_initialized;
private:
    void clearFutexes();

#ifdef BOXEDWINE_X64
    __declspec(thread) 
#endif
    static KThread* runningThread;    

    BOXEDWINE_CONDITION sleepCond;      

    struct user_desc tls[TLS_ENTRIES];
    BOXEDWINE_MUTEX tlsMutex;

    static BOXEDWINE_MUTEX futexesMutex;
};

class ChangeThread {
public:
    ChangeThread(KThread* thread);
    ~ChangeThread();
    KThread* savedThread;
};

#define SIGSUSPEND_RETURN 0xF000000000000000l
#define RESTORE_SIGNAL_MASK 0x0FFFFFFFFFFFFFFFl

void OPCALL onExitSignal(CPU* cpu, DecodedOp* op);

void common_signalIllegalInstruction(CPU* cpu, int code);

#endif
