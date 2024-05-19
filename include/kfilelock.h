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
    KFileLock() = default;

    U32 l_type = 0;
    U32 l_whence = 0;
    U64 l_start = 0;
    U64 l_len = 0;
    U32 l_pid = 0;

    bool operator==(const KFileLock& r) const
    {
        return l_type == r.l_type && l_whence == r.l_whence && l_start == r.l_start && l_len == r.l_len && l_pid == r.l_pid;
    }

    void writeFileLock(KThread* thread, U32 address, bool is64);
    void readFileLock(KThread* thread, U32 address, bool is64);
};

#endif