#include "boxedwine.h"
#include "x11.h"

XDrawable::XDrawable(U32 width, U32 height, U32 depth) : id(XServer::getNextId()), w(width), h(height), depth(depth) {
	data = nullptr;
	setSize(width, height);
}

XDrawable::~XDrawable() {
	delete[] data;
}

void XDrawable::setSize(U32 width, U32 height) {
	w = width;
	h = height;
	if (depth == 24) {
		bits_per_pixel = 32;
	} else {
		bits_per_pixel = depth;
	}
	bytes_per_line = width * bits_per_pixel / 8;
	bytes_per_line = (bytes_per_line + 3) / 4 * 4;
	size = height * bytes_per_line;
	if (data) {
		delete[] data;
	}
	data = new U8[size];
	memset(data, 0, size);
}

int XDrawable::putImage(KThread* thread, const std::shared_ptr<XGC>& gc, XImage* image, S32 src_x, S32 src_y, S32 dest_x, S32 dest_y, U32 width, U32 height) {
	if (gc->values.function != GXcopy) {
		kpanic("XPixmap::putImage function not supported %d", gc->values.function);
	}
	if (depth != image->bits_per_pixel) {
		return BadMatch;
	}
	U32 src = image->data + image->bytes_per_line * src_y;
	U8* dst = data + bytes_per_line * dest_y;
	KMemory* memory = thread->memory;
	U32 copyPerLine = (image->bits_per_pixel * width + 7) / 8;

	for (U32 y = 0; y < height; y++) {
		memory->memcpy(dst, src, copyPerLine);
		src += image->bytes_per_line;
		dst += bytes_per_line;
	}
	isDirty = true;
	return Success;
}

int XDrawable::fillRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height) {
	if (width > this->width() - x) {
		width = this->width() - x;
	}
	if (height > this->height() - y) {
		height = this->height() - y;
	}

	if (bits_per_pixel == 32) {
		U32* p = (U32*)data;
		U32 color = gc->values.foreground;
		p += bytes_per_line * y;
		for (U32 dstY = 0; dstY < height; dstY++) {
			for (U32 dstX = 0; dstX < width; dstX++) {
				p[x + dstX] = color;
			}
			p += bytes_per_line/4;
		}
	}
	isDirty = true;
	return Success;
}
