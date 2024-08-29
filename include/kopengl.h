#ifndef __KOPENGL_H__
#define __KOPENGL_H__

#include "../../source/x11/xfbconfig.h"

class GLPixelFormat;
class XWindow;
class XDrawable;

class KOpenGL {
public:
    virtual U32 glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContext) = 0;
    virtual void glDestroyContext(KThread* thread, U32 contextId) = 0;
    virtual bool glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) = 0;
    virtual void glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) = 0;
    virtual void glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) = 0;
    virtual void glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) = 0;
    virtual GLPixelFormatPtr getFormat(U32 pixelFormatId) = 0;
};

typedef std::shared_ptr<KOpenGL> KOpenGLPtr;

#endif
