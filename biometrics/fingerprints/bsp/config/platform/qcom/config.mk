#################################################################################
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
##  liudingtong   2019/09/28        Add silead support
##  Ran.Chen      2019/08/17        Add for hypnus (SDM7250)
##  Ran.Chen      2019/11/11        remove FP_HBM_BRIGHTNESS_DELAY (SDM7250)
##  zhoubo        2019/12/25        Add OPLUS_CONFIG_GOODIX_VERSION=V03_02_02_162 (SM6125)
##  zhoubo        2020/03/05        Add for hypnus (SM6125)
##################################################################################

#====================================SDM710 BEGIN==============================================#

ifeq ($(TARGET_PRODUCT),sdm710)

#add common feature
FP_DCS_ENABLE=y

ifneq ($(filter 18097 18539 18397 18041 18383, $(BUILD_PRODUCT_VARIANT)),)
OPLUS_DEVICE_SUPPORT_DSP=yes
endif

#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y

FP_SILEAD_OPTICAL_SUPPORT=y

OPLUS_CONFIG_ANDROID_O_NEW_DEVICE=1

#add for hypnus bind core
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
FP_CONFIG_HYPNUSD_ENABLE=1


#add for  bind big core
FP_CONFIG_BINDCORE_BYTID=yes
#add for  bind big core

endif
#====================================SDM710 END================================================#

#====================================SM7150 BEGIN==============================================#

ifeq ($(TARGET_PRODUCT),sm6150)

#add common feature
FP_DCS_ENABLE=y

#enable DSP
OPLUS_DEVICE_SM7150=yes
#OPLUS_DEVICE_SUPPORT_DSP=yes
#OPLUS_CONFIG_DYNAMIC_COMPATIBLE_DSP=yes

#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y

#add for hypnus
FP_CONFIG_HYPNUSD_ENABLE=1
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y

endif

#====================================SM7150 END================================================#

#====================================SM7250 BEGIN==============================================#

ifeq ($(TARGET_PRODUCT),lito)
#add goodix capacitive fingerprint module support feature
ifneq ($(OPLUS_FEATURE_FINGERPRINT),oneplus_upgrade)
FP_GOODIX_SUPPORT=y
endif
#add silead capacitive fingerprint module support feature
FP_SILEAD_SUPPORT=y
#add jiiov fingerprint module support feature
FP_JIIOV_OPTICAL_SUPPORT=y
FP_JIIOV_SENSOR_TYPE=JV0301
#add goodix fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
FP_DCS_ENABLE=y
#add for optical calibration
FP_SETTINGS_ENABLE=y

#add for hypnus
FP_CONFIG_HYPNUSD_ENABLE=1
FP_CONFIG_BINDCORE_BYTID=yes
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
#add for ux thread
FP_CONFIG_SET_PRIORITY=yes
FP_CONFIG_SET_UXTHREAD=yes
#add for cdsp
OPLUS_DEVICE_SDM7250=yes
OPLUS_DEVICE_SUPPORT_DSP=yes
OPLUS_DEVICE_SUPPORT_DSP_COMPATIBLE_VERSION_G3=yes
OPLUS_DEVICE_SUPPORT_DSP_G5=yes
OPLUS_DEVICE_SUPPORT_DSP_G6=yes
#OPLUS_SUPPORT_STABILITYTEST=yes
FP_JIIOV_TEMPLATE_UPDATE=y

#dump encrypt
OPLUS_CUSTOM_ENCRYPT_ENABLE =y
endif
#====================================SM7250 END================================================#

#====================================SM8150 BEGIN==============================================#

ifeq ($(TARGET_PRODUCT),msmnile)
FP_GOODIX_OPTICAL_SUPPORT=y

FP_GOODIX_SUPPORT := y
FPC_TEE_RUNTIME := QSEE

OPLUS_DEVICE_SUPPORT_DSP=yes
OPLUS_DEVICE_SUPPORT_DSP_COMPATIBLE_VERSION_G3=yes

FP_DCS_ENABLE=y

FP_CONFIG_HYPNUSD_ENABLE=1
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y
FP_CONFIG_BINDCORE_BYTID=yes
endif

#====================================SM8150 END================================================#

#====================================SM8250 BEGIN==============================================#

ifeq ($(TARGET_PRODUCT), kona)

#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
FP_JIIOV_OPTICAL_SUPPORT=y
FP_JIIOV_SENSOR_TYPE=JV0301
#add DSP support feature
OPLUS_DEVICE_SDM8250=yes
OPLUS_DEVICE_SUPPORT_DSP_G5=yes
OPLUS_DEVICE_SUPPORT_DSP_G6=yes
#FP_HBM_BRIGHTNESS_DELAY=yes
FP_SETTINGS_ENABLE=y
#add for hypnus and dcsmsg feature
FP_CONFIG_HYPNUSD_ENABLE=1
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_CONFIG_SET_PRIORITY=yes
FP_CONFIG_SET_UXTHREAD=yes
OPLUS_DEVICE_SUPPORT_DSP=yes
OPLUS_DEVICE_SUPPORT_DSP_COMPATIBLE_VERSION_G3=yes
#add for distinguish different TA_NAME in one project name
FP_CONFIG_MULTI_TA=yes

#fingerprint dump encrypt
OPLUS_CUSTOM_ENCRYPT_ENABLE =y
endif

#====================================SM8250 END==============================================#

#====================================SM6125 BEGIN==============================================#

ifeq ($(TARGET_PRODUCT),trinket)
#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
FP_DCS_ENABLE=y

#add for hypnus bind core
FP_CONFIG_HYPNUSD_ENABLE=1
FP_CONFIG_BINDCORE_BYTID=yes
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y

#add for 19021 series FPC and GOODIX
FPC_FP_ENABLE=y
FP_FPC_VERSION=side_fpc_fp

FP_GOODIX_SUPPORT=y
FP_GOODIX_CAMERA_KEY=y

FP_CONFIG_NAVIGATION_ENABLE=y
FPC_CONFIG_SEND_RESET=1
OPLUS_CONFIG_ANDROID_O_NEW_DEVICE=1
FP_CONFIG_SHMBRIDGE_ION_FUNCTION=yes
endif

#====================================SM6125 END================================================#

#====================================SDM660 BEGIN==============================================#

ifeq ($(TARGET_PRODUCT),sdm660_64)

#add fingerprint module support feature
FPC_FP_ENABLE=y
FP_FPC_SW23_SUPPORT=y

FP_GOODIX_SUPPORT=y
FP_GOOIDX_VERSION=v010101


OPLUS_CONFIG_ANDROID_O_NEW_DEVICE=1

#add for hypnus
FP_CONFIG_HYPNUSD_ENABLE=1


#copy ta to vendor partition for there is no product partition now
-include vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/17081_17085/fpc_ta/common.mk
-include vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/17081_17085/goodix_ta/common.mk

endif

#====================================SDM660 END================================================#

#====================================SM7125 BEGIN==============================================#
ifeq ($(TARGET_PRODUCT), atoll)
#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
#add jiiov fingerprint module support feature
FP_JIIOV_OPTICAL_SUPPORT=y
FP_JIIOV_SENSOR_TYPE=JV0301
FP_JIIOV_TEMPLATE_UPDATE=y
#FP_HBM_BRIGHTNESS_DELAY=yes
FP_CONFIG_HYPNUSD_ENABLE=1
FP_DCS_ENABLE=y
FP_SETTINGS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y
FP_CONFIG_BINDCORE_BYTID=yes
#add for cdsp config
OPLUS_DEVICE_SM7125=yes
OPLUS_DEVICE_SUPPORT_DSP_G6=yes
#add below two for DSP_G3
OPLUS_DEVICE_SUPPORT_DSP=yes
OPLUS_DEVICE_SUPPORT_DSP_COMPATIBLE_VERSION_G3=yes
endif
#====================================SM7125 BEGIN==============================================#

#====================================SM8350 BEGIN==============================================#
ifeq ($(TARGET_PRODUCT), lahaina)
#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
FP_JIIOV_OPTICAL_SUPPORT=y
FP_JIIOV_SENSOR_TYPE=JV0301
FP_SETTINGS_ENABLE=y
#add DSP support feature

#add for hypnus and dcsmsg feature
FP_CONFIG_HYPNUSD_ENABLE=1
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_CONFIG_SET_PRIORITY=yes
FP_CONFIG_SET_UXTHREAD=yes
FP_CONFIG_UXTHREAD_HIGHT_LEVEL=yes
endif
#====================================SM8350 BEGIN==============================================#

#====================================SM4350 BEGIN==============================================#
ifeq ($(TARGET_PRODUCT), holi)
#add fingerprint module support feature

#add DSP support feature

#add for hypnus and dcsmsg feature
FP_CONFIG_HYPNUSD_ENABLE=1
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG67=y
FP_CONFIG_BINDCORE_BYTID=yes

#add for goodix side fingerprint

FP_GOODIX_SUPPORT=y
#FP_GOOIDX_VERSION=v010101

#add for FPC side fingerprint
FP_SUPPORT_SIDE=y
FPC_FP_ENABLE=y
FP_FPC_VERSION=side_fpc_fp
FP_CONFIG_NAVIGATION_ENABLE=y
FP_CONFIG_SHMBRIDGE_ION_FUNCTION=yes
OPLUS_CONFIG_ANDROID_O_NEW_DEVICE=1
endif
#====================================SM4350 END=========================================

#====================================SM8450 BEGIN==============================================#
ifeq ($(TARGET_PRODUCT), taro)
FP_CONFIG_HYPNUSD_ENABLE=1
FP_CONFIG_SET_UXTHREAD=yes
FP_CONFIG_UXTHREAD_HIGHT_LEVEL=yes
FP_DCS_ENABLE=y
FP_TRACE_TAG_ENABLE=1
endif
#====================================SM8450 END================================================#

#====================================SM6115 BEGIN==============================================#
ifeq ($(TARGET_PRODUCT), bengal)
#add fingerprint module support feature
FP_GOODIX_OPTICAL_SUPPORT=y
FP_JIIOV_OPTICAL_SUPPORT=y
FP_SETTINGS_ENABLE=y
FP_EGIS_CAPACITIVE_SUPPORT=y
FP_SILEAD_SUPPORT=y
FP_CHIPONE_CAPACITIVE_SUPPORT=y
#add DSP support feature


#add for hypnus and dcsmsg feature
FP_CONFIG_HYPNUSD_ENABLE=1
FP_DCS_ENABLE=y
FP_OPLUS_PLATFORMCPU_ALL8_BIG4567=y
FP_CONFIG_BINDCORE_BYTID=yes
FP_CONFIG_SET_UXTHREAD=yes
FP_CONFIG_SET_PRIORITY=yes
endif
#====================================SM6115 END==============================================#
