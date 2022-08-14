/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _QSEEPARAMS_H_
#define _QSEEPARAMS_H_

#include <stdint.h>

namespace goodix {
    typedef struct {
        char goodixTaName[256];
        // multiple path
        const char **goodixTaNamePaths;
        uint32_t goodixTaPathCount;
        char keymasterTaName[256];
        // only one path
        char keymasterTaPath[256];
    } QseeParams;

    void initCustomizedParams(QseeParams *params);
}  // namespace goodix

#endif /* _QSEEPARAMS_H_ */
