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

#ifndef __X_VISUAL_H__
#define __X_VISUAL_H__

struct Visual {
	XExtDataPtrAddress ext_data;	/* hook for extension to hang data */
	VisualID visualid;	/* visual id of this visual */
	S32 c_class;		/* class of screen (monochrome, etc.) */
	U32 red_mask, green_mask, blue_mask;	/* mask values */
	S32 bits_per_rgb;	/* log base 2 of distinct color values */
	S32 map_entries;	/* color map entries */

	void read(KMemory* memory, U32 address);
	void write(KMemory* memory, U32 address);
};

typedef std::shared_ptr<Visual> VisualPtr;

#endif