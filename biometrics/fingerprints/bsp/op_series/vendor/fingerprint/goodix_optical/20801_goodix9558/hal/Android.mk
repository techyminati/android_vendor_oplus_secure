#
# Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#
# Copyright (C) 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

TARGET_MODE=debug
LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BUILD_VARIANT),eng)
TARGET_MODE := debug
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
TARGET_MODE := debug
else
TARGET_MODE := release
endif


include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_sz_dump_20801_G3_oplus
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES_32 := \
    libs/$(TARGET_MODE)/armeabi-v7a/libgf_hal_sz_dump.a
LOCAL_SRC_FILES_64 := \
    libs/$(TARGET_MODE)/arm64-v8a/libgf_hal_sz_dump.a
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_goodix_sec_20801_G3_oplus
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES_32 := \
    libs/$(TARGET_MODE)/armeabi-v7a/libgf_hal_goodix_sec.a
LOCAL_SRC_FILES_64 := \
    libs/$(TARGET_MODE)/arm64-v8a/libgf_hal_goodix_sec.a
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a

include $(BUILD_PREBUILT)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_ext_modules_20801_G3_oplus
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES_32 := \
    libs/$(TARGET_MODE)/armeabi-v7a/libgf_hal_ext_modules.a
LOCAL_SRC_FILES_64 := \
    libs/$(TARGET_MODE)/arm64-v8a/libgf_hal_ext_modules.a
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a

include $(BUILD_PREBUILT)
include $(CLEAR_VARS)

LOCAL_MODULE := libgf_ud_hal_20801_G3_oplus
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := both
LOCAL_CFLAGS += -DFP_TRACE_DEBUG

CONFIG_INCLUDE_DIRS :=
CONFIG_INCLUDE_DIRS += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../inc \
    $(LOCAL_PATH)/../../../hwbinder/dcs/include \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
    $(TOPDIR)system/core/libion/include \
    $(TOPDIR)system/core/libion/kernel-headers \
    $(TOPDIR)vendor/qcom/proprietary/securemsm/QSEEComAPI
    
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

CONFIG_LIBRARIES :=
CONFIG_MACROS :=
ifeq ($(TARGET_MODE), debug)
CONFIG_MACROS += \
    -DGF_LOG_LEVEL=4 \
    -DSUPPORT_DUMP
endif
ifeq ($(TARGET_MODE), release)
CONFIG_MACROS += \
    -DGF_LOG_LEVEL=3
endif
CONFIG_MACROS += \
    -D__ANDROID_P \
    -D__QSEE \
    -DTARGET_ION_ABI_VERSION=2
#    -DSUPPORT_DSP_HAL \
#    -DSHIFT_DSP_ION_MEM_FLAG
ifeq ($(TARGET_MODE), userdebug)
CONFIG_MACROS += \
    -DGF_LOG_LEVEL=3 \
    -DSUPPORT_DUMP
endif
ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
endif

ifeq ($(FP_CONFIG_BINDCORE_BYTID),yes)
LOCAL_CFLAGS += -DFP_BINDCORE_BYTID
endif

LOCAL_C_INCLUDES += $(CONFIG_INCLUDE_DIRS)
LOCAL_CFLAGS += $(CONFIG_MACROS)
LOCAL_LDFLAGS += -nodefaultlibs -lc -lm -ldl
LOCAL_LDFLAGS += $(CONFIG_LIBRARIES)

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += \
    ca/IonMemory.cpp \
    ca/HalDsp.cpp \
    ca/CaEntry.cpp \
    product_test/ProductTest.cpp \
    main/Fpsys.c \
    main/SZFingerprintCore.cpp \
    main/HalBase.cpp \
    main/EventCenter.cpp \
    main/gf_error.c \
    main/CustomizedDevice.cpp \
    main/FingerprintCore.cpp \
    main/HalUtils.cpp \
    main/FingerListSync.cpp \
    main/CustomizedParams.cpp \
    main/CoreCreator.cpp \
    main/AsyncQueue.cpp \
    main/CustomizedFingerprintCore.cpp \
    main/HalContextExt.cpp \
    main/SensorDetector.cpp \
    main/SZCustomizedProductTest.cpp \
    main/SZAlgo.cpp \
    main/MsgBus.cpp \
    main/SZSensor.cpp \
    main/SZExtCustomized.cpp \
    main/ExtModuleCreator.cpp \
    main/DumpCreator.cpp \
    main/Device.cpp \
    main/Thread.cpp \
    main/HalContext.cpp \
    main/SZProductTest.cpp \
    main/Timer.cpp \
    main/Algo.cpp \
    main/Sensor.cpp 


LOCAL_SHARED_LIBRARIES :=
LOCAL_SHARED_LIBRARIES += \
    libc++ \
    libbase \
    libc \
    libbinder \
    libutils \
    libdl \
    libcutils \
    liblog \
    libhwbinder \
    libhardware \
    libQSEEComAPI \
    libhidlbase \
    libhidltransport \
    libion


LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += \
    libgf_hal_sz_dump_20801_G3_oplus \
    libgf_hal_goodix_sec_20801_G3_oplus \
    libgf_hal_ext_modules_20801_G3_oplus

LOCAL_LDLIBS :=

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
