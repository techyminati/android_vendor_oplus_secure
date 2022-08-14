/*********************************************************************************************
* Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
*
* File: FpCa.h
* Description: interface for Ca
* Version: 1.0
* Date : 2021-5-12
* Author: wangzhi(Kevin) wangzhi12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/05/12        create the file
*********************************************************************************************/
#ifndef FPCA_H_
#define FPCA_H_

#include <string>
#include "FpCommon.h"
#include "ICa.h"

namespace android {

class FpCa {
   public:
    FpCa(const std::string& teeType);
    FpCa();
    ~FpCa();

   public:
    fp_return_type_t setTeeType(const std::string& teeType);

   public:
    /* common interface for CA */
    fp_return_type_t startTa(fp_ta_name_t name);
    fp_return_type_t closeTa(void);
    fp_return_type_t sendCommand(void* cmd, uint32_t size);
    fp_return_type_t sendHmacKeyToFpta();

   private:
    ICa* mICa;
};
}
#endif  //FPCA_H_
