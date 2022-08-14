#include "plat_encryption.h"
#include "egis_definition.h"
#include "plat_log.h"

int plat_encryption(unsigned char *buffer_in, int in_len,
		    unsigned char *buffer_out, int *out_len)
{
	ex_log(LOG_ERROR, "%s not supported", __func__);
	return EGIS_OK;
}

int plat_decryption(unsigned char *buffer_in, int in_len,
		    unsigned char *buffer_out, int *out_len)
{
	ex_log(LOG_ERROR, "%s not supported", __func__);
	return EGIS_OK;
}