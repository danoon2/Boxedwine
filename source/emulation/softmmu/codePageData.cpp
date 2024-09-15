#include "boxedwine.h"
#include "codePageData.h"

#define FIRST_INDEX_SIZE 0x400
#define SECOND_INDEX_SIZE 0x400
#define GET_FIRST_INDEX_FROM_PAGE(page) (page >> 10)
#define GET_SECOND_INDEX_FROM_PAGE(page) (page & 0x3ff);

CodePerPageData::CodePerPageData() {
	memset(writeCounts, 0, sizeof(writeCounts));
	memset(blocks, 0, sizeof(blocks));
}

static U32 count;

CodePageData::CodePageData() {
	memset(pageData2, 0, sizeof(pageData2));
	count++;
	klog("CodePageData count=%d", count);
}

CodePageData::~CodePageData() {
	removeAll(); 
	count--;
	klog("CodePageData count=%d", count);
}

CodeBlock CodePageData::getBlock(U32 address) {
	CodePerPageData* codePage = getCodePage(address >> K_PAGE_SHIFT, false);
	if (codePage) {
		return codePage->blocks[address & 0xfff];
	}
	return nullptr;
}

#include <unordered_set>
void CodePageData::removeCode(KMemory* memory, U32 address, U32 len, bool becauseOfWrite) {
	std::unordered_set<CodeBlock> blocks;

	memory->iterateAddressByPage(address, len, [&blocks, this, becauseOfWrite](U32 address, U32 len) {
		U32 page = address >> K_PAGE_SHIFT;
		CodePerPageData* codePage = getCodePage(page, false);
		U32 offset = address & K_PAGE_MASK;

		if (codePage) {
			if (becauseOfWrite) {
				codePage->incrementWriteCounts(address, len);
			}
			for (U32 i = 0; i < len; i++) {
				if (codePage->blocks[i + offset]) {
					blocks.insert(codePage->blocks[i + offset]);
				}
			}
		}
	});
	for (CodeBlock block : blocks) {
		memory->iterateAddressByPage(block->getEip(), block->getEipLen(), [&blocks, this, becauseOfWrite](U32 address, U32 len) {
			U32 page = address >> K_PAGE_SHIFT;
			U32 offset = address & K_PAGE_MASK;
			CodePerPageData* codePage = getCodePage(page, false);
			if (codePage) {
				for (U32 i = 0; i < len; i++) {
					codePage->blocks[offset + i] = nullptr;
				}
			}
		});
#ifndef BOXEDWINE_BINARY_TRANSLATOR
		block->dealloc(false);
#endif
	}
}

void CodePageData::removeAll() {
	for (U32 firstIndex = 0; firstIndex < FIRST_INDEX_SIZE; firstIndex++) {
		if (pageData2[firstIndex]) {
			for (U32 secondIndex = 0; secondIndex < SECOND_INDEX_SIZE; secondIndex++) {
				delete pageData2[firstIndex][secondIndex];
			}
			pageData2[firstIndex] = nullptr;
		}
	}
}

void CodePageData::addCodeToPage(CodeBlock block, U32 page, U32 address, U32 len) {
	CodePerPageData* data = getCodePage(page, true);
	U32 offset = address & K_PAGE_MASK;
	for (U32 i = 0; i < len ; i++) {
		data->blocks[offset+i] = block;
	}
}

void CodePerPageData::incrementWriteCounts(U32 address, U32 len) {
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	if (end > K_PAGE_SIZE) {
		end = K_PAGE_SIZE;
	}
	for (U32 i = offset; i < end; i++) {
		if (writeCounts[i] < MAX_DYNAMIC_COUNT) {
			writeCounts[i]++;
		}
	}
}

void CodePerPageData::markAddressDynamic(U32 address, U32 len) {
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	if (end > K_PAGE_SIZE) {
		end = K_PAGE_SIZE;
	}
	for (U32 i = offset; i < end; i++) {
		writeCounts[i] = MAX_DYNAMIC_COUNT;
	}
}

bool CodePerPageData::isAddressDynamic(U32 address, U32 len) {
	U32 offset = address & K_PAGE_MASK;
	U32 end = offset + len;
	if (end > K_PAGE_SIZE) {
		end = K_PAGE_SIZE;
	}
	for (U32 i = offset; i < end; i++) {
		if (writeCounts[i] == MAX_DYNAMIC_COUNT) {
			return true;
		}
	}
	return false;
}

CodePerPageData* CodePageData::getCodePage(U32 page, bool create) {
	U32 firstIndex = GET_FIRST_INDEX_FROM_PAGE(page);
	U32 secondIndex = GET_SECOND_INDEX_FROM_PAGE(page);
	CodePerPageData** first = pageData2[firstIndex];
	CodePerPageData* result = nullptr;
	if (first) {
		result = first[secondIndex];
	}
	if (!result && create) {
		BOXEDWINE_CRITICAL_SECTION;
		if (!first) {
			first = pageData2[firstIndex];
		}
		if (!first) {
			pageData2[firstIndex] = new CodePerPageData* [SECOND_INDEX_SIZE];
			memset(pageData2[firstIndex], 0, sizeof(CodePerPageData*) * SECOND_INDEX_SIZE);
		}
		if (!result) {
			result = new CodePerPageData();
			pageData2[firstIndex][secondIndex] = result;
		}
	}
	return result;
}