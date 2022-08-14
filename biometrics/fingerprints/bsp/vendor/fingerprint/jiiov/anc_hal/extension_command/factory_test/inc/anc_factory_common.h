#ifndef __ANC_FACTORY_COMMON_H__
#define __ANC_FACTORY_COMMON_H__

#include "anc_error.h"
#include "anc_hal_manager.h"

typedef ANC_RETURN_TYPE (*MMICmdFuncPtr)(AncFingerprintManager *p_manager);
typedef struct {
    int32_t cmd;
    MMICmdFuncPtr exec;
    const char* p_exec_desc;
} CmdExecute;

ANC_RETURN_TYPE GetFactoryImpl(uint32_t type, CmdExecute *exec, uint32_t count);

#endif 