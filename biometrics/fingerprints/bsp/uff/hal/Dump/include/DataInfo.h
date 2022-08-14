/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/DataInfo.cpp
 **
 ** Description:
 **      DataInfo.h
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          DataInfo.h
 ************************************************************************************/
#ifndef _DATA_INFO_H_
#define _DATA_INFO_H_

#include <string>
#include "IData.h"
#include "FpCommon.h"
#include "FpType.h"

namespace android {

class DataInfo {
    public:
        DataInfo(const std::string &dataType);
        ~DataInfo();
        void setParameters(void *buf, int len);
        void getParameters(void *buf, int len);
        fp_return_type_t saveDataTaToHal();
        fp_return_type_t sendDataHalToTa();
        fp_return_type_t getDataFromTaUnlimit(char *buf,
            uint32_t len, int index, long long dataAddr);

    public:
        IData *mIData;
};
}
#endif //_DATA_INFO_H_

