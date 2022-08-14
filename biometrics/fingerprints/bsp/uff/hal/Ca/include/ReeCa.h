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
#ifndef REECA_H_
#define REECA_H_

#include "ICa.h"
#include "FpType.h"
namespace android {

class ReeCa : public ICa {
   public:
    ReeCa();
    virtual ~ReeCa();
    /* interface for CA */
    virtual fp_return_type_t startTa(fp_ta_name_t name);
    virtual fp_return_type_t closeTa(void);
    virtual fp_return_type_t sendCommand(void* cmd, uint32_t size);
    virtual fp_return_type_t sendHmacKeyToFpta();
    void*      mContext;
};
}  // namespace android
#endif  // REECA_H_
