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

#ifndef __X_IMAGE_H__
#define __X_IMAGE_H__

struct XImage {
	static void set(KMemory* memory, U32 image, S32 width, S32 height, S32 offset, S32 format, U32 data, S32 bitmapPad, S32 depth, S32 bytesPerLine, S32 bitsPerPixel, U32 redMask, U32 greenMask, U32 blueMask);
	static void read(KMemory* memory, U32 address, XImage* image);

	S32 width, height;		/* size of image */
	S32 xoffset;		/* number of pixels offset in X direction */
	S32 format;			/* XYBitmap, XYPixmap, ZPixmap */
	U32 data; // char*	/* pointer to image data */
	S32 byte_order;		/* data byte order, LSBFirst, MSBFirst */
	S32 bitmap_unit;		/* quant. of scanline 8, 16, 32 */
	S32 bitmap_bit_order;	/* LSBFirst, MSBFirst */
	S32 bitmap_pad;		/* 8, 16, 32 either XY or ZPixmap */
	S32 depth;			/* depth of image */
	S32 bytes_per_line;		/* accelerator to next line */
	S32 bits_per_pixel;		/* bits per pixel (ZPixmap) */
	U32 red_mask;	/* bits in z arrangement */
	U32 green_mask;
	U32 blue_mask;
	XPointer obdata;		/* hook for the object routines to hang on */
	// not used by winex11
	struct funcs {		/* image manipulation routines */
		U32 create_image;
		U32 destroy_image;
		U32 get_pixel;
		U32 put_pixel;
		U32 sub_image;
		U32 add_pixel;
	} f;
};

static_assert(sizeof(XImage) == 88, "emulation expects sizeof(XImage) to be 88");

#endif