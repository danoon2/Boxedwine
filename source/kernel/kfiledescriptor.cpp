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

#include "kscheduler.h"
#include "ksignal.h"
#include <string.h>

KFileDescriptor::~KFileDescriptor() {
    std::shared_ptr<KProcess> p = this->process.lock();
    if (p) {
        p->clearFdHandle(this->handle);
        /*  As well as being removed by an explicit F_UNLCK, record locks are
            automatically released when the process terminates or if it closes any
            file descriptor referring to a file on which locks are held.
        */
        this->kobject->unlockAll(p->id);
    }
}

bool KFileDescriptor::canRead() {
    return (this->accessFlags & K_O_ACCMODE)==K_O_RDONLY || (this->accessFlags & K_O_ACCMODE)==K_O_RDWR;
}

bool KFileDescriptor::canWrite() {
    return (this->accessFlags & K_O_ACCMODE)==K_O_WRONLY || (this->accessFlags & K_O_ACCMODE)==K_O_RDWR;
}


KFileDescriptor::KFileDescriptor(const std::shared_ptr<KProcess>& process, const std::shared_ptr<KObject>& kobject, U32 accessFlags, U32 descriptorFlags, S32 handle) {
    this->process = process;
    this->refCount = 1;
    this->handle = handle;
    this->accessFlags = accessFlags;
    this->descriptorFlags = descriptorFlags;
    this->kobject = kobject;  
}

void KFileDescriptor::close() {
    this->refCount--;
    if (!this->refCount) {
        delete this;
    }
}

