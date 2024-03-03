/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#include <winsock2.h>
#include <ws2tcpip.h>
#include "pixelformat.h"
#include "../source/emulation/cpu/binaryTranslation/btCpu.h"
#include <VersionHelpers.h>
#include <Shlwapi.h>
#include <winternl.h>

char* platform_strcasestr(const char* s1, const char* s2) {
    return StrStrIA(s1, s2);
}

LONGLONG PCFreq;
LONGLONG CounterStart;

#define NS100PERSEC (10000000LL)

typedef struct _kSYSTEM_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    ULONG ReadOperationCount;
    ULONG WriteOperationCount;
    ULONG OtherOperationCount;
    ULONG AvailablePages;
    ULONG TotalCommittedPages;
    ULONG TotalCommitLimit;
    ULONG PeakCommitment;
    ULONG PageFaults;
    ULONG WriteCopyFaults;
    ULONG TransitionFaults;
    ULONG Reserved1;
    ULONG DemandZeroFaults;
    ULONG PagesRead;
    ULONG PageReadIos;
    ULONG Reserved2[2];
    ULONG PagefilePagesWritten;
    ULONG PagefilePageWriteIos;
    ULONG MappedFilePagesWritten;
    ULONG MappedFilePageWriteIos;
    ULONG PagedPoolUsage;
    ULONG NonPagedPoolUsage;
    ULONG PagedPoolAllocs;
    ULONG PagedPoolFrees;
    ULONG NonPagedPoolAllocs;
    ULONG NonPagedPoolFrees;
    ULONG TotalFreeSystemPtes;
    ULONG SystemCodePage;
    ULONG TotalSystemDriverPages;
    ULONG TotalSystemCodePages;
    ULONG SmallNonPagedLookasideListAllocateHits;
    ULONG SmallPagedLookasideListAllocateHits;
    ULONG Reserved3;
    ULONG MmSystemCachePage;
    ULONG PagedPoolPage;
    ULONG SystemDriverPage;
    ULONG FastReadNoWait;
    ULONG FastReadWait;
    ULONG FastReadResourceMiss;
    ULONG FastReadNotPossible;
    ULONG FastMdlReadNoWait;
    ULONG FastMdlReadWait;
    ULONG FastMdlReadResourceMiss;
    ULONG FastMdlReadNotPossible;
    ULONG MapDataNoWait;
    ULONG MapDataWait;
    ULONG MapDataNoWaitMiss;
    ULONG MapDataWaitMiss;
    ULONG PinMappedDataCount;
    ULONG PinReadNoWait;
    ULONG PinReadWait;
    ULONG PinReadNoWaitMiss;
    ULONG PinReadWaitMiss;
    ULONG CopyReadNoWait;
    ULONG CopyReadWait;
    ULONG CopyReadNoWaitMiss;
    ULONG CopyReadWaitMiss;
    ULONG MdlReadNoWait;
    ULONG MdlReadWait;
    ULONG MdlReadNoWaitMiss;
    ULONG MdlReadWaitMiss;
    ULONG ReadAheadIos;
    ULONG LazyWriteIos;
    ULONG LazyWritePages;
    ULONG DataFlushes;
    ULONG DataPages;
    ULONG ContextSwitches;
    ULONG FirstLevelTbFills;
    ULONG SecondLevelTbFills;
    ULONG SystemCalls;
} kSYSTEM_PERFORMANCE_INFORMATION, * kPSYSTEM_PERFORMANCE_INFORMATION;

typedef struct _kSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
} kSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, * kPSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef struct _kSYSTEM_TIMEOFDAY_INFORMATION
{
    LARGE_INTEGER BootTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeZoneBias;
    ULONG CurrentTimeZoneId;
    BYTE Reserved1[20];		/* Per MSDN.  Always 0. */
} kSYSTEM_TIMEOFDAY_INFORMATION, * kPSYSTEM_TIMEOFDAY_INFORMATION;

/* 100ns difference between Windows and UNIX timebase. */
#define FACTOR (0x19db1ded53e8000LL)
/* # of nanosecs per second. */
#define NSPERSEC (1000000000LL)

time_t
to_time_t(PLARGE_INTEGER ptr)
{
    /* A file time is the number of 100ns since jan 1 1601
       stuffed into two long words.
       A time_t is the number of seconds since jan 1 1970.  */

    int64_t x = ptr->QuadPart;

    /* pass "no time" as epoch */
    if (x == 0)
        return 0;

    x -= FACTOR;			/* number of 100ns between 1601 and 1970 */
    x /= NS100PERSEC;		/* number of 100ns in a second */
    return x;
}

// function from https://github.com/cygwin/cygwin
/* fhandler_proc.cc: fhandler for /proc virtual filesystem

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */
BString Platform::procStat() {
    U32 pages_in = 0UL, pages_out = 0UL, interrupt_count = 0UL, context_switches = 0UL, swap_in = 0UL, swap_out = 0UL;
    U32 cpuCount = getCpuCount();
    time_t boot_time = 0;
    NTSTATUS status;
    /* Sizeof SYSTEM_PERFORMANCE_INFORMATION on 64 bit systems.  It
       appears to contain some trailing additional information from
       what I can tell after examining the content.
       FIXME: It would be nice if this could be verified somehow. */
    const size_t sizeof_spi = sizeof(kSYSTEM_PERFORMANCE_INFORMATION) + 16;
    kPSYSTEM_PERFORMANCE_INFORMATION spi = (kPSYSTEM_PERFORMANCE_INFORMATION)alloca(sizeof_spi);
    kSYSTEM_TIMEOFDAY_INFORMATION stodi;
    BString result;

    kSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* spt = new kSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[cpuCount];
    status = NtQuerySystemInformation(SystemProcessorPerformanceInformation, (PVOID)spt, sizeof spt[0] * cpuCount, NULL);
    if (!NT_SUCCESS(status))
        klog("NtQuerySystemInformation(SystemProcessorPerformanceInformation), status %x", status);
    else {
        U64 user_time = 0ULL;
        U64 kernel_time = 0ULL;
        U64 idle_time = 0ULL;

        for (U32 i = 0; i < cpuCount; i++) {
            kernel_time += (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart) * CLOCKS_PER_SEC / NS100PERSEC;
            user_time += spt[i].UserTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
            idle_time += spt[i].IdleTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
        }
        result += "cpu ";
        result += user_time;
        result += " 0 ";
        result += kernel_time;
        result += " ";
        result += idle_time;
        result += "\n";

        user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
        for (U32 i = 0; i < cpuCount; i++) {
            interrupt_count += spt[i].InterruptCount;
            kernel_time = (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart) * CLOCKS_PER_SEC / NS100PERSEC;
            user_time = spt[i].UserTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
            idle_time = spt[i].IdleTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;

            result += "cpu";
            result += i;
            result += " ";
            result += user_time;
            result += " 0 ";
            result += kernel_time;
            result += " ";
            result += idle_time;
            result += "\n";
        }

        status = NtQuerySystemInformation(SystemPerformanceInformation, (PVOID)spi, sizeof_spi, NULL);
        if (!NT_SUCCESS(status)) {
            klog("NtQuerySystemInformation(SystemPerformanceInformation) status %x", status);
            memset(spi, 0, sizeof_spi);
        }
        status = NtQuerySystemInformation(SystemTimeOfDayInformation, (PVOID)&stodi, sizeof stodi, NULL);
        if (!NT_SUCCESS(status)) {
            klog("NtQuerySystemInformation(SystemTimeOfDayInformation), status %x", status);
        }
    }
    if (!NT_SUCCESS(status)) {
        BString::empty;
    }

    pages_in = spi->PagesRead;
    pages_out = spi->PagefilePagesWritten + spi->MappedFilePagesWritten;
    /* Note: there is no distinction made in this structure between pages read
       from the page file and pages read from mapped files, but there is such
       a distinction made when it comes to writing.  Goodness knows why.  The
       value of swap_in, then, will obviously be wrong but its our best guess. */
    swap_in = spi->PagesRead;
    swap_out = spi->PagefilePagesWritten;
    context_switches = spi->ContextSwitches;
    boot_time = to_time_t(&stodi.BootTime);

    result += "intr ";
    result += interrupt_count;
    result += "\nctxt ";
    result += context_switches;
    result += "\nbtime ";
    result += boot_time;
    return result;
}

void Platform::init() {
    if (!IsWindows8OrGreater()) {
        SetEnvironmentVariable("SDL_AUDIODRIVER", "directsound");
    }
}

void Platform::writeCodeToMemory(void* address, U32 len, std::function<void()> callback) {
    callback();
}

void Platform::startMicroCounter()
{
    LARGE_INTEGER li;

    QueryPerformanceFrequency(&li);

    PCFreq = li.QuadPart;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

ULONGLONG Platform::getMicroCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (li.QuadPart-CounterStart)*1000000/PCFreq;
}

ULONGLONG Platform::getSystemTimeAsMicroSeconds() {
    FILETIME tm;
    ULONGLONG t;

    GetSystemTimeAsFileTime( &tm );
    t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
    t-=116444736000000000l;
    t/=10;
    /*
    if (!startTime) {
        startTime = t;
    } else {
        ULONGLONG diff = t - startTime;
        t = startTime+diff/20;
    }
    */
    return t;
}

void Platform::listNodes(BString nativePath, std::vector<ListNodeResult>& results) {
    BString path;
    WIN32_FIND_DATA findData;
    HANDLE hFind;

    path = nativePath+"\\*.*";;
    hFind = FindFirstFile(path.c_str(), &findData); 
    if(hFind != INVALID_HANDLE_VALUE)  { 		
        do  { 
            if (strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, ".."))  {
                results.push_back(ListNodeResult(BString::copy(findData.cFileName), (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0));
            }
        } while(FindNextFile(hFind, &findData)); 
        FindClose(hFind); 
    }
}

int Platform::nativeSocketPair(S32 socks[2]) {
    union {
       struct sockaddr_in inaddr;
       struct sockaddr addr;
    } a = {};
    socklen_t addrlen = sizeof(a.inaddr);
    DWORD flags = 0;
    int reuse = 1;

    if (socks == nullptr) {
      WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }

    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) 
        return SOCKET_ERROR;

    a.inaddr.sin_family = AF_INET;
    a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.inaddr.sin_port = 0; 

    socks[0] = socks[1] = (U32)INVALID_SOCKET;
    do {
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,  (char*) &reuse, (socklen_t) sizeof(reuse)) == -1)
            break;
        if  (bind(listener, &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
            break;
        if  (getsockname(listener, &a.addr, &addrlen) == SOCKET_ERROR)
            break;
        if (listen(listener, 1) == SOCKET_ERROR)
            break;
        socks[0] = (U32)WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, flags);
        if (socks[0] == (U32)INVALID_SOCKET)
            break;
        if (connect(socks[0], &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
            break;
        socks[1] = (U32)accept(listener, nullptr, nullptr);
        if (socks[1] == INVALID_SOCKET)
            break;

        closesocket(listener);
        return 0;
    } while (0);

    int e = WSAGetLastError();
    closesocket(listener);
    closesocket(socks[0]);
    closesocket(socks[1]);
    WSASetLastError(e);
    return SOCKET_ERROR;
}

typedef struct _PROCESSOR_POWER_INFORMATION
{
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;
typedef LONG (WINAPI *CNPI)(POWER_INFORMATION_LEVEL,PVOID,ULONG,PVOID,ULONG);
static U32 cachedCpuMaxFreq;

std::shared_ptr<BYTE> getProcessorPowerInformation() {
    unsigned int num_cpus = 0;
    SYSTEM_INFO system_info;
    system_info.dwNumberOfProcessors = 0;

    // Get the number of CPUs.
    GetSystemInfo(&system_info);
    if (system_info.dwNumberOfProcessors == 0) {
        num_cpus = 1;
    } else {
        num_cpus = system_info.dwNumberOfProcessors;
    }
    // Allocate size.
    ULONG size = num_cpus * sizeof(PROCESSOR_POWER_INFORMATION);
    BYTE* p = (BYTE*)LocalAlloc(LPTR, size);
    if (!p) {
        return nullptr;
    }
    std::shared_ptr<BYTE> pBuffer(p, [](BYTE* p) {
        LocalFree(p);
        });

    // Syscall.
    HMODULE hModule = LoadLibrary(TEXT("powrprof.dll"));
    if (!hModule) {
        return nullptr;
    }
    CNPI Pwrinfo = (CNPI)GetProcAddress(hModule, "CallNtPowerInformation");
    if (!Pwrinfo) {
        return nullptr;
    }
    NTSTATUS ret = Pwrinfo(ProcessorInformation, nullptr, 0, pBuffer.get(), size);
    if (ret != 0) {
        return nullptr;
    }
    return pBuffer;
}

U32 Platform::getCpuFreqMHz() {
    if (cachedCpuMaxFreq) {
        return cachedCpuMaxFreq;
    }
    
    std::shared_ptr<BYTE> pBuffer = getProcessorPowerInformation();
    if (!pBuffer) {
        return 0;
    }
    PROCESSOR_POWER_INFORMATION* ppi = (PROCESSOR_POWER_INFORMATION *)pBuffer.get();
    cachedCpuMaxFreq = ppi->MaxMhz;
    return cachedCpuMaxFreq;
}

U32 Platform::getCpuCurScalingFreqMHz(U32 cpuIndex) {
    unsigned int num_cpus = Platform::getCpuCount();    

    if (cpuIndex>=num_cpus) {
        return 0;
    }
    std::shared_ptr<BYTE> pBuffer = getProcessorPowerInformation();
    if (!pBuffer) {
        return 0;
    }
    PROCESSOR_POWER_INFORMATION* ppi = (PROCESSOR_POWER_INFORMATION*)pBuffer.get();
    return ppi[cpuIndex].CurrentMhz;
}

U32 Platform::getCpuMaxScalingFreqMHz(U32 cpuIndex) {
    unsigned int num_cpus = Platform::getCpuCount();    

    if (cpuIndex>=num_cpus) {
        return 0;
    }
    std::shared_ptr<BYTE> pBuffer = getProcessorPowerInformation();
    if (!pBuffer) {
        return 0;
    }
    PROCESSOR_POWER_INFORMATION* ppi = (PROCESSOR_POWER_INFORMATION*)pBuffer.get();

    return ppi[cpuIndex].MhzLimit;
}


U32 Platform::getCpuCount() {
#ifdef BOXEDWINE_MULTI_THREADED
    SYSTEM_INFO system_info;
    system_info.dwNumberOfProcessors = 0;

    // Get the number of CPUs.
    GetSystemInfo(&system_info);
    if (system_info.dwNumberOfProcessors == 0) {
        return 1;
    } else {
        return system_info.dwNumberOfProcessors;
    }
#else
    return 1;
#endif
}

int getPixelFormats(PixelFormat* pfd, int maxPfs) {
    PIXELFORMATDESCRIPTOR p = {};
    HDC hdc = GetDC(GetDesktopWindow());
    int count = DescribePixelFormat(hdc, 0, 0, nullptr);
    int result = 1;
    int i;

    for (i=1;i<=count && result<maxPfs;i++) {
        DescribePixelFormat(hdc, i, sizeof(p), &p);
        if ((p.dwFlags & PFD_SUPPORT_OPENGL) && p.cColorBits<=32 && !(p.dwFlags & PFD_GENERIC_FORMAT)) {
            pfd[result].nSize = 40;
            pfd[result].nVersion = 1;
            pfd[result].dwFlags = p.dwFlags;
            pfd[result].iPixelType = p.iPixelType;
            pfd[result].cColorBits = p.cColorBits;
            pfd[result].cRedBits = p.cRedBits;
            pfd[result].cRedShift = p.cRedShift;
            pfd[result].cGreenBits = p.cGreenBits;
            pfd[result].cGreenShift = p.cGreenShift;
            pfd[result].cBlueBits = p.cBlueBits;
            pfd[result].cBlueShift = p.cBlueShift;
            pfd[result].cAlphaBits = p.cAlphaBits;
            pfd[result].cAlphaShift = p.cAlphaShift;
            pfd[result].cAccumBits = p.cAccumBits;
            pfd[result].cAccumRedBits = p.cAccumRedBits;
            pfd[result].cAccumGreenBits = p.cAccumGreenBits;
            pfd[result].cAccumBlueBits = p.cAccumBlueBits;
            pfd[result].cAccumAlphaBits = p.cAccumAlphaBits;
            pfd[result].cDepthBits = p.cDepthBits;
            pfd[result].cStencilBits = p.cStencilBits;
            pfd[result].cAuxBuffers = p.cAuxBuffers;
            pfd[result].iLayerType = p.iLayerType;
            pfd[result].bReserved = p.bReserved;
            pfd[result].dwLayerMask = p.dwLayerMask;
            pfd[result].dwVisibleMask = p.dwVisibleMask;
            pfd[result].dwDamageMask = p.dwDamageMask;
            result++;
            kdebug("Pixel Format: %d bit (%d%d%d%d) %s:%s depth=%d stencil=%d accum=%d", (int)p.cColorBits, (int)p.cRedBits, (int)p.cBlueBits, (int)p.cGreenBits, (int)p.cAlphaBits, (p.dwFlags & K_PFD_GENERIC_FORMAT)?"not accelerated":"accelerated", (p.dwFlags & K_PFD_DOUBLEBUFFER)?"double buffered":"single buffered", (int)p.cDepthBits, (int)p.cStencilBits, (int)p.cAccumBits);
        }
    }
    if (result==1) {
        for (i=1;i<=count && result<maxPfs;i++) {
            DescribePixelFormat(hdc, i, sizeof(p), &p);
            if ((p.dwFlags & PFD_SUPPORT_OPENGL) && p.cColorBits<=32) {
                pfd[result].nSize = 40;
                pfd[result].nVersion = 1;
                pfd[result].dwFlags = p.dwFlags;
                pfd[result].iPixelType = p.iPixelType;
                pfd[result].cColorBits = p.cColorBits;
                pfd[result].cRedBits = p.cRedBits;
                pfd[result].cRedShift = p.cRedShift;
                pfd[result].cGreenBits = p.cGreenBits;
                pfd[result].cGreenShift = p.cGreenShift;
                pfd[result].cBlueBits = p.cBlueBits;
                pfd[result].cBlueShift = p.cBlueShift;
                pfd[result].cAlphaBits = p.cAlphaBits;
                pfd[result].cAlphaShift = p.cAlphaShift;
                pfd[result].cAccumBits = p.cAccumBits;
                pfd[result].cAccumRedBits = p.cAccumRedBits;
                pfd[result].cAccumGreenBits = p.cAccumGreenBits;
                pfd[result].cAccumBlueBits = p.cAccumBlueBits;
                pfd[result].cAccumAlphaBits = p.cAccumAlphaBits;
                pfd[result].cDepthBits = p.cDepthBits;
                pfd[result].cStencilBits = p.cStencilBits;
                pfd[result].cAuxBuffers = p.cAuxBuffers;
                pfd[result].iLayerType = p.iLayerType;
                pfd[result].bReserved = p.bReserved;
                pfd[result].dwLayerMask = p.dwLayerMask;
                pfd[result].dwVisibleMask = p.dwVisibleMask;
                pfd[result].dwDamageMask = p.dwDamageMask;
                result++;
                kdebug("Pixel Format: %d bit (%d%d%d%d) %s:%s depth=%d stencil=%d accum=%d", p.cColorBits, p.cRedBits, p.cBlueBits, p.cGreenBits, p.cAlphaBits, (p.dwFlags & K_PFD_GENERIC_FORMAT)?"not accelerated":"accelerated", (p.dwFlags & K_PFD_DOUBLEBUFFER)?"double buffered":"single buffered", p.cDepthBits, p.cStencilBits, p.cAccumBits);
            }
        }
    }
    return result;
}

BString Platform::getResourceFilePath(BString location) {
    return BString::empty;
}

void Platform::openFileLocation(BString location) {
    ShellExecute(nullptr, "open", location.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void Platform::setCurrentThreadPriorityHigh() {

}

U32 Platform::getPageAllocationGranularity() {
    static U32 granularity;

    if (!granularity) {
        SYSTEM_INFO sSysInfo;

        GetSystemInfo(&sSysInfo);
        if ((sSysInfo.dwAllocationGranularity & K_PAGE_SIZE) != 0) {
            kpanic("Unexpected host allocation granularity size: %d", sSysInfo.dwAllocationGranularity);
        }
        granularity = sSysInfo.dwAllocationGranularity;
    }
    return granularity / K_PAGE_SIZE;
}

U32 Platform::getPagePermissionGranularity() {
    return 1;
}

U32 Platform::allocateNativeMemory(U64 address) {
    if (!VirtualAlloc((void*)address, getPageAllocationGranularity() << K_PAGE_SHIFT, MEM_COMMIT, PAGE_READWRITE)) {
        LPSTR messageBuffer = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
        kpanic("allocateNativeMemory: failed to commit memory: page=%x : %s", address, messageBuffer);
    }
    return 0;
}

void Platform::releaseNativeMemory(void* address, U64 len) {
    // per Windows spec, if MEM_RELEASE is used, then the dwSize must be 0 and the entire chunk will be released
    if (!VirtualFree(address, 0, MEM_RELEASE)) {
        LPSTR messageBuffer = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
        kpanic("failed to release executable memory: %s", messageBuffer);
    }
}

U8* Platform::alloc64kBlock(U32 count, bool executable) {
    DWORD permission = executable ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;

    U8* result = static_cast<U8*>(VirtualAlloc(nullptr, 64 * 1024 * count, MEM_COMMIT, permission));
    if (!result) {
        LPSTR messageBuffer = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
        kpanic("alloc64kBlock: failed to commit memory : %s", messageBuffer);
    }
    return result;
}

U32 Platform::updateNativePermission(U64 address, U32 permission, U32 len) {
    DWORD proto = 0;
    DWORD oldProtect = 0;

    if (len == 0) {
        len = getPagePermissionGranularity() << K_PAGE_SHIFT;
    }
    permission &= PAGE_PERMISSION_MASK;
    if (permission & PAGE_WRITE) {
        proto = PAGE_READWRITE;
    }
    else if ((permission & PAGE_READ) || (permission & PAGE_EXEC)) {
        proto = PAGE_READONLY;
    }
    else {
        proto = PAGE_NOACCESS;
    }
    if (!VirtualProtect((void*)address, len, proto, &oldProtect)) {
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
        kpanic("failed to protect memory: %s", messageBuffer);
    }
    return 0;
}

U32 Platform::nanoSleep(U64 nano) {
    DWORD millies = (DWORD)(nano / 1000000);
    LARGE_INTEGER startTime = {};

    if (millies > NUMBER_OF_MILLIES_TO_SPIN_FOR_WAIT || !PCFreq || !QueryPerformanceCounter(&startTime)) {
        Sleep(millies);
    } else {
        LONGLONG endTime = startTime.QuadPart + nano * PCFreq / 1000000000;
        while (true) {
            LARGE_INTEGER currentTime;
            if (!QueryPerformanceCounter(&currentTime))
                break;
            if (currentTime.QuadPart >= endTime) {
                break;
            }
        }
    }
    return 0;
}

#ifdef BOXEDWINE_MULTI_THREADED
void Platform::setCpuAffinityForThread(KThread* thread, U32 count) {
    if (KSystem::cpuAffinityCountForApp) {
        U32 cores = Platform::getCpuCount();
        if (cores <= 1) {
            return;
        }
        if (cores > 63) {
            cores = 63;
        }
        U64 mask = 0;
        if (count == 0) {
            mask = (1l << cores) - 1;
        } else if (count == 1) {
            mask = 1 << 1; // rumor has it that core 0 isn't the best one to use
        } else {
            mask = (1l << count) - 1;
        }
        klog("Process %s (PID=%d) set thread %d cpu affinity to %X", thread->process->name.c_str(), thread->process->id, thread->id, mask);
        SetThreadAffinityMask((HANDLE)((BtCPU*)thread->cpu)->nativeHandle, mask);
    }
}
#endif

#ifdef BOXEDWINE_X64
bool platformHasBMI2() {
    int regs[4] = {};

    __cpuidex(regs, 7, 0);
    if (regs[1] & (1 << 8)) {
        return true;
    }
    return false;
}
#endif
