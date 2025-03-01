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

#ifdef BOXEDWINE_RECORDER
Recorder* Recorder::instance;

void Recorder::start(BString directory) {
    Recorder::instance = new Recorder();
    instance->directory = directory;
    Fs::makeNativeDirs(directory);
    instance->file.createNew(BString(directory+"/"+RECORDER_SCRIPT));
    instance->screenShotCount = 0;
    instance->out("VERSION=2\r\n");
}

void Recorder::out(const char* s) {
    file.write(s);
    file.flush();
}

void Recorder::initCommandLine(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args) {
    out("ROOT=");
    out(root.c_str());
    out("\r\n");

    for (auto& zip : zips) {
        out("ZIP=");
        out(zip.c_str());
        out("\r\n");
    }

    out("CWD=");
    out(working.c_str());
    out("\r\n");

    out("ARGC=");    
    out(BString::valueOf((U32)args.size()).c_str());
    out("\r\n");

    for (U32 i=0;i<args.size();i++) {
        out("ARG");
        out(BString::valueOf(i).c_str());
        out("=");
        out(args[i].c_str());
        out("\r\n");
    }
}

void Recorder::fullScrennShot() {
    BString fileName(this->directory);

    this->screenShotCount++;
    fileName.append(Fs::nativePathSeperator);
    fileName.append("screenshot");
    fileName.append(BString::valueOf(this->screenShotCount));
    fileName.append(".bmp");    
    KNativeSystem::getScreen()->screenShot(fileName, nullptr, 0);
    out("SCREENSHOT=");
    out(Fs::getFileNameFromNativePath(fileName).c_str());
    out("\r\n");
}

void Recorder::partialScreenShot(U32 x, U32 y, U32 w, U32 h) {
    BString fileName(this->directory);

    this->screenShotCount++;
    fileName.append(Fs::nativePathSeperator);
    fileName.append("screenshot");
    fileName.append(BString::valueOf(this->screenShotCount));
    fileName.append(".bmp");  
    KNativeSystem::getScreen()->partialScreenShot(fileName, x, y, w, h, nullptr, 0);
    out("SCREENSHOT=");
    out(BString::valueOf(x).c_str());
    out(",");
    out(BString::valueOf(y).c_str());
    out(",");
    out(BString::valueOf(w).c_str());
    out(",");
    out(BString::valueOf(h).c_str());
    out(",");
    out(Fs::getFileNameFromNativePath(fileName).c_str());
    out("\r\n");
}

void Recorder::takeScreenShot() {
    bool tracking = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    KNativeScreenPtr screen = KNativeSystem::getScreen();
    screen->startRecorderScreenShot();
    screen->getInput()->processCustomEvents([this, &x, &y, &w, &h, screen](bool isKeyDown, int key, bool isF11) {
            if (isF11) {
                if (w == 0 || h == 0) {
                    fullScrennShot();
                } else {                    
                    partialScreenShot(x, y, w, h);
                }
                screen->finishRecorderScreenShot();
                return false;
            }
            return true;
        }, [&tracking, &x, &y, &w, &h](bool isButtonDown, int button, int mousex, int mousey) {
            if (isButtonDown) {
                tracking = true;
                x = mousex;
                y = mousey;
            } else {
                tracking = false;
                if (mousex > x)
                    w = mousex - x;
                if (mousey > y)
                    h = mousey - y;
            }
            return true;
        }, [&tracking, &x, &y, &w, &h, screen](int mousex, int mousey) {
            if (tracking) {
                if (mousex > x)
                    w = mousex - x;
                if (mousey > y)
                    h = mousey - y;
                screen->drawRectOnPushedSurfaceAndDisplay(x, y, w, h, 0x80, 0x80, 0x80, 0x80);
            }
            return true;
        });     
}

void Recorder::checkInputModifiers() {
    U32 inputModifers = KNativeSystem::getCurrentInput()->getInputModifiers();
    if (inputModifers != currentInputModifiers) {
        currentInputModifiers = inputModifers;
        out("MODIFIERS=");
        out(BString::valueOf(currentInputModifiers).c_str());
        out("\r\n");
    }
}

void Recorder::onMouseMove(U32 x, U32 y) {
    checkInputModifiers();
    out("MOVETO=");
    out(BString::valueOf(x).c_str());
    out(",");
    out(BString::valueOf(y).c_str());
    out("\r\n");
}

void Recorder::onMouseButton(U32 down, U32 button, U32 x, U32 y) {
    checkInputModifiers();
    if (down) {
        out("MOUSEDOWN=");
    } else {
        out("MOUSEUP=");
    }
    out(BString::valueOf(button).c_str());
    out(",");
    out(BString::valueOf(x).c_str());
    out(",");
    out(BString::valueOf(y).c_str());
    out("\r\n");
}

void Recorder::onKey(U32 key, U32 down) {
    checkInputModifiers();
    if (down) {
        out("KEYDOWN=");
    } else {
        out("KEYUP=");
    }
    out(BString::valueOf(key).c_str());
    out("\r\n");
}

void Recorder::close() {
    out("DONE");
    file.close();
}

void BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(int x, int y) {
    if (Recorder::instance) {
        Recorder::instance->onMouseMove(x, y);
    }
}

void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(int button, int x, int y) {
    if (Recorder::instance) {
        Recorder::instance->onMouseButton(1, button, x, y);
    }
}

void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(int button, int x, int y) {
    if (Recorder::instance) {
        Recorder::instance->onMouseButton(0, button, x, y);
    }
}

bool BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(int key, bool isF11) {
    if (Recorder::instance) {
        if (isF11) {
            Recorder::instance->takeScreenShot();
            return true;
        } else {
            Recorder::instance->onKey(key, 1);
        }
    }
    return false;
}

bool BOXEDWINE_RECORDER_HANDLE_KEY_UP(int key, bool isF11) {
    if (Recorder::instance) {
        if (isF11) {
            return true;
        }    
        Recorder::instance->onKey(key, 0);
    }
    return false;
}

U32 BOXEDWINE_RECORDER_QUIT() {
    if (Recorder::instance) {
        Recorder::instance->close();
    }
    if (Player::instance) {
        Player::instance->quit();
        if (Player::instance->nextCommand=="DONE") {
            klog("script: success.  Setting exit code to 111");
            return 111;
        } else {
            klog("script: failed");
            klog_fmt("  nextCommand is: %s", Player::instance->nextCommand.c_str());
            KNativeScreenPtr screen = KNativeSystem::getScreen();
            if (screen) {
                screen->screenShot(B("failed.bmp"), nullptr, 0);
            }
        }        
    }
    return 1;
}

void BOXEDWINE_RECORDER_RUN_SLICE() {
    if (Player::instance) {
        Player::instance->runSlice();
    }
}

void BOXEDWINE_RECORDER_INIT(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args) {
    if (Recorder::instance) {
        Recorder::instance->initCommandLine(root, zips, working, args);
    } 
    if (Player::instance) {
        Player::instance->initCommandLine(root, zips, working, args);
    }
}
#endif
