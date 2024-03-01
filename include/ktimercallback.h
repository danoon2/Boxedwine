#ifndef __KTIMERCALLBACK_H__
#define __KTIMERCALLBACK_H__

class KTimerCallback {
public:
    KTimerCallback() : node(this), millies(0), resetMillies(0), active(false) {}
    ~KTimerCallback();

    virtual bool run() = 0; // return true of the timer should be removed, don't remove a timer manually in run because it will invalidate the iterator

    KListNode<KTimerCallback*> node;
    U32 millies;
    U32 resetMillies;
    bool active;
};

#endif