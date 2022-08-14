/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *
 * Description: Decode dumped data and save
 * History: None
 * Version: 1.0
 */
#ifndef _GF_DUMP_DATA_DECODER_H_
#define _GF_DUMP_DATA_DECODER_H_


#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

int32_t gf_decode_and_write_file(const void* buf, uint32_t buf_len, const uint8_t* root_dir);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _GF_DUMP_DATA_DECODER_H_

