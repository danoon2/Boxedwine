#ifndef __PLATFORM_OPENGL_H__
#define __PLATFORM_OPENGL_H__

class GLPixelFormat {
public:
	U32 id = 0;
	U32 nativeId = 0;

	PixelFormat pf;
	U32 depth = 0;
	U32 bitsPerPixel = 0;
	bool sampleBuffers = false;
	U32 samples = 0;
	bool pbuffer = false;
	U32 pbufferMaxWidth = 0;
	U32 pbufferMaxHeight = 0;
	U32 pbufferMaxPixels = 0;
};

#define PIXEL_FORMAT_NATIVE_INDEX_MASK 0x80000000

typedef std::shared_ptr<GLPixelFormat> GLPixelFormatPtr;

class XWindow;

class PlatformOpenGL {
public:
	static void init(); // run as soon as app starts, will fill out visuals
	static void iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback);
	static GLPixelFormatPtr getFormat(U32 pixelFormatId);
	static bool hardwareListLoaded;
	static U32 createContext(const GLPixelFormatPtr& format, U32 shareContext, U32 major, U32 minor, U32 profile, U32 flags);
	static void destroyContext(U32 context);
	static void* getNativeContext(U32 id);
	static void makeCurrent(const std::shared_ptr<XWindow>& wnd, U32 contextId);
	static void swapBuffer(const std::shared_ptr<XWindow>& wnd);
	static void createWindow(const std::shared_ptr<XWindow>& wnd);
	static void resize(const std::shared_ptr<XWindow>& wnd);
	static void show(const std::shared_ptr<XWindow>& wnd, bool bShow);
	static void pump();
private:
	static BHashTable<U32, GLPixelFormatPtr> formatsById;
	static std::vector<GLPixelFormatPtr> formats;
	static bool initialized;
};

#endif