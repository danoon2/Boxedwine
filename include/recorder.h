#ifndef __RECORDER_H__
#define __RECORDER_H__

#ifdef BOXEDWINE_RECORDER
class Recorder {
public:
    static void start(std::string directory);
    static Recorder* instance;

    void initCommandLine(std::string root, const std::vector<std::string>& zips, std::string working, const char **argv, U32 argc);
    void takeScreenShot();
    void onMouseMove(U32 x, U32 y);
    void onMouseButton(U32 down, U32 button, U32 x, U32 y);
    void onKey(U32 key, U32 down);
    void close();

    FILE* file;
    std::string directory;
private:
    void out(const char* s);
    int screenShotCount;
    void fullScrennShot();
    void partialScreenShot(U32 x, U32 y, U32 w, U32 h);
};

void BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(void* e);
void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(void* e);
void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(void* e);
bool BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(void* e);
bool BOXEDWINE_RECORDER_HANDLE_KEY_UP(void* e);
U32 BOXEDWINE_RECORDER_QUIT();
void BOXEDWINE_RECORDER_RUN_SLICE();
void BOXEDWINE_RECORDER_INIT(std::string root, const std::vector<std::string> zips, std::string working, const char **argv, U32 argc);
#else
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(x)
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(x)
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(x)
#define BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(x) false
#define BOXEDWINE_RECORDER_HANDLE_KEY_UP(x) false
#define BOXEDWINE_RECORDER_QUIT() 0
#define BOXEDWINE_RECORDER_RUN_SLICE();
#define BOXEDWINE_RECORDER_INIT(root, zips, working, argv, argc)
#endif

#endif