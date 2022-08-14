/******************************************************************************
 * @file   silead_config_upd.h
 * @brief  Contains Chip configurations header file.
 *
 *
 * Copyright (c) 2016-2019 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * calvin wang  2018/12/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_CONFIG_UPD_H__
#define __SILEAD_CONFIG_UPD_H__

#define GEN_CFG_UPD_ID(f, a, e)         cfg_upd_id_##f##_##a##_##e
#define GEN_CFG_UPD_ID_2(f, a, b, e)    cfg_upd_id_##f##_##a##_##b##_##e
#define GEN_CFG_UPD_ID_3(f, a, b, c, e) cfg_upd_id_##f##_##a##_##b##_##c##_##e

#define CFG_UPD_MAGIC 0x511EADCF

#define PARAM_UPD_ITEM_DEV_VER_INDEX    0
#define PARAM_UPD_ITEM_MASK_INDEX       1
#define PARAM_UPD_ITEM_INDEX_START      2

#define PARAM_UPD_ID_FROM(f, a, e, v)           GEN_CFG_UPD_ID(f, a, e) = (((v) << 24) & 0xFF000000),
#define PARAM_UPD_ID_FROM_2(f, a, b, e, v)      GEN_CFG_UPD_ID_2(f, a, b, e) = (((v) << 24) & 0xFF000000),
#define PARAM_UPD_ITEM(f, a, e, p, t)           GEN_CFG_UPD_ID(f, a, e),
#define PARAM_UPD_ITEM_2(f, a, b, e, p, t)      GEN_CFG_UPD_ID_2(f, a, b, e),
#define PARAM_UPD_ITEM_3(f, a, b, c, e, p, t)   GEN_CFG_UPD_ID_3(f, a, b, c, e),

enum cfg_upd_id {
#include "silead_config_upd_param.h"
#include "silead_config_upd_config.h"
};

#undef PARAM_UPD_ID_FROM
#undef PARAM_UPD_ID_FROM_2
#undef PARAM_UPD_ITEM
#undef PARAM_UPD_ITEM_2
#undef PARAM_UPD_ITEM_3

typedef struct __attribute__ ((packed)) _dev_ver {
    uint32_t id;
    uint32_t sid;
    uint32_t vid;
} dev_ver_t;

typedef struct __attribute__ ((packed)) _reg_cfg {
    uint32_t addr;
    uint32_t val;
} reg_cfg_t;

#endif /* __SILEAD_CONFIG_UPD_H__ */