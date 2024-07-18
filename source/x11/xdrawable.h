#ifndef __X_DRAWABLE_H__
#define __X_DRAWABLE_H__

class XDrawable {
public:
	XDrawable(U32 width, U32 height) : id(nextId++), width(width), height(height) {}
	const U32 id;

private:
	static U32 nextId;

	U32 width;
	U32 height;
};

#endif