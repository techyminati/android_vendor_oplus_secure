# 
#   Copyright 2017 Egis Technology Inc.
# 
#   This software is protected by copyright, international
#   treaties and various patents. Any copy, reproduction or otherwise use of
#   this software must be authorized by Egis in a license agreement and
#   include this Copyright Notice and any other notices specified
#   in the license agreement. Any redistribution in binary form must be
#   authorized in the license agreement and include this Copyright Notice
#   and any other notices specified in the license agreement and/or in
#   materials provided with the binary distribution.
#

#SHARED LIBRARIES
LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_MODULE        := libRbsFlow
#LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
#LOCAL_MODULE_SUFFIX := .so
#LOCAL_STRIP_MODULE  := false
#LOCAL_MULTILIB      := 64
#LOCAL_MODULE_TAGS   := optional
#LOCAL_SRC_FILES     := libs/libRbsFlow.so
#LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64
#LOCAL_PROPRIETARY_MODULE := true
#include $(BUILD_PREBUILT)




include $(CLEAR_VARS)

LOCAL_MODULE := libegis_hal
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CFLAGS += -D__LINUX__ -DEGIS_DBG
# LOCAL_CFLAGS += -DAUTHTOKEN_HMAC -DQSEE
LOCAL_MULTILIB      := 64
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    hardware/libhardware/include
#	frameworks/base/include \
#	system/core/include \
#	bionic/libc/include

LOCAL_SRC_FILES :=  \
    src/rbs_oplus.c  \
    src/plat_log_linux.c  \
    src/rbs_fingerprint.c

LOCAL_SHARED_LIBRARIES := liblog  \
                          libutils  \
                          libRbsFlow

LOCAL_STATIC_LIBRARIES := libcutils
LOCAL_LDFLAGS += -nodefaultlibs -lc -lm -ldl
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under, $(LOCAL_PATH))
