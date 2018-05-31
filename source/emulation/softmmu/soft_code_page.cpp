#include "boxedwine.h"

#include "soft_code_page.h"

CodePage::CodePageEntry* CodePage::freeCodePageEntries;

CodePage::CodePageEntry* CodePage::allocCodePageEntry() {
    CodePageEntry* result;

    if (freeCodePageEntries) {
        result = freeCodePageEntries;
        freeCodePageEntries = result->next;        
    } else {
       result = new CodePageEntry();
    }	
    memset(result, 0, sizeof(CodePageEntry));
    return result;
}

void CodePage::freeCodePageEntry(CodePageEntry* entry) {	
	U32 offset = entry->offset >> CODE_ENTRIES_SHIFT;
	CodePageEntry** entries = entry->page->entries;
   
    if (entry->block)
        entry->block->dealloc(false);

	// remove any entries linked to this one from other pages
	if (entry->linkedPrev) {
		entry->linkedPrev->linkedNext = NULL;
		freeCodePageEntry(entry->linkedPrev);
		entry->linkedPrev = NULL;
	}
	if (entry->linkedNext) {
		entry->linkedNext->linkedPrev = NULL;
		freeCodePageEntry(entry->linkedNext);
		entry->linkedNext = NULL;
	}

	// remove this entry from this page's list
	if (entry->prev)
		entry->prev->next = entry->next;
	else
		entries[offset] = entry->next;
    if (entry->next)
        entry->next->prev = entry->prev;

	// add the entry to the free list
	entry->next = freeCodePageEntries;
	freeCodePageEntries = entry; 
}

CodePage* CodePage::alloc(U8* page, U32 address, U32 flags) {
    return new CodePage(page, address, flags);
}

CodePage::CodePage(U8* page, U32 address, U32 flags) : RWPage(page, address, flags, Code_Page) {
    memset(this->entries, 0, sizeof(this->entries));
}

CodePage::~CodePage() {
    int i;

    for (i=0;i<CODE_ENTRIES;i++) {
        CodePageEntry* entry = entries[i];
        while (entry) {
            CodePageEntry* next = entry->next;
                                            
            freeCodePageEntry(entry);			
            entry = next;
        }
    }
}

// :TODO: what if address+len is in the next page
CodePage::CodePageEntry* CodePage::findCode(U32 address, U32 len) {
    U32 offset = address & PAGE_MASK;

    for (U32 i=0;i<CODE_ENTRIES;i++) {
        CodePageEntry* entry = entries[i];
        while (entry) {
            if (((entry->offset <= offset && offset < (entry->offset + entry->len)) || (entry->offset <= (offset + len) && (offset + len) < (entry->offset + entry->len))) && !entry->linkedPrev)
                return entry;
            entry = entry->next;
        }
    }
    return 0;
}

void CodePage::removeBlockAt(U32 address, U32 len) {
    CodePageEntry* entry = findCode(address, len);

    while (entry) {
        if (entry->block==DecodedBlock::currentBlock) {
            KThread* thread = KThread::currentThread();

            if (address < thread->cpu->seg[CS].address + thread->cpu->eip.u32) {
                entry->block->dealloc(true);
                entry->block = NULL; // so that freeCodePageEntry won't dealloc it
            } else {
                kpanic("self modifying code tried to modify current running block");
            }
        }
        freeCodePageEntry(entry);
        entry = findCode(address, len);
    }
}

void CodePage::addCode(U32 eip, DecodedBlock* block, U32 len, CodePageEntry* link) {
    U32 offset = eip & PAGE_MASK;

    CodePageEntry** entry = &this->entries[offset >> CODE_ENTRIES_SHIFT];

    if (!*entry) {
        *entry = allocCodePageEntry();
        (*entry)->next = 0;
    } else {
        CodePageEntry* add = allocCodePageEntry();
        add->next = *entry;
		(*entry)->prev = add;
        *entry = add;
    }
    (*entry)->offset = offset;
    (*entry)->block = block;
	(*entry)->page = this;
	if (offset+len>PAGE_SIZE)
		(*entry)->len = PAGE_SIZE-offset;
	else
		(*entry)->len = len;
	if (link) {
		(*entry)->linkedPrev = link;
		link->linkedNext = (*entry);
        if (link->linkedPrev) {
            kpanic("Code block too big");
        }
	}
	if (offset + len > PAGE_SIZE) {
		U32 nextPage = (eip + 0xFFF) & 0xFFFFF000;
		this->addCode(nextPage, NULL, len - (nextPage - eip), *entry);
	}
}

void CodePage::addCode(U32 eip, DecodedBlock* op, U32 len) {
    this->addCode(eip, op, len, NULL);
}

DecodedBlock* CodePage::getCode(U32 eip) {
    U32 offset = eip & PAGE_MASK;
    CodePageEntry* entry = this->entries[offset >> CODE_ENTRIES_SHIFT];
    while (entry) {
        if (entry->offset == offset && !entry->linkedPrev)
            return entry->block;
        entry = entry->next;
    }
    return 0;
}

void CodePage::writeb(U32 address, U8 value) {    
    if (value!=this->readb(address)) {
        removeBlockAt(address, 1);
        RWPage::writeb(address, value);
    }
}

void CodePage::writew(U32 address, U16 value) {
    if (value!=this->readw(address)) {
        removeBlockAt(address, 2);
        RWPage::writew(address, value);
    }
}

void CodePage::writed(U32 address, U32 value) {
    if (value!=this->readd(address)) {
        removeBlockAt(address, 3);
        RWPage::writed(address, value);
    }
}

U8* CodePage::physicalAddress(U32 address) {
    return NULL;
}