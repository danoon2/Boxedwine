#ifndef __CODE_PAGE_DATA_H__
#define __CODE_PAGE_DATA_H__

#define MAX_DYNAMIC_COUNT 0xff

#include <map>

class CodePerPageData {
public:
	CodePerPageData();	

	void incrementWriteCounts(U32 address, U32 len);
	void markAddressDynamic(U32 address, U32 len);
	bool isAddressDynamic(U32 address, U32 len);

	U8 writeCounts[K_PAGE_SIZE];
	CodeBlock blocks[K_PAGE_SIZE];
};

class CodePageData {
public:
	CodePageData();
	~CodePageData();

	CodePerPageData* getCodePage(U32 page, bool create);
	CodeBlock getBlock(U32 address);
	CodeBlock getBlock(U32 address, U32 len);
	void removeCode(KMemory* memory, U32 address, U32 len, bool becauseOfWrite);
	void removeAll();
	void addCodeToPage(CodeBlock block, U32 page, U32 address, U32 len);

private:
	CodePerPageData** pageData2[0x400];
};

#endif