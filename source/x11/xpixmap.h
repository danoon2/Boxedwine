#ifndef __X_PIXMAP_H__
#define __X_PIXMAP_H__

#define XPixmapPtr std::shared_ptr<XPixmap>

class XPixmap : public XDrawable {
public:
	XPixmap(U32 width, U32 height, U32 depth) : XDrawable(width, height, depth) {}

	int putImage(KThread* thread, const std::shared_ptr<XGC>& gc, XImage* image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height) override;
};

#endif