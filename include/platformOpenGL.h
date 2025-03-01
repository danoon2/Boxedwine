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

#ifndef __PLATFORM_OPENGL_H__
#define __PLATFORM_OPENGL_H__

#include "pixelformat.h"

class GLPixelFormat {
public:
	U32 id = 0;
	U32 nativeId = 0;

	PixelFormat pf;
	U32 depth = 0;
	U32 bitsPerPixel = 0;
	bool sampleBuffers = false;
	U32 samples = 0;
	bool pbuffer = false;
	U32 pbufferMaxWidth = 0;
	U32 pbufferMaxHeight = 0;
	U32 pbufferMaxPixels = 0;
};

#define PIXEL_FORMAT_NATIVE_INDEX_MASK 0x80000000

typedef std::shared_ptr<GLPixelFormat> GLPixelFormatPtr;

class XWindow;

class PlatformOpenGL {
public:
	static void init(); // run as soon as app starts, will fill out visuals
	static void iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback);
	static GLPixelFormatPtr getFormat(U32 pixelFormatId);
	static bool hardwareListLoaded;
private:
	static BHashTable<U32, GLPixelFormatPtr> formatsById;
	static std::vector<GLPixelFormatPtr> formats;
};

#endif
