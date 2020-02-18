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
    virtual U32  ioctl(U32 request);
    virtual S64  seek(S64 pos);
    virtual S64  length();
    virtual S64  getPos();
    virtual void setBlocking(bool blocking);
    virtual bool isBlocking();
    virtual void setAsync(bool isAsync);
    virtual bool isAsync();
    virtual KFileLock* getLock(KFileLock* lock);
    virtual U32  setLock(KFileLock* lock, bool wait);
    virtual bool supportsLocks();
    virtual bool isOpen();
    virtual bool isReadReady();
    virtual bool isWriteReady();
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events);
    virtual U32  write(U32 buffer, U32 len);
    virtual U32  writeNative(U8* buffer, U32 len);
    virtual U32  read(U32 buffer, U32 len);
    virtual U32  readNative(U8* buffer, U32 len);
    virtual U32  stat(U32 address, bool is64);
    virtual U32  map(U32 address, U32 len, S32 prot, S32 flags, U64 off);
    virtual bool canMap();

    U32 pwrite(U32 buffer, S64 offset, U32 len);
    U32 pread(U32 buffer,S64 offset,  U32 len);

    FsOpenNode* openFile;

private:
    BOXEDWINE_MUTEX filePosMutex;
};

#endif