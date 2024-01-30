#include "boxedwine.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "btMemory.h"
#include "btCodeChunk.h"

BtMemory::BtMemory(KMemory* memory) : memory(memory) {
#ifdef BOXEDWINE_64BIT_MMU
    if (!KSystem::useLargeAddressSpace) {
        this->eipToHostInstructionPages = new void** [K_NUMBER_OF_PAGES];
        memset(this->eipToHostInstructionPages, 0, K_NUMBER_OF_PAGES * sizeof(void**));
    } else {
        this->eipToHostInstructionPages = NULL;
    }
    this->eipToHostInstructionAddressSpaceMapping = NULL;
#else
    this->eipToHostInstructionPages = new void** [K_NUMBER_OF_PAGES];
    memset(this->eipToHostInstructionPages, 0, K_NUMBER_OF_PAGES * sizeof(void**));
#endif    
    memset(this->committedEipPages, 0, sizeof(this->committedEipPages));
}

BtMemory::~BtMemory() {    
    if (this->eipToHostInstructionPages) {
        for (U32 i = 0; i < K_NUMBER_OF_PAGES; i++) {
            if (this->eipToHostInstructionPages[i]) {
                delete[] this->eipToHostInstructionPages[i];
            }
        }
        delete[] this->eipToHostInstructionPages;
    }
}

// call during code translation, this needs to be fast
void* BtMemory::getExistingHostAddress(U32 eip) {
#ifdef BOXEDWINE_64BIT_MMU
    if (KSystem::useLargeAddressSpace) {
        if (!this->isEipPageCommitted(eip >> K_PAGE_SHIFT)) {
            return NULL;
        }
        void* result = (void*)(*(U64*)(((U8*)this->eipToHostInstructionAddressSpaceMapping) + ((U64)eip * sizeof(void*))));
        if (result == KThread::currentThread()->process->reTranslateChunkAddressFromReg) {
            return NULL;
        }
        return result;
    } else 
#endif
    {
        U32 page = eip >> K_PAGE_SHIFT;
        U32 offset = eip & K_PAGE_MASK;
        if (this->eipToHostInstructionPages[page])
            return this->eipToHostInstructionPages[page][offset];
        return NULL;
    }
}

bool BtMemory::isAddressExecutable(void* address) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(executableMemoryMutex);
    for (auto& p : this->allocatedExecutableMemory) {
        if (address >= p.memory && address < (U8*)p.memory + p.size) {
            return true;
        }
    }
    return false;
}

int powerOf2(U32 requestedSize, U32& size) {
    size = 2;
    U32 powerOf2Size = 1;
    while (size < requestedSize) {
        size <<= 1;
        powerOf2Size++;
    }
    return powerOf2Size;
}

void* BtMemory::allocateExcutableMemory(U32 requestedSize, U32* allocatedSize) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(executableMemoryMutex);
    U32 size = 0;
    U32 powerOf2Size = powerOf2(requestedSize, size);

    if (powerOf2Size < EXECUTABLE_MIN_SIZE_POWER) {
        powerOf2Size = EXECUTABLE_MIN_SIZE_POWER;
        size = 1 << EXECUTABLE_MIN_SIZE_POWER;
    } else if (powerOf2Size > EXECUTABLE_MAX_SIZE_POWER) {
        kpanic("x64 code chunk was larger than 64k");
    }
    if (allocatedSize) {
        *allocatedSize = size;
    }
    U32 index = powerOf2Size - EXECUTABLE_MIN_SIZE_POWER;
    if (!this->freeExecutableMemory[index].empty()) {
        void* result = this->freeExecutableMemory[index].front();
        this->freeExecutableMemory[index].pop_front();
        return result;
    }
    U32 count = (size + 65535) / 65536;
    void* result = Platform::alloc64kBlock(count, true);
    this->allocatedExecutableMemory.push_back(BtMemory::AllocatedMemory(result, count * 64 * 1024));
    count = 65536 / size;
    for (U32 i = 1; i < count; i++) {
        this->freeExecutableMemory[index].push_back(((U8*)result) + size * i);
    }
    return result;
}

void BtMemory::freeExcutableMemory(void* hostMemory, U32 actualSize) {
    Platform::writeCodeToMemory(hostMemory, actualSize, [hostMemory, actualSize] {
        memset(hostMemory, 0xcd, actualSize);
        });

    //U32 size = 0;
    //U32 powerOf2Size = powerOf2(actualSize, size);
    //U32 index = powerOf2Size - EXECUTABLE_MIN_SIZE_POWER;
    //this->freeExecutableMemory[index].push_back(hostMemory);

    // :TODO: when this recycled, make sure we delay the recycling in case another thread is also waiting in seh_filter 
    // for its turn to jump to this chunk at the same time another thread retranslated it
    //
    // I saw this in the Real Deal installer
}

void BtMemory::executableMemoryReleased() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(executableMemoryMutex);

    for (U32 i = 0; i < EXECUTABLE_SIZES; i++) {
        this->freeExecutableMemory[i].clear();
    }
}

std::shared_ptr<BtCodeChunk> BtMemory::getCodeChunkContainingEip(U32 eip) {
    return memory->findCodeBlockContaining(eip, 1);
}

// called when BtCodeChunk is being dealloc'd
void BtMemory::removeCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk) {
    
}

// called when BtCodeChunk is being alloc'd
void BtMemory::addCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    memory->addCodeBlock(chunk->getEip(), chunk);
}

bool BtMemory::isEipPageCommitted(U32 page) {
    return this->committedEipPages[page];
}

#ifdef BOXEDWINE_64BIT_MMU
void BtMemory::setEipForHostMapping(U32 eip, void* host) {
    U32 page = eip >> K_PAGE_SHIFT;
    if (!this->isEipPageCommitted(page)) {
        commitHostAddressSpaceMapping(page, 1, (U64)KThread::currentThread()->process->reTranslateChunkAddressFromReg);
    }
    U64* address = (U64*)(((U8*)this->eipToHostInstructionAddressSpaceMapping) + ((U64)eip) * sizeof(void*));
    *address = (U64)host;
}

void BtMemory::commitHostAddressSpaceMapping(U32 page, U32 pageCount, U64 defaultValue) {
    for (U32 i = 0; i < pageCount; i++) {
        if (!this->isEipPageCommitted(page + i)) {
            U8* address = (U8*)this->eipToHostInstructionAddressSpaceMapping + ((U64)(page + i)) * K_PAGE_SIZE * sizeof(void*);
            // won't worry about granularity size (Platform::getPageAllocationGranularity()) since Platform::commitNativeMemory doesn't require a multiple of it
            Platform::commitNativeMemory(address, (sizeof(void*) << K_PAGE_SHIFT));
            U64* address64 = (U64*)address;
            for (U32 j = 0; j < K_PAGE_SIZE; j++, address64++) {
                *address64 = defaultValue;
            }
            this->setEipPageCommitted(page + i);
        }
    }
}
#endif

#endif