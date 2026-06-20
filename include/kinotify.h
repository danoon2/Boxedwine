/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __KINOTIFY_H__
#define __KINOTIFY_H__

#define K_IN_ACCESS        0x00000001
#define K_IN_MODIFY        0x00000002
#define K_IN_ATTRIB        0x00000004
#define K_IN_CLOSE_WRITE   0x00000008
#define K_IN_CLOSE_NOWRITE 0x00000010
#define K_IN_OPEN          0x00000020
#define K_IN_MOVED_FROM    0x00000040
#define K_IN_MOVED_TO      0x00000080
#define K_IN_CREATE        0x00000100
#define K_IN_DELETE        0x00000200
#define K_IN_DELETE_SELF   0x00000400
#define K_IN_MOVE_SELF     0x00000800
#define K_IN_ALL_EVENTS    0x00000fff

#define K_IN_UNMOUNT       0x00002000
#define K_IN_Q_OVERFLOW    0x00004000
#define K_IN_IGNORED       0x00008000
#define K_IN_ONLYDIR       0x01000000
#define K_IN_DONT_FOLLOW   0x02000000
#define K_IN_EXCL_UNLINK   0x04000000
#define K_IN_MASK_ADD      0x20000000
#define K_IN_ISDIR         0x40000000
#define K_IN_ONESHOT       0x80000000

class KInotifyObject : public KObject {
public:
    KInotifyObject();
    virtual ~KInotifyObject();

    static U32 create(KThread* thread, U32 flags);
    static U32 addWatch(KThread* thread, FD fd, BString path, U32 mask);
    static U32 removeWatch(KThread* thread, FD fd, S32 wd);
    static void notifyPath(const BString& fullPath, U32 mask);

    U32 addWatch(KProcess* process, const BString& path, U32 mask);
    U32 removeWatch(S32 wd);

    U32 ioctl(KThread* thread, U32 request) override;
    S64 seek(S64 pos) override;
    S64 length() override;
    S64 getPos() override;
    void setBlocking(bool blocking) override;
    bool isBlocking() override;
    void setAsync(bool isAsync) override;
    bool isAsync() override;
    KFileLock* getLock(KFileLock* lock) override;
    U32 setLock(KFileLock* lock, bool wait) override;
    bool supportsLocks() override;
    bool isOpen() override;
    bool isReadReady() override;
    bool isWriteReady() override;
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    U32 write(KThread* thread, U32 buffer, U32 len) override;
    U32 writeNative(U8* buffer, U32 len) override;
    U32 read(KThread* thread, U32 buffer, U32 len) override;
    U32 readNative(U8* buffer, U32 len) override;
    U32 stat(KProcess* process, U32 address, bool is64) override;
    U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override;
    bool canMap() override;
    BString selfFd() override;

private:
    struct Watch {
        S32 wd = 0;
        BString path;
        U32 mask = 0;
    };

    BOXEDWINE_CONDITION lockCond;
    std::vector<Watch> watches;
    std::deque<U8> recvBuffer;
    S32 nextWatch = 1;
    bool blocking = true;
    bool async = false;
    U64 createdTime = 0;

    void queueEvent(const BString& parentPath, const BString& name, U32 mask);
    void appendEvent(S32 wd, U32 mask, const BString& name);
    U32 bytesAvailableForRead(U32 len);
    U32 eventLengthAt(U32 offset);
};

#endif
