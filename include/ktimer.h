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

#ifndef __KTIMER_H__
#define __KTIMER_H__

class KTimer;

class KRunTimer : public KTimerCallback {
public:
    KRunTimer(KTimer* timer) : timer(timer) {}
    bool run() override;
private:
    KTimer* timer;
};

class KTimer : public KObject {
public:
    KTimer();
    virtual ~KTimer();

    // From KObject
    U32 ioctl(KThread* thread, U32 request) override;
    S64 seek(S64 pos) override;
    S64 length() override;
    S64 getPos() override;
    void setBlocking(bool blocking) override;
    bool isBlocking() override;
    void setAsync(bool isAsync) override;
    bool isAsync() override;
    KFileLock* getLock(KFileLock* lock) override;
    U32  setLock(KFileLock* lock, bool wait) override;
    void unlockAll(U32 pid) override;
    bool supportsLocks() override;
    bool isOpen() override;
    bool isReadReady() override;
    bool isWriteReady() override;
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    U32  write(KThread* thread, U32 buffer, U32 len) override;
    U32  writeNative(U8* buffer, U32 len) override;
    U32  read(KThread* thread, U32 buffer, U32 len) override;
    U32  readNative(U8* buffer, U32 len) override;
    U32  stat(KProcess* process, U32 address, bool is64) override;
    U32  map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override;
    bool canMap() override;
    BString selfFd() override;
    
    U64 getMicroInterval() {
        return microInterval;
    }
    U64 getMicroNextTimer() {
        return microNextTimer;
    }

    void setTimes(U64 microInterval, U64 microNextTimer);

private:    
    friend class KRunTimer;

    U64 microInterval = 0; // if 0 then the timer will only fire once
    U64 microNextTimer = 0;

    bool blocking = true;
    U64 numberOfExpirations = 0;
    BOXEDWINE_CONDITION waitCond;
    KRunTimer timer;
    
};

#endif