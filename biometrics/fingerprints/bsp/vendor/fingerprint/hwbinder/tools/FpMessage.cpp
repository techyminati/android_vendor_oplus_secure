/**************************************************************************************
 ** File: - FpMessage.cpp
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

#include "FpMessage.h"

namespace android {
FpMessage::FpMessage() {
}

FpMessage::FpMessage(int what) {
    this->what = what;
}

FpMessage::FpMessage(int what, int arg0) {
    this->what = what;
    this->arg0 = arg0;
}

FpMessage::FpMessage(int what, int arg0, int arg1) {
    this->what = what;
    this->arg0 = arg0;
    this->arg1 = arg1;
}

FpMessage::FpMessage(int what, float arg2) {
    this->what = what;
    this->arg2 = arg2;
}

FpMessage::FpMessage(int what, std::string str0) {
    this->what = what;
    this->str0 = str0;
}

FpMessage::~FpMessage() {
}

}; // namespace android
