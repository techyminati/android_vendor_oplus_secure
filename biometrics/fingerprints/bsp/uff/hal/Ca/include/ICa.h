/*********************************************************************************************
* Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
*
* File: ICa.h
* Description: interface for Ca
* Version: 1.0
* Date : 2021-5-12
* Author: wangzhi(Kevin) wangzhi12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/05/12        create the file
*********************************************************************************************/
#ifndef ICA_H_
#define ICA_H_

#include "FpCommon.h"
#include "FpType.h"

namespace android {
class ICa {
public:
    virtual ~ICa() {}

public:
    /* interface for CA */
    virtual fp_return_type_t startTa(fp_ta_name_t name)            = 0;
    virtual fp_return_type_t closeTa(void)                         = 0;
    virtual fp_return_type_t sendCommand(void* cmd, uint32_t size) = 0;
    virtual fp_return_type_t sendHmacKeyToFpta(void)              = 0;
    void* mContext                                                 = nullptr;
};
}  // namespace android
#endif  // ICA_H_
