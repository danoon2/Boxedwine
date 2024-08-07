#ifndef __X_PIXMAP_H__
#define __X_PIXMAP_H__

#include "xdrawable.h"

#define XPixmapPtr std::shared_ptr<XPixmap>

class XPixmap : public XDrawable {
public:
	XPixmap(U32 width, U32 height, U32 depth) : XDrawable(width, height, depth) {}
};

#endif