#ifndef __OPT_FILE_H__
#define __OPT_FILE_H__

#include "common_definition.h"

#ifdef __TRUSTONIC__
#include "opt_file_trustonic.h"
#endif

#define MAX_PATH_LEN 256

int opt_create_template_folder(const char* path);
int opt_send_data(int type, unsigned char* in_data, int in_data_size);
int opt_receive_data(int type, unsigned char* in_data, int in_data_size,
		     unsigned char* out_data, int* out_data_size);

#endif
