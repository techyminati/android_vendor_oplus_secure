/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/Message.h
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _FP_MESSAGE_H_
#define _FP_MESSAGE_H_

#include <inttypes.h>
#include <string>

namespace android {
class FpMessage {
   public:
    FpMessage();
    ~FpMessage();
    FpMessage(int what);
    FpMessage(int what, int arg0);
    FpMessage(int what, int arg0, int arg1);
    FpMessage(int what, std::string str0);

    int         what;
    int         arg0;
    int         arg1;
    int         arg2;
    uint64_t    arg3;
    int64_t     when;
    std::string str0;
    void*       str1;
};

}  // namespace android

#endif  // FP_MESSAGE_H_
