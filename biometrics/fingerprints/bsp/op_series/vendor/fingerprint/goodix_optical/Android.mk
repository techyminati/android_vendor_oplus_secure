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
##  Ziqing.Guo   2019/08/16        create the file for goodix optical Android.mk
##  oujinrong    2019/08/17        remove feature config
####################################################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Include all makefiles in subdirectories
include $(call all-makefiles-under,$(LOCAL_PATH))
