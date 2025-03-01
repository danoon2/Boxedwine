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

#ifndef __KNATIVEINPUT_H__
#define __KNATIVEINPUT_H__

#define NATIVE_LEFT_BUTTON_MASK 0x01
#define NATIVE_MIDDLE_BUTTON_MASK 0x02
#define NATIVE_RIGHT_BUTTON_MASK 0x04
#define NATIVE_BUTTON_4_MASK 0x08
#define NATIVE_BUTTON_5_MASK 0x10
#define NATIVE_SHIFT_MASK 0x20
#define NATIVE_CAPS_MASK 0x40
#define NATIVE_CONTROL_MASK 0x80
#define NATIVE_ALT_MASK 0x100
#define NATIVE_NUM_MASK 0x200
#define NATIVE_SCROLL_MASK 0x400

class KNativeInput {
public:
    virtual ~KNativeInput() {}

    virtual void setScreenSize(U32 cx, U32 cy) = 0;
    virtual U32 screenWidth() = 0;
    virtual U32 screenHeight() = 0;    

    virtual bool waitForEvent(U32 ms) = 0; // if return is true, then event is available
    virtual bool processEvents() = 0; // if return is false, then shutdown    
    virtual void processCustomEvents(std::function<bool(bool isKeyDown, int key, bool isF11)> onKey, std::function<bool(bool isButtonDown, int button, int x, int y)> onMouseButton, std::function<bool(int x, int y)> onMouseMove) = 0;

    virtual bool mouseMove(int x, int y, bool relative) = 0;
    virtual bool mouseWheel(int amount, int x, int y) = 0;
    virtual bool mouseButton(U32 down, U32 button, int x, int y) = 0;
    virtual bool key(U32 sdlScanCode, U32 key, U32 down) = 0;  // the key code is specific to the back end

    virtual bool getMousePos(int* x, int* y, bool allowWarp = true) = 0;
    virtual void setMousePos(int x, int y) = 0;
    virtual U32 getInputModifiers() = 0;    

    virtual void runOnUiThread(std::function<void()> callback) = 0;

    std::vector<std::function<void()>> onFocusGained;
    std::vector<std::function<void()>> onFocusLost;
};

typedef std::shared_ptr<KNativeInput> KNativeInputPtr;

#define BOXED_KEYEVENTF_EXTENDEDKEY        0x0001
#define BOXED_KEYEVENTF_KEYUP              0x0002
#define BOXED_KEYEVENTF_UNICODE            0x0004
#define BOXED_KEYEVENTF_SCANCODE           0x0008

#define BOXED_VK_CANCEL              0x03
#define BOXED_VK_BACK                0x08
#define BOXED_VK_TAB                 0x09
#define BOXED_VK_RETURN              0x0D
#define BOXED_VK_SHIFT               0x10
#define BOXED_VK_CONTROL             0x11
#define BOXED_VK_MENU                0x12
#define BOXED_VK_PAUSE               0x13
#define BOXED_VK_CAPITAL             0x14

#define BOXED_VK_ESCAPE              0x1B

#define BOXED_VK_SPACE               0x20
#define BOXED_VK_PRIOR               0x21
#define BOXED_VK_NEXT                0x22
#define BOXED_VK_END                 0x23
#define BOXED_VK_HOME                0x24
#define BOXED_VK_LEFT                0x25
#define BOXED_VK_UP                  0x26
#define BOXED_VK_RIGHT               0x27
#define BOXED_VK_DOWN                0x28
#define BOXED_VK_INSERT              0x2D
#define BOXED_VK_DELETE              0x2E
#define BOXED_VK_HELP                0x2F

#define BOXED_VK_MULTIPLY            0x6A
#define BOXED_VK_ADD                 0x6B
#define BOXED_VK_DECIMAL             0x6E
#define BOXED_VK_DIVIDE              0x6F

#define BOXED_VK_F1                  0x70
#define BOXED_VK_F2                  0x71
#define BOXED_VK_F3                  0x72
#define BOXED_VK_F4                  0x73
#define BOXED_VK_F5                  0x74
#define BOXED_VK_F6                  0x75
#define BOXED_VK_F7                  0x76
#define BOXED_VK_F8                  0x77
#define BOXED_VK_F9                  0x78
#define BOXED_VK_F10                 0x79
#define BOXED_VK_F11                 0x7A
#define BOXED_VK_F12                 0x7B
#define BOXED_VK_F24                 0x87

#define BOXED_VK_NUMLOCK             0x90
#define BOXED_VK_SCROLL              0x91

#define BOXED_VK_LSHIFT              0xA0
#define BOXED_VK_RSHIFT              0xA1
#define BOXED_VK_LCONTROL            0xA2
#define BOXED_VK_RCONTROL            0xA3
#define BOXED_VK_LMENU               0xA4
#define BOXED_VK_RMENU               0xA5

#define BOXED_VK_OEM_1               0xBA
#define BOXED_VK_OEM_PLUS            0xBB
#define BOXED_VK_OEM_COMMA           0xBC
#define BOXED_VK_OEM_MINUS           0xBD
#define BOXED_VK_OEM_PERIOD          0xBE
#define BOXED_VK_OEM_2               0xBF
#define BOXED_VK_OEM_3               0xC0
#define BOXED_VK_OEM_4               0xDB
#define BOXED_VK_OEM_5               0xDC
#define BOXED_VK_OEM_6               0xDD
#define BOXED_VK_OEM_7               0xDE

#endif
