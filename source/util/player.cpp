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
            exit(0);
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
        exit(1);
    }
}

bool Player::start(std::string directory) {
    Player::instance = new Player();
    std::string script = std::string(directory+"/script.txt");
    instance->directory = directory;
    instance->file = fopen(script.c_str(), "rb");
    instance->lastCommandTime = 0;
    if (!instance->file) {
        klog("script not found: %s", script.c_str());
        exit(1);
    }
    instance->readCommand();
    instance->version = instance->nextValue;
    if (instance->version!="1") {
        klog("script is wrong version, was expecting 1 and instead got %s", instance->version);
        exit(1);
    }
    instance->readCommand();
    return true;
}

void Player::initCommandLine(std::string root, std::string zip, std::string working, const char **argv, U32 argc) {
    while (this->nextCommand=="ROOT" || this->nextCommand=="ZIP" || this->nextCommand=="CWD" || this->nextCommand=="ARGC" || stringStartsWith(this->nextCommand, "ARG")) {
        instance->readCommand();
    }
}

void Player::runSlice() {    
    if (this->nextCommand=="MOVETO") {
        std::vector<std::string> items;
        stringSplit(items, this->nextValue, ',');
        if (items.size()!=2) {
            klog("script: %s MOVETO should have 2 values: %s", this->directory.c_str(), this->nextValue.c_str());
            exit(1);
        }
        sdlMouseMouse(atoi(items[0].c_str()), atoi(items[1].c_str()));
        instance->readCommand();
        return;
    } 
    if (Platform::getMicroCounter()<this->lastCommandTime+1000000)
        return;
    if (this->nextCommand=="MOUSEDOWN" || this->nextCommand=="MOUSEUP") {
        std::vector<std::string> items;
        stringSplit(items, this->nextValue, ',');
        if (items.size()!=3) {
            klog("script: %s %s should have 3 values: %s", this->directory.c_str(), this->nextCommand.c_str(), this->nextValue.c_str());
            exit(1);
        }
        sdlMouseButton((this->nextCommand=="MOUSEDOWN")?1:0, atoi(items[0].c_str()), atoi(items[1].c_str()), atoi(items[2].c_str()));
        instance->readCommand();
    } else if (this->nextCommand=="KEYDOWN" || this->nextCommand=="KEYUP") {
        sdlKey(atoi(this->nextValue.c_str()), (this->nextCommand=="KEYDOWN")?1:0);
        instance->readCommand();
    } else if (this->nextCommand=="WAIT") {
        if (Platform::getMicroCounter()>this->lastCommandTime+1000000l*atoi(this->nextValue.c_str())) {
            klog("script: done waiting %s", this->nextValue.c_str());
            instance->readCommand();
        }
    } else if (this->nextCommand=="DONE") {
        //exit(0);, let it exit gracefully
    }
    if (Platform::getMicroCounter()>this->lastCommandTime+1000000*60*5) {
        klog("script timed out %s", this->directory.c_str());
        sdlScreenShot("failed.bmp", NULL);
        exit(1);
    }
}

void Player::screenChanged() {
    if (this->nextCommand=="SCREENSHOT") {
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
            }
        } else if (items.size()>0) {
            U32 expectedCRC = atoi(items[0].c_str());
            U32 currentCRC = 0;
            sdlScreenShot("", &currentCRC);
            if (currentCRC==expectedCRC) {
                klog("script: screen shot matched");
                instance->readCommand();
            }
        }
    }
}

#endif
