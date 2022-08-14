/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <cutils/properties.h>
#include <cutils/fs.h>
#include <string.h>
#include "TaLogDump.h"
#include "HalLog.h"
#include "gf_base_types.h"

#define LOG_TAG "[GF_HAL][TaLogDump]"

#define PROPERTY_DUMP_TA_LOG "gf.debug.dump_talog_data"

typedef struct
{
    uint32_t startIndex;
    uint32_t dataSize;
    uint8_t logData[];
} __attribute__((packed)) DUMP_LOG_INFO_T;

namespace goodix
{
#ifdef SUPPORT_TA_LOG_DUMP_TO_CA

    static const uint32_t MAX_LOG_DATA_SIZE = \
        TA_LOG_DUMP_BUFFER_SIZE - offsetof(DUMP_LOG_INFO_T, logData);

    uint32_t TaLogDump::getDumpLevel(void)
    {
        this->mLogDumpLevel = GF_LOG_VERBOSE_LEVEL;
        int32_t ret = property_get_int32(PROPERTY_DUMP_TA_LOG, 0);
        if (ret == 1)
        {
            this->mLogDumpLevel = GF_LOG_DEBUG_LEVEL;
        }
        return this->mLogDumpLevel;
    }

    void TaLogDump::printLog(uint8_t *buffer)
    {
        const DUMP_LOG_INFO_T *info = (DUMP_LOG_INFO_T *) buffer;
        uint32_t i = 0;
        uint32_t dataSize = 0;
        const char *logData = NULL;

        if (this->mLogDumpLevel == 0)
        {
            return;
        }

        dataSize = info->dataSize;
        logData = (char *) info->logData;

        if (dataSize > MAX_LOG_DATA_SIZE)
        {
            dataSize = MAX_LOG_DATA_SIZE;
        }
        while (i < dataSize)
        {
            if (logData[i] != '\0')
            {
                LOG_D("talog", "%s", logData + i)
                i += strlen(logData + i);
            }
            else
            {
                i++;
            }
        }
    }
#else  // SUPPORT_TA_LOG_DUMP_TO_CA
    uint32_t TaLogDump::getDumpLevel(void)
    {
        UNUSED_VAR(this->mLogDumpLevel);
        return 0;
    }

    void TaLogDump::printLog(uint8_t *buffer)
    {
        UNUSED_VAR(buffer);
    }
#endif  // SUPPORT_TA_LOG_DUMP_TO_CA
}  // namespace goodix
