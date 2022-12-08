#include "boxedwine.h"
#include "kobject.h"

KObject::KObject(U32 type) : type(type) {
    if (KThread::currentThread()) {
        this->pid = KThread::currentThread()->process->id;
    } else {
        this->pid = 0;
    }
}

U32 KObject::writev(U32 iov, S32 iovcnt) {
    U32 len=0;
    S32 i;

    for (i=0;i<iovcnt;i++) {
        U32 buf = readd(iov + i * 8);
        U32 toWrite = readd(iov + i * 8 + 4);
        S32 result;

        result = this->write(buf, toWrite);
        if (result<0) {
            if (i>0) {
                kwarn("writev partial fail: TODO file pointer should not change");
            }
            return result;
        }
        len+=result;
    }
    return len;
}

U32 KObject::read(U32 address, U32 len) {
    U8* ram = getPhysicalWriteAddress(address, len);

    if (ram) {
        return this->readNative(ram, len);	
    }

    U32 result = 0;
    while (len) {
        U32 todo = K_PAGE_SIZE-(address & (K_PAGE_SIZE-1));
        S32 didRead;
        
        ram = getPhysicalWriteAddress(address, todo);

        if (todo>len)
            todo = len;
        if (ram) {
            didRead=this->readNative(ram, todo);		
        } else {
            char tmp[K_PAGE_SIZE];
            didRead = this->readNative((U8*)tmp, todo);
            memcopyFromNative(address, tmp, didRead);
        }
        if (didRead<(S32)todo)
            break;
        len-=didRead;
        address+=didRead;
        result+=didRead;
    }
    return result;
}

U32 KObject::write(U32 address, U32 len) {
    U8* ram = getPhysicalReadAddress(address, len);

    if (ram) {
        return this->writeNative(ram, len);
    }

    U32 wrote = 0;
    while (len) {
        U32 todo = K_PAGE_SIZE-(address & (K_PAGE_SIZE-1));        
        char tmp[K_PAGE_SIZE];

        ram = getPhysicalReadAddress(address, todo);
        if (todo>len)
            todo = len;
        if (ram)
            wrote+=this->writeNative(ram, todo);
        else {
            memcopyToNative(address, tmp, todo);
            wrote+=this->writeNative((U8*)tmp, todo);		
        }
        len-=todo;
        address+=todo;
    }
    return wrote;
}