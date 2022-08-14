/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/include/CreateData.h
 **
 ** Description:
 **      CreateData.h
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          CreateData.h
 ************************************************************************************/
#ifndef _CREATE_DATA_H_
#define _CREATE_DATA_H_

#include <string>
#include "IData.h"

namespace android {
class CreateData {
    public:
    static IData *createDataInfo(const std::string& dataType);
};
}  // namespace android
#endif  // _CREATE_DATA_H_
