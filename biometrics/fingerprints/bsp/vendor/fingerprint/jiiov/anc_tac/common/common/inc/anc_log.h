#ifndef __ANC_LOG_H__
#define __ANC_LOG_H__

#include "anc_type.h"

#define ANC_LOG_INFO    0x00
#define ANC_LOG_DEBUG   0x01
#define ANC_LOG_WARNING 0x04
#define ANC_LOG_ERROR   0x08

void AncLogSetMask(uint8_t pri_flags);

#define ANC_LOG_SET_MASK AncLogSetMask



void AncLog(uint8_t level, const char *tag, const char *format, ...);

#define ANC_LOGI(format, ...)\
  AncLog(ANC_LOG_INFO, LOG_TAG, format, ##__VA_ARGS__)

#define ANC_LOGD(format, ...)\
  AncLog(ANC_LOG_DEBUG, LOG_TAG, format, ##__VA_ARGS__)

#define ANC_LOGW(format, ...)\
  AncLog(ANC_LOG_WARNING, LOG_TAG, format, ##__VA_ARGS__)

#define ANC_LOGE(format, ...)\
  AncLog(ANC_LOG_ERROR, LOG_TAG, format, ##__VA_ARGS__)

#endif
