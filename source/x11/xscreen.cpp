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

void Screen::read(KMemory* memory, U32 address) {
	ext_data = memory->readd(address);
	display = memory->readd(address + 4);
	root = memory->readd(address + 8);
	width = memory->readd(address + 12);
	height = memory->readd(address + 16);
	mwidth = memory->readd(address + 20);
	mheight = memory->readd(address + 24);
	ndepths = memory->readd(address + 28);
	depths = memory->readd(address + 32);
	root_depth = memory->readd(address + 36);
	root_visual = memory->readd(address + 40);
	default_gc = memory->readd(address + 44);
	cmap = memory->readd(address + 48);
	white_pixel = memory->readd(address + 52);
	black_pixel = memory->readd(address + 56);
	max_maps = memory->readd(address + 60);
	min_maps = memory->readd(address + 64);
	backing_store = memory->readd(address + 68);
	save_unders = memory->readd(address + 72);
	root_input_mask = memory->readd(address + 76);
}