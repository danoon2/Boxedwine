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

#ifndef __KFILE_H__
#define __KFILE_H__

class KFile : public KObject {
public:
    KFile(FsOpenNode* openFile);
    virtual ~KFile();

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

    U32 pwrite(KThread* thread, U32 buffer, S64 offset, U32 len);
    U32 pread(KThread* thread, U32 buffer,S64 offset,  U32 len);

    FsOpenNode* openFile;

private:
    BOXEDWINE_MUTEX filePosMutex;
};

#endif