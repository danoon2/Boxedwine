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
#include <X11/extensions/Xinerama.h>

#include "X11_def.h"

Bool XineramaQueryExtension(Display* dpy, int* event_base_return, int* error_base_return) {
	return True;
}

XineramaScreenInfo* XineramaQueryScreens(Display* dpy, int* number) {
	CALL_2_R(X11_XINERAMA_QUERY_SCREENS, dpy, number);
}