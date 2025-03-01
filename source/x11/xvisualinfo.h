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

#ifndef __X_VISUAL_INFO_H__
#define __X_VISUAL_INFO_H__

struct XVisualInfo {
	VisualPtrAddress visual;
	VisualID visualid;
	S32 screen;
	S32 depth;
	S32 c_class;
	U32 red_mask;
	U32 green_mask;
	U32 blue_mask;
	S32 colormap_size;
	S32 bits_per_rgb;

	void set(S32 screenIndex, U32 visualAddress, U32 depth, Visual* visual);
	void read(KMemory* memory, U32 address);
	void write(KMemory* memory, U32 address);
	bool match(U32 mask, S32 screenIndex, const Depth* depth, const Visual* visual);
};

static_assert(sizeof(XVisualInfo) == 40, "emulation expects sizeof(XVisualInfo) to be 40");

#endif