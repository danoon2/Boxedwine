#ifndef __X_SERVER_H__
#define __X_SERVER_H__

#define XI_DEVICE_ID 1

#define XServerPtr std::shared_ptr<XServer>

#define XServerDisplayDataPtr std::shared_ptr<XServerDisplayData>

class XServer {
public:	
	static XServer* getServer(bool existingOnly = false);
	static void shutdown();

	XServer();
	
	void mouseMove(S32 x, S32 y, bool relative);
	void mouseButton(U32 button, S32 x, S32 y, bool pressed);
	void key(U32 key, bool pressed);

	U32 internAtom(const BString& name, bool onlyIfExists);
	bool getAtom(U32 atom, BString& name);
	U32 getNextQuark();
	U32 getExtensionInput2() {return this->extensionXinput2;}
	U32 getExtensionGLX() {return this->extensionGLX;}

	XWindowPtr createNewWindow(U32 displayId, const XWindowPtr& parent, U32 width, U32 height, U32 depth, U32 x, U32 y, U32 c_class, U32 border_width, const VisualPtr& visual);
	XWindowPtr getWindow(U32 window);
	int destroyWindow(U32 window);

	XPixmapPtr createNewPixmap(U32 width, U32 height, U32 depth, const VisualPtr& visual);
	XPixmapPtr getPixmap(U32 pixmap);
	int removePixmap(U32 pixmap);

	XGCPtr createGC(XDrawablePtr drawable);
	XGCPtr getGC(U32 gc);
	void removeGC(U32 gc);

	XDrawablePtr getDrawable(U32 xid);	
	
	void addCursor(const XCursorPtr& cursor);
	XCursorPtr getCursor(U32 id);
	void updateCursor(const XWindowPtr& wnd);

	void iterateEventMask(U32 wndId, U32 mask, std::function<void(const DisplayDataPtr& data)> callback);
	void iterateInput2Mask(U32 wndId, U32 mask, std::function<void(const DisplayDataPtr& data)> callback);

	U32 openDisplay(KThread* thread);
	int closeDisplay(KThread* thread, const DisplayDataPtr& data);
	DisplayDataPtr getDisplayDataByAddressOfDisplay(KMemory* memory, U32 address);
	DisplayDataPtr getDisplayDataById(U32 id);
	void changeScreen(U32 width, U32 height);
	void processExit(U32 pid);

	void draw(bool drawNow = false);
	const XWindowPtr& getRoot();
	U32 getEventTime();
	U32 getInputModifiers();
	U32 grabPointer(const DisplayDataPtr& display, const XWindowPtr& grabbed, XWindowPtr confined, U32 mask, U32 time);
	U32 ungrabPointer(U32 time);
	U32 setInputFocus(const DisplayDataPtr& data, U32 window, U32 revertTo, U32 time, bool trace = false);
	int mapWindow(const DisplayDataPtr& data, const XWindowPtr& window);
	int unmapWindow(const DisplayDataPtr& data, const XWindowPtr& window);
	U32 createColorMap(const Visual* visual, int alloc);
	XColorMapPtr getDefaultColorMap();
	XColorMapPtr getColorMap(U32 id);
	VisualPtr getVisual(U32 id) {return visuals.get(id);}

	static U32 getNextId();

	XWindowPtr inputFocus;
	U32 inputFocusRevertTo = 0;

	XWindowPtr pointerWindow;
	VisualPtr visual;
#ifdef BOXEDWINE_MULTI_THREADED
	BOXEDWINE_MUTEX mutex;
#else
	BOXEDWINE_CONDITION cond;
#endif
	bool isLocked = false;

	bool trace = false;
	bool traceGC = false;
	bool isDisplayDirty = false;

	CLXFBConfigPtr getFbConfig(U32 id);	
	U32 getFbConfigCount();
	void iterateFbConfigs(std::function<bool(const CLXFBConfigPtr& cfg)> callback);

private:
	static std::atomic_int nextId;
	static XServer* server;

	XWindowPtr root;

	U32 grabbedId;
	U32 grabbedConfinedId;
	U32 grabbedDisplayId;
	BOXEDWINE_MUTEX grabbedMutex;
	bool isGrabbed = false;
	U32 grabbedMask;
	U32 grabbedTime;

	U32 extensionXinput2;
	U32 extensionGLX;

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

	BOXEDWINE_MUTEX colorMapMutex;
	BHashTable<U32, XColorMapPtr> colorMaps;
	XColorMapPtr defaultColorMap;

	// created up front and not modified, so reason to thread protect
	BHashTable<U32, VisualPtr> visuals;
	BHashTable<U32, std::shared_ptr<std::vector<VisualPtr>>> visualsByDepth;
	std::vector<U32> depths;
	BHashTable<U32, CLXFBConfigPtr> fbConfigById;	

	void initAtoms();
	void initDepths();
	void initVisuals();
	void setAtom(const BString& name, U32 key);
	VisualPtr addVisual(U32 redMask, U32 greenMask, U32 blueMask, U32 depth, U32 bitsPerPixel, U32 pixelFormatIndex);
	U32 createScreen(KThread* thread, U32 displayAddress);

	void pointerMoved(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode);
	void pointerMovedBetweenSiblings(const XWindowPtr& from, const XWindowPtr& to, const XWindowPtr& commonAncestor, S32 x, S32 y, U32 mode);
	void pointerMovedChildToParent(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode);
	void pointerMovedParentToChild(const XWindowPtr& from, const XWindowPtr& to, S32 x, S32 y, U32 mode);
};

#endif