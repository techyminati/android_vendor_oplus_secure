##***********************************************************************************
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
## 
## Description:
##      fingerprints init sh
## 
## Version: 1.0
## Date created: 2017/02/14
## Author: Haitao.Zhou@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## 
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Haitao.Zhou   2017/02/14        NULL
##**********************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE       := init.oplus.fingerprints.sh
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_EXECUTABLES)
ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_SRC_FILES    := init.oplus.fingerprints_qcom.sh
else
LOCAL_SRC_FILES    := init.oplus.fingerprints_mtk.sh
endif
include $(BUILD_PREBUILT)
