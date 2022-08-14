#/*************************************************************
# ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
# ** OPLUS_FEATURE_SECURITY_COMMON
# ** File        : Android.mk
# ** Description : NULL
# ** Date        : 2015-08-12 14:39
# ** Author      : Lycan.Wang
# **
# ** ------------------ Revision History: ---------------------
# **      <author>        <date>          <desc>
# **     Lycan.Wang     2015/08/12         NULL
# *************************************************************/

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
					$(TARGET_OUT_HEADERS)/common/inc \
					vendor/qcom/proprietary/securemsm/QSEEComAPI
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        libQSEEComAPI \
        liblog \


LOCAL_MODULE := librpmbengclient
LOCAL_SRC_FILES := rpmbengclient.cpp
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS) -Wno-unused-parameter
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
LOCAL_VENDOR_MODULE:=true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
					$(TARGET_OUT_HEADERS)/common/inc \
					vendor/qcom/proprietary/securemsm/QSEEComAPI
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        libQSEEComAPI \
        liblog \

LOCAL_SHARED_LIBRARIES += librpmbengclient
LOCAL_MODULE := rpmbengclient_test
LOCAL_SRC_FILES := rpmbengclient_test.cpp
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS) -Wno-unused-parameter
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
LOCAL_VENDOR_MODULE:=true

include $(BUILD_EXECUTABLE)
