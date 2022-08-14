#ifndef __ANC_EXTENSION_H__
#define __ANC_EXTENSION_H__

#include "extension_command.h"
#include "anc_error.h"


#ifdef ANC_GET_IMAGE_FROM_TA
ANC_RETURN_TYPE ExtensionGetImage(uint8_t **pp_buffer, uint32_t *p_buffer_length, int need_save);
#endif
ANC_RETURN_TYPE ExtensionSetCurrentCATime();
ANC_RETURN_TYPE ExtensionGetAuthToken(uint8_t *p_token, uint32_t token_size);

ANC_RETURN_TYPE ExtensionTransmitNoLockSharedBuffer(AncExtensionCommand *p_extension, AncExtensionCommandRespond *p_extension_respond);
ANC_RETURN_TYPE ExtensionTransmit(AncExtensionCommand *p_extension, AncExtensionCommandRespond *p_extension_respond);
ANC_RETURN_TYPE ExtensionLoadCalibration();
#ifdef FP_JIIOV_TEMPLATE_UPDATE_ENABLE
ANC_RETURN_TYPE ExtensionUpdateFile(AncExtensionUpdateFileInfo *p_update_info);
#endif
#endif
