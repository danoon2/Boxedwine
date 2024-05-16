#include "boxedwine.h"
#include "kobject.h"

KObject::KObject(U32 type) : type(type) {
}

U32 KObject::writev(KThread* thread, U32 iov, S32 iovcnt) {
    U32 len=0;
    KMemory* memory = thread->memory;

    for (S32 i=0;i<iovcnt;i++) {
        U32 buf = memory->readd(iov + i * 8);
        U32 toWrite = memory->readd(iov + i * 8 + 4);
        S32 result;

        result = this->write(thread, buf, toWrite);
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

U32 KObject::read(KThread* thread, U32 address, U32 len) {
    U32 result = 0;
    KMemory* memory = thread->memory;

    memory->performOnMemory(address, len, true, [&result, this](U8* ram, U32 len) {
        U32 read = this->readNative(ram, len);
        if ((S32)read < 0) {
            result = read;
            return false;
        }
        result += read;
        return read == len;
        });
    return result;
}

U32 KObject::write(KThread* thread, U32 address, U32 len) {
    U32 result = 0;
    KMemory* memory = thread->memory;

    memory->performOnMemory(address, len, false, [&result, this](U8* ram, U32 len) {
        U32 written = this->writeNative(ram, len);
        if ((S32)written < 0) {
            result = written;
            return false;
        }
        result += written;
        return written == len;
        });
    return result;
}