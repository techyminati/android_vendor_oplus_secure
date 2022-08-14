
#
# Copyright (c) 2014 TRUSTONIC LIMITED
# All rights reserved
#
# The present software is the confidential and proprietary information of
# TRUSTONIC LIMITED. You shall not disclose the present software and shall
# use it only in accordance with the terms of the license agreement you
# entered into with TRUSTONIC LIMITED. This software may be subject to
# export or import laws in certain countries.
#
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Long.Liu      2018/12/12        add libTBaseProvisioningAT for VTS test fail
#
# =============================================================================

# Do not remove this - Android build needs the definition
ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)
$(warning trustonic env)
LOCAL_PATH := $(call my-dir)

MOBICORE_LIB_PATH +=\
    $(LOCAL_PATH)/../../../trustzone/t-sdk/TlcSdk/Out/Public \
    $(LOCAL_PATH)/../../../trustzone/t-sdk/TlcSdk/Out/Public/GP \
    $(LOCAL_PATH)/../../../trustzone/t-sdk/TlSdk/Out/Public/MobiCore/inc \
    $(LOCAL_PATH)/../../../trustzone/t-sdk/TlSdk/Out/Public/MobiCore/inc/TlApi \
    $(LOCAL_PATH)/../../../external/mobicore/common/LogWrapper

# =============================================================================
include $(CLEAR_VARS)

LOCAL_32_BIT_ONLY := true

LOCAL_MODULE := libTBaseProvisioning
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := libs/armeabi/libTBaseProvisioning.so
LOCAL_SRC_FILES_arm64 := libs/armeabi/libTBaseProvisioning.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_32_BIT_ONLY := true

LOCAL_MODULE := libTBaseProvisioningAT
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := libs/armeabi/libTBaseProvisioningAT.so
LOCAL_SRC_FILES_arm64 := libs/armeabi/libTBaseProvisioningAT.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)

#LOCAL_PREBUILT_LIBS := libTBaseProvisioning:libs/armeabi/libTBaseProvisioning.so
#include $(BUILD_MULTI_PREBUILT)


# =============================================================================
# Executable TBaseDeviceBinding
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE := TBaseDeviceBinding
LOCAL_VENDOR_MODULE := true

# Add your folders with header files here
LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/include \
        $(MOBICORE_LIB_PATH)
#       $(COMP_PROV_LIB_ROOT)/include \
#       $(CMTL_PATH)/Public/TlCm/3.0 \inc \
#       $(COMP_PATH_TlSdk)/Public/MobiCore/inc \
#       $(COMP_PATH_MobiCoreDriverLib)/Public

# Add your source files here (relative paths)
LOCAL_SRC_FILES += main.cpp
LOCAL_SRC_FILES += utils.cpp

LOCAL_32_BIT_ONLY := true

LOCAL_SHARED_LIBRARIES := libTBaseProvisioning
LOCAL_SHARED_LIBRARIES := libTBaseProvisioningAT
LOCAL_SHARED_LIBRARIES += libMcClient
LOCAL_SHARED_LIBRARIES += liblog

include $(BUILD_EXECUTABLE)
endif
# =============================================================================
