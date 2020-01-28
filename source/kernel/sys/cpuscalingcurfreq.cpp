#include "boxedwine.h"

#include "bufferaccess.h"

FsOpenNode* openSysCpuScalingCurrentFrequency(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, std::to_string(Platform::getCpuCurScalingFreqMHz(data)*1000));
}