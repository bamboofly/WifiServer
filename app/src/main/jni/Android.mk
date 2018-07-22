#include $(call all-subdir-makefiles)
#PROJ_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#include $(PROJ_PATH)/yuv/Android.mk
#include $(PROJ_PATH)/property/Android.mk
#include $(PROJ_PATH)/serial_port/Android.mk
#include $(PROJ_PATH)/led/Android.mk

LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_MODULE := libffmpeg
#LOCAL_SRC_FILES := libffmpeg.so
#include $(PREBUILT_SHARED_LIBRARY)


PATH_TEST := /home/bamboofly/Android/android-ndk-r14b/build/core
$(warning $(PATH_TEST),$(call my-dir))





include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_codec
LOCAL_SRC_FILES := lib/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

$(warning 'hello2',$(call my-dir))
ifeq ($(call my-dir),$(PATH_TEST))

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_filter
LOCAL_SRC_FILES := lib/libavfilter.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_format
LOCAL_SRC_FILES := lib/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_util
LOCAL_SRC_FILES := lib/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_resample
LOCAL_SRC_FILES := lib/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg_scale
LOCAL_SRC_FILES := lib/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := flv2yuv
LOCAL_SRC_FILES := $(LOCAL_PATH)/ffmpeg_jni.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/

#LOCAL_CFLAGS += -undefined -nostdlib -Bsymbolic

#LOCAL_SHARED_LIBRARIES := ffmpeg
LOCAL_STATIC_LIBRARIES := ffmpeg_format ffmpeg_codec ffmpeg_filter ffmpeg_scale ffmpeg_resample ffmpeg_util
#LOCAL_STATIC_LIBRARIES := ffmpeg_scale

LOCAL_LDLIBS += -llog -lc -lm -lz -ldl -llog
$(warning 'aaa',$(LOCAL_PATH))
include $(BUILD_SHARED_LIBRARY)

$(warning 'hello',$(call my-dir))

#include $(CLEAR_VARS)

#include $(LOCAL_PATH)/property/Android.mk

endif