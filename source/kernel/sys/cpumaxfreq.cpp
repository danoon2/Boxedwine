#include "boxedwine.h"

#include "bufferaccess.h"

FsOpenNode* openSysCpuMaxFrequency(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, BString::valueOf(Platform::getCpuFreqMHz()*1000));
}