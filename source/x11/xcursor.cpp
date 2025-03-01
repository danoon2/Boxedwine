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

void XcursorImage::read(KMemory* memory, U32 address) {
	version = memory->readd(address);
	size = memory->readd(address + 4);
	width = memory->readd(address + 8);
	height = memory->readd(address + 12);
	xhot = memory->readd(address + 16);
	yhot = memory->readd(address + 20);
	delay = memory->readd(address + 24);
	pixelsAddress = memory->readd(address + 28);
}

void XcursorImage::write(KMemory* memory, U32 address, U32 width, U32 height, U32 pixels) {
	memory->writed(address, 1);
	memory->writed(address + 4, 32);
	memory->writed(address + 8, width);
	memory->writed(address + 12, height);
	memory->writed(address + 28, pixels);
}

void XcursorImages::read(KMemory* memory, U32 address) {
	nimage = memory->readd(address);
	imagesAddress = memory->readd(address + 4);
	nameAddress = memory->readd(address + 8);
}

void XcursorImages::write(KMemory* memory, U32 address, U32 nimage, U32 imageAddress) {
	memory->writed(address, nimage);
	memory->writed(address + 4, imageAddress);
}

XCursor::XCursor(const XPixmapPtr& pixmap, const XPixmapPtr& mask, const XColor& fg, const XColor& bg, U32 x, U32 y) : id(XServer::getNextId()), pixmap(pixmap), mask(mask), fg(fg), bg(bg), x(x), y(y) {
}

XCursor::XCursor(U32 shape) : id(XServer::getNextId()), shape(shape) {
}