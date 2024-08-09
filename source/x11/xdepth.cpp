#include "boxedwine.h"
#include "x11.h"

void Depth::read(KMemory* memory, U32 address) {
	depth = X11_READD(Depth, address, depth);
	nvisuals = X11_READD(Depth, address, nvisuals);
	visuals = X11_READD(Depth, address, visuals);
}

U32 Depth::create(KThread* thread, S32 bpp, U32 visualArrayAddress, S32 visualArrayCount) {
	KMemory* memory = thread->memory;
	U32 depthAddress = thread->process->alloc(thread, sizeof(Depth));

	X11_WRITED(Depth, depthAddress, depth, bpp);
	X11_WRITED(Depth, depthAddress, nvisuals, visualArrayCount);
	X11_WRITED(Depth, depthAddress, visuals, visualArrayAddress);

	return depthAddress;
}