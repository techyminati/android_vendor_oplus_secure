####################################################################################
## File: - vendor/fingerprint/goodix_optical/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2019-2022, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint Common Feature Config for Android
##

####################################################################################

TARGET_MODE=debug
LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BUILD_VARIANT),eng)
TARGET_MODE := debug
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
TARGET_MODE := userdebug
else
TARGET_MODE := release
endif


include $(CLEAR_VARS)

LOCAL_MODULE := fingerprint.19811_goodix9638_G6.default

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
    libgf_hal_19811_G6_oplus

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += \
   ./fingerprint.cpp
LOCAL_STATIC_LIBRARIES :=
LOCAL_LDLIBS :=
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
