#include "boxedwine.h"

FsOpenNode::FsOpenNode(std::shared_ptr<FsNode> node, U32 flags) : node(node), flags(flags), listNode(this) {
    node->addOpenNode(&this->listNode);
}

FsOpenNode::~FsOpenNode() {
    this->node->removeOpenNode(this);
}

U32 FsOpenNode::internalRead(KThread* thread, U32 address, U32 len) {
    U32 result = 0;
    KMemory* memory = thread->memory;

    memory->performOnMemory(address, len, false, [&result, this](U8* ram, U32 len) {
        U32 read = this->readNative(ram, len);
        if ((S32)read < 0) {
            return false;
        }
        result += read;
        return read == len;
        });
    return result;
}

U32 FsOpenNode::read(KThread* thread, U32 address, U32 len) {
    BOXEDWINE_MUTEX* mutex = getReadMutex();
    if (mutex) {
        // this will reduce thashing in the zip file
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(*mutex);
        return internalRead(thread, address, len);
    } else {
        return internalRead(thread, address, len);
    }
}

U32 FsOpenNode::write(KThread* thread, U32 address, U32 len) {
    U32 result = 0;
    KMemory* memory = thread->memory;

    memory->performOnMemory(address, len, true, [&result, this](U8* ram, U32 len) {
        U32 written = this->writeNative(ram, len);
        if ((S32)written < 0) {
            return false;
        }
        result += written;
        return written == len;
        });
    return result;
}

void FsOpenNode::loadDirEntries() {
    BOXEDWINE_CRITICAL_SECTION;
    if (this->dirEntries.size()==0 && this->node) {
        this->dirEntries.reserve(2 + node->getChildCount());
        this->dirEntries.push_back(this->node);
        std::shared_ptr<FsNode> parent = this->node->getParent().lock();
        if (parent) {
            this->dirEntries.push_back(parent);
        }
        this->node->getAllChildren(this->dirEntries);        
    }
}

U32 FsOpenNode::getDirectoryEntryCount() {
    this->loadDirEntries();
    return (U32)this->dirEntries.size();
}

std::shared_ptr<FsNode> FsOpenNode::getDirectoryEntry(U32 index, BString& name) {
    if (!this->node) {
        return nullptr;
    }
    this->loadDirEntries();
    if (index==0)
        name = B(".");
    else if (index==1 && this->node->getParent().lock())
        name = B("..");
    else
        name = this->dirEntries[index]->name;
    return this->dirEntries[index];
}