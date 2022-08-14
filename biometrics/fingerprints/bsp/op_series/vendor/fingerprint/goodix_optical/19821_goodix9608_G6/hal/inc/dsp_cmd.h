/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version:
 * Description:
 * History:
 */

#ifndef _DSP_CMD_H_
#define _DSP_CMD_H_

typedef enum {
    SEC_FP_HVX_ALGO_INIT,
    SEC_FP_HVX_ALGO_DEINIT,
    SEC_FP_HVX_GET_FEATURE_TWO,
    SEC_FP_HVX_GET_FEATURE_FOUR,
    SEC_FP_HVX_AGING_TEST,
    SEC_FP_HVX_GET_VERSION,
    SEC_PROCESS_MAP_BUF,
    SEC_PROCESS_UNMAP_BUF,
    SEC_ENABLE_LOG_DUMP,
    SEC_DISABLE_LOG_DUMP,
    SEC_FP_SET_CHIP_TYPE,
    SEC_CMD_MAX
} dsp_cmd_type;

#endif  // #ifndef _DSP_CMD_H_
