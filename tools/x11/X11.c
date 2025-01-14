#define XUTIL_DEFINE_FUNCTIONS

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include "X11_def.h"

Display* XOpenDisplay(const char* displayName) {
	CALL_1_R(X11_OPEN_DISPLAY, displayName);
}

int XCloseDisplay(Display* display) {
	CALL_1_R(X11_CLOSE_DISPLAY, display);
}

char* XDisplayName(_Xconst char* string) {
	CALL_1_R(X11_DISPLAY_NAME, string);
}

int XGrabServer(Display* display) {
	CALL_1_R(X11_GRAB_SERVER, display);
}

int XUngrabServer(Display* display) {
	CALL_1_R(X11_UNGRAB_SERVER, display);
}

Status XInitThreads() {
	CALL_0_R(X11_INIT_THREADS);
}

int XClearArea(Display* display, Window w, int x, int y, unsigned int width, unsigned int height, Bool exposures) {
	CALL_7_R(X11_CLEAR_AREA, display, w, x, y, width, height, exposures);
}

int XSync(Display* display, Bool discard) {
	CALL_2_R(X11_SYNC, display, discard);
}

Window XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int class, Visual* visual, unsigned long valuemask, XSetWindowAttributes* attributes) {
	CALL_12_R(X11_CREATE_WINDOW, display, parent, x, y, width, height, border_width, depth, class, visual, valuemask, attributes);
}

Bool XTranslateCoordinates(Display* display, Window src_w, Window dest_w, int src_x, int src_y, int* dest_x_return, int* dest_y_return, Window* child_return) {
	CALL_8_R(X11_TRANSLATE_COORDINATES, display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, child_return);
}

int XDestroyWindow(Display* display, Window w) {
	CALL_2_R(X11_DESTROY_WINDOW, display, w);
}

int XReparentWindow(Display* display, Window w, Window parent, int x, int y) {
	CALL_5_R(X11_REPARENT_WINDOW, display, w, parent, x, y);
}

Status XQueryTree(Display* display, Window w, Window* root_return, Window* parent_return, Window** children_return, unsigned int* nchildren_return) {
	CALL_6_R(X11_QUERY_TREE, display, w, root_return, parent_return, children_return, nchildren_return);
}

Status XGetWindowAttributes(Display* display, Window w, XWindowAttributes* window_attributes_return) {
	CALL_3_R(X11_GET_WINDOW_ATTRIBUTES, display, w, window_attributes_return);
}

int XChangeWindowAttributes(Display* display, Window w, unsigned long valuemask, XSetWindowAttributes* attributes) {
	CALL_4_R(X11_CHANGE_WINDOW_ATTRIBUTES, display, w, valuemask, attributes);
}

int XConfigureWindow(Display* display, Window w, unsigned int value_mask, XWindowChanges* values) {
	CALL_4_R(X11_CONFIGURE_WINDOW, display, w, value_mask, values);
}

Status XIconifyWindow(Display* display, Window w, int screen_number) {
	CALL_3_R(X11_ICONIFY_WINDOW, display, w, screen_number);
}

int XWindowEvent(Display* display, Window w, long event_mask, XEvent* event_return) {
	CALL_4_R(X11_WINDOW_EVENT, display, w, event_mask, event_return);
}

Status XWithdrawWindow(Display* display, Window w, int screen_number) {
	CALL_3_R(X11_WITHDRAW_WINDOW, display, w, screen_number);
}

char* XGetDefault(Display* display, _Xconst char* program, _Xconst char* option) {	
	CALL_3_R(X11_GET_DEFAULT, display, program, option);
}

int XSetInputFocus(Display* display, Window focus, int revert_to, Time time) {
	CALL_4_R(X11_SET_INPUT_FOCUS, display, focus, revert_to, time);
}

int XSelectInput(Display* display, Window w, long event_mask) {
	CALL_3_R(X11_SELECT_INPUT, display, w, event_mask);
}

int XFindContext(Display* display, XID rid, XContext context, XPointer* data_return) {
	CALL_4_R(X11_FIND_CONTEXT, display, rid, context, data_return);
}

int XSaveContext(Display* display, XID rid, XContext context, _Xconst char* data) {
	CALL_4_R(X11_SAVE_CONTEXT, display, rid, context, data);
}

int XDeleteContext(Display* display, XID rid, XContext context) {
	CALL_3_R(X11_DELETE_CONTEXT, display, rid, context);
}

int XGetInputFocus(Display* display, Window* focus_return, int* revert_to_return) {
	CALL_3_R(X11_GET_INPUT_FOCUS, display, focus_return, revert_to_return);
}

XFontSet XCreateFontSet(Display* display, _Xconst char* base_font_name_list, char*** missing_charset_list, int* missing_charset_count, char** def_string) {
	CALL_5_R(X11_CREATE_FONT_SET, display, base_font_name_list, missing_charset_list, missing_charset_count, def_string);
}

void XFreeFontSet(Display* display, XFontSet font_set) {
	CALL_2(X11_FREE_FONT_SET, display, font_set);
}

int XFreeFont(Display* display, XFontStruct* font_struct) {
	CALL_2_R(X11_FREE_FONT, display, font_struct);
}

Bool XkbUseExtension(Display* dpy, int* major_rtrn, int* minor_rtrn) {	
	return True;
}

Bool XkbSetDetectableAutoRepeat(Display* dpy, Bool detectable, Bool* supported) {
	return True;
}

typedef int (*pfnXSynchronize)(Display*);

pfnXSynchronize XSynchronize(Display* display,Bool onoff) {
	// boxedwine is always synchronous
	return NULL;
}

int XMoveResizeWindow(Display* display, Window w, int x, int y, unsigned int width, unsigned int height) {
	CALL_6_R(X11_MOVE_RESIZE_WINDOW, display, w, x, y, width, height);
}

int XMapWindow(Display* display, Window w) {
	CALL_2_R(X11_MAP_WINDOW, display, w);
}

int XUnmapWindow(Display* display, Window w) {
	CALL_2_R(X11_UNMAP_WINDOW, display, w);
}

int XGrabPointer(Display* display, Window grab_window, Bool owner_events, unsigned int event_mask, int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor, Time time) {
	CALL_9_R(X11_GRAB_POINTER, display, grab_window, owner_events, event_mask, pointer_mode, keyboard_mode, confine_to, cursor, time);
}

int XUngrabPointer(Display* display, Time time) {
	CALL_2_R(X11_UNGRAB_POINTER, display, time);
}

int XWarpPointer(Display* display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y) {
	CALL_9_R(X11_WARP_POINTER, display, src_w, dest_w, src_x, src_y, src_width, src_height, dest_x, dest_y);
}

Bool XQueryPointer(Display* display, Window w, Window* root_return, Window* child_return, int* root_x_return, int* root_y_return, int* win_x_return, int* win_y_return, unsigned int* mask_return) {
	CALL_9_R(X11_QUERY_POINTER, display, w, root_return, child_return, root_x_return, root_y_return, win_x_return, win_y_return, mask_return);
}

// The XNoOp function sends a NoOperation protocol request to the X server, thereby exercising the connection. 
int XNoOp(Display* display) {
	return Success;
}

int XGetScreenSaver(Display* display, int* timeout_return, int* interval_return, int* prefer_blanking_return, int* allow_exposures_return) {
	if (timeout_return) {
		*timeout_return = 3600;
	}
	return Success;
}

long XExtendedMaxRequestSize(Display* display) {
	return 256 * 1024; // just a guess
}

long XMaxRequestSize(Display* display) {
	return 256 * 1024; // just a guess
}

int XmbTextListToTextProperty(Display* display, char** list, int count, XICCEncodingStyle style, XTextProperty* text_prop_return) {
	CALL_5_R(X11_MB_TEXT_LIST_TO_TEXT_PROPERTY, display, list, count, style, text_prop_return);
}

int XmbTextPropertyToTextList(Display* display, const XTextProperty* text_prop, char*** list_return, int* count_return) {
	CALL_4_R(X11_MB_TEXT_PROPERTY_TO_TEXT_LIST, display, text_prop, list_return, count_return);
}

void XSetTextProperty(Display* display, Window w, XTextProperty* text_prop, Atom property) {
	CALL_4_R(X11_SET_TEXT_PROPERTY, display, w, text_prop, property);
}

int XSetSelectionOwner(Display* display, Atom selection, Window owner, Time time) {
	CALL_4_R(X11_SET_SELECTION_OWNER, display, selection, owner, time);
}

Window XGetSelectionOwner(Display* display, Atom selection) {
	CALL_2_R(X11_GET_SELECTION_OWNER, display, selection);
}

typedef Bool(*pfnEventPredicate)(Display* display, XEvent* event, XPointer arg);

int lockEvents(Display* display) {
	CALL_1_R(X11_LOCK_EVENTS, display);
}

Bool XCheckIfEvent(Display* display, XEvent* event_return, pfnEventPredicate predicate, XPointer arg) {
	int count = lockEvents(display);
	int i;

	for (i = 0; i < count; i++) {
		XEvent event;
		CALL_3(X11_GET_EVENT, display, &event, i);
		if (predicate(display, &event, arg)) {
			*event_return = event;
			CALL_2(X11_REMOVE_EVENT, display, i);
			CALL_1(X11_UNLOCK_EVENTS, display);
			return True;
		}
	}
	CALL_1(X11_UNLOCK_EVENTS, display);
	return False;
}

Status XSendEvent(Display* display, Window w, Bool propagate, long event_mask, XEvent* event_send) {
	CALL_5_R(X11_SEND_EVENT, display, w, propagate, event_mask, event_send);
}

int XPutBackEvent(Display* display, XEvent* event) {
	CALL_2_R(X11_PUT_BACK_EVENT, display, event);
}

Bool XFilterEvent(XEvent* event, Window window) {
	CALL_2_R(X11_FILTER_EVENT, event, window);
}

int XLookupString(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer, KeySym* keysym_return, XComposeStatus* status_in_out) {
	CALL_5_R(X11_LOOKUP_STRING, event_struct, buffer_return, bytes_buffer, keysym_return, status_in_out);
}

int XmbLookupString(XIC ic, XKeyPressedEvent* event, char* buffer_return, int bytes_buffer, KeySym* keysym_return, Status* status_return) {
	CALL_6_R(X11_MB_LOOKUP_STRING, ic, event, buffer_return, bytes_buffer, keysym_return, status_return);
}

char* XKeysymToString(KeySym keysym) {
	CALL_1_R(X11_KEYSYM_TO_STRING, keysym);
}

int XkbTranslateKeySym(Display* dpy, KeySym* sym_return, unsigned int modifiers, char* buffer, int nbytes, int* extra_rtrn) {
	CALL_6_R(X11_KB_TRANSLATE_KEYSYM, dpy, sym_return, modifiers, buffer, nbytes, extra_rtrn);
}

KeySym XLookupKeysym(XKeyEvent* key_event, int index) {
	CALL_2_R(X11_LOOKUP_KEYSYM, key_event, index);
}

KeySym* XGetKeyboardMapping(Display* display, KeyCode first_keycode, int keycode_count, int* keysyms_per_keycode_return) {
	CALL_4_R(X11_GET_KEYBOARD_MAPPING, display, first_keycode, keycode_count, keysyms_per_keycode_return);
}

int XFreeModifiermap(XModifierKeymap* modmap) {
	CALL_1_R(X11_FREE_MODIFIER_MAP, modmap);
}

KeyCode XKeysymToKeycode(Display* display, KeySym keysym) {
	CALL_2_R(X11_KEYSYM_TO_KEYCODE, display, keysym);
}

KeySym XkbKeycodeToKeysym(Display* dpy, KeyCode kc, int group, int level) {
	CALL_4_R(X11_KB_KEYCODE_TO_KEYSYM, dpy, kc, group, level);
}

int XDisplayKeycodes(Display* display, int* min_keycodes_return, int* max_keycodes_return) {
	CALL_3_R(X11_DISPLAY_KEYCODES, display, min_keycodes_return, max_keycodes_return);
}

XModifierKeymap* XGetModifierMapping(Display* display) {
	CALL_1_R(X11_GET_MODIFIER_MAPPING, display);
}

int XRefreshKeyboardMapping(XMappingEvent* event_map) {
	CALL_1_R(X11_REFRESH_KEYBOARD_MAPPING, event_map);
}

int XBell(Display* display, int percent) {
	CALL_2_R(X11_BELL, display, percent);
}

int XGetWindowProperty(Display* display, Window w, Atom property, long long_offset, long long_length, Bool delete, Atom req_type, Atom* actual_type_return, int* actual_format_return, unsigned long* nitems_return, unsigned long* bytes_after_return, unsigned char** prop_return) {
	CALL_12_R(X11_GET_WINDOW_PROPERTY, display, w, property, long_offset, long_length, delete, req_type, actual_type_return, actual_format_return, nitems_return, bytes_after_return, prop_return);
}

int XChangeProperty(Display* display, Window w, Atom property, Atom type, int format, int mode, _Xconst unsigned char* data, int nelements) {
	CALL_8_R(X11_CHANGE_PROPERTY, display, w, property, type, format, mode, data, nelements);
}

int XDeleteProperty(Display* display, Window w, Atom property) {
	CALL_3_R(X11_DELETE_PROPERTY, display, w, property);
}

int XConvertSelection(Display* display, Atom selection, Atom target, Atom property, Window requestor, Time time) {
	CALL_6_R(X11_CONVERT_SELECTION, display, selection, target, property, requestor, time);
}

Bool XCheckTypedWindowEvent(Display* display, Window w, int event_type, XEvent* event_return) {
	CALL_4_R(X11_CHECK_TYPED_WINDOW_EVENT, display, w, event_type, event_return);
}

Status XGetGeometry(Display* display, Drawable d, Window* root_return, int* x_return, int* y_return, unsigned int* width_return, unsigned int* height_return, unsigned int* border_width_return, unsigned int* depth_return) {
	CALL_9_R(X11_GET_GEOMETRY, display, d, root_return, x_return, y_return, width_return, height_return, border_width_return, depth_return);
}

Atom XInternAtom(Display* display, _Xconst char* atom_name, Bool only_if_exists) {
	CALL_3_R(X11_INTERN_ATOM, display, atom_name, only_if_exists);
}

Status XInternAtoms(Display* dpy, char** names, int count, Bool onlyIfExists, Atom* atoms_return) {
	CALL_5_R(X11_INTERN_ATOMS, dpy, names, count, onlyIfExists, atoms_return);
}

char* XGetAtomName(Display* display, Atom atom) {
	CALL_2_R(X11_GET_ATOM_NAME, display, atom);
}

Status XGetAtomNames(Display* dpy, Atom* atoms, int count, char** names_return) {
	CALL_4_R(X11_GET_ATOM_NAMES, dpy, atoms, count, names_return);
}

int XSetScreenSaver(Display* display, int timeout, int interval, int prefer_blanking, int allow_exposures) {
	return Success;
}

int XResetScreenSaver(Display* display) {
	return Success;
}

Colormap XCreateColormap(Display* display, Window w, Visual* visual, int alloc) {
	CALL_4_R(X11_CREATE_COLORMAP, display, w, visual, alloc);
}

int XInstallColormap(Display* display, Colormap colormap) {
	CALL_2_R(X11_INSTALL_COLORMAP, display, colormap);
}

int XFreeColormap(Display* display, Colormap colormap) {
	CALL_2_R(X11_FREE_COLORMAP, display, colormap);
}

int XFreeColors(Display* display, Colormap colormap, unsigned long* pixels, int npixels, unsigned long planes) {
	CALL_5_R(X11_FREE_COLORS, display, colormap, pixels, npixels, planes);
}

int XStoreColor(Display* display, Colormap colormap, XColor* color) {
	CALL_3_R(X11_STORE_COLOR, display, colormap, color);
}

int XQueryColor(Display* display, Colormap colormap, XColor* def_in_out) {
	CALL_3_R(X11_QUERY_COLOR, display, colormap, def_in_out);
}

int XQueryColors(Display* display, Colormap colormap, XColor* defs_in_out, int ncolors) {
	CALL_4_R(X11_QUERY_COLORS, display, colormap, defs_in_out, ncolors);
}

Status XAllocColor(Display* display, Colormap colormap, XColor* screen_in_out) {
	CALL_3_R(X11_ALLOC_COLOR, display, colormap, screen_in_out);
}

Status XAllocColorCells(Display* display, Colormap colormap, Bool contig, unsigned long* plane_masks_return, unsigned int nplanes, unsigned long* pixels_return, unsigned int npixels) {
	CALL_7_R(X11_ALLOC_COLOR_CELLS, display, colormap, contig, plane_masks_return, nplanes, pixels_return, npixels);
}

XVisualInfo* XGetVisualInfo(Display* display, long vinfo_mask, XVisualInfo* vinfo_template, int* nitems_return) {
	CALL_4_R(X11_GET_VISUAL_INFO, display, vinfo_mask, vinfo_template, nitems_return);
}

Status XMatchVisualInfo(Display* display, int screen, int depth, int class, XVisualInfo* vinfo_return) {
	CALL_5_R(X11_MATCH_VISUAL_INFO, display, screen, depth, class, vinfo_return);
}

XPixmapFormatValues* XListPixmapFormats(Display* display, int* count_return) {
	CALL_2_R(X11_LIST_PIXEL_FORMATS, display, count_return);
}

void XLockDisplay(Display* display) {
	CALL_1(X11_LOCK_DISPLAY, display);
}

void XUnlockDisplay(Display* display) {
	CALL_1(X11_UNLOCK_DISPLAY, display);
}

XErrorHandler XSetErrorHandler(XErrorHandler handler) {
	static XErrorHandler previous;
	XErrorHandler result = previous;
	previous = handler;
	return result;
}

int XCopyArea(Display* display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y) {
	CALL_10_R(X11_COPY_AREA, display, src, dest, gc, src_x, src_y, width, height, dest_x, dest_y);
}

int XPutImage(Display* display, Drawable d, GC gc, XImage* image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height) {	
	CALL_10_R(X11_PUT_IMAGE, display, d, gc, image, src_x, src_y, dest_x, dest_y, width, height);
}

XImage* XCreateImage1(Display* display, Visual* visual, unsigned int depth, int format, int offset, char* data, unsigned int width, unsigned int height, int bitmap_pad, int bytes_per_line) {
	CALL_10_R(X11_CREATE_IMAGE, display, visual, depth, format, offset, data, width, height, bitmap_pad, bytes_per_line);
}

int XDestroyImage(XImage* ximage) {
	CALL_1_R(X11_DESTROY_IMAGE, ximage);
}

unsigned long XGetPixel(XImage* ximage, int x, int y) {
	CALL_3_R(X11_GET_PIXEL, ximage, x, y);
}

int XPutPixel(XImage* ximage, int x, int y, unsigned long pixel) {
	CALL_4_R(X11_PUT_PIXEL, ximage, x, y, pixel);
}

XImage* XSubImage(XImage* ximage, int x, int y, unsigned int width, unsigned int height) {
	CALL_5_R(X11_SUB_IMAGE, ximage, x, y, width, height);
}

int XAddPixel(XImage* ximage, long value) {
	CALL_2_R(X11_ADD_PIXEL, ximage, value);
}

XImage* XCreateImage(Display* display, Visual* visual, unsigned int depth, int format, int offset, char* data, unsigned int width, unsigned int height, int bitmap_pad, int bytes_per_line) {
	XImage* image = XCreateImage1(display, visual, depth, format, offset, data, width, height, bitmap_pad, bytes_per_line);
	image->f.destroy_image = XDestroyImage;
	image->f.get_pixel = XGetPixel;
	image->f.put_pixel = XPutPixel;
	image->f.sub_image = XSubImage;
	image->f.add_pixel = XAddPixel;
	image->f.create_image = XCreateImage;
	return image;
}

XImage* XGetImage1(Display* display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format) {
	CALL_8_R(X11_GET_IMAGE, display, d, x, y, width, height, plane_mask, format);
}

XImage* XGetImage(Display* display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format) {
	XImage* image = XGetImage1(display, d, x, y, width, height, plane_mask, format);
	image->f.destroy_image = XDestroyImage;
	image->f.get_pixel = XGetPixel;
	image->f.put_pixel = XPutPixel;
	image->f.sub_image = XSubImage;
	image->f.add_pixel = XAddPixel;
	image->f.create_image = XCreateImage;
	return image;
}

Pixmap XCreatePixmap(Display* display, Drawable d, unsigned int width, unsigned int height, unsigned int depth) {
	CALL_5_R(X11_CREATE_PIXMAP, display, d, width, height, depth);
}

Pixmap XCreateBitmapFromData(Display* display, Drawable d, const char* data, unsigned int width, unsigned int height) {
	CALL_5_R(X11_CREATE_BITMAP_FROM_DATA, display, d, data, width, height);
}

int XFreePixmap(Display* display, Pixmap pixmap) {
	CALL_2_R(X11_FREE_PIXMAP, display, pixmap);
}

Cursor XCreatePixmapCursor(Display* display, Pixmap source, Pixmap mask, XColor* foreground_color, XColor* background_color, unsigned int x, unsigned int y) {
	CALL_7_R(X11_CREATE_PIXMAP_CURSOR, display, source, mask, foreground_color, background_color, x, y);
}

Cursor XCreateFontCursor(Display* display, unsigned int shape) {
	CALL_2_R(X11_CREATE_FONT_CURSOR, display, shape);
}

int XDefineCursor(Display* display, Window w, Cursor cursor) {
	CALL_3_R(X11_DEFINE_CURSOR, display, w, cursor);
}

int XFreeCursor(Display* display, Cursor cursor) {
	CALL_2_R(X11_FREE_CURSOR, display, cursor);
}

int XSetFunction(Display* display, GC gc, int function) {
	CALL_3_R(X11_SET_FUNCTION, display, gc, function);
}

int XSetBackground(Display* display, GC gc, unsigned long background) {
	CALL_3_R(X11_SET_BACKGROUND, display, gc, background);
}

int XSetForeground(Display* display, GC gc, unsigned long foreground) {
	CALL_3_R(X11_SET_FOREGROUND, display, gc, foreground);
}

int XCopyPlane(Display* display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y, unsigned long plane) {
	CALL_11_R(X11_COPY_PLANE, display, src, dest, gc, src_x, src_y, width, height, dest_x, dest_y, plane);
}

GC XCreateGC(Display* display, Drawable d, unsigned long valuemask, XGCValues* values) {
	CALL_4_R(X11_CREATE_GC, display, d, valuemask, values);
}

int XSetDashes(Display* display, GC gc, int dash_offset, _Xconst char* dash_list, int n) {
	CALL_5_R(X11_SET_DASHES, display, gc, dash_offset, dash_list, n);
}

int XDrawLine(Display* display, Drawable d, GC gc, int x1, int y1, int x2, int y2) {
	CALL_7_R(X11_DRAW_LINE, display, d, gc, x1, y1, x2, y2);
}

int XDrawLines(Display* display, Drawable d, GC gc, XPoint* points, int npoints, int mode) {
	CALL_6_R(X11_DRAW_LINES, display, d, gc, points, npoints, mode);
}

int XSetArcMode(Display* display, GC gc, int arc_mode) {
	CALL_3_R(X11_SET_ARC_MODE, display, gc, arc_mode);
}

int XFillArc(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2) {
	CALL_9_R(X11_FILL_ARC, display, d, gc, x, y, width, height, angle1, angle2);
}

int XDrawArc(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2) {
	CALL_9_R(X11_DRAW_ARC, display, d, gc, x, y, width, height, angle1, angle2);
}

int XDrawRectangle(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height) {
	CALL_7_R(X11_DRAW_RECTANGLE, display, d, gc, x, y, width, height);
}

int XFillRectangle(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height) {
	CALL_7_R(X11_FILL_RECTANGLE, display, d, gc, x, y, width, height);
}

int XFillRectangles(Display* display, Drawable d, GC gc, XRectangle* rectangles, int nrectangles) {
	CALL_5_R(X11_FILL_RECTANGLES, display, d, gc, rectangles, nrectangles);
}

int XDrawPoint(Display* display, Drawable d, GC gc, int x, int y) {
	CALL_5_R(X11_DRAW_POINT, display, d, gc, x, y);
}

int XFillPolygon(Display* display, Drawable d, GC gc, XPoint* points, int npoints, int shape, int mode) {
	CALL_7_R(X11_FILL_POLYGON, display, d, gc, points, npoints, shape, mode);
}

int XChangeGC(Display* display, GC gc, unsigned long valuemask, XGCValues* values) {
	CALL_4_R(X11_CHANGE_GC, display, gc, valuemask, values);
}

int XFreeGC(Display* display, GC gc) {
	CALL_2_R(X11_FREE_GC, display, gc);
}

int XSetSubwindowMode(Display* display, GC gc, int subwindow_mode) {
	CALL_3_R(X11_SET_SUBWINDOW_MODE, display, gc, subwindow_mode);
}

int XSetGraphicsExposures(Display* display, GC gc, Bool graphics_exposures) {
	CALL_3_R(X11_SET_GRAPHICS_EXPOSURES, display, gc, graphics_exposures);
}

int XSetFillStyle(Display* display, GC gc, int fill_style) {
	CALL_3_R(X11_SET_FILL_STYLE, display, gc, fill_style);
}

int XFree(void* data) {
	CALL_1_R(X11_FREE, data);
}

int XSetClipMask(Display* display, GC gc, Pixmap pixmap) {
	CALL_3_R(X11_SET_CLIP_MASK, display, gc, pixmap);
}

int XSetClipRectangles(Display* display, GC gc, int clip_x_origin, int clip_y_origin, XRectangle* rectangles, int n, int ordering) {
	CALL_7_R(X11_SET_CLIP_RECTANGLES, display, gc, clip_x_origin, clip_y_origin, rectangles, n, ordering);
}

int XFlush(Display* display) {
	CALL_1_R(X11_FLUSH, display);
}

int XSetTransientForHint(Display* display, Window w, Window prop_window) {
	CALL_3_R(X11_SET_TRANSIENT_FOR_HINT, display, w, prop_window);
}

XWMHints* XAllocWMHints() {
	CALL_0_R(X11_ALLOC_WM_HINTS);
}

int XSetWMHints(Display* display, Window w, XWMHints* wm_hints) {
	CALL_3_R(X11_SET_WM_HINTS, display, w, wm_hints);
}

XSizeHints* XAllocSizeHints() {
	CALL_0_R(X11_ALLOC_SIZE_HINTS);
}

XClassHint* XAllocClassHint() {
	CALL_0_R(X11_ALLOC_CLASS_HINT);
}

int XSetClassHint(Display* display, Window w, XClassHint* class_hints) {
	CALL_3_R(X11_SET_CLASS_HINT, display, w, class_hints);
}

void XSetWMName(Display* display, Window w, XTextProperty* text_prop) {
	CALL_3_R(X11_SET_WM_NAME, display, w, text_prop);
}

void XSetWMIconName(Display* display, Window w, XTextProperty* text_prop) {
	CALL_3_R(X11_SET_WM_ICON_NAME, display, w, text_prop);
}

void XSetWMNormalHints(Display* display, Window w, XSizeHints* hints) {
	CALL_3_R(X11_SET_WM_NORMAL_HINTS, display, w, hints);
}

void XSetWMProperties(Display* display, Window w, XTextProperty* window_name, XTextProperty* icon_name, char** argv, int argc, XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints) {
	CALL_9_R(X11_SET_WM_PROPERTIES, display, w, window_name, icon_name, argv, argc, normal_hints, wm_hints, class_hints);
}

Status XReconfigureWMWindow(Display* display, Window w, int screen_number, unsigned int mask, XWindowChanges* changes) {
	CALL_5_R(X11_RECONFIGURE_WM_WINDOW, display, w, screen_number, mask, changes);
}

XVaNestedList XVaCreateNestedList(int unused, ...) {
	CALL_0_R(X11_VA_CREATE_NESTED_LIST);
}

void XUnsetICFocus(XIC ic) {
	CALL_1(X11_UNSET_IC_FOCUS, ic);
}

void XSetICFocus(XIC ic) {
	CALL_1(X11_SET_IC_FOCUS, ic);
}

XIC XCreateIC(XIM im, ...) {
	CALL_0_R(X11_CREATE_IC);
}

void XDestroyIC(XIC ic) {
	CALL_1(X11_DESTROY_IC, ic);
}

char* XSetICValues(XIC ic, ...) {
	CALL_0_R(X11_SET_IC_VALUES);
}

char* XmbResetIC(XIC ic) {
	CALL_1_R(X11_MB_RESET_IC, ic);
}

Bool XSupportsLocale() {
	return True;
}

char* XSetLocaleModifiers(const char* modifier_list) {
	CALL_1_R(X11_SET_LOCALE_MODIFIERS, modifier_list);
}

XIM XOpenIM(Display* dpy, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class) {
	CALL_4_R(X11_OPEN_IM, dpy, rdb, res_name, res_class);
}

char* XLocaleOfIM(XIM im) {
	CALL_1_R(X11_LOCALE_OF_IM, im);
}

Status XCloseIM(XIM im) {
	CALL_1_R(X11_CLOSE_IM, im);
}

char* XSetIMValues(XIM im, ...) {
	CALL_0_R(X11_SET_IM_VALUES);
}

char* XGetIMValues(XIM im, ...) {
	CALL_0_R(X11_GET_IM_VALUES);
}

Display* XDisplayOfIM(XIM im) {
	CALL_1_R(X11_DISPLAY_OF_IM, im);
}

Bool XUnregisterIMInstantiateCallback(Display* dpy, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class, XIDProc callback, XPointer client_data) {
	CALL_6_R(X11_UNREGISTER_IM_INSTANTIATE_CALLBACK, dpy, rdb, res_name, res_class, callback, client_data);
}

typedef int XrmQuark;

XrmQuark XrmUniqueQuark() {
	CALL_0_R(X11_RM_UNIQUE_QUARK);
}

Bool XRegisterIMInstantiateCallback(Display* dpy, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class, XIDProc callback, XPointer client_data) {
	CALL_6_R(X11_REGISTER_IM_INSTANTIATE_CALLBACK, dpy, rdb, res_name, res_class, callback, client_data);
}

void XFreeStringList(char** list) {
	CALL_1(X11_FREE_STRING_LIST, list);
}

Bool XQueryExtension(Display* display, _Xconst char* name, int* major_opcode_return, int* first_event_return, int* first_error_return) {
	CALL_5_R(X11_QUERY_EXTENSION, display, name, major_opcode_return, first_event_return, first_error_return);
}

char* XServerVendor(Display* display) {
	return "Boxedwine.Org";
}

int XVendorRelease(Display* display) {
	return 1;
}

Bool XGetEventData(Display* dpy, XGenericEventCookie* cookie) {
	CALL_2_R(X11_GET_EVENT_DATA, dpy, cookie);
}

void XFreeEventData(Display* dpy, XGenericEventCookie* cookie) {
	CALL_2(X11_FREE_EVENT_DATA, dpy, cookie);
}
