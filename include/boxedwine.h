#ifndef __BOXEDWINE_H__
#define __BOXEDWINE_H__

#define BOXEDWINE_VERSION_STR "231"
#define BOXEDWINE_VERSION_DISPLAY "23.0.2 (beta)"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <functional>
#include <set>
#include <list>
#include <filesystem>

#include <errno.h>

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES) || defined(BOXEDWINE_OPENGL_OSMESA)
#define BOXEDWINE_OPENGL
#endif

#ifdef BOXEDWINE_MAC_JIT
#include "TargetConditionals.h"
#if TARGET_CPU_ARM64
#define BOXEDWINE_BINARY_TRANSLATOR
#define BOXEDWINE_ARMV8BT
#define BOXEDWINE_MULTI_THREADED
#define MAP_BOXEDWINE MAP_JIT
#define BOXEDWINE_64BIT_MMU
#define K_NATIVE_PAGE_SIZE 16384
#define K_NATIVE_NUMBER_OF_PAGES 0x40000
#define K_NATIVE_PAGE_SHIFT 14
#define K_NATIVE_PAGES_PER_PAGE 4
#ifdef _DEBUG
#define BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
#endif
#else
#undef BOXEDWINE_MAC_JIT
#ifdef _DEBUG
#define BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
#endif
#define BOXEDWINE_BINARY_TRANSLATOR
#define BOXEDWINE_X64
#define BOXEDWINE_MULTI_THREADED
#define BOXEDWINE_64BIT_MMU
#define MAP_BOXEDWINE 0
#endif
#else
#define MAP_BOXEDWINE 0
#endif

#include "../source/util/boxedptr.h"

#include "platform.h"
#include "log.h"

#include "../source/util/klist.h"
#include "ktimer.h"
#include "../source/util/synchronization.h"
#include "../source/util/karray.h"
#include "../source/util/stringutil.h"
#include "../source/util/vectorutils.h"
#include "../source/util/fileutils.h"

#include "../source/emulation/cpu/common/cpu.h"
#include "kpoll.h"
#include "memory.h"
#include "kthread.h"
#include "kfilelock.h"
#include "kobject.h"
#include "kfiledescriptor.h"
#include "../source/io/fs.h"
#include "../source/io/fsnode.h"
#include "../source/io/fsopennode.h"
#include "kfile.h"
#include "ksystem.h"
#include "kprocess.h"
#include "kscheduler.h"
#include "recorder.h"
#include "player.h"

#include "log.h"
#include "kerror.h"

#endif
