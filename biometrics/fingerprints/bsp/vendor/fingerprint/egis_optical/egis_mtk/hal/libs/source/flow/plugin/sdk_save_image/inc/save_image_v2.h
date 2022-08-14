#ifndef __SAVE_IMAGE_V2_H__
#define __SAVE_IMAGE_V2_H__

#include <stdbool.h>
#include "common_definition.h"
#include "object_def_image.h"

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
    rbs_obj_array_t* rbs_obj_array;
    rbs_obj_image_v1_0_t* rbs_obj_image;
    bool is_bad_image_partial;
    bool is_image_finger_off_0;
    bool is_image_finger_off_1;
    bool is_called_cancel;
    bool is_image_too_fast;
    bool is_image_bad_image;
    bool is_image_partial;
    bool is_image_bad_image_fingeroff1;
    bool is_image_partial_fingeroff1;

} save_image_info_t;

typedef struct {
    int total_count;
    int match_count;
    int not_match_count;
} save_image_finger_count_t;

enum {
    VERIFY_TYPE_NA = 0,
    VERIFY_TYPE_NORMAL_MATCH,
    VERIFY_TYPE_QUICK_MATCH,
    VERIFY_TYPE_LQM_MATCH,
};

void debug_save_image(save_image_info_t image_info);

void save_img8_normalize(char* path, uint16_t* raw_img16, int width, int height);
int get_total_count();
int get_match_count();
int get_not_match_count();
void reset_finger_count();

#endif