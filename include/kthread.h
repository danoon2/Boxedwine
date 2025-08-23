/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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
    bool normalized = false;
};
typedef std::shared_ptr<OpenGLVetexPointer> OpenGLVetexPointerPtr;

class KProcess;
class Memory;
class Wnd;

class KThreadGlContext {
public:
    KThreadGlContext() = default;
    KThreadGlContext(void* context):context(context) {}
    void* context = nullptr;
    bool hasBeenMadeCurrent = false;
    bool sharing = false;
    std::shared_ptr<Wnd> wnd;
};

typedef std::shared_ptr<KThreadGlContext> KThreadGlContextPtr;

class KThread {
public:
    KThread(U32 id, const KProcessPtr& process);
    ~KThread();

    static void runOnMainThread(std::function<void()> callback);

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
    U32 set_robust_list(U32 head, U32 len);
    U32 get_robust_list(U32 pid, U32 head_ptr, U32 len_ptr);
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
    KProcessPtr process;
    KMemory* const memory;
    bool interrupted = false;
    U32 inSignal = 0;    
#ifdef BOXEDWINE_MULTI_THREADED
    bool exited = false;	
    bool startSignal = false;    
    U64 threadStartTime = 0;
#else
    U64 userTime = 0;    
#endif
    bool terminating = false;
    U32 clear_child_tid = 0;
    
    U64 getThreadUserTime();

    U64 kernelTime = 0;
    U32 inSysCall = 0;
    BOXEDWINE_CONDITION waitingForSignalToEndCond;
    BOXEDWINE_CONDITION sigWaitCond;
    U64 sigWaitMask = 0;
    U64 foundWaitSignal = 0;

    U64 waitingForSignalToEndMaskToRestore = 0;
    U64 pendingSignals = 0;
    BOXEDWINE_MUTEX pendingSignalsMutex;
    KThreadGlContextPtr getGlContextById(U32 id);
    void removeGlContextById(U32 id);
    KThreadGlContextPtr addGlContext(U32 id, void* context);
    void removeAllGlContexts();
    bool hasContextBeenMadeCurrentSinceCreation = false;

    BHashTable<U32, std::shared_ptr<KThreadGlContext>> glContext;
    BString name;

    std::vector<KPollData> pollData;
public:
    U32 currentContext = 0;
    U32 glLastError = 0;
    bool log = false; // syscalls
    OpenGLVetexPointer glVertextPointer; // 0 index
    BHashTable<U32, OpenGLVetexPointerPtr> glVertextPointersByIndex; // indexes greater than 0
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
    void internalCleanup();
    void exitRobustList();
    U32 handleFutexDeath(U32 uaddr, bool pi, bool pending_op);

    U32 robustList = 0;

    std::shared_ptr<FsNode> threadNode; // in /proc/<pid>/task/<tid>
    std::shared_ptr<FsNode> commNode; // in /proc/<pid>/task/<tid>/comm

    std::vector< std::function<void(U32) > > callbacksOnExit;

    void clearFutexes();

    thread_local static KThread* runningThread;

    BOXEDWINE_CONDITION sleepCond;      

    struct user_desc tls[TLS_ENTRIES] = {};
    BOXEDWINE_MUTEX tlsMutex;

    static BOXEDWINE_MUTEX_NR futexesMutex;
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
