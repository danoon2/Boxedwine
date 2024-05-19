#ifndef __BTMEMORY_H__
#define __BTMEMORY_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

class BtCodeChunk;

#define K_MAX_X86_OP_LEN 15

class BtMemory {
public:	
	BtMemory(KMemory* memory);
	~BtMemory();	

	U8* getExistingHostAddress(U32 eip);
	U8* allocateExcutableMemory(U32 size, U32* allocatedSize);
	void freeExcutableMemory(U8* hostMemory, U32 size);
	void executableMemoryReleased();
	bool isAddressExecutable(U8* address);
	bool isEipPageCommitted(U32 page);
	void setEipPageCommitted(U32 page) { this->committedEipPages[page] = true; }		

	std::shared_ptr<BtCodeChunk> getCodeChunkContainingEip(U32 eip);

	void addCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);
	void removeCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);

	BOXEDWINE_MUTEX executableMemoryMutex;

	class AllocatedMemory {
	public:
		AllocatedMemory(U8* memory, U32 size) : memory(memory), size(size) {}
		U8* memory;
		U32 size;
	};
	std::list<AllocatedMemory> allocatedExecutableMemory;

#define EXECUTABLE_MIN_SIZE_POWER 7
#define EXECUTABLE_MAX_SIZE_POWER 22
#define EXECUTABLE_SIZES 16

	std::list<U8*> freeExecutableMemory[EXECUTABLE_SIZES];		
	KMemory* memory;

	bool committedEipPages[K_NUMBER_OF_PAGES];

	U8*** eipToHostInstructionPages;
	BOXEDWINE_MUTEX mutex;

protected:
	void clearCodePageFromCache(U32 page);
};

#endif
#endif