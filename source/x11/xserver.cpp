#include "boxedwine.h"
#include "x11.h"
#include "knativesystem.h"
#include "ksocket.h"

#ifdef BOXEDWINE_OPENGL_OSMESA
#include "../../source/opengl/osmesa/osmesa.h"
#endif
#ifdef BOXEDWINE_OPENGL_SDL
#include "../../source/opengl/sdl/sdlgl.h"
#endif

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

void XServer::shutdown() {
	delete server;
	server = nullptr;
}

U32 XServer::getNextId() {
	return ++nextId;
}

XServer::XServer() 
#ifndef BOXEDWINE_MULTI_THREADED
: cond(std::make_shared<BoxedWineCondition>(B("XServer::lockCond")))
#endif
{
	initAtoms();
	initDepths();
	initVisuals();
	visual = visualsByDepth.get(depths.front())->front();
	extensionXinput2 = internAtom(B("XInputExtension"), false);
	extensionGLX = internAtom(B("GLX"), false);
}

VisualPtr XServer::addVisual(U32 redMask, U32 greenMask, U32 blueMask, U32 depth, U32 bitsPerPixel, U32 pixelFormatIndex) {
	VisualPtr visual = std::make_shared<Visual>();
	visual->visualid = getNextId();
	visual->bits_per_rgb = bitsPerPixel;
	visual->red_mask = redMask;
	visual->green_mask = greenMask;
	visual->blue_mask = blueMask;
	visual->c_class = bitsPerPixel <= 8 ? PseudoColor : TrueColor;
	visual->map_entries = 256;
	visual->ext_data = pixelFormatIndex;
	
	if (!visualsByDepth.contains(depth)) {
		visualsByDepth.set(depth, std::make_shared<std::vector<VisualPtr>>());
	}
	visualsByDepth.get(depth)->push_back(visual);
	visuals.set(visual->visualid, visual);
	return visual;
}

void XServer::initDepths() {
	U32 depth = KNativeSystem::getScreen()->screenBpp();
	depths.push_back(depth);

	if (depth != 32) {
		depths.push_back(32);
	}
	if (depth != 24) {
		depths.push_back(24);
	}
	if (depth != 16) {
		depths.push_back(16);
	}	
	if (depth != 15) {
		//depths.push_back(15);
	}
	if (depth != 8) {
		depths.push_back(8);
	}
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
	setAtom(B("CLIPBOARD"), CLIPBOARD);
	setAtom(B("UTF8_STRING"), UTF8_STRING);
	setAtom(B("TARGETS"), TARGETS);
	setAtom(B("EXPORT_CLIPBOARD"), EXPORT_CLIPBOARD);
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

XWindowPtr XServer::createNewWindow(U32 displayId, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width, const VisualPtr& visual) {
	XWindowPtr result;
	{
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(windowsMutex);
		result = std::make_shared<XWindow>(displayId, parent, width, height, depth, x, y, c_class, border_width, visual);
		windows.set(result->id, result);
	}
	result->onCreate();
	return result;
}

U32 XServer::setInputFocus(const DisplayDataPtr& data, U32 window, U32 revertTo, U32 time, bool trace) {
	if (trace && this->trace) {
		BString log;

		log.append(data->displayId, 16);
		log += " SetInputFocus";
		log += " revert-to=";
		log.append(this->inputFocusRevertTo, 16);
		log += " focus=";
		log.append(window, 16);
	}
	inputFocusRevertTo = revertTo;
	XWindowPtr w = getWindow(window);
	
	if (this->inputFocus != w) {
		if (this->inputFocus) {
			this->inputFocus->focusOut();
		}
		this->inputFocus = w;
		if (w) {
			w->focusIn();
		}
	}
	return Success;
}

const XWindowPtr& XServer::getRoot() {
	if (!root) {
		KNativeScreenPtr screen = KNativeSystem::getScreen();

		root = createNewWindow(0, nullptr, screen->screenWidth(), screen->screenHeight(), screen->screenBpp(), 0, 0, InputOutput, 0, visual);

		U32 rect[] = { 0, 0, (U32)screen->screenWidth(), (U32)screen->screenHeight() };
		U32 atom = server->internAtom(B("_GTK_WORKAREAS_D0"), false);
		root->setProperty(atom, XA_CARDINAL, 32, sizeof(U32) * 4, (U8*)&rect, false);
		root->isMapped = true; // so that grab works
		root->colorMap = getDefaultColorMap();
		root->attributes.colormap = root->colorMap->id;
		root->cursor = std::make_shared<XCursor>(132); // XC_top_left_arrow
		addCursor(root->cursor);
		pointerWindow = root;
		initClipboard();		
	}
	return root;
}

void XServer::initClipboard() {
	KNativeScreenPtr screen = KNativeSystem::getScreen();

	selectionWindow = std::make_shared<XWindow>(0, root, 0, 0, 32, 0, 0, InputOnly, 0, visual);
	windows.set(selectionWindow->id, selectionWindow);

	selectionWindow->onPropertyChanged.push_back([this](U32 propAtom) {
		if (propAtom == EXPORT_CLIPBOARD) {
			XPropertyPtr prop = this->selectionWindow->getProperty(propAtom);
			if (!prop) {
				return;
			}
			char* value = new char[prop->length + 1];
			memcpy(value, prop->value, prop->length);
			value[prop->length] = 0;
			KNativeSystem::getScreen()->clipboardSetText(value);
			delete[] value;
			this->selectionWindow->deleteProperty(propAtom);
			return;
		}
	});

	KNativeSystem::getCurrentInput()->onFocusGained.push_back([] {
		XServer* server = XServer::getServer();
		KNativeScreenPtr screen = KNativeSystem::getScreen();
		if (!screen->clipboardIsTextAvailable()) {
			return;
		}
		BString text = screen->clipboardGetText();
		if (server->sdlLastSelection != text) {
			server->sdlLastSelection = text;
			if (!server->selectionOwner) {
				server->selectionOwner = server->selectionWindow->id;
			}
			XWindowPtr owner = server->getWindow(server->selectionOwner);

			if (owner && server->selectionOwner != server->selectionWindow->id) {
				XEvent event;
				DisplayDataPtr data = server->getDisplayDataById(owner->displayId);
				event.type = SelectionClear;
				event.xselectionclear.display = data->displayAddress;
				event.xselectionclear.selection = CLIPBOARD;
				event.xselectionclear.serial = data->getNextEventSerial();
				event.xselectionclear.time = server->getEventTime();
				event.xselectionclear.send_event = False;
				event.xselectionclear.window = server->selectionOwner;
				data->putEvent(event);

				server->selectionOwner = server->selectionWindow->id;
			}
		}
	});
	KNativeSystem::getCurrentInput()->onFocusLost.push_back([] {
		if (server->selectionOwner && server->selectionOwner != server->selectionWindow->id) {
			XWindowPtr owner = server->getWindow(server->selectionOwner);
			XEvent event;
			DisplayDataPtr data = server->getDisplayDataById(owner->displayId);
			event.type = SelectionRequest;
			event.xselectionrequest.display = data->displayAddress;
			event.xselectionrequest.owner = server->selectionOwner;
			event.xselectionrequest.requestor = server->selectionWindow->id;
			event.xselectionrequest.selection = CLIPBOARD;
			event.xselectionrequest.target = UTF8_STRING;
			event.xselectionrequest.property = EXPORT_CLIPBOARD;
			event.xselectionrequest.serial = data->getNextEventSerial();
			event.xselectionrequest.time = server->getEventTime();
			event.xselectionrequest.send_event = False;

			data->putEvent(event);
		}
	});
}

void XServer::draw(bool drawNow) {
	static U32 lastDraw;

	if (!isDisplayDirty || !root) {
		return;
	}
	{
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
	}

	KNativeScreenPtr screen = KNativeSystem::getScreen();
	if (!screen->canBltToScreen()) {
		return;
	}
	screen->getInput()->runOnUiThread([screen, this]() {
		bool childWasDrawn = false;

		screen->clear();
		root->iterateMappedChildrenBackToFront([&childWasDrawn](XWindowPtr child) {
			if (child->c_class == InputOutput) {
				child->draw();
				childWasDrawn = true;
			}
			return true;
			}, true);		
		if (childWasDrawn) {
			screen->present();
		}
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
	if (w->id == grabbedId) {
		ungrabPointer(0);
	}
	if (fakeFullScreenWnd && fakeFullScreenWnd->id == window) {
		fakeFullScreenWnd = nullptr;
	}
	return Success;
}

XPixmapPtr XServer::createNewPixmap(U32 width, U32 height, U32 depth, const VisualPtr& visual) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pixmapsMutex);
	XPixmapPtr result = std::make_shared<XPixmap>(width, height, depth, visual);
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

void XServer::updateCursor(const XWindowPtr& wnd) {
	if (pointerWindow == wnd) {
		KNativeSystem::getScreen()->setCursor(wnd->getCursor());
	}
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

void XServer::iterateInput2Mask(U32 wndId, U32 mask, std::function<void(const DisplayDataPtr& display)> callback) {
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
			if (data->getInput2Mask(wndId) & mask) {
				callback(data);
			}
		}
	}
}

XColorMapPtr XServer::getDefaultColorMap() {
	if (!defaultColorMap) {
		defaultColorMap = std::make_shared<XColorMap>();
		colorMaps.set(defaultColorMap->id, defaultColorMap);
	}
	return defaultColorMap;
}

U32 XServer::createScreen(KThread* thread, U32 displayAddress) {
	KMemory* memory = thread->memory;
	KNativeScreenPtr screen = KNativeSystem::getScreen();

	U32 visualsCount = 0;
	for (auto& visuals : visualsByDepth) {
		visualsCount += (U32)visuals.value->size();
	}
	U32 screenAddress = thread->process->alloc(thread, (U32)(sizeof(Screen) + (sizeof(Depth) * depths.size()) + (sizeof(Visual) * visualsCount)));
	U32 depthVisualAddress = screenAddress + sizeof(Screen);	
	U32 defaultVisualAddress = depthVisualAddress;

	std::vector<Depth> screenDepths;

	for (auto& depth : depths) {
		Depth d;
		d.depth = depth;
		d.nvisuals = (S32)visualsByDepth.get(depth)->size();
		d.visuals = depthVisualAddress;
		screenDepths.push_back(d);

		for (S32 i = 0; i < d.nvisuals; i++) {
			VisualPtr visual = visualsByDepth.get(depth)->at(i);
			visual->write(memory, depthVisualAddress);
			
			depthVisualAddress += sizeof(Visual);
		}
	}
	U32 depthList = depthVisualAddress;
	for (auto& depth : screenDepths) {
		depth.write(memory, depthVisualAddress);
		depthVisualAddress += sizeof(Depth);
	}

	X11_WRITED(Screen, screenAddress, display, displayAddress);
	X11_WRITED(Screen, screenAddress, width, screen->screenWidth());
	X11_WRITED(Screen, screenAddress, height, screen->screenHeight());
	X11_WRITED(Screen, screenAddress, mwidth, (U32)(screen->screenWidth() * 0.2646));
	X11_WRITED(Screen, screenAddress, mheight, (U32)(screen->screenHeight() * 0.2646));
	X11_WRITED(Screen, screenAddress, ndepths, (U32)depths.size());
	X11_WRITED(Screen, screenAddress, depths, depthList);
	X11_WRITED(Screen, screenAddress, root_depth, screenDepths[0].depth);
	X11_WRITED(Screen, screenAddress, root_visual, defaultVisualAddress);
	X11_WRITED(Screen, screenAddress, white_pixel, 0x00FFFFFF);
	X11_WRITED(Screen, screenAddress, black_pixel, 0);
	X11_WRITED(Screen, screenAddress, root, server->getRoot()->id);
	X11_WRITED(Screen, screenAddress, cmap, server->getDefaultColorMap()->id);
	
	return screenAddress;
}

int XServer::closeDisplay(KThread* thread, const DisplayDataPtr& data) {
	if (grabbedDisplayId == data->displayId) {
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->grabbedMutex);
		ungrabPointer(0);
	}
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
	
	KMemory* memory = thread->memory;
	U32 vendor = X11_READD(Display, data->displayAddress, vendor);
	thread->process->free(vendor);

	U32 screen = X11_READD(Display, data->displayAddress, screens);
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
	data->processId = thread->process->id;

	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
	displays.set(displayId, data);

	return displayAddress;
}

void XServer::changeScreen(U32 width, U32 height) {
	KNativeSystem::changeScreenSize(width, height);
	root->moveResize(0, 0, width, height);

	U32 rect[] = { 0, 0, width, height };
	U32 atom = server->internAtom(B("_GTK_WORKAREAS_D0"), false);
	root->setProperty(atom, XA_CARDINAL, 32, sizeof(U32) * 4, (U8*)&rect, false);
}

void XServer::processExit(U32 pid) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(displayMutex);
	std::vector<DisplayDataPtr> processDisplays;

	for (auto& display : displays) {
		if (display.value->processId == pid) {
			processDisplays.push_back(display.value);
		}
	}
	for (auto& display : processDisplays) {
		displays.remove(display->displayId);
	}
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
	U32 modifiers = KNativeSystem::getCurrentInput()->getInputModifiers();
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
		result |= Button8Mask;
	}
	if (modifiers & NATIVE_BUTTON_5_MASK) {
		result |= Button8Mask;
	}
	if (modifiers & NATIVE_SHIFT_MASK) {
		result |= ShiftMask;
	}
	if (modifiers & NATIVE_CAPS_MASK) {
		result |= LockMask;
	}
	if (modifiers & NATIVE_CONTROL_MASK) {
		result |= ControlMask;
	}
	if (modifiers & NATIVE_ALT_MASK) {
		result |= Mod1Mask;
	}
	if (modifiers & NATIVE_NUM_MASK) {
		result |= NumMask;
	}
	if (modifiers & NATIVE_SCROLL_MASK) {
		result |= ScrollMask;
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
			(*it)->crossingNotify(data, false, x, y, mode, NotifyVirtual);
			});
	}

	XServer::getServer()->iterateEventMask(to->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
		to->crossingNotify(data, true, x, y, mode, NotifyInferior);
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
			betweenWindow->crossingNotify(data, true, x, y, mode, NotifyVirtual);
			});
		betweenWindow = betweenWindow->getParent();
	}
	XServer::getServer()->iterateEventMask(to->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
		to->crossingNotify(data, true, x, y, mode, NotifyInferior);
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
			betweenWindow->crossingNotify(data, false, x, y, mode, NotifyNonlinearVirtual);
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
			(*it)->crossingNotify(data, true, x, y, mode, NotifyNonlinearVirtual);
			});
	}

	XServer::getServer()->iterateEventMask(to->id, EnterWindowMask, [=](const DisplayDataPtr& data) {
		to->crossingNotify(data, true, x, y, mode, NotifyNonlinear);
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
	int result = window->mapWindow();

	if (result == Success) {
		int x = 0;
		int y = 0;
		KNativeSystem::getCurrentInput()->getMousePos(&x, &y, false);

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
	if (grabbedId == window->id) {
		ungrabPointer(0);
	}
	int result = window->unmapWindow();

	if (result == Success) {
		int x = 0;
		int y = 0;
		
		KNativeSystem::getCurrentInput()->getMousePos(&x, &y, false);

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
	if (isGrabbed) {
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(grabbedMutex);
		XWindowPtr grabbed = getWindow(grabbedId);
		DisplayDataPtr grabbedDisplay = getDisplayDataById(grabbedDisplayId);

		if (grabbed && grabbedDisplay) {
			if ((grabbedDisplay->getInput2Mask(root->id) & XI_RawMotionMask) && KSystem::forceRelativeMouse) {
				KNativeInputPtr input = KNativeSystem::getCurrentInput();
				S32 midX = input->screenWidth() / 2;
				S32 midY = input->screenHeight() / 2;
				x = x - midX;
				y = y - midY;
				KNativeSystem::warpMouse(midX, midY);
				// :TODO: I'm not really sure how realitive mouse is supposed to work, it was just trial and error with vkQuake
				grabbed->input2Notify(grabbedDisplay, x, y, XI_RawMotion);
				return;
			}
			if (!(grabbedMask & PointerMotionMask)) {
				return;
			}
			if (fakeFullScreenWnd) {
				fakeFullScreenWnd->windowToScreen(x, y);
			}
			grabbed->motionNotify(grabbedDisplay, x, y);
			return;
		} else {
			ungrabPointer(0);
		}
	}
	if (root) { // might not be set at the very start
		XWindowPtr wnd = root->getWindowFromPoint(x, y);
		if (wnd) {
			if (fakeFullScreenWnd) {
				fakeFullScreenWnd->windowToScreen(x, y);
			}
			if (wnd != pointerWindow) {
				pointerMoved(pointerWindow, wnd, x, y, NotifyNormal);
				pointerWindow = wnd;
				KNativeSystem::getScreen()->setCursor(wnd->getCursor());
			}
			while (wnd && !wnd->mouseMoveScreenCoords(x, y)) {
				wnd = wnd->parent;
			}
		}
	}
}

void XServer::mouseButton(U32 button, S32 x, S32 y, bool pressed) {
	if (fakeFullScreenWnd) {
		fakeFullScreenWnd->windowToScreen(x, y);
	}
	if (isGrabbed) {
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(grabbedMutex);
		XWindowPtr grabbed = getWindow(grabbedId);
		DisplayDataPtr grabbedDisplay = getDisplayDataById(grabbedDisplayId);

		if (grabbed && grabbedDisplay) {
			U32 mask = pressed ? ButtonPressMask : ButtonReleaseMask;
			if (!(grabbedMask & mask)) {
				return;
			}
			if ((grabbedDisplay->getInput2Mask(root->id) & XI_RawMotionMask) && KSystem::forceRelativeMouse) {
				KNativeInputPtr input = KNativeSystem::getCurrentInput();
				S32 midX = input->screenWidth() / 2;
				S32 midY = input->screenHeight() / 2;
				x = 0;
				y = 0;
				KNativeSystem::warpMouse(midX, midY);
			}
			grabbed->buttonNotify(grabbedDisplay, button, x, y, pressed);
			return;
		} else {
			ungrabPointer(0);
		}
	}
	if (root) { // might not be set at the very start
		XWindowPtr wnd = root->getWindowFromPoint(x, y);
		while (wnd && !wnd->mouseButtonScreenCoords(button, x, y, pressed)) {
			wnd = wnd->parent;
		}
	}
}

void XServer::key(U32 key, bool pressed) {
	if (inputFocus) {
		S32 x = 0;
		S32 y = 0;
		KNativeSystem::getCurrentInput()->getMousePos(&x, &y);
		inputFocus->keyScreenCoords(key, x, y, pressed);
	}
}

U32 XServer::grabPointer(const DisplayDataPtr& display, const XWindowPtr& grabbed, XWindowPtr confined, U32 mask, U32 time) {
	if (!time) {
		time = KSystem::getMilliesSinceStart();
	}
	bool updateCursor = false;
	{
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(grabbedMutex);

		this->grabbedId = grabbed->id;
		if (confined) {
			this->grabbedConfinedId = confined->id;
		}
		this->grabbedMask = mask;
		this->grabbedTime = time;
		this->grabbedDisplayId = display->displayId;
		this->isGrabbed = true;

		if (pointerWindow->id != grabbed->id) {
			S32 x = 0;
			S32 y = 0;
			KNativeSystem::getCurrentInput()->getMousePos(&x, &y);
			pointerMoved(pointerWindow, grabbed, x, y, NotifyGrab);
			pointerWindow = grabbed;
			updateCursor = true;
		}
	}
	if (updateCursor) {
		// don't call this with grabbedMutex locked
		KNativeSystem::getScreen()->setCursor(pointerWindow->getCursor());
	}
	return GrabSuccess;
}

// https://www.x.org/releases/X11R7.6/doc/xproto/x11protocol.html#requests:UngrabPointer
U32 XServer::ungrabPointer(U32 time) {
	if (!grabbedId) {
		return GrabSuccess; // :TODO: couldn't find this use case in the x11 spec
	}
	XWindowPtr prev = getWindow(grabbedId);
	{
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->grabbedMutex);
		this->grabbedId = 0;
		this->grabbedDisplayId = 0;
		this->grabbedConfinedId = 0;
		this->isGrabbed = false;
	}
	S32 x = 0;
	S32 y = 0;
	KNativeSystem::getCurrentInput()->getMousePos(&x, &y);

	pointerWindow = root->getWindowFromPoint(x, y);
	if (pointerWindow && prev) {
		pointerMoved(prev, pointerWindow, x, y, NotifyUngrab);
		KNativeSystem::getScreen()->setCursor(pointerWindow->getCursor());
	}
	return GrabSuccess;
}

U32 XServer::createColorMap(const Visual* visual, int alloc) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(colorMapMutex);
	XColorMapPtr colorMap = std::make_shared<XColorMap>();
	colorMaps.set(colorMap->id, colorMap);
	return colorMap->id;
}

XColorMapPtr XServer::getColorMap(U32 id) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(colorMapMutex);
	return colorMaps.get(id);
}

void XServer::initVisuals() {	
	// add these first for each depth as a default
	VisualPtr vis32 = addVisual(0xFF0000, 0xFF00, 0xFF, 32, 32, 0);
	VisualPtr vis24 = addVisual(0xFF0000, 0xFF00, 0xFF, 24, 32, 0);
	VisualPtr vis16 = addVisual(0xF800, 0x7E0, 0x1F, 16, 16, 0);
	//VisualPtr vis15 = addVisual(0x7C00, 0x3E0, 0x1F, 15, 16, 0);
	VisualPtr vis8 = addVisual(0, 0, 0, 8, 8, 0);
	 
#ifdef BOXEDWINE_OPENGL_OSMESA
    if (KSystem::openglLib == "osmesa") {
        OsMesaGL::iterateFormats([this, &vis32, &vis16, &vis8](const GLPixelFormatPtr& format) {
            PixelFormat* pf = &format->pf;

            U32 rMask = ((1 << pf->cRedBits) - 1) << pf->cRedShift;
            U32 gMask = ((1 << pf->cGreenBits) - 1) << pf->cGreenShift;
            U32 bMask = ((1 << pf->cBlueBits) - 1) << pf->cBlueShift;
            U32 depth = pf->cColorBits;

            CLXFBConfigPtr cfg = std::make_shared<CLXFBConfig>();

            // try to match a real format with a default
            if (vis32 && rMask == vis32->red_mask && gMask == vis32->green_mask && bMask == vis32->blue_mask && depth == vis32->bits_per_rgb) {
                vis32->ext_data = format->id;
                cfg->visualId = vis32->visualid;
                vis32 = nullptr;
            } else if (vis16 && rMask == vis16->red_mask && gMask == vis16->green_mask && bMask == vis16->blue_mask && depth == vis16->bits_per_rgb) {
                vis16->ext_data = format->id;
                cfg->visualId = vis16->visualid;
                vis16 = nullptr;
            } else if (vis8 && rMask == vis8->red_mask && gMask == vis8->green_mask && bMask == vis8->blue_mask && depth == vis8->bits_per_rgb) {
                vis8->ext_data = format->id;
                cfg->visualId = vis8->visualid;
                vis8 = nullptr;
            } else {
                cfg->visualId = addVisual(rMask, gMask, bMask, format->depth, format->bitsPerPixel, format->id)->visualid;
            }
            cfg->fbId = getNextId();
            cfg->glPixelFormat = format;
            cfg->depth = format->pf.cColorBits;
            fbConfigById.set(cfg->fbId, cfg);
        });
        return;
    } else
#endif

#ifdef BOXEDWINE_OPENGL_SDL
    {
        PlatformOpenGL::init();
        SDLGL::iterateFormats([this, &vis32, &vis24, &vis16, &vis8](const GLPixelFormatPtr& format) {
            PixelFormat* pf = &format->pf;
            
            U32 rMask = ((1 << pf->cRedBits) - 1) << pf->cRedShift;
            U32 gMask = ((1 << pf->cGreenBits) - 1) << pf->cGreenShift;
            U32 bMask = ((1 << pf->cBlueBits) - 1) << pf->cBlueShift;
            U32 depth = pf->cColorBits;
            
            CLXFBConfigPtr cfg = std::make_shared<CLXFBConfig>();
            
            // try to match a real format with a default
            if (vis32 && rMask == vis32->red_mask && gMask == vis32->green_mask && bMask == vis32->blue_mask && depth == vis32->bits_per_rgb) {
                vis32->ext_data = format->id;
                cfg->visualId = vis32->visualid;
                vis32 = nullptr;
            } else if (vis24 && rMask == vis24->red_mask && gMask == vis24->green_mask && bMask == vis24->blue_mask && depth == vis24->bits_per_rgb) {
                vis24->ext_data = format->id;
                cfg->visualId = vis24->visualid;
                vis24 = nullptr;
            } else if (vis16 && rMask == vis16->red_mask && gMask == vis16->green_mask && bMask == vis16->blue_mask && depth == vis16->bits_per_rgb) {
                vis16->ext_data = format->id;
                cfg->visualId = vis16->visualid;
                vis16 = nullptr;
            } else if (vis8 && rMask == vis8->red_mask && gMask == vis8->green_mask && bMask == vis8->blue_mask && depth == vis8->bits_per_rgb) {
                vis8->ext_data = format->id;
                cfg->visualId = vis8->visualid;
                vis8 = nullptr;
            } else {
                cfg->visualId = addVisual(rMask, gMask, bMask, format->depth, format->bitsPerPixel, format->id)->visualid;
            }
            cfg->fbId = getNextId();
            cfg->glPixelFormat = format;
            cfg->depth = format->pf.cColorBits;
            fbConfigById.set(cfg->fbId, cfg);
            if (format->id & PIXEL_FORMAT_NATIVE_INDEX_MASK) {
                fbNativeConfigsSorted.push_back(cfg);
            }
        });
    }
#else
	U32 count = KSystem::getPixelFormatCount();
	for (U32 i = 1; i < count; i++) {
		PixelFormat* pf = KSystem::getPixelFormat(i);

		U32 rMask = ((1 << pf->cRedBits) - 1) << pf->cRedShift;
		U32 gMask = ((1 << pf->cGreenBits) - 1) << pf->cGreenShift;
		U32 bMask = ((1 << pf->cGreenBits) - 1) << pf->cBlueShift;
		U32 depth = pf->cColorBits;

		// try to match a real format with a default
		if (vis32 && rMask == vis32->red_mask && gMask == vis32->green_mask && bMask == vis32->blue_mask && depth == vis32->bits_per_rgb) {
			vis32->ext_data = i;
			vis32 = nullptr;
		} else if (vis16 && rMask == vis16->red_mask && gMask == vis16->green_mask && bMask == vis16->blue_mask && depth == vis16->bits_per_rgb) {
			vis16->ext_data = i;
			vis16 = nullptr;
		} else if (vis8 && rMask == vis8->red_mask && gMask == vis8->green_mask && bMask == vis8->blue_mask && depth == vis8->bits_per_rgb) {
			vis8->ext_data = i;
			vis8 = nullptr;
		} else {
			addVisual(rMask, gMask, bMask, depth, depth, i);
		}
	}
#endif
}

CLXFBConfigPtr XServer::getFbConfig(U32 id) {
	return fbConfigById.get(id);
}

U32 XServer::getFbConfigCount() {
	return (U32)fbConfigById.size();
}

void XServer::iterateFbConfigs(std::function<bool(const CLXFBConfigPtr& cfg)> callback) {
	if (fbNativeConfigsSorted.size()) {
		for (auto& cfg : fbNativeConfigsSorted) {
			if (!callback(cfg)) {
				break;
			}
		}
		return;
	}
	for (auto& cfg : fbConfigById) {
		if (!callback(cfg.value)) {
			break;
		}
	}
}
