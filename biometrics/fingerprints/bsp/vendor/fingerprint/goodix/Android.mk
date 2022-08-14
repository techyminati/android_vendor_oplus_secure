####################################################################################
## File: - vendor/fingerprint/goodix/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint Common Feature Config for Android O
##
## Version: 1.0
## Date created: 18:03:11,01/30/2019
## Author: Long.Liu@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Long.Liu   2019/01/30        create the file
##  Long.Liu   2019/02/22        modify for 18161 support GOODIX gf5658
##  Jiaqi.Wu   2020/01/20        modify for 19131 support GOODIX gf3626
####################################################################################

# ----------------------------
# Add feature flags below
# ----------------------------
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/capacitive_fp/Android.mk

