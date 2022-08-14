#ifndef __OPLUS_EXTENSION_COMMAND_H__
#define __OPLUS_EXTENSION_COMMAND_H__

#include "anc_hal_manager.h"
#include "anc_error.h"


ANC_RETURN_TYPE OplusExtCommandCbOnTouchDown(AncFingerprintManager *p_manager);
ANC_RETURN_TYPE OplusExtCommandCbOnTouchUp(AncFingerprintManager *p_manager);
ANC_RETURN_TYPE OplusExtensionSetCaliProperty(ANC_BOOL isCali);

ANC_RETURN_TYPE OplusExtensionCommand(AncFingerprintManager *p_manager);


#endif
