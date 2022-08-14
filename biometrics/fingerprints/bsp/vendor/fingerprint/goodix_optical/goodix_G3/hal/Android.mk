
LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BUILD_VARIANT),eng)
MODE := debug
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
MODE := userdebug
else
MODE := release
endif

ifeq ($(TARGET_BUILD_VARIANT),user)
MODE=release
else
MODE=debug
endif

ifeq ($(OPLUS_CUSTOM_ENCRYPT_ENABLE), y)
ifeq ($(PRE_EUCLID_VERSION),1)
MODE=debug
endif
endif

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_sz_dump_G3
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_32 := libs_qcom/$(MODE)/armeabi-v7a/libgf_hal_sz_dump.a
else
LOCAL_SRC_FILES_32 := libs_mtk/$(MODE)/armeabi-v7a/libgf_hal_sz_dump.a
endif
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_64 := libs_qcom/$(MODE)/arm64-v8a/libgf_hal_sz_dump.a
else
LOCAL_SRC_FILES_64 := libs_mtk/$(MODE)/arm64-v8a/libgf_hal_sz_dump.a
endif
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libgf_hal_goodix_sec_G3
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_32 := libs_qcom/$(MODE)/armeabi-v7a/libgf_hal_goodix_sec.a
else
LOCAL_SRC_FILES_32 := libs_mtk/$(MODE)/armeabi-v7a/libgf_hal_goodix_sec.a
endif
ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES_64 := libs_qcom/$(MODE)/arm64-v8a/libgf_hal_goodix_sec.a
else
LOCAL_SRC_FILES_64 := libs_mtk/$(MODE)/arm64-v8a/libgf_hal_goodix_sec.a
endif
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := libgf_hal_G3

CONFIG_INCLUDE_DIRS :=
CONFIG_INCLUDE_DIRS += \
    $(LOCAL_PATH)/../../inc \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/kernel-headers \
    $(LOCAL_PATH)/../../../hwbinder \
    $(LOCAL_PATH)/../../../hwbinder/dcs/include \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
    $(TOP)/system/libhidl/base/include \
    $(TOP)/vendor/mediatek/proprietary/geniezone/external/uree/include

ifeq ($(FP_MTK_GREATER_THAN_TEE500),y)
CONFIG_INCLUDE_DIRS += \
    $(TOP)/vendor/mediatek/proprietary/trustzone/trustonic/source/external/mobicore/common/ClientLib/include/GP
endif

ifeq ($(FP_SETTINGS_ENABLE), y)
LOCAL_CFLAGS += -DFP_CONFIG_SETTINGS_ENABLE
CONFIG_INCLUDE_DIRS += \
    $(LOCAL_PATH)/../../../hwbinder/fingerprint_settings/include
endif

ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_CFLAGS +=  -DFP_ENABLE_UNLOCK_CALBRATION

CONFIG_INCLUDE_DIRS += \
    $(TOPDIR)vendor/qcom/proprietary/securemsm/QSEEComAPI

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

CONFIG_LIBRARIES :=
CONFIG_MACROS :=
LOCAL_VENDOR_MODULE := true

ifeq ($(TARGET_BUILD_VARIANT),user)
CONFIG_MACROS += \
    -DSUPPORT_DUMP \
    -DGF_LOG_LEVEL=1
else
CONFIG_MACROS += \
    -DSUPPORT_DUMP \
    -DGF_LOG_LEVEL=3 \
    -DSWITCH_RAWDATA_MODE
endif

ifeq ($(OPLUS_CUSTOM_ENCRYPT_ENABLE), y)
ifeq ($(PRE_EUCLID_VERSION),1)
CONFIG_MACROS += \
    -DSUPPORT_DUMP \
    -DGF_ENC_DUMP_DATA=1
endif
endif

CONFIG_MACROS += \
    -D__ANDROID_P \
    -D__QSEE

ifeq ($(OPLUS_DEVICE_SUPPORT_DSP),yes)
CONFIG_MACROS += \
    -DSUPPORT_DSP
endif

ifeq ($(OPLUS_SUPPORT_STABILITYTEST),yes)
CONFIG_MACROS += \
    -DOPLUS_SUPPORT_STABILITYTEST
endif


ifeq ($(OPLUS_CONFIG_DYNAMIC_COMPATIBLE_DSP),yes)
CONFIG_MACROS += \
    -DDYNAMIC_SUPPORT_DSP
endif

ifeq ($(FP_HBM_BRIGHTNESS_DELAY),yes)
LOCAL_CFLAGS +=  -DFP_HBM_BRIGHTNESS_DELAY
endif

ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
endif

ifeq ($(FP_CONFIG_BINDCORE_BYTID),yes)
LOCAL_CFLAGS += -DFP_BINDCORE_BYTID
endif

ifeq ($(FP_CONFIG_SET_PRIORITY),yes)
LOCAL_CFLAGS += -DFP_SET_PRIORITY
endif

ifeq ($(FP_CONFIG_SET_UXTHREAD),yes)
LOCAL_CFLAGS += -DFP_SET_UXTREAD
endif

ifeq ($(OPLUS_CONFIG_GOODIX_VERSION), V03_02_02_162)
LOCAL_CFLAGS += \
    -DALGO_VERSION_V03_02_02_162
endif

ifeq ($(OPLUS_CONFIG_GOODIX_VERSION), V03_02_02_164)
LOCAL_CFLAGS += \
    -DALGO_VERSION_V03_02_02_164
endif

ifeq ($(OPLUS_CONFIG_GOODIX_VERSION), V03_02_02_164_01)
LOCAL_CFLAGS += \
    -DALGO_VERSION_V03_02_02_164_01
endif

OPLUS_FINGERPRINT_G3_HAL_VERSION := v20_09_07_01
LOCAL_CFLAGS += -DOPLUS_FINGERPRINT_G3_HAL_VERSION='"$(OPLUS_FINGERPRINT_G3_HAL_VERSION)"'

LOCAL_C_INCLUDES += $(CONFIG_INCLUDE_DIRS)
LOCAL_CFLAGS += $(CONFIG_MACROS)
LOCAL_LDFLAGS += -nodefaultlibs -lc -lm -ldl
LOCAL_LDFLAGS += $(CONFIG_LIBRARIES)

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += \
    main/Thread.cpp \
    main/CustomizedParams.cpp \
    main/Tracer.cpp \
    main/RegisterService.cpp \
    main/SZAlgo.cpp \
    main/ExtModuleCreator.cpp \
    main/Device.cpp \
    main/Algo.cpp \
    main/HalContext.cpp \
    main/SensorDetector.cpp \
    main/MsgBus.cpp \
    main/EventCenter.cpp \
    main/Timer.cpp \
    main/ExtCutomized.cpp \
    main/SZCustomizedProductTest.cpp \
    main/SZProductTest.cpp \
    main/SZFingerprintCore.cpp \
    main/SZSensor.cpp \
    main/DumpCreator.cpp \
    main/RegisterHwbinderService.cpp \
    main/CoreCreator.cpp \
    main/HalBase.cpp \
    main/SZExtCustomized.cpp \
    main/FingerprintCore.cpp \
    main/HalUtils.cpp \
    main/ExtFido.cpp \
    main/ProductTest.cpp \
    main/Sensor.cpp \
    main/TaLogDump.cpp \
    main/gf_error.c \
    main/HalContextExt.cpp \
    main/FingerListSync.cpp \
    main/AsyncQueue.cpp \
    main/DcsInfo.cpp \
    main/FpMonitorTool.cpp \
    main/Fingerprint_cali_tool.cpp

ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_SRC_FILES += \
    ca/CaEntry.cpp \
    ca/IonMemory.cpp

ifeq ($(OPLUS_SUPPORT_STABILITYTEST),yes)
LOCAL_SRC_FILES += \
    main/StabilityTest.cpp
endif

ifeq ($(OPLUS_DEVICE_SUPPORT_DSP_COMPATIBLE_VERSION_G3),yes)
LOCAL_SRC_FILES += \
    ca/cdsp_compatible_verison/HalDsp.cpp
LOCAL_CFLAGS += -DSUPPORT_DSP_COMPATIBLE_VERSION_G3
else
LOCAL_SRC_FILES += \
    ca/HalDsp.cpp
endif

else
LOCAL_SRC_FILES += \
    ca/CaEntry_mtk.cpp \
    ca/PersistData.cpp
ifeq ($(OPLUS_DEVICE_SUPPORT_DSP),yes)
LOCAL_SRC_FILES += \
    ca/HalDsp_mtk.cpp \
    ca/IonMemory_mtk.cpp \
    ca/uree.c \
    ca/uree_mem.c
endif
endif

LOCAL_SHARED_LIBRARIES :=
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libc \
    libhwbinder \
    libhidltransport \
    libbinder \
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
LOCAL_STATIC_LIBRARIES += \
    libgf_hal_sz_dump_G3 \
    libgf_hal_goodix_sec_G3

# for android10+, to solve runtime SEGV_ACCERR error
LOCAL_XOM := false
LOCAL_LDLIBS :=
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
