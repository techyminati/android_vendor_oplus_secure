####################################################################################
## File: - vendor/fingerprint/egis_optical/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2010-2019, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint Common Feature Config for Android P
##
## Version: 1.0
## Date created: 15:22:11,11/29/2019
## Author: Qijia.Zhou
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Qijia.Zhou   2019/11/29        create the file
####################################################################################

# ----------------------------
# Add feature flags below
# ----------------------------
LOCAL_PATH := $(call my-dir)

ifeq ($(call is-vendor-board-platform,QCOM), true)
#include $(LOCAL_PATH)/egis_qcom/Android.mk
else
include $(LOCAL_PATH)/egis_mtk/Android.mk
endif

#include $(call all-subdir-makefiles)
