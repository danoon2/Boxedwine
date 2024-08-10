#ifndef __X_DRAWABLE_H__
#define __X_DRAWABLE_H__

class XGC;

class XDrawable {
public:
	XDrawable(U32 width, U32 height, U32 depth);
	virtual ~XDrawable();

	const U32 id;

	int putImage(KThread* thread, const std::shared_ptr<XGC>& gc, XImage* image, S32 src_x, S32 src_y, S32 dest_x, S32 dest_y, U32 width, U32 height);
	int fillRectangle(KThread* thread, const std::shared_ptr<XGC>& gc, S32 x, S32 y, U32 width, U32 height);

	U32 width() { return w; }
	U32 height() { return h; }
	void setSize(U32 width, U32 height);

	bool isDirty = false;
protected:		
	U32 depth;

	U8* data;
	U32 size;
	U32 bytes_per_line;
	U32 bits_per_pixel;

private:
	U32 w;
	U32 h;
};

#endif