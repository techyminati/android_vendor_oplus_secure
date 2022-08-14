/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/include/CaliData.h
 **
 ** Description:
 **      CaliData.h
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          CaliData.h
 ************************************************************************************/
#ifndef _CALI_DATA_H_
#define _CALI_DATA_H_

#include "IData.h"
#include "FpType.h"
#include <string>
using namespace std;

namespace android {

class CaliData : public IData {
    public:
        CaliData();
        virtual ~CaliData();
        virtual void setParameters(void *buf, int len);
        virtual void getParameters(void *buf, int len);
        virtual fp_return_type_t saveDataTaToHal(void);
        virtual fp_return_type_t sendDataHalToTa(void);
};
}
#endif //_CALI_DATA_H_
