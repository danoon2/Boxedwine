#ifndef __CODEPAGE_DATA_H__
#define __CODEPAGE_DATA_H__

#define MAX_DYNAMIC_COUNT 0xff

#include "soft_ram.h"

class KMemory;

class DecodedOpPageCache {
public:
	DecodedOpPageCache();
	~DecodedOpPageCache();

	U8* writeCounts = nullptr;
	DecodedOp* ops[K_PAGE_SIZE];
};

class DecodedOpCache {
public:
	DecodedOpCache();
	~DecodedOpCache();

	DecodedOp* get(U32 address);
	DecodedOp** getLocation(U32 address);
	DecodedOp* getPreviousOpAndRemoveIfOverlapping(U32 address, bool* removedOverlappingOp = nullptr);
	void remove(U32 address, U32 len, bool becauseOfWrite);
	void removeAll();
	void removePage(U32 pageIndex);
	void add(DecodedOp* op, U32 address, bool followOpNext);
	void incrementWriteCounts(U32 address, U32 len);
	void markAddressDynamic(U32 address, U32 len);
	bool isAddressDynamic(U32 address, U32 len);

private:
	void removeStartAt(U32 address, U32 len, bool becauseOfWrite);
	DecodedOp* getPreviousOp(U32 address, U32* foundAddress, DecodedOpPageCache** foundPage);
	DecodedOpPageCache* getPageCache(U32 pageIndex, bool create);
	DecodedOpPageCache** pageData[0x400];
};

#endif