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

#ifndef __BT_CPU_H__
#define __BT_CPU_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR
class BtData;
class BtCodeChunk;

#define CPU_OFFSET_CURRENT_OP (U32)(offsetof(BtCPU, currentSingleOp))

#include "../normal/normalCPU.h"

class BtCPU : public NormalCPU {
public:
    BtCPU(KMemory* memory);

    // from CPU
    void run() override;
    void reset() override;

    virtual void* init() = 0; // called from run
    
    U64 exceptionAddress = 0;
    bool inException = false;
    bool exceptionReadAddress = false;
    U64 returnHostAddress = 0; // after returning from the signalHandler, this will contain the host address we should jump to
    int exceptionSigNo = 0;
    int exceptionSigCode = 0;
    U64 exceptionIp = 0;
    void* jmpAndTranslateIfNecessary = nullptr;
    void* returnToLoopAddress = nullptr;
    void* syncToHostAddress = nullptr;
    void* syncFromHostAddress = nullptr;
    void* doSingleOpAddress = nullptr;
    int exitToStartThreadLoop = 0; // this will be checked after a syscall, if set to 1 then then x64CPU.returnToLoopAddress will be called    
#ifdef BOXEDWINE_4K_PAGE_SIZE
    bool use4kMemCheck = true;
#endif
    std::vector<U32> pendingCodePages;

    void* translateChunk(U32 ip);
    virtual void translateData(BtData* data, BtData* firstPass = nullptr) = 0;
    virtual void link(BtData* data, void* hostAddress) = 0;
    void* translateEipInternal(U32 ip);
#ifdef __TEST
    virtual void postTestRun() = 0;
#endif

    U64 reTranslateChunk();
    U64 handleMissingCode(U32 page, U32 offset);        
    void* translateEip(U32 ip);    
    void makePendingCodePagesReadOnly();
    U64 startException(U64 address, bool readAddress);
    U64 handleFpuException(int code);
    U64 handleAccessException(DecodedOp* op);
    void startThread();
    void wakeThreadIfWaiting();    
    S32 preLinkCheck(BtData* data); // returns the index of the jump that failed
    void clearTranslatedChunk(DecodedOp* op);

    U32 largeAddressJumpInstruction = 0;
    U32 pageJumpInstruction = 0;
    U32 pageOffsetJumpInstruction = 0;
    DecodedOp*** opCache = nullptr;
protected:
    virtual BtData* getData1() = 0;
    virtual BtData* getData2() = 0;
};
#endif

#endif
