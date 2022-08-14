####################################################################################
## File: - vendor/fingerprint/goodix_optical/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2019-2022, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint Common Feature Config for Android
##
## Version: 1.0
## Date created: 18:03:11,05/16/2019
## Author: Bangxiong.Wu@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>       <data>            <desc>
##  Bangxiong.Wu   2019/05/16        create the file
##  Bangxiong.Wu   2019/05/16        Copy from goodix folder(HASH:f8cd6be70a1af989dab01e321df3bb27c88464f4) to goodix_G3 folder
##  Bangxiong.Wu   2019/05/16        Modify for G3.0
##  oujinrong      2019/07/01        remove build type
####################################################################################

LOCAL_PATH := $(call my-dir)

MODE := release

include $(CLEAR_VARS)

LOCAL_MODULE := fingerprint.20828_goodix9678.default

CONFIG_INCLUDE_DIRS :=
CONFIG_INCLUDE_DIRS += \
$(LOCAL_PATH)/inc \
$(LOCAL_PATH)/hal/oneplus

CONFIG_LIBRARIES :=
CONFIG_MACROS :=


LOCAL_C_INCLUDES += $(CONFIG_INCLUDE_DIRS)
LOCAL_CFLAGS += $(CONFIG_MACROS)
LOCAL_LDFLAGS += -nodefaultlibs -lc -lm -ldl
LOCAL_LDFLAGS += $(CONFIG_LIBRARIES)
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SHARED_LIBRARIES :=
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libc \
    libhwbinder \
    libhidltransport \
    libbinder \
    libutils \
    libdl \
    libcutils \
    liblog \
    libhardware \
    libion

LOCAL_SHARED_LIBRARIES += \
    libgf_hal_20828_G6_7_oplus

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += \
   ./fingerprint.cpp
LOCAL_STATIC_LIBRARIES :=

LOCAL_LDLIBS :=
LOCAL_MODULE_TAGS := optional




include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
