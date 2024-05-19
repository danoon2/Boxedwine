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
    OpenGLVetexPointer() = default;
    U32 size = 0;
    U32 type = 0;
    U32 stride = 0;
    U32 count = 0; // used by marshalEdgeFlagPointerEXT
    U32 ptr = 0;
    U8* marshal = nullptr;
    U32 marshal_size = 0;
    U32 refreshEachCall = 0;
};

class KProcess;
class Memory;

class KThreadGlContext {
public:
    KThreadGlContext() = default;
    KThreadGlContext(void* context):context(context) {}
    void* context = nullptr;
    bool hasBeenMadeCurrent = false;
    bool sharing = false;
};

class KThread {
public:
    KThread(U32 id, const std::shared_ptr<KProcess>& process);
    ~KThread();

    void addCallbackOnExit(std::function<void(U32 id)> callback) {callbacksOnExit.push_back(callback);}

    void reset();

    struct user_desc* getLDT(U32 index);
    bool isLdtEmpty(struct user_desc* desc);
    U32 signal(U32 signal, bool wait);
    bool readyForSignal(U32 signal);
    void cleanup();

    void seg_mapper(U32 address, bool readFault, bool writeFault, bool throwException=true);
    void seg_access(U32 address, bool readFault, bool writeFault, bool throwException=true);
    bool runSignals();
    void runSignal(U32 signal, U32 trapNo, U32 errorNo);
    void signalIllegalInstruction(int code);   
    void signalTrap(U32 code);
    void clone(KThread* from);
    void setupStack();
    void setTLS(struct user_desc* desc);

    // syscalls
    U32 futex(U32 addr, U32 op, U32 value, U32 pTime, U32 val2, U32 val3, bool time64) ;
    U32 modify_ldt(U32 func, U32 ptr, U32 count);
    U32 signalstack(U32 ss, U32 oss);
    U32 sigprocmask(U32 how, U32 set, U32 oset, U32 sigsetSize);
    U32 sigreturn();
    U32 rseq(U32 rseq, U32 rseq_len, U32 flags, U32 sig);
    U32 sigsuspend(U32 mask, U32 sigsetSize);
    U32 sigtimedwait(U32 set, U32 info, U32 timeout, U32 sizeofSet, bool time64);
    U32 sleep(U32 ms);
    U32 nanoSleep(U64 nano);
    U32 clockNanoSleep(U32 clock, U32 flags, U64 nano, U32 addressRemain);

    U32 id = 0;
    U64 sigMask = 0; // :TODO: what happens when this is changed while in a signal
    U64 inSigMask = 0;
    U32 alternateStack = 0;
    U32 alternateStackSize = 0;
    CPU* cpu = nullptr;
    std::shared_ptr<KProcess> process;
    KMemory* const memory;
    bool interrupted = false;
    U32 inSignal = 0;    
#ifdef BOXEDWINE_MULTI_THREADED
    bool exited = false;	
    bool startSignal = false;
#endif
    bool terminating = false;
    U32 clear_child_tid = 0;
    U64 userTime = 0;
    U64 kernelTime = 0;
    U32 inSysCall = 0;
    BOXEDWINE_CONDITION waitingForSignalToEndCond;
    BOXEDWINE_CONDITION sigWaitCond;
    U64 sigWaitMask = 0;
    U64 foundWaitSignal = 0;

    U64 waitingForSignalToEndMaskToRestore = 0;
    U64 pendingSignals = 0;
    BOXEDWINE_MUTEX pendingSignalsMutex;
    std::shared_ptr<KThreadGlContext> getGlContextById(U32 id);
    void removeGlContextById(U32 id);
    void addGlContext(U32 id, void* context);
    void removeAllGlContexts();
    bool hasContextBeenMadeCurrentSinceCreation = false;

    BHashTable<U32, std::shared_ptr<KThreadGlContext>> glContext;
    BString name;
public:
    void* currentContext = nullptr;
    U32 glLastError = 0;
    bool log = false; // syscalls
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
    OpenGLVetexPointer glInterleavedArray;
    U32 marshalIndex = 0;

    inline static KThread* currentThread() {return runningThread;}
	inline static void setCurrentThread(KThread* thread) { runningThread = thread; }

    BOXEDWINE_CONDITION waitingCond = nullptr;    
    BOXEDWINE_CONDITION pollCond;
#ifdef BOXEDWINE_MULTI_THREADED
    BOXEDWINE_MUTEX waitingCondSync;
#else
    KListNode<KThread*> scheduledThreadNode;
    KListNode<KThread*> waitThreadNode;
        
    BoxedWineConditionTimer condTimer;
#endif

    U32 condStartWaitTime = 0;
private:
    std::shared_ptr<FsNode> threadNode; // in /proc/<pid>/task/<tid>
    std::shared_ptr<FsNode> commNode; // in /proc/<pid>/task/<tid>/comm

    std::vector< std::function<void(U32) > > callbacksOnExit;

    void clearFutexes();

    thread_local static KThread* runningThread;

    BOXEDWINE_CONDITION sleepCond;      

    struct user_desc tls[TLS_ENTRIES] = {};
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
