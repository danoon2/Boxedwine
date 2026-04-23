ifndef BUILD_DIR

.PHONY: default all clean release test multiThreaded jit testJit

default all: release

release: export BUILD_DIR := Build/Release
release: export EXTRA_CPP_FLAGS := $(RELEASE_FLAGS)
release: export SHELL_FILE := shell.html
test: export BUILD_DIR := Build/Test
test: export EXTRA_CPP_FLAGS := -D__TEST -DASMJIT_STATIC
test: export SHELL_FILE := shelltest.html
test: export EXTRA_LD_FLAGS := --emrun
multiThreaded: export EXTRA_CPP_FLAGS := -DBOXEDWINE_MULTI_THREADED -pthread
multiThreaded: export BUILD_DIR := Build/MultiThreaded
multiThreaded: export SHELL_FILE := shell.html
multiThreaded: export EXTRA_LD_FLAGS := -pthread -sPTHREAD_POOL_SIZE=32
# WASM JIT build: compiles each basic block to a standalone WASM module that
# is instantiated at runtime (inspired by the QEMU wasm64 TCG backend).
# Key flags:
#   BOXEDWINE_WASM_JIT  – enables the new wasm/ JIT backend
#   BOXEDWINE_JIT       – pulled in automatically by boxedwine.h
#   -sALLOW_TABLE_GROWTH – allows addFunction() to grow the wasmTable
#   -sALLOW_MEMORY_GROWTH – compiled blocks can allocate their own memory
jit: export BUILD_DIR := Build/Jit
jit: export EXTRA_CPP_FLAGS := -DBOXEDWINE_WASM_JIT
jit: export SHELL_FILE := shell.html
jit: export EXTRA_LD_FLAGS := -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction,wasmTable
# Like the test target but with the WASM JIT enabled, so the CPU test suite
# exercises JIT-compiled blocks instead of the normal interpreter.
testJit: export BUILD_DIR := Build/TestJit
testJit: export EXTRA_CPP_FLAGS := -D__TEST -DBOXEDWINE_WASM_JIT
testJit: export SHELL_FILE := shelltest.html
testJit: export EXTRA_LD_FLAGS := --emrun -sALLOW_TABLE_GROWTH -sALLOW_MEMORY_GROWTH -sEXPORTED_RUNTIME_METHODS=addFunction,removeFunction

cpus := $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 1)
make_jobs := $(shell n='$(cpus)'; expr "$$n" : '^[0-9][0-9]*$$' >/dev/null && [ "$$n" -gt 2 ] && expr "$$n" - 1)

ifneq ($(make_jobs),)
export MAKEFLAGS := -j $(make_jobs)
$(info MAKEFLAGS is $(MAKEFLAGS))
endif
jit release test testJit multiThreaded testMultiThreaded:
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

ifeq ($(BUILD_DIR), Build/Test)
SRCS := $(TEST_SOURCES)
else ifeq ($(BUILD_DIR), Build/TestJit)
# TestJit: use the test sources but swap in the WASM JIT emulation sources
# (exclude x32/armv8 backends which have their own startNewJIT definitions).
TESTJIT_EMULATION := $(shell find $(PROJDIR)/source/emulation -type f -name '*.cpp' | grep -v '/x32/' | grep -v '/armv8/')
SRCS := $(wildcard $(PROJDIR)/source/test/*.cpp)
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
CPPFLAGS ?= -O3 -s USE_SDL=2 -s USE_ZLIB=1 -DBOXEDWINE_ZLIB -DSDL2=1 -DBOXEDWINE_DISABLE_UI -DSIMDE_SSE2_NO_NATIVE $(GCC_EXTRA_FLAGS) -Wall -Wno-deprecated-pragma -Wno-format-security -Wno-unused-private-field -Wno-invalid-offsetof -Wno-delete-incomplete -Wno-unused-result -Wno-unknown-pragmas -Wno-unused-local-typedefs -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable $(INCLUDES) $(EXTRA_CPP_FLAGS)

LDFLAGS = -s FORCE_FILESYSTEM -lidbfs.js -s USE_SDL=2 -s USE_ZLIB=1 -fwasm-exceptions -s TOTAL_MEMORY=536870912 --shell-file $(SHELL_FILE) -s EXPORTED_RUNTIME_METHODS='["addRunDependency", "removeRunDependency","ERRNO_CODES"]' $(EXTRA_LD_FLAGS)

#$(TEST_BUILD_DIR)/boxedwineTest: $(TEST_OBJS)
#	$(CC) $(TEST_OBJS) -o $@ $(LDFLAGS)

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

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

endif
