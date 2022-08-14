/**************************************************************************************
 ** File: -DcsInfo.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2020-08-29
 ** Author: Ran.Chen@BSP.Biometrics.Fingerprint,  Add for fingerprint dcs module
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>          <data>           <desc>
 **  Ran. Chen      2020/09/29       create file
 ************************************************************************************/
#ifndef _DCSINFO_H_
#define _DCSINFO_H_

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include <hardware/hardware.h>
#include <string>
#include <log/log.h>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/String16.h>

#include "DcsType.h"
#include "DcsReport.h"
#include "FpCommon.h"
#include "FpType.h"
#include "HalContext.h"

using android::hardware::hidl_vec;
using android::hardware::hidl_array;
using android::hardware::hidl_string;
using android::hardware::Return;
using android::sp;
using android::hardware::hidl_string;

namespace android {
class HalContext;
class DcsInfo {
   public:
    explicit DcsInfo(HalContext* context);
    ~DcsInfo();
    int init();

   public:
    int getDcsEventTime(int32_t* event_times);
    int getDcsBrightnessValue(uint32_t* brightness_value);
    int getDcsHalVersion(int32_t* hal_version);
    int getDcsDriverVersion(int32_t* driver_version);
    int getDcsCdspVersion(int32_t* cdsp_version);
    int getDcsPidInfo(int32_t* pid_info);
    int getDcsAlogVerison(char* algo_version);

    int sendDcsInitEventInfo(HalContext* context);
    int sendDcsAuthEventInfo(HalContext* context);
    int sendDcsSingleEnrollEventInfo(HalContext* context);
    int sendDcsEnrollEventInfo(HalContext* context);
    int sendDcsSpecialEventInfo(HalContext* context);

    dcs_init_event_info_t                 init_event_info;
    dcs_auth_event_info_t                 auth_event_info;
    dcs_singleenroll_event_info_t         singleenroll_event_info;
    dcs_enroll_event_info_t               enroll_event_info;
    dcs_special_event_info_t              special_event_info;
    dcs_static_info_t                     dcs_static_info;
};
}  // namespace android
#endif /* _DCSINFO_H_ */
