#include "boxedwine.h"

#include "soft_native_page.h"
#include "soft_memory.h"

NativePage* NativePage::alloc(U8* nativeAddress, U32 address, U32 flags) {
    return new NativePage(nativeAddress, address, flags);
}

NativePage::NativePage(U8* nativeAddress, U32 address, U32 flags) : Page(Native_Page, flags), nativeAddress(nativeAddress), address(address) {
}

U8 NativePage::readb(U32 address) {
    return *(this->nativeAddress+(address-this->address));
}

void NativePage::writeb(U32 address, U8 value) {
    *(this->nativeAddress+(address-this->address))=value;
}

U16 NativePage::readw(U32 address) {
#ifdef UNALIGNED_MEMORY
        return this->readb(address) | ((U16)this->readb(address+1) << 8);
#else
        return *((U16*)(this->nativeAddress+(address-this->address)));
#endif
}

void NativePage::writew(U32 address, U16 value) {
#ifdef UNALIGNED_MEMORY
        this->writeb(address, (U8)value);
        this->writeb(address+1, (U8)(value >> 8));
#else
        *((U16*)(this->nativeAddress+(address-this->address))) = value;
#endif
}

U32 NativePage::readd(U32 address) {
#ifdef UNALIGNED_MEMORY
        return this->readb(address) | ((U32)this->readb(address+1) << 8) | ((U32)this->readb(address+2) << 16) | ((U32)this->readb(address+3) << 24);
#else
        return *((U32*)(this->nativeAddress+(address-this->address)));
#endif
}

void NativePage::writed(U32 address, U32 value) {
#ifdef UNALIGNED_MEMORY
        this->writeb(address, (U8)value);
        this->writeb(address+1, (U8)(value >> 8));
        this->writeb(address+2, (U8)(value >> 16));
        this->writeb(address+3, (U8)(value >> 24));
#else
        *((U32*)(this->nativeAddress+(address-this->address))) = value;
#endif
}

U8* NativePage::physicalAddress(U32 address) {
    return this->nativeAddress+(address-this->address);
}
