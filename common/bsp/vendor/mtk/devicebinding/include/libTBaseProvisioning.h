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
 * libTBaseProvisioning.h
 */

#ifndef LIB_TBASE_PROVISIONING_H_
#define LIB_TBASE_PROVISIONING_H_

#include "MobiCoreDriverApi.h"
#include "mcVersionHelper.h"

#undef LOG_TAG
#define LOG_TAG "libTBaseProvisioning"

#define CMP_VERSION_3_0 MC_MAKE_VERSION(3, 0)

#define RECEIPT_DATA_ENC_SIZE        (512)
#define SUID_LEN                     (16)
#define ADDITIONAL_ENTROPY_SIZE      (32)  // Recommended length for the external entropy input
#define CMD_HEADER_SIZE              (8)   // header + KID
#define CMD_SIGNATURE_SIZE           (256) // PSS Signature Length

/* Expected length for the GenBindingKey Command: header + signature + timestamp + serial number + entropy length + entropy */
#define EXPECTED_COMMAND_SIZE        (CMD_HEADER_SIZE + CMD_SIGNATURE_SIZE + 2 * sizeof(uint64_t) + 1 + ADDITIONAL_ENTROPY_SIZE)

#define RET_OK                       0
#define RET_ERR_INTERNAL             1
#define RET_ERR_INPUT_PARAM          2
#define RET_ERR_CMD_SIZE             3
#define RET_ERR_TA_NO_RESPONSE       4
#define RET_ERR_TA_BAD_RESPONSE_LEN  5
#define RET_ERR_TA_CALL_FAILED       6
#define RET_ERR_WRITE_CONTAINER      7
#define RET_ERR_AUTHCONT_SIZE        8
#define RET_ERR_AUTHCONT_INVALID     9

/**
 * Structure for GenBindingKey Command
 */
typedef struct {
    uint8_t pBinData[EXPECTED_COMMAND_SIZE + 1];
    size_t dataLength;
} genBindingKeyCommand_t;

/**
 * tlcOpen
 *
 * Open a TA
 *
 * @param          spid              SPID of the TA. 0 for CMTL
 *
 *
 * @returns RET_OK if operation is successful
 */
mcResult_t tlcOpen(mcSpid_t spid);

/**
 * tlcCmtlGetSuid
 *
 * Get SUID from CMTL
 *
 * @param [out]    pSuid              SUID of the device
 *
 * @returns RET_OK if operation is successful
 */
mcResult_t tlcCmtlGetSuid(uint8_t* pSuid);

/**
 * tlcCmtlGenBindingKey
 *
 * Send the GenBindingKey command to CMTL and handle the response
 *
 * @param [out]    receiptData        Encrypted ReceiptData (512 bytes)
 * @param [out]    pSuid              SUID of the device
 * @param          pCmpCommandBin     Binary of the GenBindingKey Command. Length > 8 + 256 (header + signature)
 * @param          snData             KPH Serial Number (8 bytes)
 * @param          timestamp          Timestamp (8 bytes)
 *
 * @returns RET_OK if operation is successful
 */
mcResult_t tlcCmtlGenBindingKey(
        uint8_t* pReceiptData,
        uint8_t* pSuid,
        genBindingKeyCommand_t* pCmpCommandBin,
        uint64_t snData,
        uint64_t timestamp,
        uint8_t* entropyInput);

/**
 * tlcCmtlValidateAuthToken
 *
 * Execute beginSoCAuthenticate
 *
 * @param          pfAuthTokenPath     Path to the auth token SO.SoC.Auth
 *
 * @returns RET_OK if operation is successful
 */
mcResult_t tlcCmtlValidateAuthToken(const char* pfAuthTokenPath);

/**
 * tlcClose
 *
 * Close CMTL
 *
 * @returns RET_OK if operation is successful
 */
void tlcClose(void);

#endif // LIB_TBASE_PROVISIONING_H_
