#ifndef __X_DEPTH_H__
#define __X_DEPTH_H__

struct Depth {
	S32 depth;		/* this depth (Z) of the depth */
	S32 nvisuals;		/* number of Visual types at this depth */
	VisualPtrAddress visuals;	/* list of visuals possible at this depth */

	void write(KMemory* memory, U32 address);
	void read(KMemory* memory, U32 address);
};

#endif