/*******************************************************************************************
 * Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
 *
 * File: ree_com_api.h
 * Description: communication api for ree
 * Version: 1.0
 * Date : 2021-5-12
 * Author: wangzhi(Kevin) wangzhi12
 ** -----------------------------Revision History: -----------------------
 **  <author>      <date>            <desc>
 **  Zhi.Wang   2021/05/12        create the file
 *********************************************************************************************/
#ifndef REE_COM_API_H
#define REE_COM_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
ree_ta_init:
@param[in] void
@return 0:success other:failed
@note1
**/
int ree_ta_init(void);

/**
ree_ta_init:
@param[in] buf: command buffer
@param[in] buf_len: command buffer length
@return 0:success other:failed
@note1
**/
int ree_ta_cmd_handler(void* buf, unsigned int buf_len);

/**
ree_ta_shutdown:
@param[in] void
@return 0:success other:failed
@note1
**/
int ree_ta_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif  // REE_COM_API_H
