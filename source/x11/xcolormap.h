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

#ifndef __XCOLOR_MAP_H__
#define __XCOLOR_MAP_H__

#define COLOR_ALLOCATED 1
#define COLOR_WRITE 2
#define MAX_COLORMAP_SIZE 256

#define K_RGB(r, g, b) ((((U32)(r)) << 16) | (((U32)(g)) << 8) | (U32)(b))

class XColorMapColor {
public:
	U8 r;
	U8 g;
	U8 b;
	U8 flags;

	BHashTable<U32, U32> uses;
};

class XColorMap {
public:
	XColorMap();

	const U32 id;
	XColorMapColor colors[MAX_COLORMAP_SIZE] = {};
	bool dirty = false;
	U32 nativePixels[MAX_COLORMAP_SIZE];

	void buildCache();
};

typedef std::shared_ptr<XColorMap> XColorMapPtr;

#endif