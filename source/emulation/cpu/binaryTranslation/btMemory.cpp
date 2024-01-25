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

    this->codeChunksByHostPage.clear();
    this->codeChunksByEmulationPage.clear();
    for (U32 i = 0; i < EXECUTABLE_SIZES; i++) {
        this->freeExecutableMemory[i].clear();
    }
}

// only called during code patching, if this become a performance problem maybe we could just it up
// like with soft_code_page
std::shared_ptr<BtCodeChunk> BtMemory::getCodeChunkContainingHostAddress(void* hostAddress) {
    U32 page = (U32)((size_t)hostAddress >> K_PAGE_SHIFT);
    if (this->codeChunksByHostPage.count(page)) {
        std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[page];
        for (auto& chunk : *chunks) {
            if (chunk->containsHostAddress(hostAddress)) {
                return chunk;
            }
        }
    }
    page--;
    // look to see if a chunk that starts in a previous page contains this address
    // chunks do not overlap, so find the first previous page with a chunk then
    // check on the chunks in that page
    while (page > 0) {
        if (this->codeChunksByHostPage.count(page)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[page];
            for (auto& chunk : *chunks) {
                if (chunk->containsHostAddress(hostAddress)) {
                    return chunk;
                }
            }
            break;
        }
        page--;
    }
    return NULL;
}

std::shared_ptr<BtCodeChunk> BtMemory::getCodeChunkContainingEip(U32 eip) {
    for (U32 i = 0; i < K_MAX_X86_OP_LEN; i++) {
        void* hostAddress = getExistingHostAddress(eip - i);
        if (hostAddress) {
            std::shared_ptr<BtCodeChunk> result = this->getCodeChunkContainingHostAddress(hostAddress);
            if (result->containsEip(eip)) {
                return result;
            }
            return NULL;
        }
    }
    return NULL;
}

// called when BtCodeChunk is being dealloc'd
void BtMemory::removeCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk) {
    U32 hostPage = (U32)(((size_t)chunk->getHostAddress()) >> K_PAGE_SHIFT);
    if (this->codeChunksByHostPage.count(hostPage)) {
        std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[hostPage];
        chunks->remove(chunk);
        if (chunks->size() == 0) {
            this->codeChunksByHostPage.erase(hostPage);
        }
    }

    U32 emulationPage = (chunk->getEip()) >> K_PAGE_SHIFT;
    if (this->codeChunksByEmulationPage.count(emulationPage)) {
        std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[emulationPage];
        chunks->remove(chunk);
        if (chunks->size() == 0) {
            this->codeChunksByEmulationPage.erase(emulationPage);
        }
    }        
}

// called when BtCodeChunk is being alloc'd
void BtMemory::addCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U32 hostPage = (U32)(((size_t)chunk->getHostAddress()) >> K_PAGE_SHIFT);
    U32 emulationPage = (chunk->getEip()) >> K_PAGE_SHIFT;
#ifdef _DEBUG
    U32 lastPage = (U32)(((size_t)((U8*)chunk->getHostAddress() + chunk->getHostAddressLen())) >> K_PAGE_SHIFT);
    for (U32 i = hostPage; i <= lastPage; i++) {
        if (this->codeChunksByHostPage.count(hostPage)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[hostPage];
            for (auto& otherChunk : *chunks) {
                if (otherChunk->containsHostAddress(chunk->getHostAddress())) {
                    kpanic("Memory::addCodeChunk chunks can not overlap");
                }
            }
        }
    }
#endif
    std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > hostChunks = this->codeChunksByHostPage[hostPage];
    if (!hostChunks) {
        hostChunks = std::make_shared< std::list<std::shared_ptr<BtCodeChunk>> >();
        this->codeChunksByHostPage[hostPage] = hostChunks;
    }
    hostChunks->push_back(chunk);

    std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[emulationPage];
    if (!chunks) {
        chunks = std::make_shared< std::list<std::shared_ptr<BtCodeChunk>> >();
        this->codeChunksByEmulationPage[emulationPage] = chunks;
    }
    chunks->push_back(chunk);

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

void BtMemory::clearCodePageFromCache(U32 page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
#ifdef BOXEDWINE_64BIT_MMU
    if (KSystem::useLargeAddressSpace) {
        KThread* thread = KThread::currentThread();
        std::shared_ptr<KProcess> process;

        if (thread) {
            process = thread->process;
        }
        if (process && this->isEipPageCommitted(page)) {
            U64 offset = (U64)(page << K_PAGE_SHIFT) * sizeof(void*);
            U64* address64 = (U64*)((U8*)this->eipToHostInstructionAddressSpaceMapping + offset);
            for (U32 j = 0; j < K_PAGE_SIZE; j++, address64++) {
                *address64 = (U64)process->reTranslateChunkAddressFromReg;
            }
        }
    } else 
#endif
    {
        void** table = this->eipToHostInstructionPages[page];
        if (table) {
            for (U32 i = 0; i < K_PAGE_SIZE; i++) {
                void* hostAddress = table[i];
                if (hostAddress) {
                    std::shared_ptr<BtCodeChunk> chunk = this->getCodeChunkContainingHostAddress(hostAddress);
                    if (chunk) {
                        i += chunk->getHostAddressLen();
                        chunk->release(this->memory);
                    }
                }
            }
            delete[] table;
            this->eipToHostInstructionPages[page] = NULL;
        }
    }    
}

void BtMemory::reserveNativeMemory() {
#ifdef BOXEDWINE_64BIT_MMU
    if (KSystem::useLargeAddressSpace) {
        this->eipToHostInstructionAddressSpaceMapping = Platform::reserveNativeMemory(true);
    }
#endif
}

void BtMemory::releaseNativeMemory() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->executableMemoryMutex);
    for (U32 i = 0; i < K_NUMBER_OF_PAGES; i++) {
        clearCodePageFromCache(i);
    }
    executableMemoryReleased();
    for (auto& p : this->allocatedExecutableMemory) {
        Platform::releaseNativeMemory(p.memory, p.size);
    }
    this->allocatedExecutableMemory.clear();
#ifdef BOXEDWINE_64BIT_MMU
    if (KSystem::useLargeAddressSpace) {
        Platform::releaseNativeMemory((char*)this->eipToHostInstructionAddressSpaceMapping, 0x800000000l);
        this->eipToHostInstructionAddressSpaceMapping = NULL;
    }
#endif

}

#endif