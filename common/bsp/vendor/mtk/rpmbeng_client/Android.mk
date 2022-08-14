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
# **     Long.Liu        2018/12/04      create file
# **     Bin.Li          2019/01/01      add for alikey write key rely on secure on.
# **     Meilin.Zhou     2021/11/13      add for rpmb key provision only enable for mt6983.
# *************************************************************/

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -DCONFIG_WRITE_KEY_RELY_FUSE
ifeq ($(strip $(COMPILE_PLATFORM)),k6983v1_64)
$(info configuration only avaiable for $(COMPILE_PLATFORM))
LOCAL_CFLAGS += -DCONFIG_ENABLE_RPMB_RELY_OPLUSRESERVE
endif
LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        liblog \

LOCAL_MODULE := librpmbengclient
LOCAL_SRC_FILES := rpmbengclient.cpp
LOCAL_SRC_FILES += rpmbeng_oplusreserve_status.cpp
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := mtk
LOCAL_VENDOR_MODULE:=true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        liblog \

LOCAL_SHARED_LIBRARIES += librpmbengclient
LOCAL_MODULE := rpmbengclient_test
LOCAL_SRC_FILES := rpmbengclient_test.cpp
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := mtk
LOCAL_VENDOR_MODULE:=true

include $(BUILD_EXECUTABLE)
