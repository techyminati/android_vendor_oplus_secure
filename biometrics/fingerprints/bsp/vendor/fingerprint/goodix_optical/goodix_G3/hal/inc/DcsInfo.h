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
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ran. Chen      2020/09/29       create file
 ************************************************************************************/
#ifndef _DCSINFO_H_
#define _DCSINFO_H_

#include "dcs_type.h"
#include "HalBase.h"
#include "FingerprintCore.h"
#include "MsgBus.h"

typedef struct {
    oplus_fingerprint_dcs_event_type_t dcs_type;
    union {
        oplus_fingerprint_init_ta_info_t init_ta_info;
        oplus_fingerprint_auth_ta_info_t auth_ta_info;
        oplus_fingerprint_singleenroll_ta_info_t singleenroll_ta_info;
        oplus_fingerprint_enroll_ta_info_t enroll_ta_info;
    } data;
} oplus_fingerprint_dcs_ta_cmd_info_t;

typedef struct
{
    gf_cmd_header_t header;
    oplus_fingerprint_dcs_ta_cmd_info_t dcs_ta_cmd_info;
} oplus_dcs_event_ta_cmd_t;

typedef struct
{
//base info
    int32_t continuous_authsuccess_count;
    int32_t continuous_authfail_count;
    int32_t continuous_badauth_count;
    char lcd_type[4];
    int32_t special_event_count;//
    bool need_notify_init_info = false;//
} oplus_fingerprint_dcs_static_info_t;

namespace goodix
{

class DcsInfo : public HalBase
{
    public:
        explicit DcsInfo(HalContext* context);
        ~DcsInfo();
        int init();

    public:
        int printInitEventInfo(oplus_fingerprint_init_event_info_t* int_event_info);
        int printAuthEventInfo(oplus_fingerprint_auth_event_info_t* auth_event_info);
        int printSingleEnrollEventInfo(oplus_fingerprint_singleenroll_event_info_t* singleenroll_event_info);
        int printEnrollEventInfo(oplus_fingerprint_enroll_event_info_t* enroll_event_info);
        int printSpecialEventInfo(oplus_fingerprint_special_event_info_t* special_event_info);

        int getDcsEventTime(int32_t* event_times);
        int getDcsBrightnessValue(uint32_t* brightness_value);
        int getDcsLcdType(HalContext* context);
        int getDcsHalVersion(int32_t* hal_version);
        int getDcsDriverVersion(int32_t* driver_version);
        int getDcsCdspVersion(int32_t* cdsp_version);
        int getDcsPidInfo(int32_t* pid_info);

        int sendDcsInitEventInfo(HalContext* context);
        int sendDcsAuthEventInfo(HalContext* context);
        int sendDcsSingleEnrollEventInfo(HalContext* context);
        int sendDcsEnrollEventInfo(HalContext* context);
        int sendDcsSpecialEventInfo(HalContext* context);

        oplus_fingerprint_init_event_info_t init_event_info;
        oplus_fingerprint_auth_event_info_t auth_event_info;
        oplus_fingerprint_singleenroll_event_info_t singleenroll_event_info;
        oplus_fingerprint_enroll_event_info_t enroll_event_info;
        oplus_fingerprint_special_event_info_t special_event_info;
        oplus_fingerprint_dcs_static_info_t dcs_static_info;
};
}  // namespace goodix

#endif /* _SENSOR_H_ */
