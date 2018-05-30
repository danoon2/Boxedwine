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

#include "boxedwine.h"
#include "kfilelock.h"

void KFileLock::writeFileLock(KThread* thread, U32 address, bool is64) {
    if (!is64) {
        writew(address, this->l_type);address+=2;
        writew(address, this->l_whence); address += 2;
        writed(address, (U32)this->l_start); address += 4;
        writed(address, (U32)this->l_len); address += 4;
        writed(address, (U32)this->l_pid);
    } else {
        writew(address, this->l_type); address += 2;
        writew(address, this->l_whence); address += 2;
        writeq(address, this->l_start); address += 8;
        writeq(address, this->l_len); address += 8;
        writed(address, this->l_pid);
    }
}

void KFileLock::readFileLock(KThread* thread, U32 address, bool is64) {
    if (!is64) {
        this->l_type = readw(address); address += 2;
        this->l_whence = readw(address); address += 2;
        this->l_start = readd(address); address += 4;
        this->l_len = readd(address); address += 4;
        this->l_pid = readd(address);
    } else {
        this->l_type = readw(address); address += 2;
        this->l_whence = readw(address); address += 2;
        this->l_start = readq(address); address += 8;
        this->l_len = readq(address); address += 8;
        this->l_pid = readd(address);
    }
}