#include "boxedwine.h"
#include "x11.h"

XGC::XGC(const std::shared_ptr<XDrawable>& drawable) : id(XServer::getNextId()), drawable(drawable) {
	values.function = GXcopy;
	values.plane_mask = 0xffffffff;
	values.foreground = 0;
	values.background = 0;
	values.line_width = 0;
	values.line_style = LineSolid;
	values.cap_style = CapButt;
	values.join_style = JoinMiter;
	values.fill_style = FillSolid;
	values.fill_rule = EvenOddRule;
	values.arc_mode = ArcPieSlice;
	values.tile = 0; // Pixmap of unspecified size filled with foreground pixel (that is, client specified pixel if any, else 0) (subsequent changes to foreground do not affect this pixmap) 
	values.stipple = 0; // Pixmap of unspecified size filled with ones 
	values.ts_x_origin = 0;
	values.ts_y_origin = 0;
	values.font = 0; // <implementation dependent> 
	values.subwindow_mode = ClipByChildren;
	values.graphics_exposures = True;
	values.clip_x_origin = 0;
	values.clip_y_origin = 0;
	values.clip_mask = 0;
	values.dash_offset = 0;
	values.dashes = 4;
}

void XGC::updateValues(U32 mask, XGCValues* newValues) {
	if (mask & GCFunction) {
		values.function = newValues->function;
	}
	if (mask & GCPlaneMask) {
		values.plane_mask = newValues->plane_mask;
	}
	if (mask & GCForeground) {
		values.foreground = newValues->foreground;
	}
	if (mask & GCBackground) {
		values.background = newValues->background;
	}
	if (mask & GCLineWidth) {
		values.line_width = newValues->line_width;
	}
	if (mask & GCLineStyle) {
		values.line_style = newValues->line_style;
	}
	if (mask & GCCapStyle) {
		values.cap_style = newValues->cap_style;
	}
	if (mask & GCJoinStyle) {
		values.join_style = newValues->join_style;
	}
	if (mask & GCFillStyle) {
		values.fill_style = newValues->fill_style;
	}
	if (mask & GCFillRule) {
		values.fill_rule = newValues->fill_rule;
	}
	if (mask & GCTile) {
		values.tile = newValues->tile;
	}
	if (mask & GCStipple) {
		values.stipple = newValues->stipple;
	}
	if (mask & GCTileStipXOrigin) {
		values.ts_x_origin = newValues->ts_x_origin;
	}
	if (mask & GCTileStipYOrigin) {
		values.ts_y_origin = newValues->ts_y_origin;
	}
	if (mask & GCFont) {
		values.font = newValues->font;
	}
	if (mask & GCSubwindowMode) {
		values.subwindow_mode = newValues->subwindow_mode;
	}
	if (mask & GCGraphicsExposures) {
		values.graphics_exposures = newValues->graphics_exposures;
	}
	if (mask & GCClipXOrigin) {
		values.clip_x_origin = newValues->clip_x_origin;
	}
	if (mask & GCClipYOrigin) {
		values.clip_y_origin = newValues->clip_y_origin;
	}
	if (mask & GCClipMask) {
		values.clip_mask = newValues->clip_mask;
	}
	if (mask & GCDashOffset) {
		values.dash_offset = newValues->dash_offset;
	}
	if (mask & GCDashList) {
		values.dashes = newValues->dashes;
	}
	if (mask & GCArcMode) {
		values.arc_mode = newValues->arc_mode;
	}
}

void XGCValues::read(KMemory* memory, U32 address) {
	function = (S32)memory->readd(address);
	plane_mask = memory->readd(address + 4);
	foreground = memory->readd(address + 8);
	background = memory->readd(address + 12);
	line_width = (S32)memory->readd(address + 16);
	line_style = (S32)memory->readd(address + 20);
	cap_style = (S32)memory->readd(address + 24);
	join_style = (S32)memory->readd(address + 28);
	fill_style = (S32)memory->readd(address + 32);
	fill_rule = (S32)memory->readd(address + 36);
	arc_mode = (S32)memory->readd(address + 40);
	tile = memory->readd(address + 44);
	stipple = memory->readd(address + 48);
	ts_x_origin = (S32)memory->readd(address + 52);
	ts_y_origin = (S32)memory->readd(address + 56);
	font = memory->readd(address + 60);
	subwindow_mode = (S32)memory->readd(address + 64);
	graphics_exposures = (S32)memory->readd(address + 68);
	clip_x_origin = (S32)memory->readd(address + 72);
	clip_y_origin = (S32)memory->readd(address + 76);
	clip_mask = memory->readd(address + 80);
	dash_offset = (S32)memory->readd(address + 84);
	dashes = (S8)memory->readb(address + 88);
}