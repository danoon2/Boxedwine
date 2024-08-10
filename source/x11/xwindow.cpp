#include "boxedwine.h"
#include "x11.h"
#include "knativewindow.h"

void XWindowChanges::read(KMemory* memory, U32 address) {
	x = memory->readd(address);
	y = memory->readd(address + 4);
	width = memory->readd(address + 8);
	height = memory->readd(address + 12);
	border_width = memory->readd(address + 16);
	sibling = memory->readd(address + 20);
	stack_mode = memory->readd(address + 24);
};

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
#ifndef UNALIGNED_MEMORY
	if ((address & K_PAGE_MASK) + sizeof(XSetWindowAttributes) < K_PAGE_SIZE) {
		return (XSetWindowAttributes*)memory->getIntPtr(address, true);
	}
#endif
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

XWindow::XWindow(U32 displayId, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width) : XDrawable(width, height, depth), parent(parent), x(x), y(y), displayId(displayId), c_class(c_class), border_width(border_width) {
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
		XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [=](const DisplayDataPtr& data) {
			XEvent event = {};
			event.type = CreateNotify;
			event.xcreatewindow.parent = parent->id;
			event.xcreatewindow.window = id;
			event.xcreatewindow.x = x;
			event.xcreatewindow.y = y;
			event.xcreatewindow.width = width();
			event.xcreatewindow.height = height();
			event.xcreatewindow.border_width = border_width;
			event.xcreatewindow.serial = data->getNextEventSerial();
			event.xcreatewindow.display = data->displayAddress;
			data->putEvent(event);
			});
	}
}

void XWindow::setAttributes(const DisplayDataPtr& data, XSetWindowAttributes* attributes, U32 valueMask) {
	this->attributes.copyWithMask(attributes, valueMask);
	if (valueMask & CWEventMask) {
		data->setEventMask(id, attributes->event_mask);
	}
}

void XWindow::iterateChildren(std::function<void(const XWindowPtr& child)> callback) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(childrenMutex);
	for (auto& child : children) {
		callback(child.value);
	}
}

void XWindow::iterateMappedChildren(std::function<void(const XWindowPtr& child)> callback) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(childrenMutex);
	for (auto& child : zchildren) {
		callback(child);
	}
}

void XWindow::setTextProperty(KThread* thread, XTextProperty* name, Atom property) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(property, name->encoding, name->format, name->byteLen(thread->memory), name->value);
}

XPropertyPtr XWindow::getProperty(U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	return properties.getProperty(atom);
}

void XWindow::onSetProperty(KThread* thread, U32 atom) {
	if (atom == XA_WM_HINTS) {
		XPropertyPtr prop = properties.getProperty(atom);
		if (prop->length == sizeof(XWMHints)) {
			XWMHints* hints = (XWMHints*)prop->value;
			if (hints->flags & StateHint) {
				if (hints->initial_state == NormalState) {
					mapWindow(KThread::currentThread());
				} else if (hints->initial_state == IconicState) {
					mapWindow(KThread::currentThread());
				} else {
					unmapWindow(KThread::currentThread());
				}
			}
		}
	} else if (atom == _NET_WM_STATE) {
		XPropertyPtr prop = properties.getProperty(atom);
		bool full = prop->contains32(_NET_WM_STATE_FULLSCREEN);

		if (full != isFullScreen) {
			XWindowChanges changes;
			if (full) {
				changes.x = 0;
				changes.y = 0;
				changes.width = KNativeWindow::getNativeWindow()->screenWidth();
				changes.height = KNativeWindow::getNativeWindow()->screenHeight();
				restoreRect.x = x;
				restoreRect.y = y;
				restoreRect.width = width();
				restoreRect.height = height();
				mapWindow(thread);
			} else {
				changes.x = restoreRect.x;
				changes.y = restoreRect.y;
				changes.width = restoreRect.width;
				changes.height = restoreRect.height;
			}
			isFullScreen = full;
			configure(CWX | CWY | CWWidth | CWHeight, &changes);
		}
	}
}

void XWindow::setProperty(KThread* thread, U32 atom, U32 type, U32 format, U32 length, U8* value) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(atom, type, format, length, value);
	onSetProperty(thread, atom);
	XServer::getServer()->iterateEventMask(id, PropertyChangeMask, [=](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xproperty.type = PropertyNotify;
		event.xproperty.atom = atom;
		event.xproperty.display = data->displayAddress;
		event.xproperty.send_event = False;
		event.xproperty.state = PropertyNewValue;
		event.xproperty.window = id;
		event.xproperty.serial = data->getNextEventSerial();
		event.xproperty.time = XServer::getServer()->getEventTime();
		data->putEvent(event);
		});
}

void XWindow::setProperty(KThread* thread, U32 atom, U32 type, U32 format, U32 length, U32 value) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(atom, type, format, length, value);
	onSetProperty(thread, atom);
	XServer::getServer()->iterateEventMask(id, PropertyChangeMask, [=](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xproperty.type = PropertyNotify;
		event.xproperty.atom = atom;
		event.xproperty.display = data->displayAddress;
		event.xproperty.send_event = False;
		event.xproperty.state = PropertyNewValue;
		event.xproperty.window = id;
		event.xproperty.serial = data->getNextEventSerial();
		event.xproperty.time = XServer::getServer()->getEventTime();
		data->putEvent(event);
		});
}

void XWindow::deleteProperty(KThread* thread, U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	XPropertyPtr prop = properties.getProperty(atom);
	properties.deleteProperty(atom);
	if (prop) {
		XServer::getServer()->iterateEventMask(id, PropertyChangeMask, [=](const DisplayDataPtr& data) {
			XEvent event = {};
			event.xproperty.type = PropertyNotify;
			event.xproperty.atom = atom;
			event.xproperty.display = data->displayAddress;
			event.xproperty.send_event = False;
			event.xproperty.state = PropertyDelete;
			event.xproperty.window = id;
			event.xproperty.serial = data->getNextEventSerial();
			event.xproperty.time = XServer::getServer()->getEventTime();
			data->putEvent(event);
			});
	}
}

int XWindow::handleNetWmStatePropertyEvent(KThread* thread, const XEvent& event) {
	U32 count = 0;
	for (U32 i = 1; i < 5; i++) {
		if (event.xclient.data.l[i] <= 1) {
			break;
		}
		count++;
	}
	XPropertyPtr prop = getProperty(event.xclient.message_type);
	if (!prop) {
		if (event.xclient.data.l[0] == 1) {
			setProperty(thread, (U32)event.xclient.message_type, XA_ATOM, 32, count * 4, (U8*)&event.xclient.data.l[1]);
		}
	} else {
		U32* values = (U32*)prop->value;
		U32 existingCount = prop->length / 4;
		U32 removedCount = 0;
		if (event.xclient.data.l[0] == 0) {
			for (U32 i = 0; i < count; i++) {
				for (U32 j = 0; j < existingCount; j++) {
					if (event.xclient.data.l[i + 1] == values[j]) {
						values[j] = 0;
						removedCount++;
					}
				}
			}
			if (removedCount) {
				U32 newCount = existingCount - removedCount;
				if (!newCount) {
					deleteProperty(thread, event.xclient.message_type);
				} else {
					U32* newValues = new U32[newCount];
					U32 index = 0;

					for (U32 i = 0; i < existingCount; i++) {
						if (values[i]) {
							newValues[index] = values[i];
							index++;
						}
					}
					setProperty(thread, (U32)event.xclient.message_type, XA_ATOM, 32, newCount * 4, (U8*)newValues);
					delete[] newValues;
				}
			}
		} else if (event.xclient.data.l[0] == 1) {
			U32* newValues = new U32[existingCount + count];
			U32 addIndex = existingCount;

			memcpy(newValues, values, sizeof(U32) * existingCount);
			for (U32 i = 0; i < count; i++) {
				bool found = false;
				for (U32 j = 0; j < existingCount; j++) {
					if (event.xclient.data.l[i + 1] == values[j]) {
						found = true;
						break;
					}
				}				
				if (!found) {
					newValues[addIndex] = event.xclient.data.l[i + 1];
					addIndex++;
				}
			}
			if (addIndex != existingCount) {
				setProperty(thread, (U32)event.xclient.message_type, XA_ATOM, 32, addIndex * 4, (U8*)newValues);
			}
			delete[] newValues;
		} else if (event.xclient.data.l[0] == 1) {
			kpanic("XWindow::handleNetWmStatePropertyEvent toggle not handled");
		} else {
			return BadValue;
		}
	}
	return Success;
}

void XWindow::exposeNofity(const DisplayDataPtr& data, S32 x, S32 y, S32 width, S32 height, S32 count) {
	XEvent event = {};
	event.xexpose.type = Expose;
	event.xexpose.display = data->displayAddress;
	event.xexpose.send_event = False;
	event.xexpose.window = id;
	event.xexpose.serial = data->getNextEventSerial();
	event.xexpose.x = x;
	event.xexpose.y = y;
	event.xexpose.width = width;
	event.xexpose.height = height;
	event.xexpose.count = count;
	data->putEvent(event);

	iterateChildren([](const XWindowPtr& child) {
		if (child->isMapped && child->c_class == InputOutput) {
			XServer::getServer()->iterateEventMask(child->id, ExposureMask, [=](const DisplayDataPtr& data) {
				child->exposeNofity(data, 0, 0, child->width(), child->height(), 0);
				});
		}
		});
}

void XWindow::setWmState(KThread* thread, U32 state, U32 icon) {
	XServer* server = XServer::getServer();
	U32 wmStateAtom = server->internAtom(B("WM_STATE"), false);
	XWMState wmState;

	wmState.state = state;
	wmState.icon = 0;
	setProperty(thread, wmStateAtom, wmStateAtom, 32, 8, (U8*)&wmState);
}

int XWindow::mapWindow(KThread* thread) {
	if (isMapped) {
		return Success;
	}
	isMapped = true;	
	setWmState(thread, NormalState, 0);
	if (parent) {
		{
			BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parent->childrenMutex);
			parent->zchildren.push_back(shared_from_this());
		}
		XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [=](const DisplayDataPtr& data) {
			XEvent event = {};
			event.xmap.type = MapNotify;
			event.xmap.display = data->displayAddress;
			event.xmap.send_event = False;
			event.xmap.event = parent->id;
			event.xmap.window = id;
			event.xmap.serial = data->getNextEventSerial();
			event.xmap.override_redirect = this->attributes.override_redirect;
			data->putEvent(event);
			});
	}
	XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [=](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xmap.type = MapNotify;
		event.xmap.display = data->displayAddress;
		event.xmap.send_event = False;
		event.xmap.event = id;
		event.xmap.window = id;
		event.xmap.serial = data->getNextEventSerial();
		event.xmap.override_redirect = this->attributes.override_redirect;
		data->putEvent(event);
		});
	if (attributes.backing_store != NotUseful) {
		kwarn("XWindow::mapWindow backing_store was expected to be NotUseful");
	}
	if (c_class == InputOutput) {
		XServer::getServer()->iterateEventMask(id, ExposureMask, [=](const DisplayDataPtr& data) {
			exposeNofity(data, 0, 0, width(), height(), 0);
			});		
	}
	return Success;
}

int XWindow::unmapWindow(KThread* thread) {
	if (!isMapped) {
		return Success;
	}
	isMapped = false;
	setWmState(thread, WithdrawnState, 0);
	if (parent) {
		XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [=](const DisplayDataPtr& data) {
			XEvent event = {};
			event.xmap.type = UnmapNotify;
			event.xmap.display = data->displayAddress;
			event.xmap.send_event = False;
			event.xmap.event = parent->id;
			event.xmap.window = id;
			event.xmap.serial = data->getNextEventSerial();
			event.xmap.override_redirect = this->attributes.override_redirect;
			data->putEvent(event);
			});
	}
	XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [=](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xmap.type = UnmapNotify;
		event.xmap.display = data->displayAddress;
		event.xmap.send_event = False;
		event.xmap.event = id;
		event.xmap.window = id;
		event.xmap.serial = data->getNextEventSerial();
		event.xmap.override_redirect = this->attributes.override_redirect;
		data->putEvent(event);
		});
	return Success;
}

void XWindow::draw() {
	if (c_class == InputOnly || !isMapped) {
		return;
	}

	KNativeWindowPtr nativeWindow = KNativeWindow::getNativeWindow();
	WndPtr wnd = nativeWindow->getWnd(id);
	if (wnd) {
		nativeWindow->putBitsOnWnd(wnd, data, bytes_per_line, x, y, width(), height(), isDirty);
		isDirty = false;
	}
	iterateMappedChildren([](const XWindowPtr& child) {
		child->draw();
		});
}

XWindowPtr XWindow::previousSibling() {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(childrenMutex);
	for (U32 i = 0; i < (U32)zchildren.size(); i++) {
		if (zchildren[i]->id == id) {
			if (i > 0) {
				return zchildren[i - 1];
			}
			return nullptr;
		}
	}
	return nullptr;
}

void XWindow::configureNotify() {
	XWindowPtr prev = previousSibling();
	Drawable above = None;
	if (prev) {
		above = prev->id;
	}

	if (parent) {
		XServer::getServer()->iterateEventMask(parent->id, StructureNotifyMask, [=](const DisplayDataPtr& data) {
			XEvent event = {};
			event.type = ConfigureNotify;
			event.xconfigure.event = parent->id;
			event.xconfigure.window = id;
			event.xconfigure.x = x;
			event.xconfigure.y = y;
			event.xconfigure.width = width();
			event.xconfigure.height = height();
			event.xconfigure.border_width = border_width;
			event.xconfigure.above = above;
			event.xconfigure.override_redirect = attributes.override_redirect;
			event.xconfigure.serial = data->getNextEventSerial();
			event.xconfigure.display = data->displayAddress;
			data->putEvent(event);
			});
	}
	XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [=](const DisplayDataPtr& data) {
		XEvent event = {};
		event.type = ConfigureNotify;
		event.xconfigure.event = id;
		event.xconfigure.window = id;
		event.xconfigure.x = x;
		event.xconfigure.y = y;
		event.xconfigure.width = width();
		event.xconfigure.height = height();
		event.xconfigure.border_width = border_width;
		event.xconfigure.above = above;
		event.xconfigure.override_redirect = attributes.override_redirect;
		event.xconfigure.serial = data->getNextEventSerial();
		event.xconfigure.display = data->displayAddress;
		data->putEvent(event);
		});	
}

int XWindow::configure(U32 mask, XWindowChanges* changes) {
	bool sizeChanged = false;
	U32 width = this->width();
	U32 height = this->height();

	if (mask & CWX) {
		x = changes->x;
		sizeChanged = true;
	}
	if (mask & CWY) {
		y = changes->y;
		sizeChanged = true;
	}
	if (mask & CWWidth) {
		width = changes->width;
		sizeChanged = true;
	}
	if (mask & CWHeight) {
		height = changes->height;
		sizeChanged = true;
	}
	if (mask & CWBorderWidth) {
		border_width = changes->border_width;
	}
	if (mask & CWSibling) {
		kpanic("XReconfigureWMWindow CWSibling not handled");
	}
	if (mask & CWStackMode) {
		if (parent) {
			BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parent->childrenMutex);
			int index = vectorIndexOf(parent->zchildren, shared_from_this());
			if (index >= 0) {
				parent->zchildren.erase(parent->zchildren.begin() + index);
				parent->zchildren.push_back(shared_from_this());
			}
		}
	}
	if (sizeChanged) {
		if (width != this->width() || height != this->height()) {
			setSize(width, height);
		}
		KNativeWindowPtr nativeWindow = KNativeWindow::getNativeWindow();
		WndPtr win = nativeWindow->getWnd(id);
		if (win) {
			win->windowRect.left = x;
			win->windowRect.top = y;
			win->windowRect.right = x + width;
			win->windowRect.bottom = y + height;
		}
	}
	configureNotify();
	return Success;
}