/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _HALUTILS_H_
#define _HALUTILS_H_

#include "gf_error.h"

#define TIME_STAMP_LEN (128)

namespace goodix {
    class HalUtils {
    public:
        static int64_t getCurrentTimeMicrosecond();
        static gf_error_t genTimestamp(char *timestamp, uint32_t len);
        static gf_error_t genTimestamp2(char *timestamp, uint32_t len);
        static gf_error_t mkdirs(const char *dir);
        static gf_error_t makedirsByFilePath(const char *filepath);
        static gf_error_t readFile(char *filepath, char *filename, uint8_t *read_buf,
                                   uint32_t read_buf_len);
        static gf_error_t writeFile(char *filepath, char *filename, uint8_t *write_buf,
                                    uint32_t write_buf_len);
        static gf_error_t genStringID(uint8_t *id, uint32_t len);
        static gf_error_t saveDataToSdcard(const char *filename, uint8_t *data, uint32_t len);
        static gf_error_t loadDataFromSdcard(const char *filename, uint8_t *data, uint32_t len);
    };
}  // namespace goodix



#endif /* _HALUTILS_H_ */
