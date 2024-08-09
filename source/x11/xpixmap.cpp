#include "boxedwine.h"
#include "x11.h"

int XPixmap::putImage(KThread* thread, const std::shared_ptr<XGC>& gc, XImage* image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height) {
	kpanic("XPixmap::putImage not implemented");
	return Success;
}