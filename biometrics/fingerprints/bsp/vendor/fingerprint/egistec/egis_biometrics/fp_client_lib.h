/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
EGIS QSEEComAPI Library Functions

GENERAL DESCRIPTION
  Contains the library functions for accessing egisfpapp.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2010-2014 Egis Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __FP_CLIENT_LIB_H_
#define __FP_CLIENT_LIB_H_

#define CLIENT_CMD_ETS_MESSAGE    0xe0

/* Error code: status sent as response to command from sample client*/
#define FP_OK    0
#define FP_MATCHOK    1
#define FP_FINISH    2
#define SFS_OK    3

#define FP_ERR    1000
#define FP_TIMEOUT    1001
#define FP_BADIMAGE    1002
#define FP_FEATURELOW    1003
#define FP_DUPLICATE    1004
#define FP_NOTDUPLICATE    1005
#define FP_MATCHFAIL    1006



/*----------------------------------------------------------------------------
 * Include Files
* -------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _egis_tlc_handle_t
{
    void* handle;
    uint32_t handle_size;
} EGIS_TLC_HANDLE_T;

int32_t qsc_start_app(EGIS_TLC_HANDLE_T **p_tlcComHandle,
                      const char *pStrAppname, int32_t nBufSize);
int32_t qsc_shutdown_app(EGIS_TLC_HANDLE_T **p_tlcComHandle);
int32_t ets_issue_send_modified_cmd_req(EGIS_TLC_HANDLE_T *l_tlcComHandle,
                                        unsigned char* req_tci_data,
                                        int    req_tci_size,
                                        unsigned char *rsp_tci_data,
                                        int * rsp_tci_size);

int32_t ets_issue_send_modified_cmd_req_in_data(EGIS_TLC_HANDLE_T *l_tlcComHandle,
                                        unsigned char* req_tci_data,
                                        int    req_tci_size,
                                        unsigned char *rsp_tci_data,
                                        int * rsp_tci_size,
                                        unsigned char *in_data,
                                        unsigned int in_data_len);

int32_t ets_qsee_cmd_req_in_data(EGIS_TLC_HANDLE_T *l_tlcComHandle,
                                        unsigned char* req_tci_data,
                                        int    req_tci_size,
                                        unsigned char *rsp_tci_data,
                                        int * rsp_tci_size,
                                        unsigned char *in_data,
                                        unsigned int in_data_len);
/*****************************************************/


#ifdef __cplusplus
}
#endif

#endif//__FP_CLIENT_LIB_H_
