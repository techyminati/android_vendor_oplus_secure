#ifndef __SAVE_IMAGE_H__
#define __SAVE_IMAGE_H__

#include "common_definition.h"

typedef enum {
	SAVE_GOOD_ENROLL,
	SAVE_GOOD_MATCH,
	SAVE_GOOD_NOT_MATCH,
	SAVE_BAD,
} save_image_state_t;

typedef enum {
	SAVE_CMD_CREATE_PATH,
	SAVE_CMD_CLEAN_FILE,
	SAVE_CMD_CLEAN_PATH,
	SAVE_CMD_STATE
} save_image_cmd_t;

typedef struct {
	transfer_image_type_t img_type;
	save_image_state_t img_state;
	int is_new_finger_on;
	int img_width;
	int img_height;
	int img_cnt;
	int select_index; // enroll : which is send to enroll_ex; verify : last tried index
	int img_data_size;
	unsigned char* img_buf;
} save_image_info_t;

void debug_save_image(save_image_info_t image_info);

#endif