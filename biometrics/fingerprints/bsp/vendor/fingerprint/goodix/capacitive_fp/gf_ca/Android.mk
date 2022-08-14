#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

LOCAL_PATH := $(call my-dir)
SUPPORT_WHITEBOX_CA_ENV_CHECK := no
SUPPORT_WHITEBOX_TEE_REE := no
TALOG_DUMP_TO_CA_SUPPORT := no

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libwhitebox
LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES_64 := \
    libs/arm64-v8a/libwhitebox.a
LOCAL_SRC_FILES_32 := \
    libs/armeabi-v7a/libwhitebox.a

LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)

LOCAL_MODULE := libgf_ca
LOCAL_VENDOR_MODULE := true

ifeq ($(TARGET_BUILD_VARIANT),eng)
MODE := debug
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
MODE := userdebug
else
MODE := release
endif

LOCAL_CFLAGS += -D_POSIX_THREADS
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -DLOG_ANDROID -UNDEBUG

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../public \
    $(LOCAL_PATH)/../include \
    $(TOPDIR)vendor/qcom/proprietary/securemsm/QSEEComAPI \
    $(LOCAL_PATH_QSEECOMAPI) \
    system/core/libion/kernel-headers \
    system/core/libion/include \
    $(TOPDIR)kernel/msm-$(TARGET_KERNEL_VERSION)/drivers/staging/android/uapi \
    $(LOCAL_PATH)/../../../hwbinder/fingerprint_type/include \
    $(TOPDIR)bionic/libc/include \
    $(TOPDIR)external/clang/lib/Headers \
    $(TOPDIR)kernel/msm-$(TARGET_KERNEL_VERSION)/drivers/staging/android/uapi \
    $(TOPDIR)kernel/msm-$(TARGET_KERNEL_VERSION)/include/uapi \
    $(TOPDIR)kernel/msm-$(TARGET_KERNEL_VERSION)/include/uapi/linux

LOCAL_CFLAGS += -std=c99

ifeq ($(call is-vendor-board-platform,QCOM), true)
$(warning " there is QCOM, $(TOPDIR)")
################qsee########################
LOCAL_SRC_FILES := qsee/gf_ca_entry.c \
                   qsee/gf_keymaster.c\
                   qsee/gf_sec_ion.c\
                   qsee/gf_ca_common.c \
                   qsee/gf_ca_dump_log.c

LOCAL_CFLAGS += -D__TEE_QSEE
LOCAL_CFLAGS += -DTARGET_ION_ABI_VERSION=2

LOCAL_LDLIBS += -lQSEEComAPI -lhardware -lcutils -lion
LOCAL_SHARED_LIBRARIES += \
        libQSEEComAPI \
        libion \
        liblog
################qsee end########################
else
################Tbase########################
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -D__TEE_TRUSTONIC

LOCAL_SRC_FILES := tbase/gf_ca_entry.c
LOCAL_SHARED_LIBRARIES += \
    libMcClient \
    liblog
endif
################Tbase end########################

ifeq ($(SUPPORT_WHITEBOX_TEE_REE), yes)
    ifneq ($(MODE), release)
        LOCAL_CFLAGS += -DWHITEBOX_DYNAMIC_CONTROL
    endif

    LOCAL_CFLAGS += -DWHITEBOX_TEE_REE
    LOCAL_STATIC_LIBRARIES := libwhitebox
endif

include $(BUILD_STATIC_LIBRARY)

$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
