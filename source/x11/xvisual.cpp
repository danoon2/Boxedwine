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

void Visual::write(KMemory* memory, U32 address) {
	X11_WRITED(Visual, address, visualid, visualid);
	X11_WRITED(Visual, address, c_class, c_class);
	X11_WRITED(Visual, address, red_mask, red_mask);
	X11_WRITED(Visual, address, green_mask, green_mask);
	X11_WRITED(Visual, address, blue_mask, blue_mask);
	X11_WRITED(Visual, address, bits_per_rgb, bits_per_rgb);
	X11_WRITED(Visual, address, map_entries, map_entries);
}