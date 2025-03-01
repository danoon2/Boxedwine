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

#ifndef __BT_CODE_CHUNK_H__
#define __BT_CODE_CHUNK_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

class BtCodeChunkLink {
public:
    BtCodeChunkLink(U8* fromHostOffset, U32 toEip, U32 fromEip, U8* toHostInstruction, bool direct) : fromHostOffset(fromHostOffset), toEip(toEip), fromEip(fromEip), toHostInstruction(toHostInstruction), direct(direct) {}
    // will point to an address in the middle of the instruction
    U8* fromHostOffset;

    // will point to the start of the instruction
    U32 toEip;
    U32 fromEip;
    U8* toHostInstruction;
    bool direct;
};

class BtCPU;
class DecodedBlock;

class BtCodeChunk : public std::enable_shared_from_this<BtCodeChunk> {
public:
    BtCodeChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic);

    virtual bool retranslateSingleInstruction(BtCPU* cpu, U8* address) = 0;

    void release(KMemory* memory);
    void releaseAndRetranslate();
    void invalidateStartingAt(U32 eipAddress);
    void makeLive();

    U32 getEipThatContainsHostAddress(U8* hostAddress, U8** startOfHostInstruction, U32* index);

    void* getHostAddress() { return this->hostAddress; }
    U32 getHostAddressLen() { return this->hostLen; }

    bool containsHostAddress(U8* hostAddress) { return hostAddress >= this->hostAddress && hostAddress < this->hostAddress + this->hostLen; }
    bool containsEip(U32 eip) { return eip >= this->emulatedAddress && eip < this->emulatedAddress + this->emulatedLen; }
    bool containsEip(U32 eip, U32 len);

    std::shared_ptr<BtCodeChunkLink> addLinkFrom(std::shared_ptr<BtCodeChunk>& from, U32 toEip, U8* toHostInstruction, U8* fromHostOffset, bool direct);

    U8* getHostFromEip(U32 eip) { U8* result = nullptr; if (this->getStartOfInstructionByEip(eip, &result, nullptr) == eip) { return result; } else { return nullptr; } }
    U32 getEip() { return emulatedAddress; }
    U32 getEipLen() { return emulatedLen; }
    U32 getStartOfInstructionByEip(U32 eip, U8** hostAddress, U32* index);
    
    DecodedBlock* block;
#ifdef BOXEDWINE_4K_PAGE_SIZE
    U32 startTimeForExceptionTracking = 0;
    U32 exceptionCount = 0;
    bool retranslatedForException = false;
#endif
protected:
    void detachFromHost(KMemory* memory);
    void internalDealloc();
    virtual void clearInstructionCache(U8* hostAddress, U32 len);

    U32 emulatedAddress;
    U32 emulatedLen;
    U8* emulatedInstructionLen; // must be 15 or less per op

    U8* hostAddress;
    U32 hostAddressSize;
    U32 hostLen;
    U32* hostInstructionLen;

    U32 instructionCount;

    std::list<std::shared_ptr<BtCodeChunkLink>> linksTo;
    std::list<std::shared_ptr<BtCodeChunkLink>> linksFrom;
};

#endif

#endif