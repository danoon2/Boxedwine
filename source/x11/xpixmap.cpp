#include "boxedwine.h"
#include "x11.h"

XPixmap::XPixmap(U32 width, U32 height, U32 depth, const VisualPtr& visual) : XDrawable(width, height, depth, visual, false) {	
}