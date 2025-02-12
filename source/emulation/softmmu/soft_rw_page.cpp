#include "boxedwine.h"

#include "soft_rw_page.h"
#include "soft_ram.h"

RWPage* RWPage::alloc(RamPage page, U32 address) {
    return new RWPage(page, address);
}

RWPage::RWPage(RamPage page, U32 address) : address(address) {
    if (page.value) {
        this->page = page;
        ramPageRetain(page);
    } else {
        this->page = ramPageAlloc();
    }
    this->ram = ramPageGet(this->page);
}

RWPage::~RWPage() {
    ramPageRelease(this->page);
}

U8 RWPage::readb(U32 address) {
    return this->ram[address-this->address];
}

void RWPage::writeb(U32 address, U8 value) {
    this->ram[address-this->address]=value;
}

U16 RWPage::readw(U32 address) {
#ifdef UNALIGNED_MEMORY
        return this->ram[address-this->address] | ((U16)this->ram[address-this->address+1] << 8);
#else
        return *(U16*)(&this->ram[address-this->address]);
#endif
}

void RWPage::writew(U32 address, U16 value) {
#ifdef UNALIGNED_MEMORY
        this->ram[address-this->address] = (U8)value;
        this->ram[address-this->address+1] = (U8)(value >> 8);
#else
        *(U16*)(&this->ram[address-this->address]) = value;
#endif
}

U32 RWPage::readd(U32 address) {
#ifdef UNALIGNED_MEMORY
        return this->ram[address-this->address] | ((U32)this->ram[address-this->address+1] << 8) | ((U32)this->ram[address-this->address+2] << 16) | ((U32)this->ram[address-this->address+3] << 24);
#else
        return *(U32*)(&this->ram[address-this->address]);
#endif
}

void RWPage::writed(U32 address, U32 value) {
#ifdef UNALIGNED_MEMORY
        this->ram[address++ - this->address] = (U8)value;
        this->ram[address++ - this->address] = (U8)(value >> 8);
        this->ram[address++ - this->address] = (U8)(value >> 16);
        this->ram[address - this->address] = (U8)(value >> 24);
#else
        *(U32*)(&this->ram[address - this->address]) = value;
#endif
}

U8* RWPage::getRamPtr(KMemory* memory, U32 page, bool write, bool force, U32 offset, U32 len) {
    return ram + offset;
}