#include "boxedwine.h"
#include "x11.h"

XDrawable::XDrawable(U32 width, U32 height, U32 depth, const VisualPtr& visual, bool isWindow) : id(XServer::getNextId()), isWindow(isWindow), depth(depth), visual(visual), w(width), h(height) {
	data = nullptr;
	setSize(width, height);
}

XDrawable::~XDrawable() {
	delete[] data;
}

U32 XDrawable::getImage(KThread* thread, S32 x, S32 y, U32 width, U32 height, U32 planeMask, U32 format, U32 redMask, U32 greenMask, U32 blueMask) {
	U32 image = thread->process->alloc(thread, sizeof(XImage));
	if (planeMask != AllPlanes) {
		kpanic_fmt("XDrawable::createXImage wasn't expecting planeMask = %x", planeMask);
	}
	if (format != ZPixmap) {
		kpanic_fmt("XDrawable::createXImage wasn't expecting format = %x", format);
	}
	U32 bytesPerLine = calculateBytesPerLine(width, visual->bits_per_rgb);
	U32 len = bytesPerLine * height;
	U32 data = thread->process->alloc(thread, len);

	U32 dst = data;
	U8* src = this->data + this->bytes_per_line * y + (visual->bits_per_rgb * x + 7) / 8;
	for (U32 y = 0; y < height; y++) {
		thread->memory->memcpy(dst, src, bytesPerLine);
		src += this->bytes_per_line;
		dst += bytesPerLine;
	}

	XImage::set(thread->memory, image, width, height, 0, format, data, 32, depth, bytesPerLine, visual->bits_per_rgb, redMask, greenMask, blueMask);
	return image;
}

U32 XDrawable::calculateBytesPerLine(U32 bitsPerPixel, U32 width) {
	U32 result = width * bitsPerPixel / 8;
	result = (result + 3) / 4 * 4;
	return result;
}

void XDrawable::lockData() {
	BOXEDWINE_MUTEX_LOCK(mutex);
}

void XDrawable::unlockData() {
	BOXEDWINE_MUTEX_UNLOCK(mutex);
}

void XDrawable::setSize(U32 width, U32 height) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
	w = width;
	h = height;
	bytes_per_line = calculateBytesPerLine(visual?visual->bits_per_rgb:32, width);
	size = height * bytes_per_line;
	if (data) {
		delete[] data;
	}
	data = new U8[size];
	memset(data, 0, size);
}

int XDrawable::putImage(KThread* thread, const std::shared_ptr<XGC>& gc, XImage* image, S32 src_x, S32 src_y, S32 dest_x, S32 dest_y, U32 width, U32 height) {
	if (gc->values.function != GXcopy) {
		kwarn_fmt("XPixmap::putImage function not supported %d", gc->values.function);
	}
	return copyImageData(thread, gc, image->data, image->bytes_per_line, image->bits_per_pixel, src_x, src_y, dest_x, dest_y, width, height);
}

int XDrawable::copyImageData(KThread* thread, const std::shared_ptr<XGC>& gc, U32 data, U32 bytes_per_line, S32 bits_per_pixel, S32 src_x, S32 src_y, S32 dst_x, S32 dst_y, U32 width, U32 height) {
	if (bits_per_pixel != this->visual->bits_per_rgb) {
		return BadMatch;
	}
	if (gc && (gc->clip_rects.size() || gc->values.clip_mask || gc->values.clip_x_origin || gc->values.clip_y_origin)) {
		//klog("XDrawable::copyImageData clipping not implemented");
	}
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
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

	if (!gc || gc->values.function == GXcopy) {
		for (U32 y = 0; y < height; y++) {
			if (!memory->canRead(src, copyPerLine)) {
				return BadValue;
			}
			memory->memcpy(dst, src, copyPerLine);
			src += bytes_per_line;
			dst += this->bytes_per_line;
		}
	} else if (gc->values.function == GXxor && bits_per_pixel == 32) {
		for (U32 y = 0; y < height; y++) {
			if (!memory->canRead(src, copyPerLine)) {
				return BadValue;
			}
			U32* dstPixel = (U32*)dst;
			for (U32 x = 0; x < width; x++) {
				dstPixel[x] ^= memory->readd(src + x * 4);
			}			
			src += bytes_per_line;
			dst += this->bytes_per_line;
		}
	}
	setDirty();
	return Success;
}

int XDrawable::copy(KThread* thread, const std::shared_ptr<XGC>& gc, const std::shared_ptr<XDrawable>& srcDrawable, S32 srcX, S32 srcY, U32 width, U32 height, S32 dstX, S32 dstY) {
	if (srcDrawable->visual->bits_per_rgb != this->visual->bits_per_rgb) {
		return BadMatch;
	}
	if (gc && (gc->clip_rects.size() || gc->values.clip_mask || gc->values.clip_x_origin || gc->values.clip_y_origin)) {
		//klog("XDrawable::copyImageData clipping not implemented");
	}
	U8* src = srcDrawable->data + srcDrawable->bytes_per_line * srcY + (srcDrawable->visual->bits_per_rgb * srcX + 7) / 8;
	U8* dst = this->data + this->bytes_per_line * dstY + (this->visual->bits_per_rgb * dstX + 7) / 8;

	if (dstX + width > w) {
		if ((S32)w < dstX) {
			return Success;
		}
		width = w - dstX;
	}
	if (dstY + (S32)height > (S32)h) {
		if ((S32)h < dstY) {
			return Success;
		}
		height = h - dstY;
	}
	U32 copyPerLine = (this->visual->bits_per_rgb * width + 7) / 8;

	for (U32 y = 0; y < height; y++) {
		memcpy(dst, src, copyPerLine);
		src += srcDrawable->bytes_per_line;
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
		if (visual->bits_per_rgb == 32) {
			U32* p = (U32*)data;
			U32 color = gc->values.foreground;
			p += bytes_per_line / 4 * y1;
			p += x1;
			for (S32 y = y1; y < y2 && y < (S32)h; y++) {
				*p = color;
				p += bytes_per_line / 4;
			}
		} else {
			kpanic_fmt("XDrawable::drawLine depth %d not supported", visual->bits_per_rgb);
		}
	} else if (y1 == y2) {
		if (y1 >= (S32)h) {
			return Success;
		}
		if (visual->bits_per_rgb == 32) {
			U32* p = (U32*)data;
			U32 color = 0xff00;
			p += bytes_per_line / 4 * y1;
			for (S32 x = x1; x < x2 && x < (S32)w; x++) {
				*p = color;
				p++;
			}
		} else {
			kwarn_fmt("XDrawable::drawLine depth %d not supported", visual->bits_per_rgb);
		}
	} else {
		klog("XDrawable::drawLine diag line not supported");
	}
	return Success;
}

int XDrawable::fillRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height) {
	if (gc->clip_rects.size() || gc->values.clip_mask || gc->values.clip_x_origin || gc->values.clip_y_origin) {
		//klog("XDrawable::fillRectangle clipping not implemented");
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

	if (visual->bits_per_rgb == 32) {
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