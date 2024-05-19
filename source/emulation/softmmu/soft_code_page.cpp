#include "boxedwine.h"

#include "soft_code_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"
#include "../cpu/normal/normalCPU.h"
#include "../../util/ptrpool.h"

#define DYNAMIC_MAX 3
static PtrPool<CodePage::CodePageEntry> freeEntries;
    
CodePage* CodePage::alloc(const KRamPtr& page, U32 address) {
    return new CodePage(page, address);
}

CodePage::CodePage(const KRamPtr& page, U32 address) : RWPage(page, address) {
    memset(this->entries, 0, sizeof(this->entries));
    entryCount = 0;
    writeCount = 0;
    writeCountsPerByte = nullptr;
}

CodePage::~CodePage() {
    for (int i=0;i<CODE_ENTRIES;i++) {
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

void CodePage::removeEntry(CodePageEntry* entry, U32 offset) {
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    if (entry->block == DecodedBlock::currentBlock) {
        KThread* thread = KThread::currentThread();
        U32 ip;

        if (thread->cpu->isBig())
            ip = thread->cpu->seg[CS].address + thread->cpu->eip.u32;
        else
            ip = thread->cpu->seg[CS].address + thread->cpu->eip.u16;
        if (entry->block->address + offset >= ip) {
            // we don't have a pointer to the current op, so just set them all
            DecodedOp* op = DecodedBlock::currentBlock->op;
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

void CodePage::removeBlock(CodePageEntry* entry, U32 offset) {
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

void CodePage::markOffsetDynamic(U32 offset, U32 len) {
    if (!writeCountsPerByte) {
        writeCountsPerByte = new U8[K_PAGE_SIZE];
        memset(writeCountsPerByte, 0, K_PAGE_SIZE);
    }
    for (U32 i = 0; i < len && i + offset < K_PAGE_MASK; i++) {
        writeCountsPerByte[offset + i] = DYNAMIC_MAX;
    }
}

bool CodePage::isOffsetDynamic(U32 offset, U32 len) {
    if (!writeCountsPerByte) {
        return false;
    }
    for (U32 i = 0; i < len && i + offset < K_PAGE_MASK; i++) {
        if (writeCountsPerByte[offset+i] == DYNAMIC_MAX) {
            return true;
        }
    }
    return false;
}

CodePage::CodePageEntry* CodePage::findEntry(U32 start, U32 stop) {
    U32 bucketIndexStart = start >> CODE_ENTRIES_SHIFT;
    U32 bucketIndexStop = stop >> CODE_ENTRIES_SHIFT;

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

void CodePage::addEntry(U32 start, U32 stop, CodePageEntry* entry) {
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

void CodePage::removeBlockAt(U32 address, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
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

void CodePage::addCode(U32 eip, CodeBlock& block, const InternalCodeBlock& sharedBlock, U32 len, CodePageEntry* link) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    CodePageEntry* entry = freeEntries.get();
    entry->block = block;
    entry->sharedBlock = sharedBlock;
    entry->page = this;
    U32 offset = eip & K_PAGE_MASK;
    U32 stop = 0;

    if (offset + len > K_PAGE_SIZE) {
        stop = K_PAGE_SIZE - 1;
        U32 nextPage = ((eip >> K_PAGE_SHIFT) + 1) << K_PAGE_SHIFT;
        getMemData(KThread::currentThread()->memory)->getOrCreateCodePage(nextPage)->addCode(nextPage, block, sharedBlock, len - (nextPage - eip), entry);
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

void CodePage::addCode(U32 eip, CodeBlock block, U32 len) {
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    auto deleter = [](DecodedBlock* obj) {
        obj->dealloc(false);
        };
    InternalCodeBlock sharedBlock = std::shared_ptr<DecodedBlock>(block, deleter);
#else
    auto deleter = [](BtCodeChunk* entry) {
        KMemory* memory = KThread::currentThread()->memory;
        // check to prevent recursion
        if (getMemData(memory)->getExistingHostAddress(entry->getEip())) {
            entry->release(memory);
        } else {
            int ii = 0;
        }
        };
    InternalCodeBlock sharedBlock = std::shared_ptr<BtCodeChunk>(block.get(), deleter);    
#endif    
    this->addCode(eip, block, sharedBlock, len, nullptr);
}

#ifndef BOXEDWINE_BINARY_TRANSLATOR
CodeBlock CodePage::getCode(U32 eip) {
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

CodeBlock CodePage::findCode(U32 eip, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U32 offset = eip & K_PAGE_MASK;
    CodePageEntry* entry = findEntry(offset, offset+len-1);
    if (entry) {
        return entry->block;
    }
    return nullptr;
}

void CodePage::copyOnWrite() {
    if (!KThread::currentThread()->memory->mapShared(address >> K_PAGE_SHIFT) && page.use_count() > 1) {
        KRamPtr ram = ramPageAlloc();
        ::memcpy(ram.get(), page.get(), K_PAGE_SIZE);
        page = ram;
        getMemData(KThread::currentThread()->memory)->setPage(address >> K_PAGE_SHIFT, this);
    }
}

void CodePage::writeb(U32 address, U8 value) {    
    if (value!=this->readb(address)) {
        removeBlockAt(address, 1);
        copyOnWrite();
        RWPage::writeb(address, value);
    }
}

void CodePage::writew(U32 address, U16 value) {
    if (value!=this->readw(address)) {
        if (readb(address) != (U8)value) {
            removeBlockAt(address, 1);
        }
        if (readb(address+1) != (U8)(value >> 8)) {
            removeBlockAt(address+1, 1);
        }
        copyOnWrite();
        RWPage::writew(address, value);
    }
}

void CodePage::writed(U32 address, U32 value) {
    if (value!=this->readd(address)) {
        if (readb(address) != (U8)value) {
            removeBlockAt(address, 1);
        }
        if (readb(address + 1) != (U8)(value >> 8)) {
            removeBlockAt(address + 1, 1);
        }
        if (readb(address + 2) != (U8)(value >> 16)) {
            removeBlockAt(address + 2, 1);
        }
        if (readb(address + 3) != (U8)(value >> 24)) {
            removeBlockAt(address + 3, 1);
        }
        copyOnWrite();
        RWPage::writed(address, value);
    }
}

U8* CodePage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    if (memory->canRead(address >> K_PAGE_SHIFT)) {
        return this->ram;
    }
    return nullptr;
}

U8* CodePage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    if (memory->canWrite(address >> K_PAGE_SHIFT) && makeReady) {
        removeBlockAt(address, len);
        return this->ram;
    }
    return nullptr;
}
