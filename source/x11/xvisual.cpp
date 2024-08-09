#include "boxedwine.h"
#include "x11.h"

void Visual::read(KMemory* memory, U32 address) {
	ext_data = memory->readd(address); address += 4;
	visualid = memory->readd(address); address += 4;
	c_class = (S32)memory->readd(address); address += 4;
	red_mask = memory->readd(address); address += 4;
	green_mask = memory->readd(address); address += 4;
	blue_mask = memory->readd(address); address += 4;
	bits_per_rgb = (S32)memory->readd(address); address += 4;
	map_entries = (S32)memory->readd(address);
}

U32 Visual::create(KThread* thread, U32 visualid, U32 c_class, U32 red_mask, U32 green_mask, U32 blue_mask, U32 bits_per_rgb, U32 map_entries) {
	KMemory* memory = thread->memory;
	U32 visualAddress = thread->process->alloc(thread, sizeof(Visual));

	X11_WRITED(Visual, visualAddress, visualid, visualid);
	X11_WRITED(Visual, visualAddress, c_class, c_class);
	X11_WRITED(Visual, visualAddress, red_mask, red_mask);
	X11_WRITED(Visual, visualAddress, green_mask, green_mask);
	X11_WRITED(Visual, visualAddress, blue_mask, blue_mask);
	X11_WRITED(Visual, visualAddress, bits_per_rgb, bits_per_rgb);
	X11_WRITED(Visual, visualAddress, map_entries, map_entries);

	return visualAddress;
}