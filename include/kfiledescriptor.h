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

#ifndef __KFILEDESCRIPTOR_H__
#define __KFILEDESCRIPTOR_H__

#define K_F_DUPFD    0
#define K_F_GETFD    1
#define K_F_SETFD    2
#define K_F_GETFL    3
#define K_F_SETFL    4
#define K_F_GETLK	 5
#define K_F_SETLK	 6
#define K_F_SETLKW   7
#define K_F_SETOWN   8
#define K_F_SETSIG   10
#define K_F_GETSIG   11
#define K_F_GETLK64  12
#define K_F_SETLK64  13
#define K_F_SETLKW64 14
#define K_F_DUPFD_CLOEXEC 1030	
#define K_F_ADD_SEALS        1033
#define K_F_GET_SEALS        1034

// type of lock
#define K_F_RDLCK	   0
#define K_F_WRLCK	   1
#define K_F_UNLCK	   2

class KProcess;

class KFileDescriptor {
public:
    KFileDescriptor(const std::shared_ptr<KProcess>& process, const BoxedPtr<KObject>& kobject, U32 accessFlags, U32 descriptorFlags, S32 handle);
    ~KFileDescriptor();

    bool canRead();
    bool canWrite();
    void close();

    U32 accessFlags;
    U32 descriptorFlags;
    U32 handle;
    BoxedPtr<KObject> kobject;
    U32 refCount;
    std::weak_ptr<KProcess> process;
};

#endif
