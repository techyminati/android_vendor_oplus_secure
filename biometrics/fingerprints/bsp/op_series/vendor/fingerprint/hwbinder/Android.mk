####################################################################################
## File: - vendor.
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint hwbinder Config for FPC Android O SW23.2.2 && GOODIX
##
## Version: 1.0
## Date created: 18:03:11,03/12/2017
## Author: Ziqing.guo@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Ziqing.guo   2017/12/03        create the file
##  Ran.Chen     2018/11/26        remove fpc for SDM855
##  Yang.Tan     2018/11/26        add fpc sw23 and sw28 compatible
##  Ran.Chen     2018/12/11        add for template status count
##  Long.Liu     2018/12/19        add fpc1511 sw28 version for 18151
##  Long.Liu     2019/01/03        add for 18161 fpc1511 sw28 version
##  Long.Liu     2019/02/22        modify for 18161 support GOODIX gf5658
##  Bangxiong.Wu 2019/02/24        add for 18593 silead optical fp
##  Bangxiong.Wu 2019/03/18        add for 18073 hypnusd
##  Qing.Guan    2019/04/01        add for 18041 egis fp
##  Bangxiong.Wu 2019/05/10        add for SM7150 (MSM_19031 MSM_19331)
##  Dongnan.Wu   2019/05/19        add for 18593 hypnusd
##  Dongnan.Wu   2019/05/21        add for 19011 & 19301 platform
##  Dongnan.Wu   2019/05/24        add for 19328 platform
##  Dongnan.Wu   2019/05/24        add for goodix G3 module
##  Qing.Guan    2019/05/28        modify for 19071
##  Ziqing.Guo   2019/08/21        add performace module for fingerprint common
##  Ziqing.Guo   2019/08/22        optimize the structure
##  oujinrong    2019/08/29        remove libgf_hal_G2/libgf_hal_G3
##  Ziqing.Guo   2019/08/29        add dcs,healthmonitor, tool module
##  Dingtong.Liu 2019/09/29        add for 18085 silead optical fp
##  Ran.Chen     2019/11/11        add for OPLUS_CONFIG_REMOVE_VERSION_TEMP
##  Ran.Chen     2019/12/11        remove OPLUS_CONFIG_REMOVE_VERSION_TEMP
####################################################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.oplus.hardware.biometrics.fingerprint@2.1-service
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := vendor.oplus.hardware.biometrics.fingerprint@2.1-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := oplus
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VINTF_FRAGMENTS := xml/manifest_oplus_fingerprint.xml
LOCAL_SRC_FILES := service.cpp
LOCAL_SRC_FILES += fingerprint_type/fingerprint_type.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/fingerprint_type/include
LOCAL_CFLAGS := -DLOG_TAG='"android.hardware.biometrics.fingerprint@2.1-service"'
LOCAL_SHARED_LIBRARIES := libbinder liblog libhidlbase libhidltransport \
                          libutils vendor.oplus.hardware.biometrics.fingerprint@2.1 \
                          android.hardware.biometrics.fingerprint@2.3 \
                          android.hardware.biometrics.fingerprint@2.2 \
                          android.hardware.biometrics.fingerprint@2.1

LOCAL_STATIC_LIBRARIES := libcutils

ifeq ($(FPC_TEE_RUNTIME), TBASE)
LOCAL_SHARED_LIBRARIES += libMcClient
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES += libQSEEComAPI
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
LOCAL_SHARED_LIBRARIES += lib_fpc_ta_shared
endif

ifeq ($(TARGET_PRODUCT),trinket)
LOCAL_SHARED_LIBRARIES += libion
endif

ifeq ($(TARGET_PRODUCT),holi)
LOCAL_SHARED_LIBRARIES += libion
endif

ifeq ($(TARGET_PRODUCT),msmnile)
LOCAL_SHARED_LIBRARIES += libion
endif

LOCAL_LDLIBS += -llog -lhardware -lcutils
LOCAL_C_INCLUDES += hardware/libhardware/include

# ============for module vendor start============
# for goodix
ifeq ($(FP_GOODIX_SUPPORT), y)
LOCAL_CFLAGS         += -DOPLUS_GOODIX_SUPPORT

ifeq ($(GF_V111), 1)
LOCAL_CFLAGS += -DGF_V111
endif

ifeq ($(TARGET_BUILD_VARIANT),user)
LOCAL_CFLAGS         += -DCLOSE_SIMULATOR_TP
endif


ifeq ($(FP_GOOIDX_VERSION), side_fp)
LOCAL_CFLAGS         += -DSIDE_FP_ENABLE
endif

ifeq ($(FP_FPC_VERSION), side_fpc_fp)
LOCAL_CFLAGS         += -DSIDE_FPC_ENABLE
endif

LOCAL_SRC_FILES += \
    goodix/BiometricsFingerprint.cpp

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../goodix/capacitive_fp/gf_hal \
    $(LOCAL_PATH)/../goodix/capacitive_fp/gf_hal/public \
    $(LOCAL_PATH)/../goodix/capacitive_fp/gf_hal/include \
    $(LOCAL_PATH)/../goodix/capacitive_fp/public \
    $(LOCAL_PATH)/../goodix/capacitive_fp/include

LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_STATIC_LIBRARIES += libgf_hal \
                          libgf_ca \
                          libgf_algo \
                          libcutils
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/goodix/include

ifeq ($(FP_GOODIX_OPTICAL_SUPPORT), y)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/goodix_optical/include
LOCAL_SRC_FILES +=  goodix_optical/OpticalFingerprint.cpp
LOCAL_CFLAGS    += -DGOODIX_FP_ENABLE
endif

ifeq ($(SM7125_TMP_CALIBRATION_PARA), y)
LOCAL_CFLAGS    += -DSM7125_TMP_CALIBRATION_PARA
endif

# for fpc
ifeq ($(FPC_FP_ENABLE), y)
    LOCAL_SRC_FILES += \
           fpc_sw28/fpc_hidl_sw28.cpp
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/fpc_sw28/include
    LOCAL_STATIC_LIBRARIES += fpc_hal_common_sw28 fpc_tac_sw28
    LOCAL_CFLAGS    += -DFPC_FP_ENABLE
endif

#add for silead
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/silead_optical/include

ifeq ($(FP_SILEAD_OPTICAL_SUPPORT), y)
    ifneq ($(filter sdm660 sdm670 sdm710, $(TARGET_PRODUCT)),)
        LOCAL_SRC_FILES +=  silead_optical/SileadFingerprint670update.cpp
    else
        LOCAL_SRC_FILES +=  silead_optical/SileadFingerprint.cpp
    endif
    LOCAL_CFLAGS    += -DSILEAD_FP_ENABLE

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../silead/ca/
endif

#add for egis fp
ifeq ($(FP_EGIS_OPTICAL_SUPPORT), y)
LOCAL_SHARED_LIBRARIES += libegis_hal
LOCAL_C_INCLUDES +=  \
        $(LOCAL_PATH)/../egis_optical/egis_mtk/hal/inc

ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_C_INCLUDES +=  \
        $(LOCAL_PATH)/../egis_optical/egis_qcom/hal/inc
else
LOCAL_C_INCLUDES +=  \
        $(LOCAL_PATH)/../egis_optical/hal/inc
endif

LOCAL_SRC_FILES +=  \
        rbs_optical/rbs_hidl.cpp
LOCAL_C_INCLUDES +=  $(LOCAL_PATH)/rbs_optical/include
LOCAL_CFLAGS    += -DEGIS_FP_ENABLE
endif

ifeq ($(FP_EGIS_CAPACITIVE_SUPPORT), y)
ifeq ($(call is-vendor-board-platform,QCOM), true)
#
else
LOCAL_CFLAGS    += -DEGIS_CAPACITY_FP_ENABLE
LOCAL_SHARED_LIBRARIES += libRbsFlow_cap

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../egistec/egistec_mtk/egis_biometrics \
					$(LOCAL_PATH)/../egistec/egistec_mtk/common/platform/inc \
					$(LOCAL_PATH)/../egistec/egistec_mtk/flow \
					$(LOCAL_PATH)/../egistec/egistec_mtk/flow/def \
                                        $(LOCAL_PATH)/egis/include

LOCAL_SRC_FILES +=  ../egistec/egistec_mtk/egis_biometrics/egis_fingerprint.c \
		      egis/egis_hidl.cpp
endif
endif
# ============for module vendor end============

# ============for OPLUS feature start============
#add for calling hypunsd sevice
ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
LOCAL_C_INCLUDES += vendor/oplus/hardware/orms/1.0
LOCAL_SHARED_LIBRARIES += vendor.oplus.hardware.orms@1.0  \
                          libormshalclient
endif

ifeq ($(FP_CONFIG_BINDCORE_BYTID),yes)
LOCAL_CFLAGS += -DFP_BINDCORE_BYTID
endif

# add for performance
LOCAL_SRC_FILES += Perf/Perf.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Perf/include/

ifeq ($(FP_OPLUS_PLATFORMCPU_ALL8_BIG67), y)
LOCAL_CFLAGS    += -DFP_OPLUS_PLATFORMCPU_ALL8_BIG67
endif

ifeq ($(FP_OPLUS_PLATFORMCPU_ALL8_BIG4567), y)
LOCAL_CFLAGS    += -DFP_OPLUS_PLATFORMCPU_ALL8_BIG4567
endif

# add for dcs
LOCAL_C_INCLUDES += $(LOCAL_PATH)/dcs/include/
ifeq ($(FP_DCS_ENABLE), y)
LOCAL_SRC_FILES += dcs/dcs.cpp
LOCAL_CFLAGS += -DFP_DCS_ENABLE
LOCAL_SHARED_LIBRARIES += vendor.oplus.hardware.commondcs@1.0
endif

# add for health monitor
LOCAL_SRC_FILES += health/HealthMonitor.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/health/include/

# add for all the fingerprint_settings
ifeq ($(FP_SETTINGS_ENABLE), y)
LOCAL_CFLAGS += -DFP_CONFIG_SETTINGS_ENABLE
LOCAL_SRC_FILES += fingerprint_settings/FingerprintSettings.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/fingerprint_settings/include/
endif

# add for message/handler
LOCAL_SRC_FILES += tools/FpMessage.cpp \
                   tools/Handler.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/tools/include/
# ============for OPLUS feature end============

include $(BUILD_EXECUTABLE)
