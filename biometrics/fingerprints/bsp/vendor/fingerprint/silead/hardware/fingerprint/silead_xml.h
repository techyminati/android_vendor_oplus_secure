/******************************************************************************
 * @file   silead_xml.h
 * @brief  Contains XML parse functions header file.
 *
 *
 * Copyright (c) 2016-2019 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Martin Wu  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_XML_PARSE_H__
#define __SILEAD_XML_PARSE_H__

int32_t silfp_xml_get_sysparams(void **ppbuf, uint32_t cid, uint32_t sid, uint32_t vid);
int32_t silfp_xml_dump(const char *dir, const char *module, void **buf, uint32_t *size, uint32_t *offset);

#endif /* __SILEAD_XML_PARSE_H__ */