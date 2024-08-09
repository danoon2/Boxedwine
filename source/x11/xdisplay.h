#ifndef __X_DISPLAY_H__
#define __X_DISPLAY_H__

// used directly by winex11
// fd // fcntl( ConnectionNumber(display), F_SETFD, 1 ); /* set close on exec flag */
// default_screen // init_visuals( display, DefaultScreen( display ));
// screens // root_window = DefaultRootWindow(display);
struct Display
{
	/*  0 */ XExtDataPtrAddress ext_data;	/* hook for extension to hang data */
	/*  4 */ XPrivatePtrAddress private1;
	/*  8 */ S32 fd;			/* Network socket. */
	/*  C */ S32 private2;
	/* 10 */ S32 proto_major_version;/* major version of server's X protocol */
	/* 14 */ S32 proto_minor_version;/* minor version of servers X protocol */
	/* 18 */ XCharPtrAddress vendor;		/* vendor of the server hardware */
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
	/* 44 */ ScreenFormatPtrAddress pixmap_format;	/* pixmap format list */
	/* 48 */ S32 private8;
	/* 4C */ S32 release;		/* release of the server */
	/* 50 */ XPrivatePtrAddress private9;
	/* 54 */ XPrivatePtrAddress private10;
	/* 58 */ S32 qlen;		/* Length of input event queue */
	/* 5C */ U32 last_request_read; /* seq number of last event read */
	/* 60 */ U32 request;	/* sequence number of last request. */
	/* 64 */ XPointer private11;
	/* 68 */ XPointer private12;
	/* 6C */ XPointer private13;
	/* 70 */ XPointer private14;
	/* 74 */ U32 max_request_size; /* maximum number 32 bit words in request*/
	/* 78 */ XrmHashBucketRecPtrAddress db;
	// int (*private15)(struct _XDisplay*);
	/* 7C */ U32 private15;
	/* 80 */ XCharPtrAddress display_name;	/* "host:display" string used on this connect*/
	/* 84 */ S32 default_screen;	/* default screen for operations */
	/* 88 */ S32 nscreens;		/* number of screens on this server*/
	/* 8C */ ScreenPtrAddress screens;	/* pointer to list of screens */
	/* 90 */ U32 motion_buffer;	/* size of motion buffer */
	/* 94 */ U32 private16;
	/* 98 */ S32 min_keycode;	/* minimum defined keycode */
	/* 9C */ S32 max_keycode;	/* maximum defined keycode */
	/* A0 */ XPointer private17;
	/* A4 */ XPointer private18;
	/* A8 */ S32 private19;
	/* AC */ XCharPtrAddress xdefaults;	/* contents of defaults from server */
	/* there is more to this structure, but it is private to Xlib */	
	/* B0 */ U32 id;

	static void iterateVisuals(KThread* thread, U32 displayAddress, std::function<bool(S32 screenIndex, U32 visualAddress, Depth* depth, Visual* visual)> pfn);
};

#endif