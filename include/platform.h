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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef __EMSCRIPTEN__
#define MAX_FILEPATH_LEN 256
#else
#define MAX_FILEPATH_LEN 1024
#endif

// maximum number of files per directory
#define MAX_DIR_LISTING 4096

#include "platformtypes.h"
#define FD S32

#ifndef BOXEDWINE_64BIT_MMU
#define BOXEDWINE_DEFAULT_MMU 1
#endif

#ifdef BOXEDWINE_HAS_SETJMP
#include <setjmp.h>
#endif

#ifdef __APPLE__
#define lseek64 lseek
#endif

#ifdef BOXEDWINE_MSVC
#define THREAD_LOCAL __declspec(thread)
#define PLATFORM_STAT_STRUCT struct _stat32i64
#define PLATFORM_STAT _stat32i64
#define OPCALL __fastcall
#define unlink _unlink
#define ftruncate(h, l) _chsize(h, (long)l)
#define lseek64 _lseeki64
#define platform_getcwd _getcwd
#define UNISTD <io.h>
#define UTIME <sys/utime.h>
#define CURDIR_INCLUDE <direct.h>
#define MKDIR_INCLUDE <direct.h>
#define RMDIR_INCLUDE <direct.h>
#define MKDIR(x) mkdir(x)
#define INLINE __inline
#define OPENGL_CALL_TYPE __stdcall
#define PACKED( s ) __pragma( pack(push, 1) ) s __pragma( pack(pop) )
#define socklen_t int
#define ALIGN(t, x) __declspec(align(x)) t
#else
#include <limits.h>
#define THREAD_LOCAL thread_local
#if ( __WORDSIZE == 64 )
#define BOXEDWINE_64   1
#endif
#define PLATFORM_STAT_STRUCT struct stat
#define PLATFORM_STAT stat
#define OPCALL
#define platform_getcwd getcwd
#define UNISTD <unistd.h>
#define UTIME <utime.h>
#define CURDIR_INCLUDE <unistd.h>
#define MKDIR_INCLUDE <sys/stat.h>
#define RMDIR_INCLUDE <unistd.h>
#define MKDIR(x) mkdir(x, 0777)
#define O_BINARY 0
#define INLINE inline
#define OPENGL_CALL_TYPE
#define PACKED( s ) s __attribute__((__packed__))
#define ALIGN(t, x) t __attribute__((aligned(x)))
#endif

#ifndef S_ISDIR
# define S_ISDIR(ST_MODE) (((ST_MODE) & _S_IFMT) == _S_IFDIR)
#endif

class FsNode;
class Platform {
public:
    class ListNodeResult {
    public:
        ListNodeResult(const std::string& name, bool isDirectory) : name(name), isDirectory(isDirectory) {}
        std::string name;
        bool isDirectory;
    };
    static void listNodes(const std::string& nativePath, std::vector<ListNodeResult>& results);
    static U64 getSystemTimeAsMicroSeconds();
    static U64 getMicroCounter();
    static void startMicroCounter();
    static int nativeSocketPair(S32 socks[2]);
};

#include <string.h>
#include "log.h"

INLINE void safe_strcpy(char* dest, const char* src, int bufferSize) {
    int len = (int)strlen(src);
    if (len+1>bufferSize) {
        kpanic("safe_strcpy failed to copy %s, buffer is %d bytes", src, bufferSize);
    }
    strcpy(dest, src);
}

INLINE void safe_strcat(char* dest, const char* src, int bufferSize) {
    int len = (int)(strlen(src)+strlen(dest));
    if (len+1>bufferSize) {
        kpanic("safe_strcat failed to copy %s, buffer is %d bytes", src, bufferSize);
    }
    strcat(dest, src);
}

#if defined BOXEDWINE_DYNAMIC
void freeExecutable64kBlock(void* p);
void* allocExecutable64kBlock(int count);
#endif

#ifdef BOXEDWINE_X64
class Memory;
void allocExecutable64kBlock(Memory* memory, U32 page);
bool platformHasBMI2();
#endif

#ifdef BOXEDWINE_MULTI_THREADED
void ATOMIC_WRITE64(U64* pTarget, U64 value);
#endif

#ifdef BOXEDWINE_MIDI
void PlayMsg(U8* msg);
void PlaySysex(U8 * sysex,U32 len);
#endif

#define VECTOR_CONTAINS(v, o) (std::find(v.begin(), v.end(), o) != v.end())
#define VECTOR_REMOVE(v, o) v.erase(std::remove(v.begin(), v.end(), o), v.end())
#define VECTOR_TO_ARRAY_ON_STACK(v, t, r) r = (t*)alloca(v.size()*sizeof(t)); std::copy(v.begin(), v.end(), r)
#endif
