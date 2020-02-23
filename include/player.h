#ifndef __PLAYER_H__
#define __PLAYER_H__

#ifdef BOXEDWINE_RECORDER
class Player {
public:
    static bool start(std::string directory);
    static Player* instance;

    void initCommandLine(std::string root, const std::vector<std::string>& zips, std::string working, const std::vector<std::string>& args);
    void runSlice();

    FILE* file;
    std::string directory;
    std::string version;
    U64 lastCommandTime;
    U64 lastScreenRead;
    std::string nextCommand;
private:    
    std::string nextValue;
    void readCommand();
};
#endif

#endif