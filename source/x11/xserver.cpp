#include "boxedwine.h"
#include "x11.h"
#include "knativewindow.h"
#include "ksocket.h"

std::atomic_int XServer::nextId = 0x10000;
XServer* XServer::server;

XServer* XServer::getServer(bool existingOnly) {
	if (server) {
		return server;
	}
	if (existingOnly) {
		return nullptr;
	}
	BOXEDWINE_CRITICAL_SECTION;
	if (!server) {
		server = new XServer();
	}
	return server;
}

U32 XServer::getNextId() {
	return ++nextId;
}

XServer::XServer() {
	initAtoms();
	extensionXinput2 = internAtom(B("XInputExtension"), false);
}

void XServer::initAtoms() {
	// from https://github.com/aosm/X11/blob/master/xc/include/Xatom.h
	const char* names[] = {
		"NO_ATOM",
		"PRIMARY",
		"SECONDARY",
		"ARC",
		"ATOM",
		"BITMAP",
		"CARDINAL",
		"COLORMAP",
		"CURSOR",
		"CUT_BUFFER0",
		"CUT_BUFFER1",
		"CUT_BUFFER2",
		"CUT_BUFFER3",
		"CUT_BUFFER4",
		"CUT_BUFFER5",
		"CUT_BUFFER6",
		"CUT_BUFFER7",
		"DRAWABLE",
		"FONT",
		"INTEGER",
		"PIXMAP",
		"POINT",
		"RECTANGLE",
		"RESOURCE_MANAGER",
		"RGB_COLOR_MAP",
		"RGB_BEST_MAP",
		"RGB_BLUE_MAP",
		"RGB_DEFAULT_MAP",
		"RGB_GRAY_MAP",
		"RGB_GREEN_MAP",
		"RGB_RED_MAP",
		"STRING",
		"VISUALID",
		"WINDOW",
		"WM_COMMAND",
		"WM_HINTS",
		"WM_CLIENT_MACHINE",
		"WM_ICON_NAME",
		"WM_ICON_SIZE",
		"WM_NAME",
		"WM_NORMAL_HINTS",
		"WM_SIZE_HINTS",
		"WM_ZOOM_HINTS",
		"MIN_SPACE",
		"NORM_SPACE",
		"MAX_SPACE",
		"END_SPACE",
		"SUPERSCRIPT_X",
		"SUPERSCRIPT_Y",
		"SUBSCRIPT_X",
		"SUBSCRIPT_Y",
		"UNDERLINE_POSITION",
		"UNDERLINE_THICKNESS",
		"STRIKEOUT_ASCENT",
		"STRIKEOUT_DESCENT",
		"ITALIC_ANGLE",
		"X_HEIGHT",
		"QUAD_WIDTH",
		"WEIGHT",
		"POINT_SIZE",
		"RESOLUTION",
		"COPYRIGHT",
		"NOTICE",
		"FONT_NAME",
		"FAMILY_NAME",
		"FULL_NAME",
		"CAP_HEIGHT",
		"WM_CLASS",
		"WM_TRANSIENT_FOR",
		NULL
	};
	for (int i = 0; names[i] != NULL; i++) {
		setAtom(B(names[i]), i);
	}
	setAtom(B("_NET_WM_STATE"), _NET_WM_STATE);
	setAtom(B("_NET_WM_STATE_FULLSCREEN"), _NET_WM_STATE_FULLSCREEN);
	setAtom(B("_NET_WM_WINDOW_TYPE"), _NET_WM_WINDOW_TYPE);
	setAtom(B("_NET_WM_WINDOW_TYPE_NORMAL"), _NET_WM_WINDOW_TYPE_NORMAL);
	setAtom(B("_NET_WM_WINDOW_TYPE_DIALOG"), _NET_WM_WINDOW_TYPE_DIALOG);
	setAtom(B("WM_STATE"), WM_STATE);
	setAtom(B("_NET_WM_NAME"), _NET_WM_NAME);
	setAtom(B("_MOTIF_WM_HINTS"), _MOTIF_WM_HINTS);
	setAtom(B("_NET_WM_ICON"), _NET_WM_ICON);
	nextAtomID = 200;
}

void XServer::setAtom(const BString& name, U32 key) {
	atoms.set(key, name);
	reverseAtoms.set(name, key);
}

bool XServer::getAtom(U32 atom, BString& name) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(atomMutex);
	return atoms.get(atom, name);
}

U32 XServer::internAtom(const BString& name, bool onlyIfExists) {
	U32 result;

	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(atomMutex);
	if (reverseAtoms.get(name, result)) {
		return result;
	}
	if (onlyIfExists) {
		return None;
	}
	nextAtomID++;
	result = nextAtomID;
	setAtom(name, result);
	return result;
}

U32 XServer::getNextQuark() {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(quarkMutex);
	nextQuarkID++;
	return nextQuarkID;
}

XWindowPtr XServer::createNewWindow(U32 displayId, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowsMutex);
	XWindowPtr result = std::make_shared<XWindow>(displayId, parent, width, height, depth, x, y, c_class, border_width);
	windows.set(result->id, result);
	result->onCreate();
	return result;
}

U32 XServer::setInputFocus(const DisplayDataPtr& data, U32 window, U32 revertTo, U32 time, bool trace) {
	if (trace && this->trace) {
		BString log;

		log.append(data->displayId, 16);
		log += " SetInputFocus";
		log += " revert-to=";
		log.append(server->inputFocusRevertTo, 16);
		log += " focus=";
		log.append(window, 16);
	}
	inputFocusRevertTo = revertTo;
	XWindowPtr w = getWindow(window);
	
	if (server->inputFocus != w) {
		if (server->inputFocus) {
			server->inputFocus->focusOut();
		}
		server->inputFocus = w;
		if (w) {
			w->focusIn();
		}
	}
	return Success;
}

const XWindowPtr& XServer::getRoot() {
	if (!root) {
		KNativeWindowPtr nativeWindow = KNativeWindow::getNativeWindow();
		root = createNewWindow(0, nullptr, nativeWindow->screenWidth(), nativeWindow->screenHeight(), nativeWindow->screenBpp(), 0, 0, InputOutput, 0);

		U32 rect[] = { 0, 0, (U32)nativeWindow->screenWidth(), (U32)nativeWindow->screenHeight() };
		U32 atom = server->internAtom(B("_GTK_WORKAREAS_D0"), false);
		root->setProperty(nullptr, atom, XA_CARDINAL, 32, sizeof(U32) * 4, (U8*)&rect, false);
		pointerWindow = root;
	}
	return root;
}

void XServer::draw(bool drawNow) {
	static U32 lastDraw;

	if (!isDisplayDirty) {
		return;
	}
	BOXEDWINE_CRITICAL_SECTION;
	if (!isDisplayDirty) {
		return;
	}
	U32 now = KSystem::getMilliesSinceStart();
	if (!drawNow && (now - lastDraw < 16)) {
		return;
	}
	lastDraw = now;
	isDisplayDirty = false;
	
	KNativeWindowPtr nativeWindow = KNativeWindow::getNativeWindow();	
	nativeWindow->runOnUiThread([=]() {
		nativeWindow->clear();
		root->iterateMappedChildrenBackToFront([](XWindowPtr child) {
			if (child->c_class == InputOutput) {
				child->draw();
			}
			return true;
			});
		nativeWindow->present();
		});
}

XWindowPtr XServer::getWindow(U32 window) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowsMutex);
	return windows.get(window);
}

int XServer::destroyWindow(U32 window) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowsMutex);
	XWindowPtr w = windows.get(window);
	if (!w) {
		return BadWindow;
	}
	w->onDestroy();
	windows.remove(window);
	return Success;
}

XPixmapPtr XServer::createNewPixmap(U32 width, U32 height, U32 depth) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pixmapsMutex);
	XPixmapPtr result = std::make_shared<XPixmap>(width, height, depth);
	pixmaps.set(result->id, result);
	return result;
}

XPixmapPtr XServer::getPixmap(U32 pixmap) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pixmapsMutex);
	return pixmaps.get(pixmap);
}

int XServer::removePixmap(U32 pixmap) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pixmapsMutex);
	XPixmapPtr old = pixmaps.get(pixmap);
	if (old) {
		pixmaps.remove(pixmap);
		return Success;
	}
	return BadPixmap;
}

XDrawablePtr XServer::getDrawable(U32 xid) {
	XDrawablePtr result = getPixmap(xid);
	if (result) {
		return result;
	}
	return getWindow(xid);
}

void XServer::addCursor(const XCursorPtr& cursor) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cursorsMutex);
	cursors.set(cursor->id, cursor);
}

XCursorPtr XServer::getCursor(U32 id) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cursorsMutex);
	return cursors.get(id);
}

XGCPtr XServer::createGC(XDrawablePtr drawable) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gcsMutex);
	XGCPtr result = std::make_shared<XGC>(drawable);
	gcs.set(result->id, result);
	return result;
}

XGCPtr XServer::getGC(U32 gc) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gcsMutex);
	return gcs.get(gc);
}

void XServer::removeGC(U32 gc) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gcsMutex);
	gcs.remove(gc);
}

void XServer::iterateEventMask(U32 wndId, U32 mask, std::function<void(const DisplayDataPtr& display)> callback) {
	std::vector<U32> ids;

	{
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);

		for (auto& display : displays) {
			ids.push_back(display.key);
		}
	}
	for (U32& key : ids) {
		DisplayDataPtr data;
		{
			BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
			data = displays.get(key);
		}
		if (data) {
			// don't hold displayMutext when calling this, since the callback will likely lock events
			if (data->getEventMask(wndId) & mask) {
				callback(data);
			}
		}
	}
}

static U32 createScreen(KThread* thread, U32 displayAddress) {
	KMemory* memory = thread->memory;
	XServer* server = XServer::getServer();
	KNativeWindowPtr nativeWindow = KNativeWindow::getNativeWindow();
	U32 defaultVisual = Visual::create(thread, VisualIdBase, TrueColor, 0xFF0000, 0xFF00, 0xFF, 32, 256);
	U32 defaultDepth = Depth::create(thread, 24, defaultVisual, 1);

	U32 screenAddress = thread->process->alloc(thread, sizeof(Screen));
	X11_WRITED(Screen, screenAddress, display, displayAddress);
	X11_WRITED(Screen, screenAddress, width, nativeWindow->screenWidth());
	X11_WRITED(Screen, screenAddress, height, nativeWindow->screenHeight());
	X11_WRITED(Screen, screenAddress, mwidth, (U32)(nativeWindow->screenWidth() * 0.2646));
	X11_WRITED(Screen, screenAddress, mheight, (U32)(nativeWindow->screenHeight() * 0.2646));
	X11_WRITED(Screen, screenAddress, ndepths, 1); // 24 :TODO: do I need 8 and 16 or can I make Wine emulate it
	X11_WRITED(Screen, screenAddress, depths, defaultDepth);
	X11_WRITED(Screen, screenAddress, root_depth, 24);
	X11_WRITED(Screen, screenAddress, root_visual, defaultVisual);
	X11_WRITED(Screen, screenAddress, white_pixel, 0x00FFFFFF);
	X11_WRITED(Screen, screenAddress, black_pixel, 0);
	X11_WRITED(Screen, screenAddress, root, server->getRoot()->id);
	// screen->cmap; // winex11 references this, but maybe not for TrueColor screen?
	
	return screenAddress;
}

int XServer::closeDisplay(KThread* thread, const DisplayDataPtr& data) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
	
	KMemory* memory = thread->memory;
	U32 vendor = X11_READD(Display, data->displayAddress, vendor);
	thread->process->free(vendor);

	U32 screen = X11_READD(Display, data->displayAddress, screens);
	U32 depths = X11_READD(Screen, screen, depths);
	thread->process->free(depths);
	U32 visual = X11_READD(Screen, screen, root_visual);
	thread->process->free(visual);
	thread->process->free(screen);
	thread->process->free(data->displayAddress);
	displays.remove(data->displayId);
	return Success;
}

U32 XServer::openDisplay(KThread* thread) {
	KMemory* memory = thread->memory;
	U32 displayAddress = thread->process->alloc(thread, sizeof(Display));

	U32 fdPairAddress = thread->process->alloc(thread, sizeof(U32) * 2);
	ksocketpair(thread, K_AF_UNIX, K_SOCK_STREAM, 0, fdPairAddress, 0);
	U32 fd1 = memory->readd(fdPairAddress);
	U32 fd2 = memory->readd(fdPairAddress + 4);
	thread->process->free(fdPairAddress);

	X11_WRITED(Display, displayAddress, fd, fd1);
	X11_WRITED(Display, displayAddress, proto_major_version, 11);
	X11_WRITED(Display, displayAddress, proto_minor_version, 4);

	U32 vendor = thread->process->createString(thread, B("Boxedwine.org"));
	X11_WRITED(Display, displayAddress, vendor, vendor);
	X11_WRITED(Display, displayAddress, byte_order, LSBFirst);
	X11_WRITED(Display, displayAddress, bitmap_bit_order, LSBFirst);
	X11_WRITED(Display, displayAddress, release, 1);  // NOT NEEDED /* Until version 1.10.4 rawinput was broken in XOrg, see https://bugs.freedesktop.org/show_bug.cgi?id=30068 */broken_rawevents = strstr(XServerVendor(gdi_display), "X.Org") && XVendorRelease(gdi_display) < 11004000;
	X11_WRITED(Display, displayAddress, request, 1);
	X11_WRITED(Display, displayAddress, display_name, vendor);
	X11_WRITED(Display, displayAddress, nscreens, 1);

	// values from Debian 11 32-bit
	S32 min_keycode = 0;
	S32 max_keycode = 0;
	XKeyboard::getMinMaxKeycodes(min_keycode, max_keycode);
	X11_WRITED(Display, displayAddress, min_keycode, min_keycode);
	X11_WRITED(Display, displayAddress, max_keycode, max_keycode);

	DisplayDataPtr data = std::make_shared<DisplayData>();
	data->displayAddress = displayAddress;	

	U32 screenAddress = createScreen(thread, displayAddress);
	X11_WRITED(Display, displayAddress, screens, screenAddress);
	U32 displayId = XServer::getNextId();
	X11_WRITED(Display, displayAddress, id, displayId);
	data->displayId = displayId;
	data->root = root->id;
	data->clientFd = fd1;
	data->serverFd = fd2;
	data->process = thread->process;

	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
	displays.set(displayId, data);

	return displayAddress;
}

DisplayDataPtr XServer::getDisplayDataByAddressOfDisplay(KMemory* memory, U32 address) {
	U32 id = X11_READD(Display, address, id);
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
	return displays.get(id);
}

DisplayDataPtr XServer::getDisplayDataById(U32 id) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
	return displays.get(id);
}

U32 XServer::getEventTime() {
	return KSystem::getMilliesSinceStart();
}

U32 XServer::getInputModifiers() {
	U32 modifiers = KNativeWindow::getNativeWindow()->getInputModifiers();
	U32 result = 0;

	if (modifiers & NATIVE_LEFT_BUTTON_MASK) {
		result |= Button1Mask;
	}	
	if (modifiers & NATIVE_MIDDLE_BUTTON_MASK) {
		result |= Button2Mask;
	}
	if (modifiers & NATIVE_RIGHT_BUTTON_MASK) {
		result |= Button3Mask;
	}
	if (modifiers & NATIVE_BUTTON_4_MASK) {
		result |= Button4Mask;
	}
	if (modifiers & NATIVE_BUTTON_5_MASK) {
		result |= Button5Mask;
	}
	return result;
}

// https://tronche.com/gui/x/xlib/events/window-entry-exit/normal.html
/*
EnterNotify and LeaveNotify events are generated when the pointer moves from one window to another window.Normal events are identified by XEnterWindowEvent or XLeaveWindowEvent structures whose mode member is set to NotifyNormal.

When the pointer moves from window A to window B and A is an inferior of B, the X server does the following :
  * It generates a LeaveNotify event on window A, with the detail member of the XLeaveWindowEvent structure set to NotifyAncestor.
  * It generates a LeaveNotify event on each window between window A and window B, exclusive, with the detail member of each XLeaveWindowEvent structure set to NotifyVirtual.
  * It generates an EnterNotify event on window B, with the detail member of the XEnterWindowEvent structure set to NotifyInferior.

When the pointer moves from window A to window B and B is an inferior of A, the X server does the following :
  * It generates a LeaveNotify event on window A, with the detail member of the XLeaveWindowEvent structure set to NotifyInferior.
  * It generates an EnterNotify event on each window between window A and window B, exclusive, with the detail member of each XEnterWindowEvent structure set to NotifyVirtual.
  * It generates an EnterNotify event on window B, with the detail member of the XEnterWindowEvent structure set to NotifyAncestor.

When the pointer moves from window A to window B and window C is their least common ancestor, the X server does the following :
  * It generates a LeaveNotify event on window A, with the detail member of the XLeaveWindowEvent structure set to NotifyNonlinear.
  * It generates a LeaveNotify event on each window between window A and window C, exclusive, with the detail member of each XLeaveWindowEvent structure set to NotifyNonlinearVirtual.
  * It generates an EnterNotify event on each window between window C and window B, exclusive, with the detail member of each XEnterWindowEvent structure set to NotifyNonlinearVirtual.
  * It generates an EnterNotify event on window B, with the detail member of the XEnterWindowEvent structure set to NotifyNonlinear.

When the pointer moves from window A to window B on different screens, the X server does the following :
  * It generates a LeaveNotify event on window A, with the detail member of the XLeaveWindowEvent structure set to NotifyNonlinear.
  * If window A is not a root window, it generates a LeaveNotify event on each window above window A up to and including its root, with the detail member of each XLeaveWindowEvent structure set to NotifyNonlinearVirtual.
  * If window B is not a root window, it generates an EnterNotify event on each window from window B's root down to but not including window B, with the detail member of each XEnterWindowEvent structure set to NotifyNonlinearVirtual.
  * It generates an EnterNotify event on window B, with the detail member of the XEnterWindowEvent structure set to NotifyNonlinear.
*/

// First Case: When the pointer moves from window A to window B and B is an inferior of A
void XServer::pointerMovedParentToChild(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode) {
	XServer::getServer()->iterateEventMask(from->id, LeaveWindowMask, [=](const DisplayDataPtr& data) {
		from->crossingNotify(data, false, x, y, mode, NotifyAncestor);
		});

	// gather windows from child to parent (not including child/parent)
	std::vector<XWindowPtr> betweenWindows;
	XWindowPtr betweenWindow = to->getParent();
	while (betweenWindow && betweenWindow->id != from->id) {
		betweenWindows.push_back(betweenWindow);
		betweenWindow = betweenWindow->getParent();
	}

	// now iterate from parent to child (not including child/parent)
	for (auto it = betweenWindows.rbegin(); it != betweenWindows.rend(); ++it) {
		XServer::getServer()->iterateEventMask((*it)->id, LeaveWindowMask, [=](const DisplayDataPtr& data) {
			from->crossingNotify(data, false, x, y, mode, NotifyVirtual);
			});
	}

	XServer::getServer()->iterateEventMask(to->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
		from->crossingNotify(data, true, x, y, mode, NotifyInferior);
		});
}

// Second Case: When the pointer moves from window A to window B and B is an inferior of A
void XServer::pointerMovedChildToParent(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode) {
	XServer::getServer()->iterateEventMask(from->id, LeaveWindowMask, [=](const DisplayDataPtr& data) {
		from->crossingNotify(data, false, x, y, mode, NotifyInferior);
		});

	// iterate from child to parent (not including child/parent)
	XWindowPtr betweenWindow = from->getParent();
	while (betweenWindow && betweenWindow->id != from->id) {
		XServer::getServer()->iterateEventMask(betweenWindow->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
			from->crossingNotify(data, true, x, y, mode, NotifyVirtual);
			});
		betweenWindow = betweenWindow->getParent();
	}
	XServer::getServer()->iterateEventMask(to->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
		from->crossingNotify(data, true, x, y, mode, NotifyInferior);
		});
}

// Third Case: When the pointer moves from window A to window B and window C is their least common ancestor
void XServer::pointerMovedBetweenSiblings(const XWindowPtr& from, const XWindowPtr& to, const XWindowPtr& commonAncestor, S32 x, S32 y, U32 mode) {
	XServer::getServer()->iterateEventMask(from->id, LeaveWindowMask, [=](const DisplayDataPtr& data) {
		from->crossingNotify(data, false, x, y, mode, NotifyNonlinear);
		});

	// iterate from first window to common ancestor (not including child/parent)
	XWindowPtr betweenWindow = from->getParent();
	while (betweenWindow && betweenWindow->id != commonAncestor->id) {
		XServer::getServer()->iterateEventMask(betweenWindow->id, LeaveWindowMask, [=](const DisplayDataPtr& data) {
			from->crossingNotify(data, false, x, y, mode, NotifyNonlinearVirtual);
			});
		betweenWindow = betweenWindow->getParent();
	}

	// gather windows from common ancestor to second window (not including child/parent)
	std::vector<XWindowPtr> betweenWindows;
	betweenWindow = to->getParent();
	while (betweenWindow && betweenWindow->id != commonAncestor->id) {
		betweenWindows.push_back(betweenWindow);
		betweenWindow = betweenWindow->getParent();
	}

	// now iterate from common ancestor to second window (not including child/parent)
	for (auto it = betweenWindows.rbegin(); it != betweenWindows.rend(); ++it) {
		XServer::getServer()->iterateEventMask((*it)->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
			from->crossingNotify(data, true, x, y, mode, NotifyNonlinearVirtual);
			});
	}

	XServer::getServer()->iterateEventMask(to->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
		from->crossingNotify(data, true, x, y, mode, NotifyNonlinear);
		});
}

void XServer::pointerMoved(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode) {
	const XWindowPtr& commonAncestor = from->getLeastCommonAncestor(to);

	if (commonAncestor->id == from->id) {
		pointerMovedParentToChild(from, to, x, y, mode);
	} else if (commonAncestor->id == to->id) {
		pointerMovedChildToParent(from, to, x, y, mode);
	} else {
		pointerMovedBetweenSiblings(from, to, commonAncestor, x, y, mode);
	}
}

int XServer::mapWindow(const DisplayDataPtr& data, const XWindowPtr& window) {
	if (server->trace) {
		BString log;

		log.append(data->displayId, 16);
		log += " MapWindow";
		log += " window=";
		log.append(window->id, 16);
		klog(log.c_str());
	}
	int result = window->mapWindow(data);

	if (result == Success) {
		int x = 0;
		int y = 0;
		KNativeWindow::getNativeWindow()->getMousePos(&x, &y, false);

		XWindowPtr wnd = root->getWindowFromPoint(x, y);
		if (wnd) {
			if (wnd != server->pointerWindow) {
				server->pointerMoved(server->pointerWindow, wnd, x, y, NotifyNormal);
				server->pointerWindow = wnd;
			}
		}
	}
	return result;
}

int XServer::unmapWindow(const DisplayDataPtr& data, const XWindowPtr& window) {
	if (server->trace) {
		BString log;

		log.append(data->displayId, 16);
		log += " UnmapWindow";
		log += " window=";
		log.append(window->id, 16);
		klog(log.c_str());
	}
	int result = window->unmapWindow(data);

	if (result == Success) {
		int x = 0;
		int y = 0;
		KNativeWindow::getNativeWindow()->getMousePos(&x, &y, false);

		XWindowPtr wnd = root->getWindowFromPoint(x, y);
		if (wnd) {
			if (wnd != server->pointerWindow) {
				server->pointerMoved(server->pointerWindow, wnd, x, y, NotifyNormal);
				server->pointerWindow = wnd;
			}
		}
	}
	return result;
}

void XServer::mouseMove(S32 x, S32 y, bool relative) {
	if (grabbed) {
		if (!(grabbedMask & PointerMotionMask)) {
			return;
		}
		grabbed->motionNotify(grabbedDisplay, x, y);
		return;
	}
	XWindowPtr wnd = root->getWindowFromPoint(x, y);
	if (wnd) {
		if (wnd != pointerWindow) {
			pointerMoved(pointerWindow, wnd, x, y, NotifyNormal);
			pointerWindow = wnd;
		}
		wnd->mouseMoveScreenCoords(x, y);
	}
}

void XServer::mouseButton(U32 button, S32 x, S32 y, bool pressed) {
	if (grabbed) {
		U32 mask = pressed ? ButtonPressMask : ButtonReleaseMask;
		if (!(grabbedMask & mask)) {
			return;
		}
		if (grabbedConfined) {
			int ii = 0;
		}
		grabbed->buttonNotify(grabbedDisplay, button, x, y, pressed);
		return;
	}

	XWindowPtr wnd = root->getWindowFromPoint(x, y);
	if (wnd) {
		wnd->mouseButtonScreenCoords(button, x, y, pressed);
	}
}

U32 XServer::grabPointer(const DisplayDataPtr& display, const XWindowPtr& grabbed, XWindowPtr confined, U32 mask, U32 time) {
	if (!time) {
		time = KSystem::getMilliesSinceStart();
	}
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(grabbedMutex);
	if (grabbedDisplay && grabbedDisplay->displayId != display->displayId) {
		return AlreadyGrabbed;
	}
	this->grabbed = grabbed;
	this->grabbedConfined = confined;
	this->grabbedMask = mask;
	this->grabbedTime = time;
	this->grabbedDisplay = display;	

	if (pointerWindow->id != grabbed->id) {
		S32 x = 0;
		S32 y = 0;
		KNativeWindow::getNativeWindow()->getMousePos(&x, &y);
		pointerMoved(pointerWindow, grabbed, x, y, NotifyGrab);
		pointerWindow = grabbed;
	}
	return GrabSuccess;
}

// https://www.x.org/releases/X11R7.6/doc/xproto/x11protocol.html#requests:UngrabPointer
U32 XServer::ungrabPointer(const DisplayDataPtr& display, U32 time) {
	if (!grabbed) {
		return GrabSuccess; // :TODO: couldn't find this use case in the x11 spec
	}
	XWindowPtr prev = grabbed;
	{
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->grabbedMutex);
		this->grabbed = nullptr;
		this->grabbedDisplay = nullptr;
		this->grabbedConfined = nullptr;
	}
	S32 x = 0;
	S32 y = 0;
	KNativeWindow::getNativeWindow()->getMousePos(&x, &y);

	pointerWindow = root->getWindowFromPoint(x, y);
	pointerMoved(prev, pointerWindow, x, y, NotifyUngrab);
	return GrabSuccess;
}