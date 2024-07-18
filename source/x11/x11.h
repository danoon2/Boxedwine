#ifndef __X11_H__
#define __X11_H__
#include "displaydata.h"

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

typedef U32 XExtDataPtr; // struct _XExtData*
typedef U32 XCharPtr; // char*
typedef U32 XPrivatePtr; // struct _XPrivate*
typedef U32 XrmHashBucketRecPtr; // struct _XrmHashBucketRec*
typedef U32 ScreenFormatPtr; // ScreenFormat*
typedef U32 ScreenPtr; // Screen*
typedef U32 DisplayPtr; // Display*
typedef U32 VisualPtr; // Visual*
typedef U32 DepthPtr; // Depth*

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

typedef unsigned char KeyCode;

typedef U32 GC; // actually a pointer in X11

struct XPixmapFormatValues {
	U32 depth;
	U32 bits_per_pixel;
	U32 scanline_pad;
};

struct XExtData {
	S32 number;		/* number returned by XRegisterExtension */
	XExtDataPtr next;	/* next item on list of data for structure */
	//int (*free_private)(	/* called to free private storage */struct _XExtData* extension);
	U32 free_private;
	XPointer private_data;	/* data private to this extension. */
};

struct ScreenFormat {
	XExtDataPtr ext_data;	/* hook for extension to hang data */
	S32 depth;		/* depth of this image format */
	S32 bits_per_pixel;	/* bits/pixel at this depth */
	S32 scanline_pad;	/* scanline must padded to this multiple */
};

struct Visual {
	XExtDataPtr ext_data;	/* hook for extension to hang data */
	VisualID visualid;	/* visual id of this visual */
	S32 c_class;		/* class of screen (monochrome, etc.) */
	U32 red_mask, green_mask, blue_mask;	/* mask values */
	S32 bits_per_rgb;	/* log base 2 of distinct color values */
	S32 map_entries;	/* color map entries */
};

struct Depth {
	S32 depth;		/* this depth (Z) of the depth */
	S32 nvisuals;		/* number of Visual types at this depth */
	VisualPtr visuals;	/* list of visuals possible at this depth */

	Visual* getVisual(KThread* thread, S32 visual, U32* address = nullptr);
};

struct XPoint {
	S16 x, y;
};

struct XRectangle {
	S16 x, y;
	U16 width, height;
};

// maintain emulate byte layout because of things like
// #define ConnectionNumber(dpy) 	(((_XPrivDisplay)(dpy))->fd)

// root // root_window = DefaultRootWindow(display);
struct Screen {
/*  0 */ XExtDataPtr ext_data;	/* hook for extension to hang data */
/*  4 */ DisplayPtr display;/* back pointer to display structure */
/*  8 */ Window root;		/* Root window id. */
/*  C */ S32 width;
/* 10 */ S32 height;
/* 14 */ S32 mwidth;
/* 18 */ S32 mheight;
/* 1C */ S32 ndepths;		/* number of depths possible */
/* 20 */ DepthPtr depths;		/* list of allowable depths on the screen */
/* 24 */ S32 root_depth;		/* bits per pixel */
/* 28 */ VisualPtr root_visual;	/* root visual */
/* 2C */ GC default_gc;		/* GC for the root root visual */
/* 30 */ Colormap cmap;		/* default color map */
/* 34 */ U32 white_pixel;
/* 38 */ U32 black_pixel;	/* White and Black pixel values */
/* 3C */ S32 max_maps;
/* 40 */ S32 min_maps;	/* max and min color maps */
/* 44 */ S32 backing_store;	/* Never, WhenMapped, Always */
/* 48 */ Bool save_unders;
/* 4C */ S32 root_input_mask;	/* initial root input mask */

	Depth* getDepth(KThread* thread, S32 depth);
};

// used directly by winex11
// fd // fcntl( ConnectionNumber(display), F_SETFD, 1 ); /* set close on exec flag */
// default_screen // init_visuals( display, DefaultScreen( display ));
// screens // root_window = DefaultRootWindow(display);
struct Display
{
/*  0 */ XExtDataPtr ext_data;	/* hook for extension to hang data */
/*  4 */ XPrivatePtr private1;
/*  8 */ S32 fd;			/* Network socket. */
/*  C */ S32 private2;
/* 10 */ S32 proto_major_version;/* major version of server's X protocol */
/* 14 */ S32 proto_minor_version;/* minor version of servers X protocol */
/* 18 */ XCharPtr vendor;		/* vendor of the server hardware */
/* 1C */ XID private3;
/* 20 */ XID private4;
/* 24 */ XID private5;
/* 28 */ S32 private6;
	// XID(*resource_alloc)(	/* allocator function */ struct _XDisplay*);
/* 2C */ U32 resource_alloc;
/* 30 */ S32 byte_order;		/* screen byte order, LSBFirst, MSBFirst */
/* 34 */ S32 bitmap_unit;	/* padding and data requirements */
/* 38 */ S32 bitmap_pad;		/* padding requirements on bitmaps */
/* 3C */ S32 bitmap_bit_order;	/* LeastSignificant or MostSignificant */
/* 40 */ S32 nformats;		/* number of pixmap formats in list */
/* 44 */ ScreenFormatPtr pixmap_format;	/* pixmap format list */
/* 48 */ S32 private8;
/* 4C */ S32 release;		/* release of the server */
/* 50 */ XPrivatePtr private9;
/* 54 */ XPrivatePtr private10;
/* 58 */ S32 qlen;		/* Length of input event queue */
/* 5C */ U32 last_request_read; /* seq number of last event read */
/* 60 */ U32 request;	/* sequence number of last request. */
/* 64 */ XPointer private11;
/* 68 */ XPointer private12;
/* 6C */ XPointer private13;
/* 70 */ XPointer private14;
/* 74 */ U32 max_request_size; /* maximum number 32 bit words in request*/
/* 78 */ XrmHashBucketRecPtr db;
	// int (*private15)(struct _XDisplay*);
/* 7C */ U32 private15;
/* 80 */ XCharPtr display_name;	/* "host:display" string used on this connect*/
/* 84 */ S32 default_screen;	/* default screen for operations */
/* 88 */ S32 nscreens;		/* number of screens on this server*/
/* 8C */ ScreenPtr screens;	/* pointer to list of screens */
/* 90 */ U32 motion_buffer;	/* size of motion buffer */
/* 94 */ U32 private16;
/* 98 */ S32 min_keycode;	/* minimum defined keycode */
/* 9C */ S32 max_keycode;	/* maximum defined keycode */
/* A0 */ XPointer private17;
/* A4 */ XPointer private18;
/* A8 */ S32 private19;
/* AC */ XCharPtr xdefaults;	/* contents of defaults from server */
	/* there is more to this structure, but it is private to Xlib */
	DisplayData* data;

	U32 alloc(KThread* thread, U32 len);
	void free(U32 address);
	U32 createString(KThread* thread, const BString& str);
	Screen* getScreen(KThread* thread, S32 screen);
	void iterateVisuals(KThread* thread, std::function<bool(S32 screenIndex, U32 visualAddress, Screen* screen, Depth* depth, Visual* visual)> pfn);
};

class X11 {
public:
	static U32 openDisplay(KThread* thread);
	static Display* getCurrentProcessDisplay(KThread* thread);
	static Display* getDisplay(KThread* thread, U32 address);
	static Visual* getVisual(KThread* thread, U32 address);
};

#define None                 0	/* universal null resource or null atom */
#define DummyAtom           99
#define VisualIdBase       1000

#define AllocNone		0	/* create map with no entries */
#define AllocAll		1	/* allocate entire map writeable */

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

struct XVisualInfo {
	VisualPtr visual;
	VisualID visualid;
	S32 screen;
	S32 depth;
	S32 c_class;
	U32 red_mask;
	U32 green_mask;
	U32 blue_mask;
	S32 colormap_size;
	S32 bits_per_rgb;

	void set(S32 screenIndex, U32 visualAddress, Screen* screen, Depth* depth, Visual* visual);
	void read(KMemory* memory, U32 address);
	bool match(U32 mask, S32 screenIndex, const Screen* screen, const Depth* depth, const Visual* visual);
};

#endif