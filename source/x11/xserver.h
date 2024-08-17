#ifndef __X_SERVER_H__
#define __X_SERVER_H__

#define XServerPtr std::shared_ptr<XServer>

#define XServerDisplayDataPtr std::shared_ptr<XServerDisplayData>

class XServer {
public:	
	static XServer* getServer(bool existingOnly = false);
	
	XServer();
	
	void mouseMove(S32 x, S32 y, bool relative);
	void mouseButton(U32 button, S32 x, S32 y, bool pressed);

	U32 internAtom(const BString& name, bool onlyIfExists);
	bool getAtom(U32 atom, BString& name);
	U32 getNextQuark();

	XWindowPtr createNewWindow(U32 displayId, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width);
	XWindowPtr getWindow(U32 window);
	int destroyWindow(U32 window);

	XPixmapPtr createNewPixmap(U32 width, U32 height, U32 depth);
	XPixmapPtr getPixmap(U32 pixmap);
	int removePixmap(U32 pixmap);

	XGCPtr createGC(XDrawablePtr drawable);
	XGCPtr getGC(U32 gc);
	void removeGC(U32 gc);

	XDrawablePtr getDrawable(U32 xid);	
	
	void addCursor(const XCursorPtr& cursor);
	XCursorPtr getCursor(U32 id);

	void iterateEventMask(U32 wndId, U32 mask, std::function<void(const DisplayDataPtr& data)> callback);

	U32 openDisplay(KThread* thread);
	DisplayDataPtr getDisplayDataByAddressOfDisplay(KMemory* memory, U32 address);
	DisplayDataPtr getDisplayDataById(U32 id);

	Screen* getScreen(KThread* thread, S32 screen);	

	void draw(bool drawNow = false);
	const XWindowPtr& getRoot();
	U32 getEventTime();
	U32 getInputModifiers();
	U32 grabPointer(const DisplayDataPtr& display, const XWindowPtr& grabbed, XWindowPtr confined, U32 mask, U32 time);
	U32 ungrabPointer(const DisplayDataPtr& display, U32 time);
	U32 setInputFocus(const DisplayDataPtr& data, U32 window, U32 revertTo, U32 time, bool trace = false);
	int mapWindow(const DisplayDataPtr& data, const XWindowPtr& window);
	int unmapWindow(const DisplayDataPtr& data, const XWindowPtr& window);

	static U32 getNextId();

	XrrData* xrrData = nullptr;

	XWindowPtr inputFocus;
	U32 inputFocusRevertTo = 0;

	XWindowPtr pointerWindow;
	
	bool trace = true;
	bool traceGC = false;
	bool isDisplayDirty = false;

private:
	static std::atomic_int nextId;
	static XServer* server;

	XWindowPtr root;

	XWindowPtr grabbed;
	XWindowPtr grabbedConfined;
	DisplayDataPtr grabbedDisplay;
	BOXEDWINE_MUTEX grabbedMutex;

	U32 grabbedMask;
	U32 grabbedTime;

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

	BOXEDWINE_MUTEX displayMutex;
	BHashTable<U32, DisplayDataPtr> displays;

	BOXEDWINE_MUTEX cursorsMutex;
	BHashTable<U32, XCursorPtr> cursors;

	void initAtoms();
	void setAtom(const BString& name, U32 key);

	void pointerMoved(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode);
	void pointerMovedBetweenSiblings(const XWindowPtr& from, const XWindowPtr& to, const XWindowPtr& commonAncestor, S32 x, S32 y, U32 mode);
	void pointerMovedChildToParent(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode);
	void pointerMovedParentToChild(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode);
};

#endif
