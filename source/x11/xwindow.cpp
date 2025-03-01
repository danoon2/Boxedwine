/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "x11.h"
#include "knativesystem.h"

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
		return (XSetWindowAttributes*)memory->getRamPtr(address, sizeof(XSetWindowAttributes), true);
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

XWindow::XWindow(U32 displayId, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width, const VisualPtr& visual) : XDrawable(width, height, depth, visual, true), displayId(displayId), c_class(c_class), parent(parent), left(x), top(y), border_width(border_width) {
	if (parent) {
		attributes.border_pixmap = parent->attributes.border_pixmap;
		attributes.colormap = parent->attributes.colormap;
		colorMap = parent->colorMap;
	}
}

void XWindow::removeFromParent() {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parent->childrenMutex);
	parent->children.remove(id);
	int index = vectorIndexOf(parent->zchildren, shared_from_this());
	if (index >= 0 && index < (int)parent->zchildren.size()) {
		parent->zchildren.erase(parent->zchildren.begin() + index);
	}
}

void XWindow::addToParent() {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parent->childrenMutex);
	parent->children.set(id, shared_from_this());
	parent->zchildren.push_back(shared_from_this());
}

void XWindow::onDestroy() {
	U32 transient = WM_TRANSIENT_FOR();
	if (transient) {
		XWindowPtr transientForWindow = XServer::getServer()->getWindow(transient);
		if (transientForWindow) {
			BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(transientForWindow->childrenMutex);
			int index = vectorIndexOf(transientForWindow->transientChildren, shared_from_this());
			if (index >= 0 && index < (int)transientForWindow->transientChildren.size()) {
				transientForWindow->transientChildren.erase(transientForWindow->transientChildren.begin() + index);
			}
		}
	}
	if (parent) {
		removeFromParent();
		XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [this](const DisplayDataPtr& data) {
			XEvent event = {};
			event.type = DestroyNotify;
			event.xdestroywindow.event = parent->id;
			event.xdestroywindow.window = id;
			event.xcreatewindow.serial = data->getNextEventSerial();
			event.xcreatewindow.display = data->displayAddress;
			data->putEvent(event);

			if (XServer::getServer()->trace) {
				BString log;
				log.append(data->displayId, 16);
				log += " Event";
				log += " DestroyNotify";
				log += " event=";
				log.append(parent->id, 16);
				log += " window=";
				log.append(id, 16);				
				klog(log.c_str());
			}
			});
		// unlike CreateNotify, this can generate StructureNotify
		XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [this](const DisplayDataPtr& data) {
			XEvent event = {};
			event.type = DestroyNotify;
			event.xdestroywindow.event = id;
			event.xdestroywindow.window = id;
			event.xcreatewindow.serial = data->getNextEventSerial();
			event.xcreatewindow.display = data->displayAddress;
			data->putEvent(event);

			if (XServer::getServer()->trace) {
				BString log;
				log.append(data->displayId, 16);
				log += " Event";
				log += " DestroyNotify";
				log += " event=";
				log.append(id, 16);
				log += " window=";
				log.append(id, 16);				
				klog(log.c_str());
			}
			});
	}
}

void XWindow::onCreate() {	
	if (parent) {
		{
			addToParent();
		}
		XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [this](const DisplayDataPtr& data) {
			XEvent event = {};
			event.type = CreateNotify;
			event.xcreatewindow.parent = parent->id;
			event.xcreatewindow.window = id;
			event.xcreatewindow.x = left;
			event.xcreatewindow.y = top;
			event.xcreatewindow.width = width();
			event.xcreatewindow.height = height();
			event.xcreatewindow.border_width = border_width;
			event.xcreatewindow.serial = data->getNextEventSerial();
			event.xcreatewindow.display = data->displayAddress;
			data->putEvent(event);

			if (XServer::getServer()->trace) {
				BString log;
				log.append(data->displayId, 16);
				log += " Event";
				log += " CreateNotify";
				log += " window=";
				log.append(id, 16);
				log += " parent=";
				log.append(parent->id, 16);
				klog(log.c_str());
			}
			});
		// doesn't generate StructureNotifyMask
	}
}

void XWindow::setTransient(U32 w) {
	U32 prev = WM_TRANSIENT_FOR();
	if (prev == w) {
		return;
	}
	if (prev) {
		XWindowPtr previousTransientForWindow = XServer::getServer()->getWindow(prev);
		if (previousTransientForWindow) {
			BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(previousTransientForWindow->childrenMutex);
			int  index = vectorIndexOf(previousTransientForWindow->transientChildren, shared_from_this());
			if (index >= 0 && index < (int)previousTransientForWindow->transientChildren.size()) {
				previousTransientForWindow->transientChildren.erase(previousTransientForWindow->transientChildren.begin() + index);
			}
		}
	}
	setProperty( XA_WM_TRANSIENT_FOR, XA_WINDOW, 32, 4, (U8*)&w, true);
	if (w) {
		XWindowPtr transientForWindow = XServer::getServer()->getWindow(w);
		if (transientForWindow) {
			BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(transientForWindow->childrenMutex);
			transientForWindow->transientChildren.push_back(shared_from_this());	
			transientCached = transientForWindow;
		} else {
			transientCached.reset();
		}
	} else {
		transientCached.reset();
	}
}

int XWindow::setAttributes(const DisplayDataPtr& data, XSetWindowAttributes* attributes, U32 valueMask) {	
	if (valueMask & CWEventMask) {
		// per spec
		// https://tronche.com/gui/x/xlib/window/XChangeWindowAttributes.html
		// Multiple clients can select input on the same window.Their event masks are maintained separately.When an event is generated, it is reported to all interested clients.However, only one client at a time can select for SubstructureRedirectMask ResizeRedirectMask and ButtonPressMask If a client attempts to select any of these event masks and some other client has already selected one, a BadAccess error results.There is only one do - not- propagate - mask for a window, not one per client.
		const U32 mask = ButtonPressMask | SubstructureRedirectMask | ResizeRedirectMask;
		if (attributes->event_mask & mask) {
			bool existing = false;

			XServer::getServer()->iterateEventMask(id, mask, [&existing, data](const DisplayDataPtr& foundData) {
				if (foundData->displayId != data->displayId) {
					existing = true;
				}
				});
			if (existing) {
				return BadAccess;
			}
		}
		data->setEventMask(id, attributes->event_mask);
	}
	if (valueMask & CWColormap) {
		colorMap = XServer::getServer()->getColorMap(attributes->colormap);
	}
	this->attributes.copyWithMask(attributes, valueMask);
	return Success;
}

void XWindow::iterateAllMappedDescendants(std::function<void(const XWindowPtr& child)> callback) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(childrenMutex);
	for (auto& child : zchildren) {
		if (child->mapped()) {
			callback(child);
			child->iterateAllMappedDescendants(callback);
		}
	}
}

void XWindow::iterateMappedChildrenFrontToBack(std::function<bool(const XWindowPtr& child)> callback, bool includeTransients) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(childrenMutex);
	if (includeTransients) {
		for (int i = (int)transientChildren.size() - 1; i >= 0; i--) {
			const XWindowPtr& child = transientChildren.at(i);
			if (child->mapped()) {
				if (!callback(child)) {
					return;
				}
			}
		}
	}
	for (int i = (int)zchildren.size() - 1; i >= 0; i--) {
		const XWindowPtr& child = zchildren.at(i);
		if (child->mapped() && (!includeTransients || !child->isTransient())) {
			if (!callback(child)) {
				break;
			}
		}
	}	
}

void XWindow::iterateMappedChildrenBackToFront(std::function<bool(const XWindowPtr& child)> callback, bool includeTransients) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(childrenMutex);
	for (auto& child : zchildren) {
		if (child->mapped() && (!includeTransients || !child->isTransient())) {
			if (!callback(child)) {
				return;
			}
		}
	}
	if (includeTransients) {
		for (auto& child : transientChildren) {
			if (child->mapped()) {
				if (!callback(child)) {
					break;
				}
			}
		}
	}
}

void XWindow::windowToScreen(S32& x, S32& y) {
	XWindowPtr p = shared_from_this();

	while (p) {
		x += p->left;
		y += p->top;
		p = p->parent;
	}
}

void XWindow::screenToWindow(S32& x, S32& y) {
	XWindowPtr p = shared_from_this();

	while (p) {
		x -= p->left;
		y -= p->top;
		p = p->parent;
	}
}

void XWindow::setTextProperty(KThread* thread, XTextProperty* name, Atom property, bool trace) {
	setProperty(property, name->encoding, name->format, name->byteLen(thread->memory), name->value);
}

XPropertyPtr XWindow::getProperty(U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	return properties.getProperty(atom);
}

void XWindow::setProperty(U32 atom, U32 type, U32 format, U32 length, const U8* value, bool trace) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(atom, type, format, length, value);	

	if (trace && XServer::getServer()->trace) {
		XPropertyPtr prop = properties.getProperty(atom);
		if (prop) {
			BString log;

			log += "ChangeProperty mode=Replace(0x00) window=";
			log.append(id, 16);
			log += " ";
			log += prop->description();
			klog(log.c_str());
		}
	}

	XServer::getServer()->iterateEventMask(id, PropertyChangeMask, [this, atom](const DisplayDataPtr& data) {
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

		if (XServer::getServer()->trace) {
			BString log;
			log.append(data->displayId, 16);
			log += " Event";
			log += " PropertyNotify";
			log += " window=";
			log.append(id, 16);
			log += " atom=";
			log.append(atom, 16);
			log += "(";
			BString name;
			XServer::getServer()->getAtom(atom, name);
			log += name;
			log += ")";
			log += " state=NewValue";
			klog(log.c_str());
		}
		});
	for (auto& callback : onPropertyChanged) {
		callback(atom);
	}
}

void XWindow::setProperty(U32 atom, U32 type, U32 format, U32 length, U32 value, bool trace) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	properties.setProperty(atom, type, format, length, value);
	if (trace && XServer::getServer()->trace) {
		XPropertyPtr prop = getProperty(atom);
		if (prop) {
			BString log;

			log += "ChangeProperty mode=Replace(0x00) window=";
			log.append(id, 16);
			log += " ";
			log += prop->description();
			klog(log.c_str());
		}
	}

	XServer::getServer()->iterateEventMask(id, PropertyChangeMask, [this, atom](const DisplayDataPtr& data) {
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

		if (XServer::getServer()->trace) {
			BString log;
			log.append(data->displayId, 16);
			log += " Event";
			log += " PropertyNotify";
			log += " window=";
			log.append(id, 16);
			log += " atom=";
			log.append(atom, 16);
			log += "(";
			BString name;
			XServer::getServer()->getAtom(atom, name);
			log += name;
			log += ")";
			log += " state=NewValue";
			klog(log.c_str());
		}
		});
	for (auto& callback : onPropertyChanged) {
		callback(atom);
	}
}

void XWindow::deleteProperty(U32 atom, bool trace) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	XPropertyPtr prop = properties.getProperty(atom);
	properties.deleteProperty(atom);
	if (prop) {
		XServer::getServer()->iterateEventMask(id, PropertyChangeMask, [this, atom](const DisplayDataPtr& data) {
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

			if (XServer::getServer()->trace) {
				BString log;
				log.append(data->displayId, 16);
				log += " Event";
				log += " PropertyNotify";
				log += " window=";
				log.append(id, 16);
				log += " atom=";
				log.append(atom, 16);
				log += "(";
				BString name;
				XServer::getServer()->getAtom(atom, name);
				log += name;
				log += ")";
				log += " state=Delete";
				klog(log.c_str());
			}
			});
	}
}

int XWindow::handleNetWmStatePropertyEvent(const XEvent& event) {
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
			setProperty((U32)event.xclient.message_type, XA_ATOM, 32, count * 4, (U8*)&event.xclient.data.l[1]);
		}
	} else {
		U32* values = (U32*)prop->value;
		U32 existingCount = prop->length / 4;
		U32 removedCount = 0;
		if (event.xclient.data.l[0] == 0) {
			for (U32 i = 0; i < count; i++) {
				for (U32 j = 0; j < existingCount; j++) {
					if (event.xclient.data.l[i + 1] == (S32)values[j]) {
						values[j] = 0;
						removedCount++;
					}
				}
			}
			if (removedCount) {
				U32 newCount = existingCount - removedCount;
				if (!newCount) {
					deleteProperty(event.xclient.message_type);
				} else {
					U32* newValues = new U32[newCount];
					U32 index = 0;

					for (U32 i = 0; i < existingCount; i++) {
						if (values[i]) {
							newValues[index] = values[i];
							index++;
						}
					}
					setProperty((U32)event.xclient.message_type, XA_ATOM, 32, newCount * 4, (U8*)newValues);
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
					if (event.xclient.data.l[i + 1] == (S32)values[j]) {
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
				setProperty((U32)event.xclient.message_type, XA_ATOM, 32, addIndex * 4, (U8*)newValues);
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

	if (XServer::getServer()->trace) {
		BString log;
		log.append(data->displayId, 16);
		log += " Event";
		log += " Expose";
		log += " win=";
		log.append(id, 16);
		log += " x=";
		log += x;
		log += " y=";
		log += y;
		log += " width=";
		log += width;
		log += " height=";
		log += height;
		klog(log.c_str());
	}
	iterateMappedChildrenBackToFront([](const XWindowPtr& child) {
		if (child->c_class == InputOutput) {
			XServer::getServer()->iterateEventMask(child->id, ExposureMask, [=](const DisplayDataPtr& data) {
				child->exposeNofity(data, 0, 0, child->width(), child->height(), 0);
				});
		}
		return true;
		});
}

void XWindow::setWmState(U32 state, U32 icon) {
	XWMState wmState;

	wmState.state = state;
	wmState.icon = 0;
	setProperty(WM_STATE, WM_STATE, 32, 8, (U8*)&wmState);
}

int XWindow::mapWindow() {
	if (isMapped) {
		return Success;
	}
	isMapped = true;
	setDirty();
	setWmState(NormalState, 0);
	if (parent) {
		XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [this](const DisplayDataPtr& data) {
			XEvent event = {};
			event.xmap.type = MapNotify;
			event.xmap.display = data->displayAddress;
			event.xmap.send_event = False;
			event.xmap.event = parent->id;
			event.xmap.window = id;
			event.xmap.serial = data->getNextEventSerial();
			event.xmap.override_redirect = this->attributes.override_redirect;
			data->putEvent(event);

			if (XServer::getServer()->trace) {
				BString log;
				log.append(data->displayId, 16);
				log += " Event";
				log += " MapNotify";
				log += " win=";
				log.append(id, 16);
				log += " event=";
				log.append(parent->id, 16);
				klog(log.c_str());
			}
			});
	}
	XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [this](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xmap.type = MapNotify;
		event.xmap.display = data->displayAddress;
		event.xmap.send_event = False;
		event.xmap.event = id;
		event.xmap.window = id;
		event.xmap.serial = data->getNextEventSerial();
		event.xmap.override_redirect = this->attributes.override_redirect;
		data->putEvent(event);

		if (XServer::getServer()->trace) {
			BString log;
			log.append(data->displayId, 16);
			log += " Event";
			log += " MapNotify";
			log += " win=";
			log.append(id, 16);
			log += " event=";
			log.append(id, 16);
		}
		});
	if (attributes.backing_store != NotUseful) {
		kwarn("XWindow::mapWindow backing_store was expected to be NotUseful");
	}
	if (c_class == InputOutput) {
		XServer::getServer()->iterateEventMask(id, ExposureMask, [this](const DisplayDataPtr& data) {
			exposeNofity(data, 0, 0, width(), height(), 0);
			});		
	}
	if (isThisAndAncestorsMapped()) {
		KNativeSystem::showWindow(shared_from_this(), isThisAndAncestorsMapped());
	}
	return Success;
}

int XWindow::unmapWindow() {
	if (!isMapped) {
		return Success;
	}
	isMapped = false;	
	setWmState(WithdrawnState, 0);
	if (parent) {
		setDirty();
		XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [this](const DisplayDataPtr& data) {
			XEvent event = {};
			event.xmap.type = UnmapNotify;
			event.xmap.display = data->displayAddress;
			event.xmap.send_event = False;
			event.xmap.event = parent->id;
			event.xmap.window = id;
			event.xmap.serial = data->getNextEventSerial();
			event.xmap.override_redirect = this->attributes.override_redirect;
			data->putEvent(event);

			if (XServer::getServer()->trace) {
				BString log;
				log.append(data->displayId, 16);
				log += " Event";
				log += " UnmapNotify";
				log += " win=";
				log.append(id, 16);
				log += " event=";
				log.append(parent->id, 16);
			}
			});
	}
	XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [this](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xmap.type = UnmapNotify;
		event.xmap.display = data->displayAddress;
		event.xmap.send_event = False;
		event.xmap.event = id;
		event.xmap.window = id;
		event.xmap.serial = data->getNextEventSerial();
		event.xmap.override_redirect = this->attributes.override_redirect;
		data->putEvent(event);

		if (XServer::getServer()->trace) {
			BString log;
			log.append(data->displayId, 16);
			log += " Event";
			log += " UnmapNotify";
			log += " win=";
			log.append(id, 16);
			log += " event=";
			log.append(id, 16);
		}
		});
	if (c_class == InputOutput && parent) {
		XServer::getServer()->iterateEventMask(parent->id, ExposureMask, [this](const DisplayDataPtr& data) {
			exposeNofity(data, 0, 0, parent->width(), parent->height(), 0);
			});
	}
	if (isThisAndAncestorsMapped()) {
		KNativeSystem::showWindow(shared_from_this(), false);
	}
	return Success;
}

int XWindow::reparentWindow(const XWindowPtr& parent, S32 x, S32 y) {
	if (parent->c_class == InputOnly && c_class != InputOnly) {
		return BadMatch;
	}
	bool needToRemap = false;

	if (isMapped) {
		unmapWindow();
		needToRemap = true;
	}
	removeFromParent();
	this->parent = parent;
	this->left = x;
	this->top = y;
	addToParent();

	XServer::getServer()->iterateEventMask(parent->id, SubstructureNotifyMask, [this, parent, x, y](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xreparent.type = ReparentNotify;
		event.xreparent.display = data->displayAddress;
		event.xreparent.event = parent->id;
		event.xreparent.window = id;
		event.xreparent.parent = parent->id;
		event.xreparent.x = x;
		event.xreparent.y = y;
		event.xreparent.serial = data->getNextEventSerial();
		event.xreparent.override_redirect = this->attributes.override_redirect;
		data->putEvent(event);

		if (XServer::getServer()->trace) {
			BString log;
			log.append(data->displayId, 16);
			log += " Event";
			log += " ReparentNofity";
			log += " event=";
			log.append(parent->id, 16);
			log += " win=";
			log.append(id, 16);
			log += " parent=";
			log.append(parent->id, 16);
			klog(log.c_str());
		}
	});

	XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [this, parent, x, y](const DisplayDataPtr& data) {
		XEvent event = {};
		event.xreparent.type = ReparentNotify;
		event.xreparent.display = data->displayAddress;
		event.xreparent.event = id;
		event.xreparent.window = id;
		event.xreparent.parent = parent->id;
		event.xreparent.x = x;
		event.xreparent.y = y;
		event.xreparent.serial = data->getNextEventSerial();
		event.xreparent.override_redirect = this->attributes.override_redirect;
		data->putEvent(event);

		if (XServer::getServer()->trace) {
			BString log;
			log.append(data->displayId, 16);
			log += " Event";
			log += " ReparentNofity";			
			log += " event=";
			log.append(id, 16);
			log += " win=";
			log.append(id, 16);
			log += " parent=";
			log.append(parent->id, 16);
		}
	});

	if (needToRemap) {
		mapWindow();
	}
	return Success;
}

void XWindow::setDirty() {
	if (!isDirty) {
		isDirty = true;
		XServer::getServer()->isDisplayDirty = true;
	}
}

void XWindow::draw() {
	if (c_class == InputOnly || !isMapped) {
		return;
	}

	U32* palette = nullptr;
	if (colorMap) {
		colorMap->buildCache();
		palette = colorMap->nativePixels;
	}
	S32 screenX = left;
	S32 screenY = top;
	//windowToScreen(screenX, screenY);
	lockData();
	KNativeSystem::getScreen()->putBitsOnWnd(id, data, visual?visual->bits_per_rgb:32, bytes_per_line, screenX, screenY, width(), height(), palette, isDirty);
	unlockData();
	isDirty = false;

	iterateMappedChildrenBackToFront([](const XWindowPtr& child) {
		child->draw();
		return true;
		}, true);
}

XWindowPtr XWindow::previousSibling() {
	if (parent) {
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parent->childrenMutex);
		for (U32 i = 0; i < (U32)parent->zchildren.size(); i++) {
			if (parent->zchildren[i]->id == id) {
				if (i > 0) {
					return parent->zchildren[i - 1];
				}
				return nullptr;
			}
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
		XServer::getServer()->iterateEventMask(parent->id, StructureNotifyMask, [this, above](const DisplayDataPtr& data) {
			XEvent event = {};
			event.type = ConfigureNotify;
			event.xconfigure.event = parent->id;
			event.xconfigure.window = id;
			event.xconfigure.x = left;
			event.xconfigure.y = top;
			event.xconfigure.width = width();
			event.xconfigure.height = height();
			event.xconfigure.border_width = border_width;
			event.xconfigure.above = above;
			event.xconfigure.override_redirect = attributes.override_redirect;
			event.xconfigure.serial = data->getNextEventSerial();
			event.xconfigure.display = data->displayAddress;
			data->putEvent(event);

			if (XServer::getServer()->trace) {
				BString log;
				log.append(data->displayId, 16);
				log += " Event";
				log += " ConfigureNotify";
				log += " win=";
				log.append(id, 16);
				log += " event=";
				log.append(parent->id, 16);
				klog(log.c_str());
			}
			});
	}
	XServer::getServer()->iterateEventMask(id, StructureNotifyMask, [this, above](const DisplayDataPtr& data) {
		XEvent event = {};
		event.type = ConfigureNotify;
		event.xconfigure.event = id;
		event.xconfigure.window = id;
		event.xconfigure.x = left;
		event.xconfigure.y = top;
		event.xconfigure.width = width();
		event.xconfigure.height = height();
		event.xconfigure.border_width = border_width;
		event.xconfigure.above = above;
		event.xconfigure.override_redirect = attributes.override_redirect;
		event.xconfigure.serial = data->getNextEventSerial();
		event.xconfigure.display = data->displayAddress;
		data->putEvent(event);

		if (XServer::getServer()->trace) {
			BString log;
			log.append(data->displayId, 16);
			log += " Event";
			log += " ConfigureNotify";
			log += " win=";
			log.append(id, 16);
			log += " event=";
			log.append(id, 16);
			klog(log.c_str());
		}
		});	
}

int XWindow::moveResize(S32 x, S32 y, U32 width, U32 height) {
	if (this->left == x && this->top == y && this->width() == width && this->height() == height) {
		return Success;
	}
	if (width != this->width() || height != this->height()) {
		setSize(width, height);
	}

	KNativeSystem::moveWindow(shared_from_this());
	
	configureNotify();
	// :TODO:
	// exposeNofity
	return Success;
}

int XWindow::configure(U32 mask, XWindowChanges* changes) {
	bool sizeChanged = false;
	U32 width = this->width();
	U32 height = this->height();

	if (mask & CWX) {
		left = changes->x;
		sizeChanged = true;
	}
	if (mask & CWY) {
		top = changes->y;
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
			if (index >= 0 && index < (int)parent->zchildren.size() - 1) {
				parent->zchildren.erase(parent->zchildren.begin() + index);
				parent->zchildren.push_back(shared_from_this());
			}
		}
	}
	if (sizeChanged) {
		if (width != this->width() || height != this->height()) {
			setSize(width, height);
		}
		KNativeSystem::moveWindow(shared_from_this());
	}
	configureNotify();
	// :TODO:
	if (c_class == InputOutput && isMapped) {
		XServer::getServer()->iterateEventMask(id, ExposureMask, [this](const DisplayDataPtr& data) {
			exposeNofity(data, 0, 0, this->width(), this->height(), 0);
			});
	}
	return Success;
}


bool XWindow::isTransient() {
	XWindowPtr transient = transientCached.lock();
	if (transient) {
		return transient->isThisAndAncestorsMapped();
	}
	return false;
}

bool XWindow::isDialog() {
	U32 type = NET_WM_WINDOW_TYPE();
	if (type == _NET_WM_WINDOW_TYPE_DIALOG || (type == 0 && WM_TRANSIENT_FOR() && !attributes.override_redirect)) {
		return true;
	}
	return false;
}

U32 XWindow::NET_WM_WINDOW_TYPE() {
	XPropertyPtr prop = getProperty(_NET_WM_WINDOW_TYPE);
	if (prop && prop->length) {
		return *(U32*)prop->value;
	}
	return 0;
}

U32 XWindow::WM_TRANSIENT_FOR() {
	XPropertyPtr prop = getProperty(XA_WM_TRANSIENT_FOR);
	if (prop && prop->length) {
		return *(U32*)prop->value;
	}
	return 0;
}

XWindowPtr XWindow::getWindowFromPoint(S32 screenX, S32 screenY) {
	XWindowPtr result;

	iterateMappedChildrenFrontToBack([screenX, screenY, &result](const XWindowPtr& child) {
		if (!child->isMapped) {
			return true;
		}
		S32 x = screenX;
		S32 y = screenY;

		child->parent->screenToWindow(x, y);
		
		bool inChild = x >= child->left && x < child->left + (S32)child->width() && y >= child->top && y < child->top + (S32)child->height();
		XWindowPtr found = child->getWindowFromPoint(screenX, screenY);
		if (found && (inChild || found->isTransient())) {
			result = found;
			return false;
		}
		if ((inChild || child->isTransient())) {
			result = child;
			return false;
		}

		return true;
		}, true);
	if (result) {
		return result;
	}
	return shared_from_this();
}

void XWindow::input2Notify(const DisplayDataPtr& data, S32 dx, S32 dy, U32 type) {
	XEvent event = {};
	event.type = GenericEvent;
	event.xcookie.serial = data->getNextEventSerial();
	event.xcookie.display = data->displayAddress;
	event.xcookie.extension = XServer::getServer()->getExtensionInput2();
	event.xcookie.evtype = XI_RawMotion;
	event.xcookie.cookie = 0;
	event.xcookie.data = XServer::getServer()->getEventTime();

	XIRawEvent raw = {};
	raw.type = GenericEvent;
	raw.deviceid = XI_DEVICE_ID;
	raw.displayAddress = data->displayAddress;
	raw.evtype = XI_RawMotion;
	raw.extension = event.xcookie.extension;
	// raw.detail
	// raw.flags; // not used in wine?
	// raw.raw_values; // not used in wine
	raw.serial = event.xcookie.serial;
	raw.time = XServer::getServer()->getEventTime();
	raw.valuators.mask_len = 1;
	raw.valuators.maskAddress = dx;
	raw.valuators.valuesAddress = dy;
	event.xcookie.cookie = 0x3; // mask

	// event.xcookie.data will be set to raw in x11_GetEventData
	raw.serialize((U32*)&(event.pad[8])); // XGenericEventCookie is 8 U32's long

	data->putEvent(event);
}

void XWindow::motionNotify(const DisplayDataPtr& data, S32 x, S32 y) {
	S32 window_x = x;
	S32 window_y = y;
	screenToWindow(window_x, window_y);

	// winex11 doesn't seem to use subwindow
	XEvent event = {};
	event.type = MotionNotify;
	event.xmotion.serial = data->getNextEventSerial();
	event.xmotion.display = data->displayAddress;
	event.xmotion.window = id;
	event.xmotion.root = data->root;
	event.xmotion.subwindow = 0;
	event.xmotion.time = XServer::getServer()->getEventTime();
	event.xmotion.x = window_x;
	event.xmotion.y = window_y;
	event.xmotion.x_root = x;
	event.xmotion.y_root = y;
	event.xmotion.state = XServer::getServer()->getInputModifiers();
	event.xmotion.is_hint = NotifyNormal;
	event.xmotion.same_screen = True;
	data->putEvent(event);
}

bool XWindow::mouseMoveScreenCoords(S32 x, S32 y) {		
	bool found = false;

	XServer::getServer()->iterateInput2Mask(id, XI_RawMotionMask, [&found, this, x, y](const DisplayDataPtr& data) {
		input2Notify(data, x, y, XI_RawMotionMask);
		found = true;
		});

	if (found) {
		return true;
	}
	XServer::getServer()->iterateEventMask(id, PointerMotionMask, [&found, this, x, y](const DisplayDataPtr& data) {
		motionNotify(data, x, y);
		found = true;
		});
	return found;
}

void XWindow::getAncestorTree(std::vector<XWindowPtr>& ancestors) {
	XWindowPtr w = shared_from_this();
	while (w) {
		ancestors.push_back(w);
		w = w->parent;
	}
}

XWindowPtr XWindow::getLeastCommonAncestor(const XWindowPtr& wnd) {
	if (wnd->parent && wnd->parent->id == id) {
		return wnd->parent;
	}
	if (parent && parent->id == wnd->id) {
		return wnd;
	}
	if (wnd->id == id) {
		return wnd;
	}
	std::vector<XWindowPtr> tree1;
	std::vector<XWindowPtr> tree2;

	getAncestorTree(tree1);
	wnd->getAncestorTree(tree2);

	
	XWindowPtr result = XServer::getServer()->getRoot();
	for (int i = 0; i < (int)tree1.size() && i < (int)tree2.size(); i++) {
		if (tree1.at(i)->id == tree2.at(i)->id) {
			result = tree1.at(i);
		}
	}
	return result;
}

XCursorPtr XWindow::getCursor() {
	if (cursor) {
		return cursor;
	}
	return parent->getCursor();
}

XWindowPtr XWindow::getTopMappedChild() {
	XWindowPtr result;

	iterateMappedChildrenFrontToBack([&result](const XWindowPtr& child) {
		result = child;
		return false;
		});
	return result;
}

bool XWindow::isThisAndAncestorsMapped() {
	XWindowPtr w = shared_from_this();

	while (w) {
		if (!w->isMapped) {
			return false;
		}
		w = w->parent;
	}
	return true;
}

bool XWindow::doesThisOrAncestorHaveFocus() {
	XWindowPtr w = shared_from_this();
	XWindowPtr focus = XServer::getServer()->inputFocus;

	if (!focus) {
		return false;
	}
	while (w) {
		if (w->id == focus->id) {
			return true;
		}
		w = w->parent;
	}
	return false;
}

void XWindow::crossingNotify(const DisplayDataPtr& data, bool in, S32 x, S32 y, S32 mode, S32 detail) {
	S32 window_x = x;
	S32 window_y = y;
	screenToWindow(window_x, window_y);

	XEvent event = {};
	event.type = in ? EnterNotify : LeaveNotify;
	event.xcrossing.serial = data->getNextEventSerial();
	event.xcrossing.display = data->displayAddress;
	event.xcrossing.window = id;
	event.xcrossing.root = data->root;
	event.xcrossing.subwindow = 0;
	event.xcrossing.time = XServer::getServer()->getEventTime();
	event.xcrossing.x = window_x;
	event.xcrossing.y = window_y;
	event.xcrossing.x_root = x;
	event.xcrossing.y_root = y;
	event.xcrossing.mode = mode;
	event.xcrossing.detail = detail;
	event.xcrossing.same_screen = True;
	event.xcrossing.focus = doesThisOrAncestorHaveFocus() ? True : False;
	event.xcrossing.state = XServer::getServer()->getInputModifiers();
	data->putEvent(event);

	if (XServer::getServer()->trace) {
		BString log;
		log.append(data->displayId, 16);
		log += " Event";
		log += in ? " EnterNotify" : "LeaveNotify";
		log += " win=";
		log.append(id, 16);
		klog(log.c_str());
	}
}

void XWindow::focusOut() {
	XServer::getServer()->iterateEventMask(id, FocusChangeMask, [this](const DisplayDataPtr& data) {
		focusNotify(data, false, NotifyNormal, NotifyDetailNone);
		});
}

void XWindow::focusIn() {
	XServer::getServer()->iterateEventMask(id, FocusChangeMask, [this](const DisplayDataPtr& data) {
		focusNotify(data, true, NotifyNormal, NotifyDetailNone);
		});
}

void XWindow::focusNotify(const DisplayDataPtr& data, bool isIn, S32 mode, S32 detail) {
	XEvent event = {};
	event.type = isIn ? FocusIn : FocusOut;
	event.xfocus.serial = data->getNextEventSerial();
	event.xfocus.display = data->displayAddress;
	event.xfocus.window = id;
	event.xfocus.mode = mode;
	event.xfocus.detail = detail;
	data->putEvent(event);

	if (XServer::getServer()->trace) {
		BString log;
		log.append(data->displayId, 16);
		log += " Event";
		log += isIn ? " FocusIn" : "FocusOut";
		log += " win=";
		log.append(id, 16);
		klog(log.c_str());
	}
}

void XWindow::keyNotify(const DisplayDataPtr& data, U32 key, S32 x, S32 y, bool pressed) {
	S32 window_x = x;
	S32 window_y = y;
	screenToWindow(window_x, window_y);

	// winex11 doesn't seem to use subwindow
	XEvent event = {};

	event.type = pressed ? KeyPress : KeyRelease;
	event.xkey.serial = data->getNextEventSerial();
	event.xkey.display = data->displayAddress;
	event.xkey.window = id;
	event.xkey.root = data->root;
	event.xkey.subwindow = 0;
	event.xkey.time = XServer::getServer()->getEventTime();
	event.xkey.x = window_x;
	event.xkey.y = window_y;
	event.xkey.x_root = x;
	event.xkey.y_root = y;
	event.xkey.state = XServer::getServer()->getInputModifiers();
	event.xkey.keycode = key;
	event.xkey.same_screen = True;

	if (XServer::getServer()->trace) {
		BString log;
		log.append(data->displayId, 16);
		log += " Event";
		log += pressed ? " KeyPress" : " KeyRelease";
		log += " key=";
		log.append(key, 16);
		log += " root=";
		log.append(XServer::getServer()->getRoot()->id, 16);
		log += " state=";
		log.append(event.xkey.state, 16);
		klog(log.c_str());
	}

	data->putEvent(event);
}

void XWindow::buttonNotify(const DisplayDataPtr& data, U32 button, S32 x, S32 y, bool pressed) {
	S32 window_x = x;
	S32 window_y = y;
	screenToWindow(window_x, window_y);

	// winex11 doesn't seem to use subwindow
	XEvent event = {};
	event.type = pressed ? ButtonPress : ButtonRelease;
	event.xbutton.serial = data->getNextEventSerial();
	event.xbutton.display = data->displayAddress;
	event.xbutton.window = id;
	event.xbutton.root = data->root;
	event.xbutton.subwindow = 0;
	event.xbutton.time = XServer::getServer()->getEventTime();
	event.xbutton.x = window_x;
	event.xbutton.y = window_y;
	event.xbutton.x_root = x;
	event.xbutton.y_root = y;
	event.xbutton.state = XServer::getServer()->getInputModifiers();
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	data->putEvent(event);

	// The state member is set to indicate the logical state of the pointer buttons and modifier keys just prior to the event
	if (pressed) {
		event.xbutton.state &= ~((Button1Mask) << (button - 1));
	} else {
		event.xbutton.state |= ((Button1Mask) << (button - 1));
	}
	if (XServer::getServer()->trace) {
		BString log;
		log.append(data->displayId, 16);
		log += " Event";
		log += pressed ? " ButtonPress" : " ButtonRelease";
		log += " button=";
		log += button;
		log += " root=";
		log.append(XServer::getServer()->getRoot()->id, 16);
		log += " event=";
		log.append(id, 16);
		klog(log.c_str());
	}
}

bool XWindow::mouseButtonScreenCoords(U32 button, S32 x, S32 y, bool pressed) {
	bool found = false;

	XServer::getServer()->iterateEventMask(id, pressed ? ButtonPressMask : ButtonReleaseMask, [&found, this, button, x, y, pressed](const DisplayDataPtr& data) {
		buttonNotify(data, button, x, y, pressed);
		found = true;
		});
	return found;
}

void XWindow::keyScreenCoords(U32 key, S32 x, S32 y, bool pressed) {

	XServer::getServer()->iterateEventMask(id, pressed ? KeyPressMask : KeyReleaseMask, [this, key, x, y, pressed](const DisplayDataPtr& data) {
		keyNotify(data, key, x, y, pressed);
		});
}
