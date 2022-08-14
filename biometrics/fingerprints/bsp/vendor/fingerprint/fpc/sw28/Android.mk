####################################################################################
## File: - SDM660.LA.1.0\android\vendor\qcom\proprietary\securemsm\fpc\Android.mk
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      Fingerprint TEE hal Feature Config for FPC Android O SW23.2.2
##
## Version: 1.0
## Date created: 18:03:11,09/10/2017
## Author: Ziqing.guo@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Ziqing.guo   2017/10/09        create the file
##  Ziqing.guo   2017/10/22        add FPC_CONFIG_WAKELOCK
##  Bin.Li       2017/12/12        add 6771 for back fingerpint.
##  Long.Liu     2018/11/19        add 6779_18151 for back fingerpint.
##  Ran.Chen     2018/11/26        remove fpc for SDM855
##  Hongyu.lu    2019/04/26        add 19021 19321 19026 for back fingerpint
##  Hongyu.lu    2019/04/26        add 19328 for back fingerpint
##  oujinrong    2019/06/24        remove project macro
####################################################################################

# ----------------------------
# Add feature flags below
# ----------------------------

ifeq ($(FP_CONFIG_NAVIGATION_ENABLE), y)
FPC_CONFIG_NAVIGATION=1
else
FPC_CONFIG_NAVIGATION=0
endif

ifeq ($(FP_FPC_VERSION), side_fpc_fp)
SIDE_FPC_ENABLE=1
else
SIDE_FPC_ENABLE=0
endif

ifeq ($(FP_SUPPORT_SIDE), y)
FPC_CHECK_BROKEN=1
else
FPC_CHECK_BROKEN=0
endif

ifeq ($(FP_CONFIG_SHMBRIDGE_ION_FUNCTION), yes)
FP_CONFIG_SHMBRIDGE_ION_FUNCTION=1
endif

ifeq ($(call is-vendor-board-platform,QCOM),true)
FPC_TEE_RUNTIME=QSEE
FPC_CONFIG_QSEE4=1
FPC_CONFIG_TA_FS=1
else
FPC_TEE_RUNTIME=TBASE
endif

FPC_CONFIG_TA_DB_BLOB=1
FPC_CONFIG_HW_AUTH=1
FPC_CONFIG_ENGINEERING=0
FPC_CONFIG_SENSORTEST=1
FPC_CONFIG_DEBUG=1
FPC_CONFIG_WAKE_LOCK=1
FPC_TA_HEAP_DEBUG=0

include $(call all-subdir-makefiles)
