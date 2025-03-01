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
#define XUTIL_DEFINE_FUNCTIONS

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XShm.h>

#include "X11_def.h"

void XShapeCombineMask(Display* display, Window dest, int dest_kind, int x_off, int y_off, Pixmap src, int op) {
	CALL_7(X11_SHAPE_COMBINE_MASK, display, dest, dest_kind, x_off, y_off, src, op);
}

void XShapeCombineRectangles(Display* display, Window dest, int dest_kind, int x_off, int y_off, XRectangle* rectangles, int n_rects, int op, int ordering) {
	CALL_9(X11_SHAPE_COMBINE_RECTANGLES, display, dest, dest_kind, x_off, y_off, rectangles, n_rects, op, ordering);
}

void XShapeOffsetShape(Display* display, Window dest, int dest_kind, int x_off, int y_off) {
	CALL_5(X11_SHAPE_OFFSET_SHAPE, display, dest, dest_kind, x_off, y_off);
}

Bool XShmAttach(Display* dpy, XShmSegmentInfo* shminfo) {
	CALL_2_R(X11_SHM_ATTACH, dpy, shminfo);
}

XImage* XShmCreateImage(Display* dpy, Visual* visual, unsigned int depth, int format, char* data, XShmSegmentInfo* shminfo, unsigned int width, unsigned int height) {
	CALL_8_R(X11_SHM_CREATE_IMAGE, dpy, visual, depth, format, data, shminfo, width, height);
}

Bool XShmDetach(Display* dpy, XShmSegmentInfo* shminfo) {
	CALL_2_R(X11_SHM_DETACH, dpy, shminfo);
}

Bool XShmPutImage(Display* dpy, Drawable d, GC gc, XImage* image, int src_x, int src_y, int dst_x, int dst_y, unsigned int src_width, unsigned int src_height, Bool send_event) {
	CALL_11_R(X11_SHM_PUT_IMAGE, dpy, d, gc, image, src_x, src_y, dst_x, dst_y, src_width, src_height, send_event);
}