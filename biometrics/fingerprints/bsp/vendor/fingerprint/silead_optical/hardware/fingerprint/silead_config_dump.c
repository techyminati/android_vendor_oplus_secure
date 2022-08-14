/******************************************************************************
 * @file   silead_config_dump.c
 * @brief  Contains Chip config files dump to .h files.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
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
 * Martin Wu  2018/4/2    0.1.0      Init version
 * Martin Wu  2018/5/7    0.1.1      Support 6150 related configurations.
 * Martin Wu  2018/5/9    0.1.2      Add 6150 deadpix test param
 * Martin Wu  2018/5/10   0.1.3      Add 6150 nav base param
 * Martin Wu  2018/5/11   0.1.4      Add 6150 auto adjust param
 * Martin Wu  2018/5/14   0.1.5      Add finger detect loop mode
 * John Zhang 2018/5/15   0.1.6      Dump config.h file remove line-end space.
 * Martin Wu  2018/6/8    0.1.7      Add ESD related param
 * Martin Wu  2018/6/16   0.1.8      Add OTP related param
 * Martin Wu  2018/6/18   0.1.9      Add cut_size param
 * Martin Wu  2018/6/25   0.2.0      Add optical base & SNR param
 * Martin Wu  2018/6/25   0.2.1      Add optical factory test param
 * Martin Wu  2018/6/26   0.2.2      Add optical middle tone base param
 * Martin Wu  2018/6/27   0.2.3      Add SPI check 0xBF front porch.
 * Martin Wu  2018/6/30   0.2.4      Add distortion & finger_num param.
 * Martin Wu  2018/7/4    0.2.5      Add AEC param.
 * Martin Wu  2018/7/6    0.2.6      Add dead pixel radius.
 * Martin Wu  2018/7/14   0.2.7      Add Auth/Enroll capture image param.
 * Martin Wu  2018/7/20   0.2.8      Add postprocess normalize param.
 * Martin Wu  2018/7/23   0.2.9      Add postprocess remove deadpx param.
 * Martin Wu  2018/7/25   0.3.0      Add R9O03 spd param.
 * Martin Wu  2018/7/30   0.3.1      Add enroll/auth quality param.
 * Martin Wu  2018/8/4    0.3.2      Add post-enroll control param.
 * Martin Wu  2018/8/11   0.3.3      Add icon detect param.
 * Martin Wu  2018/8/26   0.3.4      Add dual auth base param.
 * Martin Wu  2018/8/28   0.3.5      Add dual templates param.
 * Martin Wu  2018/9/5    0.3.6      Add remove slope param.
 * Martin Wu  2018/9/19   0.3.7      Add optic mask param.
 * Martin Wu  2018/9/19   0.3.8      Add optic clip param.
 * Martin Wu  2018/9/27   0.4.0      Universalize capacitor and optics param.
 * Martin Wu  2018/10/18  0.4.1      Add dry/wet template param.
 * Martin Wu  2018/10/28  0.4.2      Combine optic/capacitor param.
 * Martin Wu  2018/10/29  0.4.3      Add dry2wet update threshold param.
 * Martin Wu  2018/11/5   0.4.4      Add artifact detect param.
 * Martin Wu  2018/11/7   0.4.5      Add config file version.
 * Martin Wu  2018/11/15  0.4.6      Add Shading & P Value param.
 * Martin Wu  2018/11/27  0.4.7      Add fingerprint fake param.
 * Martin Wu  2018/12/5   0.4.8      Add factory test black_signal/fov_test/circle_test param.
 * Martin Wu  2018/12/7   0.4.9      Add check same finger mask.
 *
 *****************************************************************************/

#define FILE_TAG "silead_config"
#include "log/logmsg.h"

#ifdef __unused
#undef __unused
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "silead_config.h"

#define FULLNAME_MAX   255
#define FP_CONFIG_INCLUDE_PATH "/data/include"
#define FP_CONFIG_MODULE_DEFAULT "xxxx"
#define FP_CONFIG_MODULE_NAME_OBLITE "board_for_"

#define CONFIG_SET_NAME "cfg"
#define FP_DEV_VER_NAME "dev_ver"

#define STR_TO_UPPER(s, l) \
    do { \
        uint32_t i; \
        for (i = 0; i < l; i++) \
        { \
            if (s[i] >= 'a' && s[i] <= 'z') { \
                s[i] += ('A' - 'a'); \
            } \
        } \
    } while (0)

#define DUMP_INCLUDE_BEGIN(m) \
    do { \
        char module_upper[512] = {0}; \
        snprintf(module_upper, sizeof(module_upper), "%s", m); \
        STR_TO_UPPER(module_upper, strlen(module_upper)); \
        snprintf(buffer, sizeof(buffer), "#ifndef __SILEAD_FP_%s_H__\n", module_upper); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "#define __SILEAD_FP_%s_H__\n", module_upper); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "\n#include \"silead_config.h\"\n"); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_INCLUDE_END(m) \
    do { \
        char module_upper[512] = {0}; \
        snprintf(module_upper, sizeof(module_upper), "%s", m); \
        STR_TO_UPPER(module_upper, strlen(module_upper)); \
        snprintf(buffer, sizeof(buffer), "\n#endif /* __SILEAD_FP_%s_H__ */\n", module_upper); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_VALUE_2(a, b) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{0x%08X, 0x%08X},\n", space, " ", a, b); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_VALUE_3(a, b, c) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{0x%08X, 0x%08X, 0x%08X},\n", space, " ", a, b, c); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_BEGIN(t, name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "\n%s %s_%s = {\n", #t, name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        space += 4; \
    } while (0)

#define DUMP_STRUCT_ARRAY_BEGIN(t, name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "\n%s %s_%s[] = {\n", #t, name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        space += 4; \
    } while (0)

#define DUMP_STRUCT_END() \
    do { \
        space -= 4; \
        snprintf(buffer, sizeof(buffer), "};\n"); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer));\
    } while (0)

#define DUMP_SUB_STRUCT_BEGIN(a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = {\n", space, " ", #a); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        space += 4; \
    } while (0)

#define DUMP_SUB_STRUCT_END() \
    do { \
        space -= 4; \
        snprintf(buffer, sizeof(buffer), "%*s},\n", space, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer));\
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_2(a, b) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = 0x%08X,\n", space, " ", #b, pcfgs->a.b); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_3(a, b, c) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = 0x%08X,\n", space, " ", #c, pcfgs->a.b.c); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_HEX_ARRAY_3(a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = {0x%08X, 0x%08X, 0x%08X},\n", space, " ", #a, pcfgs->a[0], pcfgs->a[1], pcfgs->a[2]); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM(a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = %d,\n", space, " ", #a, pcfgs->a); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_2(a, b) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = %d,\n", space, " ", #b, pcfgs->a.b); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_3(a, b, c) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.%s = %d,\n", space, " ", #c, pcfgs->a.b.c); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_REG_CFG(name, m) \
    if (name != NULL) { \
        snprintf(buffer, sizeof(buffer), "%*sADD_CFG(%s_%s),\n", space, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    }

#define DUMP_STRUCT_PB_PARAM_ITEM(name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{\n", space, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s(int32_t *)%s_%s,\n", space+4, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*sARRAY_SIZE(%s_%s),\n", space+4, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s0,\n", space+4, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s},\n", space, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_DEV_VER_ITEM(name, m) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s(cf_dev_ver_t *)%s_%s,\n", space, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*sARRAY_SIZE(%s_%s),\n", space, " ", name, m); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
        snprintf(buffer, sizeof(buffer), "%*s0,\n", space, " "); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_GAIN(a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.gain = {0x%08X, 0x%08X, 0x%08X, 0x%08X, %d},\n", space, " ", a.v0c, a.v20, a.v2c, a.v24, 0); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_ITEM_GAIN_REG(a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s.gain_reg = {0x%08X, 0x%08X, 0x%08X, 0x%08X, %d},\n", space, " ", a.reg0c, a.reg20, a.reg2c, a.reg24, 0); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

#define DUMP_STRUCT_NAV_GAIN_ITEM(a) \
    do { \
        snprintf(buffer, sizeof(buffer), "%*s{0x%08X, 0x%08X, 0x%08X, 0x%08X, %d},\n", space, " ", a.v0c, a.v20, a.v2c, a.v24, 0); \
        _cfg_dump_to_file(fp, buffer, strlen(buffer)); \
    } while (0)

static int32_t _cfg_dump_mkdir(const char *path)
{
    char dir_name[FULLNAME_MAX];
    int32_t i, len;

    if (path == NULL || !path[0]) {
        LOG_MSG_DEBUG("param invalid");
        return -1;
    }

    strcpy(dir_name, path);
    len = strlen(dir_name);

    if(dir_name[len-1] != '/') {
        strcat(dir_name, "/");
    }

    len = strlen(dir_name);

    for (i = 1; i < len ; i++) {
        if(dir_name[i]=='/') {
            dir_name[i] = 0;
            if( access(dir_name, F_OK) != 0 ) {
                if(mkdir(dir_name, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
                    LOG_MSG_DEBUG("mkdir error (%d:%s)", errno, strerror(errno));
                    return -1;
                }
            }
            dir_name[i] = '/';
        }
    }

    return 0;
}

static int32_t _cfg_dump_get_file_path(char *buffer, uint32_t len, const char *save_path, char *module, int32_t update)
{
    int32_t ret = 0;
    const char *path = save_path;

    if (save_path == NULL || save_path[0] == '\0') {
        path = FP_CONFIG_INCLUDE_PATH;
    }

    ret = _cfg_dump_mkdir(path);
    if (ret >= 0) {
        if (update) {
            snprintf(buffer, len, "%s/%s_upd.h", path, module);
        } else {
            snprintf(buffer, len, "%s/%s.h", path, module);
        }
    }
    return ret;
}

static void _cfg_dump_to_file(FILE *fp, char *buffer, uint32_t len)
{
    if (fp != NULL) {
        fwrite(buffer, sizeof(char), len, fp);
    }
}

static void _cfg_dump_config(FILE *fp, const int32_t idx, const cf_mode_config_t *config, const char *module)
{
    uint32_t i = 0;
    char buffer[128] = {0};
    char name[64] = {0};
    int32_t space = 0;

    if (config != NULL) {
        snprintf(name, sizeof(name), "reg_%s", silfp_cfg_get_config_name(idx));
        DUMP_STRUCT_ARRAY_BEGIN(const cf_register_t, name, module);
        for (i = 0; i < config->len; i++) {
            DUMP_STRUCT_ITEM_HEX_VALUE_2(config->reg[i].addr, config->reg[i].val);
        }
        DUMP_STRUCT_END();
    }
}

static void _cfg_dump_pb_param(FILE *fp, const int32_t idx, const cf_pb_param_t *param, const char *module)
{
    uint32_t i = 0;
    char buffer[512] = {0};
    char value[16] = {0};
    char value2[16] = {0};
    const uint32_t count_per_line = 8;
    int32_t space = 0;

    if (param != NULL) {
        DUMP_STRUCT_ARRAY_BEGIN(const int32_t, silfp_cfg_get_param_name(idx), module);
        for (i = 0; i < param->len; i++) {
            if (i % count_per_line == 0) {
                snprintf(buffer, sizeof(buffer), "    ");
            }
            snprintf(value, sizeof(value), "%d,", param->val[i]);
            if (((i+1) % count_per_line == 0) || (i+1) == param->len) {
                strcat(buffer, (const char *)value);
            } else {
                snprintf(value2, sizeof(value2), "%-*s", 12, value);
                strcat(buffer, (const char *)value2);
            }

            if ((i+1) % count_per_line == 0) {
                strcat(buffer, (const char *)"\n");
                _cfg_dump_to_file(fp, buffer, strlen(buffer));
            }
        }
        if (i % count_per_line != 0) {
            strcat(buffer, (const char *)"\n");
            _cfg_dump_to_file(fp, buffer, strlen(buffer));
        }
        DUMP_STRUCT_END();
    }
}

static void _cfg_dump_dev_ver(FILE *fp, const cf_set_t *pcfgs, const char *module)
{
    uint32_t i = 0;
    char buffer[512] = {0};
    int32_t space = 0;

    if (pcfgs != NULL) {
        DUMP_STRUCT_ARRAY_BEGIN(const cf_dev_ver_t, FP_DEV_VER_NAME, module);
        for (i = 0; i < pcfgs->dev.len; i++) {
            DUMP_STRUCT_ITEM_HEX_VALUE_3(pcfgs->dev.ver[i].id, pcfgs->dev.ver[i].sid, pcfgs->dev.ver[i].vid);
        }
        DUMP_STRUCT_END();
    }
}

void silfp_cfg_dump_data(const cf_set_t *pcfgs, const char *save_path, char *board_module, int32_t update)
{
    int32_t i = 0;
    FILE *fp = NULL;
    char buffer[128] = {0};
    char dump_file_path[256] = {0};
    char *module = NULL;
    int32_t space = 0;

    if (pcfgs == NULL) {
        LOG_MSG_VERBOSE("config NULL");
    } else {
        if (board_module == NULL) {
            module = FP_CONFIG_MODULE_DEFAULT;
        } else {
            if (strncmp(board_module, FP_CONFIG_MODULE_NAME_OBLITE, strlen(FP_CONFIG_MODULE_NAME_OBLITE)) == 0) {
                module = board_module + strlen(FP_CONFIG_MODULE_NAME_OBLITE);
            } else {
                module = board_module;
            }
        }

        if (_cfg_dump_get_file_path(dump_file_path, sizeof(dump_file_path), save_path, module, update) < 0) {
            LOG_MSG_ERROR("get file path failed");
            return;
        }

        LOG_MSG_DEBUG("dump file (%s)", dump_file_path);
        unlink(dump_file_path);
        if ((fp = fopen(dump_file_path, "a+")) == NULL) {
            LOG_MSG_VERBOSE("fail to open dump file (%d:%s)", errno, strerror(errno));
        }

        DUMP_INCLUDE_BEGIN(module);

        for (i = 0; i < CFG_MAX; i++) {
            _cfg_dump_config(fp, i, &pcfgs->cfg[i], module);
        }

        for (i = 0; i < CFG_PB_PARAM_MAX; i ++) {
            _cfg_dump_pb_param(fp, i, &pcfgs->pb.param[i], module);
        }

        _cfg_dump_dev_ver(fp, pcfgs, module);

        DUMP_STRUCT_BEGIN(cf_set_t, CONFIG_SET_NAME, module);

        // main
        DUMP_SUB_STRUCT_BEGIN(dev);
        DUMP_STRUCT_DEV_VER_ITEM(FP_DEV_VER_NAME, module);
        DUMP_SUB_STRUCT_END();
        DUMP_STRUCT_ITEM_HEX_ARRAY_3(mask);

        // common
        DUMP_SUB_STRUCT_BEGIN(common);
        DUMP_STRUCT_ITEM_HEX_2(common, id);
        DUMP_STRUCT_ITEM_HEX_2(common, sid);
        DUMP_STRUCT_ITEM_HEX_2(common, vid);
        DUMP_STRUCT_ITEM_2(common, w);
        DUMP_STRUCT_ITEM_2(common, h);
        DUMP_STRUCT_ITEM_2(common, wp);
        DUMP_STRUCT_ITEM_2(common, hp);
        DUMP_STRUCT_ITEM_2(common, w_hwagc);
        DUMP_STRUCT_ITEM_2(common, h_hwagc);
        DUMP_STRUCT_ITEM_2(common, wc);
        DUMP_STRUCT_ITEM_2(common, hc);
        DUMP_STRUCT_ITEM_2(common, rw);
        DUMP_STRUCT_ITEM_2(common, wdpi);
        DUMP_STRUCT_ITEM_2(common, hdpi);
        DUMP_STRUCT_ITEM_2(common, fg_loop);
        DUMP_STRUCT_ITEM_2(common, ver);
        DUMP_STRUCT_ITEM_GAIN(pcfgs->common.gain);
        DUMP_STRUCT_ITEM_GAIN_REG(pcfgs->common.gain_reg);
        // otp
        DUMP_SUB_STRUCT_BEGIN(otp);
        DUMP_STRUCT_ITEM_HEX_3(common, otp, otp0);
        DUMP_STRUCT_ITEM_HEX_3(common, otp, otp1);
        DUMP_STRUCT_ITEM_HEX_3(common, otp, otp2);
        DUMP_STRUCT_ITEM_HEX_3(common, otp, otp3);
        DUMP_STRUCT_ITEM_HEX_3(common, otp, otp4);
        DUMP_STRUCT_ITEM_HEX_3(common, otp, otp5);
        DUMP_STRUCT_ITEM_HEX_3(common, otp, otp_a0);
        DUMP_SUB_STRUCT_END();
        DUMP_SUB_STRUCT_END();

        // spi
        DUMP_SUB_STRUCT_BEGIN(spi);
        DUMP_STRUCT_ITEM_2(spi, ms_frm);
        DUMP_STRUCT_ITEM_2(spi, retry);
        DUMP_STRUCT_ITEM_2(spi, reinit);
        DUMP_STRUCT_ITEM_2(spi, start);
        DUMP_STRUCT_ITEM_2(spi, msb);
        DUMP_SUB_STRUCT_END();

        // nav
        DUMP_SUB_STRUCT_BEGIN(nav);
        DUMP_STRUCT_ITEM_2(nav, enable);
        DUMP_STRUCT_ITEM_2(nav, mode);
        DUMP_STRUCT_ITEM_2(nav, type);
        DUMP_STRUCT_ITEM_2(nav, con_frame_get_num);
        DUMP_STRUCT_ITEM_2(nav, w);
        DUMP_STRUCT_ITEM_2(nav, h);
        DUMP_STRUCT_ITEM_2(nav, wh);
        DUMP_STRUCT_ITEM_2(nav, hh);
        DUMP_STRUCT_ITEM_2(nav, w_ds);
        DUMP_STRUCT_ITEM_2(nav, h_ds);
        DUMP_STRUCT_ITEM_2(nav, w_hg_ds);
        DUMP_STRUCT_ITEM_2(nav, h_hg_ds);
        DUMP_SUB_STRUCT_BEGIN(gain); // pb.param
        for (i = 0; i < CFG_NAV_AGC_MODE_MAX; i ++) {
            DUMP_STRUCT_NAV_GAIN_ITEM(pcfgs->nav.gain[i]);
        }
        DUMP_SUB_STRUCT_END();
        DUMP_STRUCT_ITEM_2(nav, vk_timeout);
        DUMP_STRUCT_ITEM_2(nav, dclick_gap);
        DUMP_STRUCT_ITEM_2(nav, longpress);
        DUMP_SUB_STRUCT_END();

        // pb
        DUMP_SUB_STRUCT_BEGIN(pb);
        DUMP_SUB_STRUCT_BEGIN(param); // pb.param
        for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
            DUMP_STRUCT_PB_PARAM_ITEM(silfp_cfg_get_param_name(i), module);
        }
        DUMP_SUB_STRUCT_END();
        DUMP_SUB_STRUCT_BEGIN(agc); // pb.agc
        DUMP_STRUCT_ITEM_3(pb, agc, skip_fd);
        DUMP_STRUCT_ITEM_3(pb, agc, fd_threshold);
        DUMP_STRUCT_ITEM_3(pb, agc, skip_small);
        DUMP_STRUCT_ITEM_3(pb, agc, max);
        DUMP_STRUCT_ITEM_3(pb, agc, max_small);
        DUMP_STRUCT_ITEM_3(pb, agc, hwagc_enable);
        DUMP_STRUCT_ITEM_3(pb, agc, hwcov_wake);
        DUMP_STRUCT_ITEM_3(pb, agc, hwcov_tune);
        DUMP_STRUCT_ITEM_3(pb, agc, exp_size);
        DUMP_SUB_STRUCT_END();
        DUMP_SUB_STRUCT_BEGIN(threshold); // pb.threshold
        DUMP_STRUCT_ITEM_3(pb, threshold, alg_select);
        DUMP_STRUCT_ITEM_3(pb, threshold, enrolNum);
        DUMP_STRUCT_ITEM_3(pb, threshold, max_templates_num);
        DUMP_STRUCT_ITEM_3(pb, threshold, templates_size);
        DUMP_STRUCT_ITEM_3(pb, threshold, identify_far_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, update_far_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, enroll_quality_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, enroll_coverage_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, quality_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, coverage_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, skin_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, artificial_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, samearea_detect);
        DUMP_STRUCT_ITEM_3(pb, threshold, samearea_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, samearea_dist);
        DUMP_STRUCT_ITEM_3(pb, threshold, samearea_start);
        DUMP_STRUCT_ITEM_3(pb, threshold, samearea_check_once_num);
        DUMP_STRUCT_ITEM_3(pb, threshold, samearea_check_num_total);
        DUMP_STRUCT_ITEM_3(pb, threshold, dy_fast);
        DUMP_STRUCT_ITEM_3(pb, threshold, segment);
        DUMP_STRUCT_ITEM_3(pb, threshold, water_finger_detect);
        DUMP_STRUCT_ITEM_3(pb, threshold, shake_coe);
        DUMP_STRUCT_ITEM_3(pb, threshold, noise_coe);
        DUMP_STRUCT_ITEM_3(pb, threshold, gray_prec);
        DUMP_STRUCT_ITEM_3(pb, threshold, water_detect_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, fail_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, spd_flag);
        DUMP_STRUCT_ITEM_3(pb, threshold, samefinger_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, identify_epay_threshold);
        DUMP_STRUCT_ITEM_3(pb, threshold, force_up_threshold);
        DUMP_STRUCT_ITEM_HEX_3(pb, threshold, samearea_mask);
        DUMP_SUB_STRUCT_END();
        DUMP_SUB_STRUCT_END();

        // test
        DUMP_SUB_STRUCT_BEGIN(test);
        DUMP_STRUCT_ITEM_2(test, fd_threshold);
        DUMP_STRUCT_ITEM_2(test, deadpx_hard_threshold);
        DUMP_STRUCT_ITEM_2(test, deadpx_norm_threshold);
        DUMP_STRUCT_ITEM_2(test, scut);
        DUMP_STRUCT_ITEM_2(test, detev_ww);
        DUMP_STRUCT_ITEM_2(test, detev_hh);
        DUMP_STRUCT_ITEM_2(test, deteline_h);
        DUMP_STRUCT_ITEM_2(test, deteline_w);
        DUMP_STRUCT_ITEM_2(test, deadpx_max);
        DUMP_STRUCT_ITEM_2(test, badline_max);
        DUMP_STRUCT_ITEM_2(test, finger_detect_mode);
        DUMP_STRUCT_ITEM_HEX_2(test, deadpx_cut);
        DUMP_SUB_STRUCT_END();

        // mmi
        DUMP_SUB_STRUCT_BEGIN(mmi);
        DUMP_STRUCT_ITEM_HEX_2(mmi, dac_min);
        DUMP_STRUCT_ITEM_HEX_2(mmi, dac_max);
        DUMP_STRUCT_ITEM_2(mmi, grey_range_left);
        DUMP_STRUCT_ITEM_2(mmi, grey_range_right);
        DUMP_STRUCT_ITEM_HEX_2(mmi, nav_base_frame_num);
        DUMP_STRUCT_ITEM_2(mmi, max_tune_time);
        DUMP_STRUCT_ITEM_2(mmi, auto_adjust_w);
        DUMP_STRUCT_ITEM_2(mmi, auto_adjust_h);
        DUMP_STRUCT_ITEM_2(mmi, frm_loop_max);
        DUMP_STRUCT_ITEM_HEX_2(mmi, postprocess_ctl);
        DUMP_STRUCT_ITEM_2(mmi, white_base_white_thr);
        DUMP_STRUCT_ITEM_2(mmi, white_base_black_thr);
        DUMP_STRUCT_ITEM_2(mmi, black_base_white_thr);
        DUMP_STRUCT_ITEM_2(mmi, black_base_black_thr);
        DUMP_STRUCT_ITEM_2(mmi, middle_base_white_thr);
        DUMP_STRUCT_ITEM_2(mmi, middle_base_black_thr);
        DUMP_STRUCT_ITEM_2(mmi, diff_base_min_thr);
        DUMP_STRUCT_ITEM_2(mmi, diff_base_max_thr);
        DUMP_STRUCT_ITEM_HEX_2(mmi, snr_cut);
        DUMP_STRUCT_ITEM_2(mmi, base_size);
        DUMP_STRUCT_ITEM_2(mmi, snr_img_num);
        DUMP_STRUCT_ITEM_2(mmi, snr_thr);
        DUMP_STRUCT_ITEM_2(mmi, distortion);
        DUMP_STRUCT_ITEM_2(mmi, finger_num);
        DUMP_STRUCT_ITEM_2(mmi, storage_interval);
        DUMP_STRUCT_ITEM_2(mmi, sum_type);
        DUMP_STRUCT_ITEM_2(mmi, deadpx_radius);
        DUMP_STRUCT_ITEM_2(mmi, cut_radius);
        DUMP_STRUCT_ITEM_2(mmi, normalize_blk);
        DUMP_STRUCT_ITEM_2(mmi, normalize_ratio);
        DUMP_STRUCT_ITEM_HEX_2(mmi, fft_ratio);
        // mmi.touch_info
        DUMP_SUB_STRUCT_BEGIN(touch_info);
        DUMP_STRUCT_ITEM_3(mmi, touch_info, center_x);
        DUMP_STRUCT_ITEM_3(mmi, touch_info, center_y);
        DUMP_STRUCT_ITEM_3(mmi, touch_info, b1_distance_threshold);
        DUMP_STRUCT_ITEM_3(mmi, touch_info, b2_distance_threshold);
        DUMP_STRUCT_ITEM_3(mmi, touch_info, b2_b1_distance_threshold);
        DUMP_STRUCT_ITEM_3(mmi, touch_info, c1_coverage_threshold);
        DUMP_STRUCT_ITEM_3(mmi, touch_info, c2_coverage_threshold);
        DUMP_SUB_STRUCT_END();
        DUMP_SUB_STRUCT_END();

        // pp
        DUMP_SUB_STRUCT_BEGIN(pp);
        DUMP_STRUCT_ITEM_HEX_2(pp, aec_left);
        DUMP_STRUCT_ITEM_HEX_2(pp, aec_right);
        DUMP_STRUCT_ITEM_2(pp, aec_time);
        DUMP_STRUCT_ITEM_2(pp, cal_max_loop);
        DUMP_STRUCT_ITEM_2(pp, dead_a);
        DUMP_STRUCT_ITEM_2(pp, dead_b);
        DUMP_STRUCT_ITEM_HEX_2(pp, quality_cut);
        DUMP_STRUCT_ITEM_2(pp, quality_thr);
        DUMP_STRUCT_ITEM_2(pp, enroll_quality_chk_num);
        DUMP_STRUCT_ITEM_2(pp, enroll_post_num);
        DUMP_STRUCT_ITEM_HEX_2(pp, enroll_post_mask);
        DUMP_STRUCT_ITEM_2(pp, icon_ratio_z);
        DUMP_STRUCT_ITEM_2(pp, icon_ratio_m);
        DUMP_STRUCT_ITEM_2(pp, big_blot_thr);
        DUMP_STRUCT_ITEM_2(pp, duo_tpl_thr);
        DUMP_STRUCT_ITEM_2(pp, tpl_upd_leakage_thr);
        DUMP_STRUCT_ITEM_2(pp, tp_coverage_default);
        DUMP_STRUCT_ITEM_2(pp, slope_len);
        DUMP_STRUCT_ITEM_2(pp, slope_cnt);
        DUMP_STRUCT_ITEM_2(pp, slope_h);
        DUMP_STRUCT_ITEM_2(pp, slope_w);
        DUMP_STRUCT_ITEM_2(pp, cut_angle);
        DUMP_STRUCT_ITEM_2(pp, cut_ud);
        DUMP_STRUCT_ITEM_2(pp, cut_lr);
        DUMP_STRUCT_ITEM_2(pp, dry_sub_cnt);
        DUMP_STRUCT_ITEM_2(pp, after_verify_cnt);
        DUMP_STRUCT_ITEM_2(pp, wts_threshold);
        DUMP_STRUCT_ITEM_2(pp, wts_up_cnt);
        DUMP_STRUCT_ITEM_2(pp, attack_fail_cnt);
        DUMP_STRUCT_ITEM_2(pp, w2d_verify);
        DUMP_STRUCT_ITEM_2(pp, w2d_update);
        DUMP_SUB_STRUCT_END();

        // ft
        DUMP_SUB_STRUCT_BEGIN(ft);
        DUMP_STRUCT_ITEM_2(ft, line_step_min);
        DUMP_STRUCT_ITEM_2(ft, ignore);
        DUMP_STRUCT_ITEM_2(ft, min_theta);
        DUMP_STRUCT_ITEM_2(ft, max_theta);
        DUMP_STRUCT_ITEM_2(ft, quality_thr);
        DUMP_STRUCT_ITEM_2(ft, line_distance_min);
        DUMP_STRUCT_ITEM_2(ft, line_distance_max);
        DUMP_STRUCT_ITEM_HEX_2(ft, cut);
        DUMP_STRUCT_ITEM_2(ft, mask_min1);
        DUMP_STRUCT_ITEM_2(ft, mask_min2);
        DUMP_STRUCT_ITEM_2(ft, mask_min3);
        DUMP_STRUCT_ITEM_2(ft, mask_max1);
        DUMP_STRUCT_ITEM_2(ft, mask_max2);
        DUMP_STRUCT_ITEM_2(ft, mask_max3);
        DUMP_STRUCT_ITEM_2(ft, mask_thr);
        DUMP_STRUCT_ITEM_2(ft, mask_err_thr);
        DUMP_STRUCT_ITEM_2(ft, mask_ex);
        DUMP_STRUCT_ITEM_2(ft, bias_thr);
        DUMP_STRUCT_ITEM_2(ft, bias_err_thr);
        DUMP_STRUCT_ITEM_2(ft, shading_thr);
        DUMP_STRUCT_ITEM_2(ft, shading_unit_thr);
        DUMP_STRUCT_ITEM_2(ft, p_gray_thr);
        DUMP_STRUCT_ITEM_2(ft, p_w_b_thr);
        DUMP_STRUCT_ITEM_2(ft, p_gray_area_thr);
        DUMP_STRUCT_ITEM_2(ft, p_w_b_area_thr);
        DUMP_STRUCT_ITEM_2(ft, dark_percent_thr);
        DUMP_STRUCT_ITEM_2(ft, max_diameter_thr);
        DUMP_STRUCT_ITEM_2(ft, min_diameter_thr);
        DUMP_STRUCT_ITEM_2(ft, circle_thr);
        DUMP_STRUCT_ITEM_2(ft, black_signal_thr);
        DUMP_SUB_STRUCT_END();

        // esd
        DUMP_SUB_STRUCT_BEGIN(esd);
        DUMP_STRUCT_ITEM_2(esd, irq_check);
        DUMP_STRUCT_ITEM_HEX_2(esd, irq_reg);
        DUMP_STRUCT_ITEM_HEX_2(esd, irq_val);
        DUMP_STRUCT_ITEM_HEX_2(esd, int_reg);
        DUMP_STRUCT_ITEM_HEX_2(esd, int_val);
        DUMP_STRUCT_ITEM_HEX_2(esd, int_beacon);
        DUMP_SUB_STRUCT_END();

        // ci
        DUMP_SUB_STRUCT_BEGIN(ci);
        DUMP_STRUCT_ITEM_2(ci, auth_reverse_skip);
        DUMP_STRUCT_ITEM_HEX_2(ci, auth_reverse_grey);
        DUMP_STRUCT_ITEM_2(ci, enroll_loop);
        DUMP_STRUCT_ITEM_2(ci, enroll_skip);
        DUMP_STRUCT_ITEM_2(ci, auth_buf_num);
        DUMP_STRUCT_ITEM_2(ci, artificial_verify_bias);
        DUMP_STRUCT_ITEM_2(ci, artificial_update_bias);
        DUMP_STRUCT_ITEM_2(ci, reserved8);
        DUMP_STRUCT_ITEM_2(ci, fingerprint_spd_score_thr);
        DUMP_STRUCT_ITEM_HEX_2(ci, alg_ctl);
        DUMP_STRUCT_ITEM_2(ci, fake_score);
        DUMP_SUB_STRUCT_END();

        DUMP_SUB_STRUCT_BEGIN(cfg);
        for (i = 0; i < CFG_MAX; i++) {
            DUMP_STRUCT_REG_CFG(silfp_cfg_get_config_name(i), module);
        }
        DUMP_SUB_STRUCT_END();

        DUMP_STRUCT_END();

        DUMP_INCLUDE_END(module);

        fclose(fp);
    }
}
