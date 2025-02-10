#include "boxedwine.h"
#include "codePageData.h"
#include "../../util/ptrpool.h"
#include "kmemory_soft.h"

#define FIRST_INDEX_SIZE 0x400
#define SECOND_INDEX_SIZE 0x400
#define GET_FIRST_INDEX_FROM_PAGE(page) (page >> 10)
#define GET_SECOND_INDEX_FROM_PAGE(page) (page & 0x3ff);

#define DYNAMIC_MAX 3
static PtrPool<CodePageData::CodePageEntry> freeEntries;

CodePageData::CodePageData() {
    memset(this->entries, 0, sizeof(this->entries));
    entryCount = 0;
    writeCount = 0;
    writeCountsPerByte = nullptr;
}

CodePageData::~CodePageData() {
    for (int i = 0; i < CODE_ENTRIES; i++) {
        while (entries[i]) {
            removeBlock(entries[i]);
        }
    }
    if (writeCountsPerByte) {
        delete[] writeCountsPerByte;
    }
}

static void OPCALL emptyOp(CPU* cpu, DecodedOp* op) {
    cpu->nextBlock = nullptr;
    cpu->yield = true;
}

void CodePageData::removeEntry(CodePageEntry* entry, U32 offset) {
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    DecodedBlock* currentBlock = KThread::currentThread()->cpu->currentBlock;
    if (entry->block == currentBlock) {
        KThread* thread = KThread::currentThread();
        U32 ip;

        if (thread->cpu->isBig())
            ip = thread->cpu->seg[CS].address + thread->cpu->eip.u32;
        else
            ip = thread->cpu->seg[CS].address + thread->cpu->eip.u16;
        if (entry->block->address + offset >= ip) {
            // we don't have a pointer to the current op, so just set them all
            DecodedOp* op = currentBlock->op;
            while (op) {
                op->pfn = emptyOp; // This will cause the current block to return
                op = op->next;
            }
        }

        entry->block->dealloc(true);
        entry->block = nullptr; // so that freeEntries.put won't dealloc it
    }
#endif
    freeEntries.put(entry);
}

void CodePageData::removeBlock(CodePageEntry* entry, U32 offset) {
    CodePageEntry* next = entry->next;

    // take care of page links
    if (next) {
        entry->next = nullptr;
        next->prev = nullptr;
        next->page->removeBlock(next);
    }

    CodePageEntry* prev = entry->prev;
    if (prev) {
        entry->prev = nullptr;
        prev->next = nullptr;
        prev->page->removeBlock(prev);
    }
    U32 s = entry->start;
    U32 bucketIndexStart = entry->start >> CODE_ENTRIES_SHIFT;
    U32 bucketIndexStop = entry->stop >> CODE_ENTRIES_SHIFT;
    CodeBlock block = entry->block;

    for (U32 i = bucketIndexStart; i <= bucketIndexStop; i++) {
        CodePageEntry* e = entries[i];

        while (e) {
            if (e->block == block) {
                if (e->prevEntry) {
                    e->prevEntry->nextEntry = e->nextEntry;
                } else {
                    entries[i] = e->nextEntry;
                }
                if (e->nextEntry) {
                    e->nextEntry->prevEntry = e->prevEntry;
                }
                removeEntry(e, offset);
                e = entries[i];
            } else {
                e = e->nextEntry;
            }
        }
    }
}

void CodePageData::markOffsetDynamic(U32 offset, U32 len) {
    if (!writeCountsPerByte) {
        writeCountsPerByte = new U8[K_PAGE_SIZE];
        memset(writeCountsPerByte, 0, K_PAGE_SIZE);
    }
    for (U32 i = 0; i < len && i + offset < K_PAGE_MASK; i++) {
        writeCountsPerByte[offset + i] = DYNAMIC_MAX;
    }
}

bool CodePageData::isOffsetDynamic(U32 offset, U32 len) {
    if (!writeCountsPerByte) {
        return false;
    }
    for (U32 i = 0; i < len && i + offset < K_PAGE_MASK; i++) {
        if (writeCountsPerByte[offset + i] == DYNAMIC_MAX) {
            return true;
        }
    }
    return false;
}

CodePageData::CodePageEntry* CodePageData::findEntry(U32 start, U32 stop) {
    U32 bucketIndexStart = start >> CODE_ENTRIES_SHIFT;
    U32 bucketIndexStop = stop >> CODE_ENTRIES_SHIFT;

    if (bucketIndexStop >= CODE_ENTRIES) {
        bucketIndexStop = CODE_ENTRIES - 1;
    }
    for (U32 i = bucketIndexStart; i <= bucketIndexStop; i++) {
        CodePageEntry* e = entries[i];
        while (e) {
            if (std::max(start, e->start) <= std::min(stop, e->stop)) {
                return e;
            }
            e = e->nextEntry;
        }
    }
    return nullptr;
}

void CodePageData::addEntry(U32 start, U32 stop, CodePageEntry* entry) {
    U32 bucketIndexStart = start >> CODE_ENTRIES_SHIFT;
    U32 bucketIndexStop = stop >> CODE_ENTRIES_SHIFT;

    entry->start = start;
    entry->stop = stop;
    for (U32 i = bucketIndexStart; i <= bucketIndexStop; i++) {
        CodePageEntry* e = entries[i];

        if (i > bucketIndexStart) {
            CodePageEntry* nextBucket = freeEntries.get();
            nextBucket->block = entry->block;
            nextBucket->start = entry->start;
            nextBucket->stop = entry->stop;
            nextBucket->sharedBlock = entry->sharedBlock;
            nextBucket->page = this;
            entry = nextBucket;
        }

        if (e) {
            entry->nextEntry = e;
            e->prevEntry = entry;
        }
        entries[i] = entry;
    }
}

void CodePageData::removeBlockAt(U32 address, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    nolock_removeBlockAt(address, len);
}

void CodePageData::nolock_removeBlockAt(U32 address, U32 len) {
    U32 offset = address & K_PAGE_MASK;
    CodePageEntry* entry = findEntry(offset, offset + len - 1);

    if (entry) {
        if (writeCount < DYNAMIC_MAX) {
            writeCount++;
        } else if (!writeCountsPerByte) {
            writeCountsPerByte = new U8[K_PAGE_SIZE];
            memset(writeCountsPerByte, 0, K_PAGE_SIZE);
        } else {
            for (U32 i = 0; i < len && i + offset < K_PAGE_MASK; i++) {
                if (writeCountsPerByte[offset + i] < DYNAMIC_MAX) {
                    writeCountsPerByte[offset + i]++;
                }
            }
        }
        while (entry) {
            // get the duplicated entry in the earliest bucket of entries, since that one will have the prev/next pages set        
            CodePageEntry* e = entries[entry->start >> CODE_ENTRIES_SHIFT];
            while (e) {
                if (e->block == entry->block) {
                    entry = e;
                    break;
                }
                e = e->nextEntry;
            }
            removeBlock(entry, offset);
            entry = findEntry(offset, offset + len - 1);
        }
    }
}

void CodePageData::addCode(KMemory* memory, U32 eip, CodeBlockParam block, const InternalCodeBlock& sharedBlock, U32 len, CodePageEntry* link) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    CodePageEntry* entry = freeEntries.get();
    entry->block = block;
    entry->sharedBlock = sharedBlock;
    entry->page = this;
    U32 offset = eip & K_PAGE_MASK;
    U32 stop = 0;

    if (offset + len > K_PAGE_SIZE) {
        stop = K_PAGE_SIZE - 1;
        U32 nextAddress = ((eip >> K_PAGE_SHIFT) + 1) << K_PAGE_SHIFT;
        getMemData(memory)->getOrCreateCodePage(nextAddress);
        CodePageData* page = getMemData(memory)->codeCache.getCodePageData(nextAddress, true);
        page->addCode(memory, nextAddress, block, sharedBlock, len - (nextAddress - eip), entry);
    } else {
        stop = offset + len - 1;
    }

    if (link) {
        entry->prev = link;
        link->next = entry;
    }

    addEntry(offset, stop, entry);
    entryCount++;
}

void CodePageData::addCode(KMemory* memory, CodeBlockParam block) {
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    auto deleter = [](DecodedBlock* obj) {
        obj->dealloc(false);
        };
    InternalCodeBlock sharedBlock = std::shared_ptr<DecodedBlock>(block, deleter);
#else
    auto deleter = [](BtCodeChunk* entry) {
        KThread* thread = KThread::currentThread();
        if (thread) {
            KMemory* memory = KThread::currentThread()->memory;
            // check to prevent recursion
            if (getMemData(memory)->getExistingHostAddress(entry->getEip())) {
                entry->release(memory);
            }
        }
        };
    InternalCodeBlock sharedBlock = std::shared_ptr<BtCodeChunk>(block.get(), deleter);
#endif    
    this->addCode(memory, block->getEip(), block, sharedBlock, block->getEipLen(), nullptr);
}

#ifndef BOXEDWINE_BINARY_TRANSLATOR
CodeBlock CodePageData::getCode(U32 eip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U32 offset = eip & K_PAGE_MASK;
    U32 bucketIndex = offset >> CODE_ENTRIES_SHIFT;
    CodePageEntry* e = entries[bucketIndex];

    while (e) {
        if (e->block->address == eip) {
            return e->block;
        }
        e = e->nextEntry;
    }
    return nullptr;
}
#endif

CodeBlock CodePageData::findCode(U32 eip, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U32 offset = eip & K_PAGE_MASK;
    CodePageEntry* entry = findEntry(offset, offset + len - 1);
    if (entry) {
        return entry->block;
    }
    return nullptr;
}

CodeCache::CodeCache() {
    memset(pageData, 0, sizeof(pageData));
}

CodeCache::~CodeCache() {
    for (U32 i = 0; i < FIRST_INDEX_SIZE; i++) {
        if (pageData[i]) {
            for (U32 j = 0; j < SECOND_INDEX_SIZE; j++) {
                if (pageData[i][j]) {
                    delete pageData[i][j];
                }
            }
            delete[] pageData[i];
        }
    }
}

void CodeCache::markAddressDynamic(U32 address, U32 len) {
    U32 offset = address & K_PAGE_MASK;
    if (offset + len > K_PAGE_SIZE) {
        U32 todo = len - (K_PAGE_SIZE - offset);
        markAddressDynamic(address + len - todo, todo);
    }
    CodePageData* page = getCodePageData(address, false);
    if (page) {
        page->markOffsetDynamic(offset, len);
    }
}

#ifndef BOXEDWINE_BINARY_TRANSLATOR
CodeBlock CodeCache::getCode(U32 eip) {
    CodePageData* page = getCodePageData(eip, false);
    if (page) {
        return page->getCode(eip);
    }
    return nullptr;
}
#endif

void CodeCache::addCode(KMemory* memory, CodeBlockParam block) {
    CodePageData* page = getCodePageData(block->getEip(), true);
    page->addCode(memory, block);
}

void CodeCache::removeBlockAt(U32 address, U32 len) {
    CodePageData* page = getCodePageData(address, false);
    if (page) {
        page->removeBlockAt(address, len);
    }
}

CodeBlock CodeCache::findCode(U32 eip, U32 len) {    
    CodePageData* page = getCodePageData(eip, false);
    if (page) {
        return page->findCode(eip, len);
    }
    return nullptr;
}

bool CodeCache::isAddressDynamic(U32 address, U32 len) {
    bool result = false;

    U32 offset = address & K_PAGE_MASK;
    if (offset + len > K_PAGE_SIZE) {
        U32 todo = len - (K_PAGE_SIZE - offset);
        result = isAddressDynamic(address + len - todo, todo);
    }
    CodePageData* page = getCodePageData(address, false);
    if (page) {
        result |= page->isOffsetDynamic(offset, len);
    }
    return result;
}

CodePageData* CodeCache::getCodePageData(U32 address, bool create) {
    U32 page = address >> K_PAGE_SHIFT;
    U32 firstIndex = GET_FIRST_INDEX_FROM_PAGE(page);
    U32 secondIndex = GET_SECOND_INDEX_FROM_PAGE(page);
    CodePageData** first = pageData[firstIndex];
    CodePageData* result = nullptr;
    if (first) {
        result = first[secondIndex];
    }
    if (!result && create) {
        BOXEDWINE_CRITICAL_SECTION;
        if (!first) {
            first = pageData[firstIndex];
        }
        if (first) {
            result = first[secondIndex];
        }
        if (!first) {
            pageData[firstIndex] = new CodePageData * [SECOND_INDEX_SIZE];
            memset(pageData[firstIndex], 0, sizeof(CodePageData*) * SECOND_INDEX_SIZE);
        }
        if (!result) {
            result = new CodePageData();
            pageData[firstIndex][secondIndex] = result;
        }
    }
    return result;
}
