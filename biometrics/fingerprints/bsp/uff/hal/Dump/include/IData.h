/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/include/IData.h
 **
 ** Description:
 **      Interface for Dump Data Info
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          IData.h
 ************************************************************************************/
#ifndef _IDATA_H_
#define _IDATA_H_

#include "FpType.h"
namespace android {

class IData {
    public:
        virtual ~IData() {}
    /* interface for Dump Data */
    virtual void setParameters(void *buf, int len)                      = 0;
    virtual void getParameters(void *buf, int len)                      = 0;
    virtual fp_return_type_t saveDataTaToHal(void)                      = 0;
    virtual fp_return_type_t sendDataHalToTa(void)                      = 0;
};
}

#endif // _IDATA_H_
