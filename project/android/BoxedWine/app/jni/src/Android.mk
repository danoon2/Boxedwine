LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2

LOCAL_CFLAGS := -DSDL2 -DBOXEDWINE_HAS_SETJMP -DBOXEDWINE_ZLIB

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../include

# Add your application source files here...
FILE_LIST := $(wildcard $(LOCAL_PATH)/../../../../../../source/sdl/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/sdl/singleThreaded/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../platform/linux/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/emulation/cpu/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/emulation/cpu/common/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/emulation/cpu/normal/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/emulation/softmmu/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/io/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/kernel/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/kernel/devs/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/kernel/proc/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/kernel/loader/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/util/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/opengl/sdl/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../source/opengl/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../lib/zlib/contrib/minizip/ioapi.c)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../lib/zlib/contrib/minizip/mztools.c)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../lib/zlib/contrib/minizip/unzip.c)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../../../../../../lib/zlib/contrib/minizip/zip.c)

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -lz

include $(BUILD_SHARED_LIBRARY)
