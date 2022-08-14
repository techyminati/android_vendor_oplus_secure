/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Ca/include/TrustonicCa.h
 **
 ** Description:
 **      TrustonicCa for fingerprint
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
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include "FpType.h"
#include "HalContext.h"

#ifndef TRUSTONICCA_H_
#define TRUSTONICCA_H_

namespace android {
class TrustonicCa : public ICa {
public:
    TrustonicCa();
    virtual ~TrustonicCa();
    virtual fp_return_type_t startTa(fp_ta_name_t name);
    virtual fp_return_type_t closeTa();
    virtual fp_return_type_t sendCommand(void* cmd, uint32_t size);
    virtual fp_return_type_t sendHmacKeyToFpta();
    void* mContext;
};
}  // namespace android

#endif  // TRUSTONICCA_H_
