#ifndef __X_CURSOR_H__
#define __X_CURSOR_H__

class XCursor {
public:
	XCursor(U32 pixmap, U32 mask, const XColor& fg, const XColor& bg, U32 x, U32 y);

	const U32 id;
	U32 pixmap;
	U32 mask;
	XColor fg;
	XColor bg;
	U32 x;
	U32 y;
};

typedef std::shared_ptr<XCursor> XCursorPtr;

#endif