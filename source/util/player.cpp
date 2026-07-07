/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "knativesystem.h"
#include "pixelMatch.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

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

static BString getAutomationScriptPath(BString path, BString& directory) {
    int lastSlash = path.lastIndexOf('/');
    int lastBackslash = path.lastIndexOf('\\');
    int separator = lastSlash > lastBackslash ? lastSlash : lastBackslash;
    if (separator >= 0 && path.substr(separator + 1) == RECORDER_SCRIPT) {
        directory = separator == 0 ? path.substr(0, 1) : path.substr(0, separator);
        return path;
    }
    directory = path;
    return path.stringByApppendingPath(RECORDER_SCRIPT);
}

static bool readVirtualFile(BString path, std::vector<U8>& data) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(BString::empty, path, true);
    if (!node || node->isDirectory()) {
        return false;
    }
    FsOpenNode* openNode = node->open(K_O_RDONLY);
    if (!openNode) {
        return false;
    }
    S64 fileLength = openNode->length();
    if (fileLength < 0) {
        openNode->close();
        return false;
    }
    data.resize((size_t)fileLength);
    U32 totalRead = 0;
    while (totalRead < data.size()) {
        U32 read = openNode->readNative(data.data() + totalRead, (U32)(data.size() - totalRead));
        if (!read) {
            break;
        }
        totalRead += read;
    }
    openNode->close();
    data.resize(totalRead);
    return totalRead == fileLength;
}

static void splitScriptLines(const std::vector<U8>& data, std::vector<BString>& lines) {
    BString line;
    for (U8 value : data) {
        char c = (char)value;
        if (c == '\n') {
            lines.push_back(line);
            line.removeAll();
        } else if (c != '\r') {
            line.append(c);
        }
    }
    if (line.length()) {
        lines.push_back(line);
    }
}

static unsigned char* loadAutomationImage(Player* player, const BString& fileName, int* width, int* height) {
    BString path = player->directory.stringByApppendingPath(fileName);
    if (!player->useVirtualFiles) {
        return stbi_load(path.c_str(), width, height, nullptr, 4);
    }
    std::vector<U8> data;
    if (!readVirtualFile(path, data) || data.empty()) {
        return nullptr;
    }
    return stbi_load_from_memory(data.data(), (int)data.size(), width, height, nullptr, 4);
}

void Player::readCommand() {
    this->nextCommand.clear();
    this->nextValue.clear();

    BString line;
    bool hasLine = false;
    if (this->useVirtualFiles) {
        if (this->scriptLineIndex < this->scriptLines.size()) {
            line = this->scriptLines[this->scriptLineIndex++];
            hasLine = true;
        }
    } else {
        hasLine = file.readLine(line);
    }
    if (!hasLine) {
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
        klog_fmt("malformed script.  Line = %s", line.c_str());
#ifdef BOXEDWINE_MULTI_THREADED
        KSystem::destroy();
#endif
        exit(99);
    }    
    this->lastCommandTime = KSystem::getMicroCounter();
    if (this->nextCommand.length()==0) {
        klog_fmt("malformed script.  Line = %s", line.c_str());
#ifdef BOXEDWINE_MULTI_THREADED
        KSystem::destroy();
#endif
        exit(99);
    }
}

bool Player::start(BString directory) {
    Player::instance = new Player();
    BString script = getAutomationScriptPath(directory, instance->directory);
    instance->file.open(script);
    if (!instance->file.isOpen()) {
        std::vector<U8> scriptData;
        if (readVirtualFile(script, scriptData)) {
            splitScriptLines(scriptData, instance->scriptLines);
            instance->useVirtualFiles = true;
        }
    }
    instance->lastCommandTime = 0;
    instance->lastScreenRead = 0;
    if (!instance->file.isOpen() && instance->scriptLines.empty()) {
        klog_fmt("script not found: %s error=%d(%s)", script.c_str(), errno, strerror(errno));
        exit(100);
    } else {
        klog_fmt("using script: %s", script.c_str());
    }
    instance->readCommand();
    instance->version = instance->nextValue;
    if (instance->version!="2") {
        klog_fmt("script is wrong version, was expecting 2 and instead got %s", instance->version.c_str());
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

#ifdef __EMSCRIPTEN__
static void finishEmscriptenAutomation(U32 code) {
    if (Player::instance) {
        Player::instance->quit();
    }
    emscripten_force_exit(code);
}
#endif

static bool compareScreenshotPixels() {
    if (outputLen < bufferlen) {
        if (output) {
            delete[] output;
        }
        output = new U8[bufferlen];
        outputLen = bufferlen;
    }
    return pixelmatch(image_data, image_width * 4, buffer, image_width * 4, image_width, image_height, output) == 0;
}

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
        if (compareScreenshotPixels()) {
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
            klog_fmt("script: %s MOVETO should have 2 values: %s", this->directory.c_str(), this->nextValue.c_str());
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
            klog_fmt("script: %s %s should have 3 values: %s", this->directory.c_str(), this->nextCommand.c_str(), this->nextValue.c_str());
            exit(99);
        }
        input->mouseButton((this->nextCommand=="MOUSEDOWN")?1:0, atoi(items[0].c_str()), atoi(items[1].c_str()), atoi(items[2].c_str()));
        instance->readCommand();
    } else if (this->nextCommand=="KEYDOWN" || this->nextCommand=="KEYUP") {
        input->key(atoi(this->nextValue.c_str()), 0, (this->nextCommand=="KEYDOWN")?1:0);
        instance->readCommand();
    } else if (this->nextCommand=="WAIT") {
        if (KSystem::getMicroCounter()>this->lastCommandTime+1000000l*this->nextValue.toInt64()) {
            klog_fmt("script: done waiting %s", this->nextValue.c_str());
            instance->readCommand();            
        }
    } else if (this->nextCommand=="DONE") {
#ifdef __EMSCRIPTEN__
        finishEmscriptenAutomation(111);
#endif
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
                image_data = loadAutomationImage(this, fileName, &image_width, &image_height);
                if (!image_data) {
                    klog_fmt("script: screenshot image not found, %s", fileName.c_str());
                    exit(101);
                }
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
                klog_fmt("script: screen shot matched, %s", fileName.c_str());
                this->instance->readCommand();
                this->timerWhileWaiting = 0;
                this->lastCommandTime += 4000000; // sometimes the screen isn't ready for input even though you can see it
                this->instance->lastScreenRead = KSystem::getMicroCounter();
                comparingPixels = COMPARING_PIXELS_WAITING;
            } else if (comparingPixels == COMPARING_PIXELS_WAITING) {
                if (screen->partialScreenShot(B(""), x, y, w, h, buffer, bufferlen)) {
#if defined(__EMSCRIPTEN__) && !defined(BOXEDWINE_MULTI_THREADED)
                    comparingPixels = compareScreenshotPixels() ? COMPARING_PIXELS_SUCCESS : COMPARING_PIXELS_WAITING;
#else
                    if (comparingThread.native_handle() == 0) {
                        comparingThread = std::thread(bitmapCompareThread, this);
                    }
                    comparingCond.notify_one();
#endif
                }
            }
        } else if (items.size()>0) {
            BString fileName = items[0];

            if (lastFileName != fileName) {
                if (image_data) {
                    free(image_data);
                }
                image_data = loadAutomationImage(this, fileName, &image_width, &image_height);
                if (!image_data) {
                    klog_fmt("script: screenshot image not found, %s", fileName.c_str());
                    exit(101);
                }
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
                klog_fmt("script: screen shot matched, %s", fileName.c_str());
                this->instance->readCommand();
                this->timerWhileWaiting = 0;
                this->lastCommandTime += 4000000; // sometimes the screen isn't ready for input even though you can see it
                this->instance->lastScreenRead = KSystem::getMicroCounter();
                comparingPixels = COMPARING_PIXELS_WAITING;
            } else if (comparingPixels == COMPARING_PIXELS_WAITING) {
                if (screen->screenShot(B(""), buffer, bufferlen)) {
#if defined(__EMSCRIPTEN__) && !defined(BOXEDWINE_MULTI_THREADED)
                    comparingPixels = compareScreenshotPixels() ? COMPARING_PIXELS_SUCCESS : COMPARING_PIXELS_WAITING;
#else
                    if (comparingThread.native_handle() == 0) {
                        comparingThread = std::thread(bitmapCompareThread, this);
                    }
                    comparingCond.notify_one();
#endif
                }
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
        klog_fmt("script timed out %s", this->directory.c_str());
        if (this->nextCommand == "SCREENSHOT") {
            std::vector<BString> items;
            this->nextValue.split(',', items);

            if (items.size() > 4) {
                U32 x = atoi(items[0].c_str());
                U32 y = atoi(items[1].c_str());
                U32 w = atoi(items[2].c_str());
                U32 h = atoi(items[3].c_str());

                screen->partialScreenShot(B("failed.bmp"), x, y, w, h, nullptr, 0);
            } else {
                screen->screenShot(B("failed.bmp"), nullptr, 0);
            }
        } else {
            screen->screenShot(B("failed.bmp"), nullptr, 0);
        }
        screen->saveBmp(B("failed_diff.bmp"), output, 32, image_width, image_height);
#ifdef __EMSCRIPTEN__
        finishEmscriptenAutomation(2);
#else
        quit();
        exit(2);
#endif
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
