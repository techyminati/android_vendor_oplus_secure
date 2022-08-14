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
TARGET_MODE=debug
LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BUILD_VARIANT),eng)
TARGET_MODE := debug
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
TARGET_MODE := debug
else
TARGET_MODE := release
endif

ifeq ($(OBSOLETE_KEEP_ADB_SECURE),1)
TARGET_MODE=release
else
TARGET_MODE=debug
endif

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_dump_19805_G6_7_oplus
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES_32 := \
    libs/$(TARGET_MODE)/armeabi-v7a/libgf_hal_dump.a
LOCAL_SRC_FILES_64 := \
    libs/$(TARGET_MODE)/arm64-v8a/libgf_hal_dump.a
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_goodix_sec_19805_G6_7_oplus
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
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libgf_hal_ext_modules_19805_G6_7_oplus
LOCAL_SRC_FILES_32 := \
    libs/$(TARGET_MODE)/armeabi-v7a/libgf_hal_ext_modules.a
LOCAL_SRC_FILES_64 := \
    libs/$(TARGET_MODE)/arm64-v8a/libgf_hal_ext_modules.a
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libgf_hal_product_test_19805_G6_7_oplus
LOCAL_SRC_FILES_32 := \
    libs/$(TARGET_MODE)/armeabi-v7a/libgf_hal_product_test.a
LOCAL_SRC_FILES_64 := \
    libs/$(TARGET_MODE)/arm64-v8a/libgf_hal_product_test.a
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a

include $(BUILD_PREBUILT)
include $(CLEAR_VARS)


LOCAL_MODULE := libgf_hal_19805_G6_7_oplus
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := both

LOCAL_CFLAGS += -DFP_TRACE_DEBUG
CONFIG_INCLUDE_DIRS :=
CONFIG_INCLUDE_DIRS += \
    $(LOCAL_PATH)/oneplus \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../inc \
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
    -DSUPPORT_DUMP \
    -DSUPPORT_DUMP_ORIGIN_DATA
endif

ifeq ($(TARGET_MODE), release)
CONFIG_MACROS += \
    -DGF_LOG_LEVEL=3
endif


CONFIG_MACROS += \
    -D__ANDROID_Q \
    -D__QSEE \
    -DTARGET_ION_ABI_VERSION=2 \
    -DMAX_SENSOR_NUM=1 \
    -DENCRYPT_VERSION=0x000103EB \
    -DSUPPORT_DUMP_ORIGIN_DATA \
    -DCUSTOMIZE_DEV_NAME=\"/dev/goodix_fp\" \
    -DGF_HAL_VERSION=\"8589435_e01f4cf_550d6e3_2020.05.02_19:39:56\" \
    -DCUSTOMIZE_TEMPLATE_PATH=\"/mnt/vendor/persist/data/\"


ifeq ($(OPLUS_DEVICE_SUPPORT_DSP_G6),ops)
CONFIG_MACROS += \
    -DSUPPORT_DSP_HAL \
    -DSHIFT_DSP_ION_MEM_FLAG
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
    ca/CaEntry.cpp \
    ca/HalDsp.cpp \
    delmar/DelmarFingerprintCore.cpp \
    delmar/DelmarAlgo.cpp \
    delmar/DelmarSensor.cpp \
    oneplus/CustomizedDelmarAlgo.cpp \
    oneplus/CustomizedFingerprintCore.cpp \
    oneplus/CustomizedDelmarProductTest.cpp \
    oneplus/CustomizedDelmarSensor.cpp \
    oneplus/CustomizedParams.cpp \
    oneplus/CustomizedDevice.cpp \
    oneplus/Fpsys.c \
    oneplus/CustomizedHalConfig.cpp \
    main/MsgBus.cpp \
    main/gf_error.c \
    main/FingerprintCore.cpp \
    main/EventCenter.cpp \
    main/HalBase.cpp \
    main/HalContextExt.cpp \
    main/SensorDetector.cpp \
    main/Algo.cpp \
    main/HalContext.cpp \
    main/Device.cpp \
    main/CoreCreator.cpp \
    main/Thread.cpp \
    main/AsyncQueue.cpp \
    main/HalUtils.cpp \
    main/FingerListSync.cpp \
    main/Timer.cpp \
    main/Sensor.cpp 


LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += \
    libgf_hal_dump_19805_G6_7_oplus \
    libgf_hal_goodix_sec_19805_G6_7_oplus \
    libgf_hal_ext_modules_19805_G6_7_oplus \
    libgf_hal_product_test_19805_G6_7_oplus


LOCAL_SHARED_LIBRARIES :=
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libc \
    libhwbinder \
    libhidltransport \
    libutils \
    libdl \
    libcutils \
    liblog \
    libhardware \
    libion \
    libQSEEComAPI


LOCAL_LDLIBS :=



LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
