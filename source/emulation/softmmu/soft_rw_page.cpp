#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_rw_page.h"
#include "soft_memory.h"
#include "soft_ram.h"

RWPage* RWPage::alloc(U8* page, U32 address, U32 flags) {
    return new RWPage(page, address, flags);
}

RWPage::RWPage(U8* page, U32 address, U32 flags, Type type) : Page(type, flags), address(address) {
    if (page) {
        this->page = page;
        ramPageIncRef(page);
    } else {
        this->page = ramPageAlloc();
    }
}

RWPage::~RWPage() {
    ramPageDecRef(this->page);
}

U8 RWPage::readb(U32 address) {
    return this->page[address-this->address];
}

void RWPage::writeb(U32 address, U8 value) {
    this->page[address-this->address]=value;
}

U16 RWPage::readw(U32 address) {
#ifdef UNALIGNED_MEMORY
        return this->page[address-this->address] | ((U16)this->page[address-this->address+1] << 8);
#else
        return *(U16*)(&this->page[address-this->address]);
#endif
}

void RWPage::writew(U32 address, U16 value) {
#ifdef UNALIGNED_MEMORY
        this->page[address-this->address] = (U8)value;
        this->page[address-this->address+1] = (U8)(value >> 8);
#else
        *(U16*)(&this->page[address-this->address]) = value;
#endif
}

U32 RWPage::readd(U32 address) {
#ifdef UNALIGNED_MEMORY
        return this->page[address-this->address] | ((U32)this->page[address-this->address+1] << 8) | ((U32)this->page[address-this->address+2] << 16) | ((U32)this->page[address-this->address+3] << 24);
#else
        return *(U32*)(&this->page[address-this->address]);
#endif
}

void RWPage::writed(U32 address, U32 value) {
#ifdef UNALIGNED_MEMORY
        this->page[address++ - this->address] = (U8)value;
        this->page[address++ - this->address] = (U8)(value >> 8);
        this->page[address++ - this->address] = (U8)(value >> 16);
        this->page[address - this->address] = (U8)(value >> 24);
#else
        *(U32*)(&this->page[address - this->address]) = value;
#endif
}

U8* RWPage::physicalAddress(U32 address) {
    return &this->page[address - this->address];
}

U8* RWPage::getRWAddress(U32 address) {
    return &this->page[address - this->address];
}

#endif