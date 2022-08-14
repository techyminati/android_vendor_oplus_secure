/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _TALOGDUMP_H_
#define _TALOGDUMP_H_

#include <stdint.h>

namespace goodix {
    class TaLogDump {
    public:
        TaLogDump();
        ~TaLogDump();
        uint32_t getDumpLevel(void);
        void printLog(uint8_t *buffer);
    private:
        uint32_t mLogDumpLevel;
        uint32_t mCurrentLogFileBytes;
        void writeLogToFile(const char *filepath, const uint8_t *buffer,
                                     uint32_t buffer_len, uint8_t limit_write);
        bool saveAlgoLog(const uint8_t *buffer, uint32_t buffer_len);
        bool saveMemLog(const uint8_t *buffer, uint32_t buffer_len);
    };
}

#endif  // _TALOGDUMP_H_
