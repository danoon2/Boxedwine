#ifndef __X11_H__
#define __X11_H__

#define Bool S32
#define Status S32
#define True 1
#define False 0

#define StaticGray		0
#define GrayScale		1
#define StaticColor		2
#define PseudoColor		3
#define TrueColor		4
#define DirectColor		5

#define AnyPropertyType      0L	/* special Atom, passed to GetProperty */

#define XA_PRIMARY 1
#define XA_SECONDARY 2
#define XA_ARC 3
#define XA_ATOM 4
#define XA_BITMAP 5
#define XA_CARDINAL 6
#define XA_COLORMAP 7
#define XA_CURSOR 8
#define XA_CUT_BUFFER0 9
#define XA_CUT_BUFFER1 10
#define XA_CUT_BUFFER2 11
#define XA_CUT_BUFFER3 12
#define XA_CUT_BUFFER4 13
#define XA_CUT_BUFFER5 14
#define XA_CUT_BUFFER6 15
#define XA_CUT_BUFFER7 16
#define XA_DRAWABLE 17
#define XA_FONT 18
#define XA_INTEGER 19
#define XA_PIXMAP 20
#define XA_POINT 21
#define XA_RECTANGLE 22
#define XA_RESOURCE_MANAGER 23
#define XA_RGB_COLOR_MAP 24
#define XA_RGB_BEST_MAP 25
#define XA_RGB_BLUE_MAP 26
#define XA_RGB_DEFAULT_MAP 27
#define XA_RGB_GRAY_MAP 28
#define XA_RGB_GREEN_MAP 29
#define XA_RGB_RED_MAP 30
#define XA_STRING 31
#define XA_VISUALID 32
#define XA_WINDOW 33
#define XA_WM_COMMAND 34
#define XA_WM_HINTS 35
#define XA_WM_CLIENT_MACHINE 36
#define XA_WM_ICON_NAME 37
#define XA_WM_ICON_SIZE 38
#define XA_WM_NAME 39
#define XA_WM_NORMAL_HINTS 40
#define XA_WM_SIZE_HINTS 41
#define XA_WM_ZOOM_HINTS 42
#define XA_MIN_SPACE 43
#define XA_NORM_SPACE 44
#define XA_MAX_SPACE 45
#define XA_END_SPACE 46
#define XA_SUPERSCRIPT_X 47
#define XA_SUPERSCRIPT_Y 48
#define XA_SUBSCRIPT_X 49
#define XA_SUBSCRIPT_Y 50
#define XA_UNDERLINE_POSITION 51
#define XA_UNDERLINE_THICKNESS 52
#define XA_STRIKEOUT_ASCENT 53
#define XA_STRIKEOUT_DESCENT 54
#define XA_ITALIC_ANGLE 55
#define XA_X_HEIGHT 56
#define XA_QUAD_WIDTH 57
#define XA_WEIGHT 58
#define XA_POINT_SIZE 59
#define XA_RESOLUTION 60
#define XA_COPYRIGHT 61
#define XA_NOTICE 62
#define XA_FONT_NAME 63

#define XA_FAMILY_NAME 64
#define XA_FULL_NAME 65
#define XA_CAP_HEIGHT 66
#define XA_WM_CLASS 67
#define XA_WM_TRANSIENT_FOR 68

#define XA_LAST_PREDEFINED 68

#define _NET_WM_STATE 100
#define _NET_WM_STATE_FULLSCREEN 101
#define _NET_WM_WINDOW_TYPE 102
#define _NET_WM_WINDOW_TYPE_NORMAL 103
#define _NET_WM_WINDOW_TYPE_DIALOG 104
#define WM_STATE 105
#define _NET_WM_NAME 106
#define _MOTIF_WM_HINTS 107
#define _NET_WM_ICON 108

/* Byte order  used in imageByteOrder and bitmapBitOrder */

#define LSBFirst		0
#define MSBFirst		1

#define Success		   0	/* everything's okay */
#define BadRequest	   1	/* bad request code */
#define BadValue	   2	/* int parameter out of range */
#define BadWindow	   3	/* parameter not a Window */
#define BadPixmap	   4	/* parameter not a Pixmap */
#define BadAtom		   5	/* parameter not an Atom */
#define BadCursor	   6	/* parameter not a Cursor */
#define BadFont		   7	/* parameter not a Font */
#define BadMatch	   8	/* parameter mismatch */
#define BadDrawable	   9	/* parameter not a Pixmap or Window */
#define BadAccess	  10	/* depending on context:
				 - key/button already grabbed
				 - attempt to free an illegal
				   cmap entry
				- attempt to store into a read-only
				   color map entry.
				- attempt to modify the access control
				   list from other than the local host.
				*/
#define BadAlloc	  11	/* insufficient resources */
#define BadColor	  12	/* no such colormap */
#define BadGC		  13	/* parameter not a GC */
#define BadIDChoice	  14	/* choice not in range or already used */
#define BadName		  15	/* font or color name doesn't exist */
#define BadLength	  16	/* Request length incorrect */
#define BadImplementation 17	/* server is defective */

typedef U32 XPointer;
struct _XPrivate;

typedef U32 XID;

typedef U32 XExtDataPtrAddress; // struct _XExtData*
typedef U32 XCharPtrAddress; // char*
typedef U32 XPrivatePtrAddress; // struct _XPrivate*
typedef U32 XrmHashBucketRecPtrAddress; // struct _XrmHashBucketRec*
typedef U32 ScreenFormatPtrAddress; // ScreenFormat*
typedef U32 ScreenPtrAddress; // Screen*
typedef U32 DisplayPtrAddress; // Display*
typedef U32 VisualPtrAddress; // Visual*
typedef U32 DepthPtrAddress; // Depth*

typedef U32 XID;
typedef U32 Mask;
typedef U32 Atom;
typedef U32 VisualID;
typedef U32 Time;

typedef XID Window;
typedef XID Drawable;
typedef XID Font;
typedef XID Pixmap;
typedef XID Cursor;
typedef XID Colormap;
typedef XID GContext;
typedef XID KeySym;

#define XCSUCCESS 0	/* No error. */
#define XCNOMEM   1    /* Out of memory */
#define XCNOENT   2    /* No entry in table */

#define NoSymbol	     0L	/* special KeySym */

#define PropModeReplace         0
#define PropModePrepend         1
#define PropModeAppend          2

/*****************************************************************
 * EVENT DEFINITIONS
 *****************************************************************/

 /* Input Event Masks. Used as event-mask window attribute and as arguments
	to Grab requests.  Not to be confused with event names.  */

#define NoEventMask			0L
#define KeyPressMask			(1L<<0)
#define KeyReleaseMask			(1L<<1)
#define ButtonPressMask			(1L<<2)
#define ButtonReleaseMask		(1L<<3)
#define EnterWindowMask			(1L<<4)
#define LeaveWindowMask			(1L<<5)
#define PointerMotionMask		(1L<<6)
#define PointerMotionHintMask		(1L<<7)
#define Button1MotionMask		(1L<<8)
#define Button2MotionMask		(1L<<9)
#define Button3MotionMask		(1L<<10)
#define Button4MotionMask		(1L<<11)
#define Button5MotionMask		(1L<<12)
#define ButtonMotionMask		(1L<<13)
#define KeymapStateMask			(1L<<14)
#define ExposureMask			(1L<<15)
#define VisibilityChangeMask		(1L<<16)
#define StructureNotifyMask		(1L<<17)
#define ResizeRedirectMask		(1L<<18)
#define SubstructureNotifyMask		(1L<<19)
#define SubstructureRedirectMask	(1L<<20)
#define FocusChangeMask			(1L<<21)
#define PropertyChangeMask		(1L<<22)
#define ColormapChangeMask		(1L<<23)
#define OwnerGrabButtonMask		(1L<<24)

	/* Event names.  Used in "type" field in XEvent structures.  Not to be
	confused with event masks above.  They start from 2 because 0 and 1
	are reserved in the protocol for errors and replies. */

#define KeyPress		2
#define KeyRelease		3
#define ButtonPress		4
#define ButtonRelease		5
#define MotionNotify		6
#define EnterNotify		7
#define LeaveNotify		8
#define FocusIn			9
#define FocusOut		10
#define KeymapNotify		11
#define Expose			12
#define GraphicsExpose		13
#define NoExpose		14
#define VisibilityNotify	15
#define CreateNotify		16
#define DestroyNotify		17
#define UnmapNotify		18
#define MapNotify		19
#define MapRequest		20
#define ReparentNotify		21
#define ConfigureNotify		22
#define ConfigureRequest	23
#define GravityNotify		24
#define ResizeRequest		25
#define CirculateNotify		26
#define CirculateRequest	27
#define PropertyNotify		28
#define SelectionClear		29
#define SelectionRequest	30
#define SelectionNotify		31
#define ColormapNotify		32
#define ClientMessage		33
#define MappingNotify		34
#define GenericEvent		35
#define LASTEvent		36	/* must be bigger than any event # */

#define NotifyNormal		0
#define NotifyGrab		1
#define NotifyUngrab		2
#define NotifyWhileGrabbed	3

/* Notify detail */

#define NotifyAncestor		0
#define NotifyVirtual		1
#define NotifyInferior		2
#define NotifyNonlinear		3
#define NotifyNonlinearVirtual	4
#define NotifyPointer		5
#define NotifyPointerRoot	6
#define NotifyDetailNone	7

#define NotifyHint		1	/* for MotionNotify events */

#define XRAND_Base 10000
#define XRAND_Error_Base 11000

#define GrabModeSync		0
#define GrabModeAsync		1

#define GrabSuccess		0
#define AlreadyGrabbed		1
#define GrabInvalidTime		2
#define GrabNotViewable		3
#define GrabFrozen		4

#define ShiftMask		(1<<0)
#define LockMask		(1<<1)
#define ControlMask		(1<<2)
#define Mod1Mask		(1<<3)
#define Mod2Mask		(1<<4)
#define Mod3Mask		(1<<5)
#define Mod4Mask		(1<<6)
#define Mod5Mask		(1<<7)

#define NumMask Mod2Mask
#define ScrollMask Mod3Mask

/* modifier names.  Used to build a SetModifierMapping request or
   to read a GetModifierMapping request.  These correspond to the
   masks defined above. */
#define ShiftMapIndex		0
#define LockMapIndex		1
#define ControlMapIndex		2
#define Mod1MapIndex		3
#define Mod2MapIndex		4
#define Mod3MapIndex		5
#define Mod4MapIndex		6
#define Mod5MapIndex		7


   /* button masks.  Used in same manner as Key masks above. Not to be confused
	  with button names below. */

#define Button1Mask		(1<<8)
#define Button2Mask		(1<<9)
#define Button3Mask		(1<<10)

// v scroll up/down
#define Button4Mask		(1<<11)
#define Button5Mask		(1<<12)

// h scroll up/down
#define Button6Mask		(1<<13)
#define Button7Mask		(1<<14)

// x1
#define Button8Mask		(1<<15)
// x2
#define Button9Mask		(1<<16)

typedef unsigned char KeyCode;

typedef U32 GC; // actually a pointer in X11

struct XKeyEvent {
	S32 type;		/* of event */
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	S32 x, y;		/* pointer x, y coordinates in event window */
	S32 x_root, y_root;	/* coordinates relative to root */
	U32 state;	/* key or button mask */
	U32 keycode;	/* detail */
	Bool same_screen;	/* same screen flag */

	void read(KMemory* memory, U32 address);
};

typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

struct XButtonEvent {
	S32 type;		/* of event */
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	S32 x, y;		/* pointer x, y coordinates in event window */
	S32 x_root, y_root;	/* coordinates relative to root */
	U32 state;	/* key or button mask */
	U32 button;	/* detail */
	Bool same_screen;	/* same screen flag */
};
typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

struct XMotionEvent {
	S32 type;		/* of event */
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	S32 x, y;		/* pointer x, y coordinates in event window */
	S32 x_root, y_root;	/* coordinates relative to root */
	U32 state;	/* key or button mask */
	char is_hint;		/* detail */
	Bool same_screen;	/* same screen flag */
};
typedef XMotionEvent XPointerMovedEvent;

struct XCrossingEvent {
	S32 type;		/* of event */
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	S32 x, y;		/* pointer x, y coordinates in event window */
	S32 x_root, y_root;	/* coordinates relative to root */
	S32 mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	S32 detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior,
	 * NotifyNonlinear,NotifyNonlinearVirtual
	 */
	Bool same_screen;	/* same screen flag */
	Bool focus;		/* boolean focus */
	U32 state;	/* key or button mask */
};
typedef XCrossingEvent XEnterWindowEvent;
typedef XCrossingEvent XLeaveWindowEvent;

struct XFocusChangeEvent {
	S32 type;		/* FocusIn or FocusOut */
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;		/* window of event */
	S32 mode;		/* NotifyNormal, NotifyWhileGrabbed,
				   NotifyGrab, NotifyUngrab */
	S32 detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior,
	 * NotifyNonlinear,NotifyNonlinearVirtual, NotifyPointer,
	 * NotifyPointerRoot, NotifyDetailNone
	 */
};
typedef XFocusChangeEvent XFocusInEvent;
typedef XFocusChangeEvent XFocusOutEvent;

/* generated on EnterWindow and FocusIn  when KeyMapState selected */
struct XKeymapEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	char key_vector[32];
};

struct XExposeEvent  {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	S32 x, y;
	S32 width, height;
	S32 count;		/* if non-zero, at least this many more */
};

struct XGraphicsExposeEvent  {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Drawable drawable;
	S32 x, y;
	S32 width, height;
	S32 count;		/* if non-zero, at least this many more */
	S32 major_code;		/* core is CopyArea or CopyPlane */
	S32 minor_code;		/* not defined in the core */
};

struct XNoExposeEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Drawable drawable;
	S32 major_code;		/* core is CopyArea or CopyPlane */
	S32 minor_code;		/* not defined in the core */
};

struct XVisibilityEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	S32 state;		/* Visibility state */
};

struct XCreateWindowEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window parent;		/* parent of the window */
	Window window;		/* window id of window created */
	S32 x, y;		/* window location */
	S32 width, height;	/* size of window */
	S32 border_width;	/* border width */
	Bool override_redirect;	/* creation should be overridden */
};

struct XDestroyWindowEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window event;
	Window window;
};

struct XUnmapEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window event;
	Window window;
	Bool from_configure;
};

struct XMapEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window event;
	Window window;
	Bool override_redirect;	/* boolean, is override set... */
};

struct XMapRequestEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window parent;
	Window window;
};

struct XReparentEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window event;
	Window window;
	Window parent;
	S32 x, y;
	Bool override_redirect;
};

struct XConfigureEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window event;
	Window window;
	S32 x, y;
	S32 width, height;
	S32 border_width;
	Window above;
	Bool override_redirect;
};

struct XGravityEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window event;
	Window window;
	S32 x, y;
};

struct XResizeRequestEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	S32 width, height;
};

struct XConfigureRequestEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window parent;
	Window window;
	S32 x, y;
	S32 width, height;
	S32 border_width;
	Window above;
	S32 detail;		/* Above, Below, TopIf, BottomIf, Opposite */
	U32 value_mask;
};

struct XCirculateEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window event;
	Window window;
	S32 place;		/* PlaceOnTop, PlaceOnBottom */
};

struct XCirculateRequestEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window parent;
	Window window;
	S32 place;		/* PlaceOnTop, PlaceOnBottom */
};

struct XPropertyEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	Atom atom;
	Time time;
	S32 state;		/* NewValue, Deleted */
};

struct XSelectionClearEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	Atom selection;
	Time time;
};

struct XSelectionRequestEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window owner;
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;
	Time time;
};

struct XSelectionEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;		/* ATOM or None */
	Time time;
};

struct XColormapEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	Colormap colormap;	/* COLORMAP or None */
	Bool c_new;		/* C++ */
	S32 state;		/* ColormapInstalled, ColormapUninstalled */
};

struct XClientMessageEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;
	Atom message_type;
	S32 format;
	union {
		S8 b[20];
		S16 s[10];
		S32 l[5];
	} data;
};

struct XMappingEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;	/* Display the event was read from */
	Window window;		/* unused */
	S32 request;		/* one of MappingModifier, MappingKeyboard,
				   MappingPointer */
	S32 first_keycode;	/* first keycode */
	S32 count;		/* defines range of change w. first_keycode*/
};

struct XErrorEvent {
	S32 type;
	U32 display;	/* Display the event was read from */
	XID resourceid;		/* resource id */
	U32 serial;	/* serial number of failed request */
	U8 error_code;	/* error code of failed request */
	U8 request_code;	/* Major op-code of failed request */
	U8 minor_code;	/* Minor op-code of failed request */
};

struct XAnyEvent {
	S32 type;
	U32 serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	U32 display;/* Display the event was read from */
	Window window;	/* window on which event was requested in event mask */
};


/***************************************************************
 *
 * GenericEvent.  This event is the standard event for all newer extensions.
 */

struct XGenericEvent {
	S32            type;         /* of event. Always GenericEvent */
	U32  serial;       /* # of last request processed */
	Bool           send_event;   /* true if from SendEvent request */
	U32 display;     /* Display the event was read from */
	S32            extension;    /* major opcode of extension that caused the event */
	S32            evtype;       /* actual event type. */
};

struct XGenericEventCookie {
	S32            type;         /* of event. Always GenericEvent */
	U32  serial;       /* # of last request processed */
	Bool           send_event;   /* true if from SendEvent request */
	U32 display;     /* Display the event was read from */
	S32            extension;    /* major opcode of extension that caused the event */
	S32            evtype;       /* actual event type. */
	U32   cookie;
	U32 data;
};

/*
 * this union is defined so Xlib can always use the same sized
 * event structure internally, to avoid memory fragmentation.
 */
union XEvent {
	S32 type;		/* must not be changed; first element */
	XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclient;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
	XGenericEvent xgeneric;
	XGenericEventCookie xcookie;
	U32 pad[24];
};

/* Property notification */

#define PropertyNewValue	0
#define PropertyDelete		1

struct XPixmapFormatValues {
	U32 depth;
	U32 bits_per_pixel;
	U32 scanline_pad;

	static U32 write(KMemory* memory, U32 address, U32 depth, U32 bits_per_pixel, U32 scanline_pad);
};

struct XExtData {
	S32 number;		/* number returned by XRegisterExtension */
	XExtDataPtrAddress next;	/* next item on list of data for structure */
	//int (*free_private)(	/* called to free private storage */struct _XExtData* extension);
	U32 free_private;
	XPointer private_data;	/* data private to this extension. */
};

struct ScreenFormat {
	XExtDataPtrAddress ext_data;	/* hook for extension to hang data */
	S32 depth;		/* depth of this image format */
	S32 bits_per_pixel;	/* bits/pixel at this depth */
	S32 scanline_pad;	/* scanline must padded to this multiple */
};

struct XModifierKeymap  {
	U32 max_keypermod;	/* This server's max number of keys per modifier */
	U32 /* KeyCode* */ modifiermap;	/* An 8 by max_keypermod array of the modifiers */
};

struct XPoint {
	S16 x, y;
};

struct XRectangle {
	XRectangle() {}
	XRectangle(KMemory* memory, U32 address) { read(memory, address); }

	S16 x, y;
	U16 width, height;

	void read(KMemory* memory, U32 address);
};

class DisplayData;
class XrrData;

#define None                 0	/* universal null resource or null atom */
#define DummyAtom           99

#define AllocNone		0	/* create map with no entries */
#define AllocAll		1	/* allocate entire map writeable */

#define DoRed			(1<<0)
#define DoGreen			(1<<1)
#define DoBlue			(1<<2)

#define VisualNoMask		0x0
#define VisualIDMask 		0x1
#define VisualScreenMask	0x2
#define VisualDepthMask		0x4
#define VisualClassMask		0x8
#define VisualRedMaskMask	0x10
#define VisualGreenMaskMask	0x20
#define VisualBlueMaskMask	0x40
#define VisualColormapSizeMask	0x80
#define VisualBitsPerRGBMask	0x100
#define VisualAllMask		0x1FF

#define XDrawablePtr std::shared_ptr<XDrawable>
#define XGCPtr std::shared_ptr<XGC>

#define PointerWindow        0L	/* destination window in SendEvent */
#define InputFocus           1L	/* destination window in SendEvent */

#define PointerRoot          1L	/* focus window in SetInputFocus */

#define XYBitmap		0	/* depth 1, XYFormat */
#define XYPixmap		1	/* depth == drawable depth */
#define ZPixmap			2	/* depth == drawable depth */

#define AllPlanes 		((U32)~0)

class XGC;
class XDrawable;

#include "xcolor.h"
#include "xvisual.h"
#include "xdepth.h"
#include "xvisualinfo.h"
#include "xdisplay.h"
#include "xscreen.h"
#include "displaydata.h"
#include "xkeyboard.h"
#include "xrandr.h"
#include "xcolormap.h"
#include "xgc.h"
#include "xproperties.h"
#include "ximage.h"
#include "xdrawable.h"
#include "xpixmap.h"
#include "xcursor.h"
#include "xwindow.h"
#include "xfbconfig.h"
#include "xserver.h"
#include "xinput2.h"

#define X11_WRITED(c, a, f, v) memory->writed(a + offsetof(c, f), v)
#define X11_READD(c, a, f) memory->readd(a + offsetof(c, f))

#endif
