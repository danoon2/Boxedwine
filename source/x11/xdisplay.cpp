#include "boxedwine.h"
#include "x11.h"
#include "displaydata.h"
#include "knativewindow.h"
#include "knativesystem.h"
#include "ksocket.h"
#include "xkeyboard.h"

static U32 createDepth(KMemory* memory, U32& nextAddress, S32 bpp, U32 visualArrayAddress, S32 visualArrayCount) {
	U32 depthAddress = nextAddress;
	Depth* depth = (Depth*)memory->getIntPtr(depthAddress, true);
	nextAddress += sizeof(Depth);

	depth->depth = bpp;
	depth->nvisuals = visualArrayCount;
	depth->visuals = visualArrayAddress;
	return depthAddress;
}

static U32 createVisual(KMemory* memory, U32& nextAddress, U32 visualid, U32 c_class, U32 red_mask, U32 green_mask, U32 blue_mask, U32 bits_per_rgb, U32 map_entries) {
	U32 visualAddress = nextAddress;
	Visual* visual = (Visual*)memory->getIntPtr(visualAddress, true);
	nextAddress += sizeof(Visual);

	visual->visualid = visualid;
	visual->c_class = c_class;
	visual->red_mask = red_mask;
	visual->green_mask = green_mask;
	visual->blue_mask = blue_mask;
	visual->bits_per_rgb = bits_per_rgb;
	visual->map_entries = map_entries;
	return visualAddress;
}

static U32 allocString(KMemory* memory, U32& nextAddress, const char* str) {
	U32 strAddress = nextAddress;
	memory->strcpy(strAddress, str);
	nextAddress += (U32)strlen(str) + 1;
	return strAddress;
}

static U32 createScreen(KThread* thread, U32& nextAddress, Display* display, U32 displayAddress) {
	KMemory* memory = thread->memory;
	std::shared_ptr<KNativeWindow> nativeWindow = KNativeWindow::getNativeWindow();
	U32 defaultVisual = createVisual(memory, nextAddress, VisualIdBase, TrueColor, 0xFF0000, 0xFF00, 0xFF, 32, 256);
	U32 defaultDepth = createDepth(memory, nextAddress, 32, defaultVisual, 1);

	U32 screenAddress = nextAddress;
	Screen* screen = (Screen*)memory->getIntPtr(screenAddress, true);
	nextAddress += sizeof(Screen);

	screen->display = displayAddress;
	screen->width = nativeWindow->screenWidth();
	screen->height = nativeWindow->screenHeight();
	screen->mwidth = (U32)(screen->width * 0.2646);
	screen->mheight = (U32)(screen->height * 0.2646);
	screen->ndepths = 1; // 32 :TODO: do I need 8 and 16 or can I make Wine emulate it
	screen->depths = defaultDepth;
	screen->root_depth = 32;
	screen->root_visual = defaultVisual;
	screen->default_gc = 0;
	// screen->cmap; // winex11 references this, but maybe not for TrueColor screen?
	screen->white_pixel = 0x00FFFFFF;
	screen->black_pixel = 0x0;

	XWindowPtr rootWindow = display->data->createNewWindow(thread, nullptr, screen->width, screen->height, screen->root_depth, 0, 0, InputOutput, 0);
	screen->root = rootWindow->id;

	U32 rect[] = { 0, 0, (U32)screen->width, (U32)screen->height };
	U32 atom = display->data->internAtom(B("_GTK_WORKAREAS_D0"), false);
	rootWindow->setProperty(thread, atom, XA_CARDINAL, 32, sizeof(U32)*4, (U8*)&rect);
	return screenAddress;
}

U32 X11::openDisplay(KThread* thread) {
	Display* display = getCurrentProcessDisplay(thread);
	if (display) {
		display->refCount++;
		return display->displayAddress;
	}
	KMemory* memory = thread->memory;
	U32 displayAddress = memory->mmap(thread, 0, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE, K_MAP_ANONYMOUS | K_MAP_PRIVATE, -1, 0);
	display = (Display*)memory->getIntPtr(displayAddress, true);
	U32 nextAddress = displayAddress + sizeof(Display);

	ksocketpair(thread, K_AF_UNIX, K_SOCK_STREAM, 0, nextAddress, 0);
	U32 fd1 = memory->readd(nextAddress);
	U32 fd2 = memory->readd(nextAddress+4);
	display->fd = fd1;
	display->proto_major_version = 11;
	display->proto_minor_version = 4;
	display->vendor = allocString(memory, nextAddress, "Boxedwine.org");
	display->byte_order = LSBFirst;
	// display->bitmap_unit
	// bitmap_pad
	display->bitmap_bit_order = LSBFirst;
	// nformats
	// pixmap_format
	display->release = 1; // NOT NEEDED /* Until version 1.10.4 rawinput was broken in XOrg, see https://bugs.freedesktop.org/show_bug.cgi?id=30068 */broken_rawevents = strstr(XServerVendor(gdi_display), "X.Org") && XVendorRelease(gdi_display) < 11004000;
	// qlen
	// last_request_read
	display->request = 1;
	// db
	display->display_name = display->vendor;
	display->default_screen = 0;
	display->nscreens = 1;
	// motion_buffer

	// values from Debian 11 32-bit
	XKeyboard::getMinMaxKeycodes(display->min_keycode, display->max_keycode);

	display->data = new DisplayData(memory);
	display->displayAddress = displayAddress;
	display->refCount = 1;

	U32 screenAddress = createScreen(thread, nextAddress, display, displayAddress);
	display->screens = screenAddress;
	thread->process->perProcessData.set(B("XDisplay"), display);	

	printf("display = %x, screens = %x, screen = %x", displayAddress, display->screens, screenAddress);
	return displayAddress;
}

Screen* Display::getScreen(KThread* thread, S32 screen) {
	if (screen >= this->nscreens) {
		return nullptr;
	}
	return (Screen*)thread->memory->getIntPtr(this->screens + screen * sizeof(Screen));
}

Visual* Depth::getVisual(KThread* thread, S32 visual, U32* address) {
	if (visual >= this->nvisuals) {
		return nullptr;
	}
	U32 visualAddress = this->visuals + visual * sizeof(Visual);
	if (address) {
		*address = visualAddress;
	}
	return (Visual*)thread->memory->getIntPtr(visualAddress);
}

Depth* Screen::getDepth(KThread* thread, S32 depth) {
	if (depth >= this->ndepths) {
		return nullptr;
	}
	return (Depth*)thread->memory->getIntPtr(this->depths + depth * sizeof(U32));
}

U32 Display::createString(KThread* thread, const BString& str) {
	U32 result = alloc(thread, str.length() + 1);
	thread->memory->memcpy(result, str.c_str(), str.length() + 1);
	return result;
}

U32 Display::alloc(KThread* thread, U32 len) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(data->heapMutex);
	return data->heap.alloc(thread, len);
}

void Display::free(U32 address) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(data->heapMutex);
	data->heap.free(address);
}

void Display::iterateVisuals(KThread* thread, std::function<bool(S32 screenIndex, U32 visualAddress, Screen* screen, Depth* depth, Visual* visual)> pfn) {
	for (S32 screenIndex = 0; screenIndex < this->nscreens; screenIndex++) {
		Screen* screen = getScreen(thread, screenIndex);
		for (S32 depthIndex = 0; depthIndex < screen->ndepths; depthIndex++) {
			Depth* depth = screen->getDepth(thread, depthIndex);
			for (S32 visualIndex = 0; visualIndex < depth->nvisuals; visualIndex++) {
				U32 visualAddress = 0;
				Visual* visual = depth->getVisual(thread, visualIndex, &visualAddress);
				if (!pfn(screenIndex, visualAddress, screen, depth, visual)) {
					return;
				}
			}
		}
	}
}

void XVisualInfo::read(KMemory* memory, U32 address) {
	this->visual = memory->readd(address); address += 4;
	visualid = memory->readd(address); address += 4;
	screen = (S32)memory->readd(address); address += 4;
	depth = (S32)memory->readd(address); address += 4;
	c_class = (S32)memory->readd(address); address += 4;
	red_mask = memory->readd(address); address += 4;
	green_mask = memory->readd(address); address += 4;
	blue_mask = memory->readd(address); address += 4;
	colormap_size = (S32)memory->readd(address); address += 4;
	bits_per_rgb = (S32)memory->readd(address);
}

bool XVisualInfo::match(U32 mask, S32 screenIndex, const Screen* screen, const Depth* depth, const Visual* visual) {
	if ((mask & VisualIDMask) && this->visualid != visual->visualid) {
		return false;
	}
	if ((mask & VisualScreenMask) && this->screen != screenIndex) {
		return false;
	}
	if ((mask & VisualDepthMask) && this->depth != depth->depth) {
		return false;
	}
	if ((mask & VisualClassMask) && this->c_class != visual->c_class) {
		return false;
	}
	if ((mask & VisualRedMaskMask) && this->red_mask != visual->red_mask) {
		return false;
	}
	if ((mask & VisualGreenMaskMask) && this->green_mask != visual->green_mask) {
		return false;
	}
	if ((mask & VisualBlueMaskMask) && this->blue_mask != visual->blue_mask) {
		return false;
	}
	if ((mask & VisualColormapSizeMask) && this->colormap_size != visual->map_entries) {
		return false;
	}
	if ((mask & VisualBitsPerRGBMask) && this->bits_per_rgb != visual->bits_per_rgb) {
		return false;
	}
	return true;
}

void XVisualInfo::set(S32 screenIndex, U32 visualAddress, Screen* screen, Depth* depth, Visual* visual) {
	this->visual = visualAddress;
	this->visualid = visual->visualid;
	this->screen = screenIndex;
	this->depth = depth->depth;
	this->c_class = visual->c_class;
	this->red_mask = visual->red_mask;
	this->green_mask = visual->green_mask;
	this->blue_mask = visual->blue_mask;
	this->colormap_size = visual->map_entries;
	this->bits_per_rgb = visual->bits_per_rgb;
}