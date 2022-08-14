#ifndef __PLAT_ENCRYPTION_H__
#define __PLAT_ENCRYPTION_H__

int plat_encryption(unsigned char *buffer_in, int in_len,
		    unsigned char *buffer_out, int *out_len);
int plat_decryption(unsigned char *buffer_in, int in_len,
		    unsigned char *buffer_out, int *out_len);

#endif