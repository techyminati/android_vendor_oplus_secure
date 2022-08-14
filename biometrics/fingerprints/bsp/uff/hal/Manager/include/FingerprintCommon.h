
/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Manager/include/FingerprintCommon.h
 **
 ** Description:
 **      FingerprintCommon for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _FINGERPRINTCOMMON_H_
#define _FINGERPRINTCOMMON_H_

#include <stddef.h>
#include <stdint.h>
#include <semaphore.h>
#include "Handler.h"
#include "fingerprint.h"
#include "FpType.h"
#include "Utils.h"

namespace android {
class HalContext;
class FingerprintCommon : public Utils{
   public:
    explicit FingerprintCommon(HalContext* context);
    virtual ~FingerprintCommon();
    //printf
    void print_token(fp_hw_auth_token_t* token);

    //uiready
    fp_return_type_t waitUiready(uint32_t ms);
    fp_return_type_t postUiready();

    //perf
    fp_return_type_t perfSetAction(int orms_type, int orms_timeout);
    fp_return_type_t perfSetUxThread(int ux_enable);
    fp_return_type_t perfBindBigCore();

   private:
    sem_t mUireadySem;
    HalContext* mContext;
};
}  // namespace android

#endif /* _FINGERPRINTCOMMON_H_ */
