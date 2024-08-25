#ifndef __X_VISUAL_INFO_H__
#define __X_VISUAL_INFO_H__

struct XVisualInfo {
	VisualPtrAddress visual;
	VisualID visualid;
	S32 screen;
	S32 depth;
	S32 c_class;
	U32 red_mask;
	U32 green_mask;
	U32 blue_mask;
	S32 colormap_size;
	S32 bits_per_rgb;

	void set(S32 screenIndex, U32 visualAddress, U32 depth, Visual* visual);
	void read(KMemory* memory, U32 address);
	void write(KMemory* memory, U32 address);
	bool match(U32 mask, S32 screenIndex, const Depth* depth, const Visual* visual);
};

#endif