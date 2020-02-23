#include "boxedwine.h"
#include "../sdl/sdlwindow.h"

#ifdef BOXEDWINE_RECORDER
Player* Player::instance;

void Player::readCommand() {
    char tmp[256];
    U32 count=0;

    this->nextCommand="";
    this->nextValue="";
    while (true) {
        U32 result = (U32)fread(&tmp[count], 1, 1, this->file);
        if (!result) {
            tmp[count] = 0;
            if (count>0) {
                if (this->nextCommand.size()==0) {
                    this->nextCommand = tmp;
                } else {
                    this->nextValue = tmp;
                }
                break;
            }
            klog("script finished: success");
            exit(1);
        }
        count++;
        if (tmp[count-1]=='=') {
            tmp[count-1] = 0;
            this->nextCommand = tmp;
            count = 0;
            continue;
        }
        if (tmp[count-1]=='\n') {
            if (count>=2 && tmp[count-2]=='\r') {
                tmp[count-2]=0;
            } else {
                tmp[count-1] = 0;
            }
            this->nextValue=tmp;
            break;
        }
    }
    this->lastCommandTime = Platform::getMicroCounter();
    if (this->nextCommand.length()==0) {
        klog("script did not finish properly: failed");
        sdlScreenShot("failed.bmp", NULL);
        exit(0);
    }
}

bool Player::start(std::string directory) {
    Player::instance = new Player();
    std::string script = std::string(directory+"/script.txt");
    instance->directory = directory;
    instance->file = fopen(script.c_str(), "rb");
    instance->lastCommandTime = 0;
    instance->lastScreenRead = 0;
    if (!instance->file) {
        klog("script not found: %s", script.c_str());
        exit(0);
    }
    instance->readCommand();
    instance->version = instance->nextValue;
    if (instance->version!="1") {
        klog("script is wrong version, was expecting 1 and instead got %s", instance->version.c_str());
        exit(0);
    }
    instance->readCommand();
    return true;
}

void Player::initCommandLine(std::string root, const std::vector<std::string>& zips, std::string working, const std::vector<std::string>& args) {
    while (this->nextCommand=="ROOT" || this->nextCommand=="ZIP" || this->nextCommand=="CWD" || this->nextCommand=="ARGC" || stringStartsWith(this->nextCommand, "ARG")) {
        instance->readCommand();
    }
}

void Player::runSlice() {  
    // at least 10 ms between mouse moves
    if (Platform::getMicroCounter()<this->lastCommandTime+10000)
        return;
    if (this->nextCommand=="MOVETO") {
        std::vector<std::string> items;
        stringSplit(items, this->nextValue, ',');
        if (items.size()!=2) {
            klog("script: %s MOVETO should have 2 values: %s", this->directory.c_str(), this->nextValue.c_str());
            exit(0);
        }
        sdlMouseMouse(atoi(items[0].c_str()), atoi(items[1].c_str()), false);
        instance->readCommand();
        return;
    } 
    // 1000 ms between all other commands
    if (Platform::getMicroCounter()<this->lastCommandTime+1000000 && this->nextCommand!="MOUSEUP" && this->nextCommand!="KEYUP" && this->nextCommand!="SCREENSHOT")
        return;
    // at least 100 ms between mouse or key down/up
    if (Platform::getMicroCounter()<this->lastCommandTime+100000)
        return;
    if (this->nextCommand=="MOUSEDOWN" || this->nextCommand=="MOUSEUP") {
        std::vector<std::string> items;
        stringSplit(items, this->nextValue, ',');
        if (items.size()!=3) {
            klog("script: %s %s should have 3 values: %s", this->directory.c_str(), this->nextCommand.c_str(), this->nextValue.c_str());
            exit(0);
        }
        sdlMouseButton((this->nextCommand=="MOUSEDOWN")?1:0, atoi(items[0].c_str()), atoi(items[1].c_str()), atoi(items[2].c_str()));
        instance->readCommand();
        if (this->nextCommand=="MOUSEUP") {
            runSlice();
        }
    } else if (this->nextCommand=="KEYDOWN" || this->nextCommand=="KEYUP") {
        sdlKey(atoi(this->nextValue.c_str()), (this->nextCommand=="KEYDOWN")?1:0);
        instance->readCommand();
        if (this->nextCommand=="KEYUP") {
            runSlice();
        }
    } else if (this->nextCommand=="WAIT") {
        if (Platform::getMicroCounter()>this->lastCommandTime+1000000l*atoi(this->nextValue.c_str())) {
            klog("script: done waiting %s", this->nextValue.c_str());
            instance->readCommand();            
        }
    } else if (this->nextCommand=="DONE") {
        //exit(1);, let it exit gracefully
    } else if (this->nextCommand=="SCREENSHOT") {
        if (Platform::getMicroCounter()<this->lastScreenRead+1000000) {
            return;
        }
        std::vector<std::string> items;
        stringSplit(items, this->nextValue, ',');
        if (items.size()>4) {
            U32 x = atoi(items[0].c_str());
            U32 y = atoi(items[1].c_str());
            U32 w = atoi(items[2].c_str());
            U32 h = atoi(items[3].c_str());
            U32 expectedCRC = atoi(items[4].c_str());
            U32 currentCRC = 0;

            sdlPartialScreenShot("", x, y, w, h, &currentCRC);
            if (currentCRC==expectedCRC) {
                klog("script: screen shot matched");
                instance->readCommand();
                this->lastCommandTime+=4000000; // sometimes the screen isn't ready for input even though you can see it
                instance->lastScreenRead = Platform::getMicroCounter();
            }
        } else if (items.size()>0) {
            U32 expectedCRC = atoi(items[0].c_str());
            U32 currentCRC = 0;
            sdlScreenShot("", &currentCRC);
            if (currentCRC==expectedCRC) {
                klog("script: screen shot matched");
                instance->readCommand();
                this->lastCommandTime+=4000000; // sometimes the screen isn't ready for input even though you can see it
                instance->lastScreenRead = Platform::getMicroCounter();
            }
        }
    }
    if (Platform::getMicroCounter()>this->lastCommandTime+1000000*60*10) {
        klog("script timed out %s", this->directory.c_str());
        sdlScreenShot("failed.bmp", NULL);
        exit(0);
    }
}

#endif
