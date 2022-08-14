#ifndef __ANC_LOG_STRING_H__
#define __ANC_LOG_STRING_H__

#include "anc_type.h"
#include "anc_error.h"
#include "anc_command.h"


char *AncConvertReturnTypeToString(ANC_RETURN_TYPE return_type);
char *AncConvertCommandIdToString(ANC_COMMAND_TYPE type, uint32_t id);
char *AncConvertSensorParamToString(ANC_COMMAND_SENSOR_TYPE type, int32_t param);

#endif
