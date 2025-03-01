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

#ifndef __RECORDER_H__
#define __RECORDER_H__

#ifdef BOXEDWINE_RECORDER
#define RECORDER_SCRIPT "script.txt"

class Recorder {
public:
    static void start(BString directory);
    static Recorder* instance;

    void initCommandLine(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args);
    void takeScreenShot();
    void onMouseMove(U32 x, U32 y);
    void onMouseButton(U32 down, U32 button, U32 x, U32 y);
    void onKey(U32 key, U32 down);
    void close();

    BWriteFile file;
    BString directory;
private:
    void out(const char* s);
    int screenShotCount = 0;
    void fullScrennShot();
    void partialScreenShot(U32 x, U32 y, U32 w, U32 h);
    void checkInputModifiers();

    U32 currentInputModifiers = 0;
};

void BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(int x, int y);
void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(int button, int x, int y); // 0 left, 1 right, 2 middle
void BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(int button, int x, int y);
bool BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(int key, bool isF11);
bool BOXEDWINE_RECORDER_HANDLE_KEY_UP(int key, bool isF11);
U32 BOXEDWINE_RECORDER_QUIT();
void BOXEDWINE_RECORDER_RUN_SLICE();
void BOXEDWINE_RECORDER_INIT(BString root, const std::vector<BString>& zips, BString working, const std::vector<BString>& args);
#else
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_MOVE(x, y)
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_DOWN(b, x, y)
#define BOXEDWINE_RECORDER_HANDLE_MOUSE_BUTTON_UP(b, x, y)
#define BOXEDWINE_RECORDER_HANDLE_KEY_DOWN(x, y) false
#define BOXEDWINE_RECORDER_HANDLE_KEY_UP(x, y) false
#define BOXEDWINE_RECORDER_QUIT() 0
#define BOXEDWINE_RECORDER_RUN_SLICE();
#define BOXEDWINE_RECORDER_INIT(root, zips, working, args)
#endif

#endif