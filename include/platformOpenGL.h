#ifndef __PLATFORM_OPENGL_H__
#define __PLATFORM_OPENGL_H__

class GLPixelFormat {
public:
	U32 id;
	U32 nativeId;

	PixelFormat pf;
	bool sampleBuffers = false;
	U32 samples = 0;
	bool pbuffer = false;
	U32 pbufferMaxWidth = 0;
	U32 pbufferMaxHeight = 0;
	U32 pbufferMaxPixels = 0;
};

#define PIXEL_FORMAT_NATIVE_INDEX_MASK 0x80000000

typedef std::shared_ptr<GLPixelFormat> GLPixelFormatPtr;

class PlatformOpenGL {
public:
	static void init();
	static void iterateFormats(std::function<void(const GLPixelFormatPtr& format)> callback);
	static GLPixelFormatPtr getFormat(U32 pixelFormatId);
	static bool hardwareListLoaded;

private:
	static BHashTable<U32, GLPixelFormatPtr> formatsById;
	static std::vector<GLPixelFormatPtr> formats;
	static bool initialized;
};

#endif