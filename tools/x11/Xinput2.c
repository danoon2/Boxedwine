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
#include <X11/extensions/XInput2.h>

#include "X11_def.h"

Bool XIGetClientPointer(Display* dpy, Window win, int* deviceid) {
	CALL_3_R(X11_XI_GET_CLIENT_POINTER, dpy, win, deviceid);
}

void XIFreeDeviceInfo(XIDeviceInfo* info) {
	CALL_1(X11_XI_FREE_DEVICE_INFO, info);
}

XIDeviceInfo* XIQueryDevice(Display* dpy, int deviceid, int* ndevices_return) {
	CALL_3_R(X11_XI_QUERY_DEVICE, dpy, deviceid, ndevices_return);
}

Status XIQueryVersion(Display* dpy, int* major_version_inout, int* minor_version_inout) {
	CALL_3_R(X11_XI_QUERY_VERSION, dpy, major_version_inout, minor_version_inout);
}

int XISelectEvents(Display* dpy, Window win, XIEventMask* masks, int num_masks) {
	CALL_4_R(X11_XI_SELECT_EVENTS, dpy, win, masks, num_masks);
}