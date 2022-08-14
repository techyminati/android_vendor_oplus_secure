####################################################################################
## File: - fpc\fpc_hal\fingerprint_common\Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint TEE hal Android.mk for FPC Android O SW23.2.2
##
## Version: 1.0
## Date created: 18:03:11,22/10/2017
## Author: Ziqing.guo@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Ziqing.guo   2017/10/09        create the file
##  Ziqing.guo   2017/10/22        add FPC_CONFIG_WAKELOCK
####################################################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fpc_hal_common_sw28
LOCAL_VENDOR_MODULE:=true
LOCAL_CFLAGS := -Wall -Werror \
                -DLOG_TAG='"fpc_hal"'

LOCAL_CONLYFLAGS := -std=c99

LOCAL_SRC_FILES := fpc_tee_hal.c \
                   fpc_worker.c \
                   fpc_hal_input_device.c

ifneq ($(filter 7.%,$(PLATFORM_VERSION)),)
LOCAL_CFLAGS += -DPRE_TREBLE_HAL
endif

ifneq ($(filter 6.%,$(PLATFORM_VERSION)),)
LOCAL_CFLAGS += -DPRE_TREBLE_HAL
endif

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING
endif

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_CFLAGS    += -DFPC_CONFIG_HW_AUTH
endif

ifeq ($(FPC_TA_HEAP_DEBUG),1)
LOCAL_CFLAGS += -DFPC_TA_HEAP_DEBUG
endif

# Enable authentication framework support
ifeq ($(FPC_CONFIG_FIDO_AUTH),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_FIDO_AUTH
    ifeq ($(FPC_CONFIG_FIDO_AUTH_VER_GMRZ),1)
    LOCAL_CFLAGS         += -DFPC_CONFIG_FIDO_AUTH_VER_GMRZ
    else
    LOCAL_CFLAGS         += -DFPC_CONFIG_FIDO_AUTH_VER_GENERIC
    endif
endif


ifeq ($(FPC_CONFIG_ENGINEERING),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_ENGINEERING
LOCAL_SRC_FILES      += fpc_hal_ext_engineering.c
endif

ifeq ($(FPC_CONFIG_WAKE_LOCK),1)
LOCAL_CFLAGS           += -DFPC_CONFIG_WAKE_LOCK
endif

ifeq ($(FPC_CONFIG_SENSORTEST),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SENSORTEST
LOCAL_SRC_FILES += fpc_hal_ext_sensortest.c
endif

# Enable Qualcomm authentication framework support
ifeq ($(FPC_CONFIG_QC_AUTH),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_QC_AUTH
LOCAL_SRC_FILES      += fpc_hal_ext_authenticator.c
endif

# Navigation
ifeq ($(FPC_CONFIG_NAVIGATION),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_NAVIGATION
LOCAL_SRC_FILES      += nav/fpc_hal_navigation.c \

# SW Sense Touch
ifeq ($(FPC_CONFIG_NAVIGATION_FORCE_SW),1)
LOCAL_CFLAGS          += -DFPC_CONFIG_NAVIGATION_FORCE_SW=1
LOCAL_C_INCLUDES += $(LOCAL_PATH)/sensetouch
endif
ifeq ($(SIDE_FPC_ENABLE),1)
LOCAL_CFLAGS         += -DSIDE_FPC_ENABLE
endif
ifeq ($(FPC_CHECK_BROKEN),1)
LOCAL_CFLAGS         += -DFPC_CHECK_BROKEN
endif
endif

# Storing image
ifeq ($(FPC_TEE_STORE_IMAGE),1)
LOCAL_CFLAGS         += -DFPC_TEE_STORE_IMAGE
endif

# Retry on no match feature
ifneq ($(FPC_CONFIG_RETRY_MATCH_TIMEOUT),)
LOCAL_CFLAGS         += -DFPC_CONFIG_RETRY_MATCH_TIMEOUT=$(FPC_CONFIG_RETRY_MATCH_TIMEOUT)
endif

ifeq ($(FPC_CONFIG_SWIPE_ENROL),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SWIPE_ENROL
endif

# HW Sense Touch
ifeq ($(FPC_CONFIG_FORCE_SENSOR),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_FORCE_SENSOR=1
LOCAL_SRC_FILES      += sensetouch/fpc_hal_ext_sense_touch.c \
                        sensetouch/fpc_hal_sense_touch.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/sensetouch \
LOCAL_CFLAGS += -DSENSE_TOUCH_CALIBRATION_PATH='"$(FPC_CONFIG_SENSE_TOUCH_CALIBRATION_PATH)"'
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/../include \
                    $(LOCAL_PATH)/nav \
		    hardware/libhardware/include \
		    $(LOCAL_PATH)/sensetouch \
		    $(LOCAL_PATH)/../../fpc_tac/interface \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc/hw_auth \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc/fido_auth \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc/sensortest \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc/engineering \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc/kpi \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc/navigation \
                    $(LOCAL_PATH)/../../fpc_tac/normal/inc/monitor \
                    $(LOCAL_PATH)/../../../../hwbinder \
                    $(LOCAL_PATH)/../../../../hwbinder/fingerprint_type/include

LOCAL_SHARED_LIBRARIES := liblog \
                          libbinder \
                          libutils
LOCAL_LDLIBS += -llog -lbinder -lcutils

ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)
LOCAL_CFLAGS           += -DTRUSTONIC_TEE_SUPPORT
endif

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES +=  libQSEEComAPI libion
else ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libMcClient
endif

LOCAL_STATIC_LIBRARIES += libcutils fpc_tac_sw28

include $(BUILD_STATIC_LIBRARY)
