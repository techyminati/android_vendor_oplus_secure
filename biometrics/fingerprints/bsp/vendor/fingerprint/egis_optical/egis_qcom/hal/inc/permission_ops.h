#ifndef __PERMISSION_OPS_H
#define __PERMISSION_OPS_H

#define PERMISSION_OPS_SUCCESS 0x00000000
#define PERMISSION_OPS_DENIED 0xFFFF0000
#define PERMISSION_OPS_INVALID_PARAMS 0xFFFF0002
#define PERMISSION_OPS_SEC_KEY_ERR 0xFFFF0004
#define PERMISSION_OPS_COMMAND_ERR 0xFFFF0008
#define PERMISSION_OPS_INVALID_TOKEN 0xFFFF0200
#define PERMISSION_OPS_OUTDATE_TOKEN 0xFFFF0400

/*	defined in Android Open Source Project by Google Inc.
*  	hardward/hw_auth_token.h
*/
#define AOSP_AUTH_TOKEN_LIFETIME 10 * 60 * 1000


#define HW_AUTH_TOKEN_VERSION 0

struct __attribute__((__packed__)) AospHwAuthToken {
	unsigned char version;
	unsigned long long challenge;
	unsigned long long user_id;
	unsigned long long authenticator_id;
	unsigned int authenticator_type;
	unsigned long long timestamp;
	unsigned char hmac[32];
};

int perms_generate_aosp_hw_auth_token(unsigned long long challenge,
				      unsigned char *out,
				      unsigned int *out_len);

int perms_check_aosp_hw_auth_token(unsigned char *token, unsigned int len);

#endif