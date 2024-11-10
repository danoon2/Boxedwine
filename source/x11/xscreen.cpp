#include "boxedwine.h"
#include "x11.h"

void Screen::read(KMemory* memory, U32 address) {
	ext_data = memory->readd(address);
	display = memory->readd(address + 4);
	root = memory->readd(address + 8);
	width = memory->readd(address + 12);
	height = memory->readd(address + 16);
	mwidth = memory->readd(address + 20);
	mheight = memory->readd(address + 24);
	ndepths = memory->readd(address + 28);
	depths = memory->readd(address + 32);
	root_depth = memory->readd(address + 36);
	root_visual = memory->readd(address + 40);
	default_gc = memory->readd(address + 44);
	cmap = memory->readd(address + 48);
	white_pixel = memory->readd(address + 52);
	black_pixel = memory->readd(address + 56);
	max_maps = memory->readd(address + 60);
	min_maps = memory->readd(address + 64);
	backing_store = memory->readd(address + 68);
	save_unders = memory->readd(address + 72);
	root_input_mask = memory->readd(address + 76);
}