/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/Message.cpp
 **
 ** Description:
 **      Message for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History---------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#include "Message.h"

namespace android {
FpMessage::FpMessage() {}

FpMessage::~FpMessage() {}

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

FpMessage::FpMessage(int what, std::string str0) {
    this->what = what;
    this->str0 = str0;
}

};  // namespace android
