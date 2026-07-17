ifndef BUILD_DIR

.PHONY: default all clean release test testMultiThreaded multiThreaded jit automationJit testJit testMultiThreadedJit multiThreadedJit

ZIP_ASSET_FILES := $(wildcard *.zip)

default all: release

release: export BUILD_DIR := Build/Release
release: export EXTRA_CPP_FLAGS := $(RELEASE_FLAGS)
release: export SHELL_FILE := shell.html
release: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
test: export BUILD_DIR := Build/Test
test: export EXTRA_CPP_FLAGS := -D__TEST -DASMJIT_STATIC
test: export SHELL_FILE := shelltest.html
test: export EXTRA_LD_FLAGS := --emrun
testMultiThreaded: export BUILD_DIR := Build/TestMultiThreaded
testMultiThreaded: export EXTRA_CPP_FLAGS := -D__TEST -DASMJIT_STATIC -DBOXEDWINE_MULTI_THREADED -pthread
testMultiThreaded: export SHELL_FILE := shelltest.html
testMultiThreaded: export EXTRA_LD_FLAGS := --emrun -pthread -sPTHREAD_POOL_SIZE=32 -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=0 --js-library=boxedwine-multithreaded-audio.js
multiThreaded: export EXTRA_CPP_FLAGS := -DBOXEDWINE_MULTI_THREADED -pthread
multiThreaded: export BUILD_DIR := Build/MultiThreaded
multiThreaded: export SHELL_FILE := shell.html
multiThreaded: export EXTRA_LD_FLAGS := -pthread -sPTHREAD_POOL_SIZE=32 -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=0 --js-library=boxedwine-multithreaded-audio.js
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
jit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU32 -sEXPORTED_FUNCTIONS=_main,_wasm_jit_import_from_file,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
jit: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
# CI browser automation target: same single-threaded WASM JIT runtime, with
# recorder playback enabled and --emrun so Jenkins can observe browser exit.
automationJit: export BUILD_DIR := Build/AutomationJit
automationJit: export EXTRA_CPP_FLAGS := -DBOXEDWINE_WASM_JIT -DBOXEDWINE_RECORDER $(WASM_JIT_SIMD_FLAG)
automationJit: export SHELL_FILE := shell.html
automationJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) --emrun -sEXIT_RUNTIME=1 -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU32 -sEXPORTED_FUNCTIONS=_main,_wasm_jit_import_from_file,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
automationJit: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)
# Like the test target but with the WASM JIT enabled, so the CPU test suite
# exercises JIT-compiled blocks instead of the normal interpreter.
testJit: export BUILD_DIR := Build/TestJit
testJit: export EXTRA_CPP_FLAGS := -D__TEST -DBOXEDWINE_WASM_JIT $(WASM_JIT_SIMD_FLAG)
testJit: export SHELL_FILE := shelltest.html
testJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) --emrun -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction
testMultiThreadedJit: export BUILD_DIR := Build/TestMultiThreadedJit
testMultiThreadedJit: export EXTRA_CPP_FLAGS := -D__TEST -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_WASM_JIT $(WASM_JIT_SIMD_FLAG) -pthread
testMultiThreadedJit: export SHELL_FILE := shelltest.html
testMultiThreadedJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) --emrun -pthread -sPROXY_TO_PTHREAD=1 -sOFFSCREENCANVAS_SUPPORT=0 --pre-js=boxedwine-wasm-jit-module-broker.js -sPTHREAD_POOL_SIZE=$(or $(PTHREAD_POOL_SIZE),32) -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU8 -sEXPORTED_FUNCTIONS=_main,_wasm_jit_mt_register,_wasm_jit_mt_register_group,_wasm_jit_mt_register_group_entry,_wasm_jit_mt_register_group_patch,_wasm_jit_mt_prepare_export,_wasm_jit_mt_set_module_broker_enabled,_wasm_jit_mt_broker_validate_module,_wasm_jit_mt_take_thread_start_owner,_wasm_jit_mt_copy_runtime_batch_stats,_wasm_jit_mt_record_group_worker_event,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
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
multiThreadedJit: export EXTRA_LD_FLAGS := $(WASM_JIT_SIMD_FLAG) -pthread --pre-js=boxedwine-wasm-jit-module-broker.js -sPTHREAD_POOL_SIZE=$(or $(PTHREAD_POOL_SIZE),32) -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=3221225472 -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable,HEAPU8 -sEXPORTED_FUNCTIONS=_main,_wasm_jit_mt_register,_wasm_jit_mt_register_group,_wasm_jit_mt_register_group_entry,_wasm_jit_mt_register_group_patch,_wasm_jit_mt_prepare_export,_wasm_jit_mt_set_module_broker_enabled,_wasm_jit_mt_broker_validate_module,_wasm_jit_mt_take_thread_start_owner,_wasm_jit_mt_copy_runtime_batch_stats,_wasm_jit_mt_record_group_worker_event,_wasm_jit_set_persistence_active,_wasm_jit_set_record_active,_malloc,_free
multiThreadedJit: export ASSET_FILES := boxedwine.css boxedwine-shell.js $(ZIP_ASSET_FILES)

cpus := $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 1)
make_jobs := $(shell n='$(cpus)'; expr "$$n" : '^[0-9][0-9]*$$' >/dev/null && [ "$$n" -gt 2 ] && expr "$$n" - 1)

ifneq ($(make_jobs),)
export MAKEFLAGS := -j $(make_jobs)
$(info MAKEFLAGS is $(MAKEFLAGS))
endif
release test testMultiThreaded testJit testMultiThreadedJit multiThreaded jit automationJit multiThreadedJit:
	@$(MAKE)

clean:
	$(RM) -r Build

else

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

ifneq (,$(filter $(BUILD_DIR),Build/Test Build/TestMultiThreaded))
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

$(OBJS): INCLUDES := -I../../lib/asmjit -I../../include -I../../lib/simde
$(SOFT_OBJS): INCLUDES := -I../../lib/softfloat/source/include -I../../lib/softfloat/source -I../../lib/softfloat/source/8086-SSE

CXXFLAGS ?= -std=c++20
OPTIMIZATION_FLAGS ?= -O2
TAIL_CALL_FLAG ?= -mtail-call
COMPILE_EXCEPTION_FLAG ?= -fwasm-exceptions
LINK_EXCEPTION_FLAG ?= -fwasm-exceptions
LONGJMP_FLAG ?= -sSUPPORT_LONGJMP=0
EXCEPTION_CATCHING_ALLOWED ?= ["_Z8mainloopv","_ZN3CPU15runNextSingleOpEv","_ZL14platformThreadP3CPU"]
EXCEPTION_CATCHING_FLAG ?=
CPPFLAGS ?= $(OPTIMIZATION_FLAGS) $(TAIL_CALL_FLAG) $(COMPILE_EXCEPTION_FLAG) $(LONGJMP_FLAG) $(EXCEPTION_CATCHING_FLAG) -s USE_SDL=2 -s USE_ZLIB=1 -DBOXEDWINE_ZLIB -DSDL2=1 -DBOXEDWINE_DISABLE_UI -DSIMDE_SSE2_NO_NATIVE $(GCC_EXTRA_FLAGS) -Wall -Wno-deprecated-pragma -Wno-format-security -Wno-unused-private-field -Wno-invalid-offsetof -Wno-delete-incomplete -Wno-unused-result -Wno-unknown-pragmas -Wno-unused-local-typedefs -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable $(INCLUDES) $(EXTRA_CPP_FLAGS)

SHELL_LDFLAGS := $(if $(filter %.html,$(TARGET_EXEC)),--shell-file $(SHELL_FILE),)
LDFLAGS = $(OPTIMIZATION_FLAGS) $(TAIL_CALL_FLAG) -s FORCE_FILESYSTEM -lidbfs.js -s USE_SDL=2 -s USE_ZLIB=1 $(LINK_EXCEPTION_FLAG) $(LONGJMP_FLAG) $(EXCEPTION_CATCHING_FLAG) -s INITIAL_MEMORY=536870912 $(SHELL_LDFLAGS) -s EXPORTED_RUNTIME_METHODS='["addRunDependency", "removeRunDependency","ERRNO_CODES","ccall","FS"]' $(EXTRA_LD_FLAGS)

ASSET_OUTPUTS := $(addprefix $(BUILD_DIR)/,$(ASSET_FILES))

.PHONY: all copy-assets

all: $(BUILD_DIR)/$(TARGET_EXEC) copy-assets

copy-assets: $(ASSET_OUTPUTS)

$(ASSET_OUTPUTS): $(BUILD_DIR)/%: %
	@$(MKDIR_P) $(dir $@)
	@cp "$<" "$@"

#$(TEST_BUILD_DIR)/boxedwineTest: $(TEST_OBJS)
#	$(CC) $(TEST_OBJS) -o $@ $(LDFLAGS)

ifneq (,$(filter $(BUILD_DIR),Build/TestMultiThreadedJit Build/MultiThreadedJit))
$(BUILD_DIR)/$(TARGET_EXEC): boxedwine-wasm-jit-module-broker.js
endif

$(BUILD_DIR)/$(TARGET_EXEC): $(ALL)
	@$(CC) $(ALL) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	@$(MKDIR_P) $(dir $@)
	@$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	@$(MKDIR_P) $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
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
        $(PROJDIR)/source/emulation/cpu/wasm/jitWasmCodeGen.cpp
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(WASM_EXCEPTION_CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(PROJDIR)/source/kernel/kscheduler.cpp.o: \
        $(PROJDIR)/source/kernel/kscheduler.cpp
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(WASM_EXCEPTION_CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

endif
