#ifndef __KNativeScreen_SDL_H__
#define __KNativeScreen_SDL_H__

#include "knativescreen.h"
#include "knativeinputSDL.h"

class WndCache {
public:
    ~WndCache() {
        clear();
    }
    U32 id = 0;
    SDL_Texture* sdlTexture = nullptr;
    U32 sdlTextureHeight = 0;
    U32 sdlTextureWidth = 0;

    U8* bits = nullptr;
    U32 bitsSize;

    void ensureSize(U32 size) {
        if (bitsSize < size) {
            if (bits) {
                delete[] bits;
            }
            bits = new U8[size];
            bitsSize = size;
        }
    }

    void clear() {
        if (sdlTexture) {
            SDL_DestroyTexture(sdlTexture);
        }
        sdlTexture = nullptr;
        sdlTextureHeight = 0;
        sdlTextureWidth = 0;
        if (bits) {
            delete[] bits;
        }
        bitsSize = 0;
    }
};

typedef std::shared_ptr<WndCache> WndCachePtr;
class KVulkdanSDLImpl;

class KNativeScreenSDL : public KNativeScreen {
public:
    ~KNativeScreenSDL() override;

    KNativeScreenSDL(U32 cx, U32 cy, U32 bpp, int scaleX, int scaleY, const BString& scaleQuality, U32 fullScreen, U32 vsync);
    
    KNativeInputPtr getInput() override;

    void setScreenSize(U32 cx, U32 cy) override;

    U32 screenWidth() override;
    U32 screenHeight() override;
    U32 screenBpp() override;
    U32 screenRate() override;

    void setTitle(const BString& title) override;
    void showWindow(bool show) override;
    void getPos(S32& x, S32& y) override;
    U32 getLastUpdateTime() override;

    void clear() override;
    void putBitsOnWnd(U32 id, U8* bits, U32 bitsPerPixel, U32 srcPitch, S32 dstX, S32 dstY, U32 width, U32 height, U32* palette, bool isDirty) override;
    void present() override;
    bool presentedSinceLastCheck() override;
    void clearTextureCache(U32 id) override;
    bool canBltToScreen() override;

    void warpMouse(int x, int y) override;
    bool isVisible() override;

    bool clipboardIsTextAvailable() override;
    BString clipboardGetText() override;
    void clipboardSetText(const char* text) override;

#ifdef BOXEDWINE_RECORDER
    void startRecorderScreenShot() override;
    void finishRecorderScreenShot() override;
    void drawRectOnPushedSurfaceAndDisplay(U32 x, U32 y, U32 w, U32 h, U8 r, U8 g, U8 b, U8 a) override;
#endif

    bool partialScreenShot(const BString& filepath, U32 x, U32 y, U32 w, U32 h, U8* buffer, U32 bufferlen) override;
    bool screenShot(const BString& filepath, U8* buffer, U32 bufferlen) override;
    bool saveBmp(const BString& filepath, U8* buffer, U32 bpp, U32 w, U32 h) override;
    bool internalScreenShot(const BString& filepath, SDL_Rect* r, U8* buffer, U32 bufferlen);

    void setCursor(const std::shared_ptr<XCursor>& cursor) override;
    void buildCursor(KThread* thread, const std::shared_ptr<XCursor>& cursor, U32 pixelsAddress, U32 width, U32 height, S32 xHot, S32 yHot) override;

    KNativeInputSDLPtr input;
    
private:
    friend class KVulkdanSDLImpl;

    bool visible = false;
    bool showOnDraw = true;
    bool presented = false;
    U32 bpp = 0;
    BString scaleQuality;
    U32 fullScreen;
    U32 vsync;
    U32 defaultScreenWidth;
    U32 defaultScreenHeight;
    U32 defaultScreenBpp;
    U32 lastUpdateTime = 0;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    U32 additionalSDLWindowFlags = 0;

    void recreateMainWindow();
    void destroyMainWindow();

    void destroyTextureCache();

    BOXEDWINE_MUTEX wndCacheMutex;
    BHashTable<U32, WndCachePtr> wndCache;

    BOXEDWINE_MUTEX cursorsMutex;
    BHashTable<U32, SDL_Cursor*> cursors;

#ifdef BOXEDWINE_RECORDER
    SDL_Texture* screenCopyTexture = nullptr;
    U8* recordBuffer = nullptr;
    U32 recordBufferSize = 0;
#endif

    BOXEDWINE_MUTEX drawingMutex;
};

typedef std::shared_ptr<KNativeScreenSDL> KNativeScreenSDLPtr;

#endif