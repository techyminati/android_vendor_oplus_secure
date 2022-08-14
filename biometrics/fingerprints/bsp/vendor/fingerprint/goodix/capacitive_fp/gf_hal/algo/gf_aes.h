/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: AES encrytpion and decryption
 * History: None
 * Version: 1.0
 */
#ifndef _GF_AES_H_
#define _GF_AES_H_

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdint.h>

void gf_hal_aes_encrypt(const uint8_t *in, uint8_t *out, uint32_t len);
void gf_hal_aes_decrypt(const uint8_t *in, uint8_t *out, uint32_t len);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_AES_H_

