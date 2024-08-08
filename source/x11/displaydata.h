#ifndef __DISPLAY_DATA_H__
#define __DISPLAY_DATA_H__

#include "../util/bheap.h"
#include "xwindow.h"
#include "xpixmap.h"
#include "xgc.h"

#define ContextDataPtr std::shared_ptr<ContextData>

class ContextData {
public:
	bool get(U32 contextType, U32 result) {
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
	DisplayData(KMemory* memory);	

	U32 internAtom(const BString& name, bool onlyIfExists);
	bool getAtom(U32 atom, BString& name);
	U32 getNextQuark();

	XWindowPtr createNewWindow(KThread* thread, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width);
	XWindowPtr getWindow(U32 window);

	XPixmapPtr createNewPixmap(U32 width, U32 height, U32 depth);
	XPixmapPtr getPixmap(U32 pixmap);

	XGCPtr createGC(XDrawablePtr drawable);
	XGCPtr getGC(U32 gc);
	void removeGC(U32 gc);

	XDrawablePtr getDrawable(U32 xid);

	int getContextData(U32 context, U32 contextType, U32& ptr);
	int setContextData(U32 context, U32 contextType, U32 ptr);
	int deleteContextData(U32 context, U32 contextType);

	void putEvent(const XEvent& event);
	U32 lockEvents();
	XEvent* getEvent(U32 index); // only call between lockEvents/unlockEvents
	void removeEvent(U32 index); // only call between lockEvents/unlockEvents
	void unlockEvents();

	static U32 getNextId();	
private:
	friend struct Display;

	void initAtoms();
	void setAtom(const BString& name, U32 key);

	static std::atomic_int nextId;

	KMemory* memory;	

	BOXEDWINE_MUTEX atomMutex;
	BHashTable<U32, BString> atoms;
	BHashTable<BString, U32> reverseAtoms;
	U32 nextAtomID = 0;

	BOXEDWINE_MUTEX quarkMutex;
	U32 nextQuarkID = 0;

	BOXEDWINE_MUTEX windowsMutex;
	BHashTable<U32, XWindowPtr> windows;

	BOXEDWINE_MUTEX pixmapsMutex;
	BHashTable<U32, XPixmapPtr> pixmaps;

	BOXEDWINE_MUTEX gcsMutex;
	BHashTable<U32, XGCPtr> gcs;

	BOXEDWINE_MUTEX contextMutex;
	BHashTable<U32, ContextDataPtr> contextData;

	BOXEDWINE_MUTEX eventMutex;
	std::deque<XEvent> eventQueue;
};

#endif