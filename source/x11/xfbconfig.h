#ifndef __XFBCONFIG_H__
#define __XFBCONFIG_H__

#include "platformOpenGL.h"

class CLXFBConfig {
public:
    U32 fbId;
    U32 visualId;
    GLPixelFormatPtr glPixelFormat;
    U32 depth;
};

typedef std::shared_ptr<CLXFBConfig> CLXFBConfigPtr;
#endif