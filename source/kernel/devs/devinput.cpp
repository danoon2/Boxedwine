/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#include "devinput.h"
#include "ksignal.h"
#include "kscheduler.h"

#include <string.h>

#include "../../io/fsvirtualopennode.h"

class EventData {
public:
    U64 time;
    U16 type;
    U16 code;
    S32 value;
};

#define INPUT_PROP_POINTER		    0x00	/* needs a pointer */
#define INPUT_PROP_DIRECT		    0x01	/* direct input devices */
#define INPUT_PROP_BUTTONPAD		0x02	/* has button(s) under pad */
#define INPUT_PROP_SEMI_MT		    0x03	/* touch rectangle only */
#define INPUT_PROP_TOPBUTTONPAD		0x04	/* softbuttons at top of pad */
#define INPUT_PROP_POINTING_STICK	0x05	/* is a pointing stick */
#define INPUT_PROP_ACCELEROMETER	0x06	/* has accelerometer */

#define EV_SYN                  0x00
#define EV_KEY                  0x01
#define EV_REL                  0x02
#define EV_ABS                  0x03
#define EV_MSC                  0x04
#define EV_SW                   0x05
#define EV_LED                  0x11
#define EV_SND                  0x12
#define EV_REP                  0x14
#define EV_FF                   0x15
#define EV_PWR                  0x16
#define EV_FF_STATUS            0x17
#define EV_MAX                  0x1f

class DevInput : public FsVirtualOpenNode {
public:
    DevInput(const BoxedPtr<FsNode>& node, U32 flags) : 
        FsVirtualOpenNode(node, flags), 
        asyncProcessId(0),
        asyncProcessFd(0), 
        bustype(0),
        vendor(0),
        product(0),
        version(0),
        mask(0),
        prop(0),
        bufferCond("DevInput::bufferCond") {}
    virtual U32 ioctl(U32 request);
    virtual U32 readNative(U8* buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len);
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events);
    virtual void setAsync(bool isAsync);
    virtual bool isAsync();
    virtual bool isWriteReady();
    virtual bool isReadReady();

    U32 asyncProcessId;
    U32 asyncProcessFd;
    U16 bustype;
    U16 vendor;
    U16 product;
    U16 version;
    std::string name;
    U32 mask;
    U32 prop;
    BOXEDWINE_CONDITION bufferCond;
    std::queue<EventData> eventQueue;
};

class DevInputTouch : public DevInput {
public:
    DevInputTouch(const BoxedPtr<FsNode>& node, U32 flags);
    virtual U32 ioctl( U32 request);

    U32 lastX;
    U32 lastY;
};

class DevInputMouse : public DevInput {
public:
    DevInputMouse(const BoxedPtr<FsNode>& node, U32 flags);
    virtual U32 ioctl(U32 request);
};

class DevInputKeyboard : public DevInput {
public:
    DevInputKeyboard(const BoxedPtr<FsNode>& node, U32 flags);
    virtual U32 ioctl(U32 request);
};

static DevInputTouch* touchEvents;

FsOpenNode* openDevInputTouch(const BoxedPtr<FsNode>& node, U32 flags) {
    touchEvents = new DevInputTouch(node, flags);
    return touchEvents;
}

static DevInputMouse* mouseEvents;

FsOpenNode* openDevInputMouse(const BoxedPtr<FsNode>& node, U32 flags) {
    mouseEvents = new DevInputMouse(node, flags);
    return mouseEvents;
}

static DevInputKeyboard* keyboardEvents;

FsOpenNode* openDevInputKeyboard(const BoxedPtr<FsNode>& node, U32 flags) {
    keyboardEvents = new DevInputKeyboard(node, flags);
    return keyboardEvents;
}

// :TODO: can this be blocking
U32 DevInput::readNative(U8* buffer, U32 len) {
    U32 result = 0;

    while (this->eventQueue.size() && result+16<=len) {
        const EventData& e = this->eventQueue.front();
        U32* b = (U32*)buffer+result;
        b[0] = (U32) (e.time / 1000000); // seconds
        b[1] = (U32) (e.time % 1000000); // microseconds
        b[2] = e.type | ((U32)e.code) << 16;
        b[3] = e.value;
        result+=16;
        this->eventQueue.pop();
    }
    return result;
}

U32 DevInput::writeNative(U8* buffer, U32 len) {
    return len;
}

void DevInput::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parentCondition, this->bufferCond);
    }
}

void writeBit(U32 address, U32 bit) {
    U32 b = bit/8;
    U32 p = bit % 8;
    U32 value = readb(address+b);
    value|=(1<<p);
    writeb(address+b, value);
}

//    struct input_absinfo {
//        __s32 value;
//        __s32 minimum;
//        __s32 maximum;
//        __s32 fuzz;
//        __s32 flat;
//        __s32 resolution;
//    };
void writeAbs(U32 address, U32 value, U32 min, U32 max) {
    writed(address, value);
    writed(address+4, min);
    writed(address+8, max);
    writed(address+12, 0);
    writed(address+16, 0);
    writed(address+20, 96);
}

U32 DevInput::ioctl(U32 request) {
    CPU* cpu = KThread::currentThread()->cpu;
    U32 cmd = request & 0xFFFF;

    switch (cmd) {
        case 0x4501: { // EVIOCGVERSION
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            if (len<4)
                kpanic("Bad length for EVIOCGVERSION: %d", len);
            writed(buffer, this->version);
            return 4;
        }
        case 0x4502: { // EVIOCGID
//            struct input_id {
//                __u16 bustype;
//                __u16 vendor;
//                __u16 product;
//                __u16 version;
//                };
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            if (len!=8)
                kpanic("Bad length for EVIOCGID: %d",len);
            writew(buffer, this->bustype);
            writew(buffer + 2, this->vendor);
            writew(buffer + 4, this->product);
            writew(buffer + 6, this->version);
            return 0;
        }
        case 0x4506: { // EVIOCGNAME
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            return writeNativeString2(buffer, this->name.c_str(), len);
        }
       case 0x4507: { // EVIOCGPHYS
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            return writeNativeString2(buffer, this->name.c_str(), len);
        }
        case 0x4508: { //  EVIOCGUNIQ
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            return writeNativeString2(buffer, this->name.c_str(), len);
        }
        case 0x4509: { // EVIOCGPROP
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            if (len<4)
                kpanic("Bad length for EVIOCGBIT: %d", len);
            writed(buffer, this->prop);
            return 4;
        }
        case 0x4518: { // EVIOCGKEY
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 0;
        }
        case 0x4519: { // EVIOCGLED
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 0;
        }
        case 0x451b: { // EVIOCGSW
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 0;
        }
        case 0x4520: { // EVIOCGBIT
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            if (len<4)
                kpanic("Bad length for EVIOCGBIT: %d", len);
            writed(buffer, this->mask);
            return 4;
        }
        case 0x4524: { // EVIOCGBIT(EV_MSC)
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 0;
        }
        case 0x4525: { // EVIOCGBIT(EV_SW) - Switches
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 0;
        }
        case 0x4532: { // EVIOCGBIT(EV_SND) - Sound Effects
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 0;
        }
        case 0x4535: { // EVIOCGBIT(EV_FF) - Force Feedback
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 0;
        }
        case 0x540B: // TCFLSH
            return 0;		
    }
    return -1;
}

void DevInput::setAsync(bool isAsync) {
    KProcess* process = KThread::currentThread()->process;
    if (isAsync) {
        if (this->asyncProcessId && this->asyncProcessId!=process->id) {
            kpanic("touch_setAsync only supports one process: %d tried to attached but %d already has it", process->id, this->asyncProcessId);
        } else {
            this->asyncProcessId = process->id;
            // :TODO: pass in fildes
            this->asyncProcessFd = KThread::currentThread()->cpu->reg[3].u32;
        }
    } else {
        if (process->id == this->asyncProcessId) {
            this->asyncProcessId = 0;
            this->asyncProcessFd = 0;
        }
    }
}

bool DevInput::isAsync() {
    return this->asyncProcessId == KThread::currentThread()->process->id;
}

bool DevInput::isWriteReady() {
    return this->eventQueue.size()<1000;
}

bool DevInput::isReadReady() {
    return this->eventQueue.size()>0;
}


DevInputTouch::DevInputTouch(const BoxedPtr<FsNode>& node, U32 flags) : DevInput(node, flags) {
    this->bustype = 3;
    this->vendor = 0;
    this->product = 0;
    this->version = 1;
    this->prop = (1 << INPUT_PROP_DIRECT) | (1<<INPUT_PROP_POINTER);
    this->name = "BoxedWine touchpad";
    this->mask = (1<<K_EV_SYN)|(1<<K_EV_KEY)|(1<<K_EV_ABS);
}

U32 DevInputTouch::ioctl(U32 request) {
    CPU* cpu = KThread::currentThread()->cpu;

    switch (request & 0xFFFF) {
        case 0x4521: { // EVIOCGBIT, EV_KEY
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            U32 result = K_BTN_MIDDLE;
            result = (result+7)/8;
            zeroMemory(buffer, len);
            writeBit(buffer, K_BTN_LEFT);
            writeBit(buffer, K_BTN_MIDDLE);
            writeBit(buffer, K_BTN_RIGHT);
            return result;
        }
        case 0x4522: { // EVIOCGBIT, EV_REL
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 1;
        }
        case 0x4523: { // EVIOCGBIT, EV_ABS
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            writeb(buffer, (1 << K_ABS_X) | (1 << K_ABS_Y));
            return 1;
        }
        case 0x4531: { // EVIOCGBIT, EV_LED
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 1;
        }
        case 0x4540: { // EVIOCGABS (ABS_X)
            U32 len = (request & 0x1fff0000) >> 16;
            U32 address = IOCTL_ARG1;
            if (len<24)
                kpanic("Bad length for EVIOCGABS (ABS_X)");
            writeAbs(address, this->lastX, 0, screenCx);
            return 0;
        }
        case 0x4541: { // EVIOCGABS (ABS_Y)
            int len = (request & 0x1fff0000) >> 16;
            int address = IOCTL_ARG1;
            if (len<24)
                kpanic("Bad length for EVIOCGABS (ABS_X)");
            writeAbs(address, this->lastY, 0, screenCy);
            return 0;
        }
        default:
            return DevInput::ioctl(request);
    }
    return -1;
}

DevInputMouse::DevInputMouse(const BoxedPtr<FsNode>& node, U32 flags) : DevInput(node, flags) {
    this->bustype = 3;
    this->vendor = 0x046d;
    this->product = 0xc52b;
    this->version = 0x0111;
    this->name = "BoxedWine mouse";
    this->mask = (1<<K_EV_SYN)|(1<<K_EV_KEY)|(1<<K_EV_REL);
}

U32 DevInputMouse::ioctl( U32 request) {
    CPU* cpu = KThread::currentThread()->cpu;

    switch (request & 0xFFFF) {
        case 0x4521: { // EVIOCGBIT, EV_KEY
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            U32 result = K_BTN_MIDDLE;
            result = (result+7)/8;
            zeroMemory(buffer, len);
            writeBit(buffer, K_BTN_LEFT);
            writeBit(buffer, K_BTN_MIDDLE);
            writeBit(buffer, K_BTN_RIGHT);
            writeBit(buffer, K_BTN_MOUSEWHEEL_UP);
            writeBit(buffer, K_BTN_MOUSEWHEEL_DOWN);
            return result;
        }
        case 0x4522: { // EVIOCGBIT, EV_REL
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            writeb(buffer, (1 << K_REL_X) | (1 << K_REL_Y));
            return 1;
        }
        case 0x4523: { // EVIOCGBIT, EV_ABS
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 1;
        }
        case 0x4531: { // EVIOCGBIT, EV_LED
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 1;
        }
        case 0x4540: { // EVIOCGABS (ABS_X)
            U32 len = (request & 0x1fff0000) >> 16;
            U32 address = IOCTL_ARG1;
            if (len<24)
                kpanic("Bad length for EVIOCGABS (ABS_X)");
            writeAbs(address, 0, 0, 0);
            return 0;
        }
        case 0x4541: { // EVIOCGABS (ABS_Y)
            int len = (request & 0x1fff0000) >> 16;
            int address = IOCTL_ARG1;
            if (len<24)
                kpanic("Bad length for EVIOCGABS (ABS_X)");
            writeAbs(address, 0, 0, 0);
            return 0;
        }
        default:
            return DevInput::ioctl(request);
    }
    return -1;
}


DevInputKeyboard::DevInputKeyboard(const BoxedPtr<FsNode>& node, U32 flags) : DevInput(node, flags) {
    this->bustype = 0x11;
    this->vendor = 1;
    this->product = 1;
    this->version = 0xab41;
    this->name = "BoxedWine Keyboard";
    this->mask = (1<<K_EV_SYN)|(1<<K_EV_KEY);
}

U32 DevInputKeyboard::ioctl(U32 request) {
    CPU* cpu = KThread::currentThread()->cpu;

    switch (request & 0xFFFF) {
        case 0x4521: { // EVIOCGBIT, EV_KEY
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            writeBit(buffer, K_KEY_ESC);
            writeBit(buffer, K_KEY_1);
            writeBit(buffer, K_KEY_2);
            writeBit(buffer, K_KEY_3);
            writeBit(buffer, K_KEY_4);
            writeBit(buffer, K_KEY_5);
            writeBit(buffer, K_KEY_6);
            writeBit(buffer, K_KEY_7);
            writeBit(buffer, K_KEY_8);
            writeBit(buffer, K_KEY_9);
            writeBit(buffer, K_KEY_0);
            writeBit(buffer, K_KEY_MINUS);
            writeBit(buffer, K_KEY_EQUAL);
            writeBit(buffer, K_KEY_BACKSPACE);
            writeBit(buffer, K_KEY_TAB);
            writeBit(buffer, K_KEY_Q);
            writeBit(buffer, K_KEY_W);
            writeBit(buffer, K_KEY_E);
            writeBit(buffer, K_KEY_R);
            writeBit(buffer, K_KEY_T);
            writeBit(buffer, K_KEY_Y);
            writeBit(buffer, K_KEY_U);
            writeBit(buffer, K_KEY_I);
            writeBit(buffer, K_KEY_O);
            writeBit(buffer, K_KEY_P);
            writeBit(buffer, K_KEY_LEFTBRACE);
            writeBit(buffer, K_KEY_RIGHTBRACE);
            writeBit(buffer, K_KEY_ENTER);
            writeBit(buffer, K_KEY_LEFTCTRL);
            writeBit(buffer, K_KEY_A);
            writeBit(buffer, K_KEY_S);
            writeBit(buffer, K_KEY_D);
            writeBit(buffer, K_KEY_F);
            writeBit(buffer, K_KEY_G);
            writeBit(buffer, K_KEY_H);
            writeBit(buffer, K_KEY_J);
            writeBit(buffer, K_KEY_K);
            writeBit(buffer, K_KEY_L);
            writeBit(buffer, K_KEY_SEMICOLON);
            writeBit(buffer, K_KEY_APOSTROPHE);
            writeBit(buffer, K_KEY_GRAVE);
            writeBit(buffer, K_KEY_LEFTSHIFT);
            writeBit(buffer, K_KEY_BACKSLASH);
            writeBit(buffer, K_KEY_Z);
            writeBit(buffer, K_KEY_X);
            writeBit(buffer, K_KEY_C);
            writeBit(buffer, K_KEY_V);
            writeBit(buffer, K_KEY_B);
            writeBit(buffer, K_KEY_N);
            writeBit(buffer, K_KEY_M);
            writeBit(buffer, K_KEY_COMMA);
            writeBit(buffer, K_KEY_DOT);
            writeBit(buffer, K_KEY_SLASH);
            writeBit(buffer, K_KEY_RIGHTSHIFT);
            writeBit(buffer, K_KEY_LEFTALT);
            writeBit(buffer, K_KEY_SPACE);
            writeBit(buffer, K_KEY_CAPSLOCK);
            writeBit(buffer, K_KEY_F1);
            writeBit(buffer, K_KEY_F2);
            writeBit(buffer, K_KEY_F3);
            writeBit(buffer, K_KEY_F4);
            writeBit(buffer, K_KEY_F5);
            writeBit(buffer, K_KEY_F6);
            writeBit(buffer, K_KEY_F7);
            writeBit(buffer, K_KEY_F8);
            writeBit(buffer, K_KEY_F9);
            writeBit(buffer, K_KEY_F10);
            writeBit(buffer, K_KEY_NUMLOCK);
            writeBit(buffer, K_KEY_SCROLLLOCK);

            writeBit(buffer, K_KEY_F11);
            writeBit(buffer, K_KEY_F12);
            writeBit(buffer, K_KEY_RIGHTCTRL);
            writeBit(buffer, K_KEY_RIGHTALT);
            writeBit(buffer, K_KEY_HOME);
            writeBit(buffer, K_KEY_UP);
            writeBit(buffer, K_KEY_PAGEUP);
            writeBit(buffer, K_KEY_LEFT);
            writeBit(buffer, K_KEY_RIGHT);
            writeBit(buffer, K_KEY_END);
            writeBit(buffer, K_KEY_DOWN);
            writeBit(buffer, K_KEY_PAGEDOWN);
            writeBit(buffer, K_KEY_INSERT);
            writeBit(buffer, K_KEY_DELETE);
            writeBit(buffer, K_KEY_PAUSE);
            return (K_KEY_PAUSE+7)/8;
        }
        case 0x4522: { // EVIOCGBIT, EV_REL
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            writeb(buffer, 0);
            return 1;
        }
        case 0x4523: { // EVIOCGBIT, EV_ABS
            U32 len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            writeb(buffer, 0);
            return 1;
        }
        case 0x4531: { // EVIOCGBIT, EV_LED
            int len = (request & 0x1fff0000) >> 16;
            U32 buffer = IOCTL_ARG1;
            zeroMemory(buffer, len);
            return 1;
        }
        default:
            return DevInput::ioctl(request);
    }
    return -1;
}

void queueEvent(DevInput* queue, U32 type, U32 code, U32 value, U64 time) {
    EventData data;

    data.time = time;
    data.type = type;
    data.code = code;
    data.value = value;
    if (queue)
        queue->eventQueue.push(data);
}

/*
void onMouseMove(U32 x, U32 y) {
    U32 send = 0;

    if (x!=lastX) {
        queueEvent(&mouseEvents, K_EV_REL, K_REL_X, x-lastX);
        lastX = x;
        send = 1;
    }
    if (y!=lastY) {
        queueEvent(&mouseEvents, K_EV_REL, K_REL_Y, y-lastY);
        lastY = y;
        send = 1;
    }
    if (send) {
        queueEvent(&mouseEvents, K_EV_SYN, K_SYN_REPORT, 0);
        if (mouseEvents.asyncProcessId) {
            struct KProcess* process = getProcessById(mouseEvents.asyncProcessId);
            if (process)
                signalProcess(process, K_SIGIO);
        }
    }
}

void onMouseButtonUp(U32 button) {
    if (button == 0)
        queueEvent(&mouseEvents, K_EV_KEY, K_BTN_LEFT, 0);
    else if (button == 2)
        queueEvent(&mouseEvents, K_EV_KEY, K_BTN_MIDDLE, 0);
    else if (button == 1)
        queueEvent(&mouseEvents, K_EV_KEY, K_BTN_RIGHT, 0);
    else
        return;
    queueEvent(&mouseEvents, K_EV_SYN, K_SYN_REPORT, 0);
    if (mouseEvents.asyncProcessId) {
        struct KProcess* process = getProcessById(mouseEvents.asyncProcessId);
        if (process)
            signalProcess(process, K_SIGIO);
    }
}

void onMouseButtonDown(U32 button) {
    if (button == 0)
        queueEvent(&mouseEvents, K_EV_KEY, K_BTN_LEFT, 1);
    else if (button == 2)
        queueEvent(&mouseEvents, K_EV_KEY, K_BTN_MIDDLE, 1);
    else if (button == 1)
        queueEvent(&mouseEvents, K_EV_KEY, K_BTN_RIGHT, 1);
    else
        return;
    queueEvent(&mouseEvents, K_EV_SYN, K_SYN_REPORT, 0);
    if (mouseEvents.asyncProcessId) {
        struct KProcess* process = getProcessById(mouseEvents.asyncProcessId);
        if (process)
            signalProcess(process, K_SIGIO);
    }
}
*/

void postSendEvent(DevInput* events, U64 time) {
    if (!events)
        return;
    queueEvent(events, K_EV_SYN, K_SYN_REPORT, 0, time);
    if (events->asyncProcessId) {
        KProcess* process = KSystem::getProcess(events->asyncProcessId);
        if (process) {
            process->signalIO(K_POLL_IN, 0, events->asyncProcessFd);		
        }
    }
    BOXEDWINE_CONDITION_LOCK(events->bufferCond);
    BOXEDWINE_CONDITION_SIGNAL_ALL(events->bufferCond);
    BOXEDWINE_CONDITION_UNLOCK(events->bufferCond);
}

void onMouseButtonUp(U32 button) {
    U64 time = Platform::getSystemTimeAsMicroSeconds();

    if (button == 0)
        queueEvent(touchEvents, K_EV_KEY, K_BTN_LEFT, 0, time);
    else if (button == 2)
        queueEvent(touchEvents, K_EV_KEY, K_BTN_MIDDLE, 0, time);
    else if (button == 1)
        queueEvent(touchEvents, K_EV_KEY, K_BTN_RIGHT, 0, time);
    else
        return;
    postSendEvent(touchEvents, time);
}

void onMouseButtonDown(U32 button) {
    U64 time = Platform::getSystemTimeAsMicroSeconds();

    if (button == 0)
        queueEvent(touchEvents, K_EV_KEY, K_BTN_LEFT, 1, time);
    else if (button == 2)
        queueEvent(touchEvents, K_EV_KEY, K_BTN_MIDDLE, 1, time);
    else if (button == 1)
        queueEvent(touchEvents, K_EV_KEY, K_BTN_RIGHT, 1, time);
    else
        return;
    postSendEvent(touchEvents, time);
}

void onMouseWheel(S32 value) {
    U64 time = Platform::getSystemTimeAsMicroSeconds();

    // Up direction (y > 0)
    if (value > 0)  {
        // Mouse wheel cannot really be held down so the 'up' event is sent, too
        queueEvent(touchEvents, K_EV_KEY, K_BTN_MOUSEWHEEL_UP, value, time);
        queueEvent(touchEvents, K_EV_KEY, K_BTN_MOUSEWHEEL_UP, 0, time);
    }
    // Down direction
    else if (value < 0) {
        queueEvent(touchEvents, K_EV_KEY, K_BTN_MOUSEWHEEL_DOWN, -value, time);
        queueEvent(touchEvents, K_EV_KEY, K_BTN_MOUSEWHEEL_DOWN, 0, time);
    }
    else
        return;

    postSendEvent(touchEvents, time);
}

void onMouseMove(U32 x, U32 y, bool relative) {
    U32 send = 0;
    U64 time = Platform::getSystemTimeAsMicroSeconds();

    if (touchEvents) {
        if (x!=touchEvents->lastX) {
            if (relative) {
                queueEvent(touchEvents, K_EV_REL, K_REL_X, x, time);
            } else {
                queueEvent(touchEvents, K_EV_ABS, K_ABS_X, x, time);
            }
            touchEvents->lastX = x;            
            send = 1;
        }
        if (y!=touchEvents->lastY) {
            if (relative) {
                queueEvent(touchEvents, K_EV_REL, K_REL_Y, y, time);
            } else {
                queueEvent(touchEvents, K_EV_ABS, K_ABS_Y, y, time);
            }
            touchEvents->lastY = y;            
            send = 1;
        }
        if (send) {
            postSendEvent(touchEvents, time);
        }
    }
}

void onKeyDown(U32 code) {
    U64 time = Platform::getSystemTimeAsMicroSeconds();

    if (code == 0)
        return;
    queueEvent(keyboardEvents, K_EV_KEY, code, 1, time);
    postSendEvent(keyboardEvents, time);
}

void onKeyUp(U32 code) {
    U64 time = Platform::getSystemTimeAsMicroSeconds();

    if (code == 0)
        return;
    queueEvent(keyboardEvents, K_EV_KEY, code, 0, time);
    postSendEvent(keyboardEvents, time);
}
