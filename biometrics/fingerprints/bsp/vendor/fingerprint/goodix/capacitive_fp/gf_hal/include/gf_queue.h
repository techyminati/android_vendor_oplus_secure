/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Description: gf_queue header file
 * History:
 * Version: 1.0
 */

#ifndef _GF_QUEUE_H_
#define _GF_QUEUE_H_

#include "gf_error.h"
#include "gf_common.h"

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

extern gf_error_t gf_queue_init();
extern gf_error_t gf_queue_exit();

extern gf_error_t gf_queue_clear_unused_irq();
extern uint8_t gf_empty_queue();
extern gf_error_t gf_enqueue(uint8_t cmd_type);
extern gf_error_t gf_dequeue(uint8_t *cmd_type);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_QUEUE_H_
