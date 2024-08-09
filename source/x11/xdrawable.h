#ifndef __X_DRAWABLE_H__
#define __X_DRAWABLE_H__

class XGC;

class XDrawable {
public:
	XDrawable(U32 width, U32 height, U32 depth);
	virtual ~XDrawable() {}

	const U32 id;

	virtual int putImage(KThread* thread, const std::shared_ptr<XGC>& gc, XImage* image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height) = 0;
protected:	
	U32 width;
	U32 height;
	U32 depth;
};

#endif