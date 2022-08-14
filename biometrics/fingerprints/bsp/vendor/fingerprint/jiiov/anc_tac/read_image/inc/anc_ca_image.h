#ifndef __ANC_CA_IMAGE_H__
#define __ANC_CA_IMAGE_H__

#include <stdint.h>
#include "anc_error.h"

#ifdef ANC_GET_IMAGE_FROM_TA
ANC_RETURN_TYPE ExtensionSaveImage(uint8_t *p_buffer, uint32_t p_buffer_length, uint8_t **pp_image_data, uint32_t *pp_image_size, int need_save);
#endif

#endif
