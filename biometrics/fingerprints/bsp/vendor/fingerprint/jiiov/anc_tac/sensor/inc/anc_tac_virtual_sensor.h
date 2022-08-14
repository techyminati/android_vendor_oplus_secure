#ifndef __ANC_TAC_VIRTUAL_SENSOR_H__
#define __ANC_TAC_VIRTUAL_SENSOR_H__

#include "anc_error.h"
#include "anc_type.h"


ANC_RETURN_TYPE AncGetVirtualSensorImage(uint8_t *p_input_buffer,
                                        uint32_t input_buffer_size);
ANC_RETURN_TYPE VcSetCurrentImagePath(uint8_t *p_buffer, uint32_t buffer_length);

#endif