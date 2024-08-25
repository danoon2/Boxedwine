#ifndef __XFBCONFIG_H__
#define __XFBCONFIG_H__

class CLXFBConfig {
public:
    U32 fbId;
    U32 visualId;
    U32 pixelFormatIndex;
    U32 depth;
};

typedef std::shared_ptr<CLXFBConfig> CLXFBConfigPtr;
#endif