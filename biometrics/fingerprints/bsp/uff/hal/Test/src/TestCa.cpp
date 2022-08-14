/*******************************************************************************************
 * Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
 *
 * File: test_tee_wrap.cpp
 * Description: test wrap
 * Version: 1.0
 * Date : 2021-4-2
 * Author: wangzhi(Kevin) wangzhi12
 ** -----------------------------Revision History: -----------------------
 **  <author>      <date>            <desc>
 **  Zhi.Wang   2021/03/26        create the file
 *********************************************************************************************/
#include <gtest/gtest.h>
#include "FpCa.h"

#define TEST_CASE_NAME test_ca

using namespace android;

TEST(TEST_CASE_NAME, testRee) {
    FpCa ca;
    int ret = 0;

    ret = ca.setTeeType("ree");
    EXPECT_EQ(ret, 0);

    ret = ca.startTa();
    EXPECT_EQ(ret, 0);

    ret = ca.closeTa();
    EXPECT_EQ(ret, 0);
}
