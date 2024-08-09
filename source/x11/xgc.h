#ifndef __X_GC_H__
#define __X_GC_H__

// subwindow_mode
#define ClipByChildren		0
#define IncludeInferiors	1

#define GCFunction		(1L<<0)
#define GCPlaneMask		(1L<<1)
#define GCForeground		(1L<<2)
#define GCBackground		(1L<<3)
#define GCLineWidth		(1L<<4)
#define GCLineStyle		(1L<<5)
#define GCCapStyle		(1L<<6)
#define GCJoinStyle		(1L<<7)
#define GCFillStyle		(1L<<8)
#define GCFillRule		(1L<<9)
#define GCTile			(1L<<10)
#define GCStipple		(1L<<11)
#define GCTileStipXOrigin	(1L<<12)
#define GCTileStipYOrigin	(1L<<13)
#define GCFont			(1L<<14)
#define GCSubwindowMode		(1L<<15)
#define GCGraphicsExposures	(1L<<16)
#define GCClipXOrigin		(1L<<17)
#define GCClipYOrigin		(1L<<18)
#define GCClipMask		(1L<<19)
#define GCDashOffset		(1L<<20)
#define GCDashList		(1L<<21)
#define GCArcMode		(1L<<22)

// graphics functions
#define	GXclear			0x0		/* 0 */
#define GXand			0x1		/* src AND dst */
#define GXandReverse		0x2		/* src AND NOT dst */
#define GXcopy			0x3		/* src */
#define GXandInverted		0x4		/* NOT src AND dst */
#define	GXnoop			0x5		/* dst */
#define GXxor			0x6		/* src XOR dst */
#define GXor			0x7		/* src OR dst */
#define GXnor			0x8		/* NOT src AND NOT dst */
#define GXequiv			0x9		/* NOT src XOR dst */
#define GXinvert		0xa		/* NOT dst */
#define GXorReverse		0xb		/* src OR NOT dst */
#define GXcopyInverted		0xc		/* NOT src */
#define GXorInverted		0xd		/* NOT src OR dst */
#define GXnand			0xe		/* NOT src OR NOT dst */
#define GXset			0xf		/* 1 */

/* LineStyle */

#define LineSolid		0
#define LineOnOffDash		1
#define LineDoubleDash		2

/* capStyle */

#define CapNotLast		0
#define CapButt			1
#define CapRound		2
#define CapProjecting		3

/* joinStyle */

#define JoinMiter		0
#define JoinRound		1
#define JoinBevel		2

/* fillStyle */

#define FillSolid		0
#define FillTiled		1
#define FillStippled		2
#define FillOpaqueStippled	3

/* fillRule */

#define EvenOddRule		0
#define WindingRule		1

/* subwindow mode */

#define ClipByChildren		0
#define IncludeInferiors	1

/* SetClipRectangles ordering */

#define Unsorted		0
#define YSorted			1
#define YXSorted		2
#define YXBanded		3

/* CoordinateMode for drawing routines */

#define CoordModeOrigin		0	/* relative to the origin */
#define CoordModePrevious       1	/* relative to previous point */

/* Polygon shapes */

#define Complex			0	/* paths may intersect */
#define Nonconvex		1	/* no paths intersect, but not convex */
#define Convex			2	/* wholly convex */

/* Arc modes for PolyFillArc */

#define ArcChord		0	/* join endpoints of arc */
#define ArcPieSlice		1	/* join endpoints to center of arc */

struct XGCValues {
	S32 function;		/* logical operation */
	U32 plane_mask;/* plane mask */
	U32 foreground;/* foreground pixel */
	U32 background;/* background pixel */
	S32 line_width;		/* line width */
	S32 line_style;	 	/* LineSolid, LineOnOffDash, LineDoubleDash */
	S32 cap_style;	  	/* CapNotLast, CapButt,
				   CapRound, CapProjecting */
	S32 join_style;	 	/* JoinMiter, JoinRound, JoinBevel */
	S32 fill_style;	 	/* FillSolid, FillTiled,
				   FillStippled, FillOpaqueStippled */
	S32 fill_rule;	  	/* EvenOddRule, WindingRule */
	S32 arc_mode;		/* ArcChord, ArcPieSlice */
	Pixmap tile;		/* tile pixmap for tiling operations */
	Pixmap stipple;		/* stipple 1 plane pixmap for stippling */
	S32 ts_x_origin;	/* offset for tile or stipple operations */
	S32 ts_y_origin;
	Font font;	        /* default text font for text operations */
	S32 subwindow_mode;     /* ClipByChildren, IncludeInferiors */
	Bool graphics_exposures;/* boolean, should exposures be generated */
	S32 clip_x_origin;	/* origin for clipping */
	S32 clip_y_origin;
	Pixmap clip_mask;	/* bitmap clipping; other calls for rects */
	S32 dash_offset;	/* patterned/dashed line information */
	S8 dashes;

	void read(KMemory* memory, U32 address);
};

class XGC {
public:
	XGC(const XDrawablePtr& drawable);

	const U32 id;
	std::vector<XRectangle> clip_rects;

	void updateValues(U32 mask, XGCValues* values);
	XGCValues values;
private:
	XDrawablePtr drawable;
};

#endif