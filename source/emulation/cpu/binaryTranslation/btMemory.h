#ifndef __BTMEMORY_H__
#define __BTMEMORY_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

class BtCodeChunk;

#define K_MAX_X86_OP_LEN 15

class BtMemory {
public:	
	BtMemory(KMemory* memory);
	~BtMemory();	

	void reserveNativeMemory();
	void releaseNativeMemory();

	void* getExistingHostAddress(U32 eip);
	void* allocateExcutableMemory(U32 size, U32* allocatedSize);
	void freeExcutableMemory(void* hostMemory, U32 size);
	void executableMemoryReleased();
	bool isAddressExecutable(void* address);
	bool isEipPageCommitted(U32 page);
	void setEipPageCommitted(U32 page) { this->committedEipPages[page] = true; }		

	std::shared_ptr<BtCodeChunk> getCodeChunkContainingEip(U32 eip);
	std::shared_ptr<BtCodeChunk> getCodeChunkContainingHostAddress(void* hostAddress);

	void addCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);
	void removeCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);

	BOXEDWINE_MUTEX executableMemoryMutex;

	class AllocatedMemory {
	public:
		AllocatedMemory(void* memory, U32 size) : memory(memory), size(size) {}
		void* memory;
		U32 size;
	};
	std::list<AllocatedMemory> allocatedExecutableMemory;

#define EXECUTABLE_MIN_SIZE_POWER 7
#define EXECUTABLE_MAX_SIZE_POWER 22
#define EXECUTABLE_SIZES 16

	std::unordered_map<U32, std::shared_ptr< std::list< std::shared_ptr<BtCodeChunk> > >> codeChunksByHostPage;
	std::unordered_map<U32, std::shared_ptr< std::list< std::shared_ptr<BtCodeChunk> > >> codeChunksByEmulationPage;

	std::list<void*> freeExecutableMemory[EXECUTABLE_SIZES];		
	KMemory* memory;

	bool committedEipPages[K_NUMBER_OF_PAGES];

	void*** eipToHostInstructionPages;
#ifdef BOXEDWINE_64BIT_MMU	
	void commitHostAddressSpaceMapping(U32 page, U32 pageCount, U64 defaultValue);
	void setEipForHostMapping(U32 eip, void* host);
	void* eipToHostInstructionAddressSpaceMapping;
#endif
	BOXEDWINE_MUTEX mutex;

protected:
	void clearCodePageFromCache(U32 page);
};

#endif
#endif