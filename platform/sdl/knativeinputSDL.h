#ifndef __KNATIVEWINDOW_SDL_H__
#define __KNATIVEWINDOW_SDL_H__

#include "knativeinput.h"

class KNativeInputSDL : public KNativeInput {
public:
    KNativeInputSDL(U32 cx, U32 cy, int scaleX, int scaleY);

    void setScreenSize(U32 cx, U32 cy) override;
    U32 screenWidth() override;
    U32 screenHeight() override;

    bool waitForEvent(U32 ms) override; // if return is true, then event is available
    bool processEvents() override; // if return is false, then shutdown    
    void processCustomEvents(std::function<bool(bool isKeyDown, int key, bool isF11)> onKey, std::function<bool(bool isButtonDown, int button, int x, int y)> onMouseButton, std::function<bool(int x, int y)> onMouseMove) override;

    void runOnUiThread(std::function<void()> callback) override;
    bool mouseMove(int x, int y, bool relative) override;
    bool mouseWheel(int amount, int x, int y) override;
    bool mouseButton(U32 down, U32 button, int x, int y) override;
    bool key(U32 sdlScanCode, U32 key, U32 down) override;  // the key code is specific to the back end

    bool getMousePos(int* x, int* y, bool allowWarp = true) override;
    void setMousePos(int x, int y) override;
    U32 getInputModifiers() override;

    int xToScreen(int x);
    int xFromScreen(int x);
    int yToScreen(int y);
    int yFromScreen(int y);

    bool checkMousePos(int& x, int& y, bool allowWarp);

    U32 scaleX = 100;
    U32 scaleXOffset = 0;
    U32 scaleY = 100;
    U32 scaleYOffset = 0;    
    int lastX = 0;
    int lastY = 0;
    U32 width = 0;
    U32 height = 0;           

    bool handlSdlEvent(SDL_Event* e);
};

typedef std::shared_ptr<KNativeInputSDL> KNativeInputSDLPtr;

#endif