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

void scheduleThread(KThread* thread);
void unscheduleThread(KThread* thread);
void terminateOtherThread(const std::shared_ptr<KProcess>& process, U32 threadId);
void terminateCurrentThread(KThread* thread);

void addTimer(KTimerCallback* timer);
void removeTimer(KTimerCallback* timer);

bool runSlice();
void runThreadSlice(KThread* thread);
void waitForProcessToFinish(const std::shared_ptr<KProcess>& process, KThread* thread);
U32 getMIPS();

#endif