/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "soft_ram.h"

#if defined(_WIN32) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
#define BOXEDWINE_WINDOWS_LINEAR_MEMORY
#include <windows.h>
#endif

#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
#ifdef __linux__
#include <linux/memfd.h>
#include <sys/syscall.h>
#elif defined(__APPLE__)
#include <fcntl.h>
#endif
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_set>
#endif

#define NATIVE_PAGE_REF_COUNT 0

static BOXEDWINE_MUTEX ramMutex;
int allocatedRamPages;

class RamInfo {
public:
    U16 refCount : 14;
    U16 isNative : 1;
    U16 isSystem : 1;
#ifdef BOXEDWINE_WINDOWS_LINEAR_MEMORY
    U32 linearMemorySlot;
#endif
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    std::shared_ptr<LinearMemoryBacking> linearMemoryBacking;
#endif
};

static std::unordered_map<U64, RamInfo> refCounts;
static std::vector<RAM_TYPE> freeIndexes;

enum class LinearMemoryMode {
    Disabled,
    SingleOffset4K,
    GroupedHostPage,
};

static LinearMemoryMode linearMemoryMode = LinearMemoryMode::Disabled;
static U32 linearMemoryPageCount = 1;

#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
static int linearRamFile = -1;
static U8* linearRam = nullptr;
static U32 linearRamPageCount;
static bool linearRamInitializationAttempted;
static std::vector<bool> freeLinearRamPages(K_NUMBER_OF_PAGES);
static std::unordered_set<U32> freeLinearRamBlocks;
static std::atomic<U32> nextLinearMemoryBackingId{1};

static int createLinearMemoryFile(const char* description, U32 id) {
#ifdef __linux__
    (void)id;
    return (int)syscall(SYS_memfd_create, description, MFD_CLOEXEC);
#else
    (void)description;
    char name[64];
    snprintf(name, sizeof(name), "/bw-%x-%x", (U32)getpid(), id);
    int result = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (result < 0) {
        return -1;
    }
    if (shm_unlink(name) != 0 || fcntl(result, F_SETFD, FD_CLOEXEC) != 0) {
        int error = errno;
        close(result);
        errno = error;
        return -1;
    }
    return result;
#endif
}

class LinearMemoryBacking {
public:
    LinearMemoryBacking() {
        id = nextLinearMemoryBackingId.fetch_add(1, std::memory_order_relaxed);
        if (!id || id >= (1U << 29)) {
            kpanic("linear memory: backing id exhausted");
        }
    }

    bool initialize() {
        file = createLinearMemoryFile("kmemory", id);
        if (file < 0) {
            kwarn_fmt("linear memory: KMemory shared-memory creation failed, using legacy MMU: %s", strerror(errno));
            return false;
        }
        if (ftruncate(file, (off_t)1 << 32) != 0) {
            int error = errno;
            close(file);
            file = -1;
            kwarn_fmt("linear memory: KMemory ftruncate failed, using legacy MMU: %s", strerror(error));
            return false;
        }
        address = (U8*)mmap(nullptr, (U64)1 << 32, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, file, 0);
        if (address == MAP_FAILED) {
            int error = errno;
            address = nullptr;
            close(file);
            file = -1;
            kwarn_fmt("linear memory: KMemory backing mmap failed, using legacy MMU: %s", strerror(error));
            return false;
        }
        return true;
    }

    ~LinearMemoryBacking() {
        if (address) {
            munmap(address, (U64)1 << 32);
        }
        if (file >= 0) {
            close(file);
        }
    }

    int file = -1;
    U8* address = nullptr;
    U32 id = 0;
};

static bool initializeLinearRam() {
    if (linearRam) {
        return true;
    }
    if (linearRamInitializationAttempted) {
        return false;
    }
    linearRamInitializationAttempted = true;
    linearRamFile = createLinearMemoryFile("linear-ram", 0);
    if (linearRamFile < 0) {
        kwarn_fmt("linear memory: shared-memory creation failed: %s", strerror(errno));
        return false;
    }
    if (ftruncate(linearRamFile, (off_t)1 << 32) != 0) {
        int error = errno;
        close(linearRamFile);
        linearRamFile = -1;
        kwarn_fmt("linear memory: ftruncate failed: %s", strerror(error));
        return false;
    }
    linearRam = (U8*)mmap(nullptr, (U64)1 << 32, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_NORESERVE, linearRamFile, 0);
    if (linearRam == MAP_FAILED) {
        int error = errno;
        linearRam = nullptr;
        close(linearRamFile);
        linearRamFile = -1;
        kwarn_fmt("linear memory: RAM pool mmap failed: %s", strerror(error));
        return false;
    }
    return true;
}

static bool isLinearRamPage(RamPage page) {
    if (!linearRam || !page.value) {
        return false;
    }
    U8* address = ramPageGet(page);
    return address >= linearRam && (U64)(address - linearRam) < ((U64)linearRamPageCount << K_PAGE_SHIFT);
}

static U32 getLinearRamPage(RamPage page) {
    return (U32)((ramPageGet(page) - linearRam) >> K_PAGE_SHIFT);
}

static void setLinearRamPageAllocated(RamPage page) {
    if (!isLinearRamPage(page)) {
        return;
    }
    U32 pageIndex = getLinearRamPage(page);
    freeLinearRamPages[pageIndex] = false;
    U32 pageCount = ramPageLinearMemoryPageCount();
    if (pageCount > 1) {
        freeLinearRamBlocks.erase(pageIndex & ~(pageCount - 1));
    }
}

static void setLinearRamPageFree(RamPage page) {
    if (!isLinearRamPage(page)) {
        return;
    }
    U32 pageIndex = getLinearRamPage(page);
    freeLinearRamPages[pageIndex] = true;
    U32 pageCount = ramPageLinearMemoryPageCount();
    if (pageCount <= 1) {
        return;
    }
    U32 firstPage = pageIndex & ~(pageCount - 1);
    for (U32 i = 0; i < pageCount; i++) {
        if (!freeLinearRamPages[firstPage + i]) {
            return;
        }
    }
    freeLinearRamBlocks.insert(firstPage);
}
#else
static void setLinearRamPageAllocated(RamPage page) {
    (void)page;
}

static void setLinearRamPageFree(RamPage page) {
    (void)page;
}
#endif

#ifdef BOXEDWINE_WINDOWS_LINEAR_MEMORY
using VirtualAlloc2Function = PVOID(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER*, ULONG);
using MapViewOfFile3Function = PVOID(WINAPI*)(HANDLE, HANDLE, PVOID, ULONG64, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER*, ULONG);
using UnmapViewOfFile2Function = BOOL(WINAPI*)(HANDLE, PVOID, ULONG);

static constexpr U64 WINDOWS_LINEAR_MEMORY_SLOT_SIZE = 64 * 1024;
static VirtualAlloc2Function linearMemoryVirtualAlloc2;
static MapViewOfFile3Function linearMemoryMapViewOfFile3;
static UnmapViewOfFile2Function linearMemoryUnmapViewOfFile2;
static HANDLE windowsLinearRamFile;
static U32 windowsLinearRamSlotCount;
static bool windowsLinearRamInitializationAttempted;

static FARPROC getWindowsLinearMemoryFunction(const char* name) {
    HMODULE module = GetModuleHandleW(L"KernelBase.dll");
    return module ? GetProcAddress(module, name) : nullptr;
}

static bool initializeWindowsLinearMemory() {
    if (windowsLinearRamFile) {
        return true;
    }
    if (windowsLinearRamInitializationAttempted) {
        return false;
    }
    windowsLinearRamInitializationAttempted = true;
    linearMemoryVirtualAlloc2 = reinterpret_cast<VirtualAlloc2Function>(getWindowsLinearMemoryFunction("VirtualAlloc2"));
    linearMemoryMapViewOfFile3 = reinterpret_cast<MapViewOfFile3Function>(getWindowsLinearMemoryFunction("MapViewOfFile3"));
    linearMemoryUnmapViewOfFile2 = reinterpret_cast<UnmapViewOfFile2Function>(getWindowsLinearMemoryFunction("UnmapViewOfFile2"));
    if (!linearMemoryVirtualAlloc2 || !linearMemoryMapViewOfFile3 || !linearMemoryUnmapViewOfFile2) {
        kwarn("linear memory: Windows placeholder APIs are unavailable");
        return false;
    }
    U64 size = (U64)K_NUMBER_OF_PAGES * WINDOWS_LINEAR_MEMORY_SLOT_SIZE;
    windowsLinearRamFile = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_RESERVE, (DWORD)(size >> 32), (DWORD)size, nullptr);
    if (!windowsLinearRamFile) {
        kwarn_fmt("linear memory: Windows RAM section creation failed: %u", GetLastError());
        return false;
    }
    return true;
}

static U8* reserveWindowsLinearMemory(U64 size) {
    return (U8*)linearMemoryVirtualAlloc2(GetCurrentProcess(), nullptr, (SIZE_T)size, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0);
}

static void mapWindowsLinearMemoryPage(U8* target, U32 slot, bool commit) {
    DWORD splitError = ERROR_SUCCESS;
    if (!VirtualFree(target, K_PAGE_SIZE, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER)) {
        // A page which was mapped before is already an exact placeholder after
        // UnmapViewOfFile2. Mapping it directly is valid, so only report this
        // split error if replacement also fails.
        splitError = GetLastError();
    }
    void* result = linearMemoryMapViewOfFile3(windowsLinearRamFile, GetCurrentProcess(), target, (U64)slot * WINDOWS_LINEAR_MEMORY_SLOT_SIZE, K_PAGE_SIZE, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, nullptr, 0);
    if (result != target) {
        DWORD mappingError = GetLastError();
        kpanic_fmt("linear memory: Windows page split/map failed: %u/%u", splitError, mappingError);
    }
    if (commit && VirtualAlloc(target, K_PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE) != target) {
        kpanic_fmt("linear memory: Windows RAM page commit failed: %u", GetLastError());
    }
}

static bool resetWindowsLinearMemory(U8* address, U64 size) {
    U8* current = address;
    U8* end = address + size;
    while (current < end) {
        MEMORY_BASIC_INFORMATION info;
        if (!VirtualQuery(current, &info, sizeof(info))) {
            return false;
        }
        U8* next = (U8*)info.BaseAddress + info.RegionSize;
        if (info.Type == MEM_MAPPED && !linearMemoryUnmapViewOfFile2(GetCurrentProcess(), info.BaseAddress, MEM_PRESERVE_PLACEHOLDER)) {
            return false;
        }
        if (next <= current) {
            return false;
        }
        current = next;
    }
    if (VirtualFree(address, (SIZE_T)size, MEM_RELEASE | MEM_COALESCE_PLACEHOLDERS)) {
        return true;
    }
    MEMORY_BASIC_INFORMATION info;
    return VirtualQuery(address, &info, sizeof(info)) && info.State == MEM_RESERVE && info.BaseAddress == address && info.RegionSize >= size;
}

static void releaseWindowsLinearMemory(U8* address, U64 size) {
    if (!resetWindowsLinearMemory(address, size)) {
        kpanic_fmt("linear memory: Windows placeholder reset failed: %u", GetLastError());
    }
    if (!VirtualFree(address, 0, MEM_RELEASE)) {
        kpanic_fmt("linear memory: Windows placeholder release failed: %u", GetLastError());
    }
}
#endif

void ramPageConfigureLinearMemory(bool disabled) {
    linearMemoryMode = LinearMemoryMode::Disabled;
    linearMemoryPageCount = 1;
#if ((defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))) || defined(BOXEDWINE_WINDOWS_LINEAR_MEMORY)
    if (disabled) {
        klog("Linear memory disabled; using legacy guarded MMU cache");
        return;
    }
    U32 pageCount = Platform::getPagePermissionGranularity();
#ifdef BOXEDWINE_WINDOWS_LINEAR_MEMORY
    U32 allocationPageCount = Platform::getPageAllocationGranularity();
    if (pageCount == 1 && allocationPageCount == 16) {
        if (initializeWindowsLinearMemory()) {
            linearMemoryMode = LinearMemoryMode::SingleOffset4K;
            klog("Using single-offset linear memory for Windows 4K pages");
        } else {
            kwarn("Linear memory initialization failed; using legacy guarded MMU cache");
        }
    } else {
        kwarn_fmt("Unsupported Windows %uK permission/%uK allocation granularity; using legacy guarded MMU cache", pageCount * 4, allocationPageCount * 4);
    }
#else
    if (pageCount == 1) {
        if (initializeLinearRam()) {
            linearMemoryMode = LinearMemoryMode::SingleOffset4K;
            klog("Using single-offset linear memory for 4K host pages");
        } else {
            kwarn("Linear memory initialization failed; using legacy guarded MMU cache");
        }
    } else if (pageCount == 4 || pageCount == 16) {
        linearMemoryMode = LinearMemoryMode::GroupedHostPage;
        linearMemoryPageCount = pageCount;
        klog_fmt("Using grouped single-offset linear memory for %uK host pages", pageCount * 4);
    } else {
        kwarn_fmt("Unsupported %uK host page size; using legacy guarded MMU cache", pageCount * 4);
    }
#endif
#else
    (void)disabled;
#endif
}

bool ramPageUseLinearMemory() {
    return linearMemoryMode != LinearMemoryMode::Disabled;
}

bool ramPageUseLinearMemoryAdjacent() {
    return linearMemoryMode == LinearMemoryMode::GroupedHostPage;
}

bool ramPageUseLinearMemoryBacking() {
    return linearMemoryMode == LinearMemoryMode::GroupedHostPage;
}

bool ramPageUseLinearMemoryFileCacheBlock() {
    return ramPageUseLinearMemoryBacking();
}

std::shared_ptr<LinearMemoryBacking> ramPageCreateLinearMemoryBacking() {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (ramPageUseLinearMemoryBacking()) {
        try {
            std::shared_ptr<LinearMemoryBacking> result = std::make_shared<LinearMemoryBacking>();
            if (result->initialize()) {
                return result;
            }
        } catch (const std::bad_alloc&) {
            kwarn("linear memory: KMemory backing allocation failed, using legacy MMU");
        }
    }
#endif
    return nullptr;
}

U32 ramPageLinearMemoryPageCount() {
    return linearMemoryPageCount;
}

U64 ramPageLinearMemoryApertureSize() {
    return (U64)1 << 32;
}

static U64 getLinearMemoryDataSize(U64 dataSize) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    U64 pageSize = (U64)sysconf(_SC_PAGESIZE);
    return (dataSize + pageSize - 1) & ~(pageSize - 1);
#else
    return dataSize;
#endif
}

U8* ramPageReserveLinearMemoryData(U64 dataSize, U8** linearMemoryAddress) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (!ramPageUseLinearMemoryAdjacent() || !linearMemoryAddress) {
        return nullptr;
    }
    U64 prefixSize = getLinearMemoryDataSize(dataSize);
    U64 totalSize = prefixSize + ramPageLinearMemoryApertureSize();
    U8* result = (U8*)mmap(nullptr, totalSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (result == MAP_FAILED) {
        kwarn_fmt("linear memory: adjacent reservation failed, using legacy MMU: %s", strerror(errno));
        return nullptr;
    }
    if (mprotect(result, prefixSize, PROT_READ | PROT_WRITE) != 0) {
        int error = errno;
        munmap(result, totalSize);
        kwarn_fmt("linear memory: adjacent data mapping failed, using legacy MMU: %s", strerror(error));
        return nullptr;
    }
    *linearMemoryAddress = result + prefixSize;
    return result;
#else
    return nullptr;
#endif
}

void ramPageReleaseLinearMemoryData(U8* data, U64 dataSize) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (data) {
        U64 prefixSize = getLinearMemoryDataSize(dataSize);
        munmap(data, prefixSize + ramPageLinearMemoryApertureSize());
    }
#endif
}

U8* ramPageReserveLinearMemory() {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (!ramPageUseLinearMemory()) {
        return nullptr;
    }
    U8* result = (U8*)mmap(nullptr, ramPageLinearMemoryApertureSize(), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (result == MAP_FAILED) {
        kwarn_fmt("linear memory: aperture reservation failed, using legacy MMU: %s", strerror(errno));
        return nullptr;
    }
    return result;
#elif defined(BOXEDWINE_WINDOWS_LINEAR_MEMORY)
    if (!ramPageUseLinearMemory()) {
        return nullptr;
    }
    U8* result = reserveWindowsLinearMemory(ramPageLinearMemoryApertureSize());
    if (!result) {
        kwarn_fmt("linear memory: Windows aperture reservation failed, using legacy MMU: %u", GetLastError());
    }
    return result;
#else
    return nullptr;
#endif
}

void ramPageReleaseLinearMemory(U8* address) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (address) {
        munmap(address, ramPageLinearMemoryApertureSize());
    }
#elif defined(BOXEDWINE_WINDOWS_LINEAR_MEMORY)
    if (address) {
        releaseWindowsLinearMemory(address, ramPageLinearMemoryApertureSize());
    }
#endif
}

void ramPageResetLinearMemory(U8* address) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (!address) {
        return;
    }
    U64 size = ramPageLinearMemoryApertureSize();
    void* result = mmap(address, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_NORESERVE, -1, 0);
    if (result == MAP_FAILED) {
        kpanic_fmt("linear memory: aperture reset failed: %s", strerror(errno));
    }
#elif defined(BOXEDWINE_WINDOWS_LINEAR_MEMORY)
    if (address && !resetWindowsLinearMemory(address, ramPageLinearMemoryApertureSize())) {
        kpanic_fmt("linear memory: Windows aperture reset failed: %u", GetLastError());
    }
#endif
}

void ramPageRemoveLinearMemory(U8* address, U32 guestPage, U32 pageCount) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (!address || !pageCount) {
        return;
    }
    U8* target = address + ((U64)guestPage << K_PAGE_SHIFT);
    size_t length = (size_t)pageCount << K_PAGE_SHIFT;
    void* result = mmap(target, length, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_NORESERVE, -1, 0);
    if (result == MAP_FAILED) {
        kpanic_fmt("linear memory: aperture removal failed: %s", strerror(errno));
    }
#elif defined(BOXEDWINE_WINDOWS_LINEAR_MEMORY)
    if (!address || !pageCount) {
        return;
    }
    U8* target = address + ((U64)guestPage << K_PAGE_SHIFT);
    for (U32 i = 0; i < pageCount; i++) {
        if (!linearMemoryUnmapViewOfFile2(GetCurrentProcess(), target + ((U64)i << K_PAGE_SHIFT), MEM_PRESERVE_PLACEHOLDER)) {
            kpanic_fmt("linear memory: Windows aperture page removal failed: %u", GetLastError());
        }
    }
#endif
}

U32 ramPageUpdateLinearMemory(U8* address, U32 guestPage, const RamPage* pages, U32 pageCount,
    bool canRead, bool canWrite, U32 currentMapping) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (!address || !pages || !pageCount) {
        return 0;
    }
    U32 newMapping = 0;
    std::shared_ptr<LinearMemoryBacking> processBacking;
    if (ramPageUseLinearMemoryBacking()) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
        auto first = refCounts.find(pages[0].value);
        if (first != refCounts.end()) {
            processBacking = first->second.linearMemoryBacking;
        }
        for (U32 i = 0; processBacking && i < pageCount; i++) {
            auto it = refCounts.find(pages[i].value);
            U8* expected = processBacking->address + ((U64)(guestPage + i) << K_PAGE_SHIFT);
            if (it == refCounts.end() || it->second.linearMemoryBacking != processBacking || ramPageGet(pages[i]) != expected) {
                processBacking.reset();
            }
        }
        if (processBacking && (canRead || canWrite)) {
            // The high bit distinguishes per-KMemory backing generations from
            // the global file-cache pool in the compact mapping signature.
            newMapping = 0x80000000U | (processBacking->id << 2) | (canRead ? 1 : 0) | (canWrite ? 2 : 0);
        }
    }
    // Grouped mappings normally use their per-KMemory backing, but aligned
    // file-cache blocks remain in the global pool so all processes see the
    // same coherent pages.
    bool contiguous = !processBacking && isLinearRamPage(pages[0]);
    U32 poolPage = 0;
    if (contiguous) {
        poolPage = (U32)((ramPageGet(pages[0]) - linearRam) >> K_PAGE_SHIFT);
        contiguous = (poolPage & (pageCount - 1)) == 0;
    }
    for (U32 i = 1; contiguous && i < pageCount; i++) {
        contiguous = isLinearRamPage(pages[i]) && ramPageGet(pages[i]) == ramPageGet(pages[0]) + ((U64)i << K_PAGE_SHIFT);
    }
    if (contiguous && (canRead || canWrite)) {
        // The pool contains at most 2^20 pages, leaving two low bits for the
        // read/write protection state in a compact 32-bit mapping signature.
        newMapping = (poolPage << 2) | (canRead ? 1 : 0) | (canWrite ? 2 : 0);
    }
    if (newMapping == currentMapping) {
        return newMapping;
    }
    U8* target = address + ((U64)guestPage << K_PAGE_SHIFT);
    size_t length = (size_t)pageCount << K_PAGE_SHIFT;
    if (newMapping) {
        int protection = (canRead ? PROT_READ : 0) | (canWrite ? PROT_WRITE : 0);
        off_t offset = processBacking ? (off_t)guestPage << K_PAGE_SHIFT
                                      : (off_t)(ramPageGet(pages[0]) - linearRam);
        int file = processBacking ? processBacking->file : linearRamFile;
        void* result = mmap(target, length, protection, MAP_SHARED | MAP_FIXED, file, offset);
        if (result == MAP_FAILED) {
            kpanic_fmt("linear memory: page mmap failed: %s", strerror(errno));
        }
        return newMapping;
    }
    void* result = mmap(target, length, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_NORESERVE, -1, 0);
    if (result == MAP_FAILED) {
        kpanic_fmt("linear memory: page removal failed: %s", strerror(errno));
    }
    return 0;
#elif defined(BOXEDWINE_WINDOWS_LINEAR_MEMORY)
    if (!address || !pages || pageCount != 1) {
        return 0;
    }
    U32 slot = 0;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
        auto it = refCounts.find(pages[0].value);
        if (it != refCounts.end()) {
            slot = it->second.linearMemorySlot;
        }
    }
    canWrite = canRead && canWrite;
    U32 newMapping = slot && (canRead || canWrite) ? (slot << 2) | (canRead ? 1 : 0) | (canWrite ? 2 : 0) : 0;
    if (newMapping == currentMapping) {
        return newMapping;
    }
    U8* target = address + ((U64)guestPage << K_PAGE_SHIFT);
    if (newMapping && currentMapping && (newMapping >> 2) == (currentMapping >> 2)) {
        DWORD oldProtection;
        DWORD protection = canWrite ? PAGE_READWRITE : PAGE_READONLY;
        if (!VirtualProtect(target, K_PAGE_SIZE, protection, &oldProtection)) {
            kpanic_fmt("linear memory: Windows page protection failed: %u", GetLastError());
        }
        return newMapping;
    }
    if (currentMapping && !linearMemoryUnmapViewOfFile2(GetCurrentProcess(), target, MEM_PRESERVE_PLACEHOLDER)) {
        kpanic_fmt("linear memory: Windows page unmap failed: %u", GetLastError());
    }
    if (newMapping) {
        mapWindowsLinearMemoryPage(target, slot - 1, false);
        DWORD oldProtection;
        DWORD protection = canWrite ? PAGE_READWRITE : PAGE_READONLY;
        if (!VirtualProtect(target, K_PAGE_SIZE, protection, &oldProtection)) {
            kpanic_fmt("linear memory: Windows page protection failed: %u", GetLastError());
        }
    }
    return newMapping;
#else
    return 0;
#endif
}

bool ramPageAllocLinearMemoryBlock(RamPage* pages, U32 pageCount) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (!pages || pageCount <= 1 || pageCount != ramPageLinearMemoryPageCount()) {
        return false;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    if (!initializeLinearRam()) {
        return false;
    }
    U32 firstPage;
    if (freeLinearRamBlocks.empty()) {
        U32 oldPageCount = linearRamPageCount;
        firstPage = (linearRamPageCount + pageCount - 1) & ~(pageCount - 1);
        if (firstPage > K_NUMBER_OF_PAGES - pageCount) {
            return false;
        }
        linearRamPageCount = firstPage + pageCount;
        for (U32 i = oldPageCount; i < firstPage; i++) {
            freeLinearRamPages[i] = true;
            freeIndexes.push_back((RAM_TYPE)(linearRam + ((U64)i << K_PAGE_SHIFT)) >> K_PAGE_SHIFT);
        }
    } else {
        auto it = freeLinearRamBlocks.begin();
        firstPage = *it;
        freeLinearRamBlocks.erase(it);
    }
    for (U32 i = 0; i < pageCount; i++) {
        U8* result = linearRam + ((U64)(firstPage + i) << K_PAGE_SHIFT);
        pages[i].value = (RAM_TYPE)result >> K_PAGE_SHIFT;
        freeLinearRamPages[firstPage + i] = false;
        refCounts[pages[i].value].refCount = 1;
        allocatedRamPages++;
    }
    return true;
#else
    return false;
#endif
}

bool ramPageAllocLinearMemoryBackingBlock(std::shared_ptr<LinearMemoryBacking>& backing,
    U32 guestPage, RamPage* pages, U32 pageCount) {
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (!ramPageUseLinearMemoryBacking() || !pages || pageCount <= 1 || pageCount != ramPageLinearMemoryPageCount() || (guestPage & (pageCount - 1))) {
        return false;
    }
    while (true) {
        if (!backing) {
            backing = ramPageCreateLinearMemoryBacking();
        }
        std::shared_ptr<LinearMemoryBacking> candidate = backing;
        if (!candidate) {
            return false;
        }
        bool available = true;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
            for (U32 i = 0; i < pageCount; i++) {
                RAM_TYPE value = (RAM_TYPE)(candidate->address + ((U64)(guestPage + i) << K_PAGE_SHIFT)) >> K_PAGE_SHIFT;
                auto it = refCounts.find(value);
                if (it != refCounts.end() && it->second.refCount) {
                    available = false;
                    break;
                }
            }
            if (available) {
                for (U32 i = 0; i < pageCount; i++) {
                    U8* address = candidate->address + ((U64)(guestPage + i) << K_PAGE_SHIFT);
                    pages[i].value = (RAM_TYPE)address >> K_PAGE_SHIFT;
                    RamInfo& info = refCounts[pages[i].value];
                    info.refCount = 1;
                    info.isNative = 0;
                    info.isSystem = 0;
                    info.linearMemoryBacking = candidate;
                    allocatedRamPages++;
                }
            }
        }
        if (available) {
            memset(ramPageGet(pages[0]), 0, (size_t)pageCount << K_PAGE_SHIFT);
            return true;
        }
        // A fork can keep an old page alive after this KMemory unmaps the same
        // guest address. Start a new sparse backing generation rather than
        // overwriting the child's page at that fixed file offset.
        backing = ramPageCreateLinearMemoryBacking();
    }
#else
    (void)backing;
    (void)guestPage;
    (void)pages;
    (void)pageCount;
    return false;
#endif
}

struct alignas(K_PAGE_SIZE) AlignedU8 {
    U8 data;
};

std::vector<U8*> allocatedPages4k;
std::vector<U8*> allocatedPages4kChunk;
std::vector<AlignedU8*> allocatedPages;
#define CHUNK_SIZE_4K 8

RamPage ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    while (freeIndexes.size()) {
        RamPage found;
        found.value = freeIndexes.back();
        freeIndexes.pop_back();
        if (refCounts[found.value].refCount) {
            continue;
        }
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
        // In grouped mode the global pool is reserved for atomic file-cache
        // blocks. Ordinary pages use the per-KMemory backing (or the normal
        // allocator) so they cannot split a free cache block.
        if (ramPageUseLinearMemoryBacking() && isLinearRamPage(found)) {
            continue;
        }
#endif
        setLinearRamPageAllocated(found);
        refCounts[found.value].refCount = 1;
        memset((U8*)(found.value << K_PAGE_SHIFT), 0, K_PAGE_SIZE);
        allocatedRamPages++;
        return found;
    }
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (ramPageUseLinearMemory() && !ramPageUseLinearMemoryBacking()) {
        if (initializeLinearRam() && linearRamPageCount < K_NUMBER_OF_PAGES) {
            U8* result = linearRam + ((U64)linearRamPageCount++ << K_PAGE_SHIFT);
            RamPage found;
            found.value = (RAM_TYPE)result >> K_PAGE_SHIFT;
            refCounts[found.value].refCount = 1;
            memset(result, 0, K_PAGE_SIZE);
            allocatedRamPages++;
            return found;
        }
    }
#endif
    if (KSystem::canJitUse4KPage) {
        U8* chunk;
#ifdef BOXEDWINE_WINDOWS_LINEAR_MEMORY
        bool useWindowsLinearMemory = ramPageUseLinearMemory();
        chunk = useWindowsLinearMemory ? reserveWindowsLinearMemory(CHUNK_SIZE_4K * 64 * 1024) : (U8*)Platform::reserveNativeMemory64k(CHUNK_SIZE_4K);
        if (!chunk) {
            kpanic_fmt("linear memory: Windows guarded RAM reservation failed: %u", GetLastError());
        }
#else
        chunk = (U8*)Platform::reserveNativeMemory64k(CHUNK_SIZE_4K);
#endif
        allocatedPages4kChunk.push_back(chunk);
        allocatedPages4k.push_back(chunk);

        U8* result = chunk;
        U8* pages = chunk;
        U32 count = (CHUNK_SIZE_4K * 16 / 2) - 1; // -1 so that there is an uncommitted page at the end
        for (U32 i = 0; i < count; i++) {
            pages += K_PAGE_SIZE; // keep uncommitted page between each committed so that if a read/write crosses a page boundry it will generate an exception
#ifdef BOXEDWINE_WINDOWS_LINEAR_MEMORY
            if (useWindowsLinearMemory) {
                if (windowsLinearRamSlotCount >= K_NUMBER_OF_PAGES) {
                    kpanic("linear memory: Windows RAM section exhausted");
                }
                U32 slot = windowsLinearRamSlotCount++;
                mapWindowsLinearMemoryPage(pages, slot, true);
                RAM_TYPE index = ((RAM_TYPE)pages) >> K_PAGE_SHIFT;
                refCounts[index].linearMemorySlot = slot + 1;
            } else {
                Platform::commitNativeMemoryPage(pages);
            }
#else
            Platform::commitNativeMemoryPage(pages);
#endif
            if (i == 0) {
                result = pages;
            } else {
                RAM_TYPE index = ((RAM_TYPE)pages) >> K_PAGE_SHIFT;
                freeIndexes.push_back(index);
                allocatedRamPages++;
            }
            pages += K_PAGE_SIZE;
        }
        RAM_TYPE index = ((RAM_TYPE)result) >> K_PAGE_SHIFT;

        memset(result, 0, K_PAGE_SIZE);
        RamPage found;
        found.value = index;

        refCounts[index].refCount = 1;
        allocatedRamPages++;
        return found;
    } else {
        AlignedU8* result = new AlignedU8[64]; // need to create a few since an aligned new will over allocate to make the alignment work.
        allocatedPages.push_back(result);
        RAM_TYPE index = ((RAM_TYPE)result) >> K_PAGE_SHIFT;
        for (int i = 1; i < 64; i++) {
            allocatedRamPages++;
            freeIndexes.push_back(index + i);
        }

        memset(result, 0, K_PAGE_SIZE);
        RamPage found;
        found.value = index;

        refCounts[index].refCount = 1;
        allocatedRamPages++;
        return found;
    }
}

RamPage ramPageAllocNative(U8* native) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    RAM_TYPE index = ((RAM_TYPE)native) >> K_PAGE_SHIFT;

    if (((U64)native) & 0xfff) {
        kpanic("ramPageAllocNative must be aligned to a page");
    }
    refCounts[index].refCount = 1;
    refCounts[index].isNative = 1;
    RamPage result;
    result.value = index;
    return result;
}

void shutdownRam() {
    refCounts.clear();
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
    if (linearRam) {
        munmap(linearRam, (U64)1 << 32);
        linearRam = nullptr;
        linearRamPageCount = 0;
        std::fill(freeLinearRamPages.begin(), freeLinearRamPages.end(), false);
        freeLinearRamBlocks.clear();
        close(linearRamFile);
        linearRamFile = -1;
    }
    linearRamInitializationAttempted = false;
#endif
    linearMemoryMode = LinearMemoryMode::Disabled;
    linearMemoryPageCount = 1;
    if (KSystem::canJitUse4KPage) {
        for (U8* p : allocatedPages4kChunk) {
#ifdef BOXEDWINE_WINDOWS_LINEAR_MEMORY
            if (windowsLinearRamFile) {
                releaseWindowsLinearMemory(p, CHUNK_SIZE_4K * 64 * 1024);
            } else {
                Platform::releaseNativeMemory(p, CHUNK_SIZE_4K * 64 * 1024);
            }
#else
            Platform::releaseNativeMemory(p, CHUNK_SIZE_4K * 64 * 1024);
#endif
        }
        allocatedPages4kChunk.clear();
        allocatedPages4k.clear();
    } else {        
        for (AlignedU8* p : allocatedPages) {
            delete[] p;
        }
        allocatedPages.clear();
    }
#ifdef BOXEDWINE_WINDOWS_LINEAR_MEMORY
    if (windowsLinearRamFile) {
        CloseHandle(windowsLinearRamFile);
        windowsLinearRamFile = nullptr;
    }
    windowsLinearRamSlotCount = 0;
    windowsLinearRamInitializationAttempted = false;
#endif
    freeIndexes.clear();
    allocatedRamPages = 0;
}

RamPage ramPageAllocNativeContinuous(U8* native, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    RAM_TYPE ramIndex = ((RAM_TYPE)native) >> K_PAGE_SHIFT;

    for (U32 i = 0; i < pageCount; i++) {
        refCounts[ramIndex + i].refCount = 1;
        refCounts[ramIndex + i].isNative = 1;
        native += K_PAGE_SIZE;
    }
    RamPage result;
    result.value = ramIndex;
    return result;
}

void ramPageRetain(RamPage page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    refCounts[page.value].refCount++;
}

U32 ramPageUseCount(RamPage page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    return refCounts[page.value].refCount;
}

void ramPageMarkSystem(RamPage page, bool isSystem) {
    refCounts[page.value].isSystem = isSystem ? 1 : 0;
}

bool ramPageIsSystem(RamPage page) {
    return refCounts[page.value].isSystem != 0;
}

bool ramPageIsNative(RamPage page) {
    return refCounts[page.value].isNative != 0;
}

void ramPageRelease(RamPage page) {
    if (page.value == 0) {
        return;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    RamInfo& info = refCounts[page.value];
    info.refCount--;
    if (info.refCount == 0) {
        if (info.isNative) {
            info.isNative = 0;
            info.isSystem = 0;
        } else {
            allocatedRamPages--;
#if (defined(__linux__) || defined(__APPLE__)) && (defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
            if (info.linearMemoryBacking) {
                refCounts.erase(page.value);
                return;
            }
#endif
            setLinearRamPageFree(page);
            freeIndexes.push_back(page.value);
        }
    }    
}