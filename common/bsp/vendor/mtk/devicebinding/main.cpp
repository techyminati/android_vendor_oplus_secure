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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "libTBaseProvisioning.h"
#include <android/log.h>
#define TAG "TBaseDeviceBinding"

#define LOG_i(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOG_e(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)

/* Minimum number of parameter: 2+1: timestamp serial number */
#define PARAM_NUMBER_MIN (3)

/* Path to the GenBindingKey command file. This can be modified  */
static const char* pCmpBinPath = "/sdcard/data/app/tmpDeviceBinding/cmp_genbindinkey.bin";
/* Prefix of the generated encrypted receipt. This can be modified  */
static const char* pfRDPathPrefix = "/sdcard/data/app/tmpDeviceBinding/receipt.";
/* Suffix of the generated encrypted receipt. This can  be modified  */
static const char* pdRDextension = ".bin";
/* Path to the Auth Token SO.SoC.Auth. Used in tlcCmtlValidateAuthToken  */
static const char* pfAuthTokenPath =  "/data/app/mcRegistry/00000000.authtokcont";


static const char* mode = "w";
static uint8_t pSuid[SUID_LEN];
static char pCharSuid[SUID_LEN*2];

static void returnExitCode(int exitCode);

/**
 * Call the GenBindingKey command
 */
int main(int argc, char *args[]) {
    mcResult_t ret = RET_OK;
    size_t  retSize = 0;
    uint8_t  receiptData[RECEIPT_DATA_ENC_SIZE];
    char* pdRDFullPath = NULL;
    FILE*    pRDFile = NULL;
    uint64_t sn;
    uint64_t timestamp;
    char entropyInput[ADDITIONAL_ENTROPY_SIZE * 2];
    uint8_t  entropy[ADDITIONAL_ENTROPY_SIZE];
    genBindingKeyCommand_t cmtlCommand;
    uint8_t *pointerData = NULL;
    size_t nCmdSize;
    LOG_i("Copyright (c) Trustonic Limited 2014\n");

    nCmdSize = readFile(pCmpBinPath, &pointerData);
    if (nCmdSize != EXPECTED_COMMAND_SIZE) {
        LOG_e("Error: Binary command file size != %d\n", EXPECTED_COMMAND_SIZE);
        goto exit;
    }
    cmtlCommand.dataLength = nCmdSize;
    LOG_i("memcopy\n");
    memcpy(cmtlCommand.pBinData, pointerData, 264);

    ret = tlcOpen(0); //System TA so SPID is ignored
    if (MC_DRV_OK != ret) {
        LOG_e("Could not open Trusted Application session\n");
        returnExitCode(2);
    }

    ret = tlcCmtlGetSuid(pSuid);
    if (MC_DRV_OK != ret) {
        LOG_e("Call to tlcCmtlGetSuid failed with ret = %X\n", ret);
        goto exit;
    }

    if (argc < PARAM_NUMBER_MIN) {
        LOG_e("%d parameter is not enough. Please specify the serial Number and the timestamp.\n", argc - 1);
        ret = RET_ERR_INPUT_PARAM;
        goto exit;
    }

    if (strlen(args[1]) > 2 * sizeof(uint64_t) || strlen(args[2]) > 2 * sizeof(uint64_t)) {
        LOG_e("Input length error: strlen(Serial Number) = %d | strlen(Timestamp) = %d \n", strlen(args[1]), strlen(args[2]));
        ret = RET_ERR_INPUT_PARAM;
        goto exit;
    }
    sn = atoll(args[1]);
    timestamp = atoll(args[2]);

    if(args[3] == NULL) {
        LOG_i("No entropy input");
    } else {
        LOG_i("Entropy Input string length = %d\n", strlen(args[3]));
        if (strlen(args[3]) == 2 * ADDITIONAL_ENTROPY_SIZE) {
            memcpy((uint8_t*) entropyInput, (uint8_t*) args[3], ADDITIONAL_ENTROPY_SIZE);
        } else {
            LOG_e("Entropy Input Length must be = %d bytes\n", ADDITIONAL_ENTROPY_SIZE);
            ret = RET_ERR_INPUT_PARAM;
            goto exit;
        }
    }

    if (argc > PARAM_NUMBER_MIN) {
        size_t entropyStrLen = ADDITIONAL_ENTROPY_SIZE * 2;
        hexStringToBin(entropy, entropyInput, &entropyStrLen);
        if (entropyStrLen != ADDITIONAL_ENTROPY_SIZE * 2) {
            LOG_e("Error while copying entropy bytes\n");
            goto exit;
        }
        ret = tlcCmtlGenBindingKey(receiptData, pSuid, &cmtlCommand, sn, timestamp, entropy);
    } else {
        ret = tlcCmtlGenBindingKey(receiptData, pSuid, &cmtlCommand, sn, timestamp, NULL);
    }

    if (MC_DRV_OK != ret) {
        LOG_e("Call to TA failed with ret = %X\n", ret);
        goto exit;
    }
	
	/*
    ret = tlcCmtlValidateAuthToken(pfAuthTokenPath);
    if (MC_DRV_OK != ret) {
        LOG_e("Call to tlcCmtlValidateAuthToken failed with ret = %X", ret);
        goto exit;
    }
	*/
	
    if (SUID_LEN != suidBinToChar(pCharSuid, pSuid, SUID_LEN)) {
        LOG_e("Error while writing SUID\n");
        goto exit;
    }

    pdRDFullPath = (char*) malloc(sizeof(char) * (strlen(pfRDPathPrefix) + strlen(pdRDextension) + SUID_LEN*20));
    if (NULL == pdRDFullPath) {
        LOG_e("Dynamic memory allocation failed\n");
        goto exit;
    }
    strcpy(pdRDFullPath, pfRDPathPrefix);
    strcat(pdRDFullPath, pCharSuid);
    strcat(pdRDFullPath, pdRDextension);

    pRDFile = fopen(pdRDFullPath, mode);
    if (pRDFile == NULL) {
        LOG_e("Error: Cannot open file: %s\n", pdRDFullPath);
        goto exit;
    }
    retSize = fwrite(receiptData, sizeof(uint8_t), RECEIPT_DATA_ENC_SIZE, pRDFile);
    if (retSize == 0) {
        LOG_e("Error while writing to the file\n");
        goto exit;
    }
    if (retSize != RECEIPT_DATA_ENC_SIZE) {
        LOG_e("Cannot write all data: len = %d = %d\n", RECEIPT_DATA_ENC_SIZE, retSize);
        goto exit;
    }

exit:
    if (pdRDFullPath != NULL) {
        free(pdRDFullPath);
    }
    if (pRDFile != NULL)
    {
        fclose(pRDFile);
    }
    if (pointerData != NULL) {
        free(pointerData);
    }
    tlcClose();

    returnExitCode(ret);
    return ret;
}

static void returnExitCode(int exitCode) {
    if (0 != exitCode) {
        LOG_e("Failure\n");
    }
    else {
        LOG_i("Success\n");
    }
    LOG_i("TLC exit code: %08x\n", exitCode);
    /* Print exit code for adb shell return code analysis */
    printf("%d\n", exitCode);
    exit(exitCode);
}
