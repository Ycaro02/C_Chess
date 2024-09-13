LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

# SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../rsc/lib/install/include

# Define SDL_DISABLE_IMMINTRIN_H for ARM architectures
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
    LOCAL_CFLAGS += -DSDL_DISABLE_IMMINTRIN_H
endif
ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
    LOCAL_CFLAGS += -DSDL_DISABLE_IMMINTRIN_H
endif

$(info LOCAL_C_INCLUDES is $(LOCAL_C_INCLUDES))

# Define source directories
SRC_DIRS := $(LOCAL_PATH)/../../../../../libft/libft \
            $(LOCAL_PATH)/../../../../../libft/list \
            $(LOCAL_PATH)/../../../../../libft/printf_fd \
            $(LOCAL_PATH)/../../../../../libft/parse_flag \
            $(LOCAL_PATH)/../../../../../libft/stack_string \
            $(LOCAL_PATH)/../../../../../src

# Initialize the source files variable
LOCAL_SRC_FILES :=

# Loop through each source directory and add the source files
$(foreach dir,$(SRC_DIRS), $(eval LOCAL_SRC_FILES += $(wildcard $(dir)/*.c)))

$(info SRC_DIRS is $(SRC_DIRS))
$(info LOCAL_SRC_FILES is $(LOCAL_SRC_FILES))

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid

include $(BUILD_SHARED_LIBRARY)

# Include SDL2
include $(CLEAR_VARS)
LOCAL_MODULE := SDL2
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../src/main/jniLibs/$(TARGET_ARCH_ABI)/libSDL2.so
include $(PREBUILT_SHARED_LIBRARY)

# Include SDL2_ttf
include $(CLEAR_VARS)
LOCAL_MODULE := SDL2_ttf
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../src/main/jniLibs/$(TARGET_ARCH_ABI)/libSDL2_ttf.so
include $(PREBUILT_SHARED_LIBRARY)