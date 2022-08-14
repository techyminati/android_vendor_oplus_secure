#ifndef __ANC_HAL_EXTENSION_COMMAND_H__
#define __ANC_HAL_EXTENSION_COMMAND_H__

#include <stdint.h>

#include "anc_hal_manager.h"
#include "anc_error.h"

ANC_RETURN_TYPE ExtensionCommandWork(AncFingerprintManager *p_manager,
          uint8_t **p_output_buffer, uint32_t *p_output_buffer_length);

#endif
