#include "boxedwine.h"
#include "btCodeChunk.h"
#include "btCpu.h"
#include "../../softmmu/kmemory_soft.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR

BtCodeChunk::BtCodeChunk(U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) {
    CPU* cpu = KThread::currentThread()->cpu;
    this->emulatedAddress = eip + cpu->seg[CS].address;
    this->emulatedLen = eipLen;
    this->hostAddress = getMemData(cpu->memory)->allocateExcutableMemory(hostInstructionBufferLen + 4, &this->hostMemoryLen); // +4 for a guard
    this->hostLen = hostInstructionBufferLen;
    Platform::writeCodeToMemory(this->hostAddress, this->hostLen, [this]() {
        memset(this->hostAddress, 0xce, this->hostLen);
        });
    
    Platform::writeCodeToMemory(this->hostAddress, this->hostLen, [=, this]() {
        memcpy(this->hostAddress, hostInstructionBuffer, hostInstructionBufferLen);
        });
}

void BtCodeChunk::makeLive(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex) {
    CPU* cpu = KThread::currentThread()->cpu;
    if (getEipLen()) { // might be custom code, not part of the emulation
        KMemoryData* mem = getMemData(cpu->memory);

        for (U32 i = 0; i < instructionCount; i++) {
            U32 eip = eipInstructionAddress[i];
            U8* host = this->hostAddress + hostInstructionIndex[i];
            U32 page = eip >> K_PAGE_SHIFT;
            U32 offset = eip & K_PAGE_MASK;
            U8** table = mem->eipToHostInstructionPages[page];
            if (!table) {
                table = new U8* [K_PAGE_SIZE];
                memset(table, 0, sizeof(U8*) * K_PAGE_SIZE);
                mem->eipToHostInstructionPages[page] = table;
            }
            if (table[offset]) {
                kpanic("BtCodeChunk::allocChunk eip already mapped");
            }
            table[offset] = host;
        }
        mem->addCodeChunk(this);
    }
    this->clearInstructionCache(this->hostAddress, this->hostLen);
}

void BtCodeChunk::detachFromHost(KMemory* memory) {
    KThread* thread = KThread::currentThread();
    KProcessPtr process;
    KMemoryData* mem = getMemData(thread->memory);

    if (thread) {
        process = thread->process;
    }

    for (U32 i = 0; i < emulatedLen; i++) {
        U32 eip = emulatedAddress + i;
        if (mem->eipToHostInstructionPages[eip >> K_PAGE_SHIFT]) { // might span multiple pages and the other pages are already deleted
            mem->eipToHostInstructionPages[eip >> K_PAGE_SHIFT][eip & K_PAGE_MASK] = nullptr;
        }
    }
    mem->removeCodeChunk(this);
}

void BtCodeChunk::release(KMemory* memory) {
    if (!locked) {
        this->detachFromHost(memory);
        this->internalDealloc();
    }
}

void BtCodeChunk::internalDealloc() {
    KMemoryData* mem = getMemData(KThread::currentThread()->memory);
    if (this->hostAddress) {
        mem->freeExcutableMemory(this->hostAddress, this->hostMemoryLen);
        this->hostAddress = nullptr;
    }    
    delete this;
}

void BtCodeChunk::releaseAndRetranslate() {
    // remove this chunk and its mappings from being used (since it is about to be replaced)
    BtCPU* cpu = (BtCPU*)KThread::currentThread()->cpu;

    KMemoryData* mem = getMemData(cpu->memory);
    mem->memory->removeCodeBlock(getEip(), getEipLen());    
    BtCodeChunk* chunk = cpu->translateChunk(this->emulatedAddress - cpu->seg[CS].address);
    cpu->makePendingCodePagesReadOnly();

    this->internalDealloc(); // don't call dealloc() because the new chunk occupies the memory cache and we don't want to mess with it
}

void BtCodeChunk::clearInstructionCache(U8* hostAddress, U32 len) {
    // x86 doesn't need to do anything
}

bool BtCodeChunk::containsEip(U32 eip, U32 len) {
    // do we begin in this chunk?
    if (this->containsEip(eip)) {
        return true;
    }
    // do we end in this chunk?
    if (this->containsEip(eip + len - 1)) {
        return true;
    }
    // we we span this chunk
    if (eip < this->emulatedAddress && eip + len>this->emulatedAddress + this->emulatedLen) {
        return true;
    }
    return false;
}

#endif
