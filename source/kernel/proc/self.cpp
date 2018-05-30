#include "boxedwine.h"

#include "bufferaccess.h"

FsOpenNode* openProcSelfExe(const BoxedPtr<FsNode>& node, U32 flags) {
    return new BufferAccess(node, flags, KThread::currentThread()->process->exe);
}