ifeq ($(TARGET_PRODUCT),bengal)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_C_INCLUDES += $(TOPDIR)system/core/libion/include
LOCAL_C_INCLUDES += $(TOPDIR)system/core/libion/kernel-headers
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(TOPDIR)vendor/qcom/proprietary/securemsm/QSEEComAPI

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := first
LOCAL_CFLAGS += -Wall -Werror

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libutils \
	libQSEEComAPI

LOCAL_SHARED_LIBRARIES += libion

LOCAL_MODULE := libets_teeclient_v2
LOCAL_SRC_FILES := fp_client_lib.c
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
#LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
LOCAL_PRELINK_MODULE := false
#LOCAL_MODULE_OWNER := qcom
LOCAL_LDLIBS := -llog
#include $(BUILD_EXECUTABLE)
#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)
endif