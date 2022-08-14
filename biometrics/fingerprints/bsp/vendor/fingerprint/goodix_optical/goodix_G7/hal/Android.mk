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

ifeq ($(TARGET_BUILD_VARIANT),user)
TARGET_MODE := release
else
TARGET_MODE := userdebug
endif

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_dump_G7
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_32 := libs_qcom/$(TARGET_MODE)/armeabi-v7a/libgf_hal_dump.a
else
LOCAL_SRC_FILES_32 := libs_mtk/$(TARGET_MODE)/armeabi-v7a/libgf_hal_dump.a
endif
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_64 := libs_qcom/$(TARGET_MODE)/arm64-v8a/libgf_hal_dump.a
else
LOCAL_SRC_FILES_64 := libs_mtk/$(TARGET_MODE)/arm64-v8a/libgf_hal_dump.a
endif

LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libgf_hal_goodix_sec_G7
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_32 := libs_qcom/$(TARGET_MODE)/armeabi-v7a/libgf_hal_goodix_sec.a
else
LOCAL_SRC_FILES_32 := libs_mtk/$(TARGET_MODE)/armeabi-v7a/libgf_hal_goodix_sec.a
endif
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_64 := libs_qcom/$(TARGET_MODE)/arm64-v8a/libgf_hal_goodix_sec.a
else
LOCAL_SRC_FILES_64 := libs_mtk/$(TARGET_MODE)/arm64-v8a/libgf_hal_goodix_sec.a
endif
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_ext_modules_G7
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_32 := libs_qcom/$(TARGET_MODE)/armeabi-v7a/libgf_hal_ext_modules.a
else
LOCAL_SRC_FILES_32 := libs_mtk/$(TARGET_MODE)/armeabi-v7a/libgf_hal_ext_modules.a
endif
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_64 := libs_qcom/$(TARGET_MODE)/arm64-v8a/libgf_hal_ext_modules.a
else
LOCAL_SRC_FILES_64 := libs_mtk/$(TARGET_MODE)/arm64-v8a/libgf_hal_ext_modules.a
endif
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_product_test_G7
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_32 := libs_qcom/$(TARGET_MODE)/armeabi-v7a/libgf_hal_product_test.a
else
LOCAL_SRC_FILES_32 := libs_mtk/$(TARGET_MODE)/armeabi-v7a/libgf_hal_product_test.a
endif
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_64 := libs_qcom/$(TARGET_MODE)/arm64-v8a/libgf_hal_product_test.a
else
LOCAL_SRC_FILES_64 := libs_mtk/$(TARGET_MODE)/arm64-v8a/libgf_hal_product_test.a
endif
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_VENDOR_MODULE := true

#include $(BUILD_PREBUILT)
##include $(CLEAR_VARS)
##LOCAL_MODULE_TAGS := optional


include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := libgf_hal_G7

CONFIG_INCLUDE_DIRS :=
CONFIG_INCLUDE_DIRS += \
    $(LOCAL_PATH)/../../inc \
    $(LOCAL_PATH)/../../../hwbinder \
    $(LOCAL_PATH)/../../../hwbinder/dcs/include \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/oplus \
    $(LOCAL_PATH)/kernel-headers \
    $(TOP)/system/libhidl/base/include \
    $(TOP)/vendor/mediatek/proprietary/geniezone/external/uree/include \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

CONFIG_INCLUDE_DIRS += \
    $(LOCAL_PATH)/../../../hwbinder/fingerprint_settings/include

ifeq ($(FP_MTK_GREATER_THAN_TEE500),y)
CONFIG_INCLUDE_DIRS += \
    $(TOP)/vendor/mediatek/proprietary/trustzone/trustonic/source/external/mobicore/common/ClientLib/include/GP
endif

ifeq ($(call is-vendor-board-platform,QCOM), true)
CONFIG_INCLUDE_DIRS += \
    $(TOPDIR)vendor/qcom/proprietary/securemsm/QSEEComAPI

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

CONFIG_LIBRARIES :=
CONFIG_MACROS :=
LOCAL_VENDOR_MODULE := true
ifeq ($(TARGET_MODE), release)
CONFIG_MACROS += \
    -DGF_LOG_LEVEL=2 \
    -DANDROID_VERSION_MODE=0
else
CONFIG_MACROS += \
    -DGF_LOG_LEVEL=4 \
    -DANDROID_VERSION_MODE=1 \
    -DSWITCH_RAWDATA_MODE
endif
CONFIG_MACROS += \
    -DPERSIST_DATA_BACKUP_DIR=\"/mnt/vendor/persist/fingerprint/\" \
    -DCUSTOMIZED_ION_QSECOM_CARVEOUT_HEAP_ID=15 \
    -DMAX_SENSOR_NUM=1 \
    -DGF_HAL_VERSION=\"391b8c8_391b8c8_391b8c8_2022.03.03_19:11:55\" \
    -DSUPPORT_MULTIPLE_FINGER_AUTH \
    -DSUPPORT_DUMP_HEARTBEAT_RATE \
    -DSUPPORT_OPEN_HB_PPG_DATA \
    -DCUSTOMIZE_DEV_NAME=\"/dev/goodix_fp\" \
    -DCUSTOMIZE_TEMPLATE_PATH=\"/data/vendor_de/0/fpdata\" \
    -DSUPPORT_HEARTBEAT_RATE

ifeq ($(OPPO_DEVICE_SUPPORT_DSP_G7),yes)
CONFIG_MACROS += \
    -DSUPPORT_DSP_HAL \
    -DSHIFT_DSP_ION_MEM_FLAG \
    -DENABLE_DSP_ENROLL=0x01 \
    -DTARGET_ION_ABI_VERSION=2 \
    -DCUSTOMIZED_ION_QSECOM_HEAP_ID=7 \

ifeq ($(call is-vendor-board-platform,QCOM), true)
CONFIG_SRC_C += \
    ca/HalDsp.cpp
endif
endif

ifeq ($(FP_HBM_BRIGHTNESS_DELAY),yes)
LOCAL_CFLAGS +=  -DFP_HBM_BRIGHTNESS_DELAY
endif

ifeq ($(BYPASS_BRIGHTNESSCHECK),yes)
LOCAL_CFLAGS +=  -DBYPASS_BRIGHTNESSCHECK
endif

ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
endif

ifeq ($(FP_TRACE_TAG_ENABLE),1)
LOCAL_CFLAGS += -DFP_TRACE_TAG_ENABLE
endif

ifeq ($(FP_CONFIG_BINDCORE_BYTID),yes)
LOCAL_CFLAGS += -DFP_BINDCORE_BYTID
endif

ifeq ($(FP_CONFIG_MULTI_TA),yes)
LOCAL_CFLAGS += -DFP_MULTI_TA
endif

CONFIG_SRC_C :=
CONFIG_SRC_C += \
    main/DelmarSensor.cpp \
    main/DelmarAlgo.cpp \
    main/DelmarFingerprintCore.cpp \
    main/FingerprintCore.cpp \
    main/EventCenter.cpp \
    main/Thread.cpp \
    main/MsgBus.cpp \
    main/CustomizedHalUtils.cpp \
    main/HalBase.cpp \
    main/CustomizedSensor.cpp \
    main/SensorDetector.cpp \
    main/HalUtils.cpp \
    main/HalContext.cpp \
    main/CustomizedParams.cpp \
    main/FingerListSync.cpp \
    main/HalContextExt.cpp \
    main/CustomizedHalConfig.cpp \
    main/Sensor.cpp \
    main/CustomizedDevice.cpp \
    main/CustomizedDelmarProductTest.cpp \
    main/CustomizedDelmarAlgo.cpp \
    main/gf_error.c \
    main/CustomizedSensorConfigProvider.cpp \
    main/CustomizedFingerprintCore.cpp \
    main/CoreCreator.cpp \
    main/AsyncQueue.cpp \
    main/Timer.cpp \
    main/Algo.cpp \
    main/Device.cpp \
    main/DelmarHeartbeat.cpp \
    main/DataSynchronizer.cpp \
    main/FpMonitorTool.cpp \
    oplus/DcsInfo.cpp \
    oplus/to_string.cpp \
    oplus/record.cpp

ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES += \
    ca/CaEntry.cpp \
    ca/HalDsp.cpp \
    ca/IonMemory.cpp
else
LOCAL_SRC_FILES += \
    ca/CaEntry_mtk.cpp \
    ca/IonMemory_mtk.cpp
#ifeq ($(OPPO_DEVICE_SUPPORT_DSP),yes)
#LOCAL_SRC_FILES += \
#    ca/HalDsp_mtk.cpp \
#    ca/uree.c \
#    ca/uree_mem.c
#endif
endif


CONFIG_SHARED_LIBRARIES :=
#CONFIG_SHARED_LIBRARIES += \
#    libvendor.goodix.hardware.biometrics.fingerprint@2.1

CONFIG_STATIC_LIBRARIES :=
CONFIG_STATIC_LIBRARIES += \
    libgf_hal_dump_G7 \
    libgf_hal_goodix_sec_G7 \
    libgf_hal_ext_modules_G7 \
    libgf_hal_product_test_G7


LOCAL_SHARED_LIBRARIES :=
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libc \
    libutils \
    libdl \
    libcutils \
    liblog \
    libhardware \
    libion

ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SHARED_LIBRARIES += \
    libQSEEComAPI
else
LOCAL_SHARED_LIBRARIES += \
    libMcClient

LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -D__TEE_TRUSTONIC
endif

LOCAL_STATIC_LIBRARIES :=
LOCAL_LDLIBS :=

IS_GRF_PLATFORM := false
#GRF(system = S, vendor = R)  qcom:  BOARD_SHIPPING_API_LEVEL := 30  mtk: PRODUCT_SHIPPING_API_LEVEL := 30
ifeq ($(call is-vendor-board-platform,QCOM)_$(BOARD_SHIPPING_API_LEVEL),true_30)
IS_GRF_PLATFORM=true
endif
ifeq ($(TRUSTONIC_TEE_SUPPORT)_$(PRODUCT_SHIPPING_API_LEVEL),yes_30)
IS_GRF_PLATFORM=true
endif

ifeq ($(IS_GRF_PLATFORM), false)
LOCAL_CFLAGS += -DUSE_DMA_MEMORY
LOCAL_SHARED_LIBRARIES += \
    libdmabufheap
endif

LOCAL_C_INCLUDES += $(CONFIG_INCLUDE_DIRS)
LOCAL_CFLAGS += $(CONFIG_MACROS) -Wno-error -Wno-date-time
LOCAL_LDFLAGS += -nodefaultlibs -lc -lm -ldl
LOCAL_LDFLAGS += $(CONFIG_LIBRARIES)
# for android10+, to solve runtime SEGV_ACCERR error
LOCAL_XOM := false

LOCAL_SRC_FILES += $(CONFIG_SRC_C)
LOCAL_SHARED_LIBRARIES += $(CONFIG_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES += $(CONFIG_STATIC_LIBRARIES)

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
include $(call all-makefiles-under, $(LOCAL_PATH)/hwbinder)
