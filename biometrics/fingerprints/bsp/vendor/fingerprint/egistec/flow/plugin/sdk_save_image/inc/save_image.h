#ifndef __SAVE_IMAGE_H__
#define __SAVE_IMAGE_H__

#include "common_definition.h"

typedef enum {
	SAVE_GOOD_ENROLL,
	SAVE_GOOD_MATCH,
	SAVE_GOOD_NOT_MATCH,
	SAVE_BAD,
	SAVE_PARTIAL,
	SAVE_FAKE,
	SAVE_FAIL,
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
	int single_img_size;
	int img_cnt;
	int select_index; // enroll : which is send to enroll_ex; verify : last tried index
	unsigned char* img_buf;
} save_image_info_t;

void debug_save_image(save_image_info_t image_info);
void debug_save_image_oplus_k4(save_image_info_t image_info, char *match_info);

#endif