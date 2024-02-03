#include "boxedwine.h"
#include "knativewindow.h"

#ifdef BOXEDWINE_RECORDER
Player* Player::instance;

void Player::readCommand() {
    this->nextCommand.clear();
    this->nextValue.clear();

    BString line;
    if (!file.readLine(line)) {
        klog("script finished: success");
#ifdef BOXEDWINE_MULTI_THREADED
        KSystem::destroy();
#endif
        exit(0);
    }    
    std::vector<BString> results;
    line.split("=", results);
    if (results.size() == 2) {
        this->nextCommand = results[0];
        this->nextValue = results[1];
    } else if (results.size() == 1) {
        this->nextCommand = results[0];
    } else {
        klog("malformed script.  Line = %s", line.c_str());
#ifdef BOXEDWINE_MULTI_THREADED
        KSystem::destroy();
#endif
        exit(99);
    }    
    this->lastCommandTime = KSystem::getMicroCounter();
    if (this->nextCommand.length()==0) {
        klog("malformed script.  Line = %s", line.c_str());
#ifdef BOXEDWINE_MULTI_THREADED
        KSystem::destroy();
#endif
        exit(99);
    }
}

bool Player::start(BString directory) {
    Player::instance = new Player();
    BString script = BString(directory+"/"+RECORDER_SCRIPT);
    instance->directory = directory;
    instance->file.open(script);
    instance->lastCommandTime = 0;
    instance->lastScreenRead = 0;
    if (!instance->file.isOpen()) {
        klog("script not found: %s error=%d(%s)", script.c_str(), errno, strerror(errno));
        exit(100);
    } else {
        klog("using script: %s", script.c_str());
    }
    instance->readCommand();
    instance->version = instance->nextValue;
    if (instance->version!="1") {
        klog("script is wrong version, was expecting 1 and instead got %s", instance->version.c_str());
        exit(99);
    }
    instance->readCommand();
    return true;
}

void Player::initCommandLine(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args) {
    while (this->nextCommand=="ROOT" || this->nextCommand=="ZIP" || this->nextCommand=="CWD" || this->nextCommand=="ARGC" || this->nextCommand.startsWith("ARG")) {
        instance->readCommand();
    }
}

void Player::runSlice() {  
    // at least 10 ms between mouse moves
    if (KSystem::getMicroCounter()<this->lastCommandTime+10000)
        return;
    if (this->nextCommand=="MOVETO") {
        std::vector<BString> items;
        this->nextValue.split(',', items);
        if (items.size()!=2) {
            klog("script: %s MOVETO should have 2 values: %s", this->directory.c_str(), this->nextValue.c_str());
            exit(99);
        }
        KNativeWindow::getNativeWindow()->mouseMove(atoi(items[0].c_str()), atoi(items[1].c_str()), false);
        instance->readCommand();
        return;
    } 
    // 1000 ms between all other commands
    if (KSystem::getMicroCounter()<this->lastCommandTime+1000000 && this->nextCommand!="MOUSEUP" && this->nextCommand!="KEYUP" && this->nextCommand!="SCREENSHOT")
        return;
    // at least 100 ms between mouse or key down/up
    if (KSystem::getMicroCounter()<this->lastCommandTime+100000)
        return;
    if (this->nextCommand=="MOUSEDOWN" || this->nextCommand=="MOUSEUP") {
        std::vector<BString> items;
        this->nextValue.split(',', items);
        if (items.size()!=3) {
            klog("script: %s %s should have 3 values: %s", this->directory.c_str(), this->nextCommand.c_str(), this->nextValue.c_str());
            exit(99);
        }
        KNativeWindow::getNativeWindow()->mouseButton((this->nextCommand=="MOUSEDOWN")?1:0, atoi(items[0].c_str()), atoi(items[1].c_str()), atoi(items[2].c_str()));
        instance->readCommand();
        if (this->nextCommand=="MOUSEUP") {
            runSlice();
        }
    } else if (this->nextCommand=="KEYDOWN" || this->nextCommand=="KEYUP") {
        KNativeWindow::getNativeWindow()->key(atoi(this->nextValue.c_str()), (this->nextCommand=="KEYDOWN")?1:0);
        instance->readCommand();
        if (this->nextCommand=="KEYUP") {
            runSlice();
        }
    } else if (this->nextCommand=="WAIT") {
        if (KSystem::getMicroCounter()>this->lastCommandTime+1000000l*this->nextValue.toInt64()) {
            klog("script: done waiting %s", this->nextValue.c_str());
            instance->readCommand();            
        }
    } else if (this->nextCommand=="DONE") {
        //exit(1);, let it exit gracefully
    } else if (this->nextCommand=="SCREENSHOT") {
        if (KSystem::getMicroCounter()<this->lastScreenRead+1000000) {
            return;
        }
        std::vector<BString> items;
        this->nextValue.split(',', items);
        if (items.size()>4) {
            U32 x = atoi(items[0].c_str());
            U32 y = atoi(items[1].c_str());
            U32 w = atoi(items[2].c_str());
            U32 h = atoi(items[3].c_str());
            U32 expectedCRC = atoi(items[4].c_str());
            U32 currentCRC = 0;

            KNativeWindow::getNativeWindow()->partialScreenShot(B(""), x, y, w, h, &currentCRC);
            if (currentCRC==expectedCRC) {
                klog("script: screen shot matched");
                instance->readCommand();
                this->lastCommandTime+=4000000; // sometimes the screen isn't ready for input even though you can see it
                instance->lastScreenRead = KSystem::getMicroCounter();
            }
        } else if (items.size()>0) {
            U32 expectedCRC = atoi(items[0].c_str());
            U32 currentCRC = 0;
            KNativeWindow::getNativeWindow()->screenShot(B(""), &currentCRC);
            if (currentCRC==expectedCRC) {
                klog("script: screen shot matched");
                instance->readCommand();
                this->lastCommandTime+=4000000; // sometimes the screen isn't ready for input even though you can see it
                instance->lastScreenRead = KSystem::getMicroCounter();
            }
        }
    }
    if (KSystem::getMicroCounter()>this->lastCommandTime+1000000*60*10) {
        klog("script timed out %s", this->directory.c_str());
        KNativeWindow::getNativeWindow()->screenShot(B("failed.bmp"), nullptr);
        exit(2);
    }
}

#endif
