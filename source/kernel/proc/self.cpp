#include "boxedwine.h"

#include "bufferaccess.h"

FsOpenNode* openProcSelfExe(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, KThread::currentThread()->process->exe);
}