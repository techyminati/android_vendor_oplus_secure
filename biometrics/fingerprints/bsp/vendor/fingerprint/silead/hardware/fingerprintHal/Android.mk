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
    HAL_BASE_DIR_NAME := $(HAL_CUST_DIR_NAME)
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

include $(BUILD_SHARED_LIBRARY)