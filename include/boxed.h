#ifndef __BOXED_H__
#define __BOXED_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>

//#include <boost/algorithm/cxx11/any_of.hpp>
//#include <boost/algorithm/string.hpp>
//#include <boost/algorithm/string/predicate.hpp>
//#include <boost/algorithm/string/replace.hpp>
//#include <boost/circular_buffer.hpp>
//#include <boost/range/algorithm.hpp>
//#include <boost/range/algorithm_ext.hpp>

#include "platform.h"
#include "../source/util/ringbuffer.h"
#include "../source/util/ptr.h"
#include "../source/util/klist.h"

#include "../source/emulation/cpu/common/cpu.h"
#include "kpoll.h"
#include "ktimer.h"
#include "kthread.h"
#include "kfilelock.h"
#include "kobject.h"
#include "kfiledescriptor.h"
#include "../../source/io/fs.h"
#include "../../source/io/fsnode.h"
#include "../../source/io/fsopennode.h"
#include "kfile.h"
#include "ksystem.h"
#include "kprocess.h"
#include "memory.h"
#include "kscheduler.h"

#include "log.h"
#include "kerror.h"

#endif