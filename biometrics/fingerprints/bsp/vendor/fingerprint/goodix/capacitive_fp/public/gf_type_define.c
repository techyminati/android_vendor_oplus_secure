/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description:
 * History:
 */
#include"gf_type_define.h"

/*
 * use for print navmode
 * navmode:
 * strnavmode:
 */
typedef struct
{
    gf_nav_mode_t navmode;
    const char *strnavmode;
} gf_strnavmode_t;

/*
 * use for print chip
 * chip :
 * strchip:
 */
typedef struct
{
    gf_chip_type_t chip;
    const char *strchip;
} gf_strchiptype_t;

/*
 * use for print str
 * testcmdid:
 * strtestcmd:
 */
typedef struct
{
    gf_cmd_test_id_t testcmdid;
    const char *strtestcmd;
} gf_strtestcmd_t;

/*
 * string nav mode definition
 * print the nav mod defintion
 */
gf_strnavmode_t navmode_table[] =
{
    { GF_NAV_MODE_NONE, "GF_NAV_MODE_NONE" },
    { GF_NAV_MODE_X, "GF_NAV_MODE_X" },
    { GF_NAV_MODE_Y, "GF_NAV_MODE_Y" },
    { GF_NAV_MODE_Z, "GF_NAV_MODE_Z" },
    { GF_NAV_MODE_XY, "GF_NAV_MODE_XY" },
    { GF_NAV_MODE_XZ, "GF_NAV_MODE_XZ" },
    { GF_NAV_MODE_YZ, "GF_NAV_MODE_YZ" },
    { GF_NAV_MODE_XYZ, "GF_NAV_MODE_XYZ" },
    { GF_NAV_MODE_MAX, "GF_NAV_MODE_MAX" }
};

/*
 * string chip type definition
 * print the chip type definition
 */
gf_strchiptype_t chip_table[] =
{
    { GF_CHIP_316M, "GF_CHIP_316M" },
    { GF_CHIP_318M, "GF_CHIP_318M" },
    { GF_CHIP_3118M, "GF_CHIP_3118M" },
    { GF_CHIP_516M, "GF_CHIP_516M" },
    { GF_CHIP_518M, "GF_CHIP_518M" },
    { GF_CHIP_5118M, "GF_CHIP_5118M" },
    { GF_CHIP_816M, "GF_CHIP_816M" },
    { GF_CHIP_3266, "GF_CHIP_3266" },
    { GF_CHIP_3208, "GF_CHIP_3208" },
    { GF_CHIP_3268, "GF_CHIP_3268" },
    { GF_CHIP_3228, "GF_CHIP_3228" },
    { GF_CHIP_3288, "GF_CHIP_3288" },
    { GF_CHIP_3206, "GF_CHIP_3206" },
    { GF_CHIP_3226, "GF_CHIP_3226" },
    { GF_CHIP_3258, "GF_CHIP_3258" },
    { GF_CHIP_3258DN2, "GF_CHIP_3258DN2" },
    { GF_CHIP_3216, "GF_CHIP_3216" },
    { GF_CHIP_5206, "GF_CHIP_5206" },
    { GF_CHIP_5216, "GF_CHIP_5216" },
    { GF_CHIP_5208, "GF_CHIP_5208" },
    { GF_CHIP_5218, "GF_CHIP_5218" },
    { GF_CHIP_8206, "GF_CHIP_8206" },
    { GF_CHIP_5266, "GF_CHIP_5266" },
    { GF_CHIP_5288, "GF_CHIP_5288" },
    { GF_CHIP_5288_CER, "GF_CHIP_5288_CER" },
    { GF_CHIP_5296, "GF_CHIP_5296" },
    { GF_CHIP_5296_CER, "GF_CHIP_5296_CER" },
    { GF_CHIP_5228, "GF_CHIP_5228" },
    { GF_CHIP_5298, "GF_CHIP_5298" },
    { GF_CHIP_5628DN3, "GF_CHIP_5628DN3" },
    { GF_CHIP_6226, "GF_CHIP_6226" },
    { GF_CHIP_GX556, "GF_CHIP_GX556" },
    { GF_CHIP_5236, "GF_CHIP_5236" },
    { GF_CHIP_SIMULATOR_3266, "GF_CHIP_SIMULATOR_3266" },
    { GF_CHIP_5269, "GF_CHIP_5269" },
    { GF_CHIP_3216, "GF_CHIP_3216" },
    { GF_CHIP_5258, "GF_CHIP_5258" },
    { GF_CHIP_3658DN1, "GF_CHIP_3658DN1" },
    { GF_CHIP_3658DN2, "GF_CHIP_3658DN2" },
    { GF_CHIP_3658DN3, "GF_CHIP_3658DN3" },
    { GF_CHIP_3658DN3, "GF_CHIP_3658DN3" },
    { GF_CHIP_3626ZS1, "GF_CHIP_3626ZS1" },
    { GF_CHIP_3636ZS1, "GF_CHIP_3636ZS1" },
    { GF_CHIP_3988, "GF_CHIP_3988" },
    { GF_CHIP_3956, "GF_CHIP_3956" },
    { GF_CHIP_5658ZN2, "GF_CHIP_5658ZN2" },
    { GF_CHIP_3668DN1, "GF_CHIP_3668DN1" },
    { GF_CHIP_5628DN2, "GF_CHIP_5628DN2" },
    { GF_CHIP_UNKNOWN, "GF_CHIP_UNKNOWN" }
};

/*
 * string test command definition
 * should align right
*/
gf_strtestcmd_t testcmd_table[] =
{
    { CMD_TEST_ENUMERATE, "CMD_TEST_ENUMERATE" },
    { CMD_TEST_DRIVER, "CMD_TEST_DRIVER" },
    { CMD_TEST_PIXEL_OPEN, "CMD_TEST_PIXEL_OPEN" },
    { CMD_TEST_BAD_POINT, "CMD_TEST_BAD_POINT" },
    { CMD_TEST_SENSOR_FINE, "CMD_TEST_SENSOR_FINE" },
    { CMD_TEST_PERFORMANCE, "CMD_TEST_PERFORMANCE" },
    { CMD_TEST_SPI_PERFORMANCE, "CMD_TEST_SPI_PERFORMANCE" },
    { CMD_TEST_SPI_TRANSFER, "CMD_TEST_SPI_TRANSFER" },
    { CMD_TEST_SPI, "CMD_TEST_SPI" },
    { CMD_TEST_GET_VERSION, "CMD_TEST_GET_VERSION" },
    { CMD_TEST_FRR_FAR_GET_CHIP_TYPE, "CMD_TEST_FRR_FAR_GET_CHIP_TYPE" },
    { CMD_TEST_FRR_FAR_INIT, "CMD_TEST_FRR_FAR_INIT" },
    { CMD_TEST_FRR_FAR_RECORD_CALIBRATION, "CMD_TEST_FRR_FAR_RECORD_CALIBRATION" },
    { CMD_TEST_FRR_FAR_RECORD_ENROLL, "CMD_TEST_FRR_FAR_RECORD_ENROLL" },
    { CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE, "CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE" },
    { CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH, "CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH" },
    { CMD_TEST_FRR_FAR_PLAY_CALIBRATION, "CMD_TEST_FRR_FAR_PLAY_CALIBRATION" },
    { CMD_TEST_FRR_FAR_PLAY_ENROLL, "CMD_TEST_FRR_FAR_PLAY_ENROLL" },
    { CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE, "CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE" },
    { CMD_TEST_FRR_FAR_ENROLL_FINISH, "CMD_TEST_FRR_FAR_ENROLL_FINISH" },
    { CMD_TEST_FRR_FAR_SAVE_FINGER, "CMD_TEST_FRR_FAR_SAVE_FINGER"},
    { CMD_TEST_FRR_FAR_DEL_FINGER, "CMD_TEST_FRR_FAR_DEL_FINGER"},
    { CMD_TEST_CANCEL_FRR_FAR, "CMD_TEST_CANCEL_FRR_FAR" },
    { CMD_TEST_RESET_PIN, "CMD_TEST_RESET_PIN" },
    { CMD_TEST_INTERRUPT_PIN, "CMD_TEST_INTERRUPT_PIN" },
    { CMD_TEST_CANCEL, "CMD_TEST_CANCEL" },
    { CMD_TEST_GET_CONFIG, "CMD_TEST_GET_CONFIG" },
    { CMD_TEST_SET_CONFIG, "CMD_TEST_SET_CONFIG" },
    { CMD_TEST_DOWNLOAD_FW, "CMD_TEST_DOWNLOAD_FW" },
    { CMD_TEST_DOWNLOAD_CFG, "CMD_TEST_DOWNLOAD_CFG" },
    { CMD_TEST_DOWNLOAD_FWCFG, "CMD_TEST_DOWNLOAD_FWCFG" },
    { CMD_TEST_RESET_FWCFG, "CMD_TEST_RESET_FWCFG" },
    { CMD_TEST_SENSOR_VALIDITY, "CMD_TEST_SENSOR_VALIDITY" },
    { CMD_TEST_RESET_CHIP, "CMD_TEST_RESET_CHIP" },
    { CMD_TEST_UNTRUSTED_ENROLL, "CMD_TEST_UNTRUSTED_ENROLL" },
    { CMD_TEST_UNTRUSTED_AUTHENTICATE, "CMD_TEST_UNTRUSTED_AUTHENTICATE" },
    { CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER, "CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER" },
    { CMD_TEST_CHECK_FINGER_EVENT, "CMD_TEST_CHECK_FINGER_EVENT" },
    { CMD_TEST_BIO_CALIBRATION, "CMD_TEST_BIO_CALIBRATION" },
    { CMD_TEST_HBD_CALIBRATION, "CMD_TEST_HBD_CALIBRATION" },
    { CMD_TEST_SPI_RW, "CMD_TEST_SPI_RW" },
    { CMD_TEST_REAL_TIME_DATA, "CMD_TEST_REAL_TIME_DATA" },
    { CMD_TEST_READ_CFG, "CMD_TEST_READ_CFG" },
    { CMD_TEST_READ_FW, "CMD_TEST_READ_FW" },
    { CMD_TEST_FRR_DATABASE_ACCESS, "CMD_TEST_FRR_DATABASE_ACCESS" },
    { CMD_TEST_PRIOR_CANCEL, "CMD_TEST_PRIOR_CANCEL" },
    { CMD_TEST_NOISE, "CMD_TEST_NOISE" },
    { CMD_TEST_RAWDATA_SATURATED, "CMD_TEST_RAWDATA_SATURATED" },
    { CMD_TEST_BMP_DATA, "CMD_TEST_BMP_DATA" },
    { CMD_TEST_MEMMGR_SET_CONFIG, "CMD_TEST_MEMMGR_SET_CONFIG" },
    { CMD_TEST_MEMMGR_GET_CONFIG, "CMD_TEST_MEMMGR_GET_CONFIG" },
    { CMD_TEST_MEMMGR_GET_INFO, "CMD_TEST_MEMMGR_GET_INFO" },
    { CMD_TEST_MEMMGR_DUMP_POOL, "CMD_TEST_MEMMGR_DUMP_POOL" },
    { CMD_TEST_UNTRUSTED_PAUSE_ENROLL, "CMD_TEST_UNTRUSTED_PAUSE_ENROLL" },
    { CMD_TEST_UNTRUSTED_RESUME_ENROLL, "CMD_TEST_UNTRUSTED_RESUME_ENROLL" },
    { CMD_TEST_FPC_KEY, "CMD_TEST_FPC_KEY" },
    { CMD_TEST_FPC_KEY_DOWNLOAD_CFG, "CMD_TEST_FPC_KEY_DOWNLOAD_CFG" },
    { CMD_TEST_FPC_KEY_RESET_FWCFG, "CMD_TEST_FPC_KEY_RESET_FWCFG" },
    { CMD_TEST_CALIBRATION_PARA_RETEST, "CMD_TEST_CALIBRATION_PARA_RETEST" },
    { CMD_TEST_FRR_FAR_PREPROCESS_INIT, "CMD_TEST_FRR_FAR_PREPROCESS_INIT" },
    { CMD_TEST_AUTH_RATIO, "CMD_TEST_AUTH_RATIO" },
    { CMD_TEST_UNKNOWN, "CMD_TEST_UNKNOWN" }
};

/**
 * Function: gf_strnavmode
 * Description: get nav mode string
 * Input: nav mode
 * Output: none
 * Return: nav mode string
 */
const char *gf_strnavmode(gf_nav_mode_t navmode)
{
    // nav_mode enum is not consequent
    uint32_t idx = 0;
    uint32_t len = sizeof(navmode_table) / sizeof(gf_strnavmode_t);

    do
    {
        // if navmde > GF_NAV_MODE_MAX means error
        if (navmode > GF_NAV_MODE_MAX)
        {
            idx = len - 1;
            break;
        }

        while (idx < len)
        {
            if (navmode == navmode_table[idx].navmode)
            {
                break;
            }

            idx++;
        }

        if (idx == len)
        {
            idx = len - 1;
        }
    }
    while (0);

    // idx is wrong,print error
    return navmode_table[idx].strnavmode;
}

/**
 * Function: gf_strchiptype
 * Description: get chip type string
 * Input: chip type
 * Output: none
 * Return: chip type string
 */
const char *gf_strchiptype(gf_chip_type_t chip)
{
    uint32_t idx = 0;
    uint32_t len = sizeof(chip_table) / sizeof(gf_strchiptype_t);

    do
    {
        // if chip > GF_CHIP_UNKNOWN means error
        if ((chip > GF_CHIP_UNKNOWN) || (chip - GF_CHIP_316M >= len))
        {
            idx = len - 1;
            break;
        }

        if (chip == chip_table[chip - GF_CHIP_316M].chip)
        {
            idx = chip - GF_CHIP_316M;
            break;
        }

        while (idx < len)
        {
            if (chip == chip_table[idx].chip)
            {
                break;
            }

            idx++;
        }

        if (idx == len)
        {
            idx = len - 1;
        }
    }  // do...
    while (0);

    // idx is wrong,print error
    return chip_table[idx].strchip;
}

/**
 * Function: gf_strtestcmd
 * Description: get test cmd string
 * Input: test cmd id
 * Output: none
 * Return: test cmd string
 */
const char *gf_strtestcmd(gf_cmd_test_id_t test_cmd_id)
{
    uint32_t idx = 0;
    uint32_t max = sizeof(testcmd_table) / sizeof(testcmd_table[0]) - 1;

    do
    {
        // test_cmd_id > CMD_TEST_UNKNOWN means error
        if (test_cmd_id > CMD_TEST_UNKNOWN || test_cmd_id < CMD_TEST_ENUMERATE)
        {
            idx = CMD_TEST_UNKNOWN;
        }
        else
        {
            idx = test_cmd_id;
        }
    }
    while (0);

    // idx is wrong,print error
    if (idx > max)
    {
        idx = max;
    }

    return testcmd_table[idx].strtestcmd;
}
