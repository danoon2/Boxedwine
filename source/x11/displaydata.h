#ifndef __DISPLAY_DATA_H__
#define __DISPLAY_DATA_H__

#include "../util/bheap.h"

#define DisplayDataPtr std::shared_ptr<DisplayData>

class DisplayData {
public:
	void putEvent(const XEvent& event);
	U32 lockEvents();
	XEvent* getEvent(U32 index); // only call between lockEvents/unlockEvents
	void removeEvent(U32 index); // only call between lockEvents/unlockEvents
	void unlockEvents();
		
	U32 getEventMask(U32 window);
	void setEventMask(U32 window, U32 mask);

	U32 getNextEventSerial();	
	
	U32 displayAddress;

	BOXEDWINE_MUTEX mutex;
private:	
	std::atomic_int nextEventSerial;

	BOXEDWINE_MUTEX eventMutex;
	std::deque<XEvent> eventQueue;

	BOXEDWINE_MUTEX eventMaskMutex;
	BHashTable<U32, U32> perWindowEventMask;
};

#endif