#ifndef __ANC_ALGORITHM_IMAGE_H__
#define __ANC_ALGORITHM_IMAGE_H__

#include <stdint.h>

#include "anc_error.h"
#include "algorithm_command.h"
#include "extension_command.h"


ANC_RETURN_TYPE  AlgoImageParse(uint8_t *p_share_buffer, ANC_COMMAND_EXTENSION_TYPE command_type, uint8_t *p_image_name_buffer);
ANC_RETURN_TYPE  AlgoImageRenameEnrollFolder(uint32_t finger_id, ANC_BOOL is_finished);

#endif
