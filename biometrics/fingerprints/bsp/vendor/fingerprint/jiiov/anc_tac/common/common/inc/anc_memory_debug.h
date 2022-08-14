#ifndef __ANC_MEMORY_DEBUG_H__
#define __ANC_MEMORY_DEBUG_H__

#include "anc_type.h"
#include "anc_error.h"

void AncRegisterAllocation(void *p_addr, size_t mem_size,
                           const char *p_func_name, int code_line);
void AncUnRegisterAllocation(void *p_addr);
ANC_RETURN_TYPE AncMemoryCheck();
// call when Ta shutdown, current no call
void AncMemDebugRelease();

#endif
