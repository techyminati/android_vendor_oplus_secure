####################################################################################
## File: - fingerprints_hal/vendor/fingerprint/Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
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
##  Long.Liu     2018/11/19    modify 18531 static test
##  Long.Liu     2019/01/03    add static test checklist for 18161 fpc1511
##  Ran.Chen     2019/03/08    add secureDSP for goodixfp (SDM855)
##  Bangxiong.Wu 2019/03/21    add hypnusd flag for 18073
##  Hongyu.lu    2019/04/25    add hypnusd flag for 19021 19321 19026
##  Dongnan.Wu   2019/05/17    add hypnusd flag for 18593
##  Bangxiong.Wu 2019/05/18    Add chip type flag for goodix_G3.0
##  Dongnan.Wu   2019/05/22    add g3 chip type for 19011 & 19301
##  Hongyu.lu    2019/05/24    add g3 chip type for 19328
##  Qing.Guan    2019/05/28    modify for 19071
##  oujinrong    2019/06/21    disable dsp feature for 18097
##  oujinrong    2019/06/24    remove feature config for fingerprint module
####################################################################################
ifeq ($(OPLUS_FEATURE_FINGERPRINT),yes)
include $(call all-subdir-makefiles)
endif