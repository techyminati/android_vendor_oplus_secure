/**************************************************************************************
 ** File: - dcs.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2019-08-29
 ** Author: Ziqing.Guo@BSP.Fingerprint.Basic, Add for fingerprint performance module
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ziqing.Guo      2019/08/29       create file
 **  Ziqing.Guo      2019/08/29       add the dcs api
 ************************************************************************************/

#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <hardware/hardware.h>
#include <string>
#include <log/log.h>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/String16.h>

#include "fingerprint.h"
#include "dcs_type.h"

#ifndef DCS_H_
#define DCS_H_

#define FP_AUTH_DCSMSG_LEN (19)

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_array;
using ::android::sp;
using android::String16;

namespace android {
/* SenseTimeThreshold. */
class Dcs : public virtual RefBase {
public:
    static Dcs* getInstance() {
        if (sInstance == NULL) {
            sInstance = new Dcs();
        }
        return sInstance;
    }

    hidl_string getHidlstring(uint32_t param);
    void reportMonitor(uint32_t param);
    void reportAuthenticatedInfo(fingerprint_auth_dcsmsg_t authenticated_msg);
    void reportInitEventInfo(oplus_fingerprint_init_event_info_t init_event_info);
    void reportAuthEventInfo(oplus_fingerprint_auth_event_info_t auth_event_info);
    void reportSingleEnrollEventInfo(oplus_fingerprint_singleenroll_event_info_t singleenroll_event_info);
    void reportEnrollEventInfo(oplus_fingerprint_enroll_event_info_t enroll_event_info);
    void reportSpecialEventInfo(oplus_fingerprint_special_event_info_t special_event_info);
    void reportDcsEventInfo(oplus_fingerprint_dcs_info_t dcs_info);

private:
    Dcs();
    virtual ~Dcs();
    static Dcs* sInstance;
};

} // namespace android

#endif // DCS_H_
