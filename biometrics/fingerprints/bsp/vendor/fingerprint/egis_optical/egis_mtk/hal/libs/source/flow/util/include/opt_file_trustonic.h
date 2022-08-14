#ifndef __OPT_FILE_TRUSTONIC_H__
#define __OPT_FILE_TRUSTONIC_H__

int trustonic_send_data(int type, unsigned char* in_data, int in_data_len);
int trustonic_receive_data(int type, unsigned char* in_data, int in_data_len,
                           unsigned char* out_data, unsigned int* out_data_len);
int trustonic_receive_fingerid(int type, unsigned char* out_data, int* out_data_size);
int trustonic_delete_data(int type);
int save_file(const char* file_name, unsigned char* data, int data_len);
int get_file(const char* file_name, unsigned char* data, int* data_len);
#endif