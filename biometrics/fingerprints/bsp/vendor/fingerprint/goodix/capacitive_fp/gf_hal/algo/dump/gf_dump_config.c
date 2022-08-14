/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description: dump config file, dump is configured at compile time
 * History:
 */
#include <stdlib.h>
#include <string.h>
#include "gf_dump_config.h"

// dump config
static const gf_dump_config_t g_dump_cfg =
{
    {"7"},              // dump_version[DUMP_VERSION_STR_MAX_LEN]
    {"dumpdata_1.1"},  // dump_bigdata_version[DUMP_VERSION_STR_MAX_LEN]
        1,                  // dump_enabled, 0 disable, 1 enable
        0,                  // dump_encrypt_enabled, 0 disable, 1 enable
        0,                  // dump_big_data_enabled, 0 disable, 1 enable
        DUMP_PATH_DATA,     // dump_path, DUMP_PATH_SDCARD, DUMP_PATH_DATA
    {  // dump_operation[DUMP_OP_NUM], dump fingerprint operation
        {
            DUMP_OP_ENROLL,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_AUTHENTICATE,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_FINGER_BASE,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_NAV,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_NAV_BASE,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_TEST_PERFORMANCE,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_TEST_PIXEL,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_TEST_SENSOR_FINE,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_TEST_BAD_POINT,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_DEVICE_INFO,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_TEMPLATE,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_NAV_ENHANCE_DATA,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_CALIBRATION_PARAM_RETEST,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
        {
            DUMP_OP_TEST_DATA_NOISE,
                OP_RESULT_ALL,
                DUMP_ALL_OP_DATA
        },
    }
};

/**
 *Function: gf_get_dump_op_by_irq_op
 *Description: get dump op by irq op
 *Input: irq_op, gf_operation_type_t irq operation
 *Output:none
 *Return: gf_dump_operation_type_t dump op
 */
gf_dump_operation_type_t gf_get_dump_op_by_irq_op(gf_operation_type_t irq_op)
{
    switch (irq_op)
    {
        case OPERATION_ENROLL:
        case OPERATION_TEST_UNTRUSTED_ENROLL:
        {
            return DUMP_OP_ENROLL;
        }

        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_AUTHENTICATE_SLEEP:
        case OPERATION_AUTHENTICATE_FF:
        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
        {
            return DUMP_OP_AUTHENTICATE;
        }

        case OPERATION_TEST_PERFORMANCE:
        {
            return DUMP_OP_TEST_PERFORMANCE;
        }

        case OPERATION_FINGER_BASE:
        {
            return DUMP_OP_FINGER_BASE;
        }

        case OPERATION_TEST_PIXEL_OPEN_STEP1:
        case OPERATION_TEST_PIXEL_OPEN_STEP2:
        case OPERATION_TEST_PIXEL_SHORT_STREAK:
        {
            return DUMP_OP_TEST_PIXEL;
        }

        case OPERATION_TEST_SENSOR_FINE_STEP1:
        case OPERATION_TEST_SENSOR_FINE_STEP2:
        {
            return DUMP_OP_TEST_SENSOR_FINE;
        }

        case OPERATION_NAV_BASE:
        {
            return DUMP_OP_NAV_BASE;
        }

        case OPERATION_NAV:
        {
            return DUMP_OP_NAV;
        }

        case OPERATION_TEST_BAD_POINT_RECODE_BASE:
        case OPERATION_TEST_BAD_POINT:
        {
            return DUMP_OP_TEST_BAD_POINT;
        }

        case OPERATION_TEST_CALIBRATION_PARA_RETEST_RECODE_BASE:
        case OPERATION_TEST_CALIBRATION_PARA_RETEST:
        {
            return DUMP_OP_CALIBRATION_PARAM_RETEST;
        }

        case OPERATION_TEST_DATA_NOISE_BASE:
        case OPERATION_TEST_DATA_NOISE:
        {
            return DUMP_OP_TEST_DATA_NOISE;
        }

        case OPERATION_TEST_BOOT_CALIBRATION:
        {
            return DUMP_OP_FINGER_BASE;
        }

        default:
        {
            return DUMP_OP_NOT_ALLOWED;
        }
    }   // switch...
}

/**
 *Function: gf_get_dump_config
 *Description: get a dump config copy, caller have to free the allocated memory.
 *Input: none
 *Output:none
 *Return: a gf_dump_config_t pointer to new allocated memory.
 */
gf_dump_config_t* gf_get_dump_config()
{
    gf_dump_config_t* dump_cfg = (gf_dump_config_t*)malloc(sizeof(gf_dump_config_t));
    if (NULL == dump_cfg)
    {
        return NULL;
    }
    memset(dump_cfg, 0, sizeof(gf_dump_config_t));
    memcpy(dump_cfg, &g_dump_cfg, sizeof(gf_dump_config_t));
    return dump_cfg;
}

