ifndef BUILD_DIR

.PHONY: default all clean release test multiThreaded

default all: release

release: export BUILD_DIR := Build/Release
release: export EXTRA_CPP_FLAGS := $(RELEASE_FLAGS)
release: export SHELL_FILE := shell.html
test: export BUILD_DIR := Build/Test
test: export EXTRA_CPP_FLAGS := -D__TEST
test: export SHELL_FILE := shelltest.html
test: export EXTRA_LD_FLAGS := --emrun
multiThreaded: export EXTRA_CPP_FLAGS := -DBOXEDWINE_MULTI_THREADED -pthread
multiThreaded: export BUILD_DIR := Build/MultiThreaded
multiThreaded: export SHELL_FILE := shell.html
multiThreaded: export EXTRA_LD_FLAGS := -sPROXY_TO_PTHREAD -pthread -sPTHREAD_POOL_SIZE=100

uname_s  :=$(shell uname -s)
ifeq ($(uname_s),Darwin)
cpus  := $(shell sysctl -a | grep  machdep.cpu.core_count: |  tr -d 'machdep.cpu.core_count: ')
else
cpus  := $(shell grep -c ^processor /proc/cpuinfo)
endif

ifeq ($(cpus), 0)
else ifeq ($(cpus), 1)
else ifeq ($(cpus), 2)
else
cpus := $(shell echo $$(($(cpus)-1)))
export MAKEFLAGS := -j $(cpus)
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
SOURCES += $(shell find $(PROJDIR)/source/emulation -type f -name '*.cpp')
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

TEST_SOURCES = $(wildcard $(PROJDIR)/source/test/*.cpp)
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
else
SRCS := $(SOURCES)
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
SOFT_OBJS := $(SOFT_SOURCES:%=$(BUILD_DIR)/%.o)
ALL := $(OBJS)
ALL += $(SOFT_OBJS)
DEPS := $(ALL:.o=.d)

$(OBJS): INCLUDES := -I../../include -I../../lib/simde
$(SOFT_OBJS): INCLUDES := -I../../lib/softfloat/source/include -I../../lib/softfloat/source -I../../lib/softfloat/source/8086-SSE

CXXFLAGS ?= -std=c++20
CPPFLAGS ?= -O3 -s USE_SDL=2 -s USE_ZLIB=1 -DBOXEDWINE_ZLIB -DSDL2=1 -DBOXEDWINE_DISABLE_UI -DSIMDE_SSE2_NO_NATIVE $(GCC_EXTRA_FLAGS) -Wall -Wno-format-security -Wno-unused-private-field -Wno-invalid-offsetof -Wno-delete-incomplete -Wno-unused-result -Wno-unknown-pragmas -Wno-unused-local-typedefs -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable $(INCLUDES) $(EXTRA_CPP_FLAGS)

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
