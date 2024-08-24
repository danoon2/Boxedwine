#ifndef __XCOLOR_MAP_H__
#define __XCOLOR_MAP_H__

#define COLOR_ALLOCATED 1
#define COLOR_WRITE 2
#define MAX_COLORMAP_SIZE 256

#define RGB(r, g, b) ((((U32)(r)) << 16) | (((U32)(g)) << 8) | (U32)(b))

class XColorMapColor {
public:
	U8 r;
	U8 g;
	U8 b;
	U8 flags;

	BHashTable<U32, U32> uses;
};

class XColorMap {
public:
	XColorMap();

	const U32 id;
	XColorMapColor colors[MAX_COLORMAP_SIZE] = {};
	bool dirty = false;
	U32 nativePixels[MAX_COLORMAP_SIZE];

	void buildCache();
};

typedef std::shared_ptr<XColorMap> XColorMapPtr;

#endif