/************************************************************************************
 ** File: - FingerprintCore.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      hal utils implementation for goodix optical fingerprint (android p)
 **
 ** Version: 1.0
 ** Date : 18:03:11,17/10/2018
 ** Author: wudongnan@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  Dongnan.Wu   2019/04/18      add the way to get real timestamp
 ************************************************************************************/

#ifndef _HALUTILS_H_
#define _HALUTILS_H_

#include "HalContext.h"
#include "gf_error.h"

#define TIME_STAMP_LEN (128)

namespace goodix
{
    class HalUtils
    {
    public:
        static int64_t getCurrentTimeMicrosecond();
        static int64_t getrealtime();
        static gf_error_t genTimestamp(char* timestamp, uint32_t len);
        static gf_error_t genTimestamp2(char* timestamp, uint32_t len);
        static gf_error_t mkdirs(const char* dir);
        static gf_error_t makedirsByFilePath(const char* filepath);
        static gf_error_t readFile(char *filepath, char *filename, uint8_t *read_buf, uint32_t read_buf_len);
        static gf_error_t writeFile(char *filepath, char *filename, uint8_t *write_buf, uint32_t write_buf_len);
        static gf_error_t manage_finger_dump_dir(char *path, char *filename);
        static gf_error_t customizedEncrypt(uint8_t* src, uint32_t src_len,
                                            uint8_t **encrypt, uint32_t *encrypt_len,
                                            char *filename, uint32_t operation);
    };
}   // namespace goodix



#endif /* _HALUTILS_H_ */
