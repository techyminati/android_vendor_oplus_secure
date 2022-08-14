#ifndef ETS_QSEE_KEYMASTER_H
#define ETS_QSEE_KEYMASTER_H

#define MAX_MASTERKEY_LEN 256

unsigned int get_secure_key(unsigned char* blob, unsigned int* blob_len);

#endif
