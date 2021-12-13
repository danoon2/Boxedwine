ifndef BUILD_DIR

.PHONY: default all release test

default all: release

release: export BUILD_DIR := Build/Release
test:   export BUILD_DIR := Build/Test
test: export EXTRA_CPP_FLAGS := -D__TEST

export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"
release test:
	sh buildPocoLib.sh
	@$(MAKE)

else

TARGET_EXEC ?= boxedwine
PROJDIR := $(realpath $(CURDIR)/../..)
TEST_BUILD_DIR ?= ./build/build_test

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

TEST_SOURCES = $(wildcard $(PROJDIR)/source/test/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/sdl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/sdl/singleThreaded/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/platform/sdl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/platform/linux/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/emulation/cpu/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/emulation/cpu/common/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/emulation/cpu/normal/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/emulation/softmmu/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/io/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/kernel/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/kernel/**/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/util/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/opengl/sdl/*.cpp)
TEST_SOURCES += $(wildcard $(PROJDIR)/source/opengl/*.cpp)

ifeq ($(BUILD_DIR), Build/Test)
SRCS := $(TEST_SOURCES)
else
SRCS := $(SOURCES)
endif
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INCLUDES = -I../../include
INCLUDES += -I../../lib/poco/Net/include
INCLUDES += -I../../lib/poco/Crypto/include
INCLUDES += -I../../lib/poco/Util/include
INCLUDES += -I../../lib/poco/Foundation/include
INCLUDES += -I../../lib/poco/NetSSL_OpenSSL/include
INCLUDES += -I../../lib/glew/include
INCLUDES += -I../../lib/imgui

SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)
CPPFLAGS ?= -std=c++17 -O2 -Wall -Wno-delete-incomplete -Wno-unused-result -Wno-unknown-pragmas -Wno-unused-local-typedefs -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable $(INCLUDES) -DBOXEDWINE_RECORDER -DBOXEDWINE_ZLIB -DBOXEDWINE_HAS_SETJMP -DSDL2=1 "-DGLH=<SDL_opengl.h>" -DBOXEDWINE_OPENGL_SDL -DSIMDE_SSE2_NO_NATIVE -DPOCO_UTIL_NO_JSONCONFIGURATION -DPOCO_UTIL_NO_XMLCONFIGURATION -DBOXEDWINE_POSIX -DBOXEDWINE_OPENGL_IMGUI_V2 $(SDL_CFLAGS) $(EXTRA_CPP_FLAGS)

LDFLAGS = -L./linux_build/lib -lPocoNetSSL -lPocoNet -lPocoCrypto -lPocoUtil -lPocoFoundation -lssl -lcrypto -lpthread -lm -lz -lminizip -lGL -lstdc++ -lstdc++fs $(SDL_LIBS)

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
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(TEST_BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(TEST_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

endif
