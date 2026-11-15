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
#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#include "X11_def.h"

XcursorImage* XcursorImageCreate(int width, int height) {
	CALL_2_R(X11_CURSOR_IMAGE_CREATE, width, height);
}

void XcursorImageDestroy(XcursorImage* image) {
	CALL_1(X11_CURSOR_IMAGE_DESTROY, image);
}

Cursor XcursorImageLoadCursor(Display* dpy, const XcursorImage* image) {
	CALL_2_R(X11_CURSOR_IMAGE_LOAD_CURSOR, dpy, image);
}

XcursorImages* XcursorImagesCreate(int size) {
	CALL_1_R(X11_CURSOR_IMAGES_CREATE, size);
}

void XcursorImagesDestroy(XcursorImages* images) {
	CALL_1(X11_CURSOR_IMAGES_DESTROY, images);
}

Cursor XcursorImagesLoadCursor(Display* dpy, const XcursorImages* images) {
	CALL_2_R(X11_CURSOR_IMAGES_LOAD_CURSOR, dpy, images);
}

Cursor XcursorLibraryLoadCursor(Display* dpy, const char* file) {
	CALL_2_R(X11_CURSOR_LIBRARY_LOAD_CURSOR, dpy, file);
}