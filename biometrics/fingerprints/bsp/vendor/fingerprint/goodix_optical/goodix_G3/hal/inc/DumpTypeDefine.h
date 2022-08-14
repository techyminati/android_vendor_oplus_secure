/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _DUMPTYPEDEFINE_H_
#define _DUMPTYPEDEFINE_H_

#define PROPERTY_DUMP_DATA "gf.debug.dump_data"
#define OPLUS_LIGHT_SPOT_TYPE "persist.vendor.fingerprint.optical.iconnumber"

#define GF_DUMP_FILE_PATH_MAX_LEN (256)
#define GF_DUMP_SDCARD_ROOT_PATH "/sdcard"
#define GF_DUMP_ENCRYPT_DIR "/gf_data/encrypted_data/"
#define GF_DUMP_AUTHENTICATE_DIR "/gf_data/authenticate/"
#define GF_DUMP_ENROLL_DIR "/gf_data/enroll/"
#define GF_DUMP_KB_CALIBRATION "/gf_data/kb_calibration/"
#define GF_DUMP_DEV_INFO_PATH "/gf_data/device/"
#define GF_DUMP_REAL_PATH "/gf_data/real_data/"
#define GF_DUMP_TEMPLATES_PATH "/gf_data/templates/"

namespace goodix
{
    typedef enum
    {
        DUMP_PATH_SDCARD = 0,  //
        DUMP_PATH_DATA,
    } DUMP_PATH;

    typedef enum
    {
        OP_RESULT_ALL = 0,
        OP_RESULT_SUCCESS,
        OP_RESULT_FAIL
    } OP_RESULT;

    typedef enum
    {
        OP_DATA_NONE = 0,
        OP_DATA_RAW_DATA = 1 << 0,
        OP_DATA_CALIBRATION_AUTO_PARAMS = 1 << 1,
        OP_DATA_CALIBRATION_DATA = 1 << 2,
        OP_DATA_DATA_BMP = 1 << 3,
        OP_DATA_SITO_BMP = 1 << 4,
        OP_DATA_TEMPLATE_DATA = 1 << 5,
        OP_DATA_DEVICE_INFO = 1 << 6,
        OP_DATA_CIRCLE_BMP = 1 << 7,
        OP_DATA_BIGDATA = 1 << 8,
        OP_DATA_HVX_DATA = 1 << 9,
        // Add other data type
        OP_DATA_ALL = OP_DATA_RAW_DATA | OP_DATA_CALIBRATION_AUTO_PARAMS
        | OP_DATA_CALIBRATION_DATA | OP_DATA_DATA_BMP | OP_DATA_SITO_BMP
        | OP_DATA_TEMPLATE_DATA | OP_DATA_DEVICE_INFO | OP_DATA_CIRCLE_BMP
        | OP_DATA_BIGDATA | OP_DATA_HVX_DATA
    } OP_DATA_TYPE;

    typedef struct
    {
        uint32_t dumpOp;
        OP_RESULT result;
        OP_DATA_TYPE type;
    } DumpConfigOption;

    typedef struct
    {
        bool encryped;
        DUMP_PATH dumpPath;
        DumpConfigOption* table;
        uint32_t tableSize;
    } DumpConfig;
}  // namespace goodix

#endif /* _DUMPTYPEDEFINE_H_ */
