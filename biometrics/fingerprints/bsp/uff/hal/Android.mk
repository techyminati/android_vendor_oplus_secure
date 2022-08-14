####################################################################################
## Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
##
## File: - vendor/fingerprint/Android.mk
##
## Description:
##      Android.mk for fingerprint
##
## Version: 1.0
## Date created: 18:03:11,26/03/2021
## Author: Ran.Chen@BSP.Fingerprint.Basic
## TAG: BSP.Fingerprint.Basic
## ---------------------------Revision History--------------------------------
##  <author>      <data>            <desc>
##  Ran.Chen   2021/03/26        create the file
####################################################################################
LOCAL_PATH := $(call my-dir)

# ENABLE REE FP, CONFIG SET: CONFIG_FINGERPRINT_PLATFORM_REE=yes
#CONFIG_FINGERPRINT_PLATFORM_REE=yes
CONFIG_FINGERPRINT_CHIPID_ENABLE=yes
#CONFIG_FINGERPRINT_DEBUG_SOCKET=yes
ifeq ($(CONFIG_FINGERPRINT_PLATFORM_REE), yes)
include $(CLEAR_VARS)
LOCAL_MODULE := libfpta
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
LOCAL_SRC_FILES := Ca/ree/lib/ReeTa.c
include $(BUILD_SHARED_LIBRARY)
endif
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.oplus.hardware.biometrics.fingerprint@2.1-service
LOCAL_INIT_RC := vendor.oplus.hardware.biometrics.fingerprint@2.1-service.rc
#system/vendor/lib or system/lib
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
#user/eng/tests/optional(all)
LOCAL_MODULE_TAGS := optional
LOCAL_VINTF_FRAGMENTS := ../config/xml/manifest_oplus_fingerprint.xml
ifeq ($(CONFIG_FINGERPRINT_PLATFORM_REE), yes)
LOCAL_SHARED_LIBRARIES += libfpta
endif

LOCAL_STATIC_LIBRARIES := libcutils
LOCAL_LDLIBS += -llog -lhardware -lcutils

LOCAL_C_INCLUDES += $(LOCAL_PATH)/fingerprint_type/include
LOCAL_C_INCLUDES += hardware/libhardware/include

LOCAL_SRC_FILES := \
    Hidl/Service.cpp \
    Manager/FingerprintManager.cpp \
    Manager/FingerprintFunction.cpp \
    Manager/FingerprintCommon.cpp \
    Notify/FingerprintNotify.cpp \
    Message/FingerprintMessage.cpp \
    Context/HalContext.cpp \
    Config/FingerprintConfig.cpp \
    Dcs/DcsInfo.cpp \
    Device/Device.cpp \
    Dump/Dump.cpp \
    Dump/CreateData.cpp \
    Dump/DataInfo.cpp \
    Dump/UserData/UserData.cpp \
    Dump/FactoryTestData/FactoryTestData.cpp \
    Dump/CaliData/CaliData.cpp \
    Dump/AutoTestData/AutoTestData.cpp \
    Dump/AppData/AppData.cpp \
    Utils/Utils.cpp \
    FactoryTest/FactoryTest.cpp \
    Handler/Handler.cpp \
    Handler/Message.cpp \
    Health/HealthMonitor.cpp \
    Perf/Perf.cpp \
    VndCode/VndCode.cpp \
    AutoSmoking/AutoSmoking.cpp

# add *.cpp dir for srcs
SRC_DIR := Ca

LOCAL_SRC_FILES += $(foreach d, $(SRC_DIR), $(shell cd $(LOCAL_PATH); find $(d) -maxdepth 5 -name "*.cpp"))
$(info ===LOCAL_SRC_FILES:$(LOCAL_SRC_FILES))
$(info ===TARGET_BOARD_PLATFORM:$(TARGET_BOARD_PLATFORM))

ifeq ($(call is-vendor-board-platform,QCOM), true)
TEE_PLATFORM := QSEE
LOCAL_SHARED_LIBRARIES += libQSEEComAPI libion
LOCAL_C_INCLUDES += \
    vendor/qcom/proprietary/securemsm/QSEEComAPI \
    $(LOCAL_PATH_QSEECOMAPI) \
    system/memory/libion/kernel-headers \
    system/memory/libion/include

else
TEE_PLATFORM := TBASE
LOCAL_SHARED_LIBRARIES += libMcClient
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_C_INCLUDES += \
    $(TOP)/vendor/mediatek/proprietary/trustzone/trustonic/source/external/mobicore/common/ClientLib/include/GP
endif

ifeq ($(CONFIG_FINGERPRINT_PLATFORM_REE), yes)
TEE_PLATFORM = REE
endif

$(info ===LOCAL_SHARED_LIBRARIES:$(LOCAL_SHARED_LIBRARIES))

# for TEE_PLATFORM := REE
ifeq ($(TEE_PLATFORM), REE)
$(info ===Select the REE TEE_PLATFORM:$(TEE_PLATFORM))
endif

# after select the tee platform
LOCAL_CFLAGS += -DTEE_$(TEE_PLATFORM)
LOCAL_CFLAGS += -DTEE_PLATFORM=\"$(TEE_PLATFORM)\"
$(info ===current TEE_PLATFORM:$(TEE_PLATFORM))

ifeq ($(CONFIG_FINGERPRINT_CHIPID_ENABLE), yes)
LOCAL_CFLAGS += -DFINGERPRINT_CHIPID_ENABLE
endif

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/Hidl/include \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/Device/include \
    $(LOCAL_PATH)/Ca/include \
    $(LOCAL_PATH)/Config/include \
    $(LOCAL_PATH)/Dcs/include \
    $(LOCAL_PATH)/Device/include \
    $(LOCAL_PATH)/Dump/include \
    $(LOCAL_PATH)/Utils/include \
    $(LOCAL_PATH)/FactoryTest/include \
    $(LOCAL_PATH)/Handler/include \
    $(LOCAL_PATH)/Health/include \
    $(LOCAL_PATH)/Perf/include \
    $(LOCAL_PATH)/hardware/libhardware/include \
    $(LOCAL_PATH)/VndCode/include \
    $(LOCAL_PATH)/Context/include \
    $(LOCAL_PATH)/Manager/include \
    $(LOCAL_PATH)/Message/include \
    $(LOCAL_PATH)/Notify/include \
    $(LOCAL_PATH)/AutoSmoking/include


ifeq ($(CONFIG_FINGERPRINT_DEBUG_SOCKET), yes)
LOCAL_CFLAGS += -DFINGERPRINT_DEBUG_SOCKET
LOCAL_SRC_FILES += Socket/FingerprintSocket.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Socket
endif

$(warning "uff_fingerprint PLATFORM_VERSION $(PLATFORM_VERSION)")
ifeq ($(PLATFORM_VERSION), 11)
LOCAL_SRC_FILES += Hidl/AndroidR/BiometricsFingerprint.cpp
LOCAL_C_INCLUDES +=$(LOCAL_PATH)/Hidl/AndroidR/include
LOCAL_CFLAGS += -DVERSION_ANDROID_R
LOCAL_SHARED_LIBRARIES += \
    libbinder \
    liblog \
    libhidlbase \
    libhidltransport \
    libcutils \
    libion\
    libutils \
    vendor.oplus.hardware.biometrics.fingerprint@2.1

else
LOCAL_VINTF_FRAGMENTS := VintXml/manifest_oplus_fingerprint.xml
LOCAL_SRC_FILES += Hidl/AndroidS/BiometricsFingerprintAdaptS.cpp
LOCAL_C_INCLUDES +=$(LOCAL_PATH)/Hidl/AndroidS/include
LOCAL_CFLAGS += -DVERSION_ANDROID_S
LOCAL_SHARED_LIBRARIES += libbinder liblog libhidlbase libhidltransport \
                          libutils vendor.oplus.hardware.biometrics.fingerprint@2.1 \
                          android.hardware.biometrics.fingerprint@2.3 \
                          android.hardware.biometrics.fingerprint@2.2 \
                          android.hardware.biometrics.fingerprint@2.1

endif

# ============for OPLUS feature start============
#add for calling hypunsd sevice
# ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
# LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
# LOCAL_C_INCLUDES += vendor/oplus/hardware/orms/1.0
# LOCAL_SHARED_LIBRARIES += vendor.oplus.hardware.orms@1.0  \
#                           libormshalclient
# endif

# add for Dcs
#ifeq ($(FP_DCS_ENABLE), y)
LOCAL_SRC_FILES += Dcs/DcsReport.cpp
LOCAL_CFLAGS += -DFP_DCS_ENABLE
LOCAL_SHARED_LIBRARIES += vendor.oplus.hardware.commondcs@1.0
#endif

# ============for OPLUS feature end============

include $(BUILD_EXECUTABLE)
