#ifndef __KNATIVESYSTEM_H__
#define __KNATIVESYSTEM_H__

#define SCALE_DENOMINATOR 1000

#include "knativeinput.h"
#include "knativescreen.h"
#include "kopengl.h"

class KNativeSystem {
public:
	static bool init(bool allowVideo, bool allowAudio);
	static void initWindow(U32 cx, U32 cy, U32 bpp, int scaleX, int scaleY, const BString& scaleQuality, U32 fullScreen, U32 vsync);
	static void exit(const char* msg, U32 code);
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

	static void scheduledNewThread(KThread* thread);
	static void changeScreenSize(U32 cx, U32 cy);
	static void windowDestroyed(U32 id);

	static void moveWindow(U32 id, S32 x, S32 y, U32 w, U32 h);
	static void showWindow(U32 id, bool bShow);

	static void shutdown();
};

#endif