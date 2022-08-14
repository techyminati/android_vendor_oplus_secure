#ifndef __PAYMENT_
#define __PAYMENT_

#include <stdint.h>

#define SUPPORT_MAX_ENROLL_COUNT 5

struct LastIdentifyResult {
	unsigned int group_id;
	unsigned int matched_fingerprint_id;
	uint64_t operation_id;
	unsigned int x_len;
	unsigned char x[0];
};

typedef enum {
    IFAA_ERR_SUCCESS                     = 0x00000000,
    IFAA_ERR_UNKNOWN                     = 0x7A000001,
    IFAA_ERR_BAD_ACCESS                  = 0x7A000002,
    IFAA_ERR_BAD_PARAM                   = 0x7A000003,
    IFAA_ERR_UNKNOWN_CMD                 = 0x7A000004,
    IFAA_ERR_BUF_TO_SHORT                = 0x7A000005,
    IFAA_ERR_OUT_OF_MEM                  = 0x7A000006,
    IFAA_ERR_TIMEOUT                     = 0x7A000007,
    IFAA_ERR_HASH                        = 0x7A000008,
    IFAA_ERR_SIGN                        = 0x7A000009,
    IFAA_ERR_VERIFY                      = 0x7A00000A,
    IFAA_ERR_KEY_GEN                     = 0x7A00000B,
    IFAA_ERR_READ                        = 0x7A00000C,
    IFAA_ERR_WRITE                       = 0x7A00000D,
    IFAA_ERR_ERASE                       = 0x7A00000E,
    IFAA_ERR_NOT_MATCH                   = 0x7A00000F,
    IFAA_ERR_GEN_RESPONSE                = 0x7A000010,
    IFAA_ERR_GET_DEVICEID                = 0x7A000011,
    IFAA_ERR_GET_LAST_IDENTIFIED_RESULT  = 0x7A000012,
    IFAA_ERR_AUTHENTICATOR_SIGN          = 0x7A000013,
    IFAA_ERR_GET_ID_LIST                 = 0x7A000014,
    IFAA_ERR_GET_AUTHENTICATOR_VERSION   = 0x7A000015,
    IFAA_ERR_UN_INITIALIZED              = 0x7A000016,

    // more in considering....
} IFAA_Result;

void payment_update_last_identify_result(struct LastIdentifyResult* result);
unsigned int payment_update_fingerprint_ids(unsigned int* ids, unsigned int count, char* path);

unsigned int payment_get_last_identify_fingerid();
unsigned int payment_get_version();

unsigned int payment_setAuthenticatorVersion(unsigned int version, char* path);

#endif