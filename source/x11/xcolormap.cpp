#include "boxedwine.h"
#include "x11.h"

XColorMap::XColorMap() : id(XServer::getNextId()) {
}

void XColorMap::buildCache() {
	if (dirty) {
		dirty = false;
		for (U32 i = 0; i < MAX_COLORMAP_SIZE; i++) {
			nativePixels[i] = K_RGB(colors[i].r, colors[i].g, colors[i].b);
		}
	}
}