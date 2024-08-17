/*
 *  Copyright (C) 2024  The BoxedWine Team
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
#include "../../../tools/x11/X11_def.h"
#include "x11.h"
#include "knativewindow.h"

Int99Callback int9BCallback[X11_COUNT];
U32 int9BCallbackSize = X11_COUNT;

#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)
#define ARG5 cpu->peek32(5)
#define ARG6 cpu->peek32(6)
#define ARG7 cpu->peek32(7)
#define ARG8 cpu->peek32(8)
#define ARG9 cpu->peek32(9)
#define ARG10 cpu->peek32(10)
#define ARG11 cpu->peek32(11)
#define ARG12 cpu->peek32(12)

// Display* XOpenDisplay(const char* displayName) 
static void x11_OpenDisplay(CPU* cpu) {
    // winex11 always pass null for displayName
    EAX = XServer::getServer()->openDisplay(cpu->thread);
}

static void x11_CloseDisplay(CPU* cpu) {
    kpanic("x11_CloseDisplay");
}

static void x11_GrabServer(CPU* cpu) {
    kpanic("x11_GrabServer");
}

static void x11_UnGrabServer(CPU* cpu) {
    kpanic("x11_UnGrabServer");
}

// Status XInitThreads()
// This function returns a nonzero status if initialization was successful; otherwise, it returns zero
static void x11_InitThread(CPU* cpu) {
    EAX = 1;
}

static void x11_ClearArea(CPU* cpu) {
    kpanic("x11_ClearArea");
}

// int XSync(Display* display, Bool discard)
static void x11_Sync(CPU* cpu) {
    EAX = Success;
}

// Window XCreateWindow(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int class, Visual* visual, unsigned long valuemask, XSetWindowAttributes* attributes)
static void x11_CreateWindow(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr parent = server->getWindow(ARG2);
    U32 x = ARG3;
    U32 y = ARG4;
    U32 width = ARG5;
    U32 height = ARG6;
    U32 border_width = ARG7;
    U32 depth = ARG8;
    U32 c_class = ARG9;
    // Visual visual;
    // visual.read(memory, ARG10);
    U32 valuemask = ARG11;
    XSetWindowAttributes tmpAttributes;
    XSetWindowAttributes* attributes = XSetWindowAttributes::get(memory, ARG12, &tmpAttributes);    

    if (c_class == CopyFromParent) {
        c_class = parent->c_class;
    }
    XWindowPtr result = server->createNewWindow(data->displayId, parent, width, height, depth, x, y, c_class, border_width);

    if (server->trace) {
        BString log;

        log.append(data->displayId, 16);
        log += " CreateWindow depth=";
        log.append(depth, 16);
        log += " window=";
        log.append(result->id, 16);
        log += " parent=";
        log.append(parent->id, 16);
        log += " x=";
        log.append(x);
        log += " y=";
        log.append(y);
        log += " width=";
        log.append(width);
        log += " width=";
        log.append(height);
        klog(log.c_str());
    }

    if (attributes) {
        result->setAttributes(data, attributes, valuemask);
    }
    wRECT windowRect;
    windowRect.left = x;
    windowRect.top = y;
    windowRect.right = x + width;
    windowRect.bottom = y + height;
    wRECT clientRect;
    clientRect.left = 0;
    clientRect.top = 0;
    clientRect.right = width;
    clientRect.bottom = height;
    KNativeWindow::getNativeWindow()->createWnd(thread, thread->process->id, result->id, windowRect, clientRect);    
    EAX = result->id;
}

// Bool XTranslateCoordinates(Display* display, Window src_w, Window dest_w, int src_x, int src_y, int* dest_x_return, int* dest_y_return, Window* child_return) {
static void x11_TranslateCoordinates(CPU* cpu) {
    // child_return not used in winx11
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr src_w = server->getWindow(ARG2);
    XWindowPtr dest_w = server->getWindow(ARG3);
    if (!src_w || !dest_w) {
        EAX = BadWindow;
        return;
    }
    S32 x = ARG4;
    S32 y = ARG5;
    src_w->windowToScreen(x, y);
    XWindowPtr w = server->getRoot()->getWindowFromPoint(x, y);
    dest_w->screenToWindow(x, y);
    memory->writed(ARG6, x);
    memory->writed(ARG7, y);
    memory->writed(ARG8, w ? w->id : 0);
    EAX = Success;
}

// int XDestroyWindow(Display* display, Window w) 
static void x11_DestroyWindow(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr w = server->getWindow(ARG2);
    U32 mask = ARG3;
    if (!w) {
        EAX = BadWindow;
        return;
    }
    EAX = server->destroyWindow(w->id);
    WndPtr nativeWindow = KNativeWindow::getNativeWindow()->getWnd(w->id);
    if (nativeWindow) {
        nativeWindow->destroy();
    }
}

static void x11_ReparentWindow(CPU* cpu) {
    kpanic("x11_ReparentWindow");
}

static void x11_QueryTree(CPU* cpu) {
    kpanic("x11_QueryTree");
}

//int XChangeWindowAttributes(Display* display, Window w, unsigned long valuemask, XSetWindowAttributes* attributes)
static void x11_ChangeWindowAttributes(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr w = server->getWindow(ARG2);
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    U32 mask = ARG3;
    if (!w) {
        EAX = BadWindow;
        return;
    }
    XSetWindowAttributes tmpAttributes;
    XSetWindowAttributes* attributes = XSetWindowAttributes::get(memory, ARG4, &tmpAttributes);
    w->setAttributes(data, attributes, mask);
    EAX = Success;
}

// int XConfigureWindow(Display* display, Window w, unsigned int value_mask, XWindowChanges* values)
static void x11_ConfigureWindow(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr w = server->getWindow(ARG2);
    U32 mask = ARG3;
    if (!w) {
        EAX = BadWindow;
        return;
    }
    XWindowChanges changes;
    changes.read(memory, ARG4);

    if (server->trace) {
        BString log;
        DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);

        log.append(data->displayId, 16);
        log += " ConfigureWindow";
        log += " window=";
        log.append(w->id, 16);
        log += " values={";
        if (mask & CWX) {
            log += "x=";
            log += changes.x;
        }
        if (mask & CWY) {
            log += " y=";
            log += changes.y;
        }
        if (mask & CWWidth) {
            log += " width=";
            log += changes.width;
        }
        if (mask & CWHeight) {
            log += " height=";
            log += changes.height;
        }
        if (mask & CWStackMode) {
            log += " stack-mode=";
            log.append(changes.stack_mode, 16);
        }
        log += "}";
        klog(log.c_str());
    }

    EAX = w->configure(mask, &changes);    
}

// Status XReconfigureWMWindow(Display* display, Window w, int screen_number, unsigned int mask, XWindowChanges* changes) {
static void x11_ReconfigureWMWindow(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr w = server->getWindow(ARG2);
    U32 mask = ARG4;
    if (!w) {
        EAX = BadWindow;
        return;
    }
    XWindowChanges changes;
    changes.read(memory, ARG5);
    EAX = w->configure(mask, &changes);
}

// int XSetInputFocus(Display* display, Window focus, int revert_to, Time time)
static void x11_SetInputFocus(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    U32 focus = ARG2;
    XWindowPtr w;

    if (focus != PointerRoot) {
        w = server->getWindow(ARG2);
        if (!w) {
            EAX = BadWindow;
            return;
        }
    }    
    EAX = server->setInputFocus(data, focus, ARG3, ARG4, true);
}

// int XSelectInput(Display* display, Window w, long event_mask)
static void x11_SelectInput(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    //Display* display = X11::getDisplay(thread, ARG1);
    XWindowPtr window = server->getWindow(ARG2);

    if (!window) {
        EAX = BadWindow;
        return;
    }
    data->setEventMask(ARG2, ARG3);
    EAX = Success;
}

// int XFindContext(Display* display, XID rid, XContext context, XPointer* data_return)
static void x11_FindContext(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    U32 result = 0;

    EAX = data->getContextData(ARG2, ARG3, result);
    if (EAX == XCSUCCESS) {
        memory->writed(ARG4, result);
    }
}

// int XSaveContext(Display* display, XID rid, XContext context, _Xconst char* data)
static void x11_SaveContext(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    EAX = data->setContextData(ARG2, ARG3, ARG4);
}

// int XDeleteContext(Display* display, XID rid, XContext context)
static void x11_DeleteContext(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    EAX = data->deleteContextData(ARG2, ARG3);
}

// int XGetInputFocus(Display* display, Window* focus_return, int* revert_to_return)
static void x11_GetInputFocus(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    memory->writed(ARG2, server->inputFocus->id);
    memory->writed(ARG3, server->inputFocusRevertTo);
    EAX = Success;
}

static void x11_FreeFont(CPU* cpu) {
    kpanic("x11_FreeFont");
}

static void x11_MoveResizeWindow(CPU* cpu) {
    kpanic("x11_MoveResizeWindow");
}

// int XMapWindow(Display* display, Window w) {
static void x11_MapWindow(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr window = server->getWindow(ARG2);

    if (!window) {
        EAX = BadWindow;
        return;
    }
    EAX = server->mapWindow(data, window);
}

// int XUnmapWindow(Display* display, Window w) {
static void x11_UnmapWindow(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr window = server->getWindow(ARG2);

    if (!window) {
        EAX = BadWindow;
        return;
    }
    EAX = server->unmapWindow(data, window);
}

// int XGrabPointer(Display* display, Window grab_window, Bool owner_events, unsigned int event_mask, int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor, Time time)
static void x11_GrabPointer(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr window = server->getWindow(ARG2);
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);        

    if (!window) {
        EAX = BadWindow;
        return;
    }
    if (!window->mapped()) {
        EAX = GrabNotViewable;
        return;
    }
    if (ARG3) {
        // not used by winex11
        kpanic("XGrabPointer owner_events not implemented");
    }
    if (ARG4 == GrabModeSync || ARG5 == GrabModeSync) {
        // not used by winex11
        kpanic("XGrabPointer GrabModeSync not implemented");
    }
    if (ARG8) {
        // not used by winex11
        kpanic("XGrabPointer cursor not implemented");
    }
    XWindowPtr confined;

    if (ARG7) {
        confined = server->getWindow(ARG7);
        if (!confined) {
            EAX = BadWindow;
            return;
        }
    }

    if (server->trace) {
        BString log;

        log.append(data->displayId, 16);
        log += " GrabPointer";
        log += " grab-window=";
        log.append(window->id, 16);
        log += " event-mask=";
        log.append(ARG4, 16);
        klog(log.c_str());
    }

    EAX = server->grabPointer(data, window, confined, ARG4, ARG9);
}

// int XUngrabPointer(Display* display, Time time)
static void x11_UngrabPointer(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);

    klog("XUngrabPointer thread=%x process=%x", cpu->thread->id, cpu->thread->process->id);

    EAX = server->ungrabPointer(data, ARG2);
}

// int XWarpPointer(Display* display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int dest_y)
static void x11_WarpPointer(CPU* cpu) {
    EAX = Success;
}

// Bool XQueryPointer(Display* display, Window w, Window* root_return, Window* child_return, int* root_x_return, int* root_y_return, int* win_x_return, int* win_y_return, unsigned int* mask_return)
static void x11_QueryPointer(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr window = server->getWindow(ARG2);
    XWindowPtr root = server->getRoot();

    if (!window) {
        EAX = BadWindow;
        return;
    }
    S32 x = 0;
    S32 y = 0;
    KNativeWindow::getNativeWindow()->getMousePos(&x, &y);

    memory->writed(ARG3, root->id);

    XWindowPtr child = root->getWindowFromPoint(x, y);
    memory->writed(ARG4, child->id);
    memory->writed(ARG5, x);
    memory->writed(ARG6, y);
    window->screenToWindow(x, y);
    memory->writed(ARG7, x);
    memory->writed(ARG8, y);
    memory->writed(ARG9, server->getInputModifiers());
    EAX = Success;
}

// int XmbTextListToTextProperty(Display* display, char** list, int count, XICCEncodingStyle style, XTextProperty* text_prop_return)
static void x11_MbTextListToTextProperty(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    // Display* display = X11::getDisplay(thread, ARG1);
    XTextProperty property;
    if (ARG4 == 0 && ARG4 != 3) {
        kpanic("x11_MbTextListToTextProperty style = %d not handled", ARG4);
    }
    XTextProperty::create(thread, XA_STRING, ARG2, ARG3, &property);
    property.write(memory, ARG5);
}

// void XSetTextProperty(Display* display, Window w, XTextProperty* text_prop, Atom property)
static void x11_SetTextProperty(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);

    if (!w) {
        return;
    }
    XTextProperty text_prop(memory, ARG3);
    w->setTextProperty(data, thread, &text_prop, ARG4, true);
}

static void x11_SetSelectionOwner(CPU* cpu) {
    kpanic("x11_SetSelectionOwner");
}

// Window XGetSelectionOwner(Display* display, Atom selection)
static void x11_GetSelectionOwner(CPU* cpu) {
    EAX = 0;
}

// Bool XGetEvent(Display* display, XEvent* event_return, int index)
static void x11_GetEvent(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XEvent* event = data->getEvent(ARG3);
    if (event) {
        EAX = 1;
        memory->memcpy(ARG2, event, sizeof(XEvent));
    } else {
        EAX = 0;
    }
}

// Display* display, int index
static void x11_RemoveEvent(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    data->removeEvent(ARG2);    
}

// U32 XLockEvents(Display* display)
static void x11_LockEvents(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    EAX = data->lockEvents();
}

// void XUnlockEvents(Display* display)
static void x11_UnlockEvents(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    data->unlockEvents();
}

// Status XSendEvent(Display* display, Window w, Bool propagate, long event_mask, XEvent* event_send)
static void x11_SendEvent(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    U32 windowId = ARG2;
    XWindowPtr window = server->getWindow(windowId);
    U32 propagate = ARG3;
    U32 event_mask = ARG4;

    if (windowId == PointerWindow) {
        // not used in winex11
        kpanic("XSendEvent w=PointerWindow not implemented");
    } else if (windowId == InputFocus) {
        // not used in winex11
        kpanic("XSendEvent w=InputFocus not implemented");
    } else if (!window) {
        EAX = BadWindow;
        return;
    }

    XEvent event = {};
#ifdef UNALIGNED_MEMORY
    // :TODO: properly marshal
    oops
#else
    memory->memcpy(&event, ARG5, sizeof(XEvent));
#endif
    if (event.type == ClientMessage && event.xclient.message_type == _NET_WM_STATE) {
        EAX = window->handleNetWmStatePropertyEvent(data, event);
        return;
    }
    event.xany.send_event = True;    
    if (!event_mask) {
        DisplayDataPtr dst = server->getDisplayDataById(window->displayId);
        if (!dst) {
            EAX = BadValue;
            return;
        }
        event.xany.serial = dst->getNextEventSerial();
        event.xany.display = dst->displayAddress;
        dst->putEvent(event);
    } else if (!propagate) {
        server->iterateEventMask(window->id, event_mask, [&event](const DisplayDataPtr dst) {
            event.xany.serial = dst->getNextEventSerial();
            event.xany.display = dst->displayAddress;
            dst->putEvent(event);
            });
    } else {
        // not used in winex11
        kpanic("XSendEvent propagate not implemented");
    }
}

// Bool XFilterEvent(XEvent* event, Window window)
static void x11_FilterEvent(CPU* cpu) {
    EAX = False;
}

// int XLookupString(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer, KeySym* keysym_return, XComposeStatus* status_in_out) 
static void x11_LookupString(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XKeyEvent event;
    event.read(memory, ARG1);
    U32 buffer_return = ARG2;
    U32 bytes_buffer = ARG3;
    U32 keysym_return = ARG4;
    U32 status_in_out = ARG5;
    char buffer[16];

    if (status_in_out) {
        // winex11 always passes null for this
        kpanic("x11_LookupString status_in_out was not null");
    }
    U32 keysym = XKeyboard::keycodeToKeysym(event.keycode, 0);
    EAX = 0;

    if (!keysym) {
        return;
    }
    U32 result = XKeyboard::translate(keysym, event.state, buffer, 16);    
    if (result) {
        if (buffer[0] != 0) {
            memory->memcpy(buffer_return, buffer, result);
            EAX = result;
        }        
    }    
    if (keysym_return) {
        memory->writed(keysym_return, keysym);
    }
}

static void x11_MbLookupString(CPU* cpu) {
    kpanic("x11_MbLookupString");
}

// char* XKeysymToString(KeySym keysym)
static void x11_KeysymToString(CPU* cpu) {
    char buffer[16];
    U32 keysym = ARG1;
    KProcessPtr process = cpu->thread->process;
    U32 result = 0;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->keySymToNameMutex);
    if (process->keySymToName.get(keysym, result)) {
        EAX = result;
        return;
    }
    U32 len = XKeyboard::translate(keysym, 0, buffer, 16);
    if (!len || !buffer[0]) {
        EAX = 0;
        return;
    }
    result = process->alloc(cpu->thread, len + 1);
    cpu->memory->memcpy(result, buffer, len);
    cpu->memory->writeb(result + len, 0);
    process->keySymToName.set(keysym, result);
    EAX = result;
}

// int XkbTranslateKeySym(Display* dpy, KeySym* sym_return, unsigned int modifiers, char* buffer, int nbytes, int* extra_rtrn)
static void x11_KbTranslateKeysym(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 sys_return = ARG2;
    U32 modifiers = ARG3;
    U32 bufferAddress = ARG4;
    U32 nbytes = ARG5;
    U32 extra_rtrn = ARG6;
    U32 keysym = memory->readd(sys_return);
    char buffer[16];

    U32 result = XKeyboard::translate(keysym, modifiers, buffer, 16);
    if (result) {
        if (result > nbytes && extra_rtrn) {
            kpanic("x11_KbTranslateKeysym extra_rtrn not implemented");
        }
        memory->memcpy(bufferAddress, buffer, result);
        memory->writeb(bufferAddress + result, 0);
        memory->writed(sys_return, keysym);
    }
    EAX = result;
}

// KeySym XLookupKeysym(XKeyEvent* key_event, int index) {
static void x11_LookupKeysym(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XKeyEvent event;
    event.read(memory, ARG1);
    EAX = XKeyboard::keycodeToKeysym(event.keycode, ARG2);
}

// KeySym* XGetKeyboardMapping(Display* display, KeyCode first_keycode, int keycode_count, int* keysyms_per_keycode_return) 
static void x11_GetKeyboardMapping(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 displayAddress = ARG1;
    U32 max_keycode = X11_READD(Display, displayAddress, max_keycode);
    U32 first_keycode = (KeyCode)ARG2;
    U32 keycode_count = ARG3;
    U32 keysyms_per_keycode_return = ARG4;
    U32 modifiersCount = 0;
    U32 numberOfKeysPerModifier = 0;

    XKeyboard::getModifiers(modifiersCount, numberOfKeysPerModifier);

    if (first_keycode + keycode_count - 1 > max_keycode) {
        EAX = BadValue;
        return;
    }

    memory->writed(keysyms_per_keycode_return, modifiersCount);

    U32 keysyms = thread->process->alloc(thread, sizeof(U32) * keycode_count * modifiersCount);
    EAX = keysyms;

    for (U32 i = 0; i < keycode_count; i++) {
        for (U32 j = 0; j < modifiersCount; j++) {
            KeySym keysym = XKeyboard::keycodeToKeysym(first_keycode + i, j);
            memory->writed(keysyms, keysym); keysyms += 4;
        }
    }

}

// KeySym XkbKeycodeToKeysym(Display* dpy, KeyCode kc, int group, int level)
static void x11_KbKeycodeToKeysym(CPU* cpu) {
    EAX = XKeyboard::keycodeToKeysym(ARG2, ARG4);
}

// int XDisplayKeycodes(Display* display, int* min_keycodes_return, int* max_keycodes_return) {
static void x11_DisplayKeycodes(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 displayAddress = ARG1;
    U32 min_keycode = X11_READD(Display, displayAddress, min_keycode);
    U32 max_keycode = X11_READD(Display, displayAddress, max_keycode);
    memory->writed(ARG2, min_keycode);
    memory->writed(ARG3, max_keycode);
    EAX = Success;
}

// XModifierKeymap* XGetModifierMapping(Display* display)
static void x11_GetModifierMapping(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 resultAddress = thread->process->alloc(thread, sizeof(XModifierKeymap));
    U32 numberOfModifiers = 0;
    U32 numberOfKeysPerModifier = 0;
    const U16* modifiers = XKeyboard::getModifiers(numberOfModifiers, numberOfKeysPerModifier);
    U32 modifiermapAddress = thread->process->alloc(thread, sizeof(KeyCode) * 8 * numberOfKeysPerModifier);

    memory->writed(resultAddress, numberOfKeysPerModifier);
    memory->writed(resultAddress + 4, modifiermapAddress);

    for (U32 i = 0; i < 8; i++) {
        for (U32 j = 0; j < numberOfKeysPerModifier; j++) {
            if (i < numberOfModifiers) {
                memory->writeb(modifiermapAddress, (KeyCode)*modifiers);
                modifiermapAddress++;
                modifiers++;
            }
        }
    }
    EAX = resultAddress;
}

// int XFreeModifiermap(XModifierKeymap* modmap) {
static void x11_FreeModifierMap(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XModifierKeymap* result = (XModifierKeymap*)memory->getIntPtr(ARG1, true);
    thread->process->free(result->modifiermap);
    thread->process->free(ARG1);
    EAX = Success;
}

static void x11_RefreshKeyboardMapping(CPU* cpu) {
    kpanic("x11_RefreshKeyboardMapping");
}

static void x11_Bell(CPU* cpu) {
    kpanic("x11_Bell");
}

// int XGetWindowProperty(Display* display, Window w, Atom property, long long_offset, long long_length, Bool delete, Atom req_type, Atom* actual_type_return, int* actual_format_return, unsigned long* nitems_return, unsigned long* bytes_after_return, unsigned char** prop_return)
//
// https://tronche.com/gui/x/xlib/window-information/XGetWindowProperty.html
//
// long_offset 	Specifies the offset in the specified property (in 32-bit quantities) where the data is to be retrieved.
// long_length 	Specifies the length in 32 - bit multiples of the data to be retrieved.
// delete 	Specifies a Boolean value that determines whether the property is deleted.
// req_type 	Specifies the atom identifier associated with the property type or AnyPropertyType.
// actual_type_return 	Returns the atom identifier that defines the actual type of the property.
// actual_format_return 	Returns the actual format of the property.
// nitems_return 	Returns the actual number of 8 - bit, 16 - bit, or 32 - bit items stored in the prop_return data.
// bytes_after_return 	Returns the number of bytes remaining to be read in the property if a partial read was performed.
// prop_return 	Returns the data in the specified format.
// 
// If the specified property does not exist for the specified window, XGetWindowProperty() returns None to actual_type_return and the value zero to actual_format_return and bytes_after_return. The nitems_return argument is empty. In this case, the delete argument is ignored. 
//
// If the specified property exists but its type does not match the specified type, XGetWindowProperty() returns the actual property type to actual_type_return, the actual property format (never zero) to actual_format_return, and the property length in bytes (even if the actual_format_return is 16 or 32) to bytes_after_return. It also ignores the delete argument. The nitems_return argument is empty. 
//
// If delete is True and bytes_after_return is zero, XGetWindowProperty() deletes the property from the window and generates a PropertyNotify event on the window. 
//
// BadAtom 	A value for an Atom argument does not name a defined Atom.
// BadValue 	Some numeric value falls outside the range of values accepted by the request.Unless a specific range is specified for an argument, the full range defined by the argument's type is accepted. Any argument defined as a set of alternatives can generate this error.
// BadWindow 	A value for a Window argument does not name a defined Window.
static void x11_GetWindowProperty(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    // Display* display = X11::getDisplay(thread, ARG1);
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr window = server->getWindow(ARG2);    
    U32 propertyAtom = ARG3;
    U32 long_offset = ARG4 * 4;
    U32 long_length = ARG5 * 4;
    U32 deleteProperty = ARG6;
    U32 req_type = ARG7;
    U32 actual_type_return = ARG8;
    U32 actual_format_return = ARG9;
    U32 nitems_return = ARG10;
    U32 bytes_after_return = ARG11;
    U32 prop_return = ARG12;

    memory->writed(actual_type_return, 0);
    memory->writed(actual_format_return, 0);
    memory->writed(nitems_return, 0);
    memory->writed(bytes_after_return, 0);
    memory->writed(prop_return, 0);

    if (!window) {
        EAX = BadWindow;
        return;
    }

    BString propertyName;
    if (!server->getAtom(propertyAtom, propertyName)) {
        EAX = BadAtom;
        return;
    }
    BString propertyType;
    if (!server->getAtom(req_type, propertyType)) {
        EAX = BadAtom;
        return;
    }
    EAX = Success;

    XPropertyPtr property = window->getProperty(propertyAtom);
    if (!property) {
        return;
    }
    if (long_offset > property->length) {
        EAX = BadValue;
        return;
    }

    memory->writed(actual_type_return, property->type);
    memory->writed(actual_format_return, property->format);    

    U32 toCopy = std::min(long_length, property->length - long_offset);
    if (toCopy == 0) {
        // XGetWindowProperty() always allocates one extra byte in prop_return (even if the property is zero length) and sets it to zero so that simple properties consisting of characters do not have to be copied into yet another string before use. 
        U32 valueAddress = thread->process->alloc(thread, 1); // alloc guarantees zero'd out memory
        memory->writed(prop_return, valueAddress);
        return;
    }
    U32 valueAddress = thread->process->alloc(thread, toCopy + 1);
    memory->memcpy(valueAddress, property->value + long_offset, toCopy);
    memory->writed(prop_return, valueAddress);

    if (req_type == AnyPropertyType || req_type == property->type) {
        memory->writed(nitems_return, toCopy * 8 / property->format);
        memory->writed(bytes_after_return, property->length - long_offset - toCopy);
        if (deleteProperty) {
            window->deleteProperty(data, propertyAtom);
        }
    } else {
        memory->writed(bytes_after_return, property->length);
    }
}

// XChangeProperty(Display* display, Window w, Atom property, Atom type, int format, int mode, _Xconst unsigned char* data, int nelements)
static void x11_ChangeProperty(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr window = server->getWindow(ARG2);
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);

    U32 propertyAtom = ARG3;
    U32 typeAtom = ARG4;
    U32 format = ARG5;
    U32 mode = ARG6;
    U32 nelements = ARG8;

    if (!window) {
        EAX = BadWindow;
        return;
    }

    BString propertyName;
    if (!server->getAtom(propertyAtom, propertyName)) {
        EAX = BadAtom;
        return;
    }
    BString propertyType;
    if (!server->getAtom(typeAtom, propertyType)) {
        EAX = BadAtom;
        return;
    }
    EAX = Success;

    if (mode == PropModeReplace) {
        window->setProperty(data, propertyAtom, typeAtom, format, nelements * format / 8, ARG7, true);        
    } else {
        kpanic("x11_ChangeProperty mode=%d", mode);
    }
}

// int XDeleteProperty(Display* display, Window w, Atom property)
static void x11_DeleteProperty(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr window = server->getWindow(ARG2);
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    U32 propertyAtom = ARG3;

    if (!window) {
        EAX = BadWindow;
        return;
    }

    BString propertyName;
    if (!server->getAtom(propertyAtom, propertyName)) {
        EAX = BadAtom;
        return;
    }
    if (server->trace) {
        XPropertyPtr prop = window->getProperty(propertyAtom);
        if (prop) {
            BString log;
            DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(cpu->memory, ARG1);

            log.append(data->displayId, 16);
            log += " DeleteProperty window=";
            log.append(window->id, 16);
            log += " property=";
            log.append(prop->atom, 16);
            log += "(";
            BString name;
            server->getAtom(prop->atom, name);
            log += ")";
            klog(log.c_str());
        }
    }
    window->deleteProperty(data, propertyAtom);
    EAX = Success;
}

static void x11_ConvertSelection(CPU* cpu) {
    kpanic("x11_ConvertSelection");
}

static void x11_CheckTypedWindowEvent(CPU* cpu) {
    kpanic("x11_CheckTypedWindowEvent");
}

static void x11_GetGeometry(CPU* cpu) {
    kpanic("x11_GetGeometry");
}

// Status XInternAtoms(Display* dpy, char** names, int count, Bool onlyIfExists, Atom* atoms_return)
static void x11_InternAtoms(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    // Display* display = X11::getDisplay(thread, ARG1);
    U32 names = ARG2;
    U32 count = ARG3;
    bool onlyIfExists = ARG4 != 0;
    U32 atoms_return = ARG5;

    for (U32 i = 0; i < count; i++) {
        U32 nameAddress = memory->readd(names); names += 4;
        BString name = memory->readString(nameAddress);
        bool exists = false;
        U32 atom = server->internAtom(name, onlyIfExists);
        memory->writed(atoms_return, atom); atoms_return += 4;
    }
    EAX = Success;
}

// char* XGetAtomName(Display* display, Atom atom)
static void x11_GetAtomName(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    U32 atom = ARG2;
    BString name;

    if (!server->getAtom(atom, name)) {
        EAX = 0;
    } else {
        EAX = thread->process->createString(thread, name);        
    }
}

// Status XGetAtomNames(Display* dpy, Atom* atoms, int count, char** names_return)
static void x11_GetAtomNames(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = thread->memory;
    // Display* display = X11::getDisplay(thread, ARG1);
    U32 atoms = ARG2;
    U32 count = ARG3;
    U32 names_return = ARG4;

    for (U32 i = 0; i < count; i++) {
        U32 atom = memory->readd(atoms);
        U32 result = 0;
        BString name;

        if (server->getAtom(atom, name)) {            
            result = thread->process->createString(thread, name);
        }
        memory->writed(names_return, result);

        names_return += 4;
        atoms += 4;
    }
    EAX = Success;
}

// Colormap XCreateColormap(Display* display, Window w, Visual* visual, int alloc)
static void x11_CreateColorMap(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = thread->memory;
    Visual visual;
    U32 visualAddress = ARG3;

    if (visualAddress) {
        visual.read(memory, visualAddress);
    }

    if (ARG4 == AllocNone && ((visualAddress && visual.c_class == TrueColor) || !visualAddress)) {
        EAX = DummyAtom;
    } else {
        EAX = None;
    }
}

static void x11_FreeColorMap(CPU* cpu) {
    kpanic("x11_FreeColorMap");
}

static void x11_FreeColors(CPU* cpu) {
    kpanic("x11_FreeColors");
}

static void x11_QueryColors(CPU* cpu) {
    kpanic("x11_QueryColors");
}

static void x11_AllocColor(CPU* cpu) {
    kpanic("x11_AllocColor");
}

static void x11_AllocColorCells(CPU* cpu) {
    kpanic("x11_AllocColorCells");
}

// XVisualInfo* XGetVisualInfo(Display* display, long vinfo_mask, XVisualInfo* vinfo_template, int* nitems_return)
static void x11_GetVisualInfo(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 displayAddress = ARG1;
    U32 mask = ARG2;
    XVisualInfo infoTemplate;
    infoTemplate.read(memory, ARG3);

    S32 count = 0;

    Display::iterateVisuals(thread, displayAddress, [mask, &count, &infoTemplate](S32 screenIndex, U32 visualAddress, Depth* depth, Visual* visual) {
        if (infoTemplate.match(mask, screenIndex, depth, visual)) {
            count++;
        }
        return true;
        });
    memory->writed(ARG4, count);
    if (!count) {
        EAX = 0;
        return;
    }
    U32 listAddress = thread->process->alloc(thread, (sizeof(XVisualInfo) + sizeof(U32)) * count);
    EAX = listAddress;
    U32 itemAddress = listAddress + sizeof(U32) * count;
    Display::iterateVisuals(thread, displayAddress, [&memory, &listAddress, &itemAddress, mask, &count, &infoTemplate](S32 screenIndex, U32 visualAddress, Depth* depth, Visual* visual) {
        if (infoTemplate.match(mask, screenIndex, depth, visual)) {
            XVisualInfo* visualInfo = (XVisualInfo*)memory->getIntPtr(itemAddress, true);
            memory->writed(listAddress, itemAddress);
            itemAddress += sizeof(XVisualInfo);
            listAddress += sizeof(U32);
            visualInfo->set(screenIndex, visualAddress, depth, visual);
        }
        return true;
        });
}

// XPixmapFormatValues* XListPixmapFormats(Display* display, int* count_return)
static void x11_ListPixelFormats(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 displayAddress = ARG1;
    U32 defaultScreen = X11_READD(Display, displayAddress, default_screen);
    U32 screenAddress = X11_READD(Display, displayAddress, screens) + sizeof(Screen) * defaultScreen;
    U32 ndepths = X11_READD(Screen, screenAddress, ndepths);
    U32 count_return = ARG2;

    if (count_return) {
        memory->writed(count_return, ndepths);
    }
    U32 listAddress = thread->process->alloc(thread, sizeof(XPixmapFormatValues) * ndepths);
    EAX = listAddress;

    U32 depthsAddress = X11_READD(Screen, screenAddress, depths);
    for (U32 i = 0; i < ndepths; i++) {
        U32 depthAddress = depthsAddress + i * sizeof(Depth);
        Depth depth;
        depth.read(memory, depthAddress);
        U32 bits_per_rgb = X11_READD(Visual, depth.visuals, bits_per_rgb);
        listAddress += XPixmapFormatValues::write(memory, listAddress, depth.depth, bits_per_rgb, 32);
    }
}

// void XLockDisplay(Display* display)
static void x11_LockDisplay(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    BOXEDWINE_MUTEX_LOCK(data->mutex);
}

// void XUnlockDisplay(Display* display)
static void x11_UnlockDisplay(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    BOXEDWINE_MUTEX_UNLOCK(data->mutex);
}

static void x11_CopyArea(CPU* cpu) {
    kpanic("x11_CopyArea");
}

static void x11_GetImage(CPU* cpu) {
    kpanic("x11_GetImage");
}

// int XPutImage(Display* display, Drawable d, GC gc, XImage* image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height)
static void x11_PutImage(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    // Display* display = X11::getDisplay(thread, ARG1);
    XDrawablePtr d = server->getDrawable(ARG2);
    if (!d) {
        EAX = BadDrawable;
        return;
    }
    XGCPtr gc = server->getGC(ARG3);    
    if (!gc) {
        EAX = BadGC;
        return;
    }
    XImage image;
    XImage::read(memory, ARG4, &image);
    U32 src_x = ARG5;
    U32 src_y = ARG6;
    U32 dest_x = ARG7;
    U32 dest_y = ARG8;
    U32 width = ARG9;
    U32 height = ARG10;
    EAX = d->putImage(thread, gc, &image, src_x, src_y, dest_x, dest_y, width, height);    
}

// int XFlush(Display* display)
static void x11_Flush(CPU* cpu) {
    XServer* server = XServer::getServer();
    server->draw(true);
    EAX = Success;
}

// int XDestroyImage(XImage* ximage)
static void x11_DestroyImage(CPU* cpu) {
    KThread* thread = cpu->thread;
    thread->process->free(ARG1);
    EAX = Success;
}

static void x11_GetPixel(CPU* cpu) {
    kpanic("x11_GetPixel");
}

static void x11_PutPixel(CPU* cpu) {
    kpanic("x11_PutPixel");
}

static void x11_SubImage(CPU* cpu) {
    kpanic("x11_SubImage");
}

static void x11_AddPixel(CPU* cpu) {
    kpanic("x11_AddPixel");
}

// Pixmap XCreatePixmap(Display* display, Drawable d, unsigned int width, unsigned int height, unsigned int depth)
static void x11_CreatePixmap(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    XWindowPtr window = server->getWindow(ARG2); // even though it's a Drawable passed in, spec says it must be an InputOnly window
    U32 width = ARG3;
    U32 height = ARG4;
    U32 depth = ARG5;

    std::shared_ptr<XPixmap> pixmap = server->createNewPixmap(ARG3, ARG4, ARG5);
    EAX = pixmap->id;
}

// Pixmap XCreateBitmapFromData(Display* display, Drawable d, const char* data, unsigned int width, unsigned int height)
static void x11_CreateBitmapFromData(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XWindowPtr window = server->getWindow(ARG2); // even though it's a Drawable passed in, spec says it must be an InputOnly window
    U32 data = ARG3;
    U32 width = ARG4;
    U32 height = ARG5;

    std::shared_ptr<XPixmap> pixmap = server->createNewPixmap(width, height, window->getDepth());
    pixmap->copyImageData(thread, nullptr, data, pixmap->getBytesPerLine(), pixmap->getBitsPerPixel() , 0, 0, 0, 0, width, height);
    EAX = pixmap->id;
}

// int XFreePixmap(Display* display, Pixmap pixmap)
static void x11_FreePixmap(CPU* cpu) {
    XServer* server = XServer::getServer();
    EAX = server->removePixmap(ARG2);
}

// Cursor XCreatePixmapCursor(Display* display, Pixmap source, Pixmap mask, XColor* foreground_color, XColor* background_color, unsigned int x, unsigned int y)
static void x11_CreatePixmapCursor(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XColor fg;
    XColor bg;
    fg.read(memory, ARG4);
    bg.read(memory, ARG5);
    XCursorPtr cursor = std::make_shared<XCursor>(ARG2, ARG3, fg, bg, ARG6, ARG7);
    server->addCursor(cursor);
    EAX = cursor->id;
}

// Cursor XCreateFontCursor(Display* display, unsigned int shape)
static void x11_CreateFontCursor(CPU* cpu) {
    EAX = ARG2;
}

// int XDefineCursor(Display* display, Window w, Cursor cursor)
static void x11_DefineCursor(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XWindowPtr window = server->getWindow(ARG2);
    if (!window) {
        EAX = BadWindow;
        return;
    }
    window->cursor = server->getCursor(ARG3);
    EAX = Success;
}

static void x11_FreeCursor(CPU* cpu) {
    kpanic("x11_FreeCursor");
}

// int XSetFunction(Display* display, GC gc, int function)
static void x11_SetFunction(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    gc->values.function = ARG3;
    EAX = Success;
}

// int XSetBackground(Display* display, GC gc, unsigned long background)
static void x11_SetBackground(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    gc->values.background = ARG3;
    EAX = Success;
}

// int XSetForeground(Display* display, GC gc, unsigned long foreground)
static void x11_SetForeground(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    gc->values.foreground = ARG3;
    EAX = Success;
}

static void x11_CopyPlane(CPU* cpu) {
    kpanic("x11_CopyPlane");
}

// GC XCreateGC(Display* display, Drawable d, unsigned long valuemask, XGCValues* values) {
static void x11_CreateGC(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    XDrawablePtr drawable = server->getDrawable(ARG2);
    U32 valuemask = ARG3;
    if (valuemask) {
        kpanic("x11_CreateGC valuemask not implemented"); // winex11 doesn't use this
    }
    if (!drawable) {
        EAX = BadDrawable;
        return;
    }
    XGCPtr gc = server->createGC(drawable);
    if (server->traceGC) {
        BString log;
        DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(cpu->memory, ARG1);

        log.append(data->displayId, 16);
        log += " CreateGC id=";
        log.append(gc->id, 16);
        log += " drawable=";
        log.append(ARG2, 16);
        klog(log.c_str());
    }
    EAX = gc->id;
}

static void x11_SetDashes(CPU* cpu) {
    kpanic("x11_SetDashes");
}

// int XDrawLine(Display* display, Drawable d, GC gc, int x1, int y1, int x2, int y2)
static void x11_DrawLine(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XDrawablePtr drawable = server->getDrawable(ARG2);
    if (!drawable) {
        EAX = BadDrawable;
        return;
    }
    XGCPtr gc = server->getGC(ARG3);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    if (server->traceGC) {
        BString log;
        DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(cpu->memory, ARG1);

        log.append(data->displayId, 16);
        log += " DrawLine drawable=";
        log.append(ARG2, 16);
        log += " gc=";
        log.append(ARG3, 16);
        log += " x1=";
        log += ARG4;
        log += " y1=";
        log += ARG5;
        log += " x2=";
        log += ARG6;
        log += " y2=";
        log += ARG7;
        klog(log.c_str());
    }
    EAX = drawable->drawLine(thread, gc, ARG4, ARG5, ARG6, ARG7);
}

static void x11_DrawLines(CPU* cpu) {
    //kpanic("x11_DrawLines");
}

static void x11_SetArcMode(CPU* cpu) {
    kpanic("x11_SetArcMode");
}

static void x11_FillArc(CPU* cpu) {
    kpanic("x11_FillArc");
}

static void x11_DrawArc(CPU* cpu) {
    kpanic("x11_DrawArc");
}

static void x11_DrawRectangle(CPU* cpu) {
    kpanic("x11_DrawRectangle");
}

// int XFillRectangle(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height)
static void x11_FillRectangle(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    XDrawablePtr drawable = server->getDrawable(ARG2);
    if (!drawable) {
        EAX = BadDrawable;
        return;
    }
    XGCPtr gc = server->getGC(ARG3);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    if (server->traceGC) {
        BString log;
        DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(cpu->memory, ARG1);

        log.append(data->displayId, 16);
        log += " FillRectangle drawable=";
        log.append(ARG2, 16);
        log += " gc=";
        log.append(ARG3, 16);
        log += " x=";
        log += ARG4;
        log += " y=";
        log += ARG5;
        log += " width=";
        log += ARG6;
        log += " height=";
        log += ARG7;
        klog(log.c_str());
    }
    EAX = drawable->fillRectangle(thread, gc, ARG4, ARG5, ARG6, ARG7);
}

static void x11_FillRectangles(CPU* cpu) {
    kpanic("x11_FillRectangles");
}

static void x11_DrawPoint(CPU* cpu) {
    kpanic("x11_DrawPoint");
}

static void x11_FillPolygon(CPU* cpu) {
    kpanic("x11_FillPolygon");
}

// int XChangeGC(Display* display, GC gc, unsigned long valuemask, XGCValues* values)
static void x11_ChangeGC(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    // Display* display = X11::getDisplay(thread, ARG1);
    U32 mask = ARG3;
    XGCValues values;
    values.read(memory, ARG4);

    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }

    if (server->traceGC) {
        BString log;
        DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(cpu->memory, ARG1);

        log.append(data->displayId, 16);
        log += " ChangeGC gc=";
        log.append(ARG2, 16);
        if (mask & GCFunction) {
            log += " function=";
            log.append(values.function, 16);
        }
        if (mask & GCPlaneMask) {
            log += " plane_mask=";
            log.append(values.plane_mask, 16);
        }
        if (mask & GCForeground) {
            log += " foreground=";
            log.append(values.foreground, 16);
        }
        if (mask & GCBackground) {
            log += " background=";
            log.append(values.background, 16);
        }
        if (mask & GCLineWidth) {
            log += " line_width=";
            log.append(values.line_width, 16);
        }
        if (mask & GCLineStyle) {
            log += " line_style=";
            log.append(values.line_style, 16);
        }
        if (mask & GCCapStyle) {
            log += " cap_style=";
            log.append(values.cap_style, 16);
        }
        if (mask & GCJoinStyle) {
            log += " join_style=";
            log.append(values.join_style, 16);
        }
        if (mask & GCFillStyle) {
            log += " fill_style=";
            log.append(values.fill_style, 16);
        }
        if (mask & GCFillRule) {
            log += " fill_rule=";
            log.append(values.fill_rule, 16);
        }
        if (mask & GCTile) {
            log += " tile=";
            log.append(values.tile, 16);
        }
        if (mask & GCStipple) {
            log += " stipple=";
            log.append(values.stipple, 16);
        }
        if (mask & GCTileStipXOrigin) {
            log += " ts_x_origin=";
            log += values.ts_x_origin;
        }
        if (mask & GCTileStipYOrigin) {
            log += " ts_y_origin=";
            log += values.ts_y_origin;
        }
        if (mask & GCFont) {
            log += " font=";
            log.append(values.font, 16);
        }
        if (mask & GCSubwindowMode) {
            log += " subwindow_mode=";
            log.append(values.subwindow_mode, 16);
        }
        if (mask & GCGraphicsExposures) {
            log += " graphics_exposures=";
            log.append(values.graphics_exposures, 16);
        }
        if (mask & GCClipXOrigin) {
            log += " clip_x_origin=";
            log += values.clip_x_origin;
        }
        if (mask & GCClipYOrigin) {
            log += " clip_y_origin=";
            log += values.clip_y_origin;
        }
        if (mask & GCClipMask) {
            log += " clip_mask=";
            log.append(values.clip_mask, 16);
        }
        if (mask & GCDashOffset) {
            log += " dash_offset=";
            log += values.dash_offset;
        }
        if (mask & GCDashList) {
            log += " dashes=";
            log.append(values.dashes, 16);
        }
        if (mask & GCArcMode) {
            log += " arc_mode=";
            log.append(values.arc_mode, 16);
        }
    }
    gc->updateValues(mask, &values);
    EAX = Success;
}

// int XFreeGC(Display* display, GC gc)
static void x11_FreeGC(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    server->removeGC(ARG2);
    EAX = Success;
}

// int XSetSubwindowMode(Display* display, GC gc, int subwindow_mode) {
static void x11_SetSubwindowMode(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    gc->values.subwindow_mode = ARG3;
    EAX = Success;
}

// int XSetGraphicsExposures(Display* display, GC gc, Bool graphics_exposures)
static void x11_SetGraphicsExposures(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    gc->values.graphics_exposures = ARG3?true:false;
    EAX = Success;
}

static void x11_SetFillStyle(CPU* cpu) {
    kpanic("x11_SetFillStyle");
}

// int XFree(void* data)
//
// It seems like this should return something, but I'm not sure, it's never check in winex11 so for now I'm not setting EAX
static void x11_Free(CPU* cpu) {
    U32 address = ARG1;
    if (!address) {
        return;
    }
    cpu->thread->process->free(ARG1);
}

// int XSetClipMask(Display* display, GC gc, Pixmap pixmap)
static void x11_SetClipMask(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    // Display* display = X11::getDisplay(thread, ARG1);
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    gc->values.clip_mask = ARG3;
    EAX = Success;
}

// int XSetClipRectangles(Display* display, GC gc, int clip_x_origin, int clip_y_origin, XRectangle* rectangles, int n, int ordering) {
static void x11_SetClipRectangles(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    // Display* display = X11::getDisplay(thread, ARG1);
    XGCPtr gc = server->getGC(ARG2);
    if (!gc) {
        EAX = BadGC;
        return;
    }
    gc->values.clip_x_origin = (S32)ARG3;
    gc->values.clip_y_origin = (S32)ARG4;
    gc->clip_rects.clear();

    U32 count = ARG6;
    U32 rectAddress = ARG5;
    for (U32 i = 0; i < count; i++) {
        XRectangle r(memory, rectAddress);
        gc->clip_rects.push_back(r);
        rectAddress += sizeof(XRectangle);
    }
    EAX = Success;
}

// int XSetTransientForHint(Display* display, Window w, Window prop_window)
static void x11_SetTransientForHint(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    XWindowPtr w = server->getWindow(ARG2);
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);

    if (!w) {
        EAX = BadWindow;
        return;
    }
    U32 prop_window = ARG3;
    w->setProperty(data, XA_WM_TRANSIENT_FOR, XA_WINDOW, 32, 4, (U8*)&prop_window, true);
    EAX = Success;
}

// int XSetWMHints(Display* display, Window w, XWMHints* wm_hints)
static void x11_SetWMHints(CPU* cpu) {
    KMemory* memory = cpu->memory;
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);

    if (!w) {
        EAX = BadWindow;
        return;
    }
    U32 value = thread->process->alloc(thread, sizeof(XWMHints));
    memory->memcpy(value, ARG3, sizeof(XWMHints));
    w->setProperty(data, XA_WM_HINTS, XA_CARDINAL, 32, sizeof(XWMHints), value, true);
    EAX = Success;
}

// XWMHints* XAllocWMHints()
static void x11_AllocWMHints(CPU* cpu) {
    KThread* thread = KThread::currentThread();
    EAX = thread->process->alloc(thread, sizeof(XWMHints));
}

// XClassHint* XAllocClassHint()
static void x11_AllocClassHint(CPU* cpu) {
    KThread* thread = KThread::currentThread();
    EAX = thread->process->alloc(thread, 8);
}

// int XSetClassHint(Display* display, Window w, XClassHint* class_hints)
static void x11_SetClassHint(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);
    if (!w) {
        EAX = BadWindow;
        return;
    }
    XTextProperty prop;
    XTextProperty::create(thread, XA_STRING, ARG3, 2, &prop);    

    w->setTextProperty(data, thread, &prop, XA_WM_CLASS, true);

    EAX = Success;
}

// void XSetWMName(Display* display, Window w, XTextProperty* text_prop)
static void x11_SetWMName(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);
    if (!w) {
        return;
    }
    XTextProperty window_name(memory, ARG3);
    w->setTextProperty(data, thread, &window_name, XA_WM_NAME, true);
}

// void XSetWMIconName(Display* display, Window w, XTextProperty* text_prop)
static void x11_SetWMIconName(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);

    if (!w) {
        return;
    }
    XTextProperty window_name(memory, ARG3);
    w->setTextProperty(data, thread, &window_name, XA_WM_ICON_NAME, true);
}

// void XSetWMNormalHints(Display* display, Window w, XSizeHints* hints)
static void x11_SetWMNormalHints(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);

    if (!w) {
        EAX = BadWindow;
        return;
    }
    U32 value = thread->process->alloc(thread, sizeof(XSizeHints));
    memory->memcpy(value, ARG3, sizeof(XSizeHints));
    w->setProperty(data, XA_WM_NORMAL_HINTS, XA_CARDINAL, 32, sizeof(XSizeHints), value, true);
    EAX = Success;
}

// XSizeHints* XAllocSizeHints()
static void x11_AllocSizeHints(CPU* cpu) {
    KThread* thread = KThread::currentThread();
    EAX = thread->process->alloc(thread, sizeof(XSizeHints));
}

// void XSetWMProperties(Display* display, Window w, XTextProperty* window_name, XTextProperty* icon_name, char** argv, int argc, XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
static void x11_SetWMProperties(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    KMemory* memory = cpu->memory;
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);

    if (!w) {
        EAX = BadWindow;
        return;
    }
    if (ARG3) {
        XTextProperty window_name(memory, ARG3);
        w->setTextProperty(data, thread, &window_name, server->internAtom(B("WM_NAME"), false), true);
    }
    if (ARG4) {
        XTextProperty window_name(memory, ARG4);
        w->setTextProperty(data, thread, &window_name, server->internAtom(B("WM_ICON_NAME"), false), true);
    }
    if (ARG7) {
        U32 value = thread->process->alloc(thread, sizeof(XSizeHints));
        memory->memcpy(value, ARG7, sizeof(XSizeHints));
        w->setProperty(data, XA_WM_NORMAL_HINTS, XA_CARDINAL, 32, sizeof(XSizeHints), value, true);
    }
    if (ARG8) {
        U32 value = thread->process->alloc(thread, sizeof(XWMHints));
        memory->memcpy(value, ARG8, sizeof(XWMHints));
        w->setProperty(data, XA_WM_HINTS, XA_CARDINAL, 32, sizeof(XWMHints), value, true);
    }
    if (ARG9) {
        XTextProperty prop;
        XTextProperty::create(thread, XA_STRING, ARG9, 2, &prop);
        w->setTextProperty(data, thread, &prop, server->internAtom(B("WM_CLASS"), false), true);
    }
    w->setProperty(data, XA_WM_CLIENT_MACHINE, XA_STRING, 8, 6, (U8*)"debian", true);
    w->setProperty(data, server->internAtom(B("WM_LOCALE_NAME"), false), XA_STRING, 8, 11, (U8*)"en_US.UTF-8", true);

    if (ARG5 || ARG6) {
        kpanic("x11_SetWMProperties not implemented");
    }
    EAX = Success;
}

static void x11_VaCreateNestedList(CPU* cpu) {
    kpanic("x11_VaCreateNestedList");
}

static void x11_UnsetICFocus(CPU* cpu) {
    kpanic("x11_UnsetICFocus");
}

static void x11_SetICFocus(CPU* cpu) {
    kpanic("x11_SetICFocus");
}

static void x11_DestroyIC(CPU* cpu) {
    kpanic("x11_DestroyIC");
}

static void x11_SetICValues(CPU* cpu) {
    kpanic("x11_SetICValues");
}

static void x11_MbResetIC(CPU* cpu) {
    kpanic("x11_MbResetIC");
}

// char* XSetLocaleModifiers(const char* modifier_list)
static void x11_SetLocaleModifiers(CPU* cpu) {
    // :TODO:
    EAX = 1;
}

// XIM XOpenIM(Display* dpy, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class) {
static void x11_OpenIM(CPU* cpu) {
    EAX = 0;
}

static void x11_CloseIM(CPU* cpu) {
    kpanic("x11_CloseIM");
}

// char* XSetIMValues(XIM im, ...) 
static void x11_SetIMValues(CPU* cpu) {
    kpanic("x11_SetIMValues");
}

static void x11_GetIMValues(CPU* cpu) {
    kpanic("x11_GetIMValues");
}

static void x11_DisplayOfIM(CPU* cpu) {
    kpanic("x11_DisplayOfIM");
}

static void x11_UnregisterIMInstantiateCallback(CPU* cpu) {
    kpanic("x11_UnregisterIMInstantiateCallback");
}

// Bool XRegisterIMInstantiateCallback(Display* dpy, struct _XrmHashBucketRec* rdb, char* res_name, char* res_class, XIDProc callback, XPointer client_data)
static void x11_RegisterIMInstantiateCallback(CPU* cpu) {
    EAX = False;
}

static void x11_FreeStringList(CPU* cpu) {
    kpanic("x11_FreeStringList");
}

// XFontSet XCreateFontSet(Display* display, _Xconst char* base_font_name_list, char*** missing_charset_list, int* missing_charset_count, char** def_string) {
static void x11_CreateFontSet(CPU* cpu) {
    KMemory* memory = cpu->memory;
    if (ARG3) {
        memory->writed(ARG3, 0);
    }
    if (ARG4) {
        memory->writed(ARG4, 0);
    }
    EAX = 1;
}

static void x11_FreeFontSet(CPU* cpu) {
    kpanic("x11_FreeFontSet");
}

static void x11_CreateIC(CPU* cpu) {
    kpanic("x11_CreateIC");
}

static U32 roundUp(U32 numToRound, U32 multiple)
{
    if (!multiple) {
        return numToRound;
    }
    return ((numToRound + multiple - 1) / multiple) * multiple;
}

// XImage* XCreateImage(Display* display, Visual* visual, unsigned int depth, int format, int offset, char* data, unsigned int width, unsigned int height, int bitmap_pad, int bytes_per_line)
static void x11_CreateImage(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    Visual visual = {};
    if (ARG2) {
        visual.read(memory, ARG2);
    }
    U32 depth = ARG3;
    U32 format = ARG4; // winex11: will always be ZPixmap
    U32 offset = ARG5;
    U32 data = ARG6;
    U32 width = ARG7;
    U32 height = ARG8;
    U32 bitmap_pad = ARG9;
    U32 bytes_per_line = ARG10;
    U32 bitmap_unit = 32;
    U32 bits_per_pixel = depth == 24 ? 32 : depth;

    if (depth == 8) {
        bitmap_unit = 8;
    } else if (depth == 16) {
        bitmap_unit = 16;
    }
    if (bytes_per_line == 0) {
        bytes_per_line = roundUp((bits_per_pixel * width / 8), bitmap_pad);
    }
    U32 image = thread->process->alloc(thread, sizeof(XImage));
    XImage::set(memory, image, width, height, offset, format, data, bitmap_pad, depth, bytes_per_line, bits_per_pixel, visual.red_mask, visual.green_mask, visual.blue_mask);
    EAX = image;
}

static void x11_DisplayName(CPU* cpu) {
    kpanic("x11_DisplayName");
}

static void x11_GetDefault(CPU* cpu) {
    kpanic("x11_GetDefault");
}

static void x11_GetWindowAttributes(CPU* cpu) {
    kpanic("x11_GetWindowAttributes");
}

static void x11_IconifyWindow(CPU* cpu) {
    kpanic("x11_IconifyWindow");
}

static void x11_InternAtom(CPU* cpu) {
    kpanic("x11_InternAtom");
}

static void x11_KeysymToKeycode(CPU* cpu) {
    kpanic("x11_KeysymToKeycode");
}

static void x11_LocaleOfIM(CPU* cpu) {
    kpanic("x11_LocaleOfIM");
}

static void x11_MatchVisualInfo(CPU* cpu) {
    kpanic("x11_MatchVisualInfo");
}

static void x11_QueryColor(CPU* cpu) {
    kpanic("x11_QueryColor");
}

static void x11_QueryExtension(CPU* cpu) {
    kpanic("x11_QueryExtension");
}

static void x11_ShapeCombineMask(CPU* cpu) {
    kpanic("x11_ShapeCombineMask");
}

static void x11_ShapeCombineRectangles(CPU* cpu) {
    kpanic("x11_ShapeCombineRectangles");
}

static void x11_ShapeOffsetShape(CPU* cpu) {
    kpanic("x11_ShapeOffsetShape");
}

static void x11_ShmAttach(CPU* cpu) {
    kpanic("x11_ShmAttach");
}

// XImage* XShmCreateImage(Display* dpy, Visual* visual, unsigned int depth, int format, char* data, XShmSegmentInfo* shminfo, unsigned int width, unsigned int height)
static void x11_ShmCreateImage(CPU* cpu) {
    kpanic("x11_ShmCreateImage");
}

static void x11_ShmDetach(CPU* cpu) {
    kpanic("x11_ShmDetach");
}

static void x11_ShmPutImage(CPU* cpu) {
    kpanic("x11_ShmPutImage");
}

static void x11_StoreColor(CPU* cpu) {
    kpanic("x11_StoreColor");
}

static void x11_WindowEvent(CPU* cpu) {
    kpanic("x11_WindowEvent");
}

// Status XWithdrawWindow(Display* display, Window w, int screen_number)
static void x11_WithDrawWindow(CPU* cpu) {
    KMemory* memory = cpu->memory;
    XServer* server = XServer::getServer();
    DisplayDataPtr data = server->getDisplayDataByAddressOfDisplay(memory, ARG1);
    XWindowPtr w = server->getWindow(ARG2);

    if (!w) {
        EAX = BadWindow;
        return;
    }
    EAX = w->unmapWindow(data);
}

static void x11_MbTextPropertyToTextList(CPU* cpu) {
    kpanic("x11_MbTextPropertyToTextList");
}

// XrmQuark XrmUniqueQuark()
static void x11_RmUniqueQuark(CPU* cpu) {
    XServer* server = XServer::getServer();
    EAX = server->getNextQuark();
}

struct XineramaScreenInfo {
    S32   screen_number;
    S16 x_org;
    S16 y_org;
    S16 width;
    S16 height;
};

static void x11_XineramaQueryScreens(CPU* cpu) {
    KThread* thread = cpu->thread;
    KMemory* memory = cpu->memory;
    U32 resultAddress = thread->process->alloc(thread, sizeof(XineramaScreenInfo));
    XineramaScreenInfo* info = (XineramaScreenInfo*)memory->getIntPtr(resultAddress, true);
    info->screen_number = 1;
    info->x_org = 0;
    info->y_org = 0;
    info->width = KNativeWindow::defaultScreenWidth;
    info->height = KNativeWindow::defaultScreenHeight;
    EAX = resultAddress;
}

// SizeID XRRConfigCurrentConfiguration(XRRScreenConfiguration* config, Rotation* rotation)
static void x11_XRRConfigCurrentConfiguration(CPU* cpu) {
    KThread* thread = cpu->thread;
    XServer* server = XServer::getServer();
    EAX = XrrConfigCurrentConfiguration(thread, ARG1, ARG2);
}

// short XRRConfigCurrentRate(XRRScreenConfiguration* config)
static void x11_XRRConfigCurrentRate(CPU* cpu) {
    EAX = XrrConfigCurrentRate();
}

// void XRRFreeScreenConfigInfo(XRRScreenConfiguration* config)
static void x11_XRRFreeScreenConfigInfo(CPU * cpu) {
    // config is hard code constants, not a real pointer
}

// XRRScreenConfiguration* XRRGetScreenInfo(Display* dpy, Drawable draw)
static void x11_XRRGetScreenInfo(CPU* cpu) {
    EAX = 1;// for now just assume one screen, XRRScreenConfiguration* is private so it doesn't need to be a real structure
}

// Bool XRRQueryExtension(Display* dpy, int* event_base_return, int* error_base_return)
static void x11_XRRQueryExtension(CPU* cpu) {
    cpu->memory->writed(ARG2, XRAND_Base);
    cpu->memory->writed(ARG3, XRAND_Error_Base);
    EAX = True;
}

// Status XRRQueryVersion(Display* dpy, int* major_version_return, int* minor_version_return)
static void x11_XRRQueryVersion(CPU* cpu) {
    EAX = 1;
    cpu->memory->writed(ARG2, 1);
    cpu->memory->writed(ARG3, 1); // :TODO: maybe figure out 1.4?
}

// short* XRRRates(Display* dpy, int screen, int size_index, int* nrates)
static void x11_XRRRates(CPU* cpu) {
    KThread* thread = cpu->thread;
    EAX = XrrRates(thread, ARG2, ARG3, ARG4);
}

// Status XRRSetScreenConfig(Display* dpy, XRRScreenConfiguration* config, Drawable draw, int size_index, Rotation rotation, Time timestamp)
static void x11_XRRSetScreenConfig(CPU* cpu) {
    kpanic("x11_XRRSetScreenConfig");
}

// Status XRRSetScreenConfigAndRate(Display* dpy, XRRScreenConfiguration* config, Drawable draw, int size_index, Rotation rotation, short rate, Time timestamp)
static void x11_XRRSetScreenConfigAndRate(CPU* cpu) {
    kpanic("x11_XRRSetScreenConfigAndRate");
}

// XRRScreenSize* XRRSizes(Display* dpy, int screen, int* nsizes)
static void x11_XRRSizes(CPU* cpu) {
    KThread* thread = cpu->thread;
    EAX = XrrGetSizes(thread, ARG2, ARG3);
}

// void XRRFreeCrtcInfo(XRRCrtcInfo* crtcInfo)
static void x11_XRRFreeCrtcInfo(CPU* cpu) {
    kpanic("x11_XRRFreeCrtcInfo");
}

// void XRRFreeOutputInfo(XRROutputInfo* outputInfo)
static void x11_XRRFreeOutputInfo(CPU* cpu) {
    kpanic("x11_XRRFreeOutputInfo");
}

// void XRRFreeScreenResources(XRRScreenResources* resources)
static void x11_XRRFreeScreenResources(CPU* cpu) {
    kpanic("x11_XRRFreeScreenResources");
}

// XRRCrtcInfo* XRRGetCrtcInfo(Display* dpy, XRRScreenResources* resources, RRCrtc crtc)
static void x11_XRRGetCrtcInfo(CPU* cpu) {
    kpanic("x11_XRRGetCrtcInfo");
}

// XRROutputInfo* XRRGetOutputInfo(Display* dpy, XRRScreenResources* resources, RROutput output)
static void x11_XRRGetOutputInfo(CPU* cpu) {
    kpanic("x11_XRRGetOutputInfo");
}

// int XRRGetOutputProperty(Display* dpy, RROutput output, Atom property, long offset, long length, Bool _delete, Bool pending, Atom req_type, Atom* actual_type, int* actual_format, unsigned long* nitems, unsigned long* bytes_after, unsigned char** prop)
static void x11_XRRGetOutputProperty(CPU* cpu) {
    kpanic("x11_XRRGetOutputProperty");
}

// XRRScreenResources* XRRGetScreenResources(Display* dpy, Window window)
static void x11_XRRGetScreenResources(CPU* cpu) {
    kpanic("x11_XRRGetScreenResources");
}

// XRRScreenResources* XRRGetScreenResourcesCurrent(Display* dpy, Window window)
static void x11_XRRGetScreenResourcesCurrent(CPU* cpu) {
    kpanic("x11_XRRGetScreenResourcesCurrent");
}

// Status XRRGetScreenSizeRange(Display* dpy, Window window, int* minWidth, int* minHeight, int* maxWidth, int* maxHeight)
static void x11_XRRGetScreenSizeRange(CPU* cpu) {
    kpanic("x11_XRRGetScreenSizeRange");
}

// Status XRRSetCrtcConfig(Display* dpy, XRRScreenResources* resources, RRCrtc crtc, Time timestamp, int x, int y, RRMode mode, Rotation rotation, RROutput* outputs, int noutputs)
static void x11_XRRSetCrtcConfig(CPU* cpu) {
    kpanic("x11_XRRSetCrtcConfig");
}

// void XRRSetScreenSize(Display* dpy, Window window, int width, int height, int mmWidth, int mmHeight)
static void x11_XRRSetScreenSize(CPU* cpu) {
    kpanic("x11_XRRSetScreenSize");
}

// void XRRSelectInput(Display* dpy, Window window, int mask)
static void x11_XRRSelectInput(CPU* cpu) {
    kpanic("x11_XRRSelectInput");
}

// RROutput XRRGetOutputPrimary(Display* dpy, Window window)
static void x11_XRRGetOutputPrimary(CPU* cpu) {
    kpanic("x11_XRRGetOutputPrimary");
}

// XRRProviderResources* XRRGetProviderResources(Display* dpy, Window window)
static void x11_XRRGetProviderResources(CPU* cpu) {
    kpanic("x11_XRRGetProviderResources");
}

// void XRRFreeProviderResources(XRRProviderResources* resources)
static void x11_XRRFreeProviderResources(CPU* cpu) {
    kpanic("x11_XRRFreeProviderResources");
}

// XRRProviderInfo* XRRGetProviderInfo(Display* dpy, XRRScreenResources* resources, RRProvider provider)
static void x11_XRRGetProviderInfo(CPU* cpu) {
    kpanic("x11_XRRGetProviderInfo");
}

// void XRRFreeProviderInfo(XRRProviderInfo* provider)
static void x11_XRRFreeProviderInfo(CPU* cpu) {
    kpanic("x11_XRRFreeProviderInfo");
}

void x11_init() {
    XKeyboard::init();

    int9BCallbackSize = X11_COUNT;   

    int9BCallback[X11_OPEN_DISPLAY] = x11_OpenDisplay;
    int9BCallback[X11_CLOSE_DISPLAY] = x11_CloseDisplay;
    int9BCallback[X11_GRAB_SERVER] = x11_GrabServer;
    int9BCallback[X11_UNGRAB_SERVER] = x11_UnGrabServer;
    int9BCallback[X11_INIT_THREADS] = x11_InitThread;
    int9BCallback[X11_CLEAR_AREA] = x11_ClearArea;
    int9BCallback[X11_SYNC] = x11_Sync;
    int9BCallback[X11_CREATE_WINDOW] = x11_CreateWindow;
    int9BCallback[X11_TRANSLATE_COORDINATES] = x11_TranslateCoordinates;
    int9BCallback[X11_DESTROY_WINDOW] = x11_DestroyWindow;
    int9BCallback[X11_REPARENT_WINDOW] = x11_ReparentWindow;
    int9BCallback[X11_QUERY_TREE] = x11_QueryTree;
    int9BCallback[X11_CHANGE_WINDOW_ATTRIBUTES] = x11_ChangeWindowAttributes;
    int9BCallback[X11_CONFIGURE_WINDOW] = x11_ConfigureWindow;
    int9BCallback[X11_SET_INPUT_FOCUS] = x11_SetInputFocus;
    int9BCallback[X11_SELECT_INPUT] = x11_SelectInput;
    int9BCallback[X11_FIND_CONTEXT] = x11_FindContext;
    int9BCallback[X11_SAVE_CONTEXT] = x11_SaveContext;
    int9BCallback[X11_DELETE_CONTEXT] = x11_DeleteContext;
    int9BCallback[X11_GET_INPUT_FOCUS] = x11_GetInputFocus;
    int9BCallback[X11_FREE_FONT] = x11_FreeFont;
    int9BCallback[X11_MOVE_RESIZE_WINDOW] = x11_MoveResizeWindow;
    int9BCallback[X11_MAP_WINDOW] = x11_MapWindow;
    int9BCallback[X11_UNMAP_WINDOW] = x11_UnmapWindow;
    int9BCallback[X11_GRAB_POINTER] = x11_GrabPointer;
    int9BCallback[X11_UNGRAB_POINTER] = x11_UngrabPointer;
    int9BCallback[X11_WARP_POINTER] = x11_WarpPointer;
    int9BCallback[X11_QUERY_POINTER] = x11_QueryPointer;
    int9BCallback[X11_MB_TEXT_LIST_TO_TEXT_PROPERTY] = x11_MbTextListToTextProperty;
    int9BCallback[X11_SET_TEXT_PROPERTY] = x11_SetTextProperty;
    int9BCallback[X11_SET_SELECTION_OWNER] = x11_SetSelectionOwner;
    int9BCallback[X11_GET_SELECTION_OWNER] = x11_GetSelectionOwner;
    int9BCallback[X11_GET_EVENT] = x11_GetEvent;
    int9BCallback[X11_SEND_EVENT] = x11_SendEvent;
    int9BCallback[X11_FILTER_EVENT] = x11_FilterEvent;
    int9BCallback[X11_LOOKUP_STRING] = x11_LookupString;
    int9BCallback[X11_MB_LOOKUP_STRING] = x11_MbLookupString;
    int9BCallback[X11_KEYSYM_TO_STRING] = x11_KeysymToString;
    int9BCallback[X11_KB_TRANSLATE_KEYSYM] = x11_KbTranslateKeysym;
    int9BCallback[X11_LOOKUP_KEYSYM] = x11_LookupKeysym;
    int9BCallback[X11_GET_KEYBOARD_MAPPING] = x11_GetKeyboardMapping;
    int9BCallback[X11_FREE_MODIFIER_MAP] = x11_FreeModifierMap;
    int9BCallback[X11_KB_KEYCODE_TO_KEYSYM] = x11_KbKeycodeToKeysym;
    int9BCallback[X11_DISPLAY_KEYCODES] = x11_DisplayKeycodes;
    int9BCallback[X11_GET_MODIFIER_MAPPING] = x11_GetModifierMapping;
    int9BCallback[X11_REFRESH_KEYBOARD_MAPPING] = x11_RefreshKeyboardMapping;
    int9BCallback[X11_BELL] = x11_Bell;
    int9BCallback[X11_GET_WINDOW_PROPERTY] = x11_GetWindowProperty;
    int9BCallback[X11_DELETE_PROPERTY] = x11_DeleteProperty;
    int9BCallback[X11_CONVERT_SELECTION] = x11_ConvertSelection;
    int9BCallback[X11_CHECK_TYPED_WINDOW_EVENT] = x11_CheckTypedWindowEvent;
    int9BCallback[X11_GET_GEOMETRY] = x11_GetGeometry;
    int9BCallback[X11_INTERN_ATOMS] = x11_InternAtoms;
    int9BCallback[X11_GET_ATOM_NAMES] = x11_GetAtomNames;
    int9BCallback[X11_CREATE_COLORMAP] = x11_CreateColorMap;
    int9BCallback[X11_FREE_COLORMAP] = x11_FreeColorMap;
    int9BCallback[X11_FREE_COLORS] = x11_FreeColors;
    int9BCallback[X11_QUERY_COLORS] = x11_QueryColors;
    int9BCallback[X11_ALLOC_COLOR] = x11_AllocColor;
    int9BCallback[X11_ALLOC_COLOR_CELLS] = x11_AllocColorCells;
    int9BCallback[X11_GET_VISUAL_INFO] = x11_GetVisualInfo;
    int9BCallback[X11_LIST_PIXEL_FORMATS] = x11_ListPixelFormats;
    int9BCallback[X11_LOCK_DISPLAY] = x11_LockDisplay;
    int9BCallback[X11_UNLOCK_DISPLAY] = x11_UnlockDisplay;
    int9BCallback[X11_COPY_AREA] = x11_CopyArea;
    int9BCallback[X11_GET_IMAGE] = x11_GetImage;
    int9BCallback[X11_PUT_IMAGE] = x11_PutImage;
    int9BCallback[X11_DESTROY_IMAGE] = x11_DestroyImage;
    int9BCallback[X11_GET_PIXEL] = x11_GetPixel;
    int9BCallback[X11_PUT_PIXEL] = x11_PutPixel;
    int9BCallback[X11_CREATE_PIXMAP] = x11_CreatePixmap;
    int9BCallback[X11_CREATE_BITMAP_FROM_DATA] = x11_CreateBitmapFromData;
    int9BCallback[X11_FREE_PIXMAP] = x11_FreePixmap;
    int9BCallback[X11_CREATE_PIXMAP_CURSOR] = x11_CreatePixmapCursor;
    int9BCallback[X11_CREATE_FONT_CURSOR] = x11_CreateFontCursor;
    int9BCallback[X11_DEFINE_CURSOR] = x11_DefineCursor;
    int9BCallback[X11_FREE_CURSOR] = x11_FreeCursor;
    int9BCallback[X11_SET_FUNCTION] = x11_SetFunction;
    int9BCallback[X11_SET_BACKGROUND] = x11_SetBackground;
    int9BCallback[X11_SET_FOREGROUND] = x11_SetForeground;
    int9BCallback[X11_COPY_PLANE] = x11_CopyPlane;
    int9BCallback[X11_CREATE_GC] = x11_CreateGC;
    int9BCallback[X11_SET_DASHES] = x11_SetDashes;
    int9BCallback[X11_DRAW_LINE] = x11_DrawLine;
    int9BCallback[X11_DRAW_LINES] = x11_DrawLines;
    int9BCallback[X11_SET_ARC_MODE] = x11_SetArcMode;
    int9BCallback[X11_FILL_ARC] = x11_FillArc;
    int9BCallback[X11_DRAW_ARC] = x11_DrawArc;
    int9BCallback[X11_DRAW_RECTANGLE] = x11_DrawRectangle;
    int9BCallback[X11_FILL_RECTANGLE] = x11_FillRectangle;
    int9BCallback[X11_FILL_RECTANGLES] = x11_FillRectangles;
    int9BCallback[X11_DRAW_POINT] = x11_DrawPoint;
    int9BCallback[X11_FILL_POLYGON] = x11_FillPolygon;
    int9BCallback[X11_CHANGE_GC] = x11_ChangeGC;
    int9BCallback[X11_FREE_GC] = x11_FreeGC;
    int9BCallback[X11_SET_SUBWINDOW_MODE] = x11_SetSubwindowMode;
    int9BCallback[X11_SET_GRAPHICS_EXPOSURES] = x11_SetGraphicsExposures;
    int9BCallback[X11_SET_FILL_STYLE] = x11_SetFillStyle;
    int9BCallback[X11_FREE] = x11_Free;
    int9BCallback[X11_SET_CLIP_MASK] = x11_SetClipMask;
    int9BCallback[X11_SET_CLIP_RECTANGLES] = x11_SetClipRectangles;
    int9BCallback[X11_SET_TRANSIENT_FOR_HINT] = x11_SetTransientForHint;
    int9BCallback[X11_ALLOC_WM_HINTS] = x11_AllocWMHints;
    int9BCallback[X11_ALLOC_CLASS_HINT] = x11_AllocClassHint;
    int9BCallback[X11_SET_CLASS_HINT] = x11_SetClassHint;
    int9BCallback[X11_SET_WM_NAME] = x11_SetWMName;
    int9BCallback[X11_SET_WM_ICON_NAME] = x11_SetWMIconName;
    int9BCallback[X11_SET_WM_NORMAL_HINTS] = x11_SetWMNormalHints;
    int9BCallback[X11_SET_WM_PROPERTIES] = x11_SetWMProperties;
    int9BCallback[X11_RECONFIGURE_WM_WINDOW] = x11_ReconfigureWMWindow;
    int9BCallback[X11_VA_CREATE_NESTED_LIST] = x11_VaCreateNestedList;
    int9BCallback[X11_UNSET_IC_FOCUS] = x11_UnsetICFocus;
    int9BCallback[X11_SET_IC_FOCUS] = x11_SetICFocus;
    int9BCallback[X11_DESTROY_IC] = x11_DestroyIC;
    int9BCallback[X11_SET_IC_VALUES] = x11_SetICValues;
    int9BCallback[X11_MB_RESET_IC] = x11_MbResetIC;
    int9BCallback[X11_SET_LOCALE_MODIFIERS] = x11_SetLocaleModifiers;
    int9BCallback[X11_OPEN_IM] = x11_OpenIM;
    int9BCallback[X11_CLOSE_IM] = x11_CloseIM;
    int9BCallback[X11_SET_IM_VALUES] = x11_SetIMValues;
    int9BCallback[X11_GET_IM_VALUES] = x11_GetIMValues;
    int9BCallback[X11_DISPLAY_OF_IM] = x11_DisplayOfIM;
    int9BCallback[X11_UNREGISTER_IM_INSTANTIATE_CALLBACK] = x11_UnregisterIMInstantiateCallback;
    int9BCallback[X11_REGISTER_IM_INSTANTIATE_CALLBACK] = x11_RegisterIMInstantiateCallback;
    int9BCallback[X11_FREE_STRING_LIST] = x11_FreeStringList;
    int9BCallback[X11_ALLOC_SIZE_HINTS] = x11_AllocSizeHints;
    int9BCallback[X11_CHANGE_PROPERTY] = x11_ChangeProperty;
    int9BCallback[X11_CREATE_FONT_SET] = x11_CreateFontSet;
    int9BCallback[X11_FREE_FONT_SET] = x11_FreeFontSet;
    int9BCallback[X11_CREATE_IC] = x11_CreateIC;
    int9BCallback[X11_CREATE_IMAGE] = x11_CreateImage;
    int9BCallback[X11_DISPLAY_NAME] = x11_DisplayName;
    int9BCallback[X11_GET_ATOM_NAME] = x11_GetAtomName;
    int9BCallback[X11_GET_DEFAULT] = x11_GetDefault;
    int9BCallback[X11_GET_WINDOW_ATTRIBUTES] = x11_GetWindowAttributes;
    int9BCallback[X11_ICONIFY_WINDOW] = x11_IconifyWindow;
    int9BCallback[X11_INTERN_ATOM] = x11_InternAtom;
    int9BCallback[X11_KEYSYM_TO_KEYCODE] = x11_KeysymToKeycode;
    int9BCallback[X11_LOCALE_OF_IM] = x11_LocaleOfIM;
    int9BCallback[X11_MATCH_VISUAL_INFO] = x11_MatchVisualInfo;
    int9BCallback[X11_QUERY_COLOR] = x11_QueryColor;
    int9BCallback[X11_QUERY_EXTENSION] = x11_QueryExtension;
    int9BCallback[X11_SET_WM_HINTS] = x11_SetWMHints;
    int9BCallback[X11_SHAPE_COMBINE_MASK] = x11_ShapeCombineMask;
    int9BCallback[X11_SHAPE_COMBINE_RECTANGLES] = x11_ShapeCombineRectangles;
    int9BCallback[X11_SHAPE_OFFSET_SHAPE] = x11_ShapeOffsetShape;
    int9BCallback[X11_SHM_ATTACH] = x11_ShmAttach;
    int9BCallback[X11_SHM_CREATE_IMAGE] = x11_ShmCreateImage;
    int9BCallback[X11_SHM_DETACH] = x11_ShmDetach;
    int9BCallback[X11_SHM_PUT_IMAGE] = x11_ShmPutImage;
    int9BCallback[X11_STORE_COLOR] = x11_StoreColor;
    int9BCallback[X11_WINDOW_EVENT] = x11_WindowEvent;
    int9BCallback[X11_WITHDRAW_WINDOW] = x11_WithDrawWindow;
    int9BCallback[X11_MB_TEXT_PROPERTY_TO_TEXT_LIST] = x11_MbTextListToTextProperty;
    int9BCallback[X11_RM_UNIQUE_QUARK] = x11_RmUniqueQuark;
    int9BCallback[X11_LOCK_EVENTS] = x11_LockEvents;
    int9BCallback[X11_UNLOCK_EVENTS] = x11_UnlockEvents;
    int9BCallback[X11_REMOVE_EVENT] = x11_RemoveEvent;
    int9BCallback[X11_XINERAMA_QUERY_SCREENS] = x11_XineramaQueryScreens;
    int9BCallback[X11_XRR_CONFIG_CURRENT_CONFIGURATION] = x11_XRRConfigCurrentConfiguration;
    int9BCallback[X11_XRR_CONFIG_CURRENT_RATE] = x11_XRRConfigCurrentRate;
    int9BCallback[X11_XRR_FREE_SCREEN_CONFIG_INFO] = x11_XRRFreeScreenConfigInfo;
    int9BCallback[X11_XRR_GET_SCREEN_INFO] = x11_XRRGetScreenInfo;
    int9BCallback[X11_XRR_QUERY_EXTENSION] = x11_XRRQueryExtension;
    int9BCallback[X11_XRR_QUERY_VERSION] = x11_XRRQueryVersion;
    int9BCallback[X11_XRR_RATES] = x11_XRRRates;
    int9BCallback[X11_XRR_SET_SCREEN_CONFIG] = x11_XRRSetScreenConfig;
    int9BCallback[X11_XRR_SET_SCREEN_CONFIG_AND_RATE] = x11_XRRSetScreenConfigAndRate;
    int9BCallback[X11_XRR_SIZES] = x11_XRRSizes;
    int9BCallback[X11_XRR_FREE_CRTC_INFO] = x11_XRRFreeCrtcInfo;
    int9BCallback[X11_XRR_FREE_OUTPUT_INFO] = x11_XRRFreeOutputInfo;
    int9BCallback[X11_XRR_FREE_SCREEN_RESOURCES] = x11_XRRFreeScreenResources;
    int9BCallback[X11_XRR_GET_CRTC_INFO] = x11_XRRGetCrtcInfo;
    int9BCallback[X11_XRR_GET_OUTPUT_INFO] = x11_XRRGetOutputInfo;
    int9BCallback[X11_XRR_GET_OUTPUT_PROPERTY] = x11_XRRGetOutputProperty;
    int9BCallback[X11_XRR_GET_SCREEN_RESOURCES] = x11_XRRGetScreenResources;
    int9BCallback[X11_XRR_GET_SCREEN_RESOURCES_CURRENT] = x11_XRRGetScreenResourcesCurrent;
    int9BCallback[X11_XRR_GET_SCREEN_SIZE_RANGE] = x11_XRRGetScreenSizeRange;
    int9BCallback[X11_XRR_SET_CRTC_CONFIG] = x11_XRRSetCrtcConfig;
    int9BCallback[X11_XRR_SET_SCREEN_SIZE] = x11_XRRSetScreenSize;
    int9BCallback[X11_XRR_SELECT_INPUT] = x11_XRRSelectInput;
    int9BCallback[X11_XRR_GET_OUTPUT_PRIMARY] = x11_XRRGetOutputPrimary;
    int9BCallback[X11_XRR_GET_PROVIDER_RESOURCES] = x11_XRRGetProviderResources;
    int9BCallback[X11_XRR_FREE_PROVIDER_RESOURCES] = x11_XRRFreeProviderResources;
    int9BCallback[X11_XRR_GET_PROVIDER_INFO] = x11_XRRGetProviderInfo;
    int9BCallback[X11_XRR_FREE_PROVIDER_INFO] = x11_XRRFreeProviderInfo;
    int9BCallback[X11_FLUSH] = x11_Flush;
    int9BCallback[X11_SUB_IMAGE] = x11_SubImage;
    int9BCallback[X11_ADD_PIXEL] = x11_AddPixel;
}

void callX11(CPU* cpu, U32 index) {
    if (index < int9BCallbackSize) {
        if (int9BCallback[index]) {
            int9BCallback[index](cpu);
        } else {
            kpanic("x11 tried to call missing function: %d", index);
        }
    } else
    {
        kpanic("x11 not compiled into Boxedwine: %d", index);
    }
}