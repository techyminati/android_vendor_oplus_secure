####################################################################################
## File: - fingerprints_hal/vendor/fingerprint/fpc/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint Common Feature Config for Android O
##
## Version: 1.0
## Date created: 18:03:11,08/12/2017
## Author: Ziqing.guo@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Ziqing.guo   2017/12/08        create the file
##  Yang.Tan     2018/11/26        add fpc sw23 and sw28 compatible
##  Long.Liu     2018/12/19        add fpc1511 sw28 version for 18151
##  Long.Liu     2019/01/03        add for 18161 fpc1511 sw28 version
##  Hobgyu.lu    2019/04/25        add for 19021 19321 19026 fpc1511 sw28 version
##  Bangxiong.Wu 2019/05/10        remove fpc for SM7150 (MSM_19031 MSM_19331)
##  Hobgyu.lu    2019/05/24        remove fpc for 19328
##  oujinrong    2019/06/24        remove project ID
####################################################################################

# ----------------------------
# Add feature flags below
# ----------------------------
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/sw28/Android.mk
