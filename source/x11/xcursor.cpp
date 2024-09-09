#include "boxedwine.h"
#include "x11.h"

XCursor::XCursor(U32 pixmap, U32 mask, const XColor& fg, const XColor& bg, U32 x, U32 y) : id(XServer::getNextId()), pixmap(pixmap), mask(mask), fg(fg), bg(bg), x(x), y(y) {
}

XCursor::XCursor(U32 shape) : id(XServer::getNextId()), shape(shape) {
}