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

void Display::iterateVisuals(KThread* thread, U32 displayAddress, std::function<bool(S32 screenIndex, U32 visualAddress, Depth* depth, Visual* visual)> pfn) {
	KMemory* memory = thread->memory;
	S32 nscreens = X11_READD(Display, displayAddress, nscreens);

	U32 screensAddress = X11_READD(Display, displayAddress, screens);
	for (S32 screenIndex = 0; screenIndex < nscreens; screenIndex++) {
		U32 screenAddress = screensAddress + screenIndex * sizeof(Screen);
		S32 ndepths = X11_READD(Screen, screenAddress, ndepths);
		U32 depthsAddress = X11_READD(Screen, screenAddress, depths);

		for (S32 depthIndex = 0; depthIndex < ndepths; depthIndex++) {
			U32 depthAddress = depthsAddress + depthIndex * sizeof(Depth);
			S32 nvisuals = X11_READD(Depth, depthAddress, nvisuals);
			U32 visualsAddress = X11_READD(Depth, depthAddress, visuals);
			Depth depth;
			depth.read(memory, depthAddress);

			for (S32 visualIndex = 0; visualIndex < nvisuals; visualIndex++) {
				U32 visualAddress = visualsAddress + visualIndex * sizeof(Visual);
				Visual visual;
				visual.read(memory, visualAddress);
				if (!pfn(screenIndex, visualAddress, &depth, &visual)) {
					return;
				}
			}
		}
	}
}