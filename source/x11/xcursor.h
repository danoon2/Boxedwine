#ifndef __X_CURSOR_H__
#define __X_CURSOR_H__

class XCursor {
public:
	XCursor(U32 pixmap, U32 mask, const XColor& fg, const XColor& bg, U32 x, U32 y);
	XCursor(U32 shape);

	const U32 id;
	U32 pixmap = 0;
	U32 mask = 0;
	XColor fg = { 0 };
	XColor bg = { 0 };
	U32 x = 0;
	U32 y = 0;
	U32 shape = 0;
};

typedef std::shared_ptr<XCursor> XCursorPtr;

#endif