#ifndef __X_GC_H__
#define __X_GC_H__

#include "x11.h"
#include "xdrawable.h"
#include "xpixmap.h"

#define XGCPtr std::shared_ptr<XGC>

// subwindow_mode
#define ClipByChildren		0
#define IncludeInferiors	1

class XGC {
public:
	XGC(const XDrawablePtr& drawable);

	const U32 id;
	bool graphics_exposures;
	S32 subwindow_mode;
	XPixmapPtr clip_mask;
	std::vector<XRectangle> clip_rects;
	S32 clip_x_origin;
	S32 clip_y_origin;
private:
	XDrawablePtr drawable;
};

#endif