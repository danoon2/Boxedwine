ifndef BUILD_DIR

.PHONY: default all clean release test 

default all: release

uname_m := $(shell uname -m)
uname_n := $(shell uname -n)
$(info uname -m is $(uname_m))
ifeq ($(uname_m), armv7l)
export GCC_EXTRA_FLAGS := -fno-tree-vectorize
else ifeq ($(uname_m), aarch64)
BOXED_FLAGS := -DASMJIT_NO_X86 -DASMJIT_STATIC -DBOXEDWINE_64 -DBOXEDWINE_JIT -DBOXEDWINE_JIT_ARMV8 -DSIMDE_NO_NEON -DSIMDE_SSE2_NO_NEON -DBOXEDWINE_MULTI_THREADED
else ifeq ($(uname_m), i386)
BOXED_FLAGS := -DASMJIT_NO_AARCH64 -DASMJIT_STATIC -DBOXEDWINE_JIT_X86 -DBOXEDWINE_JIT -DBOXEDWINE_MULTI_THREADED
else ifeq ($(uname_m), i686)
BOXED_FLAGS := -DASMJIT_NO_AARCH64 -DASMJIT_STATIC -DBOXEDWINE_JIT_X86 -DBOXEDWINE_JIT -DBOXEDWINE_MULTI_THREADED
else ifeq ($(uname_m), x86_64)
BOXED_FLAGS := -DASMJIT_NO_AARCH64 -DASMJIT_STATIC -DBOXEDWINE_64 -DBOXEDWINE_JIT -DBOXEDWINE_JIT_X64 -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_MEM_CACHE -DBOXEDWINE_HOST_EXCEPTIONS -DBOXEDWINE_4K_PAGE_SIZE
endif

release: export BUILD_DIR := Build/Release
release: export EXTRA_CPP_FLAGS := $(BOXED_FLAGS)
test:   export BUILD_DIR := Build/Test
test: export EXTRA_CPP_FLAGS := $(BOXED_FLAGS) -D__TEST

cpus  := $(shell grep -c ^processor /proc/cpuinfo)
ifeq ($(cpus), 0)
else ifeq ($(cpus), 1)
else ifeq ($(cpus), 2)
else
cpus := $(shell echo $$(($(cpus)-1)))
export MAKEFLAGS := -j $(cpus)
$(info MAKEFLAGS is $(MAKEFLAGS))
endif
release test:
	@$(MAKE)

clean:
	$(RM) -r Build

else

TARGET_EXEC ?= boxedwine
PROJDIR := $(realpath $(CURDIR)/../..)
TEST_BUILD_DIR ?= ./build/build_test
$(info MAKEFLAGS $(MAKE_FLAGS))
SOURCES = $(shell find $(PROJDIR)/lib/asmjit/asmjit -type f -name '*.cpp')
SOURCES += $(shell find $(PROJDIR)/source -type f -name '*.cpp')
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

SOFT_SOURCES = $(wildcard $(PROJDIR)/lib/softfloat/source/*.c)
SOFT_SOURCES += $(wildcard $(PROJDIR)/lib/softfloat/source/8086-SSE/*.c)

TEST_SOURCES = $(shell find $(PROJDIR)/lib/asmjit/asmjit -type f -name '*.cpp')
TEST_SOURCES += $(wildcard $(PROJDIR)/source/test/*.cpp)
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
TEST_SOURCES += $(wildcard $(PROJDIR)/source/x11/*.cpp)

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

$(OBJS): INCLUDES := -I../../lib/asmjit -I../../include -I../../lib/glew/include -I../../lib/imgui -I../../lib/simde -I../../lib/pugixml/src
$(SOFT_OBJS): INCLUDES := -I../../lib/softfloat/source/include -I../../lib/softfloat/source -I../../lib/softfloat/source/8086-SSE

SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)
CPPFLAGS ?= -std=c++20 -O2 $(EXTRA_CPP_FLAGS) $(GCC_EXTRA_FLAGS) -Werror=switch -Wall -Wno-invalid-offsetof -Wno-delete-incomplete -Wno-unused-result -Wno-unknown-pragmas -Wno-unused-local-typedefs -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable $(INCLUDES) -DBOXEDWINE_RECORDER -DBOXEDWINE_ZLIB -DBOXEDWINE_HAS_SETJMP -DSDL2=1 "-DGLH=<SDL_opengl.h>" -DBOXEDWINE_OPENGL_SDL -DSIMDE_SSE2_NO_NATIVE -DBOXEDWINE_POSIX -DBOXEDWINE_OPENGL_IMGUI_V2 $(SDL_CFLAGS)
CFLAGS ?= -O2 $(INCLUDES)
LDFLAGS = -L./linux_build/lib -lcurl -lssl -lcrypto -lpthread -lm -lz -lminizip -lGL -lstdc++ -lstdc++fs $(SDL_LIBS)

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
	@$(CC) $(CFLAGS) -c $< -o $@

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
