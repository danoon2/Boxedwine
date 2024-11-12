#ifndef __KNATIVESCREEN_H__
#define __KNATIVESCREEN_H__

#include "knativeinput.h"

class XCursor;

class KNativeScreen {
public:
	virtual ~KNativeScreen() {}
	virtual KNativeInputPtr getInput() = 0;

	virtual void setScreenSize(U32 cx, U32 cy) = 0;

	virtual U32 screenBpp() = 0;
	virtual U32 screenRate() = 0;	
	virtual U32 screenWidth() = 0;
	virtual U32 screenHeight() = 0;

	virtual void setTitle(const BString& title) = 0;
	virtual void showWindow(bool show) = 0;
	virtual void getPos(S32& x, S32& y) = 0;
	virtual U32 getLastUpdateTime() = 0;

	virtual void clear() = 0;
	// id is used for texture caching
	// if isDirty is false then the cached texture will be drawn
	virtual void putBitsOnWnd(U32 id, U8* bits, U32 bitsPerPixel, U32 srcPitch, S32 dstX, S32 dstY, U32 width, U32 height, U32* palette, bool isDirty) = 0;
	virtual void present() = 0;
	virtual bool presentedSinceLastCheck() = 0;
	virtual void clearTextureCache(U32 id) = 0;	

#ifdef BOXEDWINE_RECORDER
	virtual void startRecorderScreenShot() = 0;
	virtual void finishRecorderScreenShot() = 0;
	virtual void drawRectOnPushedSurfaceAndDisplay(U32 x, U32 y, U32 w, U32 h, U8 r, U8 g, U8 b, U8 a) = 0;
#endif

	virtual bool partialScreenShot(const BString& filepath, U32 x, U32 y, U32 w, U32 h, U8* buffer, U32 bufferlen) = 0;
	virtual bool screenShot(const BString& filepath, U8* buffer, U32 bufferlen) = 0;
	virtual bool saveBmp(const BString& filepath, U8* buffer, U32 bpp, U32 w, U32 h) = 0;

	virtual void setCursor(const std::shared_ptr<XCursor>& cursor) = 0;
	virtual void buildCursor(KThread* thread, const std::shared_ptr<XCursor>& cursor, U32 pixelsAddress, U32 width, U32 height, S32 xHot, S32 yHot) = 0;

	virtual void warpMouse(int x, int y) = 0;
	virtual bool isVisible() = 0;

	virtual bool clipboardIsTextAvailable() = 0;
	virtual BString clipboardGetText() = 0;
	virtual void clipboardSetText(const char* text) = 0;
};

typedef std::shared_ptr<KNativeScreen> KNativeScreenPtr;

#endif