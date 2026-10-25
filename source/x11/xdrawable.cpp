/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "x11.h"

XDrawable::XDrawable(U32 width, U32 height, U32 depth, const VisualPtr& visual, bool isWindow, bool isPBuffer) : id(XServer::getNextId()), isWindow(isWindow), isPBuffer(isPBuffer), depth(depth), visual(visual), w(width), h(height) {
	data = nullptr;
	setSize(width, height);
}

XPBuffer::XPBuffer(U32 width, U32 height, U32 depth, const VisualPtr& visual, U32 fbConfigId, bool preservedContents, U32 eventMask)
	: XDrawable(width, height, depth, visual, false, true), fbConfigId(fbConfigId), preservedContents(preservedContents), eventMask(eventMask) {
	isOpenGL = true;
}

XDrawable::~XDrawable() {
	delete[] data;
}

U32 XDrawable::getImage(KThread* thread, S32 x, S32 y, U32 width, U32 height, U32 planeMask, U32 format, U32 redMask, U32 greenMask, U32 blueMask) {
	if (!this->data || !width || !height) {
		return 0;
	}
	if (planeMask != AllPlanes) {
		kpanic_fmt("XDrawable::createXImage wasn't expecting planeMask = %x", planeMask);
	}
	if (format != ZPixmap) {
		kpanic_fmt("XDrawable::createXImage wasn't expecting format = %x", format);
	}
	U64 stride = (((U64)visual->bits_per_rgb * width + 31) / 32) * 4;
	if (stride > 0xffffffff || stride * height > 0xffffffff) return 0;
	U32 bytesPerLine = (U32)stride;
	U32 len = bytesPerLine * height;
	U32 image = thread->process->alloc(thread, sizeof(XImage));
	U32 data = thread->process->alloc(thread, len);

	// Wine's software-cursor save rectangle can cross any drawable edge.
	// Preserve its requested size and clear unavailable pixels, as copying
	// through a temporary pixmap would do. The filesystem's legacy X11 shim
	// dereferences the returned image, so rejecting an edge rectangle with
	// NULL faults while Wine holds its graphics error-handler mutex.
	std::vector<U8> pixels(len, 0);
	readImage(pixels.data(), bytesPerLine, x, y, width, height, 0, 0);
	thread->memory->memcpy(data, pixels.data(), len);

	XImage::set(thread->memory, image, width, height, 0, format, data, 32, depth, bytesPerLine, visual->bits_per_rgb, redMask, greenMask, blueMask);
	return image;
}

void XDrawable::readImage(U8* dst, U32 dstPitch, S32 x, S32 y, U32 width, U32 height, U32 dstX, U32 dstY) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
	S64 left = std::max<S64>(0, x), top = std::max<S64>(0, y);
	S64 right = std::min<S64>(w, (S64)x + width), bottom = std::min<S64>(h, (S64)y + height);
	if (!data || left >= right || top >= bottom) return;
	for (S64 sourceY = top; sourceY < bottom; ++sourceY) {
		const U8* src = data + bytes_per_line * sourceY;
		U8* row = dst + dstPitch * ((U64)dstY + sourceY - y);
		if (visual->bits_per_rgb == 1) {
			for (S64 pixel = left; pixel < right; ++pixel) {
				U32 target = dstX + (U32)(pixel - x);
				U8 mask = (U8)(1 << (target & 7));
				row[target / 8] = (row[target / 8] & ~mask) |
					(((src[pixel / 8] >> (pixel & 7)) & 1) << (target & 7));
			}
		} else {
			U32 pixelBytes = visual->bits_per_rgb / 8;
			memcpy(row + (dstX + left - x) * pixelBytes, src + left * pixelBytes, (right - left) * pixelBytes);
		}
	}
}

U32 XDrawable::calculateBytesPerLine(U32 bitsPerPixel, U32 width) {
	U32 result = (width * bitsPerPixel + 7) / 8;
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
	size = isPBuffer ? 0 : height * bytes_per_line;
	if (data) {
		delete[] data;
		data = nullptr;
	}
	if (size) {
		data = new U8[size];
		memset(data, 0, size);
	}
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

void XDrawable::drawPixel8(const XGC& gc, S32 x, S32 y) {
	if (x < 0 || y < 0 || (U32)x >= w || (U32)y >= h) return;
	S64 cx = (S64)x - gc.values.clip_x_origin, cy = (S64)y - gc.values.clip_y_origin;
	if (gc.hasClipRects) {
		bool visible = false;
		for (const auto& rect : gc.clip_rects) {
			if (cx >= rect.x && cy >= rect.y && cx < rect.x + rect.width && cy < rect.y + rect.height) {
				visible = true;
				break;
			}
		}
		if (!visible) return;
	}
	if (gc.values.clip_mask) {
		auto mask = XServer::getServer()->getPixmap(gc.values.clip_mask);
		if (!mask || cx < 0 || cy < 0 || cx >= mask->width() || cy >= mask->height() ||
			!(mask->getData()[cy * mask->getBytesPerLine() + cx / 8] & (1 << (cx & 7)))) return;
	}
	U8& dst = data[y * bytes_per_line + x];
	U8 src = (U8)gc.values.foreground, result = src;
	switch (gc.values.function) {
	case GXclear: result = 0; break;
	case GXand: result = src & dst; break;
	case GXandReverse: result = src & ~dst; break;
	case GXcopy: break;
	case GXandInverted: result = ~src & dst; break;
	case GXnoop: return;
	case GXxor: result = src ^ dst; break;
	case GXor: result = src | dst; break;
	case GXnor: result = ~(src | dst); break;
	case GXequiv: result = ~(src ^ dst); break;
	case GXinvert: result = ~dst; break;
	case GXorReverse: result = src | ~dst; break;
	case GXcopyInverted: result = ~src; break;
	case GXorInverted: result = ~src | dst; break;
	case GXnand: result = ~(src & dst); break;
	case GXset: result = 0xff; break;
	}
	dst = (dst & ~gc.values.plane_mask) | (result & gc.values.plane_mask);
}

int XDrawable::drawLine(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x1, S32 y1, S32 x2, S32 y2) {
	if (visual->bits_per_rgb == 8 && (x1 == x2 || y1 == y2)) {
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
		S32 left = std::max<S32>(0, std::min(x1, x2));
		S32 top = std::max<S32>(0, std::min(y1, y2));
		S32 right = std::min<S32>(w - 1, std::max(x1, x2));
		S32 bottom = std::min<S32>(h - 1, std::max(y1, y2));
		for (S32 y = top; y <= bottom; y++) {
			for (S32 x = left; x <= right; x++) {
				if (gc->values.cap_style != CapNotLast || x != x2 || y != y2) drawPixel8(*gc, x, y);
			}
		}
		setDirty();
		return Success;
	}
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
	if (visual->bits_per_rgb == 8 && gc->values.fill_style == FillSolid) {
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
		S32 left = std::max<S32>(0, x), top = std::max<S32>(0, y);
		S32 right = (S32)std::min<S64>(w, (S64)x + width);
		S32 bottom = (S32)std::min<S64>(h, (S64)y + height);
		for (S32 dstY = top; dstY < bottom; dstY++) {
			for (S32 dstX = left; dstX < right; dstX++) {
				drawPixel8(*gc, dstX, dstY);
			}
		}
		setDirty();
		return Success;
	}
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
        kwarn_fmt("XDrawable::fillRectangle only %d-bit not handled", visual->bits_per_rgb);
	}
	setDirty();
	return Success;
}

int XDrawable::drawRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height) {
	if (visual->bits_per_rgb != 8 && (gc->clip_rects.size() || gc->values.clip_mask || gc->values.clip_x_origin || gc->values.clip_y_origin)) {
		klog("XDrawable::drawRectangle clipping not implemented");
	}
	// draw rectangle with no overlapping pixels
	drawLine(thread, gc, x, y, x + width, y); // top (includes left and right)
	drawLine(thread, gc, x + width, y + 1, x + width, y + height); // right	( includes bottom but not top)
	drawLine(thread, gc, x, y + 1, x, y + height); // left (includes bottom but not top)
	drawLine(thread, gc, x + 1, y + height, x + width - 1, y + height); // bottom (does not include left and right)
	return Success;
}
