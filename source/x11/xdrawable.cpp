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
	return copyImageData(thread, image->data, image->bytes_per_line, image->bits_per_pixel, src_x, src_y, dest_x, dest_y, width, height);
}

int XDrawable::copyImageData(KThread* thread, U32 data, U32 bytes_per_line, U32 bits_per_pixel, S32 src_x, S32 src_y, S32 dst_x, S32 dst_y, U32 width, U32 height) {
	if (bits_per_pixel != this->bits_per_pixel) {
		return BadMatch;
	}
	U32 src = data + bytes_per_line * src_y;
	U8* dst = this->data + this->bytes_per_line * dst_y;
	KMemory* memory = thread->memory;
	if (dst_x + width > w) {
		width = w - dst_x;
	}
	if (dst_y + height > h) {
		height = h - dst_y;
	}
	U32 copyPerLine = (bits_per_pixel * width + 7) / 8;

	for (U32 y = 0; y < height; y++) {
		memory->memcpy(dst + dst_x * this->bits_per_pixel, src + src_x * bits_per_pixel, copyPerLine);
		src += bytes_per_line;
		dst += this->bytes_per_line;
	}
	isDirty = true;
	return Success;
}

int XDrawable::drawLine(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x1, S32 y1, S32 x2, S32 y2) {
	if (x1 == x2) {
		if (bits_per_pixel == 32) {
			U32* p = (U32*)data;
			U32 color = gc->values.foreground;
			p += bytes_per_line / 4 * y1;
			p += x1;
			for (U32 y = y1; y < y2; y++) {
				*p = color;
				p += bytes_per_line / 4;
			}
		} else {
			kpanic("XDrawable::drawLine depth %d not supported", bits_per_pixel);
		}
	} else if (y1 == y2) {
		if (bits_per_pixel == 32) {
			U32* p = (U32*)data;
			U32 color = 0xff00;
			p += bytes_per_line / 4 * y1;
			for (U32 x = x1; x < x2; x++) {
				*p = color;
				p++;
			}
		} else {
			kpanic("XDrawable::drawLine depth %d not supported", bits_per_pixel);
		}
	} else {
		kpanic("XDrawable::drawLine diag line not supported");
	}
	return Success;
}

int XDrawable::fillRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height) {
	if (gc->values.tile) {
		kpanic("XDrawable::fillRectangle tile not supported");
	}
	if (width > this->width() - x) {
		width = this->width() - x;
	}
	if (height > this->height() - y) {
		height = this->height() - y;
	}

	if (bits_per_pixel == 32) {
		U32* p = (U32*)data;
		U32 color = gc->values.foreground;
		p += bytes_per_line / 4 * y;
		for (U32 dstY = 0; dstY < height; dstY++) {
			for (U32 dstX = 0; dstX < width; dstX++) {
				p[x + dstX] = color;
			}
			p += bytes_per_line/4;
		}
	} else {
		int ii = 0;
	}
	isDirty = true;
	return Success;
}
