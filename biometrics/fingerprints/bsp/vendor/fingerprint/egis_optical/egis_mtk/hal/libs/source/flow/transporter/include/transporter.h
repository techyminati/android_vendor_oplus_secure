#ifndef __TRANSPORTER_H__
#define __TRANSPORTER_H__

#define TEE_DEFAULT_COMMAND 1000
#define TEE_CONFIRM_COMMAND 2000

int transporter(unsigned char* msg_data, unsigned int msg_data_len, unsigned char* out_data,
                unsigned int* out_data_len);

#endif