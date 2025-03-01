/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __KOPENGL_H__
#define __KOPENGL_H__

#include "../../source/x11/xfbconfig.h"

class GLPixelFormat;
class XWindow;
class XDrawable;

class KOpenGL {
public:
    virtual ~KOpenGL() {}
    virtual U32 glCreateContext(KThread* thread, const std::shared_ptr<GLPixelFormat>& pixelFormat, int major, int minor, int profile, int flags, U32 sharedContext) = 0;
    virtual void glDestroyContext(KThread* thread, U32 contextId) = 0;
    virtual bool glMakeCurrent(KThread* thread, const std::shared_ptr<XDrawable>& d, U32 contextId) = 0;
    virtual void glSwapBuffers(KThread* thread, const std::shared_ptr<XDrawable>& d) = 0;
    virtual void glCreateWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd, const CLXFBConfigPtr& cfg) = 0;
    virtual void glDestroyWindow(KThread* thread, const std::shared_ptr<XWindow>& wnd) = 0;
    virtual void glResizeWindow(const std::shared_ptr<XWindow>& wnd) = 0;
    virtual bool isActive() = 0;
    virtual GLPixelFormatPtr getFormat(U32 pixelFormatId) = 0;
    virtual void warpMouse(int x, int y) = 0;
    virtual U32 getLastUpdateTime() = 0;
    virtual void hideCurrentWindow() = 0;
};

typedef std::shared_ptr<KOpenGL> KOpenGLPtr;

#endif
