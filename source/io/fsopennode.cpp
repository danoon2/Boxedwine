#include "boxedwine.h"

FsOpenNode::FsOpenNode(BoxedPtr<FsNode> node, U32 flags) : node(node), flags(flags), listNode(this) {
    node->addOpenNode(&this->listNode);
}

FsOpenNode::~FsOpenNode() {
    this->node->removeOpenNode(this);
}

U32 FsOpenNode::read(U32 address, U32 len) {
    if (!len) {
        return 0;
    }
    if (K_PAGE_SIZE-(address & (K_PAGE_SIZE-1)) >= len) {
        U8* ram = getPhysicalWriteAddress(address, len);
        U32 result;
        S64 pos = this->getFilePointer();

        if (ram) {
            result = this->readNative(ram, len);	
        } else {
            char tmp[K_PAGE_SIZE];
            result = this->readNative((U8*)tmp, len);
            memcopyFromNative(address, tmp, result);
        }        
        // :TODO: why does this happen
        //
        // installing dpkg with dpkg
        // 1) dpkg will be mapped into memory (kfmmap on demand)
        // 2) dpkg will remove dpkg, this triggers the logic to close the handle move the file to /tmp then re-open it
        // 3) dpkg continues to run then hits a new part of the code that causes an on demand load
        // 4) on windows 7 x64 this resulted in a full read of one page, but the result returned by read was less that 4096, it was 0xac8 (2760)
        if (result<len) {
            if (this->getFilePointer()==pos+len) {
                result = len;
            }
        }
        return result;
    } else {		
        U32 result = 0;
        while (len) {
            U32 todo = K_PAGE_SIZE-(address & (K_PAGE_SIZE-1));
            S32 didRead;
            U8* ram = getPhysicalWriteAddress(address, todo);

            if (todo>len)
                todo = len;
            if (ram) {
                didRead=this->readNative(ram, todo);		
            } else {
                char tmp[K_PAGE_SIZE];
                didRead = this->readNative((U8*)tmp, todo);
                memcopyFromNative(address, tmp, didRead);
            }
            if (didRead<=0)
                break;
            len-=didRead;
            address+=didRead;
            result+=didRead;
        }
        return result;
    }
}

U32 FsOpenNode::write(U32 address, U32 len) {
    U32 wrote = 0;
    while (len) {
        U32 todo = K_PAGE_SIZE-(address & (K_PAGE_SIZE-1));
        U8* ram = getPhysicalReadAddress(address, todo);
        char tmp[K_PAGE_SIZE];

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

void FsOpenNode::loadDirEntries() {
    BOXEDWINE_CRITICAL_SECTION;
    if (this->dirEntries.size()==0 && this->node) {
        this->dirEntries.push_back(this->node);
        if (this->node->getParent())
            this->dirEntries.push_back(this->node->getParent());
        this->node->getAllChildren(this->dirEntries);        
    }
}

U32 FsOpenNode::getDirectoryEntryCount() {
    this->loadDirEntries();
    return (U32)this->dirEntries.size();
}

BoxedPtr<FsNode> FsOpenNode::getDirectoryEntry(U32 index, BString& name) {
    if (!this->node) {
        return NULL;
    }
    this->loadDirEntries();
    if (index==0)
        name = B(".");
    else if (index==1 && this->node->getParent())
        name = B("..");
    else
        name = this->dirEntries[index]->name;
    return this->dirEntries[index];
}