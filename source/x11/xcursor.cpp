#include "boxedwine.h"
#include "x11.h"

void XcursorImage::read(KMemory* memory, U32 address) {
	version = memory->readd(address);
	size = memory->readd(address + 4);
	width = memory->readd(address + 8);
	height = memory->readd(address + 12);
	xhot = memory->readd(address + 16);
	yhot = memory->readd(address + 20);
	delay = memory->readd(address + 24);
	pixelsAddress = memory->readd(address + 28);
}

void XcursorImage::write(KMemory* memory, U32 address, U32 width, U32 height, U32 pixels) {
	memory->writed(address, 1);
	memory->writed(address + 4, 32);
	memory->writed(address + 8, width);
	memory->writed(address + 12, height);
	memory->writed(address + 28, pixels);
}

void XcursorImages::read(KMemory* memory, U32 address) {
	nimage = memory->readd(address);
	imagesAddress = memory->readd(address + 4);
	nameAddress = memory->readd(address + 8);
}

void XcursorImages::write(KMemory* memory, U32 address, U32 nimage, U32 imageAddress) {
	memory->writed(address, nimage);
	memory->writed(address + 4, imageAddress);
}

XCursor::XCursor(U32 pixmap, U32 mask, const XColor& fg, const XColor& bg, U32 x, U32 y) : id(XServer::getNextId()), pixmap(pixmap), mask(mask), fg(fg), bg(bg), x(x), y(y) {
}

XCursor::XCursor(U32 shape) : id(XServer::getNextId()), shape(shape) {
}