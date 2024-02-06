ifndef BUILD_DIR

.PHONY: default all clean release test jit testJit multiThreaded testMultiThreaded

default all: multiThreaded

uname_m := $(shell uname -m)
uname_n := $(shell uname -n)
$(info uname -m is $(uname_m))
ifeq ($(uname_m), armv7l)
JIT_FLAGS := -DBOXEDWINE_DYNAMIC_ARMV7 -DBOXEDWINE_DYNAMIC
export GCC_EXTRA_FLAGS := -fno-tree-vectorize
else ifeq ($(uname_m), aarch64)
BT_FLAGS := -O0 -DBOXEDWINE_64 -DBOXEDWINE_BINARY_TRANSLATOR -DBOXEDWINE_ARMV8BT -DBOXEDWINE_MULTI_THREADED -DSIMDE_NO_NEON -DSIMDE_SSE2_NO_NEON
RELEASE_FLAGS := -DBOXEDWINE_64
else ifeq ($(uname_m), i386)
JIT_FLAGS := -DBOXEDWINE_DYNAMIC32 -DBOXEDWINE_DYNAMIC
else ifeq ($(uname_m), i686)
JIT_FLAGS := -DBOXEDWINE_DYNAMIC32 -DBOXEDWINE_DYNAMIC
else ifeq ($(uname_m), x86_64)
BT_FLAGS := -DBOXEDWINE_64 -DBOXEDWINE_BINARY_TRANSLATOR -DBOXEDWINE_X64 -DBOXEDWINE_MULTI_THREADED
RELEASE_FLAGS := -DBOXEDWINE_64
endif

ifeq ($(uname_n), raspberrypi)
BT_FLAGS += -DBOXEDWINE_SMALL_VIRTUAL_MEMORY
endif
jit: export BUILD_DIR := Build/Jit
jit: export EXTRA_CPP_FLAGS := $(JIT_FLAGS)
testJit: export BUILD_DIR := Build/TestJit
testJit: export EXTRA_CPP_FLAGS := $(JIT_FLAGS) -D__TEST
release: export BUILD_DIR := Build/Release
release: export EXTRA_CPP_FLAGS := $(RELEASE_FLAGS)
test:   export BUILD_DIR := Build/Test
test: export EXTRA_CPP_FLAGS := -D__TEST
multiThreaded: export EXTRA_CPP_FLAGS := $(BT_FLAGS)
multiThreaded: export BUILD_DIR := Build/MultiThreaded
testMultiThreaded: export EXTRA_CPP_FLAGS := $(BT_FLAGS) -D__TEST
testMultiThreaded: export BUILD_DIR := Build/TestMultiThreaded

cpus  := $(shell grep -c ^processor /proc/cpuinfo)
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

TARGET_EXEC ?= boxedwine
PROJDIR := $(realpath $(CURDIR)/../..)
TEST_BUILD_DIR ?= ./build/build_test
$(info MAKEFLAGS $(MAKE_FLAGS))
SOURCES = $(shell find $(PROJDIR)/source -type f -name '*.cpp')
SOURCES += $(wildcard $(PROJDIR)/platform/linux/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/platform/sdl/*.cpp)
SOURCES += $(wildcard $(PROJDIR)/lib/pugixml/src/*.cpp)
SOURCES += $(PROJDIR)/lib/glew/src/glew.cpp
SOURCES += $(PROJDIR)/lib/imgui/addon/imguitinyfiledialogs.cpp
SOURCES += $(PROJDIR)/lib/imgui/examples/imgui_impl_sdl.cpp
SOURCES += $(PROJDIR)/lib/imgui/examples/imgui_impl_opengl2.cpp
SOURCES += $(PROJDIR)/lib/imgui/imgui_widgets.cpp
SOURCES += $(PROJDIR)/lib/imgui/imgui_draw.cpp
SOURCES += $(PROJDIR)/lib/imgui/imgui.cpp
SOURCES += $(PROJDIR)/lib/tiny-process/process.cpp
SOURCES += $(PROJDIR)/lib/tiny-process/process_unix.cpp

TEST_SOURCES = $(wildcard $(PROJDIR)/source/test/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/sdl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/sdl/**/*.cpp)
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

ifeq ($(BUILD_DIR), Build/Test)
SRCS := $(TEST_SOURCES)
else ifeq ($(BUILD_DIR), Build/TestJit)
SRCS := $(TEST_SOURCES)
else ifeq ($(BUILD_DIR), Build/TestMultiThreaded)
SRCS := $(TEST_SOURCES)
else
SRCS := $(SOURCES)
endif
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INCLUDES = -I../../include
INCLUDES += -I../../lib/glew/include
INCLUDES += -I../../lib/imgui
INCLUDES += -I../../lib/simde
INCLUDES += -I../../lib/pugixml/src

SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)
CPPFLAGS ?= -std=c++20 -O2 $(GCC_EXTRA_FLAGS) -Wall -Wno-invalid-offsetof -Wno-delete-incomplete -Wno-unused-result -Wno-unknown-pragmas -Wno-unused-local-typedefs -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable $(INCLUDES) -DBOXEDWINE_RECORDER -DBOXEDWINE_ZLIB -DBOXEDWINE_HAS_SETJMP -DSDL2=1 "-DGLH=<SDL_opengl.h>" -DBOXEDWINE_OPENGL_SDL -DSIMDE_SSE2_NO_NATIVE -DBOXEDWINE_POSIX -DBOXEDWINE_OPENGL_IMGUI_V2 $(SDL_CFLAGS) $(EXTRA_CPP_FLAGS)

LDFLAGS = -L./linux_build/lib -lcurl -lssl -lcrypto -lpthread -lm -lz -lminizip -lGL -lstdc++ -lstdc++fs $(SDL_LIBS)

#$(TEST_BUILD_DIR)/boxedwineTest: $(TEST_OBJS)
#	$(CC) $(TEST_OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

endif
