####################################################################################
## File: -vendor/fingerprint/silead_optical/hardware/fingerprintHal/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      DD235 HAL makefile, common feature
##
## Version: 1.0
## Date created: 23:30,03/23/2019
## Author: Bangxiong.Wu@BSP.Fingerprint.Basic
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>          <data>        <desc>
##  Bangxiong.Wu      2019/03/23    create the file
##  Bangxiong.Wu      2019/03/23    add hypnusd MACRO for 18073
##  Bangxiong.Wu      2019/04/10    Add include path and delete repeat head file
####################################################################################

LOCAL_PATH := $(call my-dir)

#------------------------------------------------
HAL_CUST_TYPE :=
ifneq ($(strip $(SIL_CUST_TYPE)),)
    HAL_CUST_TYPE := $(shell echo $(SIL_CUST_TYPE) | tr A-Z a-z)
endif

HAL_BASE_DIR_NAME :=
HAL_CUST_DIR_PATH := $(shell find $(LOCAL_PATH)/cust_$(HAL_CUST_TYPE) ! -path "." -type d 2>/dev/null)
HAL_CUST_DIR_NAME := $(strip $(foreach type, $(HAL_CUST_DIR_PATH), $(shell basename $(type) | tr A-Z a-z)))
ifeq ($(HAL_CUST_DIR_NAME),cust_$(HAL_CUST_TYPE))
#HAL_BASE_DIR_NAME := $(HAL_CUST_DIR_NAME)
endif

$(warning $(HAL_BASE_DIR_NAME))

#------------------------------------------------
include $(CLEAR_VARS)

# if $(PLATFORM_SDK_VERSION) is android8.0 (26) or higher, enable vendor img
local_silead_support_vendor := $(shell if [ $(PLATFORM_SDK_VERSION) -ge 26 ]; then echo yes; else echo no; fi)
ifeq ($(strip $(local_silead_support_vendor)),yes)
    LOCAL_VENDOR_MODULE := true
endif

ifeq ($(strip $(PLATFORM_SDK_VERSION)),23)      #android6
    LOCAL_CFLAGS := -DANDROID6
endif

ifneq ($(strip $(SILEAD_FP_HAL_MODULE_ID)),)
    LOCAL_CFLAGS := -DSILEAD_FP_HAL_MODULE_ID="\"$(SILEAD_FP_HAL_MODULE_ID)\""
endif
LOCAL_CFLAGS += -DSL_FP_FEATURE_OPLUS_CUSTOMIZE
LOCAL_CFLAGS += -DSL_FP_FEATURE_OPLUS_CUSTOMIZE_OPTIC
LOCAL_CFLAGS += -Wno-unused-function

#------------------------------------------------
#select hypnusd method
#------------------------------------------------
ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
endif

#------------------------------------------------
#build setting
#------------------------------------------------
ifneq ($(strip $(SILEAD_FP_HAL_LIB_NAME)),)
LOCAL_MODULE := $(SILEAD_FP_HAL_LIB_NAME)
else
LOCAL_MODULE := fingerprint.silead.default
endif

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SHARED_LIBRARIES := liblog libutils libhardware libsl_fp_impl
LOCAL_REQUIRED_MODULES := libsl_fp_impl
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := silead
ifneq ($(strip $(HAL_BASE_DIR_NAME)),)
    LOCAL_SRC_FILES := $(HAL_BASE_DIR_NAME)/fingerprint.cpp
else
    LOCAL_SRC_FILES := fingerprint.cpp
endif
LOCAL_C_INCLUDES := $(LOCAL_PATH)\
    $(LOCAL_PATH)/../fingerprint \
    $(LOCAL_PATH)/../fingerprint/include \
    $(LOCAL_PATH)/../fingerprint/cust_dd235
include $(BUILD_SHARED_LIBRARY)
