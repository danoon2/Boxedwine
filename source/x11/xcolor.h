#ifndef __X_COLOR_H__
#define __X_COLOR_H__

struct XColor {
	U32 pixel;
	U16 red, green, blue;
	U8 flags;  /* do_red, do_green, do_blue */
	U8 pad;

	void read(KMemory* memory, U32 address);
};

#endif