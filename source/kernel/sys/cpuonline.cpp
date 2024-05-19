#include "boxedwine.h"

#include "bufferaccess.h"

FsOpenNode* openSysCpuOnline(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    int count = Platform::getCpuCount();
    if (count<2) {
        return new BufferAccess(node, flags, B("0"));
    }
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "0-%d", (count-1));
    return new BufferAccess(node, flags, BString::copy(tmp));
}
