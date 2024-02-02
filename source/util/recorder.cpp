#include "boxedwine.h"
#include "knativewindow.h"

#ifdef BOXEDWINE_RECORDER
Recorder* Recorder::instance;

void Recorder::start(BString directory) {
    Recorder::instance = new Recorder();
    instance->directory = directory;
    instance->file.createNew(BString(directory+"/"+RECORDER_SCRIPT));
    instance->screenShotCount = 0;
    instance->out("VERSION=1\r\n");
}

void Recorder::out(const char* s) {
    file.write(s);
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
    U32 crc = 0;

    this->screenShotCount++;
    fileName.append(Fs::nativePathSeperator);
    fileName.append("screenshot");
    fileName.append(BString::valueOf(this->screenShotCount));
    fileName.append(".bmp");    
    KNativeWindow::getNativeWindow()->screenShot(fileName, &crc);
    out("SCREENSHOT=");
    out(BString::valueOf(crc).c_str());
    out(",");
    out(fileName.c_str());
    out("\r\n");
}

void Recorder::partialScreenShot(U32 x, U32 y, U32 w, U32 h) {
    BString fileName(this->directory);
    U32 crc = 0;

    this->screenShotCount++;
    fileName.append(Fs::nativePathSeperator);
    fileName.append("screenshot");
    fileName.append(BString::valueOf(this->screenShotCount));
    fileName.append(".bmp");  
    KNativeWindow::getNativeWindow()->partialScreenShot(fileName, x, y, w, h, &crc);
    out("SCREENSHOT=");
    out(BString::valueOf(x).c_str());
    out(",");
    out(BString::valueOf(y).c_str());
    out(",");
    out(BString::valueOf(w).c_str());
    out(",");
    out(BString::valueOf(h).c_str());
    out(",");
    out(BString::valueOf(crc).c_str());
    out(",");
    out(fileName.c_str());
    out("\r\n");
}

void Recorder::takeScreenShot() {
    bool tracking = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    KNativeWindow::getNativeWindow()->pushWindowSurface();
    KNativeWindow::getNativeWindow()->processCustomEvents([this, &x, &y, &w, &h](bool isKeyDown, int key, bool isF11) {
            if (isF11) {
                if (w == 0 || h == 0) {
                    fullScrennShot();
                } else {
                    KNativeWindow::getNativeWindow()->popWindowSurface();
                    partialScreenShot(x, y, w, h);
                }
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
        }, [&tracking, &x, &y, &w, &h](int mousex, int mousey) {
            if (tracking) {
                if (mousex > x)
                    w = mousex - x;
                if (mousey > y)
                    h = mousey - y;
                KNativeWindow::getNativeWindow()->drawRectOnPushedSurfaceAndDisplay(x, y, w, h, 0x80, 0x80, 0x80, 0x80);
            }
            return true;
        });     
}

void Recorder::onMouseMove(U32 x, U32 y) {
    out("MOVETO=");
    out(BString::valueOf(x).c_str());
    out(",");
    out(BString::valueOf(y).c_str());
    out("\r\n");
}

void Recorder::onMouseButton(U32 down, U32 button, U32 x, U32 y) {
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
        if (Player::instance->nextCommand=="DONE") {
            klog("script: success.  Setting exit code to 111");
            return 111;
        } else {
            klog("script: failed");
            klog("  nextCommand is: %s", Player::instance->nextCommand.c_str());
            KNativeWindow::getNativeWindow()->screenShot(B("failed.bmp"), nullptr);
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
