#ifndef __X_DRAWABLE_H__
#define __X_DRAWABLE_H__

#define XDrawablePtr std::shared_ptr<XDrawable>

class XDrawable {
public:
	XDrawable(U32 width, U32 height, U32 depth);
	const U32 id;

protected:	
	U32 width;
	U32 height;
	U32 depth;
};

#endif