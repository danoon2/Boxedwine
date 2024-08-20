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
	return copyImageData(thread, gc, image->data, image->bytes_per_line, image->bits_per_pixel, src_x, src_y, dest_x, dest_y, width, height);
}

int XDrawable::copyImageData(KThread* thread, const std::shared_ptr<XGC>& gc, U32 data, U32 bytes_per_line, U32 bits_per_pixel, S32 src_x, S32 src_y, S32 dst_x, S32 dst_y, U32 width, U32 height) {
	if (bits_per_pixel != this->bits_per_pixel) {
		return BadMatch;
	}
	if (gc && (gc->clip_rects.size() || gc->values.clip_mask || gc->values.clip_x_origin || gc->values.clip_y_origin)) {
		klog("XDrawable::copyImageData clipping not implemented");
	}
	U32 src = data + bytes_per_line * src_y + (bits_per_pixel * src_x + 7) / 8;
	U8* dst = this->data + this->bytes_per_line * dst_y + (bits_per_pixel * dst_x + 7) / 8;
	KMemory* memory = thread->memory;
	if (dst_x + width > w) {
		if ((S32)w < dst_x) {
			return Success;
		}
		width = w - dst_x;
	}
	if (dst_y + (S32)height > (S32)h) {
		if ((S32)h < dst_y) {
			return Success;
		}
		height = h - dst_y;
	}
	U32 copyPerLine = (bits_per_pixel * width + 7) / 8;

	for (U32 y = 0; y < height; y++) {
		memory->memcpy(dst, src, copyPerLine);
		src += bytes_per_line;
		dst += this->bytes_per_line;
	}
	setDirty();
	return Success;
}

int XDrawable::drawLine(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x1, S32 y1, S32 x2, S32 y2) {
	if (x1 == x2) {
		if (x1 >= (S32)w) {
			return Success;
		}
		if (bits_per_pixel == 32) {
			U32* p = (U32*)data;
			U32 color = gc->values.foreground;
			p += bytes_per_line / 4 * y1;
			p += x1;
			for (S32 y = y1; y < y2 && y < (S32)h; y++) {
				*p = color;
				p += bytes_per_line / 4;
			}
		} else {
			kpanic("XDrawable::drawLine depth %d not supported", bits_per_pixel);
		}
	} else if (y1 == y2) {
		if (y1 >= (S32)h) {
			return Success;
		}
		if (bits_per_pixel == 32) {
			U32* p = (U32*)data;
			U32 color = 0xff00;
			p += bytes_per_line / 4 * y1;
			for (S32 x = x1; x < x2 && x < (S32)w; x++) {
				*p = color;
				p++;
			}
		} else {
			kpanic("XDrawable::drawLine depth %d not supported", bits_per_pixel);
		}
	} else {
		klog("XDrawable::drawLine diag line not supported");
	}
	return Success;
}

int XDrawable::fillRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height) {
	if (gc->clip_rects.size() || gc->values.clip_mask || gc->values.clip_x_origin || gc->values.clip_y_origin) {
		klog("XDrawable::fillRectangle clipping not implemented");
	}
	if (gc->values.tile) {
		kpanic("XDrawable::fillRectangle tile not supported");
	}
	if (width > w - x) {
		width = w - x;
	}
	if (height > h - y) {
		height = h - y;
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
	setDirty();
	return Success;
}

int XDrawable::drawRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height) {
	if (gc->clip_rects.size() || gc->values.clip_mask || gc->values.clip_x_origin || gc->values.clip_y_origin) {
		klog("XDrawable::drawRectangle clipping not implemented");
	}
	// draw rectangle with no overlapping pixels
	drawLine(thread, gc, x, y, x + width, y); // top (includes left and right)
	drawLine(thread, gc, x + width, y + 1, x + width, y + height); // right	( includes bottom but not top)
	drawLine(thread, gc, x, y + 1, x, y + height); // left (includes bottom but not top)
	drawLine(thread, gc, x + 1, y + height, x + width - 1, y + height); // bottom (does not include left and right)
	return Success;
}