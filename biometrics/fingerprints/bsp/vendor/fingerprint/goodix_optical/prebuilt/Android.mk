####################################################################################
## File: - vendor/fingerprint/goodix_optical/prebuilt/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2019-2022, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint Prebuilt Config for Android
##
## Version: 1.0
## Date created: 18:03:11,08/16/2019
## Author: Ziqing.Guo@BSP.Fingerprint.Basic
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>       <data>            <desc>
##  Ziqing.Guo   2019/08/16        create the file for goodix optical prebuilt file
##  Ran.Chen     2019/11/27        add for OPLUS_DEVICE_SUPPORT_DSP_G5
####################################################################################

ifeq ($(call is-vendor-board-platform,QCOM), true)
LOCAL_PATH := $(call my-dir)
MODE := release

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

ifneq ($(filter yes, $(OPLUS_DEVICE_SUPPORT_DSP))_$(filter yes, $(OPLUS_DEVICE_SUPPORT_DSP_G5))_$(filter yes, $(OPLUS_DEVICE_SUPPORT_DSP_G6)),__)
include $(CLEAR_VARS)
LOCAL_MODULE        := libproxy_skel
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 32
LOCAL_MODULE_TAGS   := optional
ifeq ($(TARGET_PRODUCT),msmnile)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/dsp_lib_SDM855/libproxy_skel.so
else ifeq ($(TARGET_PRODUCT),sm6150)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/dsp_lib_SM7150/libproxy_skel.so
else ifeq ($(OPLUS_DEVICE_SDM7250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/dsp_lib_SDM7250/libproxy_skel.so
else ifeq ($(OPLUS_DEVICE_SDM8250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/dsp_lib_SDM8250/libproxy_skel.so
else ifeq ($(OPLUS_DEVICE_SM7125),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/dsp_lib_SM7125/libproxy_skel.so
else
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/dsp_lib/libproxy_skel.so
endif
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/rfsa/adsp
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libhvx_proxy_stub
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 32
LOCAL_MODULE_TAGS   := optional
ifeq ($(TARGET_PRODUCT),msmnile)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM855/armeabi-v7a/libhvx_proxy_stub.so
else ifeq ($(OPLUS_DEVICE_SDM7250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM7250/armeabi-v7a/libhvx_proxy_stub.so
else ifeq ($(OPLUS_DEVICE_SDM8250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM8250/armeabi-v7a/libhvx_proxy_stub.so
else ifeq ($(OPLUS_DEVICE_SM7125),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SM7125/armeabi-v7a/libhvx_proxy_stub.so
else
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib/armeabi-v7a/libhvx_proxy_stub.so
endif
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libarm_proxy_skel
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 32
LOCAL_MODULE_TAGS   := optional
ifeq ($(TARGET_PRODUCT),msmnile)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM855/armeabi-v7a/libarm_proxy_skel.so
else ifeq ($(OPLUS_DEVICE_SDM7250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM7250/armeabi-v7a/libarm_proxy_skel.so
else ifeq ($(OPLUS_DEVICE_SDM8250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM8250/armeabi-v7a/libarm_proxy_skel.so
else ifeq ($(OPLUS_DEVICE_SM7125),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SM7125/armeabi-v7a/libarm_proxy_skel.so
else
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib/armeabi-v7a/libarm_proxy_skel.so
endif
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib/
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libhvx_proxy_stub
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_TAGS   := optional
ifeq ($(TARGET_PRODUCT),msmnile)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM855/arm64-v8a/libhvx_proxy_stub.so
else ifeq ($(OPLUS_DEVICE_SM7150),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SM7150/arm64-v8a/libhvx_proxy_stub.so
else ifeq ($(OPLUS_DEVICE_SDM7250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM7250/arm64-v8a/libhvx_proxy_stub.so
else ifeq ($(OPLUS_DEVICE_SDM8250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM8250/arm64-v8a/libhvx_proxy_stub.so
else ifeq ($(OPLUS_DEVICE_SM7125),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SM7125/arm64-v8a/libhvx_proxy_stub.so
else
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib/arm64-v8a/libhvx_proxy_stub.so
endif
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE        := libarm_proxy_skel
LOCAL_MODULE_CLASS  := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_STRIP_MODULE  := false
LOCAL_MULTILIB      := 64
LOCAL_MODULE_TAGS   := optional
ifeq ($(TARGET_PRODUCT),msmnile)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM855/arm64-v8a/libarm_proxy_skel.so
else ifeq ($(OPLUS_DEVICE_SM7150),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SM7150/arm64-v8a/libarm_proxy_skel.so
else ifeq ($(OPLUS_DEVICE_SDM7250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM7250/arm64-v8a/libarm_proxy_skel.so
else ifeq ($(OPLUS_DEVICE_SDM8250),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SDM8250/arm64-v8a/libarm_proxy_skel.so
else ifeq ($(OPLUS_DEVICE_SM7125),yes)
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib_SM7125/arm64-v8a/libarm_proxy_skel.so
else
LOCAL_SRC_FILES     := libs_qcom/hvx_lib/ap_lib/arm64-v8a/libarm_proxy_skel.so
endif
LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/$(TARGET_COPY_OUT_VENDOR)/lib64/
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
endif
endif
