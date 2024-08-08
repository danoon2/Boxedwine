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

void XRectangle::read(KMemory* memory, U32 address) {
	x = (S16)memory->readw(address); address += 2;
	y = (S16)memory->readw(address); address += 2;
	width = memory->readw(address); address += 2;
	height = memory->readw(address);
}

U32 Display::getNextEventSerial() {
	return ++nextEventSerial;
}

U32 Display::getEventTime() {
	return KSystem::getMilliesSinceStart();
}

Display* X11::getProcessDisplay(U32 pid) {
	KProcessPtr process = KSystem::getProcess(pid);
	if (!process) {
		return nullptr;
	}
	return (Display*)process->perProcessData.get(B("XDisplay"));
}

Display* X11::getDisplay(KThread* thread, U32 address) {
	return (Display*)thread->memory->getIntPtr(address, true);
}

U32 XPixmapFormatValues::write(KMemory* memory, U32 address, U32 depth, U32 bits_per_pixel, U32 scanline_pad) {
	memory->writed(address, depth);
	memory->writed(address+4, bits_per_pixel);
	memory->writed(address+8, scanline_pad);
	return 12;
}