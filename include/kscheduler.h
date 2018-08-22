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

#ifndef __KSCHEDULER_H__
#define __KSCHEDULER_H__

void unscheduleThread(KThread* thread); // will not return if thread is current thread and BOXEDWINE_VM
void wakeThread(KThread* thread);
void wakeThreads(U32 wakeType);
void addTimer(KTimer* timer);
void removeTimer(KTimer* timer);

bool runSlice();
void runThreadSlice(KThread* thread);
void platformRunThreadSlice(KThread* thread);
#ifdef BOXEDWINE_DEFAULT_MMU
#define platformRunThreadSlice runThreadSlice
#endif
void waitThread(KThread* thread);
void scheduleThread(KThread* thread);
U32 getMIPS();

#endif