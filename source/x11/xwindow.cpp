#include "boxedwine.h"
#include "x11.h"
#include "xwindow.h"
#include "displaydata.h"

void XKeyEvent::read(KMemory* memory, U32 address) {
	type = (S32)memory->readd(address); address += 4;
	serial = memory->readd(address); address += 4;
	send_event = (S32)memory->readd(address); address += 4;
	display = memory->readd(address); address += 4;
	window = memory->readd(address); address += 4;
	root = memory->readd(address); address += 4;
	subwindow = memory->readd(address); address += 4;
	time = memory->readd(address); address += 4;
	x = (S32)memory->readd(address); address += 4; 
	y = (S32)memory->readd(address); address += 4;
	x_root = (S32)memory->readd(address); address += 4;
	y_root = (S32)memory->readd(address); address += 4;
	state = memory->readd(address); address += 4;
	keycode = memory->readd(address); address += 4;
	same_screen = (S32)memory->readd(address);
}

void XSetWindowAttributes::read(KMemory* memory, U32 address) {
	background_pixmap = memory->readd(address); address += 4;
	background_pixel = memory->readd(address); address += 4;
	border_pixmap = memory->readd(address); address += 4;
	border_pixel = memory->readd(address); address += 4;
	bit_gravity = (S32)memory->readd(address); address += 4;
	win_gravity = (S32)memory->readd(address); address += 4;
	backing_store = (S32)memory->readd(address); address += 4;
	backing_planes = memory->readd(address); address += 4;
	backing_pixel = memory->readd(address); address += 4;
	save_under = (S32)memory->readd(address); address += 4;
	event_mask = (S32)memory->readd(address); address += 4;
	do_not_propagate_mask = (S32)memory->readd(address); address += 4;
	override_redirect = (S32)memory->readd(address); address += 4;
	colormap = memory->readd(address); address += 4;
	cursor = memory->readd(address);
}

XSetWindowAttributes* XSetWindowAttributes::get(KMemory* memory, U32 address, XSetWindowAttributes* tmp) {
	if (!address) {
		return nullptr;
	}
	if ((address & K_PAGE_MASK) + sizeof(XSetWindowAttributes) < K_PAGE_SIZE) {
		return (XSetWindowAttributes*)memory->getIntPtr(address, true);
	}
	tmp->read(memory, address);
	return tmp;
}

#define CWBackPixmap		(1L<<0)
#define CWBackPixel		(1L<<1)
#define CWBorderPixmap		(1L<<2)
#define CWBorderPixel           (1L<<3)
#define CWBitGravity		(1L<<4)
#define CWWinGravity		(1L<<5)
#define CWBackingStore          (1L<<6)
#define CWBackingPlanes	        (1L<<7)
#define CWBackingPixel	        (1L<<8)
#define CWOverrideRedirect	(1L<<9)
#define CWSaveUnder		(1L<<10)
#define CWEventMask		(1L<<11)
#define CWDontPropagate	        (1L<<12)
#define CWColormap		(1L<<13)
#define CWCursor	        (1L<<14)

void XSetWindowAttributes::copyWithMask(XSetWindowAttributes* attributes, U32 valueMask) {
	if (valueMask & CWBackPixmap) {
		background_pixmap = attributes->background_pixmap;
	}
	if (valueMask & CWBackPixel) {
		background_pixel = attributes->background_pixel;
	}
	if (valueMask & CWBorderPixmap) {
		border_pixmap = attributes->border_pixmap;
	}
	if (valueMask & CWBorderPixel) {
		border_pixel = attributes->border_pixel;
	}
	if (valueMask & CWBitGravity) {
		bit_gravity = attributes->bit_gravity;
	}
	if (valueMask & CWWinGravity) {
		win_gravity = attributes->win_gravity;
	}
	if (valueMask & CWBackingStore) {
		backing_store = attributes->backing_store;
	}
	if (valueMask & CWBackingPlanes) {
		backing_planes = attributes->backing_planes;
	}
	if (valueMask & CWBackingPixel) {
		backing_pixel = attributes->backing_pixel;
	}
	if (valueMask & CWSaveUnder) {
		save_under = attributes->save_under;
	}
	if (valueMask & CWEventMask) {
		event_mask = attributes->event_mask;
	}
	if (valueMask & CWDontPropagate) {
		do_not_propagate_mask = attributes->do_not_propagate_mask;
	}
	if (valueMask & CWOverrideRedirect) {
		override_redirect = attributes->override_redirect;
	}
	if (valueMask & CWColormap) {
		colormap = attributes->colormap;
	}
	if (valueMask & CWCursor) {
		cursor = attributes->cursor;
	}
}

XWindow::XWindow(KThread* thread, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width) : XDrawable(width, height, depth), parent(parent), x(x), y(y), c_class(c_class), border_width(border_width), isMapped(false) {
	if (parent) {
		attributes.border_pixmap = parent->attributes.border_pixmap;
		attributes.colormap = parent->attributes.colormap;					
	}
}

void XWindow::onCreate(const XWindowPtr& self) {	
	if (parent) {
		{
			BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parent->childrenMutex);
			parent->children.set(id, self);
		}
		parent->iterateEventMask(SubstructureNotifyMask, [=](Display* display) {
			XEvent event = {};
			event.type = CreateNotify;
			event.xcreatewindow.parent = parent->id;
			event.xcreatewindow.window = id;
			event.xcreatewindow.x = x;
			event.xcreatewindow.y = y;
			event.xcreatewindow.width = width;
			event.xcreatewindow.height = height;
			event.xcreatewindow.border_width = border_width;
			display->data->putEvent(event);
			});
	}
}

void XWindow::setAttributes(KThread* thread, XSetWindowAttributes* attributes, U32 valueMask) {
	this->attributes.copyWithMask(attributes, valueMask);
	if (valueMask & CWEventMask) {
		setEventMask(thread, attributes->event_mask);
	}
}

void XWindow::iterateEventMask(U32 mask, std::function<void(Display* display)> callback) {

}

void XWindow::iterateChildren(std::function<void(const XWindowPtr& child)> callback) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(childrenMutex);
	for (auto& child : children) {
		callback(child.value);
	}
}

U32 XWindow::getEventMask(KThread* thread) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMaskMutex);
	U32 result = 0;
	perProcessEventMask.get(thread->process->id, result);
	return result;
}

void XWindow::setEventMask(KThread* thread, U32 mask) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMaskMutex);
	perProcessEventMask.set(KThread::currentThread()->process->id, mask);
}

void XWindow::setTextProperty(KThread* thread, Display* display, XTextProperty* name, Atom property) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(property, name->encoding, name->format, name->byteLen(thread->memory), name->value);
}

XPropertyPtr XWindow::getProperty(U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	return properties.getProperty(atom);
}

void XWindow::setProperty(KThread* thread, U32 atom, U32 type, U32 format, U32 length, U8* value) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(atom, type, format, length, value);
	iterateEventMask(PropertyChangeMask, [=](Display* display) {
		XEvent event;
		event.xproperty.type = PropertyNotify;
		event.xproperty.atom = atom;
		event.xproperty.display = display->displayAddress;
		event.xproperty.send_event = False;
		event.xproperty.state = PropertyNewValue;
		event.xproperty.window = id;
		event.xproperty.serial = display->getNextEventSerial();
		event.xproperty.time = display->getEventTime();
		display->data->putEvent(event);
		});
}

void XWindow::setProperty(KThread* thread, U32 atom, U32 type, U32 format, U32 length, U32 value) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(atom, type, format, length, value);
	iterateEventMask(PropertyChangeMask, [=](Display* display) {
		XEvent event;
		event.xproperty.type = PropertyNotify;
		event.xproperty.atom = atom;
		event.xproperty.display = display->displayAddress;
		event.xproperty.send_event = False;
		event.xproperty.state = PropertyNewValue;
		event.xproperty.window = id;
		event.xproperty.serial = display->getNextEventSerial();
		event.xproperty.time = display->getEventTime();
		display->data->putEvent(event);
		});
}

void XWindow::deleteProperty(KThread* thread, U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	XPropertyPtr prop = properties.getProperty(atom);
	properties.deleteProperty(atom);
	if (prop) {
		iterateEventMask(PropertyChangeMask, [=](Display* display) {
			XEvent event;
			event.xproperty.type = PropertyNotify;
			event.xproperty.atom = atom;
			event.xproperty.display = display->displayAddress;
			event.xproperty.send_event = False;
			event.xproperty.state = PropertyDelete;
			event.xproperty.window = id;
			event.xproperty.serial = display->getNextEventSerial();
			event.xproperty.time = display->getEventTime();
			display->data->putEvent(event);
			});
	}
}

void XWindow::exposeNofity(Display* display, S32 x, S32 y, S32 width, S32 height, S32 count) {
	XEvent event;
	event.xexpose.type = Expose;
	event.xexpose.display = display->displayAddress;
	event.xexpose.send_event = False;
	event.xexpose.window = id;
	event.xexpose.serial = display->getNextEventSerial();
	event.xexpose.x = x;
	event.xexpose.y = y;
	event.xexpose.width = width;
	event.xexpose.height = height;
	event.xexpose.count = count;
	display->data->putEvent(event);

	iterateChildren([](const XWindowPtr& child) {
		if (child->isMapped && child->c_class == InputOutput) {
			child->iterateEventMask(ExposureMask, [child](Display* display) {
				child->exposeNofity(display, 0, 0, child->width, child->height, 0);
				});
		}
		});
}

int XWindow::mapWindow(KThread* thread) {
	if (isMapped) {
		return Success;
	}
	isMapped = true;
	if (parent) {
		parent->iterateEventMask(SubstructureNotifyMask, [=](Display* display) {
			XEvent event;
			event.xmap.type = MapNotify;
			event.xmap.display = display->displayAddress;
			event.xmap.send_event = False;
			event.xmap.event = parent->id;
			event.xmap.window = id;
			event.xmap.serial = display->getNextEventSerial();
			event.xmap.override_redirect = this->attributes.override_redirect;
			display->data->putEvent(event);
			});
	}
	iterateEventMask(StructureNotifyMask, [=](Display* display) {
		XEvent event;
		event.xmap.type = MapNotify;
		event.xmap.display = display->displayAddress;
		event.xmap.send_event = False;
		event.xmap.event = id;
		event.xmap.window = id;
		event.xmap.serial = display->getNextEventSerial();
		event.xmap.override_redirect = this->attributes.override_redirect;
		display->data->putEvent(event);
		});
	if (attributes.backing_store != NotUseful) {
		kwarn("XWindow::mapWindow backing_store was expected to be NotUseful");
	}
	if (c_class == InputOutput) {
		iterateEventMask(ExposureMask, [=](Display* display) {
			exposeNofity(display, 0, 0, width, height, 0);
			});		
	}
	return Success;
}

int XWindow::unmapWindow(KThread* thread) {
	if (!isMapped) {
		return Success;
	}
	isMapped = false;
	if (parent) {
		parent->iterateEventMask(SubstructureNotifyMask, [=](Display* display) {
			XEvent event;
			event.xmap.type = UnmapNotify;
			event.xmap.display = display->displayAddress;
			event.xmap.send_event = False;
			event.xmap.event = parent->id;
			event.xmap.window = id;
			event.xmap.serial = display->getNextEventSerial();
			event.xmap.override_redirect = this->attributes.override_redirect;
			display->data->putEvent(event);
			});
	}
	iterateEventMask(StructureNotifyMask, [=](Display* display) {
		XEvent event;
		event.xmap.type = UnmapNotify;
		event.xmap.display = display->displayAddress;
		event.xmap.send_event = False;
		event.xmap.event = id;
		event.xmap.window = id;
		event.xmap.serial = display->getNextEventSerial();
		event.xmap.override_redirect = this->attributes.override_redirect;
		display->data->putEvent(event);
		});
	return Success;
}