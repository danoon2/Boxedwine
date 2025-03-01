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

#ifndef __KTIMERCALLBACK_H__
#define __KTIMERCALLBACK_H__

class KTimerCallback {
public:
    KTimerCallback() : node(this), millies(0), resetMillies(0), active(false) {}
    ~KTimerCallback();

    virtual bool run() = 0; // return true of the timer should be removed, don't remove a timer manually in run because it will invalidate the iterator

    KListNode<KTimerCallback*> node;
    U32 millies;
    U32 resetMillies;
    bool active;
};

#endif