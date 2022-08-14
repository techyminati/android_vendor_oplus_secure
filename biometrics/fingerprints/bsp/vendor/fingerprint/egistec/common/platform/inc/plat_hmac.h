#ifndef __PLAT_HMAC_H_
#define __PLAT_HMAC_H_

#define HMAC_SHA256_BITS 256
#define HMAC_SHA256_BYTES 32

#define MAX_SECURE_KEY_LEN 256

struct PlatSecureKey {
	unsigned char key[MAX_SECURE_KEY_LEN];
	unsigned int len;
};

int hmac_setup_platform_sec_key(unsigned char *ext_data,
				unsigned int ext_data_len,
				unsigned char *out_key,
				unsigned int *out_key_len);

int hmac_sha256(unsigned char const *in, const unsigned int in_len,
		unsigned char *key, unsigned int key_len,
		unsigned char *out_hmac);

int fp_get_hmac_key(unsigned char *hmac_key, unsigned int key_length);

#endif