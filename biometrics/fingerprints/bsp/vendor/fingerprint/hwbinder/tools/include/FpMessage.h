/**************************************************************************************
 ** File: - FpMessage.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **
 ** Version: V1.0
 ** Date : 11:03:26, 2019-08-29
 ** Author: Ziqing.Guo@BSP.Fingerprint.Basic, Add for fingerprint helathmonitor module
 **
 ** --------------------------- Revision History: --------------------------------
 **  <author>          <data>           <desc>
 **  Ziqing.Guo       2019/08/29       create file
 ************************************************************************************/
#ifndef FP_MESSAGE_H_
#define FP_MESSAGE_H_

#include <string>
#include <inttypes.h>
#include <log/log.h>

namespace android {

class FpMessage {
public:
    ~FpMessage();
    FpMessage();
    FpMessage(int what);
    FpMessage(int what, int arg0);
    FpMessage(int what, int arg0, int arg1);
    FpMessage(int what, float arg2);
    FpMessage(int what, std::string str0);

    int what;
    int arg0;
    int arg1;
    int arg2;
    int64_t when;
    std::string str0;
};

} // namespace android

#endif // FP_MESSAGE_H_
