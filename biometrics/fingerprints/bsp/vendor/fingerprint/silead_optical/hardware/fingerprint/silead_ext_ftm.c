/******************************************************************************
 * @file   silead_ext_ftm.c
 * @brief  Contains fingerprint extension operate functions.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * calvin Wang  2018/10/30    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_ext_ftm"
#include "log/logmsg.h"

#include <errno.h>

#include "silead_const.h"
#include "silead_ext.h"
#include "silead_common.h"
#include "silead_impl.h"
#include "silead_ext_ftm.h"

static uint32_t m_ftm_test_item = 0;

typedef struct _ext_ftm_test {
    uint32_t cmd;
    int32_t (*dispatch) ();
    char *desc;
} ext_ftm_test_t;

static int32_t _ext_ftm_spi_test();
static int32_t _ext_ftm_reset_pin_test();
static int32_t _ext_ftm_deadpix_test(void);
static int32_t _ext_ftm_version_test(void);
static int32_t _ext_ftm_flash_test(void);
static ext_ftm_test_t ext_ftm_cmd[] = {
    {EXT_FTM_TEST_SPI,          _ext_ftm_spi_test,          "SPI_TEST"},
    {EXT_FTM_TEST_RESET,        _ext_ftm_reset_pin_test,    "RESET_TEST"},
    {EXT_FTM_TEST_DEAD_PIXEL,   _ext_ftm_deadpix_test,      "DEADPIXEL_TEST"},
    {EXT_FTM_TEST_VERSION,      _ext_ftm_version_test,      "VERSION_TEST"},
    {EXT_FTM_TEST_FLASH,        _ext_ftm_flash_test,        "FLASH_TEST"},
};

static int32_t _ext_ftm_spi_test()
{
    int32_t ret = 0;
    uint32_t chipid = 0, subid = 0;

    ret = silfp_ext_test_spi(&chipid, &subid);
    if (ret >= 0) {
        LOG_MSG_ERROR("silead main chip id = %#08x, sub chip id = %#08x", chipid, subid);
    } else {
        LOG_MSG_ERROR("silead spi test error");
    }
    return ret;
}

static int32_t _ext_ftm_reset_pin_test()
{
    int32_t ret = 0;

    ret = silfp_impl_download_normal();
    if (ret >= 0) {
        LOG_MSG_ERROR("silead reset pin test success");
    } else {
        LOG_MSG_ERROR("silead reset pin test error");
    }
    return ret;
}

static int32_t _ext_ftm_deadpix_test(void)
{
    int32_t ret = 0;
    uint32_t result = 0;
    uint32_t deadpixelnum = 0;
    uint32_t badlinenum = 0;

    int32_t optic = silfp_impl_is_optic();

    ret = silfp_ext_test_deadpx(optic, &result, &deadpixelnum, &badlinenum);
    if (ret >= 0) {
        if (!result) {
            LOG_MSG_ERROR("silead deadpixel test success");
        } else {
            LOG_MSG_ERROR("silead deadpixel test fail, deadpixelnum = %d, badlinenum = %d", deadpixelnum, badlinenum);
            ret = -1;
        }
    } else {
        LOG_MSG_ERROR("silead deadpixel test error");
    }
    return ret;
}

static int32_t _ext_ftm_version_test(void)
{
    int32_t ret = 0;
    uint32_t algoVer = 0;
    uint32_t taVer = 0;

    ret = silfp_ext_test_get_ta_ver(&algoVer, &taVer);
    if (ret >= 0) {
        LOG_MSG_ERROR("silead ver test success, algoVer = %d, taVer = %d", algoVer, taVer);
    } else {
        LOG_MSG_ERROR("silead ver test error");
    }
    return ret;
}

static int32_t _ext_ftm_flash_test(void)
{
    int32_t ret = 0;

    int32_t optic = silfp_impl_is_optic();
    if (optic) {
        ret = silfp_ext_test_flash();
        if (ret >= 0) {
            LOG_MSG_ERROR("silead flash test success");
        } else {
            LOG_MSG_ERROR("silead flash test error");
        }
    }

    return ret;
}

int32_t silfp_ext_ftm_test_entry(uint32_t item, uint32_t *result)
{
    int32_t ret = 0;
    uint32_t i = 0;
    uint32_t mask = 0;
    uint32_t item_support = 0;

    m_ftm_test_item = item;
    if (m_ftm_test_item == 0) {
        m_ftm_test_item = 0xFFFFFFFF;
    }

    for (i = 0; i < NUM_ELEMS(ext_ftm_cmd); i++) {
        item_support |= (1 << ext_ftm_cmd[i].cmd);
    }

    m_ftm_test_item &= item_support;

    for (i = 0; i < NUM_ELEMS(ext_ftm_cmd); i++) {
        mask = (1 << ext_ftm_cmd[i].cmd);
        if (mask & m_ftm_test_item) {
            ret = ext_ftm_cmd[i].dispatch();
            if (ret >= 0) {
                m_ftm_test_item &= (~mask);
            }
        }
    }

    if (m_ftm_test_item != 0) {
        LOG_MSG_ERROR("silead fac test isn't successful completefully");
        for (i = 0; i < NUM_ELEMS(ext_ftm_cmd); i++) {
            if ((1 << ext_ftm_cmd[i].cmd) & m_ftm_test_item) {
                LOG_MSG_ERROR("the fail item is: %s", ext_ftm_cmd[i].desc);
            }
        }
        ret = -EINVAL;
    } else {
        LOG_MSG_ERROR("silead fac test is successful");
    }

    if (result != NULL) {
        *result = m_ftm_test_item;
    }

    return ret;
}

int32_t silfp_ext_ftm_init(void)
{
    return silfp_common_init();
}

void silfp_ext_ftm_deinit(void)
{
    silfp_common_deinit();
}