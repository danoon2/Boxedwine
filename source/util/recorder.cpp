#include "boxedwine.h"
#include "../sdl/sdlwindow.h"
#include <SDL.h>

#ifdef BOXEDWINE_RECORDER
Recorder* Recorder::instance;

void Recorder::start(std::string directory) {
    Recorder::instance = new Recorder();
    instance->directory = directory;
    instance->file = fopen(std::string(directory+"/script.txt").c_str(), "w");
    instance->screenShotCount = 0;
    instance->out("VERSION=1\r\n");
}

void Recorder::out(const char* s) {
    fwrite(s, strlen(s), 1, this->file);
}

void Recorder::initCommandLine(std::string root, std::string zip, std::string working, const char **argv, U32 argc) {
    out("ROOT=");
    out(root.c_str());
    out("\r\n");

    out("ZIP=");
    out(zip.c_str());
    out("\r\n");

    out("CWD=");
    out(working.c_str());
    out("\r\n");

    out("ARGC=");    
    out(std::to_string(argc).c_str());
    out("\r\n");

    for (U32 i=0;i<argc;i++) {
        out("ARG");
        out(std::to_string(i).c_str());
        out("=");
        out(argv[i]);
        out("\r\n");
    }
}

void Recorder::fullScrennShot() {
    std::string fileName(this->directory);
    U32 crc = 0;

    this->screenShotCount++;
    fileName.append(Fs::nativePathSeperator);
    fileName.append("screenshot");
    fileName.append(std::to_string(this->screenShotCount));
    fileName.append(".bmp");    
    sdlScreenShot(fileName, &crc);
    out("SCREENSHOT=");
    out(std::to_string(crc).c_str());
    out(",");
    out(fileName.c_str());
    out("\r\n");
}

void Recorder::partialScreenShot(U32 x, U32 y, U32 w, U32 h) {
    std::string fileName(this->directory);
    U32 crc = 0;

    this->screenShotCount++;
    fileName.append(Fs::nativePathSeperator);
    fileName.append("screenshot");
    fileName.append(std::to_string(this->screenShotCount));
    fileName.append(".bmp");    
    sdlPartialScreenShot(fileName, x, y, w, h, &crc);
    out("SCREENSHOT=");
    out(std::to_string(x).c_str());
    out(",");
    out(std::to_string(y).c_str());
    out(",");
    out(std::to_string(w).c_str());
    out(",");
    out(std::to_string(h).c_str());
    out(",");
    out(std::to_string(crc).c_str());
    out(",");
    out(fileName.c_str());
    out("\r\n");
}

void Recorder::takeScreenShot() {
    SDL_Event e;
    SDL_Rect r;
    bool tracking = false;
    r.x = 0;
    r.y = 0;
    r.w = 0;
    r.h = 0;
    sdlPushWindowSurface();
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_KEYUP) {
            if (e.key.keysym.sym == SDLK_F11) {
                if (r.w==0 || r.h==0) {
                    fullScrennShot();
                } else {
                    sdlPopWindowSurface();
                    partialScreenShot(r.x, r.y, r.w, r.h);
                }
                return;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            tracking = true;
            r.x = e.motion.x;
            r.y = e.motion.y;
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            tracking = false;
            if (e.motion.x>r.x)
                r.w = e.motion.x-r.x;
            if (e.motion.y>r.y)
                r.h = e.motion.y-r.y;            
        } else if (e.type == SDL_MOUSEMOTION) { 
            if (tracking) {
                if (e.motion.x>r.x)
                    r.w = e.motion.x-r.x;
                if (e.motion.y>r.y)
                    r.h = e.motion.y-r.y;
                sdlDrawRectOnPushedSurfaceAndDisplay(r.x, r.y, r.w, r.h, 0x80, 0x80, 0x80, 0x80);
            }
        }
    }       
}

void Recorder::onMouseMove(U32 x, U32 y) {
    out("MOVETO=");
    out(std::to_string(x).c_str());
    out(",");
    out(std::to_string(y).c_str());
    out("\r\n");
}

void Recorder::onMouseButton(U32 down, U32 button, U32 x, U32 y) {
    if (down) {
        out("MOUSEDOWN=");
    } else {
        out("MOUSEUP=");
    }
    out(std::to_string(button).c_str());
    out(",");
    out(std::to_string(x).c_str());
    out(",");
    out(std::to_string(y).c_str());
    out("\r\n");
}

void Recorder::onKey(U32 key, U32 down) {
    if (down) {
        out("KEYDOWN=");
    } else {
        out("KEYUP=");
    }
    out(std::to_string(key).c_str());
    out("\r\n");
}

void Recorder::close() {
    out("DONE");
    fclose(this->file);
}

void BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(void* p) {
    SDL_Event* e = (SDL_Event*)p;
    if (Recorder::instance) {
        Recorder::instance->onMouseMove(e->motion.x, e->motion.y);
    }
}

void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(void* p) {
    SDL_Event* e = (SDL_Event*)p;
    if (Recorder::instance) {
        if (e->button.button==SDL_BUTTON_LEFT) {
            Recorder::instance->onMouseButton(1, 0, e->motion.x, e->motion.y);
        } else if (e->button.button == SDL_BUTTON_MIDDLE) {
            Recorder::instance->onMouseButton(1, 2, e->motion.x, e->motion.y);
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            Recorder::instance->onMouseButton(1, 1, e->motion.x, e->motion.y);
        }
    }
}

void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(void* p) {
    SDL_Event* e = (SDL_Event*)p;
    if (Recorder::instance) {
        if (e->button.button==SDL_BUTTON_LEFT) {
            Recorder::instance->onMouseButton(0, 0, e->motion.x, e->motion.y);
        } else if (e->button.button == SDL_BUTTON_MIDDLE) {
            Recorder::instance->onMouseButton(0, 2, e->motion.x, e->motion.y);
        } else if (e->button.button == SDL_BUTTON_RIGHT) {
            Recorder::instance->onMouseButton(0, 1, e->motion.x, e->motion.y);
        }
    }
}

bool BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(void* p) {
    SDL_Event* e = (SDL_Event*)p;
    if (Recorder::instance) {
        if (e->key.keysym.sym == SDLK_F11) {
            Recorder::instance->takeScreenShot();
            return true;
        } else {
            Recorder::instance->onKey(e->key.keysym.sym, 1);
        }
    }
    return false;
}

bool BOXEDWINE_RECORDER_HANDLE_KEY_UP(void* p) {
    SDL_Event* e = (SDL_Event*)p;
    if (Recorder::instance) {
        if (e->key.keysym.sym == SDLK_F11) {
            return true;
        }    
        Recorder::instance->onKey(e->key.keysym.sym, 0);
    }
    return false;
}

U32 BOXEDWINE_RECORDER_QUIT() {
    if (Recorder::instance) {
        Recorder::instance->close();
    }
    if (Player::instance) {
        if (Player::instance->nextCommand=="DONE") {
            klog("script: success");
            return 1;
        } else {
            klog("script: failed");
            klog("  nextCommand is: %s", Player::instance->nextCommand.c_str());
            sdlScreenShot("failed.bmp", NULL);
        }
    }
    return 0;
}

void BOXEDWINE_RECORDER_RUN_SLICE() {
    if (Player::instance) {
        Player::instance->runSlice();
    }
}

void BOXEDWINE_RECORDER_INIT(std::string root, std::string zip, std::string working, const char **argv, U32 argc) {
    if (Recorder::instance) {
        Recorder::instance->initCommandLine(root, zip, working, argv, argc);
    } 
    if (Player::instance) {
        Player::instance->initCommandLine(root, zip, working, argv, argc);
    }
}
#endif
