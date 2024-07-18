#ifndef __DISPLAY_DATA_H__
#define __DISPLAY_DATA_H__

#include "../util/bheap.h"
#include "xwindow.h"

class DisplayData {
public:
	DisplayData(KMemory* memory);	

	U32 internAtom(const BString& name, bool onlyIfExists);
	bool getAtom(U32 atom, BString& name);
	U32 getNextQuark();

	XWindowPtr createNewWindow(const XWindowPtr& parent, U32 width, U32 height);
	XWindowPtr getWindow(U32 window);
private:
	friend struct Display;

	void initAtoms();
	void setAtom(const BString& name, U32 key);

	KMemory* memory;

	BOXEDWINE_MUTEX heapMutex;
	BHeap heap;

	BOXEDWINE_MUTEX atomMutex;
	BHashTable<U32, BString> atoms;
	BHashTable<BString, U32> reverseAtoms;
	U32 nextAtomID = 0;

	BOXEDWINE_MUTEX quarkMutex;
	U32 nextQuarkID = 0;

	BOXEDWINE_MUTEX windowsMutex;
	BHashTable<U32, XWindowPtr> windows;
};

#endif