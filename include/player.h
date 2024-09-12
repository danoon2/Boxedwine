#ifndef __PLAYER_H__
#define __PLAYER_H__

#ifdef BOXEDWINE_RECORDER
class Player {
public:
    static bool start(BString directory);
    static Player* instance;

    void initCommandLine(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args);
    void runSlice();
    void quit();

    BReadFile file;
    BString directory;
    BString version;
    U64 lastCommandTime = 0;
    U64 lastScreenRead = 0;
    BString nextCommand;
    U32 currentInputModifiers = 0;
private:    
    BString nextValue;
    std::thread comparingThread;

    void readCommand();        
};
#endif

#endif