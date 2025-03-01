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

#ifndef __X_CURSOR_H__
#define __X_CURSOR_H__

struct XcursorImages {
	U32	nimage; /* number of images */
	U32 imagesAddress; /* array of XcursorImage pointers */
	U32 nameAddress; /* name used to load images */

	void read(KMemory* memory, U32 address);
	static void write(KMemory* memory, U32 address, U32 nimage, U32 imageAddress);
};

static_assert(sizeof(XcursorImages) == 12, "emulation expects sizeof(XcursorImages) to be 12");

struct XcursorImage {
	U32	    version;	/* version of the image data */
	U32	    size;	/* nominal size for matching */
	U32	    width;	/* actual width */
	U32	    height;	/* actual height */
	U32	    xhot;	/* hot spot x (must be inside image) */
	U32	    yhot;	/* hot spot y (must be inside image) */
	U32	    delay;	/* animation delay to next frame (ms) */
	U32     pixelsAddress;	/* pointer to pixels */

	void read(KMemory* memory, U32 address);
	static void write(KMemory* memory, U32 address, U32 width, U32 height, U32 pixels);
};

static_assert(sizeof(XcursorImage) == 32, "emulation expects sizeof(XcursorImage) to be 32");

class XCursor {
public:
	XCursor(const XPixmapPtr& pixmap, const XPixmapPtr& mask, const XColor& fg, const XColor& bg, U32 x, U32 y);
	XCursor(U32 shape);

	const U32 id;
	XPixmapPtr pixmap;
	XPixmapPtr mask;
	XColor fg = { 0 };
	XColor bg = { 0 };
	U32 x = 0;
	U32 y = 0;
	U32 shape = 0;
};

typedef std::shared_ptr<XCursor> XCursorPtr;

#endif