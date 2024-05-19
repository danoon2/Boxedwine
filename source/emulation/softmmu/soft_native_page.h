#ifndef __SOFT_NATIVE_PAGE_H__
#define __SOFT_NATIVE_PAGE_H__

#include "soft_page.h"

class NativePage : public Page {
protected:
    NativePage(U8* nativeAddress, U32 address);

public:
    static NativePage* alloc(U8* nativeAddress, U32 address);

    // from Page
    U8 readb(U32 address) override;
    void writeb(U32 address, U8 value) override;
    U16 readw(U32 address) override;
    void writew(U32 address, U16 value) override;
    U32 readd(U32 address) override;
    void writed(U32 address, U32 value) override;
    U8* getReadPtr(KMemory* memory, U32 address, bool makeReady = false) override;
    U8* getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady = false) override;
    Type getType() override { return Type::Native_Page; }
    bool inRam() override {return true;}
    void close() override {delete this;}

    U8* nativeAddress;
    U32 address;
};

#endif