#ifndef __X_WINDOW_H__
#define __X_WINDOW_H__

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

#define CopyFromParent 0
#define InputOutput		1
#define InputOnly		2

#define ForgetGravity		0
#define NorthWestGravity	1
#define NorthGravity		2
#define NorthEastGravity	3
#define WestGravity		4
#define CenterGravity		5
#define EastGravity		6
#define SouthWestGravity	7
#define SouthGravity		8
#define SouthEastGravity	9
#define StaticGravity		10

#define NotUseful               0
#define WhenMapped              1
#define Always                  2

struct XWMHints {
	U32 flags;	/* marks which fields in this structure are defined */
	Bool input;	/* does this application rely on the window manager to
			get keyboard input? */
	S32 initial_state;	/* see below */
	Pixmap icon_pixmap;	/* pixmap to be used as icon */
	Window icon_window; 	/* window to be used as icon */
	S32 icon_x, icon_y; 	/* initial position of icon */
	Pixmap icon_mask;	/* icon mask bitmap */
	XID window_group;	/* id of related window group */
	/* this structure may be extended in the future */
};

/* definition for flags of XWMHints */

#define InputHint 		(1L << 0)
#define StateHint 		(1L << 1)
#define IconPixmapHint		(1L << 2)
#define IconWindowHint		(1L << 3)
#define IconPositionHint 	(1L << 4)
#define IconMaskHint		(1L << 5)
#define WindowGroupHint		(1L << 6)
#define AllHints (InputHint|StateHint|IconPixmapHint|IconWindowHint| \
IconPositionHint|IconMaskHint|WindowGroupHint)
#define XUrgencyHint		(1L << 8)

/* definitions for initial window state */
#define WithdrawnState 0	/* for windows that are not mapped */
#define NormalState 1	/* most applications want to start this way */
#define IconicState 3	/* application wants to start as an icon */

struct XWMState {
	U32 state;
	U32 icon; 	// WINDOW 	ID of icon window
};

struct XWindowChanges {
	S32 x, y;
	S32 width, height;
	S32 border_width;
	Window sibling;
	S32 stack_mode;

	void read(KMemory* memory, U32 address);
};

#define CWX			(1<<0)
#define CWY			(1<<1)
#define CWWidth			(1<<2)
#define CWHeight		(1<<3)
#define CWBorderWidth		(1<<4)
#define CWSibling		(1<<5)
#define CWStackMode		(1<<6)

typedef struct {
	U32 flags;	/* marks which fields in this structure are defined */
	S32 x, y;		/* obsolete for new window mgrs, but clients */
	S32 width, height;	/* should set so old wm's don't mess up */
	S32 min_width, min_height;
	S32 max_width, max_height;
	S32 width_inc, height_inc;
	struct {
		S32 x;	/* numerator */
		S32 y;	/* denominator */
	} min_aspect, max_aspect;
	S32 base_width, base_height;		/* added by ICCCM version 1 */
	S32 win_gravity;			/* added by ICCCM version 1 */
} XSizeHints;

struct XSetWindowAttributes {
	Pixmap background_pixmap = 0;	/* background or None or ParentRelative */
	U32 background_pixel = 0;	/* background pixel */
	Pixmap border_pixmap = 0;	/* border of the window */
	U32 border_pixel = 0;	/* border pixel value */
	S32 bit_gravity = ForgetGravity;		/* one of bit gravity values */
	S32 win_gravity = NorthWestGravity;		/* one of the window gravity values */
	S32 backing_store = NotUseful;		/* NotUseful, WhenMapped, Always */
	U32 backing_planes = 0xffffffff;/* planes to be preserved if possible */
	U32 backing_pixel = 0;/* value to use in restoring planes */
	S32 save_under = 0;		/* should bits under be saved? (popups) */
	S32 event_mask = 0;		/* set of events that should be saved */
	S32 do_not_propagate_mask = 0;	/* set of events that should not propagate */
	S32 override_redirect = 0;	/* boolean value for override-redirect */
	Colormap colormap = 0;		/* color map to be associated with window */
	Cursor cursor = 0;		/* cursor to be displayed (or None) */

	void read(KMemory* memory, U32 address);
	void copyWithMask(XSetWindowAttributes* attributes, U32 valueMask);
	static XSetWindowAttributes* get(KMemory* memory, U32 address, XSetWindowAttributes* tmp);
};

#define XWindowPtr std::shared_ptr<XWindow>
class XWindow : public XDrawable, public std::enable_shared_from_this<XWindow> {
public:
	XWindow(U32 displayId, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width, const VisualPtr& visual);
	void onCreate();
	void onDestroy();

	int setAttributes(const DisplayDataPtr& data, XSetWindowAttributes* attributes, U32 valueMask);
	void setTransient(U32 w);
	void iterateMappedChildrenFrontToBack(std::function<bool(const XWindowPtr& child)> callback, bool includeTransients = false);
	void iterateMappedChildrenBackToFront(std::function<bool(const XWindowPtr& child)> callback, bool includeTransients = false);
	void iterateAllMappedDescendants(std::function<void(const XWindowPtr& child)> callback);
	VisualPtr getVisual() {return visual;}

	void setTextProperty(KThread* thread, XTextProperty* name, Atom property, bool trace = false);
	int configure(U32 mask, XWindowChanges* changes);
	int moveResize(S32 x, S32 y, U32 width, U32 height);

	XPropertyPtr getProperty(U32 atom);
	void setProperty(U32 atom, U32 type, U32 format, U32 length, U8* value, bool trace = false);
	void setProperty(U32 atom, U32 type, U32 format, U32 length, U32 value, bool trace = false);
	void deleteProperty(U32 atom, bool trace = false);
	int handleNetWmStatePropertyEvent(const XEvent& event);

	int mapWindow();
	int unmapWindow();
	int reparentWindow(const XWindowPtr& parent, S32 x, S32 y);

	void windowToScreen(S32& x, S32& y);
	void screenToWindow(S32& x, S32& y);
	bool mouseMoveScreenCoords(S32 x, S32 y);
	bool mouseButtonScreenCoords(U32 button, S32 x, S32 y, bool pressed);
	void keyScreenCoords(U32 key, S32 x, S32 y, bool pressed);
	void motionNotify(const DisplayDataPtr& data, S32 x, S32 y);
	void buttonNotify(const DisplayDataPtr& data, U32 button, S32 x, S32 y, bool pressed);
	void keyNotify(const DisplayDataPtr& data, U32 key, S32 x, S32 y, bool pressed);
	void crossingNotify(const DisplayDataPtr& data, bool in, S32 x, S32 y, S32 mode, S32 detail);
	
	void input2MotionNotify(const DisplayDataPtr& data, S32 x, S32 y);

	XWindowPtr getParent() {return parent;}
	bool mapped() {return this->isMapped;}
	bool isThisAndAncestorsMapped();
	XWindowPtr getTopMappedChild();
	XCursorPtr getCursor();

	const U32 displayId;
	const U32 c_class;

	void draw();
	void setDirty() override;
	XWindowPtr getWindowFromPoint(S32 screenX, S32 screenY);

	void focusOut();
	void focusIn();

	bool doesThisOrAncestorHaveFocus();
	XWindowPtr getLeastCommonAncestor(const XWindowPtr& wnd);

	void getAncestorTree(std::vector<XWindowPtr>& ancestors);

	XCursorPtr cursor;
	XColorMapPtr colorMap;	
private:
	friend class XServer;

	XWindowPtr parent;
	S32 left;
	S32 top;	
	U32 border_width;
	XSetWindowAttributes attributes;
	bool isMapped = false;	
	bool isFullScreen = false;
	U32 transientForCache = 0;
	XRectangle restoreRect;

	BOXEDWINE_MUTEX propertiesMutex;
	XProperties properties;

	BOXEDWINE_MUTEX childrenMutex;
	BHashTable<U32, XWindowPtr> children;
	std::vector<XWindowPtr> zchildren;
	std::vector<XWindowPtr> transientChildren;

	void exposeNofity(const DisplayDataPtr& data, S32 x, S32 y, S32 width, S32 height, S32 count);
	void configureNotify();
	void focusNotify(const DisplayDataPtr& data, bool isIn, S32 mode, S32 detail);

	void setWmState(U32 state, U32 icon);
	XWindowPtr previousSibling();
	void removeFromParent();
	void addToParent();

	bool isDialog();
	bool isTransient();
	U32 NET_WM_WINDOW_TYPE();
	U32 WM_TRANSIENT_FOR();
};

#endif