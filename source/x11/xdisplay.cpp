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