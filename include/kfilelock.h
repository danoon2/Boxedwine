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

#ifndef __KFILE_LOCK__
#define __KFILE_LOCK__

class KFileLock {
public:
    U32 l_type;
    U32 l_whence;
    U64 l_start;
    U64 l_len;
    U32 l_pid;

    void writeFileLock(KThread* thread, U32 address, bool is64);
    void readFileLock(KThread* thread, U32 address, bool is64);
};

#endif