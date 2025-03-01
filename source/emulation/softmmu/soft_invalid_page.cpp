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

#include "boxedwine.h"

#include "soft_invalid_page.h"
#include "kmemory_soft.h"
#include "soft_mmu.h"

U8 InvalidPage::readb(MMU* mmu, U32 address) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writeb(MMU* mmu, U32 address, U8 value) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, false, true);
}

U16 InvalidPage::readw(MMU* mmu, U32 address) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writew(MMU* mmu, U32 address, U16 value) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, false, true);
}

U32 InvalidPage::readd(MMU* mmu, U32 address) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writed(MMU* mmu, U32 address, U32 value) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, false, true);
}

bool InvalidPage::canReadRam(MMU* mmu) {
    return false;
}

bool InvalidPage::canWriteRam(MMU* mmu) {
    return false;
}

U8* InvalidPage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    return nullptr;
}

void InvalidPage::onDemmand(MMU* mmu, U32 page) {
}
