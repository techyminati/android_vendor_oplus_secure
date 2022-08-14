/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_ca dump log header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_CA_DUMP_LOG_H_
#define _GF_CA_DUMP_LOG_H_

#ifdef __cplusplus
extern "C"
{
#endif  // end ifdef __cplusplus

#define GF_TA_LOG_BUF_LENGTH (1024 * 32)

#define GF_EXTEND_SIZE GF_TA_LOG_BUF_LENGTH

#define GF_LOG_OUTPUT_LENGTH 1024

#define GF_LOGBUF_MAGIC    0x6C6470

typedef enum gf_log_level
{
    GF_LOG_SILENT = 0,
    GF_LOG_ERROR,
    GF_LOG_INFO,
    GF_LOG_DEBUG,
    GF_LOG_VERBOSE,
    GF_LOG_UNKNOWN,
} gf_log_level_t;
typedef struct gf_ta_log_info
{
    uint32_t logbuf_magic;
    uint32_t logbuf_index;
    uint32_t logbuf_sum;
    uint32_t logbuf_dump_level;
    int8_t logbuf[];
} gf_ta_log_info_t;

extern volatile gf_log_level_t g_logdump_level;

uint32_t gf_ca_get_logbuf_length(void);

void gf_ca_logdump(gf_ta_log_info_t *log_ptr);

#ifdef __cplusplus
}
#endif  // end ifdef __cplusplus

#endif  // _GF_CA_DUMP_LOG_H_
