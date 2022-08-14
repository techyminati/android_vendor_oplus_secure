##################################################################################
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2019, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      fingerprints_hal qcom config
##
## Version: 2.0
## Date created: 2019/08/17
## Author: oujinrong@BSP.Fingerprint.Basic
## TAG: BSP.Fingerprint.Basic
##
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  oujinrong     2019/08/17        Create file
##  ziqing.guo    2019/08/21        Add platform define
##  shengwang.luo 2019/09/26        Add FPC fingerprint support
##  Zemin.Li      2020/02/18        Add goodix optical support for 19531 mt6771
##  Zemin.Li      2020/03/13        Add spi_ta/common.mk for mt6771
##################################################################################


#====================================MT6779 BEGIN==============================================#

ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6779)
#GF_OPEN_DUMP_TEMP=yes
FP_HBM_BRIGHTNESS_DELAY=yes
FP_CONFIG_HYPNUSD_ENABLE=1
#OPLUS_DEVICE_SUPPORT_DSP=yes
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
FP_GOODIX_OPTICAL_SUPPORT=y
#FP_SILEAD_OPTICAL_SUPPORT=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_EGIS_OPTICAL_SUPPORT=y
endif
#====================================MT6779 END================================================#


#====================================MT6771 BEGIN==============================================#
ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6771)
FP_MTK_GREATER_THAN_TEE500=y
FPC_FP_ENABLE=y
FP_GOODIX_SUPPORT=y
FP_GOODIX_OPTICAL_SUPPORT=y
FP_DCS_ENABLE=y
endif
#====================================MT6771 END==============================================#


#====================================MT6785 BEGIN==============================================#

ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6785)
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_MTK_GREATER_THAN_TEE500=y
FP_GOODIX_SUPPORT=y
FP_GOOIDX_VERSION=side_fp
FPC_FP_ENABLE=y
FP_FPC_VERSION=side_fpc_fp
endif
#====================================MT6785 END================================================#


#====================================MT6768 BEGIN==============================================#

ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6769)
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_MTK_GREATER_THAN_TEE500=y
FP_EGIS_CAPACITIVE_SUPPORT =y
endif
#====================================MT6768 END================================================#

#====================================MT6877 BEGIN==============================================#

ifeq ($(strip $(TARGET_PRODUCT)),vnd_k6877v1_64)
FP_DCS_ENABLE=y
FP_OPPO_PLATFORMCPU_ALL8_BIG67=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_MTK_GREATER_THAN_TEE500=y

#add fingerprint module support feature
FP_SETTINGS_ENABLE=y
#add fingerprint module support feature
FP_HBM_BRIGHTNESS_DELAY=yes
#add for hypnus
FP_CONFIG_HYPNUSD_ENABLE=1
#add for cdsp
FP_JIIOV_OPTICAL_SUPPORT=y
endif
#====================================MT6877 END================================================#

#====================================MT6877T BEGIN==============================================#

ifeq ($(strip $(TARGET_PRODUCT)),vnd_k6877v1_64_6877T)
FP_DCS_ENABLE=y
FP_OPPO_PLATFORMCPU_ALL8_BIG67=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_MTK_GREATER_THAN_TEE500=y

#add fingerprint module support feature
FP_SETTINGS_ENABLE=y
#add fingerprint module support feature
FP_HBM_BRIGHTNESS_DELAY=yes
#add for hypnus
FP_CONFIG_HYPNUSD_ENABLE=1
#add for cdsp
FP_JIIOV_OPTICAL_SUPPORT=y
endif
#====================================MT6877T END================================================#

#====================================MT6889 BEGIN==============================================#

ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6889)

#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
FP_DCS_ENABLE=y
#FP_HBM_BRIGHTNESS_LEVEL=2048
FP_SETTINGS_ENABLE=y

#add fingerprint module support feature
#FP_HBM_BRIGHTNESS_DELAY=yes

#open goodixfp dump
GF_OPEN_DUMP_TEMP=yes
#GF_BUILD_BY_RELEASE_LIB=yes

#add for hypnus
FP_CONFIG_HYPNUSD_ENABLE=1
FP_CONFIG_BINDCORE_BYTID=yes
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y

#add for cdsp
#OPLUS_DEVICE_SUPPORT_DSP=no

endif
#====================================MT6889 END================================================#

#====================================MT6853 BEGIN==============================================#
ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6853)
#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
GF_OPEN_DUMP_TEMP=yes
#add for optical calibration
FP_SETTINGS_ENABLE=y
#add DSP support feature
OPLUS_DEVICE_SUPPORT_DSP=no
FP_CONFIG_HYPNUSD_ENABLE=1
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_DCS_ENABLE=y
#add fingerprint module support feature
FP_HBM_BRIGHTNESS_DELAY=yes
OPLUS_CONFIG_GOODIX_VERSION=V03_02_02_164_01
FP_MTK_GREATER_THAN_TEE500=y
endif
#====================================MT6853 END================================================#

#====================================MT6873 BEGIN==============================================#
ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6873)
#Add for FPC Fingerprint IC
FPC_FP_ENABLE=y
FP_FPC_VERSION=side_fpc_fp
FP_CONFIG_NAVIGATION_ENABLE=y

#Add for Gooidx Fingerprint IC
FP_GOODIX_SUPPORT=y
FP_GOOIDX_VERSION=side_fp

#FP_CONFIG_HYPNUSD_ENABLE=1
#FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
#FP_CONFIG_BINDCORE_BYTID=yes
FP_DCS_ENABLE=y
FP_MTK_GREATER_THAN_TEE500=y
endif
#====================================MT6873 END================================================#

#====================================MT6885 BEGIN==============================================#
ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6885)
#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
GF_OPEN_DUMP_TEMP=yes

#add DSP support feature
#OPLUS_DEVICE_SUPPORT_DSP=yes
#FP_CONFIG_HYPNUSD_ENABLE=1
#FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y
FP_DCS_ENABLE=y

#add fingerprint module support feature
FP_HBM_BRIGHTNESS_DELAY=yes
OPLUS_CONFIG_GOODIX_VERSION=V03_02_02_164_01
FP_MTK_GREATER_THAN_TEE500=y
endif

#====================================MT6885 END================================================#
#====================================MT6891 BEGIN==============================================#

ifeq ($(strip $(TARGET_PRODUCT)),vnd_oppo6891)
FP_HBM_BRIGHTNESS_DELAY=yes
FP_CONFIG_HYPNUSD_ENABLE=1
#OPPO_DEVICE_SUPPORT_DSP=yes
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y
FP_GOODIX_OPTICAL_SUPPORT=y
FP_SETTINGS_ENABLE=y
OPLUS_CUSTOM_ENCRYPT_ENABLE=y
FP_CONFIG_BINDCORE_BYTID=yes
endif
#====================================MT891 END================================================#
