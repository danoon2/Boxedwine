#ifndef __X_SCREEN_H__
#define __X_SCREEN_H__

// maintain emulate byte layout because of things like
// #define ConnectionNumber(dpy) 	(((_XPrivDisplay)(dpy))->fd)

// root // root_window = DefaultRootWindow(display);
struct Screen {
	/*  0 */ XExtDataPtrAddress ext_data;	/* hook for extension to hang data */
	/*  4 */ DisplayPtrAddress display;/* back pointer to display structure */
	/*  8 */ Window root;		/* Root window id. */
	/*  C */ S32 width;
	/* 10 */ S32 height;
	/* 14 */ S32 mwidth;
	/* 18 */ S32 mheight;
	/* 1C */ S32 ndepths;		/* number of depths possible */
	/* 20 */ DepthPtrAddress depths;		/* list of allowable depths on the screen */
	/* 24 */ S32 root_depth;		/* bits per pixel */
	/* 28 */ VisualPtrAddress root_visual;	/* root visual */
	/* 2C */ GC default_gc;		/* GC for the root root visual */
	/* 30 */ Colormap cmap;		/* default color map */
	/* 34 */ U32 white_pixel;
	/* 38 */ U32 black_pixel;	/* White and Black pixel values */
	/* 3C */ S32 max_maps;
	/* 40 */ S32 min_maps;	/* max and min color maps */
	/* 44 */ S32 backing_store;	/* Never, WhenMapped, Always */
	/* 48 */ Bool save_unders;
	/* 4C */ S32 root_input_mask;	/* initial root input mask */

	void read(KMemory* memory, U32 address);
};

#endif