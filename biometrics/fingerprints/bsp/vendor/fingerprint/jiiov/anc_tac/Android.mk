LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := anc_ca_test
LOCAL_MODULE_TAGS := tests
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := anc

#include $(TOPDIR)vendor/oplus/secure/biometrics/fingerprints/bsp/config/platform/qcom/config.mk
include $(LOCAL_PATH)/../config.mk

ifeq ($(ANC_CONFIG_DEBUG), TRUE)
LOCAL_CFLAGS += -DANC_DEBUG
endif

ifeq ($(ANC_CONFIG_USE_VIRTUAL_SENSOR), TRUE)
LOCAL_CFLAGS += -DVIRTUAL_SENSOR
endif
ifeq ($(ANC_CONFIG_LOAD_BASE_IMAGE_FROM_RAW_FILE), TRUE)
LOCAL_CFLAGS += -DLOAD_BASE_IMAGE_FROM_RAW_FILE
endif

ifeq ($(ANC_CONFIG_LOCAL_TEE_PLATFORM), TRUSTONIC)
LOCAL_CFLAGS += -DANC_SENSOR_SPI_MTK
endif

LOCAL_ANC_TAC_DIR  := .
LOCAL_ANC_TAC_PATH := $(LOCAL_PATH)/$(LOCAL_ANC_TAC_DIR)
include $(LOCAL_ANC_TAC_PATH)/anc_tac.mk

# ta code invoke hal code
# ifeq ($(ANC_CONFIG_LOCAL_TEE_PLATFORM), REE)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../anc_hal/common/inc/
LOCAL_SRC_FILES += ../anc_hal/common/src/anc_hal_sensor_device.c
# endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/
LOCAL_SRC_FILES += main.c
LOCAL_SRC_FILES += test.c








include $(BUILD_EXECUTABLE)

