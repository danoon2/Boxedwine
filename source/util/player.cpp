#include "boxedwine.h"
#include "knativesystem.h"
#include "pixelMatch.h"

#ifdef BOXEDWINE_RECORDER

#pragma warning(push)
#pragma warning (disable : ALL_CODE_ANALYSIS_WARNINGS)
#include "stb_image.h"

static void flipRGBBitmap(unsigned char* data, int stride, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < stride; x += 4) {
            unsigned char b = data[x];
            data[x] = data[x + 2];
            data[x + 2] = b;
        }
        data += stride;
    }
}

Player* Player::instance;

void Player::readCommand() {
    this->nextCommand.clear();
    this->nextValue.clear();

    BString line;
    if (!file.readLine(line)) {
        klog("script finished: success");
        this->nextCommand = B("DONE"); // will cause success exit code to be returned
        KSystem::killTime = KSystem::getMilliesSinceStart() + 30000;
        return;
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
    if (instance->version!="2") {
        klog("script is wrong version, was expecting 2 and instead got %s", instance->version.c_str());
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

static U8* buffer;
static U32 bufferlen;
static BString lastFileName;
static U32 comparingPixels;
static int image_width;
static int image_height;
static unsigned char* image_data;
static U8* output;
static U32 outputLen;

#define COMPARING_PIXELS_WAITING 0
#define COMPARING_PIXELS_WORKING 1
#define COMPARING_PIXELS_SUCCESS 2
#define COMPARING_PIXELS_DONE 3

static std::mutex comparingCondMutex;
static std::condition_variable comparingCond;

void bitmapCompareThread(Player* player) {    
    while (comparingPixels != COMPARING_PIXELS_DONE) {
        {
            std::unique_lock<std::mutex> boxedWineCriticalSection(comparingCondMutex);
            comparingCond.wait(boxedWineCriticalSection);
            if (comparingPixels == COMPARING_PIXELS_DONE) {
                break;
            }
            comparingPixels = COMPARING_PIXELS_WORKING;
        }
        if (outputLen < bufferlen) {
            if (output) {
                delete[] output;
            }
            output = new U8[bufferlen];
            outputLen = bufferlen;
        }
        if (pixelmatch(image_data, image_width * 4, buffer, image_width * 4, image_width, image_height, output) == 0) {
            std::unique_lock<std::mutex> boxedWineCriticalSection(comparingCondMutex);
            if (comparingPixels == COMPARING_PIXELS_DONE) {
                break;
            }
            comparingPixels = COMPARING_PIXELS_SUCCESS;
        } else {
            std::unique_lock<std::mutex> boxedWineCriticalSection(comparingCondMutex);
            if (comparingPixels == COMPARING_PIXELS_DONE) {
                break;
            }
            comparingPixels = COMPARING_PIXELS_WAITING;            
        }
        Platform::nanoSleep(500000l);
    }
}

void Player::runSlice() {  
    // at least 10 ms between mouse moves
    if (this->nextCommand == "MODIFIERS") {
        currentInputModifiers = atoi(nextValue.c_str());
        instance->readCommand();
    } else if (this->nextCommand == "WHILEWAITING") {
        std::vector<BString> items;
        this->nextValue.split(',', items);
        timerWhileWaiting = atoi(items[0].c_str());
        if (items.size() > 1) {
            waitCommand = items[1];
        }
        if (waitCommand == "LBUTTON") {
            if (items.size() > 3) {
                waitMouseX = atoi(items[2].c_str());
                waitMouseY = atoi(items[3].c_str());
            } else {
                waitMouseX = 1;
                waitMouseY = 1;
            }
        } else if (waitCommand == "KEY") {
            waitKey = atoi(items[2].c_str());
        }
        nextWaitTime = KSystem::getMilliesSinceStart() + timerWhileWaiting * 1000;
        instance->readCommand();
    }

    if (KSystem::getMicroCounter()<this->lastCommandTime+10000)
        return;

    KNativeScreenPtr screen = KNativeSystem::getScreen();
    KNativeInputPtr input = KNativeSystem::getCurrentInput();

    if (this->nextCommand=="MOVETO") {
        std::vector<BString> items;
        this->nextValue.split(',', items);
        if (items.size()!=2) {
            klog("script: %s MOVETO should have 2 values: %s", this->directory.c_str(), this->nextValue.c_str());
            exit(99);
        }
        input->mouseMove(atoi(items[0].c_str()), atoi(items[1].c_str()), false);
        instance->readCommand();
        return;
    } 
    // 1000 ms between all other commands
    if (KSystem::getMicroCounter()<this->lastCommandTime+1000000 && this->nextCommand!="MOUSEUP" && this->nextCommand!="KEYUP" && this->nextCommand != "SCREENSHOT" && !processWaitCommand)
        return;
    // at least 100 ms between mouse or key down/up
    if (KSystem::getMicroCounter()<this->lastCommandTime+100000)
        return;

    // process before any other command so that button up/down and key up/down stays balanced
    if (processWaitCommand) {
        if (waitCommand == "LBUTTON") {
            processWaitCommand = false;
            this->lastCommandTime = KSystem::getMicroCounter();
            input->mouseButton(0, 0, waitMouseX, waitMouseY);
            klog("script WHILEWAITING LBUTTON");
            return;
        } else if (waitCommand == "KEY") {
            processWaitCommand = false;
            this->lastCommandTime = KSystem::getMicroCounter();
            input->key(waitKey, 0, 0);
            klog("script WHILEWAITING KEY");
            return;
        }
    }

    if (this->nextCommand=="MOUSEDOWN" || this->nextCommand=="MOUSEUP") {
        std::vector<BString> items;
        this->nextValue.split(',', items);
        if (items.size()!=3) {
            klog("script: %s %s should have 3 values: %s", this->directory.c_str(), this->nextCommand.c_str(), this->nextValue.c_str());
            exit(99);
        }
        input->mouseButton((this->nextCommand=="MOUSEDOWN")?1:0, atoi(items[0].c_str()), atoi(items[1].c_str()), atoi(items[2].c_str()));
        instance->readCommand();
    } else if (this->nextCommand=="KEYDOWN" || this->nextCommand=="KEYUP") {
        input->key(atoi(this->nextValue.c_str()), 0, (this->nextCommand=="KEYDOWN")?1:0);
        instance->readCommand();
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
        {
            std::unique_lock<std::mutex> boxedWineCriticalSection(comparingCondMutex);
            if (comparingPixels == COMPARING_PIXELS_WORKING) {
                return;
            }
        }
        std::vector<BString> items;
        this->nextValue.split(',', items);              

        if (items.size()>4) {
            U32 x = atoi(items[0].c_str());
            U32 y = atoi(items[1].c_str());
            U32 w = atoi(items[2].c_str());
            U32 h = atoi(items[3].c_str());
            BString fileName = items[4];
                
            if (lastFileName != fileName) {
                if (image_data) {
                    free(image_data);
                }
                image_data = stbi_load((directory.stringByApppendingPath(fileName)).c_str(), &image_width, &image_height, nullptr, 4);
                lastFileName = fileName;
                U32 len = image_width * 4 * image_height;
                if (bufferlen < len) {
                    if (buffer) {
                        delete[] buffer;
                    }
                    buffer = new U8[len];
                    bufferlen = len;
                }
                // reverses RGB
                flipRGBBitmap(image_data, image_width * 4, image_height);
            }            
            std::unique_lock<std::mutex> boxedWineCriticalSection(comparingCondMutex);
            if (comparingPixels == COMPARING_PIXELS_SUCCESS) {
                klog("script: screen shot matched, %s", fileName.c_str());
                this->instance->readCommand();
                this->timerWhileWaiting = 0;
                this->lastCommandTime += 4000000; // sometimes the screen isn't ready for input even though you can see it
                this->instance->lastScreenRead = KSystem::getMicroCounter();
                comparingPixels = COMPARING_PIXELS_WAITING;
            } else if (comparingPixels == COMPARING_PIXELS_WAITING && screen->partialScreenShot(B(""), x, y, w, h, buffer, bufferlen)) {
                if (comparingThread.native_handle() == 0) {
                    comparingThread = std::thread(bitmapCompareThread, this);
                }
                comparingCond.notify_one();
            }
        } else if (items.size()>0) {
            BString fileName = items[0];

            if (lastFileName != fileName) {
                if (image_data) {
                    free(image_data);
                }
                image_data = stbi_load((directory.stringByApppendingPath(fileName)).c_str(), &image_width, &image_height, nullptr, 4);
                lastFileName = fileName;
                U32 len = image_width * 4 * image_height;
                if (bufferlen < len) {
                    if (buffer) {
                        delete[] buffer;
                    }
                    buffer = new U8[len];
                    bufferlen = len;
                }
                // reverses RGB
                flipRGBBitmap(image_data, image_width * 4, image_height);
            }
            std::unique_lock<std::mutex> boxedWineCriticalSection(comparingCondMutex);
            if (comparingPixels == COMPARING_PIXELS_SUCCESS) {
                klog("script: screen shot matched, %s", fileName.c_str());
                this->instance->readCommand();
                this->timerWhileWaiting = 0;
                this->lastCommandTime += 4000000; // sometimes the screen isn't ready for input even though you can see it
                this->instance->lastScreenRead = KSystem::getMicroCounter();
                comparingPixels = COMPARING_PIXELS_WAITING;
            } else if (comparingPixels == COMPARING_PIXELS_WAITING && screen->screenShot(B(""), buffer, bufferlen)) {
                if (comparingThread.native_handle() == 0) {
                    comparingThread = std::thread(bitmapCompareThread, this);
                }
                comparingCond.notify_one();
            }
        }        
    }
    if ((this->nextCommand == "SCREENSHOT" || this->nextCommand == "DONE") && timerWhileWaiting && nextWaitTime < KSystem::getMilliesSinceStart()) {
        nextWaitTime = KSystem::getMilliesSinceStart() + timerWhileWaiting * 1000;
        if (waitCommand == "LBUTTON") {
            input->mouseButton(1, 0, waitMouseX, waitMouseY);
            processWaitCommand = true;
            this->lastCommandTime = KSystem::getMicroCounter();
        } else if (waitCommand == "KEY") {
            input->key(waitKey, 0, 1);
            processWaitCommand = true;
            this->lastCommandTime = KSystem::getMicroCounter();
        }
    }
    if (KSystem::getMicroCounter()>this->lastCommandTime+1000000*60*5) {
        klog("script timed out %s", this->directory.c_str());
        screen->screenShot(B("failed.bmp"), nullptr, 0);
        screen->saveBmp(B("failed_diff.bmp"), output, 32, image_width, image_height);
        quit();
        exit(2);
    }
}

void Player::quit() {
    {
        std::unique_lock<std::mutex> boxedWineCriticalSection(comparingCondMutex);
        comparingPixels = COMPARING_PIXELS_DONE;
        comparingCond.notify_one();
    }
    if (comparingThread.joinable()) {
        comparingThread.join();
    }
}

#endif
