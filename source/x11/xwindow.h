#ifndef __X_WINDOW_H__
#define __X_WINDOW_H__

#include "xdrawable.h"
#include "xproperties.h"

#define XWindowPtr std::shared_ptr<XWindow>
class XWindow : public XDrawable {
public:
	XWindow(const XWindowPtr& parent, U32 width, U32 height) : XDrawable(width, height), parent(parent) {}

	XProperties properties;
private:
	XWindowPtr parent;
};

#endif