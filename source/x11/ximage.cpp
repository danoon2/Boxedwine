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

void XImage::set(KMemory* memory, U32 image, S32 width, S32 height, S32 offset, S32 format, U32 data, S32 bitmapPad, S32 depth, S32 bytesPerLine, S32 bitsPerPixel, U32 redMask, U32 greenMask, U32 blueMask) {
	U32 bitmap_unit = 32;
	if (depth == 8) {
		bitmap_unit = 8;
	} else if (depth == 16) {
		bitmap_unit = 16;
	}

	memory->writed(image, width);
	memory->writed(image + 4, height);
	memory->writed(image + 8, offset);
	memory->writed(image + 12, format);
	memory->writed(image + 16, data);
	memory->writed(image + 20, LSBFirst);
	memory->writed(image + 24, bitmap_unit);
	memory->writed(image + 28, LSBFirst);
	memory->writed(image + 32, bitmapPad);
	memory->writed(image + 36, depth);
	memory->writed(image + 40, bytesPerLine);
	memory->writed(image + 44, bitsPerPixel);
	memory->writed(image + 48, redMask);
	memory->writed(image + 52, greenMask);
	memory->writed(image + 56, blueMask);
}

void XImage::read(KMemory* memory, U32 address, XImage* image) {
	image->width = memory->readd(address);
	image->height = memory->readd(address + 4);
	image->xoffset = memory->readd(address + 8);
	image->format = memory->readd(address + 12);
	image->data = memory->readd(address + 16);
	image->byte_order = memory->readd(address + 20);
	image->bitmap_unit = memory->readd(address + 24);
	image->bitmap_bit_order = memory->readd(address + 28);
	image->bitmap_pad = memory->readd(address + 32);
	image->depth = memory->readd(address + 36);
	image->bytes_per_line = memory->readd(address + 40);
	image->bits_per_pixel = memory->readd(address + 44);
	image->red_mask = memory->readd(address + 48);
	image->green_mask = memory->readd(address + 52);
	image->blue_mask = memory->readd(address + 56);
}