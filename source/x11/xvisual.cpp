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

void Visual::read(KMemory* memory, U32 address) {
	ext_data = memory->readd(address); address += 4;
	visualid = memory->readd(address); address += 4;
	c_class = (S32)memory->readd(address); address += 4;
	red_mask = memory->readd(address); address += 4;
	green_mask = memory->readd(address); address += 4;
	blue_mask = memory->readd(address); address += 4;
	bits_per_rgb = (S32)memory->readd(address); address += 4;
	map_entries = (S32)memory->readd(address);
}

void Visual::write(KMemory* memory, U32 address) {
	X11_WRITED(Visual, address, ext_data, ext_data);
	X11_WRITED(Visual, address, visualid, visualid);
	X11_WRITED(Visual, address, c_class, c_class);
	X11_WRITED(Visual, address, red_mask, red_mask);
	X11_WRITED(Visual, address, green_mask, green_mask);
	X11_WRITED(Visual, address, blue_mask, blue_mask);
	X11_WRITED(Visual, address, bits_per_rgb, bits_per_rgb);
	X11_WRITED(Visual, address, map_entries, map_entries);
}