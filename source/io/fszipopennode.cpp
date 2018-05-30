#include "boxedwine.h"

#ifdef BOXEDWINE_ZLIB

#include "fszipopennode.h"
#include "fszipnode.h"
#include "fszip.h"


FsZipOpenNode::FsZipOpenNode(BoxedPtr<FsZipNode> node, U32 flags, U64 offset) : FsOpenNode(node, flags), zipNode(node), pos(0), offset(offset) {
}

S64 FsZipOpenNode::length() {
    return this->node->length();
}

bool FsZipOpenNode::setLength(S64 len) {
    // if this file was open for write, it would have been copied to the file system
    kpanic("FsZipOpenNode::setLength should not have been called"); 
    return false;
}

S64 FsZipOpenNode::getFilePointer() {
    return this->pos;
}

S64 FsZipOpenNode::seek(S64 pos) {
    if (pos>(S64)this->node->length())
        this->pos = this->node->length();
    else
        this->pos = pos;
    return this->pos;
}

static U64 lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
static U64 lastZipFileOffset;

static void setupZipRead(U64 zipOffset, U64 zipFileOffset) {
    char tmp[4096];

    if (zipOffset != lastZipOffset || zipFileOffset < lastZipFileOffset) {
        unzCloseCurrentFile(FsZip::zipfile);
        unzSetOffset64(FsZip::zipfile, zipOffset);
        lastZipFileOffset = 0;
        unzOpenCurrentFile(FsZip::zipfile);
        lastZipOffset = zipOffset;
    }
    if (zipFileOffset != lastZipFileOffset) {
        U32 todo = (U32)(zipFileOffset - lastZipFileOffset);
        while (todo) {
            todo-=unzReadCurrentFile(FsZip::zipfile, tmp, todo>4096?4096:todo);
        }
    }        
}

void FsZipOpenNode::close() {
}

U32 FsZipOpenNode::ioctl(U32 request) {
    return -K_ENODEV;
}

void FsZipOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kwarn("FsZipOpenNode::setAsync not implemented");
}

bool FsZipOpenNode::isAsync() {
    return false;
}

void FsZipOpenNode::waitForEvents(U32 events) {
    kwarn("FsZipOpenNode::waitForEvents not implemented");
}

bool FsZipOpenNode::isWriteReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

bool FsZipOpenNode::isReadReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 FsZipOpenNode::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsZipOpenNode::canMap() {
    return true;
}

U32 FsZipOpenNode::readNative(U8* buffer, U32 len) {
    U32 result;

    setupZipRead(this->offset, this->pos);    
    result = unzReadCurrentFile(FsZip::zipfile, buffer, len);
    this->pos+=result;
    lastZipFileOffset = this->pos;
    return result;
}

U32 FsZipOpenNode::writeNative(U8* buffer, U32 len) {
    kpanic("FsZipOpenNode::writeNative not implemented");
    return 0;
}

void FsZipOpenNode::reopen() {
    this->pos = 0;
}

#endif