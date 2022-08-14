#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
# Tbase specific includes for the TAC
# Note that LOCAL_PATH is set to the location of the normal/Android.mk that
# includes this file
# =============================================================================
LOCAL_PATH_PLATFORM    := ../normal/platform/tbase
LOCAL_PATH_PLATFORM_IF := ../interface/platform/tbase

# =============================================================================
LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM)/inc \
		$(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM_IF) \
		$(COMP_PATH_MobiCoreDriverLib)

LOCAL_SRC_FILES += \
		$(LOCAL_PATH_PLATFORM)/src/fpc_tbase_tac.c

ifdef FPC_CONFIG_HW_AUTH
LOCAL_SRC_FILES += $(LOCAL_PATH_PLATFORM)/src/hw_auth/fpc_tee_hw_auth_tbase.c
endif

# Path to the Kernel REE device driver sysfs interface
LOCAL_CFLAGS += -DFPC_REE_DEVICE_ALIAS_FILE='"modalias"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_NAME='"fpc_interrupt"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_PATH='"/sys/bus/platform/devices"'

LOCAL_SHARED_LIBRARIES += \
		libMcClient

# LOCAL_EXPORT_C_INCLUDE_DIRS += \

# setup external deps
#ifndef ANDROID_SYSTEM_ROOT
#If not defined use default folder used in main build git
#ANDROID_SYSTEM_ROOT := ~/devtools/platforms/mt6795
#endif
#t_base_dev_kit=$(ANDROID_SYSTEM_ROOT)/vendor/mediatek/proprietary/trustzone/trustonic/source/bsp/platform/mt6795
#COMP_PATH_Logwrapper=$(t_base_dev_kit)/t-sdk/TlcSdk/Out/Logwrapper
#COMP_PATH_OTA=$(t_base_dev_kit)/t-sdk/OTA
#COMP_PATH_Tools=$(t_base_dev_kit)/tools
#relative path needed for COMP_PATH_MobiCoreDriverLib (multi-OS compatibility for including library)
#COMP_PATH_MobiCoreDriverLib=$(t_base_dev_kit)/t-sdk/TlcSdk
#COMP_PATH_TlSdk=$(t_base_dev_kit)/t-sdk/TlSdk
#COMP_PATH_DrSdk=$(t_base_dev_kit)/t-sdk/DrSdk/Out
#COMP_PATH_MobiCore=$(t_base_dev_kit)/t-sdk/TlcSdk/Public/

TBASE_API_LEVEL := 5

#include $(COMP_PATH_Logwrapper)/Android.mk
