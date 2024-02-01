#ifndef __KNATIVEWINDOW_H__
#define __KNATIVEWINDOW_H__

class wRECT {
public:
    wRECT() : left(0), top(0), right(0), bottom(0) {}
    S32 left;
    S32 top;
    S32 right;
    S32 bottom;
    void readRect(KMemory* memory, U32 address) {
        if (address) {
            left = memory->readd(address);
            top = memory->readd(address + 4);
            right = memory->readd(address + 8);
            bottom = memory->readd(address + 12);
        }
    }
};

class Wnd {
public:
    Wnd() : surface(0) {}
    virtual ~Wnd() {}
    virtual void setText(BString text) = 0;
    virtual void show(bool bShow) = 0;    
    virtual void destroy() = 0;
    virtual U32 glSetPixelFormat(U32 index) = 0;
    virtual U32 glGetPixelFormat() = 0;
    virtual bool setFocus() = 0;

    wRECT windowRect;
    wRECT clientRect;
    U32 surface;
};

class KNativeWindow {
public:
	static std::shared_ptr<KNativeWindow> getNativeWindow();
    static void init(U32 cx, U32 cy, U32 bpp, int scaleX, int scaleY, BString scaleQuality, U32 fullScreen, U32 vsync);
    static void shutdown();

	static U32 defaultScreenWidth;
	static U32 defaultScreenHeight;
	static U32 defaultScreenBpp;
    static bool windowUpdated;

    bool isVulkan;
    bool needsVulkan;

    KNativeWindow() : isVulkan(false), needsVulkan(false) {}
    virtual ~KNativeWindow() {}
    void updateDisplay(KThread* thread) { screenChanged(thread, screenWidth(), screenHeight(), screenBpp()); }
	virtual void screenChanged(KThread* thread, U32 width, U32 height, U32 bpp) = 0;
	virtual U32 screenWidth() = 0;
	virtual U32 screenHeight() = 0;
	virtual U32 screenBpp() = 0;
	virtual bool getMousePos(int* x, int* y) = 0;
	virtual void setMousePos(int x, int y) = 0;
	
	virtual bool setCursor(const char* moduleName, const char* resourceName, int resource) = 0;
	virtual void createAndSetCursor(const char* moduleName, const char* resourceName, int resource, U8* and_bits, U8* xor_bits, int width, int height, int hotX, int hotY) = 0;

    virtual std::shared_ptr<Wnd> getWnd(U32 hwnd) = 0;
    virtual std::shared_ptr<Wnd> createWnd(KThread* thread, U32 processId, U32 hwnd, U32 windowRect, U32 clientRect) = 0;
    virtual void bltWnd(KThread* thread, U32 hwnd, U32 bits, S32 xOrg, S32 yOrg, U32 width, U32 height, U32 rect) = 0;
    virtual void drawWnd(KThread* thread, std::shared_ptr<Wnd> w, U8* bytes, U32 pitch, U32 bpp, U32 width, U32 height) = 0;
#ifndef BOXEDWINE_MULTI_THREADED
    virtual void flipFB() = 0;
#endif
    virtual void setPrimarySurface(KThread* thread, U32 bits, U32 width, U32 height, U32 pitch, U32 flags, U32 palette) = 0;    
    virtual void drawAllWindows(KThread* thread, U32 hWnd, int count) = 0;
    virtual void setTitle(BString title) = 0;

    virtual U32 getGammaRamp(KThread* thread, U32 ramp) = 0;

    virtual U32 glCreateContext(KThread* thread, std::shared_ptr<Wnd> wnd, int major, int minor, int profile, int flags) = 0;
    virtual void glDeleteContext(KThread* thread, U32 contextId) = 0;
    virtual U32 glMakeCurrent(KThread* thread, U32 arg) = 0;
    virtual U32 glShareLists(KThread* thread, U32 srcContext, U32 destContext) = 0;
    virtual void glSwapBuffers(KThread* thread) = 0;
    virtual void glUpdateContextForThread(KThread* thread) = 0;
    virtual void preOpenGLCall(U32 index) = 0;

    virtual bool partialScreenShot(BString filepath, U32 x, U32 y, U32 w, U32 h, U32* crc) = 0;
    virtual bool screenShot(BString filepath, U32* crc) = 0;

    virtual bool waitForEvent(U32 ms) = 0; // if return is true, then event is available
    virtual bool processEvents() = 0; // if return is false, then shutdown    

    virtual int mouseMove(int x, int y, bool relative) = 0;
    virtual int mouseWheel(int amount, int x, int y) = 0;
    virtual int mouseButton(U32 down, U32 button, int x, int y) = 0;
    virtual int key(U32 key, U32 down) = 0;  // the key code is specific to the back end

#ifdef BOXEDWINE_RECORDER
    // return true to continue processing for custom handlers
    virtual void processCustomEvents(std::function<bool(bool isKeyDown, int key, bool isF11)> onKey, std::function<bool(bool isButtonDown, int button, int x, int y)> onMouseButton, std::function<bool(int x, int y)> onMouseMove) = 0;

    virtual void pushWindowSurface() = 0;
    virtual void popWindowSurface() = 0;
    virtual void drawRectOnPushedSurfaceAndDisplay(U32 x, U32 y, U32 w, U32 h, U8 r, U8 g, U8 b, U8 a) = 0;
#endif
    virtual void* createVulkanSurface(void* instance) = 0;
};

#define BOXED_KEYEVENTF_EXTENDEDKEY        0x0001
#define BOXED_KEYEVENTF_KEYUP              0x0002
#define BOXED_KEYEVENTF_UNICODE            0x0004
#define BOXED_KEYEVENTF_SCANCODE           0x0008

#define BOXED_VK_CANCEL              0x03
#define BOXED_VK_BACK                0x08
#define BOXED_VK_TAB                 0x09
#define BOXED_VK_RETURN              0x0D
#define BOXED_VK_SHIFT               0x10
#define BOXED_VK_CONTROL             0x11
#define BOXED_VK_MENU                0x12
#define BOXED_VK_PAUSE               0x13
#define BOXED_VK_CAPITAL             0x14

#define BOXED_VK_ESCAPE              0x1B

#define BOXED_VK_SPACE               0x20
#define BOXED_VK_PRIOR               0x21
#define BOXED_VK_NEXT                0x22
#define BOXED_VK_END                 0x23
#define BOXED_VK_HOME                0x24
#define BOXED_VK_LEFT                0x25
#define BOXED_VK_UP                  0x26
#define BOXED_VK_RIGHT               0x27
#define BOXED_VK_DOWN                0x28
#define BOXED_VK_INSERT              0x2D
#define BOXED_VK_DELETE              0x2E
#define BOXED_VK_HELP                0x2F

#define BOXED_VK_MULTIPLY            0x6A
#define BOXED_VK_ADD                 0x6B
#define BOXED_VK_DECIMAL             0x6E
#define BOXED_VK_DIVIDE              0x6F

#define BOXED_VK_F1                  0x70
#define BOXED_VK_F2                  0x71
#define BOXED_VK_F3                  0x72
#define BOXED_VK_F4                  0x73
#define BOXED_VK_F5                  0x74
#define BOXED_VK_F6                  0x75
#define BOXED_VK_F7                  0x76
#define BOXED_VK_F8                  0x77
#define BOXED_VK_F9                  0x78
#define BOXED_VK_F10                 0x79
#define BOXED_VK_F11                 0x7A
#define BOXED_VK_F12                 0x7B
#define BOXED_VK_F24                 0x87

#define BOXED_VK_NUMLOCK             0x90
#define BOXED_VK_SCROLL              0x91

#define BOXED_VK_LSHIFT              0xA0
#define BOXED_VK_RSHIFT              0xA1
#define BOXED_VK_LCONTROL            0xA2
#define BOXED_VK_RCONTROL            0xA3
#define BOXED_VK_LMENU               0xA4
#define BOXED_VK_RMENU               0xA5

#define BOXED_VK_OEM_1               0xBA
#define BOXED_VK_OEM_PLUS            0xBB
#define BOXED_VK_OEM_COMMA           0xBC
#define BOXED_VK_OEM_MINUS           0xBD
#define BOXED_VK_OEM_PERIOD          0xBE
#define BOXED_VK_OEM_2               0xBF
#define BOXED_VK_OEM_3               0xC0
#define BOXED_VK_OEM_4               0xDB
#define BOXED_VK_OEM_5               0xDC
#define BOXED_VK_OEM_6               0xDD
#define BOXED_VK_OEM_7               0xDE

#endif
