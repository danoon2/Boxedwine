/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "X11_def.h"

SizeID XRRConfigCurrentConfiguration(XRRScreenConfiguration* config, Rotation* rotation) {
	CALL_2_R(X11_XRR_CONFIG_CURRENT_CONFIGURATION, config, rotation);
}

short XRRConfigCurrentRate(XRRScreenConfiguration* config) {
	CALL_1_R(X11_XRR_CONFIG_CURRENT_RATE, config);
}

void XRRFreeScreenConfigInfo(XRRScreenConfiguration* config) {
	CALL_1(X11_XRR_FREE_SCREEN_CONFIG_INFO, config);
}

XRRScreenConfiguration* XRRGetScreenInfo(Display* dpy, Drawable draw) {
	CALL_2_R(X11_XRR_GET_SCREEN_INFO, dpy, draw);
}

Bool XRRQueryExtension(Display* dpy, int* event_base_return, int* error_base_return) {
	CALL_3_R(X11_XRR_QUERY_EXTENSION, dpy, event_base_return, error_base_return);
}

Status XRRQueryVersion(Display* dpy, int* major_version_return, int* minor_version_return) {
	CALL_3_R(X11_XRR_QUERY_VERSION, dpy, major_version_return, minor_version_return);
}

short* XRRRates(Display* dpy, int screen, int size_index, int* nrates) {
	CALL_4_R(X11_XRR_RATES, dpy, screen, size_index, nrates);
}

Status XRRSetScreenConfig(Display* dpy, XRRScreenConfiguration* config, Drawable draw, int size_index, Rotation rotation, Time timestamp) {
	CALL_6_R(X11_XRR_SET_SCREEN_CONFIG, dpy, config, draw, size_index, rotation, timestamp);
}

Status XRRSetScreenConfigAndRate(Display* dpy, XRRScreenConfiguration* config, Drawable draw, int size_index, Rotation rotation, short rate, Time timestamp) {
	CALL_7_R(X11_XRR_SET_SCREEN_CONFIG_AND_RATE, dpy, config, draw, size_index, rotation, rate, timestamp);
}

XRRScreenSize* XRRSizes(Display* dpy, int screen, int* nsizes) {
	CALL_3_R(X11_XRR_SIZES, dpy, screen, nsizes);
}

void XRRFreeCrtcInfo(XRRCrtcInfo* crtcInfo) {
	CALL_1(X11_XRR_FREE_CRTC_INFO, crtcInfo);
}

void XRRFreeOutputInfo(XRROutputInfo* outputInfo) {
	CALL_1(X11_XRR_FREE_OUTPUT_INFO, outputInfo);
}

void XRRFreeScreenResources(XRRScreenResources* resources) {
	CALL_1(X11_XRR_FREE_SCREEN_RESOURCES, resources);
}

XRRCrtcInfo* XRRGetCrtcInfo(Display* dpy, XRRScreenResources* resources, RRCrtc crtc) {
	CALL_3_R(X11_XRR_GET_CRTC_INFO, dpy, resources, crtc);
}

XRROutputInfo* XRRGetOutputInfo(Display* dpy, XRRScreenResources* resources, RROutput output) {
	CALL_3_R(X11_XRR_GET_OUTPUT_INFO, dpy, resources, output);
}

int XRRGetOutputProperty(Display* dpy, RROutput output, Atom property, long offset, long length, Bool _delete, Bool pending, Atom req_type, Atom* actual_type, int* actual_format, unsigned long* nitems, unsigned long* bytes_after, unsigned char** prop) {
	CALL_13_R(X11_XRR_GET_OUTPUT_PROPERTY, dpy, output, property, offset, length, _delete, pending, req_type, actual_type, actual_format, nitems, bytes_after, prop);
}

XRRScreenResources* XRRGetScreenResources(Display* dpy, Window window) {
	CALL_2_R(X11_XRR_GET_SCREEN_RESOURCES, dpy, window);
}

XRRScreenResources* XRRGetScreenResourcesCurrent(Display* dpy, Window window) {
	CALL_2_R(X11_XRR_GET_SCREEN_RESOURCES_CURRENT, dpy, window);
}

Status XRRGetScreenSizeRange(Display* dpy, Window window, int* minWidth, int* minHeight, int* maxWidth, int* maxHeight) {
	CALL_6_R(X11_XRR_GET_SCREEN_SIZE_RANGE, dpy, window, minWidth, minHeight, maxWidth, maxHeight);
}

Status XRRSetCrtcConfig(Display* dpy, XRRScreenResources* resources, RRCrtc crtc, Time timestamp, int x, int y, RRMode mode, Rotation rotation, RROutput* outputs, int noutputs) {
	CALL_10_R(X11_XRR_SET_CRTC_CONFIG, dpy, resources, crtc, timestamp, x, y, mode, rotation, outputs, noutputs);
}

void XRRSetScreenSize(Display* dpy, Window window, int width, int height, int mmWidth, int mmHeight) {
	CALL_6(X11_XRR_SET_SCREEN_SIZE, dpy, window, width, height, mmWidth, mmHeight);
}

void XRRSelectInput(Display* dpy, Window window, int mask) {
	CALL_3(X11_XRR_SELECT_INPUT, dpy, window, mask);
}

RROutput XRRGetOutputPrimary(Display* dpy, Window window) {
	CALL_2_R(X11_XRR_GET_OUTPUT_PRIMARY, dpy, window);
}

XRRProviderResources* XRRGetProviderResources(Display* dpy, Window window) {
	CALL_2_R(X11_XRR_GET_PROVIDER_RESOURCES, dpy, window);
}

void XRRFreeProviderResources(XRRProviderResources* resources) {
	CALL_1(X11_XRR_FREE_PROVIDER_RESOURCES, resources);
}

XRRProviderInfo* XRRGetProviderInfo(Display* dpy, XRRScreenResources* resources, RRProvider provider) {
	CALL_3_R(X11_XRR_GET_PROVIDER_INFO, dpy, resources, provider);
}

void XRRFreeProviderInfo(XRRProviderInfo* provider) {
	CALL_1(X11_XRR_FREE_PROVIDER_INFO, provider)
}