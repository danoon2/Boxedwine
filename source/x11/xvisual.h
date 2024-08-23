#ifndef __X_VISUAL_H__
#define __X_VISUAL_H__

struct Visual {
	XExtDataPtrAddress ext_data;	/* hook for extension to hang data */
	VisualID visualid;	/* visual id of this visual */
	S32 c_class;		/* class of screen (monochrome, etc.) */
	U32 red_mask, green_mask, blue_mask;	/* mask values */
	S32 bits_per_rgb;	/* log base 2 of distinct color values */
	S32 map_entries;	/* color map entries */

	void read(KMemory* memory, U32 address);
	void write(KMemory* memory, U32 address);
};

typedef std::shared_ptr<Visual> VisualPtr;

#endif