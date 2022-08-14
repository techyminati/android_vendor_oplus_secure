##################################################################################
## Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
##
## File: - vendor/fingerprint/Android.mk
##
## Description:
##      HIDL Service entry for fingerprint
##
## Version: 1.0
## Date created: 18:03:11,26/03/2021
## Author: Ran.Chen@BSP.Fingerprint.Basic
## TAG: BSP.Fingerprint.Basic
## ---------------------------Revision History--------------------------------
##  <author>      <data>            <desc>
##  Ran.Chen   2021/03/26        create the file
##################################################################################

CUSTOM_IMAGE_MODULES += vendor.oplus.hardware.biometrics.fingerprint@2.1-service.rc
CUSTOM_IMAGE_MODULES += vendor.oplus.hardware.biometrics.fingerprint@2.1-service

CUSTOM_IMAGE_MODULES += init.oplus.fingerprints.rc
CUSTOM_IMAGE_MODULES += init.oplus.fingerprints.sh

ifeq ($(CONFIG_FINGERPRINT_PLATFORM_REE),yes)
CUSTOM_IMAGE_MODULES += libfpta
endif