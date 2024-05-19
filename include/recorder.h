#ifndef __RECORDER_H__
#define __RECORDER_H__

#ifdef BOXEDWINE_RECORDER
#ifdef BOXEDWINE_64
#ifdef BOXEDWINE_ARMV8BT
#define RECORDER_SCRIPT "script.arm64.txt"
#else
#define RECORDER_SCRIPT "script.x64.txt"
#endif
#else
#define RECORDER_SCRIPT "script.txt"
#endif
class Recorder {
public:
    static void start(BString directory);
    static Recorder* instance;

    void initCommandLine(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args);
    void takeScreenShot();
    void onMouseMove(U32 x, U32 y);
    void onMouseButton(U32 down, U32 button, U32 x, U32 y);
    void onKey(U32 key, U32 down);
    void close();

    BWriteFile file;
    BString directory;
private:
    void out(const char* s);
    int screenShotCount = 0;
    void fullScrennShot();
    void partialScreenShot(U32 x, U32 y, U32 w, U32 h);
};

void BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(int x, int y);
void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(int button, int x, int y); // 0 left, 1 right, 2 middle
void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(int button, int x, int y);
bool BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(int key, bool isF11);
bool BOXEDWINE_RECORDER_HANDLE_KEY_UP(int key, bool isF11);
U32 BOXEDWINE_RECORDER_QUIT();
void BOXEDWINE_RECORDER_RUN_SLICE();
void BOXEDWINE_RECORDER_INIT(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args);
#else
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(x, y)
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(b, x, y)
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(b, x, y)
#define BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(x, y) false
#define BOXEDWINE_RECORDER_HANDLE_KEY_UP(x, y) false
#define BOXEDWINE_RECORDER_QUIT() 0
#define BOXEDWINE_RECORDER_RUN_SLICE();
#define BOXEDWINE_RECORDER_INIT(root, zips, working, args)
#endif

#endif