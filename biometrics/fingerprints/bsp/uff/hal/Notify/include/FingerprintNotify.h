/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/FingerprintNotify.h
 **
 ** Description:
 **      FingerprintNotify for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,30/09/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/09/30        create the file
 ************************************************************************************/
#ifndef _FINGERPRINTNOTIFY_H_
#define _FINGERPRINTNOTIFY_H_

#include <stddef.h>
#include <stdint.h>
#include "fingerprint.h"
#include "FpType.h"

namespace android {
class FingerprintNotify {
   public:
    explicit FingerprintNotify();
    virtual ~FingerprintNotify();
    virtual fp_return_type_t setNotify(fingerprint_notify_t mNotify);

    //nofity
    fp_return_type_t notifyAcquiredInfo(fingerprint_acquired_info_t info);
    fp_return_type_t notifyEnrollResult(fp_enroll_image_result_t* remainCount);
    fp_return_type_t notifyAuthResult(fp_auth_compare_result* compare_result);
    fp_return_type_t notifyEnumerate(fp_enumerate_t cmd, uint32_t current_gid);
    fp_return_type_t notifyRemove(uint32_t gid, uint32_t fid, uint32_t remainCount);
    fp_return_type_t notifyTouch(fingerprint_touch_state_type_t touch_state);
    fp_return_type_t notifyError(fingerprint_error_t error);
    fp_return_type_t notifyFingerprintCmd(
        int64_t devId, int32_t cmdId, const int8_t* result, int32_t resultLen);
    fp_return_type_t checkAcquiredInfo(fp_common_result_t result_info);
    fingerprint_notify_t   mNotify;
};
}  // namespace android

#endif /* _FINGERPRINTNOTIFY_H_ */
