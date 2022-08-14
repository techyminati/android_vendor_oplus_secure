/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/include/AppData.h
 **
 ** Description:
 **      AppData.h
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          AppData.h
 ************************************************************************************/
#ifndef _APP_DATA_H_
#define _APP_DATA_H_

#include "IData.h"
#include "FpType.h"
#include <string>
using namespace std;

namespace android {

class AppData : public IData {
    public:
        AppData();
        virtual ~AppData();
        virtual void setParameters(void *buf, int len);
        virtual void getParameters(void *buf, int len);
        virtual fp_return_type_t saveDataTaToHal(void);
        virtual fp_return_type_t sendDataHalToTa(void);
    private:
        fp_user_data_t *mAppDumpPara;
};
}
#endif //_APP_DATA_H_
