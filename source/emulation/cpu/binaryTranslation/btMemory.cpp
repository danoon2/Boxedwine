#include "boxedwine.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "btMemory.h"
#include "btCodeChunk.h"

BtMemory::BtMemory(KMemory* memory) : memory(memory) {
    this->eipToHostInstructionPages = new U8** [K_NUMBER_OF_PAGES];
    memset(this->eipToHostInstructionPages, 0, K_NUMBER_OF_PAGES * sizeof(void**));
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
U8* BtMemory::getExistingHostAddress(U32 eip) {
    U32 page = eip >> K_PAGE_SHIFT;
    U32 offset = eip & K_PAGE_MASK;
    if (this->eipToHostInstructionPages[page])
        return this->eipToHostInstructionPages[page][offset];
    return nullptr;
}

bool BtMemory::isAddressExecutable(U8* address) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(executableMemoryMutex);
    for (auto& p : this->allocatedExecutableMemory) {
        if (address >= p.memory && address < p.memory + p.size) {
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

U8* BtMemory::allocateExcutableMemory(U32 requestedSize, U32* allocatedSize) {
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
        U8* result = this->freeExecutableMemory[index].front();
        this->freeExecutableMemory[index].pop_front();
        return result;
    }
    U32 count = (size + 65535) / 65536;
    U8* result = Platform::alloc64kBlock(count, true);
    this->allocatedExecutableMemory.push_back(BtMemory::AllocatedMemory(result, count * 64 * 1024));
    count = 65536 / size;
    for (U32 i = 1; i < count; i++) {
        this->freeExecutableMemory[index].push_back(((U8*)result) + size * i);
    }
    return result;
}

void BtMemory::freeExcutableMemory(U8* hostMemory, U32 actualSize) {
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

#endif