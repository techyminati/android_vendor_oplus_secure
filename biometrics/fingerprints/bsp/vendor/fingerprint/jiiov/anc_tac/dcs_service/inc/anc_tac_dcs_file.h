#ifndef __ANC_TAC_DCS_FILE_H__
#define __ANC_TAC_DCS_FILE_H__

#include <stdint.h>

#include "anc_error.h"
#include "extension_command.h"

#define ANC_ALGO_SAVE_INFO_PATH_LEN 255

ANC_RETURN_TYPE AlgoInfoParse(uint8_t *p_share_buffer, ANC_COMMAND_EXTENSION_TYPE extension_command_type, uint8_t * p_image_name_buffer);

#endif