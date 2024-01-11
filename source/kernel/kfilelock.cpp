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
    KMemory* memory = thread->memory;

    if (!is64) {
        memory->writew(address, this->l_type);address+=2;
        memory->writew(address, this->l_whence); address += 2;
        memory->writed(address, (U32)this->l_start); address += 4;
        memory->writed(address, (U32)this->l_len); address += 4;
        memory->writed(address, (U32)this->l_pid);
    } else {
        memory->writew(address, this->l_type); address += 2;
        memory->writew(address, this->l_whence); address += 2;
        memory->writeq(address, this->l_start); address += 8;
        memory->writeq(address, this->l_len); address += 8;
        memory->writed(address, this->l_pid);
    }
}

void KFileLock::readFileLock(KThread* thread, U32 address, bool is64) {
    KMemory* memory = thread->memory;

    if (!is64) {
        this->l_type = memory->readw(address); address += 2;
        this->l_whence = memory->readw(address); address += 2;
        this->l_start = memory->readd(address); address += 4;
        this->l_len = memory->readd(address); address += 4;
        this->l_pid = memory->readd(address);
    } else {
        this->l_type = memory->readw(address); address += 2;
        this->l_whence = memory->readw(address); address += 2;
        this->l_start = memory->readq(address); address += 8;
        this->l_len = memory->readq(address); address += 8;
        this->l_pid = memory->readd(address);
    }
}