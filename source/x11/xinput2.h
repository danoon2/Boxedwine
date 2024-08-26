#ifndef __XINPUT2_H__
#define __XINPUT2_H__

//https://www.x.org/releases/X11R7.7/doc/inputproto/XI2proto.txt

#define XI_2_Major                              2
#define XI_2_Minor                              4

/* Property event flags */
#define XIPropertyDeleted                       0
#define XIPropertyCreated                       1
#define XIPropertyModified                      2

/* Property modes */
#define XIPropModeReplace                       0
#define XIPropModePrepend                       1
#define XIPropModeAppend                        2

/* Special property type used for XIGetProperty */
#define XIAnyPropertyType                       0L

/* Enter/Leave and Focus In/Out modes */
#define XINotifyNormal                          0
#define XINotifyGrab                            1
#define XINotifyUngrab                          2
#define XINotifyWhileGrabbed                    3
#define XINotifyPassiveGrab                     4
#define XINotifyPassiveUngrab                   5

/* Enter/Leave and focus In/out detail */
#define XINotifyAncestor                        0
#define XINotifyVirtual                         1
#define XINotifyInferior                        2
#define XINotifyNonlinear                       3
#define XINotifyNonlinearVirtual                4
#define XINotifyPointer                         5
#define XINotifyPointerRoot                     6
#define XINotifyDetailNone                      7

/* Grab modes */
#define XIGrabModeSync                          0
#define XIGrabModeAsync                         1
#define XIGrabModeTouch                         2

/* Grab reply status codes */
#define XIGrabSuccess                           0
#define XIAlreadyGrabbed                        1
#define XIGrabInvalidTime                       2
#define XIGrabNotViewable                       3
#define XIGrabFrozen                            4

/* Grab owner events values */
#define XIOwnerEvents                           True
#define XINoOwnerEvents                         False

/* Passive grab types */
#define XIGrabtypeButton                        0
#define XIGrabtypeKeycode                       1
#define XIGrabtypeEnter                         2
#define XIGrabtypeFocusIn                       3
#define XIGrabtypeTouchBegin                    4
#define XIGrabtypeGesturePinchBegin             5
#define XIGrabtypeGestureSwipeBegin             6

/* Passive grab modifier */
#define XIAnyModifier                           (1U << 31)
#define XIAnyButton                             0
#define XIAnyKeycode                            0

/* XIAllowEvents event-modes */
#define XIAsyncDevice                           0
#define XISyncDevice                            1
#define XIReplayDevice                          2
#define XIAsyncPairedDevice                     3
#define XIAsyncPair                             4
#define XISyncPair                              5
#define XIAcceptTouch                           6
#define XIRejectTouch                           7

/* DeviceChangedEvent change reasons */
#define XISlaveSwitch                           1
#define XIDeviceChange                          2

/* Hierarchy flags */
#define XIMasterAdded                           (1 << 0)
#define XIMasterRemoved                         (1 << 1)
#define XISlaveAdded                            (1 << 2)
#define XISlaveRemoved                          (1 << 3)
#define XISlaveAttached                         (1 << 4)
#define XISlaveDetached                         (1 << 5)
#define XIDeviceEnabled                         (1 << 6)
#define XIDeviceDisabled                        (1 << 7)

/* ChangeHierarchy constants */
#define XIAddMaster                             1
#define XIRemoveMaster                          2
#define XIAttachSlave                           3
#define XIDetachSlave                           4

#define XIAttachToMaster                        1
#define XIFloating                              2

/* Valuator modes */
#define XIModeRelative                          0
#define XIModeAbsolute                          1

/* Device types */
#define XIMasterPointer                         1
#define XIMasterKeyboard                        2
#define XISlavePointer                          3
#define XISlaveKeyboard                         4
#define XIFloatingSlave                         5

/* Device classes: classes that are not identical to Xi 1.x classes must be
 * numbered starting from 8. */
#define XIKeyClass                              0
#define XIButtonClass                           1
#define XIValuatorClass                         2
#define XIScrollClass                           3
#define XITouchClass                            8
#define XIGestureClass                          9

 /* Scroll class types */
#define XIScrollTypeVertical                    1
#define XIScrollTypeHorizontal                  2

/* Scroll class flags */
#define XIScrollFlagNoEmulation                 (1 << 0)
#define XIScrollFlagPreferred                   (1 << 1)

/* Device event flags (common) */
/* Device event flags (key events only) */
#define XIKeyRepeat                             (1 << 16)
/* Device event flags (pointer events only) */
#define XIPointerEmulated                       (1 << 16)
/* Device event flags (touch events only) */
#define XITouchPendingEnd                       (1 << 16)
#define XITouchEmulatingPointer                 (1 << 17)

/* Barrier event flags */
#define XIBarrierPointerReleased                (1 << 0)
#define XIBarrierDeviceIsGrabbed                (1 << 1)

/* Gesture pinch event flags */
#define XIGesturePinchEventCancelled            (1 << 0)

/* Gesture swipe event flags */
#define XIGestureSwipeEventCancelled            (1 << 0)

/* Touch modes */
#define XIDirectTouch                           1
#define XIDependentTouch                        2

/* XI2 event mask macros */
#define XISetMask(ptr, event)   (((unsigned char*)(ptr))[(event)>>3] |=  (1 << ((event) & 7)))
#define XIClearMask(ptr, event) (((unsigned char*)(ptr))[(event)>>3] &= ~(1 << ((event) & 7)))
#define XIMaskIsSet(ptr, event) (((unsigned char*)(ptr))[(event)>>3] &   (1 << ((event) & 7)))
#define XIMaskLen(event)        (((event) >> 3) + 1)

/* Fake device ID's for event selection */
#define XIAllDevices                            0
#define XIAllMasterDevices                      1

/* Event types */
#define XI_DeviceChanged                 1
#define XI_KeyPress                      2
#define XI_KeyRelease                    3
#define XI_ButtonPress                   4
#define XI_ButtonRelease                 5
#define XI_Motion                        6
#define XI_Enter                         7
#define XI_Leave                         8
#define XI_FocusIn                       9
#define XI_FocusOut                      10
#define XI_HierarchyChanged              11
#define XI_PropertyEvent                 12
#define XI_RawKeyPress                   13
#define XI_RawKeyRelease                 14
#define XI_RawButtonPress                15
#define XI_RawButtonRelease              16
#define XI_RawMotion                     17
#define XI_TouchBegin                    18 /* XI 2.2 */
#define XI_TouchUpdate                   19
#define XI_TouchEnd                      20
#define XI_TouchOwnership                21
#define XI_RawTouchBegin                 22
#define XI_RawTouchUpdate                23
#define XI_RawTouchEnd                   24
#define XI_BarrierHit                    25 /* XI 2.3 */
#define XI_BarrierLeave                  26
#define XI_GesturePinchBegin             27 /* XI 2.4 */
#define XI_GesturePinchUpdate            28
#define XI_GesturePinchEnd               29
#define XI_GestureSwipeBegin             30
#define XI_GestureSwipeUpdate            31
#define XI_GestureSwipeEnd               32
#define XI_LASTEVENT                     XI_GestureSwipeEnd
/* NOTE: XI2LASTEVENT in xserver/include/inputstr.h must be the same value
 * as XI_LASTEVENT if the server is supposed to handle masks etc. for this
 * type of event. */

 /* Event masks.
  * Note: the protocol spec defines a mask to be of (1 << type). Clients are
  * free to create masks by bitshifting instead of using these defines.
  */
#define XI_DeviceChangedMask             (1 << XI_DeviceChanged)
#define XI_KeyPressMask                  (1 << XI_KeyPress)
#define XI_KeyReleaseMask                (1 << XI_KeyRelease)
#define XI_ButtonPressMask               (1 << XI_ButtonPress)
#define XI_ButtonReleaseMask             (1 << XI_ButtonRelease)
#define XI_MotionMask                    (1 << XI_Motion)
#define XI_EnterMask                     (1 << XI_Enter)
#define XI_LeaveMask                     (1 << XI_Leave)
#define XI_FocusInMask                   (1 << XI_FocusIn)
#define XI_FocusOutMask                  (1 << XI_FocusOut)
#define XI_HierarchyChangedMask          (1 << XI_HierarchyChanged)
#define XI_PropertyEventMask             (1 << XI_PropertyEvent)
#define XI_RawKeyPressMask               (1 << XI_RawKeyPress)
#define XI_RawKeyReleaseMask             (1 << XI_RawKeyRelease)
#define XI_RawButtonPressMask            (1 << XI_RawButtonPress)
#define XI_RawButtonReleaseMask          (1 << XI_RawButtonRelease)
#define XI_RawMotionMask                 (1 << XI_RawMotion)
#define XI_TouchBeginMask                (1 << XI_TouchBegin)
#define XI_TouchEndMask                  (1 << XI_TouchEnd)
#define XI_TouchOwnershipChangedMask     (1 << XI_TouchOwnership)
#define XI_TouchUpdateMask               (1 << XI_TouchUpdate)
#define XI_RawTouchBeginMask             (1 << XI_RawTouchBegin)
#define XI_RawTouchEndMask               (1 << XI_RawTouchEnd)
#define XI_RawTouchUpdateMask            (1 << XI_RawTouchUpdate)
#define XI_BarrierHitMask                (1 << XI_BarrierHit)
#define XI_BarrierLeaveMask              (1 << XI_BarrierLeave)

struct XIModifierState {
    S32    base;
    S32    latched;
    S32    locked;
    S32    effective;
};

typedef XIModifierState XIGroupState;

struct XIButtonState {
    S32 mask_len;
    U32 maskAddress;
};

struct XIValuatorState {
    S32 mask_len;
    U32 maskAddress; // char*
    U32 valuesAddress; // double*
};

struct XIEventMask {
    S32 deviceid;
    S32 mask_len;
    U32 maskAddress;

    void read(KMemory* memory, U32 address);
};

struct XIAnyClassInfo {
    S32         type;
    S32         sourceid;
};

struct XIButtonClassInfo {
    S32 type;
    S32 sourceid;
    S32 num_buttons;
    U32 labelsAddress; // Atom*
    XIButtonState state;
};

struct XIKeyClassInfo {
    S32 type;
    S32 sourceid;
    S32 num_keycodes;
    U32 keycodesAddress; // int*
};

struct XIValuatorClassInfo {
    S32         type;
    S32         sourceid;
    S32         number;
    Atom        label;
    double      min;
    double      max;
    double      value;
    S32         resolution;
    S32         mode;

    static void write(KMemory* memory, U32 address, S32 type, S32 sourceid, S32 number, Atom label, double min, double max, double value, S32 resolution, S32 mode);
};

/* new in XI 2.1 */
struct XIScrollClassInfo
{
    S32         type;
    S32         sourceid;
    S32         number;
    S32         scroll_type;
    double      increment;
    S32         flags;
};

struct XITouchClassInfo {
    S32         type;
    S32         sourceid;
    S32         mode;
    S32         num_touches;
};

/* new in XI 2.4 */
struct XIGestureClassInfo {
    S32         type;
    S32         sourceid;
    S32         num_touches;
};

struct XIDeviceInfo {
    S32                 deviceid;
    U32 nameAddress; // char*
    S32                 use;
    S32                 attachment;
    Bool                enabled;
    S32                 num_classes;
    U32 classesAddress; // XIAnyClassInfo**

    static void write(KMemory* memory, U32 address, S32 deviceid, U32 name, S32 use, S32 attachment, Bool enabled, S32 num_classes, U32 classes);
};

struct XIRawEvent {
    S32           type;         /* GenericEvent */
    U32 serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    U32 displayAddress;     /* Display the event was read from */
    S32           extension;    /* XI extension offset */
    S32           evtype;       /* XI_RawKeyPress, XI_RawKeyRelease, etc. */
    Time          time;
    S32           deviceid;
    S32           sourceid;     /* Bug: Always 0. https://bugs.freedesktop.org//show_bug.cgi?id=34240 */
    S32           detail;
    S32           flags;
    XIValuatorState valuators;
    U32 raw_values; // double*

    void serialize(U32* data);
    void unserialize(U32* data);
};

#endif