#ifndef __OPT_FILE_TRUSTONIC_H__
#define __OPT_FILE_TRUSTONIC_H__

#define MAX_FIX_PATTERN_SIZE   40 * 1024
typedef struct {
	unsigned char fix_data[MAX_FIX_PATTERN_SIZE];
	int fix_data_size;
	unsigned char enroll_mask[MAX_FIX_PATTERN_SIZE];
	int enroll_mask_size;
} fix_pattern_data;

int trustonic_send_data(int type, unsigned char* in_data, int in_data_len);
int trustonic_receive_data(int type, unsigned char* in_data, int in_data_len);

#endif