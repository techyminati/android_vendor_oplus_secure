#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

LOCAL_PATH := $(call my-dir)
CHIP_SERIES := milan_f_series
SUPPORT_TEST_CODE := yes
TEE := qsee
CPU_TYPE := qualcomm
TARGET_PLATFORM := sdm6125
ANDROID_VERSION := android9.0
BUILD_TIME := 2019.11.25_18:37:50
GIT_COMMIT_ID := 3f168ed3d8fb05ed5d66cc60d101766404c25485
GIT_BRANCH := oplus_cc122_gf5658
PACKAGE_VERSION_NAME := V00.04.05.01
PACKAGE_VERSION_CODE := 10000

define all-c-files-under
$(patsubst ./%,%, $(shell cd $(LOCAL_PATH) ;\
        find -L $(1) -name "*.c" -and -not -name ".*"))
endef

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libgf_algo
LOCAL_VENDOR_MODULE := true

ifeq ($(TARGET_BUILD_VARIANT),eng)
MODE := debug
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
MODE := userdebug
else
MODE := release
endif

LOCAL_SRC_FILES_64 := \
    algo/$(MODE)/arm64-v8a/libgf_algo.a
LOCAL_SRC_FILES_32 := \
    algo/$(MODE)/armeabi-v7a/libgf_algo.a

LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libgf_hal
LOCAL_VENDOR_MODULE := true
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/public \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/../public \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../gf_ca \
    $(shell find $(LOCAL_PATH) -maxdepth 2 -type d) \
    $(LOCAL_PATH)/../../../hwbinder \
    $(LOCAL_PATH)/../../../hwbinder/fingerprint_type/include

LOCAL_CFLAGS += -D__HAL__
ifeq ($(ANDROID_VERSION),android6.0)
LOCAL_CFLAGS += -D__ANDROID_M
else ifeq ($(ANDROID_VERSION),android7.0)
LOCAL_CFLAGS += -D__ANDROID_N
else ifeq ($(ANDROID_VERSION), android8.0)
LOCAL_CFLAGS += -D__ANDROID_O
else ifeq ($(ANDROID_VERSION), android9.0)
LOCAL_CFLAGS += -D__ANDROID_P
endif

ifeq ($(CPU_TYPE), mtk)
LOCAL_CFLAGS += -D__PLATFORM_MTK
endif

# GF_LOG_LEVEL is 4, will output verbose, debug, info, error log
# GF_LOG_LEVEL is 3, will output debug, info, error log
# GF_LOG_LEVEL is 2, will output info, error log
# GF_LOG_LEVEL is 1, only output error log
ifeq ($(OBSOLETE_KEEP_ADB_SECURE), 1)
LOCAL_CFLAGS += -DGF_LOG_LEVEL=2
else
LOCAL_CFLAGS += -DGF_LOG_LEVEL=3
LOCAL_CFLAGS += -DGOODIX_CONFIG_DUMP
endif
ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
endif

ifeq ($(FP_SUPPORT_SIDE),y)
LOCAL_CFLAGS         += -DGOODIX_CHECK_BROKEN
endif

LOCAL_CFLAGS += \
    -DGF_TARGET_MODE=\"$(MODE)\" \
    -DGF_PACKAGE_VERSION_CODE=\"$(PACKAGE_VERSION_CODE)\" \
    -DGF_PACKAGE_VERSION_NAME=\"$(PACKAGE_VERSION_NAME)\" \
    -DGF_GIT_BRANCH=\"$(GIT_BRANCH)\" \
    -DGF_COMMIT_ID=\"$(COMMIT_ID)\" \
    -DGF_BUILD_TIME=\"$(BUILD_TIME)\" \
    -DGF_ANDROID_VERSION=\"$(ANDROID_VERSION)\" \
    -DGF_TARGET_PLATFORM=\"$(TARGET_PLATFORM)\"

LOCAL_SRC_FILES := \
    gf_hal.c \
    gf_hal_common.c \
    gf_hal_device.c \
    gf_hal_frr_database.c \
    gf_hal_timer.c \
    gf_queue.c \
    gf_hal_test.c \
    gf_hal_dump.c \
    gf_hal_test_utils.c \
    IsScreenInteractive.cpp \
    IPowerManager.cpp \
    gf_sec_ptrace.c

ifeq (trustone, ${TEE})
LOCAL_SRC_FILES += \
    $(LOCAL_PATH)/platform/${TEE}/gf_hal_trustone.c \
    $(LOCAL_PATH)/platform/${TEE}/gf_trustone_entry.c
endif

LOCAL_SRC_FILES += $(call all-c-files-under, algo/snr algo/dump milan_a_series milan_an_series milan oswego_m ../include ../public)

ifeq ($(CHIP_SERIES), simulator)
LOCAL_SRC_FILES := $(shell echo $(LOCAL_SRC_FILES) | sed 's/gf_hal_device\.c//g')
LOCAL_SRC_FILES += $(call all-c-files-under, simulator)
LOCAL_CFLAGS += -D__SIMULATOR
endif

ifeq ($(SUPPORT_TEST_CODE), no)
LOCAL_SRC_FILES := $(shell echo $(LOCAL_SRC_FILES) |  sed 's/[^ ]*test[^ ]*\.c//g')
LOCAL_SRC_FILES += \
    gf_hal_test_utils.c \
    gf_hal_test_stub.c
endif

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/milan_a_series \
    $(LOCAL_PATH)/milan_an_series \
    $(LOCAL_PATH)/milan \
    $(LOCAL_PATH)/oswego_m

LOCAL_SHARED_LIBRARIES := \
    libhardware \
    libcutils

LOCAL_STATIC_LIBRARIES := libgf_ca libgf_algo
LOCAL_LDLIBS := -llog -lhardware -lbinder -lutils -lcutils
ifeq ($(ANDROID_VERSION), $(filter $(ANDROID_VERSION), android8.0 android9.0))
    LOCAL_LDFLAGS += -nodefaultlibs -lc -lm -ldl
endif

include $(BUILD_STATIC_LIBRARY)
