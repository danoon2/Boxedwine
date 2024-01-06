#include "boxedwine.h"

#include "bufferaccess.h"

FsOpenNode* openSysCpuScalingMaxFrequency(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, BString::valueOf(Platform::getCpuMaxScalingFreqMHz(data)*1000));
}