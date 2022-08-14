#include "plat_hmac.h"
#include "plat_log.h"
int hmac_setup_platform_sec_key(unsigned char *ext_data,
				unsigned int ext_data_len,
				unsigned char *out_key,
				unsigned int *out_key_len)
{
	// TODO:implements
	ex_log(LOG_ERROR,
	       "hmac_setup_platform_sec_key needs implements  !!!!!!");
	return 0;
}

int hmac_sha256(unsigned char const *in, const unsigned int in_len,
		unsigned char *key, unsigned int key_len,
		unsigned char *out_hmac)
{
	// TODO:implements
	ex_log(LOG_ERROR, "hmac_sha256 needs implements  !!!!!!");
	return 0;
}