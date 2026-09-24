ifndef BUILD_DIR

.PHONY: default all clean release test testMultiThreaded testMultiThreadedOpenGL multiThreaded jit automation automationMultiThreaded automationJit automationMultiThreadedJit testJit testMultiThreadedJit multiThreadedJit

ZIP_ASSET_FILES := $(wildcard *.zip)
SINGLE_THREADED_MEMORY_FLAGS := -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=2147483648

# JIT modes replace the common runtime exports; counters need ccall for names.
ifeq ($(WEBGL_COUNTERS),1)
WEBGL_COUNTER_JIT_EXPORTS := ,ccall
endif

BUILD_PHASE ?= all

default all: release

release: export BUILD_DIR := Build/Release
release: export EXTRA_CPP_FLAGS := $(RELEASE_FLAGS)
release: export SHELL_FILE := shell.html
release: export EXTRA_LD_FLAGS := $(SINGLE_THREADED_MEMORY_FLAGS)
release: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
test: export BUILD_DIR := Build/Test
test: export EXTRA_CPP_FLAGS := -D__TEST -DASMJIT_STATIC
test: export SHELL_FILE := shelltest.html
test: export EXTRA_LD_FLAGS := --emrun
testMultiThreaded: export BUILD_DIR := Build/TestMultiThreaded
testMultiThreaded: export EXTRA_CPP_FLAGS := -D__TEST -DASMJIT_STATIC -DBOXEDWINE_MULTI_THREADED -pthread
testMultiThreaded: export SHELL_FILE := shelltest.html
testMultiThreaded: export EXTRA_LD_FLAGS := --emrun -pthread -sPTHREAD_POOL_SIZE=32 -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=0 --js-library=boxedwine-multithreaded-audio.js
testMultiThreadedOpenGL: export BUILD_DIR := Build/TestMultiThreadedOpenGL
testMultiThreadedOpenGL: export EXTRA_CPP_FLAGS := -D__TEST -DASMJIT_STATIC -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_OPENGL_BOOTSTRAP_TEST_ONLY -pthread
testMultiThreadedOpenGL: export SHELL_FILE := shelltest.html
testMultiThreadedOpenGL: export EXTRA_LD_FLAGS := --emrun -pthread -sPTHREAD_POOL_SIZE=32 -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=1 -sOFFSCREENCANVASES_TO_PTHREAD=\#canvas --js-library=boxedwine-multithreaded-audio.js
multiThreaded: export EXTRA_CPP_FLAGS := -DBOXEDWINE_MULTI_THREADED -pthread
multiThreaded: export BUILD_DIR := Build/MultiThreaded
multiThreaded: export SHELL_FILE := shell.html
multiThreaded: export EXTRA_LD_FLAGS := -pthread -sPTHREAD_POOL_SIZE=16 -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=1 -sOFFSCREENCANVASES_TO_PTHREAD=\#canvas --js-library=boxedwine-multithreaded-audio.js
multiThreaded: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
# WASM JIT build: emits each basic block as a raw one-function WASM module, then installs it standalone or merges it into a bounded runtime group.
# Key flags:
#   BOXEDWINE_WASM_JIT       – enables the new wasm/ JIT backend
#   BOXEDWINE_JIT            – pulled in automatically by boxedwine.h
#   -sALLOW_TABLE_GROWTH     – allows addFunction() to grow the wasmTable
#   -sALLOW_MEMORY_GROWTH    – compiled blocks can allocate their own memory
#   -sMAXIMUM_MEMORY=3 GiB   – long sessions accumulate many compiled
#                              blocks; without an explicit max,
#                              ALLOW_MEMORY_GROWTH caps near 2 GiB.
WASM_JIT_SIMD_FLAG := -msimd128
jit: export BUILD_DIR := Build/Jit
jit: export EXTRA_CPP_FLAGS := -DBOXEDWINE_WASM_JIT $(WASM_JIT_SIMD_FLAG)
jit: export SHELL_FILE := shell.html
# _malloc/_free + HEAPU32: used by boxedwine-shell.js (finishGroupRelocPatching,
# called from onRuntimeInitialized) to allocate and zero the per-entry reloc
# slot arrays patched into grouped cache modules.
jit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU32$(WEBGL_COUNTER_JIT_EXPORTS) -sEXPORTED_FUNCTIONS=_main,_wasm_jit_import_from_file,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
jit: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
# CI browser automation targets use the production runtime flags, plus recorder
# playback and --emrun so Jenkins can observe the browser exit status.
automation: export BUILD_DIR := Build/Automation
automation: export EXTRA_CPP_FLAGS := $(RELEASE_FLAGS) -DBOXEDWINE_RECORDER
automation: export SHELL_FILE := shell.html
automation: export EXTRA_LD_FLAGS := --emrun -sEXIT_RUNTIME=1 $(SINGLE_THREADED_MEMORY_FLAGS)
automation: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
automationMultiThreaded: export BUILD_DIR := Build/AutomationMultiThreaded
automationMultiThreaded: export EXTRA_CPP_FLAGS := -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_RECORDER -pthread
automationMultiThreaded: export SHELL_FILE := shell.html
automationMultiThreaded: export EXTRA_LD_FLAGS := --emrun -sEXIT_RUNTIME=1 -pthread -sPTHREAD_POOL_SIZE=32 -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=0 --js-library=boxedwine-multithreaded-audio.js
automationMultiThreaded: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
automationJit: export BUILD_DIR := Build/AutomationJit
automationJit: export EXTRA_CPP_FLAGS := -DBOXEDWINE_WASM_JIT -DBOXEDWINE_RECORDER $(WASM_JIT_SIMD_FLAG)
automationJit: export SHELL_FILE := shell.html
automationJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) --emrun -sEXIT_RUNTIME=1 -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU32$(WEBGL_COUNTER_JIT_EXPORTS) -sEXPORTED_FUNCTIONS=_main,_wasm_jit_import_from_file,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
automationJit: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
# Like the test target but with the WASM JIT enabled, so the CPU test suite
# exercises JIT-compiled blocks instead of the normal interpreter.
testJit: export BUILD_DIR := Build/TestJit
testJit: export EXTRA_CPP_FLAGS := -D__TEST -DBOXEDWINE_WASM_JIT $(WASM_JIT_SIMD_FLAG)
testJit: export SHELL_FILE := shelltest.html
testJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) --emrun -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction$(WEBGL_COUNTER_JIT_EXPORTS)
testMultiThreadedJit: export BUILD_DIR := Build/TestMultiThreadedJit
testMultiThreadedJit: export EXTRA_CPP_FLAGS := -D__TEST -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_WASM_JIT $(WASM_JIT_SIMD_FLAG) -pthread
testMultiThreadedJit: export SHELL_FILE := shelltest.html
testMultiThreadedJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) --emrun -pthread -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=0 --pre-js=boxedwine-wasm-jit-module-broker.js --js-library=boxedwine-multithreaded-audio.js -sPTHREAD_POOL_SIZE=$(or $(PTHREAD_POOL_SIZE),32) -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU8$(WEBGL_COUNTER_JIT_EXPORTS) -sEXPORTED_FUNCTIONS=_main,_wasm_jit_mt_register,_wasm_jit_mt_register_group,_wasm_jit_mt_register_group_entry,_wasm_jit_mt_register_group_patch,_wasm_jit_mt_prepare_export,_wasm_jit_mt_set_module_broker_enabled,_wasm_jit_mt_broker_validate_module,_wasm_jit_mt_take_thread_start_owner,_wasm_jit_mt_copy_runtime_batch_stats,_wasm_jit_mt_record_group_worker_event,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
# Multi-threaded WASM JIT: combines the multiThreaded and jit flag sets.
# Within a single emulated process, JIT compilation is serialized via
# memory->mutex, and the inline-TLB arrays are mutated under the same
# mutex (reads from the JIT fast path are unsynchronized U32 loads — same
# hazard the existing BOXEDWINE_MEM_CACHE arrays already accept).
# Cross-process concurrent compilation could in principle race on
# wasmTable.grow()/addFunction(); rare on the emscripten target since
# typically only one Wine process runs.
multiThreadedJit: export BUILD_DIR := Build/MultiThreadedJit
multiThreadedJit: export EXTRA_CPP_FLAGS := -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_WASM_JIT $(WASM_JIT_SIMD_FLAG) -pthread
multiThreadedJit: export SHELL_FILE := shell.html
# HEAPU8/_malloc/_free: used by boxedwine-shell.js (onRuntimeInitialized) to
# copy server-zip cache entries into wasm memory for _wasm_jit_mt_register.
multiThreadedJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) -pthread -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=1 -sOFFSCREENCANVASES_TO_PTHREAD=\#canvas --pre-js=boxedwine-wasm-jit-module-broker.js --js-library=boxedwine-multithreaded-audio.js -sPTHREAD_POOL_SIZE=$(or $(PTHREAD_POOL_SIZE),32) -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU8$(WEBGL_COUNTER_JIT_EXPORTS) -sEXPORTED_FUNCTIONS=_main,_wasm_jit_mt_register,_wasm_jit_mt_register_group,_wasm_jit_mt_register_group_entry,_wasm_jit_mt_register_group_patch,_wasm_jit_mt_prepare_export,_wasm_jit_mt_set_module_broker_enabled,_wasm_jit_mt_broker_validate_module,_wasm_jit_mt_take_thread_start_owner,_wasm_jit_mt_copy_runtime_batch_stats,_wasm_jit_mt_record_group_worker_event,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
multiThreadedJit: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
automationMultiThreadedJit: export BUILD_DIR := Build/AutomationMultiThreadedJit
automationMultiThreadedJit: export EXTRA_CPP_FLAGS := -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_WASM_JIT -DBOXEDWINE_RECORDER $(WASM_JIT_SIMD_FLAG) -pthread
automationMultiThreadedJit: export SHELL_FILE := shell.html
automationMultiThreadedJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) --emrun -sEXIT_RUNTIME=1 -pthread -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=1 -sOFFSCREENCANVASES_TO_PTHREAD=\#canvas --pre-js=boxedwine-wasm-jit-module-broker.js --js-library=boxedwine-multithreaded-audio.js -sPTHREAD_POOL_SIZE=$(or $(PTHREAD_POOL_SIZE),32) -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU8$(WEBGL_COUNTER_JIT_EXPORTS) -sEXPORTED_FUNCTIONS=_main,_wasm_jit_mt_register,_wasm_jit_mt_register_group,_wasm_jit_mt_register_group_entry,_wasm_jit_mt_register_group_patch,_wasm_jit_mt_prepare_export,_wasm_jit_mt_set_module_broker_enabled,_wasm_jit_mt_broker_validate_module,_wasm_jit_mt_take_thread_start_owner,_wasm_jit_mt_copy_runtime_batch_stats,_wasm_jit_mt_record_group_worker_event,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
automationMultiThreadedJit: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)

cpus := $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 1)
make_jobs := $(shell n='$(cpus)'; expr "$$n" : '^[0-9][0-9]*$$' >/dev/null && [ "$$n" -gt 2 ] && expr "$$n" - 1)

ifneq ($(make_jobs),)
export MAKEFLAGS := -j $(make_jobs)
$(info MAKEFLAGS is $(MAKEFLAGS))
endif
release test testMultiThreaded testMultiThreadedOpenGL testJit testMultiThreadedJit multiThreaded jit automation automationMultiThreaded automationJit automationMultiThreadedJit multiThreadedJit:
	@$(MAKE) $(BUILD_PHASE)

clean:
	$(RM) -r Build

else

.DEFAULT_GOAL := all
CC = emcc
CXX = em++
TARGET_EXEC ?= boxedwine.html
PROJDIR := $(realpath $(CURDIR)/../..)
TEST_BUILD_DIR ?= ./build/build_test
$(info MAKEFLAGS $(MAKE_FLAGS))

SOURCES += $(wildcard $(PROJDIR)/source/sdl/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/sdl/emscripten/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/platform/sdl/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/platform/linux/*.cpp)
# For the WASM JIT build, exclude the native JIT backends (x32, armv8) which
# each define their own startNewJIT() that would clash with the wasm/ backend.
# For normal builds, exclude the wasm/ directory to avoid empty compilation units.
ifneq (,$(findstring BOXEDWINE_WASM_JIT,$(EXTRA_CPP_FLAGS)))
SOURCES += $(shell find $(PROJDIR)/source/emulation -type f -name '*.cpp' | grep -v '/x32/' | grep -v '/armv8/')
else
SOURCES += $(shell find $(PROJDIR)/source/emulation -type f -name '*.cpp' | grep -v '/wasm/')
endif
SOURCES += $(wildcard $(PROJDIR)/source/io/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/kernel/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/kernel/**/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/util/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/opengl/sdl/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/opengl/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/vulkan/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/source/x11/*.cpp)
SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/ioapi.c
SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/mztools.c
SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/unzip.c
SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/zip.c

SOFT_SOURCES = $(wildcard $(PROJDIR)/lib/softfloat/source/*.c)
SOFT_SOURCES += $(wildcard $(PROJDIR)/lib/softfloat/source/8086-SSE/*.c)

TEST_SOURCES = $(shell find $(PROJDIR)/lib/asmjit/asmjit -type f -name '*.cpp')
TEST_SOURCES += $(wildcard $(PROJDIR)/source/test/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/test/**/*.cpp)

TEST_SOURCES += $(wildcard $(PROJDIR)/source/sdl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/sdl/emscripten/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/platform/sdl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/platform/linux/*.cpp)
TEST_SOURCES += $(shell find $(PROJDIR)/source/emulation -type f -name '*.cpp')
TEST_SOURCES += $(wildcard $(PROJDIR)/source/io/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/kernel/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/kernel/**/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/util/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/opengl/sdl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/opengl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/vulkan/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/x11/*.cpp)
TEST_SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/ioapi.c
TEST_SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/mztools.c
TEST_SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/unzip.c
TEST_SOURCES += $(PROJDIR)/lib/zlib/contrib/minizip/zip.c

ifneq (,$(filter $(BUILD_DIR),Build/Test Build/TestMultiThreaded Build/TestMultiThreadedOpenGL))
SRCS := $(TEST_SOURCES)
else ifneq (,$(filter $(BUILD_DIR),Build/TestJit Build/TestMultiThreadedJit))
# Test JIT builds: use the test sources but swap in the WASM JIT emulation sources
# (exclude x32/armv8 backends which have their own startNewJIT definitions).
TESTJIT_EMULATION := $(shell find $(PROJDIR)/source/emulation -type f -name '*.cpp' | grep -v '/x32/' | grep -v '/armv8/')
SRCS := $(shell find $(PROJDIR)/lib/asmjit/asmjit -type f -name '*.cpp')
SRCS += $(wildcard $(PROJDIR)/source/test/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/test/**/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/sdl/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/sdl/emscripten/*.cpp)
SRCS += $(wildcard $(PROJDIR)/platform/sdl/*.cpp)
SRCS += $(wildcard $(PROJDIR)/platform/linux/*.cpp)
SRCS += $(TESTJIT_EMULATION)
SRCS += $(wildcard $(PROJDIR)/source/io/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/kernel/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/kernel/**/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/util/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/opengl/sdl/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/opengl/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/vulkan/*.cpp)
SRCS += $(wildcard $(PROJDIR)/source/x11/*.cpp)
SRCS += $(PROJDIR)/lib/zlib/contrib/minizip/ioapi.c
SRCS += $(PROJDIR)/lib/zlib/contrib/minizip/mztools.c
SRCS += $(PROJDIR)/lib/zlib/contrib/minizip/unzip.c
SRCS += $(PROJDIR)/lib/zlib/contrib/minizip/zip.c
else
SRCS := $(SOURCES)
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
SOFT_OBJS := $(SOFT_SOURCES:%=$(BUILD_DIR)/%.o)
ALL := $(OBJS)
ALL += $(SOFT_OBJS)
DEPS := $(ALL:.o=.d)

COMMON_INCLUDES := -I../../lib/asmjit -I../../include -I../../lib/simde
SOFTFLOAT_INCLUDES := -I../../lib/softfloat/source/include -I../../lib/softfloat/source -I../../lib/softfloat/source/8086-SSE
$(OBJS): INCLUDES := $(COMMON_INCLUDES)
$(SOFT_OBJS): INCLUDES := $(SOFTFLOAT_INCLUDES)

CXXFLAGS ?= -std=c++20
OPTIMIZATION_FLAGS ?= -O2
TAIL_CALL_FLAG ?= -mtail-call
COMPILE_EXCEPTION_FLAG ?= -fwasm-exceptions
LINK_EXCEPTION_FLAG ?= -fwasm-exceptions
LONGJMP_FLAG ?= -sSUPPORT_LONGJMP=0
EXCEPTION_CATCHING_ALLOWED ?= ["_Z8mainloopv","_ZN3CPU15runNextSingleOpEv","_ZL14platformThreadP3CPU"]
EXCEPTION_CATCHING_FLAG ?=
CPPFLAGS ?= $(OPTIMIZATION_FLAGS) $(TAIL_CALL_FLAG) $(COMPILE_EXCEPTION_FLAG) $(LONGJMP_FLAG) $(EXCEPTION_CATCHING_FLAG) -s USE_SDL=2 -s USE_ZLIB=1 -DBOXEDWINE_ZLIB -DSDL2=1 -DBOXEDWINE_DISABLE_UI -DBOXEDWINE_OPENGL_SDL -D'GLH=<GL/gl.h>' -DSIMDE_SSE2_NO_NATIVE $(GCC_EXTRA_FLAGS) -Wall -Wno-deprecated-pragma -Wno-format-security -Wno-unused-private-field -Wno-invalid-offsetof -Wno-delete-incomplete -Wno-unused-result -Wno-unknown-pragmas -Wno-unused-local-typedefs -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable $(INCLUDES) $(EXTRA_CPP_FLAGS)

SHELL_LDFLAGS := $(if $(filter %.html,$(TARGET_EXEC)),--shell-file $(SHELL_FILE),)
LDFLAGS = $(OPTIMIZATION_FLAGS) $(TAIL_CALL_FLAG) -s FORCE_FILESYSTEM -lidbfs.js -s USE_SDL=2 -s USE_ZLIB=1 -sFULL_ES3 -s STACK_SIZE=1048576 $(LINK_EXCEPTION_FLAG) $(LONGJMP_FLAG) $(EXCEPTION_CATCHING_FLAG) -s INITIAL_MEMORY=536870912 $(SHELL_LDFLAGS) -s EXPORTED_RUNTIME_METHODS='["addRunDependency", "removeRunDependency","ERRNO_CODES","ccall","FS"]' $(EXTRA_LD_FLAGS)
LDFLAGS += --js-library=boxedwine-audio-worklet.js
# Wine's WebGL streaming buffers start at 16 MiB. Client-array draws include
# the stream offset, so Halo exceeds Emscripten's default 2 MiB upload limit.
LDFLAGS += -sGL_MAX_TEMP_BUFFER_SIZE=16777216

# ASan substitutes its own pthread argument. Keep module-broker owner lookup
# working in diagnostic builds while retaining sanitizer thread registration.
ifneq (,$(findstring -fsanitize=address,$(OPTIMIZATION_FLAGS)))
ifneq (,$(findstring -DBOXEDWINE_WASM_JIT,$(EXTRA_CPP_FLAGS)))
ifneq (,$(findstring -DBOXEDWINE_MULTI_THREADED,$(EXTRA_CPP_FLAGS)))
ASAN_PTHREAD_OBJ := $(BUILD_DIR)/boxedwine-asan-pthread.o
ALL += $(ASAN_PTHREAD_OBJ)
DEPS += $(ASAN_PTHREAD_OBJ:.o=.d)
LDFLAGS += -Wl,--wrap=pthread_create -Wl,--wrap=emscripten_builtin_pthread_create -Wl,--export=boxedwine_asan_take_thread_start_arg
endif
endif
endif

# Configuration dependencies below rebuild objects when this option changes.
# Names are metadata; instruction-level timing counters remain disabled.
ifeq ($(WASM_PROFILING),1)
GCC_EXTRA_FLAGS += -DBOXEDWINE_WASM_JIT_NAMES
LDFLAGS += --profiling-funcs
endif

# Diagnostic GL request counters are independent of JIT function names.
# Append to CPPFLAGS so caller-supplied GCC_EXTRA_FLAGS are preserved.
ifeq ($(WEBGL_COUNTERS),1)
CPPFLAGS += -DBOXEDWINE_WEBGL_COUNTERS
WEBGL_COUNTER_ASSETS := webgl-counters.mjs
endif

ASSET_OUTPUTS := $(addprefix $(BUILD_DIR)/,$(ASSET_FILES) $(WEBGL_COUNTER_ASSETS))

.PHONY: all compile copy-assets FORCE_BUILD_CONFIG

# Export settings as environment values, not shell-quoted command arguments.
# The helper preserves the stamp mtime when the settings/compiler are unchanged.
PYTHON ?= python3
COMPILE_CONFIG := $(BUILD_DIR)/.compile-config.json
LINK_CONFIG := $(BUILD_DIR)/.link-config.json
export BW_BUILD_CC = $(CC)
export BW_BUILD_CXX = $(CXX)
export BW_BUILD_AS = $(AS)
export BW_BUILD_CPPFLAGS = $(CPPFLAGS)
export BW_BUILD_CFLAGS = $(CFLAGS)
export BW_BUILD_CXXFLAGS = $(CXXFLAGS)
export BW_BUILD_ASFLAGS = $(ASFLAGS)
export BW_BUILD_COMMON_INCLUDES = $(COMMON_INCLUDES)
export BW_BUILD_SOFTFLOAT_INCLUDES = $(SOFTFLOAT_INCLUDES)
export BW_BUILD_WASM_EXCEPTION_CPPFLAGS = $(WASM_EXCEPTION_CPPFLAGS)
export BW_BUILD_LDFLAGS = $(LDFLAGS)
export BW_BUILD_OBJECTS = $(ALL)
export BW_BUILD_TARGET = $(BUILD_DIR)/$(TARGET_EXEC)

FORCE_BUILD_CONFIG:

$(COMPILE_CONFIG): FORCE_BUILD_CONFIG
	@$(PYTHON) update-build-config.py compile "$@"

$(LINK_CONFIG): FORCE_BUILD_CONFIG
	@$(PYTHON) update-build-config.py link "$@"

compile: $(ALL)

all: $(BUILD_DIR)/$(TARGET_EXEC) copy-assets

ifneq ($(ASAN_PTHREAD_OBJ),)
$(ASAN_PTHREAD_OBJ): boxedwine-asan-pthread.cpp $(COMPILE_CONFIG)
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
endif

copy-assets: $(ASSET_OUTPUTS)

$(ASSET_OUTPUTS): $(BUILD_DIR)/%: %
	@$(MKDIR_P) $(dir $@)
	@cp "$<" "$@"

#$(TEST_BUILD_DIR)/boxedwineTest: $(TEST_OBJS)
#	$(CC) $(TEST_OBJS) -o $@ $(LDFLAGS)

LINK_INPUTS := $(if $(filter %.html,$(TARGET_EXEC)),$(SHELL_FILE),)
LINK_INPUTS += boxedwine-audio-worklet.js
LINK_INPUTS += $(if $(findstring boxedwine-wasm-jit-module-broker.js,$(EXTRA_LD_FLAGS)),boxedwine-wasm-jit-module-broker.js,)
LINK_INPUTS += $(if $(findstring boxedwine-multithreaded-audio.js,$(EXTRA_LD_FLAGS)),boxedwine-multithreaded-audio.js,)

$(BUILD_DIR)/$(TARGET_EXEC): $(ALL) $(LINK_CONFIG) $(LINK_INPUTS)
	@$(CC) $(ALL) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s $(COMPILE_CONFIG)
	@$(MKDIR_P) $(dir $@)
	@$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c $(COMPILE_CONFIG)
	@$(MKDIR_P) $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp $(COMPILE_CONFIG)
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Files that need -fwasm-exceptions so their catch(...) blocks compile to WASM
# catch_all instructions and can intercept WASM exceptions from JIT blocks.
#
# jitWasmCodeGen.cpp: calls generated JIT blocks via call_indirect and contains
#   diagnostic catch paths. Keep it in the WASM EH set so exceptions thrown
#   inside JIT WASM modules stay in the same exception model while propagating.
#
# kscheduler.cpp: runThreadSlice is the top-level CPU-loop handler.  Its
#   catch(...) must be WASM catch_all so it can catch throw 2 (SIGSEGV) and
#   throw 1 (exit) that propagate out of JIT block helpers as WASM exceptions.
#   Without -fwasm-exceptions its catch uses the old JS-flag mechanism which
#   cannot catch WASM exceptions — causing the JIT to crash the browser session
#   instead of delivering the signal to Wine's handler.
#
# Adding -fwasm-exceptions globally breaks Wine loading (changes throw semantics
# for all code, triggering RAII destructors that weren't firing before).
# These explicit rules take precedence over the pattern rule above in GNU make.
ifneq (,$(findstring BOXEDWINE_WASM_JIT,$(EXTRA_CPP_FLAGS)))
WASM_EXCEPTION_CPPFLAGS = $(filter-out $(COMPILE_EXCEPTION_FLAG) $(LONGJMP_FLAG) $(EXCEPTION_CATCHING_FLAG),$(CPPFLAGS)) -fwasm-exceptions -sSUPPORT_LONGJMP=wasm
else
WASM_EXCEPTION_CPPFLAGS = $(CPPFLAGS)
endif

$(BUILD_DIR)/$(PROJDIR)/source/emulation/cpu/wasm/jitWasmCodeGen.cpp.o: \
        $(PROJDIR)/source/emulation/cpu/wasm/jitWasmCodeGen.cpp $(COMPILE_CONFIG)
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(WASM_EXCEPTION_CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(PROJDIR)/source/kernel/kscheduler.cpp.o: \
        $(PROJDIR)/source/kernel/kscheduler.cpp $(COMPILE_CONFIG)
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(WASM_EXCEPTION_CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

endif
