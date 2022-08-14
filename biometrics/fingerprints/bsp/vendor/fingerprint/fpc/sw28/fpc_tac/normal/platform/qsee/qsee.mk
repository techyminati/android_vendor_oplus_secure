#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
# QSEE specific includes for the TAC
# Note that LOCAL_PATH is set to the location of the normal/Android.mk that
# includes this file
# =============================================================================
LOCAL_PATH_PLATFORM    := ../normal/platform/qsee
LOCAL_PATH_PLATFORM_IF := ../interface/platform/qsee

# =============================================================================
LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM_IF) \
        system/core/libion/include \
        system/core/libion/kernel-headers \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_SRC_FILES += \
        $(LOCAL_PATH_PLATFORM)/src/fpc_qsee_tac.c

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_SRC_FILES += $(LOCAL_PATH_PLATFORM)/src/hw_auth/fpc_tee_hw_auth_qsee.c
endif

ifndef FPC_CONFIG_LKM_CLASS
FPC_CONFIG_LKM_CLASS=platform
endif

FPC_REE_DEVICE_PATH := /sys/bus/$(FPC_CONFIG_LKM_CLASS)/devices

# Path to the Kernel REE device driver sysfs interface
LOCAL_CFLAGS += -DFPC_REE_DEVICE_ALIAS_FILE='"modalias"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_NAME='"fpc_fpc1020"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_PATH='"$(FPC_REE_DEVICE_PATH)"'

LOCAL_CFLAGS += -Werror -Wall
LOCAL_CFLAGS += -DFPC_TAC_APPLICATION_NAME='"fingerprint"'

# Enable Qualcomm authentication framework support
ifeq ($(FPC_CONFIG_QC_AUTH),1)
LOCAL_CFLAGS     += -DFPC_CONFIG_QC_AUTH
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM)/inc/qc_auth
LOCAL_SRC_FILES  += $(LOCAL_PATH_PLATFORM)/src/qc_auth/fpc_tee_qc_auth.c
endif

ifeq ($(FPC_CONFIG_QSEE4),1)
LOCAL_CFLAGS += -DFPC_CONFIG_QSEE4
endif

ifeq ($(FPC_CONFIG_WAKE_LOCK),1)
LOCAL_CFLAGS += -DFPC_CONFIG_WAKE_LOCK
endif

ifeq ($(OPLUS_CONFIG_ANDROID_O_NEW_DEVICE),1)
LOCAL_CFLAGS += -DOPLUS_CONFIG_ANDROID_O_NEW_DEVICE
endif

LOCAL_SHARED_LIBRARIES += \
        libQSEEComAPI \
        libion

LOCAL_CFLAGS += -fstack-protector-all -fstack-protector-strong

LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM)/inc/qc_auth

