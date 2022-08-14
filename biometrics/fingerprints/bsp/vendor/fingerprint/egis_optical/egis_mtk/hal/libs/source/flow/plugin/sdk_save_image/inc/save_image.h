#ifndef __SAVE_IMAGE_H__
#define __SAVE_IMAGE_H__

#include "common_definition.h"
#include "type_def.h"

#ifdef __ET7XX__
#define MAX_IMAGE_BUFFER_SIZE (224 * 224 * 2)
#else
#define MAX_IMAGE_BUFFER_SIZE (120 * 120 * 2)
#endif

#define MULTI_MAX_IMAGE_BUFFER_SIZE (MAX_IMAGE_BUFFER_SIZE * EGIS_TRANSFER_FRAMES_PER_TIME)

void save_img16_BigEndian(char* path, uint16_t* raw_img16, int width, int height);
int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good,
                              int match_result);
#endif