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
#ifndef _ANC_HAL_DSC_INFO_H_
#define _ANC_HAL_DSC_INFO_H_


#include "dcs_type.h"
#include "anc_hal_manager.h"

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
    //base info
        int32_t continuous_authsuccess_count;
        int32_t continuous_authfail_count;
        int32_t continuous_badauth_count;
        char lcd_type[4];
        int32_t special_event_count;//
        bool need_notify_init_info;//
    } oplus_fingerprint_dcs_static_info_t;



    //int printInitEventInfo(oplus_fingerprint_init_event_info_t* int_event_info);
    int32_t printAuthEventInfo(oplus_fingerprint_auth_event_info_t print_auth_event_info);
    //int printSingleEnrollEventInfo(oplus_fingerprint_singleenroll_event_info_t* singleenroll_event_info);
    //int printEnrollEventInfo(oplus_fingerprint_enroll_event_info_t* enroll_event_info);
    //int printSpecialEventInfo(oplus_fingerprint_special_event_info_t* special_event_in);
    int32_t getDcsEventTime(int32_t* event_times);
    int32_t getDcsBrightnessValue(uint32_t* brightness_value);
    int32_t getDcsLcdType();
    //int getDcsHalVersion(int32_t* hal_version);
    //int getDcsDriverVersion(int32_t* driver_version);
    //int getDcsCdspVersion(int32_t* cdsp_version);
    int32_t getDcsPidInfo(int32_t* pid_info);
    //int sendDcsInitEventInfo(HalContext* context);
    uint32_t sendDcsAuthEventInfo(struct AncFingerprintManager *p_manager, oplus_fingerprint_auth_ta_info_t *ta_info_t);
    //int sendDcsSingleEnrollEventInfo(HalContext* context);
    //int sendDcsEnrollEventInfo(HalContext* context);
    //int sendDcsSpecialEventInfo(HalContext* context);
    //oplus_fingerprint_init_event_info_t init_event_info;
    //oplus_fingerprint_auth_event_info_t auth_event_info;
    //oplus_fingerprint_singleenroll_event_info_t singleenroll_event_info;
    //oplus_fingerprint_enroll_event_info_t enroll_event_info;
    //oplus_fingerprint_special_event_info_t special_event_info;
    //oplus_fingerprint_dcs_static_info_t dcs_static_info;

#endif // _ANC_HAL_DSC_INFO_H_
