/******************************************************************************
 * @file   silead_error.h
 * @brief  Contains CA/TA communication error codes.
 *
 *
 * Copyright (c) 2016-2019 Silead Inc.
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
 * calvin wang  2018/12/2    0.1.0      Init version
 *
 *****************************************************************************/

/* This file must be the same between CA and TA APP */

#ifndef __SILEAD_FINGERPRINT_ERROR_H__
#define __SILEAD_FINGERPRINT_ERROR_H__

typedef enum sl_error {
    SL_SUCCESS = 0,

    SL_ERROR_BASE = 1000,
    SL_ERROR_BAD_PARAMS = SL_ERROR_BASE,    /* 1000 ERR: param invalid */
    SL_ERROR_OUT_OF_MEMORY,                 /* 1001 ERR: alloc memory failed */
    SL_ERROR_DEV_IOCTL_FAILED,              /* 1002 ERR: device IOCTL failed */
    SL_ERROR_SPI_SET_PARAM_FAILED,          /* 1003 ERR: set spi param failed, not check the err currently */
    SL_ERROR_INVALID_CHALLENGE,             /* 1004 ERR: enroll challenge invalid, SPEC: for enroll() return value */
    SL_ERROR_UNTRUSTED_ENROLL,              /* 1005 ERR: enroll untrusted, challenge verify failed, SPEC: for enroll() return value */
    SL_ERROR_INVALID_HAT_VERSION,           /* 1006 ERR: enroll challenge hat version invalid, SPEC: for enroll() return value */
    SL_ERROR_CONFIG_INVALID,                /* 1007 ERR: get chip config failed, SPEC: will not load the hal lib, when get this error */
    SL_ERROR_DEV_OPEN_FAILED,               /* 1008 ERR: device(dev&spi) open failed, SPEC: will not load the hal lib, when get this error */
    SL_ERROR_TA_OPEN_FAILED,                /* 1009 ERR: open ta failed */
    SL_ERROR_TA_SEND_FAILED,                /* 1010 ERR: send command to ta failed */
    SL_ERROR_SHARED_ALLOC_FAILED,           /* 1011 ERR: shared memory alloc failed */
    SL_ERROR_SHARED_MAP_FAILED,             /* 1012 ERR: shared memory map failed in ta */
    SL_ERROR_TA_OP_FAILED,                  /* 1013 ERR: ta operate failed */
    SL_ERROR_OP_IMPL_INVALID,               /* 1014 ERR: some interface that not impliment in ta, should not be happen */
    SL_ERROR_REQ_CMD_UNSUPPORT,             /* 1015 ERR: request command id not support */

    SL_ERROR_STO_OP_FAILED,                 /* 1016 ERR: storage operate failed, such as mkdir, open, save ... */

    SL_ERROR_EROLL_DUPLICATE,               /* 1017 WARN: finger already enrolled, SPEC: just for enroll */
    SL_ERROR_MOVE_TOO_FAST,                 /* 1018 WARN: finger move too fast when capture image */
    SL_ERROR_DETECT_NO_FINGER,              /* 1019 WARN: not detect finger when capture image */
    SL_ERROR_SAME_AREA,                     /* 1020 WARN: enroll in same area, need move the finger */
    SL_ERROR_QUALITY_FAILED,                /* 1021 WARN: image quality failed */
    SL_ERROR_COVERAREA_FAILED,              /* 1022 WARN: image coverarea failed */
    SL_ERROR_QUALITY_COVERAREA_FAILED,      /* 1023 WARN: image quality and coverarea failed */
    SL_ERROR_FAKE_FINGER,                   /* 1024 WARN: fake finger */

    SL_ERROR_GAIN_IMPROVE_TIMEOUT,          /* 1025 WARN: gain improve time out */
    SL_ERROR_SPI_TIMEOUT,                   /* 1026 WARN: spi timeout */
    SL_ERROR_CONFIG_DOWNLOAD_FAILED,        /* 1027 ERR: download config failed, should not be happend */
    SL_ERROR_SPI_TRANSFER_FAILED,           /* 1028 ERR: spi data transfer failed, should not be happend */
    SL_ERROR_ENROLL_ADDIMAG_FAILED,         /* 1029 ERR: enroll failed, such as malloc failed or others */

    SL_ERROR_CANCELED,                      /* 1030 NORM: operator be canceled */
    SL_ERROR_AUTH_MISMATCH,                 /* 1031 NORM: finger mismatch, SPEC: just for authenticate */

    SL_ERROR_TEMPLATE_INVALID,              /* 1032 */
    SL_ERROR_TEMPLATE_FULL,                 /* 1033 */
    SL_ERROR_TEMPLATE_UNPACK_FAILED,        /* 1034 ERR: load template failed, just happend in load user db */

    SL_ERROR_DETECTED_ESD,                  /* 1035 */
    SL_ERROR_INT_INVALID,                   /* 1036 ERR: data_int mode check fail */

    SL_ERROR_W_BASE_QUALITY_FAILED,         /* 1037 ERR: white base image quality poor */
    SL_ERROR_B_BASE_QUALITY_FAILED,         /* 1038 ERR: black base image quality poor */
    SL_ERROR_BASE_DIFF_FAILED,              /* 1039 ERR: base image diff invalid */
    SL_ERROR_SNR_TEST_FAILED,               /* 1040 ERR: SNR test failed */
    SL_ERROR_M_BASE_QUALITY_FAILED,         /* 1041 ERR: middle base image quality poor */

    SL_ERROR_BASE_IMAGE_PARTIAL_FAILED,     /* 1042 ERR: base image partial */
    SL_ERROR_BASE_IMAGE_DIRTY_FAILED,       /* 1043 ERR: base image dirty */
    SL_ERROR_DATA_INVALID,                  /* 1044 ERR: data is invalid */
    SL_ERROR_DATA_IS_NOT_READY,             /* 1045 ERR: data is not ready */
    SL_ERROR_BUF_SIZE_TOO_SMALL,            /* 1046 ERR: Buffer size is too small */
    SL_ERROR_CHECK_CRC_FAILED,              /* 1047 ERR: Check CRC fail */
    SL_ERROR_GENERIC_FAILED,                /* 1048 ERR: Generic fail */
    SL_ERROR_UNKNOWN_FAILED,                /* 1049 ERR: Unknown fail */
    SL_ERROR_BASE_IMAGE_GLASS_DIRTY_FAILED, /* 1050 ERR: base image glass dirty */
    SL_ERROR_AEC_TIMEOUT,                   /* 1051 ERR: AEC timeout */
    SL_ERROR_BASE_IMAGE_HALO_FAILED,        /* 1052 ERR: base image check halo fail */
    SL_ERROR_CHECK_ICON_FAILED,             /* 1053 ERR: icon detected */
    SL_ERROR_B_BASE_BLOT_FAILED,            /* 1054 ERR: black base image blot detected */
    SL_ERROR_MASK_CIRCLE_FAILED,            /* 1055 ERR: mask imaging circle failed */
    SL_ERROR_CIRCLE_BIAS_FAILED,            /* 1056 ERR: check circle bias failed */
    SL_ERROR_RANGE_NARROW,                  /* 1057 ERR: range narrow */
    SL_ERROR_IMG_OVEREXPOSURE,              /* 1058 ERR: image over exposure */
    SL_ERROR_IMG_TOO_DIM,                   /* 1059 ERR: image too dark */
    SL_ERROR_SHADING_FAILED,                /* 1060 ERR: shading check failed */
    SL_ERROR_CHECK_PVALUE_FAILED,           /* 1061 ERR: check p-value failed */
    SL_ERROR_BLACK_SIGNAL_TEST_FAILED,      /* 1062 ERR: check_black_signal_test failed */
    SL_ERROR_FOV_TEST_FAILED,               /* 1063 ERR: check fov-test failed */
    SL_ERROR_CIRCLE_DOUBLE_TEST_FAILED,     /* 1064 ERR: check circle_double_test failed */
    SL_ERROR_MASK_TEST_FAILED,              /* 1065 ERR: check mask_test failed */
    SL_ERROR_ARTIFICIAL_ATTACK,             /* 1066 ERR: check artifical attack */
    SL_ERROR_MAX,                           /* max */
} sl_error_t;

typedef struct _err_desc {
    const char *errstr;
} err_desc_t;

static inline const char *silfp_err_to_string(int32_t err)
{
    static const err_desc_t desc[] = {
        {"SUCCESS"},                         /* 0 */
        {"BAD_PARAMS"},                      /* 1000 */
        {"OUT_OF_MEMORY"},                   /* 1001 */
        {"DEV_IOCTL_FAILED"},                /* 1002 */
        {"SPI_SET_PARAM_FAILED"},            /* 1003 */
        {"INVALID_CHALLENGE"},               /* 1004 */
        {"UNTRUSTED_ENROLL"},                /* 1005 */
        {"INVALID_HAT_VERSION"},             /* 1006 */
        {"CONFIG_INVALID"},                  /* 1007 */
        {"DEV_OPEN_FAILED"},                 /* 1008 */
        {"TA_OPEN_FAILED"},                  /* 1009 */
        {"TA_SEND_FAILED"},                  /* 1010 */
        {"SHARED_ALLOC_FAILED"},             /* 1011 */
        {"SHARED_MAP_FAILED"},               /* 1012 */
        {"TA_OP_FAILED"},                    /* 1013 */
        {"OP_IMPL_INVALID"},                 /* 1014 */
        {"REQ_CMD_UNSUPPORT"},               /* 1015 */
        {"STO_OP_FAILED"},                   /* 1016 */
        {"EROLL_DUPLICATE"},                 /* 1017 */
        {"MOVE_TOO_FAST"},                   /* 1018 */
        {"DETECT_NO_FINGER"},                /* 1019 */
        {"SAME_AREA"},                       /* 1020 */
        {"QUALITY_FAILED"},                  /* 1021 */
        {"COVERAREA_FAILED"},                /* 1022 */
        {"QUALITY_COVERAREA_FAILED"},        /* 1023 */
        {"FAKE_FINGER"},                     /* 1024 */
        {"GAIN_IMPROVE_TIMEOUT"},            /* 1025 */
        {"SPI_TIMEOUT"},                     /* 1026 */
        {"CONFIG_DOWNLOAD_FAILED"},          /* 1027 */
        {"SPI_TRANSFER_FAILED"},             /* 1028 */
        {"ENROLL_ADDIMAG_FAILED"},           /* 1029 */
        {"CANCELED"},                        /* 1030 */
        {"AUTH_MISMATCH"},                   /* 1031 */
        {"TEMPLATE_INVALID"},                /* 1032 */
        {"TEMPLATE_FULL"},                   /* 1033 */
        {"TEMPLATE_UNPACK_FAILED"},          /* 1034 */
        {"DETECTED_ESD"},                    /* 1035 */
        {"INT_INVALID"},                     /* 1036 */
        {"W_BASE_QUALITY_FAILED"},           /* 1037 */
        {"B_BASE_QUALITY_FAILED"},           /* 1038 */
        {"BASE_DIFF_FAILED"},                /* 1039 */
        {"SNR_TEST_FAILED"},                 /* 1040 */
        {"M_BASE_QUALITY_FAILED"},           /* 1041 */

        {"BASE_IMAGE_PARTIAL_FAILED"},       /* 1042 */
        {"BASE_IMAGE_DIRTY_FAILED"},         /* 1043 */
        {"DATA_INVALID"},                    /* 1044 */
        {"DATA_IS_NOT_READY"},               /* 1045 */
        {"BUF_SIZE_TOO_SMALL"},              /* 1046 */
        {"CHECK_CRC_FAILED"},                /* 1047 */
        {"GENERIC_FAILED"},                  /* 1048 */
        {"UNKNOWN_FAILED"},                  /* 1049 */
        {"BASE_IMAGE_GLASS_DIRTY_FAILED"},   /* 1050 */
        {"AEC_TIMEOUT"},                     /* 1051 */
        {"BASE_IMAGE_HALO_FAILED"},          /* 1052 */
        {"CHECK_ICON_FAILED"},               /* 1053 */
        {"B_BASE_BLOT_FAILED"},              /* 1054 */
        {"MASK_CIRCLE_FAILED"},              /* 1055 */
        {"CIRCLE_BIAS_FAILED"},              /* 1056 */
        {"RANGE_NARROW"},                    /* 1057 */
        {"IMG_OVEREXPOSURE"},                /* 1058 */
        {"IMG_TOO_DIM"},                     /* 1059 */
        {"SHADING_FAILED"},                  /* 1060 */
        {"CHECK_PVALUE_FAILED"},             /* 1061 */
        {"BLACK_SIGNAL_TEST_FAILED"},        /* 1062 */
        {"FOV_TEST_FAILED"},                 /* 1063 */
        {"CIRCLE_DOUBLE_TEST_FAILED"},       /* 1064 */
        {"MASK_TEST_FAILED"},                /* 1065 */
        {"ARTIFICIAL_ATTACK"},               /* 1066 */
    };

    if (err >= 0) {
        err = SL_SUCCESS;
    }

    err = -err;

    if (err == SL_SUCCESS) {
        return desc[0].errstr;
    } else if ((err >= SL_ERROR_BAD_PARAMS) && (err < SL_ERROR_MAX)) {
        return desc[err - SL_ERROR_BAD_PARAMS + 1].errstr;
    }

    return "unknow";
}

#endif // __SILEAD_FINGERPRINT_ERROR_H__
