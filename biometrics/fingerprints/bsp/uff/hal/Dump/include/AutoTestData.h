/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/include/AutoTestData.h
 **
 ** Description:
 **      AutoTestData.h
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          AutoTestData.h
 ************************************************************************************/
#ifndef _AUTO_TEST_DATA_H_
#define _AUTO_TEST_DATA_H_

#include "IData.h"
#include "FpType.h"
#include <string>
using namespace std;

namespace android {

class AutoTestData : public IData {
    public:
        AutoTestData();
        virtual ~AutoTestData();
        virtual void setParameters(void *buf, int len);
        virtual void getParameters(void *buf, int len);
        virtual fp_return_type_t saveDataTaToHal(void);
        virtual fp_return_type_t sendDataHalToTa(void);
};
}
#endif //_AUTO_TEST_DATA_H_
