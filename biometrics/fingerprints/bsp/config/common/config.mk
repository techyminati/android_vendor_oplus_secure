##################################################################################
## OPLUS_FEATURE_FINGERPRINT
## Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
##
## Description:
##      fingerprints_hal config
##
## Version: 2.0
## Date created: 2017/11/02
## Author: Ziqing.Guo@Prd.BaseDrv
## TAG: BSP.Fingerprint.Basic
##
## --------------------------- Revision History: --------------------------------
##  <author>      <data>            <desc>
##  Ziqing.Guo    2017/11/02        Create file
##  Ziqing.Guo    2017/11/16        Add alipay service
##  Ping.Liu      2017/11/16        modify for cryptoeng.
##  Hongdao.yu    2017/11/21        modify for face
##  Hongdao.yu    2017/12/07        remove face for new repo
##  ran.chen      2018/01/29        modify for goodix static_lib
##  Hongdao.yu    2018/03/14        add goodix 5298(5228+158)
##  Ping.Liu      2018/01/26        modify for cryptoeng compatible with mtk.
##  Bin.Li        2018/03/12        add for devicebinding of trustonic tee in MTK6771
##  Ran.Chen      2018/05/28        add for SDM670 (MSM_18081 MSM18381)
##  Ran.Chen      2018/05/28        add for goodix_optical
##  Ran.Chen      2018/06/15        add for silead_optical
##  Liu.Long      2018/11/19        add for MT6779 SPI ta
##  Ran.Chen      2018/11/26        add for SDM855
##  Bangxiong.Wu  2019/01/26        add for MT6779 silead_optical_ta
##  Bangxiong.Wu  2019/02/24        add for 18593 silead_optical_fp
##  Qing.Guan     2019/04/01        add for egis optical fp 18041
##  Hongyu.Lu     2019/04/25        add for egis optical fp 19021 19321 18026
##  Qing.Guan     2019/05/08        add for silead O to P 18081
##  Bangxiong.Wu  2019/05/10        add for SM7150 (MSM_19031 MSM_19331)
##  Qing.Guan     2019/05/15        add for silead P 18081 8bit alg
##  Qing.Guan     2019/05/22        splite upgrade and opring andorid P
##  Hongyu.Lu     2019/04/25        add for egis optical fp 19328
##  Qing.Guan     2019/05/25        modify for 19071
##  Ziqing.Guo    2019/08/16        modify for goodix optical euclid
##  oujinrong     2019/08/27        modify for euclid of TA
##  Ran.Chen      2019/09/10        add for goodix_G5
##  oujinrong     2019/11/25        remove for G5
##  Ran.Chen      2020/03/10        add for G6
##################################################################################

#add for goodix_optical_fp G2
CUSTOM_IMAGE_MODULES += fingerprint.goodix_G2.default
CUSTOM_IMAGE_MODULES += libgf_hal_G2

#add for goodix_optical_fp G3
CUSTOM_IMAGE_MODULES += fingerprint.goodix_G3.default
CUSTOM_IMAGE_MODULES += libgf_hal_G3

#add for goodix_optical_fp G5
CUSTOM_IMAGE_MODULES += fingerprint.goodix_G5.default
CUSTOM_IMAGE_MODULES += libgf_hal_G5

#add for goodix_optical_fp G6
CUSTOM_IMAGE_MODULES += fingerprint.goodix_G6.default
CUSTOM_IMAGE_MODULES += libgf_hal_G6

#add for goodix_optical_fp G7
CUSTOM_IMAGE_MODULES += fingerprint.goodix_G7.default
CUSTOM_IMAGE_MODULES += libgf_hal_G7

#add for oneplus 8 series
ifeq ($(strip $(OPLUS_FEATURE_FINGERPRINT)),oneplus_upgrade)

#add for oplus 20828_goodix_optical_G6_7.0
CUSTOM_IMAGE_MODULES += fingerprint.20828_goodix9678.default
CUSTOM_IMAGE_MODULES += libgf_hal_20828_G6_7_oplus

#add for oplus 19805_goodix_optical_G6_7.0
CUSTOM_IMAGE_MODULES += fingerprint.19805_goodix9678_G6_7.default
CUSTOM_IMAGE_MODULES += libgf_hal_19805_G6_7_oplus

#add for oplus 19805_goodix_optical_G6_3.0 for 19805/20838
CUSTOM_IMAGE_MODULES += fingerprint.19805_goodix9638_G6_3.default
CUSTOM_IMAGE_MODULES += libgf_hal_19805_G6_3_oplus

#add for oplus 19821_goodix_optical_G5
CUSTOM_IMAGE_MODULES += fingerprint.goodix9608_G5.default
CUSTOM_IMAGE_MODULES += libgf_hal_G5_oplus

#add for oplus 19821_goodix_optical_G6_3.0
CUSTOM_IMAGE_MODULES += fingerprint.goodix9638_G6.default
CUSTOM_IMAGE_MODULES += libgf_hal_G6_oplus

#add for oplus 19811_goodix_optical_G6_3.0
CUSTOM_IMAGE_MODULES += fingerprint.19811_goodix9638_G6.default
CUSTOM_IMAGE_MODULES += libgf_hal_19811_G6_oplus

#add for oplus 20801 goodix_optical_G3
CUSTOM_IMAGE_MODULES += fingerprint.20801_goodix9558_G3.default
CUSTOM_IMAGE_MODULES += libgf_ud_hal_20801_G3_oplus
endif
#add for oneplus 8 series end

#add for secure dsp
#CUSTOM_IMAGE_MODULES += libproxy_skel
#CUSTOM_IMAGE_MODULES += libhvx_proxy_stub
#CUSTOM_IMAGE_MODULES += libarm_proxy_skel

#add for silead_optical_fp
CUSTOM_IMAGE_MODULES += fingerprint.silead.default
CUSTOM_IMAGE_MODULES += libsl_fp_prepro
CUSTOM_IMAGE_MODULES += libsl_fp_algo
CUSTOM_IMAGE_MODULES += libsl_fp_impl_util
CUSTOM_IMAGE_MODULES += libsl_fp_nosec
CUSTOM_IMAGE_MODULES += libsl_fp_impl
CUSTOM_IMAGE_MODULES += fingerprint.silead_16bit.default
CUSTOM_IMAGE_MODULES += libsl_fp_impl_16bit

#add for egis_optional_fp
CUSTOM_IMAGE_MODULES += libegis_hal
CUSTOM_IMAGE_MODULES += libets_teeclient_v3
CUSTOM_IMAGE_MODULES += libRbsFlow

#add for jiiov
CUSTOM_IMAGE_MODULES += anc.hal

#add for egis_cap_fp
CUSTOM_IMAGE_MODULES += libRbsFlow_cap

CUSTOM_IMAGE_MODULES += vendor.oplus.hardware.biometrics.fingerprint@2.1-service.rc
CUSTOM_IMAGE_MODULES += vendor.oplus.hardware.biometrics.fingerprint@2.1-service

CUSTOM_IMAGE_MODULES += init.oplus.fingerprints.rc
CUSTOM_IMAGE_MODULES += init.oplus.fingerprints.sh
