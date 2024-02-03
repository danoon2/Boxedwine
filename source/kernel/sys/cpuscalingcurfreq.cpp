#include "boxedwine.h"

#include "bufferaccess.h"

FsOpenNode* openSysCpuScalingCurrentFrequency(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, BString::valueOf(Platform::getCpuCurScalingFreqMHz(data)*1000));
}