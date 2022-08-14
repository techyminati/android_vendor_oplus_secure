/**************************************************************************************
 ** File: -DcsReport.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2020-12-14
 ** Author: Ran.Chen@BSP.Biometrics.Fingerprint,  Add for fingerprint dcs report
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>          <data>           <desc>
 **  Ran. Chen      2020/12/14       create file
 ************************************************************************************/
#ifndef _DCSREPORT_H_
#define _DCSREPORT_H_

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
#include "FpCommon.h"
#include "FpType.h"
#include "vendor/oplus/hardware/commondcs/1.0/ICommonDcsHalService.h"
#include "vendor/oplus/hardware/commondcs/1.0/types.h"

#define TODCSINT(X, Y) \
    dataArray[dcs_key_number].key = Y;\
    dataArray[dcs_key_number].value = getHidlstring(X);\
    dcs_key_number++;

#define TODCSSTRING(X, Y) \
    dataArray[dcs_key_number].key = Y; \
    dataArray[dcs_key_number].value = hidl_string(X, strlen(X));\
    dcs_key_number++;


using android::hardware::hidl_vec;
using android::hardware::hidl_array;
using android::hardware::hidl_string;
using android::hardware::Return;
using android::sp;
using android::hardware::hidl_string;

namespace android {
class DcsReport {
public:
    static DcsReport* getInstance() {
        if (sInstance == NULL) {
            sInstance = new DcsReport();
        }
        return sInstance;
    }
    hidl_string getHidlstring(uint32_t param);
    void dcsprintf(vendor::oplus::hardware::commondcs::V1_0::StringPair *dataArray, int dcs_key_number);
    void reportInitEventInfo(dcs_init_event_info_t* init_event_info);
    void reportAuthEventInfo(dcs_auth_event_info_t* auth_event_info);
    void reportSingleEnrollEventInfo(dcs_singleenroll_event_info_t* singleenroll_event_info);
    void reportEnrollEventInfo(dcs_enroll_event_info_t* enroll_event_info);
    void reportSpecialEventInfo(dcs_special_event_info_t* special_event_info);
    int    sensorExposureTime;

private:
    DcsReport();
    virtual ~DcsReport();
    static DcsReport* sInstance;
};
}  // namespace android
#endif /* _DCSREPORT_H_ */
