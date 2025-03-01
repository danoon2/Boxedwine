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

void XRectangle::read(KMemory* memory, U32 address) {
	x = (S16)memory->readw(address); address += 2;
	y = (S16)memory->readw(address); address += 2;
	width = memory->readw(address); address += 2;
	height = memory->readw(address);
}

U32 XPixmapFormatValues::write(KMemory* memory, U32 address, U32 depth, U32 bits_per_pixel, U32 scanline_pad) {
	memory->writed(address, depth);
	memory->writed(address+4, bits_per_pixel);
	memory->writed(address+8, scanline_pad);
	return 12;
}