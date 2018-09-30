#ifndef __SOFT_NATIVE_PAGE_H__
#define __SOFT_NATIVE_PAGE_H__

#include "soft_page.h"

class NativePage : public Page {
protected:
    NativePage(U8* nativeAddress, U32 address, U32 flags);

public:
    static NativePage* alloc(U8* nativeAddress, U32 address, U32 flags);

    U8 readb(U32 address);
    void writeb(U32 address, U8 value);
    U16 readw(U32 address);
    void writew(U32 address, U16 value);
    U32 readd(U32 address);
    void writed(U32 address, U32 value);
    U8* getCurrentReadPtr();
    U8* getCurrentWritePtr();
    U8* getReadAddress(U32 address, U32 len);
    U8* getWriteAddress(U32 address, U32 len);
    U8* getReadWriteAddress(U32 address, U32 len);
    bool inRam() {return true;}
    void close() {delete this;}

    U8* nativeAddress;
    U32 address;
};

#endif