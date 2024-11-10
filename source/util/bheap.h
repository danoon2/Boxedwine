#ifndef __BHEAP_H__
#define __BHEAP_H__

#define BHEAP_NUMBER_OF_BUCKETS 9
#define BHEAP_MIN_BUCKET 4

class BHeap {
public:
	U32 alloc(KMemory* memory, KThread* thread, U32 len);
	void free(KMemory* memory, U32 address);
	void freeAll(KMemory* memory);
private:
	std::vector<U32> bucket[BHEAP_NUMBER_OF_BUCKETS];
	std::vector<U32> pages;
};

#endif