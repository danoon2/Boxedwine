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

#ifndef __DISPLAY_DATA_H__
#define __DISPLAY_DATA_H__

#include "../util/bheap.h"

#define DisplayDataPtr std::shared_ptr<DisplayData>
#define ContextDataPtr std::shared_ptr<ContextData>

class ContextData {
public:
	bool get(U32 contextType, U32& result) {
		return data.get(contextType, result);
	}
	void put(U32 contextType, U32 ptr) {
		data.set(contextType, ptr);
	}
	bool remove(U32 contextType) {
		U32 prev;

		if (data.get(contextType, prev)) {
			data.remove(contextType);
			return true;
		}
		return false;
	}
private:
	BHashTable<U32, U32> data;
};

class DisplayData {
public:
	DisplayData();

	void close(KThread* thread);

	bool findAndRemoveEvent(U32 window, S32 type, XEvent& event);
	void putEvent(const XEvent& event, bool inFront = false);
	U32 lockEvents();
	XEvent* getEvent(U32 index); // only call between lockEvents/unlockEvents
	void removeEvent(U32 index); // only call between lockEvents/unlockEvents
	void unlockEvents();
		
	U32 getEventMask(U32 window);
	void setEventMask(U32 window, U32 mask);
	U32 getInput2Mask(U32 window);
	int setInput2Mask(U32 window, U32 mask);

	U32 getNextEventSerial();	
	
	int getContextData(U32 context, U32 contextType, U32& ptr);
	int setContextData(U32 context, U32 contextType, U32 ptr);
	int deleteContextData(U32 context, U32 contextType);

	U32 displayAddress;
	U32 displayId;
	U32 root;
	U32 clientFd;
	U32 serverFd;
	KProcessWeakPtr process;
	U32 processId;
	XrrData* xrrData = nullptr;

#ifdef BOXEDWINE_MULTI_THREADED
	BOXEDWINE_MUTEX mutex;
#else
	BOXEDWINE_CONDITION cond;
#endif
	bool isLocked = false;

private:	
	std::atomic_int nextEventSerial;

#ifdef BOXEDWINE_MULTI_THREADED
	BOXEDWINE_MUTEX eventMutex;
#else
	BOXEDWINE_CONDITION eventCond;
	bool eventQueueIsLocked = false;
#endif
	std::deque<XEvent> eventQueue;

	BOXEDWINE_MUTEX eventMaskMutex;
	BHashTable<U32, U32> perWindowEventMask;
	BHashTable<U32, U32> perWindowEventMask2;

	BOXEDWINE_MUTEX contextMutex;
	BHashTable<U32, ContextDataPtr> contextData;
};

#endif