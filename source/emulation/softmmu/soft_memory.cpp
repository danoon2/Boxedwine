/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU
#include "kscheduler.h"
#include "soft_file_map.h"
#include "ksignal.h"
#include "soft_memory.h"
#include "soft_invalid_page.h"
#include "soft_ondemand_page.h"
#include "soft_ro_page.h"
#include "soft_rw_page.h"
#include "soft_wo_page.h"
#include "soft_no_page.h"
#include "soft_copy_on_write_page.h"
#include "soft_code_page.h"
#include "soft_native_page.h"
#include "soft_ram.h"
#include "devfb.h"

#include <string.h>
#include <setjmp.h>

//#undef LOG_OPS

Page** Memory::currentMMU;
U8** Memory::currentMMUReadPtr;
U8** Memory::currentMMUWritePtr;

void Memory::log_pf(KThread* thread, U32 address) {
    U32 start = 0;
    U32 i;
    CPU* cpu = thread->cpu;
    std::shared_ptr<KProcess> process = thread->process;

    std::string name = process->getModuleName(cpu->seg[CS].address+cpu->eip.u32);
    klog("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X", cpu->seg[CS].address + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, name.c_str(), process->getModuleEip(cpu->seg[CS].address+cpu->eip.u32));

    klog("Page Fault at %.8X", address);
    klog("Valid address ranges:");
    for (i=0;i<K_NUMBER_OF_PAGES;i++) {
        if (!start) {
            if (process->memory->getPage(i) != invalidPage) {
                start = i;
            }
        } else {
            if (process->memory->getPage(i) == invalidPage) {
                klog("    %.8X - %.8X", start*K_PAGE_SIZE, i*K_PAGE_SIZE);
                start = 0;
            }
        }
    }
    klog("Mapped Files:");
    process->printMappedFiles();
    cpu->walkStack(cpu->eip.u32, EBP, 2);
    kpanic("pf");
}

U8* Memory::callbackRam;
U32 Memory::callbackRamPos;

void Memory::addCallback(OpCallback func) {
    U64 funcAddress = (U64)func;
    U8* address = callbackRam+callbackRamPos;
    
    *address=0xFE;
    address++;
    *address=0x38;
    address++;
    *address=(U8)funcAddress;
    address++;
    *address=(U8)(funcAddress >> 8);
    address++;
    *address=(U8)(funcAddress >> 16);
    address++;
    *address=(U8)(funcAddress >> 24);
    callbackRamPos+=6;
    if (sizeof(func)==8) {
        address++;
        *address=(U8)(funcAddress >> 32);
        address++;
        *address=(U8)(funcAddress >> 40);
        address++;
        *address=(U8)(funcAddress >> 48);
        address++;
        *address=(U8)(funcAddress >> 56);
        callbackRamPos+=4;
    }
}

Memory::Memory() : nativeAddressStart(0) {
    for (int i=0;i<K_NUMBER_OF_PAGES;i++) {
        this->mmu[i] = invalidPage;
        this->mmuReadPtr[i] = NULL;
        this->mmuWritePtr[i] = NULL;
    }

    if (!callbackRam) {
        callbackRam = ramPageAlloc();
        addCallback(onExitSignal);
    }

    this->setPage(CALL_BACK_ADDRESS>>K_PAGE_SHIFT, NativePage::alloc(callbackRam, CALL_BACK_ADDRESS, PAGE_READ|PAGE_EXEC));

#ifdef BOXEDWINE_DYNAMIC
    this->dynamicExecutableMemoryPos = 0;
    this->dynamicExecutableMemoryLen = 0;
#endif

    this->refCount = 1;
}

Memory::~Memory() {
    for (int i=0;i<K_NUMBER_OF_PAGES;i++) {
        this->mmu[i]->close();
    }
#ifdef BOXEDWINE_DYNAMIC
    for (U32 i=0;i<this->dynamicExecutableMemory.size();i++) {
        //freeExecutable64kBlock(this->dynamicExecutableMemory[i]);
    }
#endif
}

void Memory::reset() {
    for (int i=0;i<K_NUMBER_OF_PAGES;i++) {
        this->setPage(i, invalidPage);
    }
    this->setPage(CALL_BACK_ADDRESS>>K_PAGE_SHIFT, NativePage::alloc(callbackRam, CALL_BACK_ADDRESS, PAGE_READ|PAGE_EXEC));
}

void Memory::reset(U32 page, U32 pageCount) {
    for (U32 i=page;i<page+pageCount;i++) {
        this->setPage(i, invalidPage);
    }
}

void Memory::clone(Memory* from) {
    for (int i=0;i<0x100000;i++) {
        Page* page = from->getPage(i);

        if (page->type == Page::Type::On_Demand_Page) {
            if (page->mapShared()) {
                OnDemandPage*  p = (OnDemandPage*)from->getPage(i);
                p->ondemmand(i<<K_PAGE_SHIFT);
                // fall through
            } else {                
                this->setPage(i, OnDemandPage::alloc(page->flags));
                continue;
            }
        }
        if (page->type == Page::Type::File_Page) {
            FilePage* p = (FilePage*)from->getPage(i);
            if (page->mapShared()) {                
                p->ondemmandFile(i<<K_PAGE_SHIFT);
                // fall through
            } else {
                this->setPage(i, FilePage::alloc(p->mapped, p->index, p->flags));
                continue;
            }
        }
        page = from->getPage(i); // above code could have changed this
        if (page->type == Page::Type::RO_Page || page->type == Page::Type::RW_Page || page->type == Page::Type::WO_Page || page->type == Page::Type::NO_Page || page->type == Page::Type::Code_Page) {
            RWPage* p = (RWPage*)from->getPage(i);
            if (!page->mapShared()) {
                if (page->type == Page::Type::WO_Page) {
                    U8* ram = ramPageAlloc();
                    memcpy(ram, p->page, K_PAGE_SIZE);
                    this->setPage(i, WOPage::alloc(ram, p->address, p->flags));
                } else if (page->type == Page::Type::NO_Page) {
                    U8* ram = ramPageAlloc();
                    memcpy(ram, p->page, K_PAGE_SIZE);
                    this->setPage(i, NOPage::alloc(ram, p->address, p->flags));
                } else {
                    this->setPage(i, CopyOnWritePage::alloc(p->page, p->address, p->flags));
                    from->setPage(i, CopyOnWritePage::alloc(p->page, p->address, p->flags));
                }                
            } else {
                if (page->type == Page::Type::RO_Page) {
                    this->setPage(i, ROPage::alloc(p->page, p->address, p->flags));
                } else if (page->type == Page::Type::RW_Page) {
                    this->setPage(i, RWPage::alloc(p->page, p->address, p->flags));
                } else if (page->type == Page::Type::WO_Page) {
                    this->setPage(i, WOPage::alloc(p->page, p->address, p->flags));
                } else if (page->type == Page::Type::NO_Page) {
                    this->setPage(i, NOPage::alloc(p->page, p->address, p->flags));
                } else if (page->type == Page::Type::Code_Page) {
                    this->setPage(i, CodePage::alloc(p->page, p->address, p->flags));
                }
            }
        } else if (page->type == Page::Type::Copy_On_Write_Page) {
            CopyOnWritePage* p = (CopyOnWritePage*)from->getPage(i);
            this->setPage(i, CopyOnWritePage::alloc(p->page, p->address, p->flags));
        } else if (page->type == Page::Type::Native_Page) {
            NativePage* p = (NativePage*)from->getPage(i);
            this->setPage(i, NativePage::alloc(p->nativeAddress, p->address, p->flags));
        } else if (page->type == Page::Type::Invalid_Page) { 
            this->setPage(i, from->getPage(i));
        } 
#ifdef BOXEDWINE_EXPERIMENTAL_FRAME_BUFFER
        else if (page->type == Page::Type::Frame_Buffer) { 
            this->setPage(i, allocFBPage(from->getPageFlags(i)));
        } 
#endif
        else {
            kpanic("unhandled case when cloning memory: page type = %d", page->type);
        }
    }
}

void zeroMemory(U32 address, int len) {
    for (int i=0;i<len;i++) {
        writeb(address, 0);
        address++;
    }
}

void readMemory(U8* data, U32 address, int len) {
    for (int i=0;i<len;i++) {
        *data=readb(address);
        address++;
        data++;
    }
}

void writeMemory(U32 address, U8* data, int len) {
    for (int i=0;i<len;i++) {
        writeb(address, *data);
        address++;
        data++;
    }
}

void Memory::allocPages(U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile) {

    if (mappedFile) {
        U32 filePage = (U32)(offset>>K_PAGE_SHIFT);

        if (offset & K_PAGE_MASK) {
            kpanic("mmap: wasn't expecting the offset to be in the middle of a page");
        }    

        for (U32 i=0;i<pageCount;i++) {
            this->setPage(page+i, FilePage::alloc(mappedFile, filePage++, permissions));
        }
    } else {
        for (U32 i=0;i<pageCount;i++) {
            this->setPage(page+i, OnDemandPage::alloc(permissions));
        }
    }    
}

void Memory::protectPage(U32 i, U32 permissions) {
    Page* page = this->getPage(i);

    U32 flags = page->flags;
    flags&=~PAGE_PERMISSION_MASK;
    flags|=(permissions & PAGE_PERMISSION_MASK);

    if (page->type == Page::Type::Invalid_Page) {
        this->setPage(i, OnDemandPage::alloc(flags));
    } else if (page->type == Page::Type::RO_Page || page->type == Page::Type::RW_Page || page->type == Page::Type::WO_Page || page->type == Page::Type::NO_Page) {
        RWPage* p = (RWPage*)page;

        if ((permissions & PAGE_READ) && (permissions & PAGE_WRITE)) {
            if (page->type != Page::Type::RW_Page) {
                this->setPage(i, RWPage::alloc(p->page, p->address, flags));
            }
        } else if (permissions & PAGE_WRITE) {
            if (page->type != Page::Type::WO_Page) {
                this->setPage(i, WOPage::alloc(p->page, p->address, flags));
            }
        } else if (permissions & PAGE_READ) {
            if (page->type != Page::Type::RO_Page) {
                this->setPage(i, ROPage::alloc(p->page, p->address, flags));
            }
        } else {
            if (page->type != Page::Type::NO_Page) {
                this->setPage(i, NOPage::alloc(p->page, p->address, flags));
            }
        }
    } else if (page->type == Page::Type::Copy_On_Write_Page) {
        if ((permissions & PAGE_READ) && (permissions & PAGE_WRITE)) {
            page->flags = flags;
        } else if (permissions & PAGE_WRITE) {
            CopyOnWritePage* p = (CopyOnWritePage*)this->getPage(i);
            U8* ram = ramPageAlloc();
            memcpy(ram, p->page, K_PAGE_SIZE);
            this->setPage(i, WOPage::alloc(ram, p->address, flags));
        } else if (permissions & PAGE_READ) {
            page->flags = flags;
        } else {
            CopyOnWritePage* p = (CopyOnWritePage*)this->getPage(i);
            U8* ram = ramPageAlloc();
            memcpy(ram, p->page, K_PAGE_SIZE);
            this->setPage(i, NOPage::alloc(ram, p->address, flags));
        }
    } else if (page->type == Page::Type::File_Page || page->type == Page::Type::On_Demand_Page) {
        page->flags = flags;
    } else if (page->type == Page::Type::Code_Page) {
        if (!(permissions & PAGE_READ)) {
            kdebug("Memory::protect removing read flag from code page is not handled");
        }
        page->flags = flags;
    } else {
        kdebug("Memory::protect didn't expect page type: %d", page->type);
    }
}

bool Memory::findFirstAvailablePage(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative) {
    U32 i;
    
    for (i=startingPage;i<K_NUMBER_OF_PAGES;i++) {
        if (alignNative && !isAlignedNativePage(i)) {
            continue;
        }
        if (i + pageCount >= K_NUMBER_OF_PAGES) {
            return false;
        }
        if (this->getPage(i)->type==Page::Type::Invalid_Page || (canBeReMapped && (this->getPage(i)->flags & PAGE_MAPPED))) {
            U32 j;
            bool success = true;

            for (j=1;j<pageCount;j++) {
                U32 nextPage = i+j; // could be done a different way, but this helps the static analysis
                if (nextPage < K_NUMBER_OF_PAGES && this->getPage(nextPage)->type!=Page::Type::Invalid_Page && (!canBeReMapped || !(this->getPage(i)->flags & PAGE_MAPPED))) {
                    success = false;
                    break;
                }
            }
            if (success) {
                *result = i;
                return true;
            }
            i+=j; // no reason to check all the pages again
        }
    }
    return false;
}

bool Memory::isValidReadAddress(U32 address, U32 len) {
    U32 startPage = address>>K_PAGE_SHIFT;
    U32 endPage = (address+len-1)>>K_PAGE_SHIFT;
    for (U32 i=startPage;i<=endPage;i++) {
        if (!this->getPage(i)->canRead())
            return false;
    }
    return true;
}

bool Memory::isValidWriteAddress(U32 address, U32 len) {
    U32 startPage = address>>K_PAGE_SHIFT;
    U32 endPage = (address+len-1)>>K_PAGE_SHIFT;
    for (U32 i=startPage;i<=endPage;i++) {
        if (!this->getPage(i)->canWrite())
            return false;
    }
    return true;
}

bool Memory::isPageAllocated(U32 page) {
    return this->getPage(page)->type!=Page::Type::Invalid_Page;
}

U8* getPhysicalReadAddress(U32 address, U32 len) {
    int index = address >> 12;
    if (len<=K_PAGE_SIZE-(address & K_PAGE_MASK)) {
        return Memory::currentMMU[index]->getReadAddress(address, len);
    }
    return NULL;
}

U8* getPhysicalWriteAddress(U32 address, U32 len) {
    int index = address >> 12;
    if (len<=K_PAGE_SIZE-(address & K_PAGE_MASK)) {
        return Memory::currentMMU[index]->getWriteAddress(address, len);
    }
    return NULL;
}

U8* getPhysicalAddress(U32 address, U32 len) {
    int index = address >> 12;
    if (len<=K_PAGE_SIZE-(address & K_PAGE_MASK)) {
        return Memory::currentMMU[index]->getReadWriteAddress(address, len);
    }
    return NULL;
}

void memcopyFromNative(U32 address, const void* pv, U32 len) {
#ifdef UNALIGNED_MEMORY
    U32 i;
    U8* p = (U8*)pv;
    for (i=0;i<len;i++) {
        writeb(address+i, p[i]);
    }
#else
    U32 i;
    U8* p = (U8*)pv;

    if (len>4) {
        U32 todo = K_PAGE_SIZE-(address & (K_PAGE_SIZE-1));
        if (todo>len)
            todo=len;
        U8* ram = getPhysicalWriteAddress(address, todo);
    
        if (ram) {            
            while (1) {
                memcpy(ram, p, todo);
                len-=todo;
                if (!len) {
                    return;
                }
                address+=todo;
                p+=todo;
                ram = getPhysicalWriteAddress(address, todo);
                if (!ram) {
                    break;
                }
                todo = K_PAGE_SIZE;
                if (todo>len)
                    todo=len;
            }
        }
    }

    for (i=0;i<len;i++) {
        writeb(address+i, p[i]);
    }
#endif
}

void memcopyToNative(U32 address, void* pv, U32 len) {
#ifdef UNALIGNED_MEMORY
    U8* p = (U8*)pv;
    for (U32 i=0;i<len;i++) {
        p[i] = readb(address+i);
    }
#else
    U8* p = (U8*)pv;
    if (len>4) {
        U32 todo = K_PAGE_SIZE-(address & (K_PAGE_SIZE-1));
        if (todo>len)
            todo=len;
        U8* ram = getPhysicalReadAddress(address, todo);
    
        if (ram) {            
            while (1) {
                memcpy(p, ram, todo);
                len-=todo;
                if (!len) {
                    return;
                }
                address+=todo;
                p+=todo;
                ram = getPhysicalReadAddress(address, todo);
                if (!ram) {
                    break;
                }
                todo = K_PAGE_SIZE;
                if (todo>len)
                    todo=len;
            }
        }
    }
    
    for (U32 i=0;i<len;i++) {
        p[i] = readb(address+i);
    }
#endif
}

void writeNativeString(U32 address, const char* str) {	
    while (*str) {
        writeb(address, *str);
        str++;
        address++;
    }
    writeb(address, 0);
}

U32 writeNativeString2(U32 address, const char* str, U32 len) {	
    U32 count=0;

    while (*str && count<len-1) {
        writeb(address, *str);
        str++;
        address++;
        count++;
    }
    writeb(address, 0);
    return count;
}

void writeNativeStringW(U32 address, const char* str) {	
    while (*str) {
        writew(address, *str);
        str++;
        address+=2;
    }
    writew(address, 0);
}

char* getNativeString(U32 address, char* buffer, U32 cbBuffer) {
    char c;
    U32 i=0;

    if (!address) {
        buffer[0]=0;
        return buffer;
    }
    do {
        c = readb(address++);
        buffer[i++] = c;
    } while(c && i<cbBuffer);
    buffer[cbBuffer-1]=0;
    return buffer;
}

U32 getNativeStringLen(U32 address) {
    char c;
    U32 i=0;

    if (!address) {
        return 0;
    }
    do {
        c = readb(address++);
        i++;
    } while(c);
    return i;
}

char* getNativeStringW(U32 address, char* buffer, U32 cbBuffer) {
    char c;
    U32 i=0;

    if (!address) {
        buffer[0]=0;
        return buffer;
    }
    do {
        c = (char)readw(address);
        address+=2;
        buffer[i++] = c;
    } while(c && i<cbBuffer);
    buffer[cbBuffer-1]=0;
    return buffer;
}

U32 Memory::mapNativeMemory(void* hostAddress, U32 size) {
    U32 result = 0;

    if (this->nativeAddressStart && hostAddress>=this->nativeAddressStart && (U8*)hostAddress+size<(U8*)this->nativeAddressStart+0x10000000) {
        return (ADDRESS_PROCESS_NATIVE<<K_PAGE_SHIFT) + (U32)(((U8*)hostAddress-(U8*)this->nativeAddressStart));
    }
    if (!this->nativeAddressStart) {
        U32 i;

        // just assume we are in the middle, hopefully OpenGL won't want more than 128MB before or after this initial address
        this->nativeAddressStart = ((U8*)hostAddress - ((uintptr_t)hostAddress & 0xFFF)) - 0x08000000;
        if (this->nativeAddressStart>hostAddress) // did we wrap?
            this->nativeAddressStart = (U8*)0x1000;
        for (i=0;i<0x10000;i++) {
            this->setPage(i+ADDRESS_PROCESS_NATIVE, NativePage::alloc(this->nativeAddressStart+K_PAGE_SIZE*i, (ADDRESS_PROCESS_NATIVE<<K_PAGE_SHIFT)+K_PAGE_SIZE*i, PAGE_READ | PAGE_WRITE));
        }
        return mapNativeMemory(hostAddress, size);
    }
    U32 pageCount = (size+K_PAGE_MASK)>>K_PAGE_SHIFT;
    // hopefully this won't happen much, because it will leak address space
    if (!findFirstAvailablePage(ADDRESS_PROCESS_MMAP_START, pageCount, &result, false)) {
        kpanic("mapNativeMemory failed to map address: size=%d", size);
    }

    for (U32 i=0;i<pageCount;i++) {
        this->setPage(result+i, NativePage::alloc((U8*)hostAddress+K_PAGE_SIZE*i, (result<<K_PAGE_SHIFT)+K_PAGE_SIZE*i, PAGE_READ | PAGE_WRITE));
    }
    return result<<K_PAGE_SHIFT;
}

void Memory::map(U32 startPage, const std::vector<U8*>& pages, U32 permissions) {
    bool read = (permissions & PAGE_READ)!=0 || (permissions & PAGE_EXEC)!=0;
    bool write = (permissions & PAGE_WRITE)!=0;

    for (U32 page=0;page<pages.size();page++) {
        if (read && write) {
            this->setPage(startPage+page, RWPage::alloc(pages[page], (startPage+page)<<K_PAGE_SHIFT, permissions));
        } else if (read) {
            this->setPage(startPage+page, ROPage::alloc(pages[page], (startPage+page)<<K_PAGE_SHIFT, permissions));
        } else if (write) {
            this->setPage(startPage+page, WOPage::alloc(pages[page], (startPage+page)<<K_PAGE_SHIFT, permissions));
        } else {
            this->setPage(startPage+page, NOPage::alloc(pages[page], (startPage+page)<<K_PAGE_SHIFT, permissions));
        }
    }
}

DecodedBlock* Memory::getCodeBlock(U32 startIp) {
    Page* page = this->getPage(startIp >> K_PAGE_SHIFT);
    if (page->type == Page::Type::Code_Page) {
        CodePage* codePage = (CodePage*)page;
        return codePage->getCode(startIp);
    }
    return NULL;
}

void Memory::addCodeBlock(U32 startIp, DecodedBlock* block) {
    // might have changed after a read
    Page* page = this->getPage(startIp >> K_PAGE_SHIFT);

    CodePage* codePage; 
    if (page->type == Page::Type::Code_Page) {
        codePage = (CodePage*)page;
    } else {
        if (page->type == Page::Type::RO_Page || page->type == Page::Type::RW_Page || page->type == Page::Type::Copy_On_Write_Page || page->type == Page::Type::Native_Page) {
            RWPage* p = (RWPage*)page;
            codePage = CodePage::alloc(p->page, p->address, p->flags);
            this->setPage(startIp >> K_PAGE_SHIFT, codePage);
        } else {
            kpanic("Unhandled code caching page type: %d", page->type);
		    codePage = nullptr;
        }
    }
    codePage->addCode(startIp, block, block->bytes);
}

U32 Memory::getPageFlags(U32 page) {
    return this->getPage(page)->flags;
}

void Memory::onThreadChanged() {
    Memory::currentMMU = this->mmu;
    Memory::currentMMUReadPtr = this->mmuReadPtr;
    Memory::currentMMUWritePtr = this->mmuWritePtr;
}

void Memory::setPage(U32 index, Page* page) {
    Page* p = this->mmu[index]; 
    this->mmu[index] = page; 
    this->mmuReadPtr[index] = page->getCurrentReadPtr();
    this->mmuWritePtr[index] = page->getCurrentWritePtr();
    p->close();
}
#endif
