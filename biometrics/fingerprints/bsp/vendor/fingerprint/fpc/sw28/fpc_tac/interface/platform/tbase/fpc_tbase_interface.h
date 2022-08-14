/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef __FPC_TBASE_INTERFACE_H__
#define __FPC_TBASE_INTERFACE_H__

#define TBASE_MSG_SETUP    1
#define TBASE_MSG_COMMAND  2

#define TBASE_MSG_RES_ILLEGAL_MESSAGE 1

#define TBASE_SETUP_CMD_INIT 1
#define TBASE_SETUP_CMD_EXIT 2

#define TBASE_SETUP_RES_ILLEGAL_CMD 1

typedef struct _fpc_tbase_msg
{
    uint8_t msg_type;
    int32_t response;

    union
    {
        struct
        {
            uint32_t    command;
            int32_t     response;
        } setup;
        struct
        {
            uint32_t virtual_addr;
            uint32_t virtual_addr_len;
            int32_t     response;
        } command;
    };

} __attribute__((packed)) fpc_tbase_msg_t;


#endif //__FPC_TBASE_INTERFACE_H__
