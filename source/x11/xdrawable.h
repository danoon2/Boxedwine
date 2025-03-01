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

#ifndef __X_DRAWABLE_H__
#define __X_DRAWABLE_H__

class XGC;

class XDrawable {
public:
	XDrawable(U32 width, U32 height, U32 depth, const VisualPtr& visual, bool isWindow);
	virtual ~XDrawable();

	const U32 id;

	VisualPtr getVisual() {return visual;}
	int putImage(KThread* thread, const std::shared_ptr<XGC>& gc, XImage* image, S32 src_x, S32 src_y, S32 dest_x, S32 dest_y, U32 width, U32 height);
	int fillRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height);
	int drawRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height);
	int drawLine(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x1, S32 y1, S32 x2, S32 y2);
	int copy(KThread* thread, const std::shared_ptr<XGC>& gc, const std::shared_ptr<XDrawable>& src, S32 srcX, S32 srcY, U32 width, U32 height, S32 dstX, S32 dstY);

	int copyImageData(KThread* thread, const std::shared_ptr<XGC>& gc, U32 data, U32 bytes_per_line, S32 bits_per_pixel, S32 src_x, S32 src_y, S32 dst_x, S32 dst_y, U32 width, U32 height);

	U32 getImage(KThread* thread, S32 x, S32 y, U32 width, U32 height, U32 planeMask, U32 format, U32 redMask, U32 greenMask, U32 blueMask);

	U32 width() { return w; }
	U32 height() { return h; }
	U32 getDepth() { return depth; }
	U32 getBytesPerLine() { return bytes_per_line; }
	S32 getBitsPerPixel() { return visual->bits_per_rgb; }

	void setSize(U32 width, U32 height);
	
	void lockData();
	U8* getData() {return data;}
	U32 getDataSize() {return size;}
	void unlockData();

	virtual void setDirty() {};
	bool isDirty = false;
	const bool isWindow;
	bool isOpenGL = false;

protected:	

	static U32 calculateBytesPerLine(U32 bitsPerPixel, U32 width);
	U32 depth;
	VisualPtr visual;

	U8* data;
	U32 size;
	U32 bytes_per_line;

private:
	BOXEDWINE_MUTEX mutex;
	U32 w;
	U32 h;
};

#endif