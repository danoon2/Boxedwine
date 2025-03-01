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

void XVisualInfo::read(KMemory* memory, U32 address) {
	this->visual = memory->readd(address); address += 4;
	visualid = memory->readd(address); address += 4;
	screen = (S32)memory->readd(address); address += 4;
	depth = (S32)memory->readd(address); address += 4;
	c_class = (S32)memory->readd(address); address += 4;
	red_mask = memory->readd(address); address += 4;
	green_mask = memory->readd(address); address += 4;
	blue_mask = memory->readd(address); address += 4;
	colormap_size = (S32)memory->readd(address); address += 4;
	bits_per_rgb = (S32)memory->readd(address);
}

void XVisualInfo::write(KMemory* memory, U32 address) {
	memory->writed(address, visual); address += 4;
	memory->writed(address, visualid); address += 4;
	memory->writed(address, screen); address += 4;
	memory->writed(address, depth); address += 4;
	memory->writed(address, c_class); address += 4;
	memory->writed(address, red_mask); address += 4;
	memory->writed(address, green_mask); address += 4;
	memory->writed(address, blue_mask); address += 4;
	memory->writed(address, colormap_size); address += 4;
	memory->writed(address, bits_per_rgb);
}

bool XVisualInfo::match(U32 mask, S32 screenIndex, const Depth* depth, const Visual* visual) {
	if ((mask & VisualIDMask) && this->visualid != visual->visualid) {
		return false;
	}
	if ((mask & VisualScreenMask) && this->screen != screenIndex) {
		return false;
	}
	if ((mask & VisualDepthMask) && this->depth != depth->depth) {
		return false;
	}
	if ((mask & VisualClassMask) && this->c_class != visual->c_class) {
		return false;
	}
	if ((mask & VisualRedMaskMask) && this->red_mask != visual->red_mask) {
		return false;
	}
	if ((mask & VisualGreenMaskMask) && this->green_mask != visual->green_mask) {
		return false;
	}
	if ((mask & VisualBlueMaskMask) && this->blue_mask != visual->blue_mask) {
		return false;
	}
	if ((mask & VisualColormapSizeMask) && this->colormap_size != visual->map_entries) {
		return false;
	}
	if ((mask & VisualBitsPerRGBMask) && this->bits_per_rgb != visual->bits_per_rgb) {
		return false;
	}
	return true;
}

void XVisualInfo::set(S32 screenIndex, U32 visualAddress, U32 depth, Visual* visual) {
	this->screen = screenIndex;
	this->visual = visualAddress;
	this->visualid = visual->visualid;
	this->depth = depth;
	this->c_class = visual->c_class;
	this->red_mask = visual->red_mask;
	this->green_mask = visual->green_mask;
	this->blue_mask = visual->blue_mask;
	this->colormap_size = visual->map_entries;
	this->bits_per_rgb = visual->bits_per_rgb;
}