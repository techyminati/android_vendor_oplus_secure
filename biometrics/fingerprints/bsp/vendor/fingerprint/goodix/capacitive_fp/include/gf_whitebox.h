/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: 1.0
 * Description: Define all public interfaces to use whitebox library.
 * History: none
 */

#ifndef _GF_WHITEBOX_H_
#define _GF_WHITEBOX_H_

#include <stdint.h>

#define WB_FALSE 0
#define WB_TRUE 1

// white box operation type
#define WB_OP_GEN_AUTH_DATA                 0x0001
#define WB_OP_SET_DYN_KEY                   0x0002
#define WB_OP_CHECK_ENV                     0x0003

#define WB_OP_ENCRYPT_AES_LOOKUP_TABLE      0x1001  // aes128-lookuptable
#define WB_OP_ENCRYPT_AES_COMPOSED1         0x1002  // aes128-lookuptable + aes128-cbc
#define WB_OP_ENCRYPT_AES_COMPOSED2         0x1003  // aes128-lookuptable + aes128-cbc + xor

#define WB_OP_DECRYPT_AES_LOOKUP_TABLE      0x2001
#define WB_OP_DECRYPT_AES_COMPOSED1         0x2002
#define WB_OP_DECRYPT_AES_COMPOSED2         0x2003

#define WB_OP_ENCODE_AES_LOOKUP_TABLE       0x3001
#define WB_OP_ENCODE_AES_COMPOSED           0x3002

#define WB_OP_DECODE_AES_LOOKUP_TABLE       0x4001
#define WB_OP_DECODE_AES_COMPOSED           0x4002

#define WB_OP_HMAC_SHA256                   0x5001
#define WB_OP_HMAC_CRC                      0x5002

#define AES_BLOCK_MASK         0x0F
#define AES_BLOCK_SIZE         0x10
#define AES_BLOCK_ALIGN(x)     (((x) + AES_BLOCK_MASK) & (~AES_BLOCK_MASK))

#define HMAC_DIGEST_SIZE       32

/**
 * It's a simplified wrapper to encrypt raw data using aes128-lookuptable
 * + aes128-cbc + xor composed algorithm.
 * \param in[in] input raw data to encrypt.
 * \param out[out] output data encrypted, it can be equal to 'in'.
 * \param size[in] the data size of 'in',
 * and the data size of 'out' should be AES_BLOCK_ALIGN(size).
 * \param dynamic[in] dynamic white box or not, the dynamic key is set by
 * operation type 'WB_OP_SET_DYN_KEY'.
 */
#define WHITEBOX_ENCRYPT(in, out, size, dynamic) \
    whitebox_exec(WB_OP_ENCODE_AES_COMPOSED, in, out, size, 0); \
    whitebox_exec(WB_OP_ENCRYPT_AES_COMPOSED2, out, out, AES_BLOCK_ALIGN(size), dynamic); \
    whitebox_exec(WB_OP_DECODE_AES_COMPOSED, out, out, AES_BLOCK_ALIGN(size), 0)

/**
 * It's a simplified wrapper to decrypt data using aes128-lookuptable
 * + aes128-cbc + xor composed algorithm.
 * \param in[in] input data to decrypt.
 * \param out[out] output data decrypted, it can be equal to 'in'.
 * \param size[in] the data size of 'out',
 * and the data size of 'in' should be AES_BLOCK_ALIGN(size).
 * \param dynamic[in] dynamic white box or not, the dynamic key is set by
 * operation type 'WB_OP_SET_DYN_KEY'.
 */

#define WHITEBOX_DECRYPT(in, out, size, dynamic) \
    whitebox_exec(WB_OP_ENCODE_AES_COMPOSED, in, out, AES_BLOCK_ALIGN(size), 0); \
    whitebox_exec(WB_OP_DECRYPT_AES_COMPOSED2, out, out, AES_BLOCK_ALIGN(size), dynamic); \
    whitebox_exec(WB_OP_DECODE_AES_COMPOSED, out, out, size, 0)
/**
 * Run white box operation specified by 'op_type'
 * \param op_type[in] operation type, the value is WP_OP_*
 * \param in[in] input data buffer to read
 * \param out[out] output data buffer to write, can be equal to 'in'
 * When op_type is WP_OP_SET_DYN_KEY, output buffer is unused.
 * \param size[in] the buffer size
 * Detail:
 * (1) If op_type is WB_OP_CHECK_ENV, 'in' is not used, it's the size of 'out',
 * and must be sizeof(uint32_t).
 * (2) If op_type is WB_OP_GEN_AUTH_DATA, it's the data size of 'in' and 'out'
 * (3) If op_type is WB_OP_SET_DYN_KEY, it's the data size of 'in'
 * (4) If op_type is WB_OP_ENCRYPT_AES_* or WB_OP_DECRYPT_AES_*, it's the data
 * size of 'in' and 'out'
 * (5) If op_type is WB_OP_ENCODE_AES_*, it's the data size of 'in', the data
 * size of 'out' is AES_BLOCK_ALIGN(size)
 * (6) If op_type is WB_OP_DECODE_AES_*, it's the data size of 'out', the data
 * size of 'in' is AES_BLOCK_ALIGN(size)
 * (7) If op_type is WB_OP_HMAC_SHA256 or WB_OP_HMAC_CRC, it's the size of 'in' input data,
 * 'out' buffer size should be 256 bits.
 * \param extra1[in] extra parameter to pass
 * Detail:
 * (1) If op_type is WB_OP_ENCRYPT_AES_* or WB_OP_DECRYPT_AES_*, it means using
 * dynamic white box or not, the value is WB_TRUE or WB_FALSE
 * (2) If op_type is WB_OP_SET_DYN_KEY, it means the input key is encrypted
 * or not, the value is WB_TRUE or WB_FALSE
 * (3) Otherwise, it is unused
 */
void whitebox_exec(uint32_t op_type, void *in, void *out, uint32_t size,
        uint32_t extra1);

#endif  // _GF_WHITEBOX_H_

