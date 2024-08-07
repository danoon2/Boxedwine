#include "boxedwine.h"
#include "xgc.h"
#include "displaydata.h"

XGC::XGC(const std::shared_ptr<XDrawable>& drawable) : id(DisplayData::getNextId()), drawable(drawable) {
	graphics_exposures = true;
	subwindow_mode = ClipByChildren;
	clip_x_origin = 0;
	clip_y_origin = 0;
}