#ifndef __BHEAP_H__
#define __BEHAP_H__

#define BHEAP_NUMBER_OF_BUCKETS 9
#define BHEAP_MIN_BUCKET 4

class BHeap {
public:
	BHeap(KMemory* memory) : memory(memory) {}
	~BHeap();

	U32 alloc(KThread* thread, U32 len);
	void free(U32 address);
	void freeAll();
private:
	KMemory* memory;

	std::vector<U32> bucket[BHEAP_NUMBER_OF_BUCKETS];
	std::vector<U32> pages;
};

#endif