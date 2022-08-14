 /*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/*
 * utils.h
 */

#ifndef UTILS_H_
#define UTILS_H_

size_t suidBinToChar(
    char* cSuid,
    uint8_t* pSuid,
    size_t length);

void hexStringToBin(
    uint8_t* pArray,
    char* hexString,
    size_t* pStrlen);

size_t readFile(
    const char* pPath,
    uint8_t** ppContent);


#endif // UTILS_H_
