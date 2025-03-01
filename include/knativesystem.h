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

#ifndef __KNATIVESYSTEM_H__
#define __KNATIVESYSTEM_H__

#define SCALE_DENOMINATOR 1000

#include "knativeinput.h"
#include "knativescreen.h"
#include "kopengl.h"
#include "kvulkan.h"

class XWindow;

class KNativeSystem {
public:
	static bool init(VideoOption videoOption, bool allowAudio);
	static void initWindow(U32 cx, U32 cy, U32 bpp, int scaleX, int scaleY, const BString& scaleQuality, U32 fullScreen, U32 vsync);
	static void exit(const char* msg, U32 code);
	static void forceShutdown();
	static void cleanup();
	static void preReturnToUI();

	static void postQuit();
	static U32 getTicks();
	static bool getScreenDimensions(U32* width, U32* height);
	static BString getAppDirectory();
	static BString getLocalDirectory(); // ends with path separator
	static bool clipboardHasText();
	static BString clipboardGetText();
	static bool clipboardSetText(BString text);
	static U32 getDpiScale(); // returns 1/1000th of the scale, so a result of 1000 would mean no scaling

	static KNativeInputPtr getCurrentInput();
	static KNativeScreenPtr getScreen();
	static KOpenGLPtr getOpenGL();
	static KVulkanPtr getVulkan();
	static void showScreen(bool show);
	static void tick();
	static void warpMouse(S32 x, S32 y);

	static void scheduledNewThread(KThread* thread);
	static void changeScreenSize(U32 cx, U32 cy);

	static void moveWindow(const std::shared_ptr<XWindow>& wnd);
	static void showWindow(const std::shared_ptr<XWindow>& wnd, bool bShow);

	static void shutdown();
};

#endif