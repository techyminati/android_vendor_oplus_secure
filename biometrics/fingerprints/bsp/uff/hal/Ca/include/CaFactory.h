/*********************************************************************************************
* Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
*
* File: CaFactory.h
* Description: Factory for Ca
* Version: 1.0
* Date : 2021-5-12
* Author: wangzhi(Kevin) wangzhi12
** -----------------------------Revision History: -----------------------
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/05/12        create the file
*********************************************************************************************/
#ifndef CAFACTORY_H_
#define CAFACTORY_H_

#include <string>
#include "ICa.h"

namespace android {
class CaFactory {
    public:
    static ICa* createCa(const std::string& teeType);
};
}  // namespace android
#endif  // CAFACTORY_H_
