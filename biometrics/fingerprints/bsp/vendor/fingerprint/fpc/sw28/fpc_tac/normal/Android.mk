####################################################################################
## File: - fpc\fpc_tac\normal\Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint TAC Android.mk for FPC Android O SW23.2.2
##
## Version: 1.0
## Date created: 18:03:11,22/10/2017
## Author: Ziqing.guo@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Ziqing.guo   2017/10/09        create the file
##  Ziqing.guo   2017/10/22        add FPC_CONFIG_WAKELOCK
##  Ziqing.guo   2017/11/30        add for 1270
##  Bin.Li       2017/12/12        add for mt6771
##  Ran.Chen     2018/03/21        add for fpc_monitor
##  Long.Liu     2018/11/19        modify for 18531 static test
####################################################################################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := fpc_tac_sw28
LOCAL_VENDOR_MODULE:=true
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := fpc
LOCAL_CFLAGS := -Wall -O2 -DLOG_TAG='"fpc_tac"' -std=c11 -Werror

ifneq ($(filter 8.%,$(PLATFORM_VERSION)),)
FPC_STORE_IN_DATA_DIR := true
endif
ifneq ($(filter 7.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
FPC_STORE_IN_DATA_DIR := true
endif

ifneq ($(filter 6.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
FPC_STORE_IN_DATA_DIR := true
endif

ifeq ($(FPC_STORE_IN_DATA_DIR),true)
LOCAL_CFLAGS += -DFPC_STORE_IN_DATA_DIR
endif

ifeq ($(FPC_CONFIG_SWIPE_ENROL),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SWIPE_ENROL
endif

ifeq ($(SIDE_FPC_ENABLE),1)
LOCAL_CFLAGS         += -DSIDE_FPC_ENABLE
endif

ifeq ($(FPC_CHECK_BROKEN),1)
LOCAL_CFLAGS         += -DFPC_CHECK_BROKEN
endif
#ifeq ($(FPC_SYSTEM_LIB),)
# In Android O we should place the library in /vendor/lib64 instead of /system/lib64
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := both
#endif

LOCAL_C_INCLUDES := \
                    $(LOCAL_PATH)/inc/ \
                    $(LOCAL_PATH)/inc/kpi \
                    $(LOCAL_PATH)/inc/hw_auth \
                    $(LOCAL_PATH)/inc/fido_auth \
                    $(LOCAL_PATH)/inc/android \
                    $(LOCAL_PATH)/../interface \
                    $(LOCAL_PATH)/../../fpc_hal/fingerprint_common \
                    $(LOCAL_PATH)/../../fpc_hal/fingerprint_common/nav \
                    $(LOCAL_PATH)/../../../../hwbinder \
                    $(LOCAL_PATH)/../../../../hwbinder/fingerprint_type/include

ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_C_INCLUDES += \
                    $(TOPDIR)vendor/qcom/proprietary/securemsm/QSEEComAPI \
                    $(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM_IF) \
                    $(LOCAL_PATH_QSEECOMAPI) \
                    system/core/libion/kernel-headers \
                    system/core/libion/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
else
LOCAL_C_INCLUDES += \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
endif

LOCAL_SRC_FILES := \
                   src/fpc_irq_device.c \
                   src/fpc_sysfs.c \
                   src/fpc_tee.c \
                   src/kpi/fpc_tee_kpi.c \
                   ../interface/fpc_error_str.c

LOCAL_CFLAGS += -DFPC_TA_KEYMASTER_APP_PATH='"/firmware/image"'

ifndef FPC_CONFIG_NO_SENSOR
LOCAL_SRC_FILES  += src/fpc_tee_sensor.c
else
LOCAL_SRC_FILES  += src/fpc_tee_sensor_dummy.c
endif

ifndef FPC_CONFIG_NO_ALGO
LOCAL_SRC_FILES  += src/fpc_tee_bio.c
else
LOCAL_SRC_FILES  += src/fpc_tee_bio_dummy.c
endif

ifdef FPC_CONFIG_NORMAL_SPI_RESET
LOCAL_SRC_FILES += src/fpc_reset_device.c
LOCAL_CFLAGS += -DFPC_CONFIG_NORMAL_SPI_RESET
endif

ifdef FPC_CONFIG_NORMAL_SENSOR_RESET
ifndef FPC_CONFIG_NORMAL_SPI_RESET
LOCAL_SRC_FILES += src/fpc_reset_device.c
endif
LOCAL_CFLAGS += -DFPC_CONFIG_NORMAL_SENSOR_RESET
endif

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING -DFPC_CONFIG_DEBUG
endif

ifeq ($(FPC_CONFIG_TA_FS),1)
#LOCAL_SRC_FILES += src/fpc_tee_secure_storage.c
endif

ifeq ($(FPC_CONFIG_TA_DB_BLOB),1)
LOCAL_SRC_FILES += src/fpc_tee_host_storage.c
endif

ifeq ($(FPC_CONFIG_WAKE_LOCK),1)
LOCAL_CFLAGS += -DFPC_CONFIG_WAKE_LOCK
endif

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_CFLAGS += -DFPC_CONFIG_HW_AUTH
LOCAL_SRC_FILES += src/hw_auth/fpc_tee_hw_auth.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/hw_auth
endif

ifeq ($(FPC_CONFIG_ENGINEERING),1)
LOCAL_SRC_FILES += src/engineering/fpc_tee_engineering.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/engineering
LOCAL_CFLAGS += -DFPC_CONFIG_ENGINEERING
endif

ifeq ($(FPC_CONFIG_SENSORTEST),1)
LOCAL_SRC_FILES += src/sensortest/fpc_tee_sensortest.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/sensortest
LOCAL_CFLAGS += -DFPC_CONFIG_SENSORTEST
endif

ifeq ($(FPC_CONFIG_NAVIGATION),1)
LOCAL_SRC_FILES += src/navigation/fpc_tee_nav.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/navigation
LOCAL_CFLAGS += -DFPC_CONFIG_NAVIGATION
endif

LOCAL_SRC_FILES += src/monitor/fpc_monitor.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/monitor

ifeq ($(FPC_CONFIG_FINGER_LOST_INTERRUPT),1)
LOCAL_CFLAGS += -DFPC_CONFIG_FINGER_LOST_INTERRUPT=1
endif

ifdef FPC_CONFIG_AFD_STAY_IN_SLEEP
LOCAL_CFLAGS += -DFPC_CONFIG_AFD_STAY_IN_SLEEP
endif

ifdef FPC_CONFIG_LOGGING_IN_RELEASE
LOCAL_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE
ifdef FPC_CONFIG_LOGGING_IN_RELEASE_FILE
LOCAL_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE_FILE
LOCAL_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE=$(FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE)
endif
endif

ifdef FPC_CONFIG_APNS
LOCAL_CFLAGS    += -DFPC_CONFIG_APNS
#LOCAL_SRC_FILES += src/fpc_tee_pn.c
endif

ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
LOCAL_CFLAGS    += -DFPC_CONFIG_ALLOW_PN_CALIBRATE
endif

ifeq ($(FPC_CONFIG_FORCE_SENSOR),1)
LOCAL_CFLAGS += -DFPC_CONFIG_FORCE_SENSOR=1
endif

ifeq ($(FPC_CONFIG_SEND_RESET),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SEND_RESET=1
endif

ifeq ($(FP_CONFIG_SHMBRIDGE_ION_FUNCTION),1)
LOCAL_CFLAGS    += -DFP_CONFIG_SHMBRIDGE_ION_FUNCTION
endif

ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
endif

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libhardware

LOCAL_LDLIBS += -llog -lhardware -lcutils

LOCAL_EXPORT_C_INCLUDE_DIRS += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../interface \
    $(LOCAL_PATH)/inc/hw_auth \
    $(LOCAL_PATH)/inc/fido_auth \
    $(LOCAL_PATH)/inc/sensortest \
    $(LOCAL_PATH)/inc/engineering \
    $(LOCAL_PATH)/inc/kpi \
    $(LOCAL_PATH)/inc/navigation

ifeq ($(FPC_TEE_RUNTIME), TBASE)
include $(LOCAL_PATH)/../normal/platform/tbase/tbase.mk
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
include $(LOCAL_PATH)/../normal/platform/qsee/qsee.mk
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
include $(LOCAL_PATH)/../normal/platform/android/droid.mk
else ifeq ($(FPC_TEE_RUNTIME), ISEE)
LOCAL_CFLAGS += -DFPC_CONFIG_TEE_ISEE
include $(LOCAL_PATH)/../normal/platform/isee/isee.mk
else
$(error "Unknown FPC_TEE_RUNTIME=$(FPC_TEE_RUNTIME)")
endif

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_EXPORT_C_INCLUDE_DIRS)

LOCAL_HEADER_LIBRARIES += libhardware_headers

include $(BUILD_STATIC_LIBRARY)
