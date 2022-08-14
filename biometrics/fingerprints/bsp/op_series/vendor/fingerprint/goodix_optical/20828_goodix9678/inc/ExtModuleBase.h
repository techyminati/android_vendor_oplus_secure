/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _EXTMODULEBASE_H_
#define _EXTMODULEBASE_H_

#include "gf_base_types.h"
#include "HalBase.h"

namespace goodix {
    class HalContext;

    typedef void (*ext_module_callback)(int64_t devId, int32_t msgId,
                                        int32_t cmdId, const int8_t *data, uint32_t len);
    class ExtModuleBase {
    public:
        ExtModuleBase() : mpNotify(nullptr) {
        }
        virtual ~ExtModuleBase() {
            mpNotify = nullptr;
        }
        virtual void onInitFinished() {}
        virtual gf_error_t onCommand(int32_t cmdId, const int8_t *in, uint32_t inLen,
                                     int8_t **out, uint32_t *outLen) = 0;
        void inline setNotify(ext_module_callback callback) {
            mpNotify = callback;
        }
        virtual bool isNeedLock(int32_t cmdId) {
            UNUSED_VAR(cmdId);
            return true;
        }
    protected:
        ext_module_callback mpNotify;
    };
};  // namespace goodix


#endif /* _EXTMODULEBASE_H_ */
