/******************************************************************************
 * @file   silead_xml.cpp
 * @brief  Contains XML parse functions.
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
 * Martin Wu  2018/6/8    0.1.6      Add ESD related param
 * Martin Wu  2018/6/16   0.1.7      Add OTP related param
 * Martin Wu  2018/6/18   0.1.8      Add cut_size param
 * Martin Wu  2018/6/25   0.1.9      Add optical base & SNR param
 * Martin Wu  2018/6/25   0.2.0      Add optical factory test param
 * Martin Wu  2018/6/26   0.2.1      Add optical middle tone base param
 * Martin Wu  2018/6/27   0.2.2      Add SPI check 0xBF front porch.
 * Martin Wu  2018/6/30   0.2.3      Add distortion & finger_num param.
 * Martin Wu  2018/7/4    0.2.4      Add AEC param.
 * Martin Wu  2018/7/6    0.2.5      Add dead pixel radius.
 * Martin Wu  2018/7/14   0.2.6      Add Auth/Enroll capture image param.
 * Martin Wu  2018/7/20   0.2.7      Add postprocess normalize param.
 * Martin Wu  2018/7/23   0.2.8      Add postprocess remove deadpx param.
 * Martin Wu  2018/7/25   0.2.9      Add R9O03 spd param.
 * Martin Wu  2018/7/30   0.3.0      Add enroll/auth quality param.
 * Martin Wu  2018/8/1    0.3.1      Add optic finger detect param.
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
#ifdef __cplusplus
extern "C" {
#endif

#define FILE_TAG "silead_xml"
#define LOG_DBG_VERBOSE 0
#include "log/logmsg.h"

#ifdef __cplusplus
}
#endif

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>

#include "silead_config.h"
#include "tinyxml2.h"

extern "C" {
#include "silead_config_dump.h"
}

using namespace tinyxml2;

#define FP_SYSPARAM_PATH1 "/vendor/etc/silead/sysparms"
#define FP_SYSPARAM_PATH2 "/system/etc/silead/sysparms"
#define FP_SYSPARAM_CONFIG_FILE_NAME "silead_config.xml"
#define FP_SYSPARAM_PARAM_FILE_NAME "silead_param.xml"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

// use typeid to check typeof value: if (typeid(pcfg->a.b) == typeid(int32_t)) {
// #include <typeinfo.h> & -frtti
#define XML_UPD_VALUE_UINT_HEX_2(elm, name, pcfg, a, b, v) \
    if (_xml_update_value_uint32_hex(elm, name, &v) >= 0) { \
        SET_UPD_VALUE_2(pcfg, a, b, v); \
    }
#define XML_UPD_VALUE_UINT_HEX_3(elm, name, pcfg, a, b, c, v) \
    if (_xml_update_value_uint32_hex(elm, name, &v) >= 0) { \
        SET_UPD_VALUE_3(pcfg, a, b, c, v); \
    }
#define XML_UPD_VALUE_GAIN_ITEM_3(elm, name, pcfg, a, b, index, c, v) \
    if (_xml_update_value_uint32_hex(elm, name, &v) >= 0) { \
        SET_UPD_GAIN_ITEM_VALUE_3(pcfg, a, b, index, c, v); \
    }
#define XML_UPD_VALUE_UINT_2(elm, name, pcfg, a, b, v) \
    if (_xml_update_value_uint32(elm, name, &v) >= 0) { \
        SET_UPD_VALUE_2(pcfg, a, b, (typeof(pcfg->a.b))v); \
    }
#define XML_UPD_VALUE_UINT_3(elm, name, pcfg, a, b, c, v) \
    if (_xml_update_value_uint32(elm, name, &v) >= 0) { \
        SET_UPD_VALUE_3(pcfg, a, b, c, (typeof(pcfg->a.b.c))v); \
    }
#define XML_UPD_VALUE_INT_2(elm, name, pcfg, a, b, v) \
    if (_xml_update_value_int32(elm, name, &v) >= 0) { \
        SET_UPD_VALUE_2(pcfg, a, b, (typeof(pcfg->a.b))v); \
    }
#define XML_UPD_VALUE_INT_3(elm, name, pcfg, a, b, c, v) \
    if (_xml_update_value_int32(elm, name, &v) >= 0) { \
        SET_UPD_VALUE_3(pcfg, a, b, c, (typeof(pcfg->a.b.c))v); \
    }
#define XML_UPD_VALUE_FAR_3(elm, name, pcfg, a, b, c, v) \
    if (_xml_update_value_uint32(elm, name, &v) >= 0) { \
        v = _xml_config_uint32_to_far(v); \
        SET_UPD_VALUE_3(pcfg, a, b, c, (typeof(pcfg->a.b.c))v); \
    }

typedef struct _sl_param_mode_map {
    const char *name;
    e_mode_config_t type;
} param_reg_node_map_t;

param_reg_node_map_t modeMap[] = {
    {"normal_mode",          CFG_NORMAL},
    {"data_interrupt_mode",  CFG_DOWN_INT},
    {"leave_interrupt_mode", CFG_UP_INT},
    {"vkey_interrupt_mode",  CFG_VKEY_INT},
    {"nav_read_mode",        CFG_NAV},
    {"hg_nav_read_mode",     CFG_NAV_HG},
    {"ds_nav_read_mode",     CFG_NAV_DS},
    {"hg_ds_nav_read_mode",  CFG_NAV_HG_DS},
    {"hg_coverage_mode",     CFG_HG_COVERAGE},
    {"hwagc_read_mode",      CFG_HWAGC_READ},
    {"partial_read_mode",    CFG_PARTIAL},
    {"full_read_mode",       CFG_FULL},
    {"finger_detect_mode",   CFG_FG_DETECT},
    {"stop_mode",            CFG_STOP},
    {"start_frm_mode",       CFG_FRM_START},
    {"start_hwcov_mode",     CFG_HWCOV_START},
    {"start_hwagc_mode",     CFG_HWAGC_START},
    {"start_ds_mode",        CFG_DS_START},
    {"test_deadpix_mode1",   CFG_DEADPX1},
    {"test_deadpix_mode2",   CFG_DEADPX2},
    {"test_deadpix_mode3",   CFG_DEADPX3},
    {"test_snr_mode",        CFG_SNR},
    {"base_ori_mode",        CFG_BASE_ORI},
    {"auto_adjust_mode",     CFG_AUTO_ADJUST},
    {NULL, CFG_MAX},
};

static e_mode_config_t _xml_param_get_reg_type_by_name(const char *name)
{
    if (name != NULL) {
        for (size_t i = 0; modeMap[i].name != NULL; i++) {
            if (!strcmp(name, modeMap[i].name)) {
                return modeMap[i].type;
            }
        }
    }

    return CFG_MAX;
}

static uint32_t _xml_str_get_character_count(const char *str, const char c)
{
    uint32_t count = 0;
    const char *p = str;

    if (str == NULL) {
        return 0;
    }

    while (*p != 0) {
        if (*p == c) {
            count++;
        }
        p++;
    }

    return count;
}

static uint32_t _xml_str_to_uint32(const char *nptr, char **endptr, int32_t base)
{
    return strtoul(nptr, endptr, base);
}

static uint32_t _xml_str_to_int32(const char *nptr, char **endptr, int32_t base)
{
    return strtol(nptr, endptr, base);
}

static uint32_t _xml_str_to_uint32_array(char *str, const char *delims, uint32_t *value, uint32_t size)
{
    uint32_t count = 0;
    char *saveptr = NULL;

    char *sValue = strtok_r(str, delims, &saveptr);
    while ((sValue != NULL) && (count < size)) {
        if (sValue != NULL) {
            value[count++] = _xml_str_to_uint32(sValue, NULL, 16);
        }

        sValue = strtok_r(NULL, delims, &saveptr);
    }
    return count;
}

static uint32_t _xml_str_to_int32_array(const char *str, int32_t *value, uint32_t size)
{
    uint32_t count = 0;
    const char *p = str;
    char *end = NULL;

    while ((p != NULL) && (*p != '\0') && (count < size)) {
        if ((*p >= '0' && *p <= '9') || (*p == '+') || (*p == '-')) {
            value[count++] = _xml_str_to_int32(p, &end, 10);

            /*while (end != NULL && *end != '\0') {
                if (*end != ' ' || *end != '\t' || *end != '\n') {
                    end++;
                } else {
                    break;
                }
            }*/
            p = end;
        } else {
            p++;
        }
    }

    return count;
}

static int32_t _xml_update_value_int32(const XMLElement *iParentElement, const char *name, int32_t *p)
{
    int32_t ret = -1;
    const XMLElement *iElement = iParentElement->FirstChildElement(name);
    if (iElement != NULL) {
        const char *svalue = iElement->GetText();
        if (svalue != NULL && p != NULL) {
            *p = (int32_t)_xml_str_to_int32(svalue, NULL, 10);
            LOG_MSG_VERBOSE("%s(%d)", name, *p);
            ret = 0;
        }
    }
    return ret;
}

static int32_t _xml_update_value_uint32(const XMLElement *iParentElement, const char *name, uint32_t *p)
{
    int32_t ret = -1;
    const XMLElement *iElement = iParentElement->FirstChildElement(name);
    if (iElement != NULL) {
        const char *svalue = iElement->GetText();
        if (svalue != NULL && p != NULL) {
            *p = _xml_str_to_uint32(svalue, NULL, 10);
            LOG_MSG_VERBOSE("%s(%u)", name, *p);
            ret = 0;
        }
    }
    return ret;
}

static int32_t _xml_update_value_uint32_hex(const XMLElement *iParentElement, const char *name, uint32_t *p)
{
    int32_t ret = -1;
    const XMLElement *iElement = iParentElement->FirstChildElement(name);
    if (iElement != NULL) {
        const char *svalue = iElement->GetText();
        if (svalue != NULL && p != NULL) {
            *p = _xml_str_to_uint32(svalue, NULL, 16);
            LOG_MSG_VERBOSE("%s(%u)", name, *p);
            ret = 0;
        }
    }
    return ret;
}

static uint32_t m_buffer_count = 0;
static cf_dev_ver_t *m_dev_ver = NULL;
static uint32_t m_dev_count = 0;
static int32_t _xml_param_dump_dev_init()
{
    m_buffer_count = 0;
    m_dev_ver = NULL;
    m_dev_count = 0;
    return 0;
}

static int32_t _xml_param_dump_dev_ver(uint32_t id, uint32_t sid, uint32_t vid)
{
    if (m_dev_count + 1 > m_buffer_count) {
        m_dev_ver = (cf_dev_ver_t *)realloc(m_dev_ver, sizeof(cf_dev_ver_t) * (m_buffer_count + 4));
        m_buffer_count += 4;
    }
    if (m_dev_ver != NULL) {
        m_dev_ver[m_dev_count].id = id;
        m_dev_ver[m_dev_count].sid = sid;
        m_dev_ver[m_dev_count].vid = vid;
        m_dev_count++;
    }
    return 0;
}

static int32_t _xml_param_dump_dev_final(cf_set_t* pcfgs)
{
    int32_t ret = 0;
    if (m_dev_ver != NULL) {
        ret = silfp_cfg_dev_ver_update(pcfgs, m_dev_ver, m_dev_count);
        if (!ret) {
            free(m_dev_ver);
        }
    }
    return 0;
}

static int32_t _xml_param_check_dev_ver(const XMLElement *rootElement, cf_set_t* pcfgs, int32_t dump)
{
    int32_t ret = -1;
    uint32_t id[3];
    uint32_t id_mask[3];
    uint32_t count;

    const char *VER_LIST_DELIM = " ";
    const char *VER_ID_DELIM = "_";

    memset(id, 0, sizeof(id));
    memset(id_mask, 0, sizeof(id_mask));

    do {
        if (rootElement == NULL || strcmp("device", rootElement->Name())) {
            break;
        }

        const XMLAttribute *verAttribute = rootElement->FindAttribute("dev_ver");
        if (verAttribute == NULL) {
            break;
        }

        char *sVerList = (char *)verAttribute->Value();
        if (sVerList == NULL) {
            break;
        }

        // get id mask
        const XMLElement *sysparamElement = rootElement->FirstChildElement("SysParam");
        if (sysparamElement == NULL) {
            break;
        }

        const XMLElement *chipidMaskElement = sysparamElement->FirstChildElement("SLMaskChipID");
        if (chipidMaskElement == NULL) {
            break;
        }

        const XMLElement *idMaskElement = chipidMaskElement->FirstChildElement("SLMaskID");
        if (idMaskElement == NULL) {
            break;
        }

        char *sVerMask = (char *)idMaskElement->GetText();
        if (sVerMask == NULL) {
            break;
        }

        count = _xml_str_to_uint32_array(sVerMask, VER_ID_DELIM, id_mask, ARRAY_SIZE(id_mask));
        if (count < ARRAY_SIZE(id_mask) -1) {
            break;
        }

        if (dump) {
            _xml_param_dump_dev_init();
        }

        // get id value and check
        char *saveptr = NULL;
        char *sVerValue = strtok_r(sVerList, VER_LIST_DELIM, &saveptr);
        while (sVerValue != NULL) {
            count = _xml_str_to_uint32_array(sVerValue, VER_ID_DELIM, id, ARRAY_SIZE(id));
            if (count >= ARRAY_SIZE(id_mask) - 1) {
                if (dump) {
                    _xml_param_dump_dev_ver(id[0], id[1], id[2]);
                    memcpy(pcfgs->mask, id_mask, sizeof(id_mask));
                    ret = 0;
                } else {
                    if (((pcfgs->common.id & id_mask[0]) == (id[0] & id_mask[0])) && ((pcfgs->common.sid & id_mask[1]) == (id[1] & id_mask[1]))
                        && ((pcfgs->common.vid & id_mask[2]) == (id[2] & id_mask[2]))) {
                        ret = 0;
                        break;
                    }
                }
            }

            sVerValue = strtok_r(NULL, VER_LIST_DELIM, &saveptr);
        }

        if (dump) {
            _xml_param_dump_dev_final(pcfgs);
        }
    } while (0);

    return ret;
}

static int32_t _xml_get_param_algparam(const XMLElement *iParentElement, cf_set_t* pcfgs, e_pb_param_t type)
{
    uint32_t number;
    static const char *param_name[] = {"C_AlgParmA", "C_AlgParmN", "C_AlgParmC", "C_AlgParmB", "C_AlgParmAp", "C_AlgParmOpt", "C_AlgParmFd"};

    do {
        const XMLElement *algParamElement = iParentElement->FirstChildElement(param_name[type]);
        if (algParamElement == NULL) {
            break;
        }

        const XMLAttribute *numAttribute = algParamElement->FindAttribute("n");
        if (numAttribute == NULL) {
            break;
        }

        const char *sNumber = numAttribute->Value();
        if (sNumber == NULL) {
            break;
        }

        number = _xml_str_to_uint32(sNumber, NULL, 10);
        if (number <= 0) {
            break;
        }

        LOG_MSG_VERBOSE("number:%d", number);

        const char *sParamValues = algParamElement->GetText();
        if (sParamValues == NULL) {
            break;
        }

        int32_t *params = (int32_t*)malloc(sizeof(int32_t) * number);
        if (params == NULL) {
            break;
        }

        memset(params, 0, sizeof(int32_t) * number);

        uint32_t count = _xml_str_to_int32_array(sParamValues, params, number);
        if (count > 0) {
            LOG_MSG_VERBOSE("count:%d", count);
            silfp_cfg_pb_param_update(pcfgs, type, params, count);
        } else {
            free(params);
        }

    } while (0);

    return 0;
}

static int32_t _xml_param_get_spi_config(const XMLElement *spiElement, cf_set_t* pcfgs)
{
    uint32_t value32;
    do {
        if (spiElement == NULL || pcfgs == NULL) {
            break;
        }
        XML_UPD_VALUE_UINT_2(spiElement, "ms_frm", pcfgs, spi, ms_frm, value32);
        XML_UPD_VALUE_UINT_2(spiElement, "retry", pcfgs, spi, retry, value32);
        XML_UPD_VALUE_UINT_2(spiElement, "reinit", pcfgs, spi, reinit, value32);
        XML_UPD_VALUE_UINT_2(spiElement, "start", pcfgs, spi, start, value32);
        XML_UPD_VALUE_UINT_2(spiElement, "msb", pcfgs, spi, msb, value32);
    } while (0);

    return 0;
}

static int32_t _xml_param_get_sysparam(const XMLElement *rootElement, cf_set_t* pcfgs)
{
    int32_t i = 0;
    uint32_t value32;
    int32_t valuei32;
    do {
        const XMLElement *sysparamElement = rootElement->FirstChildElement("SysParam");
        if (sysparamElement == NULL) {
            break;
        }

        const XMLElement *fpApiElement = sysparamElement->FirstChildElement("SLFpApi");
        if (fpApiElement != NULL) {
            XML_UPD_VALUE_UINT_2(fpApiElement, "frame_h", pcfgs, common, h, value32);
            XML_UPD_VALUE_UINT_2(fpApiElement, "frame_w", pcfgs, common, w, value32);
        }

        const XMLElement *algElement = sysparamElement->FirstChildElement("SLAlg");
        if (algElement != NULL) {
            for (i = 0; i < CFG_PB_PARAM_MAX; i++) {
                _xml_get_param_algparam(algElement, pcfgs, (e_pb_param_t)i);
            }

            XML_UPD_VALUE_UINT_2(algElement, "partial_read_w", pcfgs, common, wp, value32);
            XML_UPD_VALUE_UINT_2(algElement, "partial_read_h", pcfgs, common, hp, value32);
            XML_UPD_VALUE_UINT_2(algElement, "hwagc_w", pcfgs, common, w_hwagc, value32);
            XML_UPD_VALUE_UINT_2(algElement, "hwagc_h", pcfgs, common, h_hwagc, value32);
            XML_UPD_VALUE_UINT_2(algElement, "cut_w", pcfgs, common, wc, value32);
            XML_UPD_VALUE_UINT_2(algElement, "cut_h", pcfgs, common, hc, value32);
            XML_UPD_VALUE_UINT_2(algElement, "wdpi", pcfgs, common, wdpi, value32);
            XML_UPD_VALUE_UINT_2(algElement, "hdpi", pcfgs, common, hdpi, value32);
            XML_UPD_VALUE_UINT_2(algElement, "fg_loop", pcfgs, common, fg_loop, value32);

            XML_UPD_VALUE_UINT_2(algElement, "nav_enable", pcfgs, nav, enable, value32);
            XML_UPD_VALUE_UINT_2(algElement, "nav_mode", pcfgs, nav, mode, value32);
            XML_UPD_VALUE_UINT_2(algElement, "nav_read_w", pcfgs, nav, w, value32);
            XML_UPD_VALUE_UINT_2(algElement, "nav_read_h", pcfgs, nav, h, value32);
            XML_UPD_VALUE_UINT_2(algElement, "hg_nav_read_w", pcfgs, nav, wh, value32);
            XML_UPD_VALUE_UINT_2(algElement, "hg_nav_read_h", pcfgs, nav, hh, value32);
            XML_UPD_VALUE_UINT_2(algElement, "ds_nav_read_w", pcfgs, nav, w_ds, value32);
            XML_UPD_VALUE_UINT_2(algElement, "ds_nav_read_h", pcfgs, nav, h_ds, value32);
            XML_UPD_VALUE_UINT_2(algElement, "hg_ds_nav_read_w", pcfgs, nav, w_hg_ds, value32);
            XML_UPD_VALUE_UINT_2(algElement, "hg_ds_nav_read_h", pcfgs, nav, h_hg_ds, value32);
            XML_UPD_VALUE_UINT_2(algElement, "con_frame_get_num", pcfgs, nav, con_frame_get_num, value32);

            XML_UPD_VALUE_UINT_3(algElement, "MaxLoopTime", pcfgs, pb, agc, max_small, value32);
            XML_UPD_VALUE_UINT_3(algElement, "HWAGCFlag", pcfgs, pb, agc, hwagc_enable, value32);
            XML_UPD_VALUE_UINT_3(algElement, "HWCoverageWake", pcfgs, pb, agc, hwcov_wake, value32);
            XML_UPD_VALUE_UINT_3(algElement, "HWCoverageTune", pcfgs, pb, agc, hwcov_tune, value32);
            XML_UPD_VALUE_UINT_3(algElement, "ExpSize", pcfgs, pb, agc, exp_size, value32);
            XML_UPD_VALUE_UINT_3(algElement, "SkipSmall", pcfgs, pb, agc, skip_small, value32);

            if (pcfgs->pb.param[CFG_PB_PARAM_NAVI].val != NULL && pcfgs->pb.param[CFG_PB_PARAM_NAVI].len > 14) {
                SET_UPD_VALUE_2(pcfgs, nav, type, (uint8_t)pcfgs->pb.param[CFG_PB_PARAM_NAVI].val[14]);
            }

            XML_UPD_VALUE_UINT_HEX_2(algElement, "mmi_dac_min", pcfgs, mmi, dac_min, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "mmi_dac_max", pcfgs, mmi, dac_max, value32);
            XML_UPD_VALUE_INT_2(algElement, "mmi_grey_range_left", pcfgs, mmi, grey_range_left, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mmi_grey_range_right", pcfgs, mmi, grey_range_right, valuei32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "nav_base_frame_num", pcfgs, mmi, nav_base_frame_num, value32);
            XML_UPD_VALUE_UINT_2(algElement, "mmi_max_tune_time", pcfgs, mmi, max_tune_time, value32);
            XML_UPD_VALUE_UINT_2(algElement, "auto_adjust_w", pcfgs, mmi, auto_adjust_w, value32);
            XML_UPD_VALUE_UINT_2(algElement, "auto_adjust_h", pcfgs, mmi, auto_adjust_h, value32);
            XML_UPD_VALUE_UINT_2(algElement, "frm_loop_max", pcfgs, mmi, frm_loop_max, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "postprocess_ctl", pcfgs, mmi, postprocess_ctl, value32);
            XML_UPD_VALUE_UINT_2(algElement, "whiteBaseWhiteThr", pcfgs, mmi, white_base_white_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "cen_w_start", pcfgs, mmi, white_base_white_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "whiteBaseBlackThr", pcfgs, mmi, white_base_black_thr, value32); // Combine
            XML_UPD_VALUE_UINT_2(algElement, "cen_h_start", pcfgs, mmi, white_base_black_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "blackBaseWhiteThr", pcfgs, mmi, black_base_white_thr, value32); // Combine
            XML_UPD_VALUE_UINT_2(algElement, "cen_w_ace", pcfgs, mmi, black_base_white_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "blackBaseBlackThr", pcfgs, mmi, black_base_black_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "cen_h_ace", pcfgs, mmi, black_base_black_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "middleBaseWhiteThr", pcfgs, mmi, middle_base_white_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "middleBaseBlackThr", pcfgs, mmi, middle_base_black_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "diffBaseMinThr", pcfgs, mmi, diff_base_min_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "diffBaseMaxThr", pcfgs, mmi, diff_base_max_thr, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "snr_cut", pcfgs, mmi, snr_cut, value32);
            XML_UPD_VALUE_UINT_2(algElement, "base_size", pcfgs, mmi, base_size, value32);
            XML_UPD_VALUE_UINT_2(algElement, "snr_img_num", pcfgs, mmi, snr_img_num, value32);
            XML_UPD_VALUE_INT_2(algElement, "snrThr", pcfgs, mmi, snr_thr, valuei32);
            XML_UPD_VALUE_UINT_2(algElement, "distortion", pcfgs, mmi, distortion, value32);
            XML_UPD_VALUE_UINT_2(algElement, "finger_num", pcfgs, mmi, finger_num, value32);
            XML_UPD_VALUE_UINT_2(algElement, "storage_interval", pcfgs, mmi, storage_interval, value32);
            XML_UPD_VALUE_UINT_2(algElement, "sum_type", pcfgs, mmi, sum_type, value32);
            XML_UPD_VALUE_UINT_2(algElement, "dpx_radius", pcfgs, mmi, deadpx_radius, value32);
            XML_UPD_VALUE_UINT_2(algElement, "cut_radius", pcfgs, mmi, cut_radius, value32);
            XML_UPD_VALUE_UINT_2(algElement, "auth_reverse_skip", pcfgs, ci, auth_reverse_skip, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "auth_reverse_grey", pcfgs, ci, auth_reverse_grey, value32);
            XML_UPD_VALUE_UINT_2(algElement, "enroll_loop", pcfgs, ci, enroll_loop, value32);
            XML_UPD_VALUE_UINT_2(algElement, "enroll_skip", pcfgs, ci, enroll_skip, value32);
            XML_UPD_VALUE_UINT_2(algElement, "auth_buf_num", pcfgs, ci, auth_buf_num, value32);
            XML_UPD_VALUE_UINT_2(algElement, "artificial_verify_bias", pcfgs, ci, artificial_verify_bias, value32);
            XML_UPD_VALUE_UINT_2(algElement, "artificial_update_bias", pcfgs, ci, artificial_update_bias, value32);
            XML_UPD_VALUE_INT_2(algElement, "reserved8", pcfgs, ci, reserved8, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "fingerprint_spd_score_thr", pcfgs, ci, fingerprint_spd_score_thr, valuei32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "algorithm_ctl", pcfgs, ci, alg_ctl, value32);
            XML_UPD_VALUE_INT_2(algElement, "fake_score_Threshold", pcfgs, ci, fake_score, valuei32);

            XML_UPD_VALUE_UINT_HEX_2(algElement, "shutterleft", pcfgs, pp, aec_left, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "shutterright", pcfgs, pp, aec_right, value32);
            XML_UPD_VALUE_UINT_2(algElement, "aec_sram_time", pcfgs, pp, aec_time, value32);
            XML_UPD_VALUE_UINT_2(algElement, "cal_maxlooptime", pcfgs, pp, cal_max_loop, value32);
            XML_UPD_VALUE_UINT_2(algElement, "dead_a", pcfgs, pp, dead_a, value32);
            XML_UPD_VALUE_UINT_2(algElement, "dead_b", pcfgs, pp, dead_b, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "quality_cut", pcfgs, pp, quality_cut, value32);
            XML_UPD_VALUE_INT_2(algElement, "quality_thr", pcfgs, pp, quality_thr, valuei32);
            XML_UPD_VALUE_UINT_2(algElement, "enroll_quality_chk_num", pcfgs, pp, enroll_quality_chk_num, value32);
            XML_UPD_VALUE_UINT_2(algElement, "enroll_post_num", pcfgs, pp, enroll_post_num, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "enroll_post_mask", pcfgs, pp, enroll_post_mask, value32);
            XML_UPD_VALUE_INT_2(algElement, "icon_ratio_z", pcfgs, pp, icon_ratio_z, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "icon_ratio_m", pcfgs, pp, icon_ratio_m, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "big_blot_threshold", pcfgs, pp, big_blot_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "duo_tpl_quality_threshold", pcfgs, pp, duo_tpl_thr, valuei32);
            XML_UPD_VALUE_UINT_2(algElement, "tpl_update_leakage_threshold", pcfgs, pp, tpl_upd_leakage_thr, value32);
            XML_UPD_VALUE_UINT_2(algElement, "tp_coverage_default", pcfgs, pp, tp_coverage_default, value32);
            XML_UPD_VALUE_UINT_2(algElement, "slope_len", pcfgs, pp, slope_len, value32);
            XML_UPD_VALUE_UINT_2(algElement, "slope_cnt", pcfgs, pp, slope_cnt, value32);
            XML_UPD_VALUE_INT_2(algElement, "iCharge_Noise_Threshold_Goal", pcfgs, pp, slope_h, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "slope_h", pcfgs, pp, slope_h, valuei32); // Combine
            XML_UPD_VALUE_INT_2(algElement, "iCharge_Noise_Threshold_Start", pcfgs, pp, slope_w, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "slope_w", pcfgs, pp, slope_w, valuei32); // Combine
            XML_UPD_VALUE_INT_2(algElement, "cut_angle", pcfgs, pp, cut_angle, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "cut_up_down", pcfgs, pp, cut_ud, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "cut_left_right", pcfgs, pp, cut_lr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "dry_sub_cnt", pcfgs, pp, dry_sub_cnt, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "after_verify_cnt", pcfgs, pp, after_verify_cnt, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "wts_threshold", pcfgs, pp, wts_threshold, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "wts_up_cnt", pcfgs, pp, wts_up_cnt, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "attack_fail_cnt", pcfgs, pp, attack_fail_cnt, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "w2d_verify", pcfgs, pp, w2d_verify, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "w2d_update", pcfgs, pp, w2d_update, valuei32);

            XML_UPD_VALUE_UINT_2(algElement, "ft_line_step_min", pcfgs, ft, line_step_min, value32);
            XML_UPD_VALUE_UINT_2(algElement, "ft_ignore", pcfgs, ft, ignore, value32);
            XML_UPD_VALUE_INT_2(algElement, "ft_min_theta", pcfgs, ft, min_theta, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "ft_max_theta", pcfgs, ft, max_theta, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "ft_quality_thr", pcfgs, ft, quality_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "ft_line_distance_min", pcfgs, ft, line_distance_min, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "ft_line_distance_max", pcfgs, ft, line_distance_max, valuei32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "ft_cut", pcfgs, ft, cut, value32);
            XML_UPD_VALUE_INT_2(algElement, "mask_min1", pcfgs, ft, mask_min1, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_min2", pcfgs, ft, mask_min2, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_min3", pcfgs, ft, mask_min3, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_max1", pcfgs, ft, mask_max1, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_max2", pcfgs, ft, mask_max2, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_max3", pcfgs, ft, mask_max3, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_threshold", pcfgs, ft, mask_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_error_threshold", pcfgs, ft, mask_err_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "mask_ex_threshold", pcfgs, ft, mask_ex, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "bias_threshold", pcfgs, ft, bias_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "bias_error_threshold", pcfgs, ft, bias_err_thr, valuei32);

            XML_UPD_VALUE_INT_2(algElement, "shading_threshold", pcfgs, ft, shading_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "shading_unit_threshold", pcfgs, ft, shading_unit_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "pvalue_gray_threshold", pcfgs, ft, p_gray_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "pvalue_white_black_threshold", pcfgs, ft, p_w_b_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "pvalue_gray_area_threshold", pcfgs, ft, p_gray_area_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "pvalue_white_black_area_threshold", pcfgs, ft, p_w_b_area_thr, valuei32);

            XML_UPD_VALUE_INT_2(algElement, "dark_percent_thr", pcfgs, ft, dark_percent_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "max_diameter_thr", pcfgs, ft, max_diameter_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "min_diameter_thr", pcfgs, ft, min_diameter_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "circle_thr", pcfgs, ft, circle_thr, valuei32);
            XML_UPD_VALUE_INT_2(algElement, "black_signal_thr", pcfgs, ft, black_signal_thr, valuei32);

            XML_UPD_VALUE_UINT_2(algElement, "normalize_blk", pcfgs, mmi, normalize_blk, value32);
            XML_UPD_VALUE_UINT_2(algElement, "normalize_ratio", pcfgs, mmi, normalize_ratio, value32);
            XML_UPD_VALUE_UINT_HEX_2(algElement, "fft_ratio", pcfgs, mmi, fft_ratio, value32);

            XML_UPD_VALUE_UINT_3(algElement, "touch_center_x", pcfgs, mmi, touch_info, center_x, value32);
            XML_UPD_VALUE_UINT_3(algElement, "touch_center_y", pcfgs, mmi, touch_info, center_y, value32);
            XML_UPD_VALUE_UINT_3(algElement, "touch_b1_distance_threshold", pcfgs, mmi, touch_info, b1_distance_threshold, value32);
            XML_UPD_VALUE_UINT_3(algElement, "touch_b2_distance_threshold", pcfgs, mmi, touch_info, b2_distance_threshold, value32);
            XML_UPD_VALUE_UINT_3(algElement, "touch_b2_b1_distance_threshold", pcfgs, mmi, touch_info, b2_b1_distance_threshold, value32);
            XML_UPD_VALUE_UINT_3(algElement, "touch_c1_coverage_threshold", pcfgs, mmi, touch_info, c1_coverage_threshold, value32);
            XML_UPD_VALUE_UINT_3(algElement, "touch_c2_coverage_threshold", pcfgs, mmi, touch_info, c2_coverage_threshold, value32);
        }

        const XMLElement *hwElement = sysparamElement->FirstChildElement("HardWare");
        if (hwElement != NULL) {
            const XMLElement *spiElement = hwElement->FirstChildElement("spi");
            if (spiElement != NULL) {
                _xml_param_get_spi_config(spiElement, pcfgs);
            }
        }

    } while (0);

    return 0;
}

static int32_t _xml_param_get_registers(const XMLElement *rootElement, cf_set_t* pcfgs)
{
    uint32_t value[2];
    uint32_t count;
    uint32_t number = 0;

    const char *REGS_LIST_DELIM = "\n";
    const char *REG_ITEM_DELIM = ":";

    const XMLElement *regsElement = rootElement->FirstChildElement("registers");
    if (regsElement == NULL) {
        return 0;
    }

    const XMLElement *regsChild = regsElement->FirstChildElement();
    while (regsChild != NULL) {
        do {
            e_mode_config_t type = _xml_param_get_reg_type_by_name(regsChild->Name());
            if (type < 0 || type >= CFG_MAX) {
                break;
            }

            char *sRegsList = (char *)regsChild->GetText(); // get reg list for each type
            if (sRegsList == NULL) {
                break;
            }

            number = _xml_str_get_character_count(sRegsList, REG_ITEM_DELIM[0]);
            if (number <= 0) {
                break;
            }

            cf_register_t *preg = (cf_register_t*)malloc(sizeof(cf_register_t) * number);
            if (preg == NULL) {
                break;
            }

            memset(preg, 0, sizeof(cf_register_t) * number);

            count = 0;
            char *saveptr = NULL;
            char *sRegItem = strtok_r(sRegsList, REGS_LIST_DELIM, &saveptr); // get item 0xXXXXXXXX:0xXXXXXXXX
            while (sRegItem != NULL) {
                if (_xml_str_to_uint32_array(sRegItem, REG_ITEM_DELIM, value, ARRAY_SIZE(value) ) == ARRAY_SIZE(value)) {
                    preg[count].addr = value[0];
                    preg[count].val = value[1];
                    count++;
                }
                sRegItem = strtok_r(NULL, REGS_LIST_DELIM, &saveptr);
            }

            if (count > 0) {
                silfp_cfg_update(pcfgs, type, preg, count);
            } else {
                free(preg);
            }
        } while (0);

        regsChild = regsChild->NextSiblingElement();
    }

    return 0;
}

static int32_t _xml_param_get_op_registers(const XMLElement *rootElement, cf_set_t* pcfgs)
{
    uint32_t value32 = 0;

    const XMLElement *opRegsElement = rootElement->FirstChildElement("op_registers");
    if (opRegsElement == NULL) {
        return 0;
    }

    do {
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_val_dac", pcfgs, common, gain, v0c, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_val_ag20", pcfgs, common, gain, v20, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_val_ag2c", pcfgs, common, gain, v2c, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_val_ag24", pcfgs, common, gain, v24, value32);

        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_reg_dac", pcfgs, common, gain_reg, reg0c, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_reg_ag20", pcfgs, common, gain_reg, reg20, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_reg_ag2c", pcfgs, common, gain_reg, reg2c, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "fine_tune_reg_ag24", pcfgs, common, gain_reg, reg24, value32);

        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_tune_val_dac", pcfgs, nav, gain, CFG_NAV_AGC_MODE_TUNE, v0c, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_tune_val_ag20", pcfgs, nav, gain, CFG_NAV_AGC_MODE_TUNE, v20, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_tune_val_ag2c", pcfgs, nav, gain, CFG_NAV_AGC_MODE_TUNE, v2c, value32);

        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_hg_val_dac", pcfgs, nav, gain, CFG_NAV_AGC_MODE_HG, v0c, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_hg_val_ag20", pcfgs, nav, gain, CFG_NAV_AGC_MODE_HG, v20, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_hg_val_ag2c", pcfgs, nav, gain, CFG_NAV_AGC_MODE_HG, v2c, value32);

        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_ds_tune_val_dac", pcfgs, nav, gain, CFG_NAV_AGC_MODE_DS, v0c, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_tune_val_ag20", pcfgs, nav, gain, CFG_NAV_AGC_MODE_DS, v20, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_tune_val_ag2c", pcfgs, nav, gain, CFG_NAV_AGC_MODE_DS, v2c, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_ds_tune_val_ag24", pcfgs, nav, gain, CFG_NAV_AGC_MODE_DS, v24, value32);

        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_hg_ds_val_dac", pcfgs, nav, gain, CFG_NAV_AGC_MODE_HG_DS, v0c, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_tune_val_ag20", pcfgs, nav, gain, CFG_NAV_AGC_MODE_HG_DS, v20, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_tune_val_ag2c", pcfgs, nav, gain, CFG_NAV_AGC_MODE_HG_DS, v2c, value32);
        XML_UPD_VALUE_GAIN_ITEM_3(opRegsElement, "navi_ds_tune_val_ag24", pcfgs, nav, gain, CFG_NAV_AGC_MODE_HG_DS, v24, value32);

        XML_UPD_VALUE_UINT_2(opRegsElement, "esd_irq_check", pcfgs, esd, irq_check, value32);
        XML_UPD_VALUE_UINT_HEX_2(opRegsElement, "esd_irq_reg", pcfgs, esd, irq_reg, value32);
        XML_UPD_VALUE_UINT_HEX_2(opRegsElement, "esd_irq_val", pcfgs, esd, irq_val, value32);
        XML_UPD_VALUE_UINT_HEX_2(opRegsElement, "data_int_reg", pcfgs, esd, int_reg, value32);
        XML_UPD_VALUE_UINT_HEX_2(opRegsElement, "data_int_val", pcfgs, esd, int_val, value32);
        XML_UPD_VALUE_UINT_HEX_2(opRegsElement, "data_int_beacon", pcfgs, esd, int_beacon, value32);

        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "otp_reg_otp0", pcfgs, common, otp, otp0, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "otp_reg_otp1", pcfgs, common, otp, otp1, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "otp_reg_otp2", pcfgs, common, otp, otp2, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "otp_reg_otp3", pcfgs, common, otp, otp3, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "otp_reg_otp4", pcfgs, common, otp, otp4, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "otp_reg_otp5", pcfgs, common, otp, otp5, value32);
        XML_UPD_VALUE_UINT_HEX_3(opRegsElement, "otp_a0_val", pcfgs, common, otp, otp_a0, value32);
    } while (0);

    return 0;
}

static int32_t _xml_param_get_from_xml(const char *dir, const char *module, cf_set_t* pcfgs, int32_t dump)
{
    int32_t ret = -1;
    XMLDocument doc;
    char path[PATH_MAX];

    do {
        if (pcfgs == NULL || dir == NULL) {
            break;
        }

        if (module != NULL) {

            sprintf(path, "%s/%s", dir, FP_SYSPARAM_PARAM_FILE_NAME);
        }
        LOG_MSG_VERBOSE("path = %s", path);

        doc.LoadFile(path);

        const XMLElement* rootElement = doc.RootElement();
        if (rootElement == NULL || strcmp("device", rootElement->Name())) {
            break;
        }

        if (_xml_param_check_dev_ver(rootElement, pcfgs, dump) < 0) {
            break;
        }

        LOG_MSG_DEBUG("use param file: %s", path);

        _xml_param_get_sysparam(rootElement, pcfgs);
        _xml_param_get_registers(rootElement, pcfgs);
        _xml_param_get_op_registers(rootElement, pcfgs);

        ret = 0;
    } while (0);

    return ret;
}

static uint32_t _xml_config_uint32_to_far(uint32_t value)
{
    uint32_t i = 0;
    uint32_t count;

    static const uint32_t far[] = {
        1,          2,          5,          10,         20,         50,
        100,        200,        500,        1000,       2000,       5000,
        10000,      20000,      50000,      100000,     200000,     500000,
        1000000,    2000000,    5000000,    10000000,   20000000,   50000000,
        100000000,  200000000,  500000000,  1000000000,
    };

    count = ARRAY_SIZE(far);

    for (i = 0; i < count; i++) {
        if (value <= far[i]) {
            break;
        }
    }

    return i;
}

static int32_t _xml_config_get_pb_config(const XMLElement *algElement, cf_set_t* pcfgs)
{
    uint32_t value32;
    int32_t valuei32;
    do {
        if (algElement == NULL || pcfgs == NULL) {
            break;
        }

        XML_UPD_VALUE_INT_3(algElement, "ialgi", pcfgs, pb, threshold, alg_select, valuei32);
        XML_UPD_VALUE_INT_3(algElement, "iMaxEnrollNum", pcfgs, pb, threshold, enrolNum, valuei32);
        XML_UPD_VALUE_INT_3(algElement, "iMaxTemplateNum", pcfgs, pb, threshold, max_templates_num, valuei32);
        XML_UPD_VALUE_INT_3(algElement, "imax_template_size", pcfgs, pb, threshold, templates_size, valuei32);

        XML_UPD_VALUE_UINT_3(algElement, "ienroll_quality_threshold", pcfgs, pb, threshold, enroll_quality_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "ienroll_coverage_threshold", pcfgs, pb, threshold, enroll_coverage_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "iverify_quality_threshold", pcfgs, pb, threshold, quality_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "iverify_coverage_threshold", pcfgs, pb, threshold, coverage_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "skin_threshold", pcfgs, pb, threshold, skin_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "artificial_threshold", pcfgs, pb, threshold, artificial_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "enroll_same_area", pcfgs, pb, threshold, samearea_detect, value32);
        XML_UPD_VALUE_UINT_3(algElement, "isamearea_dist", pcfgs, pb, threshold, samearea_dist, value32);
        XML_UPD_VALUE_UINT_3(algElement, "iverifyStart", pcfgs, pb, threshold, samearea_start, value32);
        XML_UPD_VALUE_UINT_3(algElement, "idyupdatefast_set", pcfgs, pb, threshold, dy_fast, value32);
        XML_UPD_VALUE_UINT_3(algElement, "isegment_set", pcfgs, pb, threshold, segment, value32);
        XML_UPD_VALUE_UINT_3(algElement, "water_finger_detect", pcfgs, pb, threshold, water_finger_detect, value32);
        XML_UPD_VALUE_UINT_3(algElement, "shake_coe", pcfgs, pb, threshold, shake_coe, value32);
        XML_UPD_VALUE_UINT_3(algElement, "noise_coe", pcfgs, pb, threshold, noise_coe, value32);
        XML_UPD_VALUE_UINT_3(algElement, "gray_prec", pcfgs, pb, threshold, gray_prec, value32);
        XML_UPD_VALUE_UINT_3(algElement, "water_detect_threshold", pcfgs, pb, threshold, water_detect_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "fail_threshold", pcfgs, pb, threshold, fail_threshold, value32);
        XML_UPD_VALUE_UINT_3(algElement, "spd_flag", pcfgs, pb, threshold, spd_flag, value32);

        XML_UPD_VALUE_FAR_3(algElement, "iverify_far_high_threshold", pcfgs, pb, threshold, identify_far_threshold, value32);
        XML_UPD_VALUE_FAR_3(algElement, "iverify_uptem_threshold", pcfgs, pb, threshold, update_far_threshold, value32);
        XML_UPD_VALUE_FAR_3(algElement, "isamearea_verify_threshold", pcfgs, pb, threshold, samearea_threshold, value32);
        XML_UPD_VALUE_FAR_3(algElement, "isamefinger_verify_threshold", pcfgs, pb, threshold, samefinger_threshold, value32);
        XML_UPD_VALUE_FAR_3(algElement, "verify_epay_threshold", pcfgs, pb, threshold, identify_epay_threshold, value32);
        XML_UPD_VALUE_FAR_3(algElement, "force_up_threshold", pcfgs, pb, threshold, force_up_threshold, value32);

        XML_UPD_VALUE_UINT_HEX_3(algElement, "isamearea_chk_mask", pcfgs, pb, threshold, samearea_mask, value32);

        XML_UPD_VALUE_UINT_3(algElement, "isamearea_chk_times_pre_step", pcfgs, pb, threshold, samearea_check_once_num, value32);
        XML_UPD_VALUE_UINT_3(algElement, "isamearea_chk_max_times", pcfgs, pb, threshold, samearea_check_num_total, value32);
    } while (0);

    return 0;
}

static int32_t _xml_config_get_test_config(const XMLElement *algElement, cf_set_t* pcfgs)
{
    uint32_t value32;
    do {
        if (algElement == NULL || pcfgs == NULL) {
            break;
        }

        XML_UPD_VALUE_UINT_2(algElement, "fingerdetectThreshold", pcfgs, test, fd_threshold, value32);
        XML_UPD_VALUE_UINT_2(algElement, "deadpointhardThershold", pcfgs, test, deadpx_hard_threshold, value32);
        XML_UPD_VALUE_UINT_2(algElement, "deadpointnormalThreashold", pcfgs, test, deadpx_norm_threshold, value32);
        XML_UPD_VALUE_UINT_2(algElement, "scut", pcfgs, test, scut, value32);
        XML_UPD_VALUE_UINT_2(algElement, "detev_ww", pcfgs, test, detev_ww, value32);
        XML_UPD_VALUE_UINT_2(algElement, "detev_hh", pcfgs, test, detev_hh, value32);
        XML_UPD_VALUE_UINT_2(algElement, "deteline_h", pcfgs, test, deteline_h, value32);
        XML_UPD_VALUE_UINT_2(algElement, "deteline_w", pcfgs, test, deteline_w, value32);
        XML_UPD_VALUE_UINT_2(algElement, "deadpointmax", pcfgs, test, deadpx_max, value32);
        XML_UPD_VALUE_UINT_2(algElement, "badlinemax", pcfgs, test, badline_max, value32);
        XML_UPD_VALUE_UINT_2(algElement, "deadpoint_finger_detect_mode", pcfgs, test, finger_detect_mode, value32);
        XML_UPD_VALUE_UINT_HEX_2(algElement, "deadpoint_cut", pcfgs, test, deadpx_cut, value32);
    } while (0);

    return 0;
}

static int32_t _xml_config_get_from_xml(const char *dir, const char *module, cf_set_t* pcfgs)
{
    XMLDocument doc;
    char path[PATH_MAX];

    do {
        if (pcfgs == NULL || dir == NULL) {
            break;
        }

        if (module != NULL) {
            sprintf(path, "%s/%s", dir, FP_SYSPARAM_CONFIG_FILE_NAME);
        }
        LOG_MSG_VERBOSE("path = %s", path);

        doc.LoadFile(path);

        const XMLElement* rootElement = doc.RootElement();
        if (rootElement == NULL || strcmp("device", rootElement->Name())) {
            break;
        }

        LOG_MSG_DEBUG("use config file: %s", path);
        const XMLElement *sysParamElement = rootElement->FirstChildElement("SysParam");
        if (sysParamElement == NULL) {
            break;
        }

        const XMLElement *algElement = sysParamElement->FirstChildElement("SLAlg");
        if (algElement != NULL) {
            _xml_config_get_pb_config(algElement, pcfgs);
            _xml_config_get_test_config(algElement, pcfgs);
        }
    } while (0);

    return 0;
}

static int32_t _xml_config_get_default(cf_set_t* pcfgs)
{
    // just for cfg dump, will not update
    if (pcfgs != NULL) {
        pcfgs->common.wdpi = 508;
        pcfgs->common.hdpi = 508;
        pcfgs->common.ver = SILEAD_CONFIG_VER;
        pcfgs->spi.ms_frm = 5;
        pcfgs->spi.retry = 100;
        pcfgs->spi.start = 0;
        pcfgs->nav.vk_timeout = 350;
        pcfgs->nav.dclick_gap = 250;
        pcfgs->nav.longpress = 400;
        pcfgs->pb.agc.skip_fd = 1;
        pcfgs->pb.agc.fd_threshold = 128;
        pcfgs->pb.agc.skip_small = 0;
        pcfgs->pb.agc.max = 5;
        pcfgs->pb.threshold.samearea_check_once_num = 1;
        pcfgs->pb.threshold.samearea_check_num_total = 6;
    }
    return 0;
}

extern "C" int32_t silfp_xml_get_sysparams(cf_set_t *pcfgs)
{
    int32_t ret = -1;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    const char *dirs[] = {FP_SYSPARAM_PATH1, FP_SYSPARAM_PATH2};
    uint32_t i = 0;

    if (pcfgs == NULL) {
        return ret;
    }

    for (i = 0 ; i < ARRAY_SIZE(dirs); i++) {
        pDir = opendir(dirs[i]);
        if (pDir == NULL) {
            continue;
        }

        while((pEntry = readdir(pDir)) != NULL) {
            if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0) {
                continue;
            } else if (pEntry->d_type == 8) { //dir
                ret = _xml_param_get_from_xml(dirs[i], pEntry->d_name, pcfgs, 0);
                if (ret >= 0) {
                    _xml_config_get_from_xml(dirs[i], pEntry->d_name, pcfgs);
                    break;
                }
            }
        }

        closedir(pDir);
        if (ret >= 0) {
            break;
        }
    }

    return ret;
}

static int32_t _xml_dump_check_update(cf_set_t *pcfgs, const char *save_path, char *name)
{
    int32_t len = 0;
    void *buffer = NULL;
    cf_set_t *pcfgs_upd = NULL;

    do {
        len = silfp_cfg_get_update_length(pcfgs);
        if (len <= 0) {
            LOG_MSG_DEBUG("silfp_cfg_get_update_length: %d", len);
            break;
        }

        buffer = malloc(len);
        if (buffer == NULL) {
            LOG_MSG_DEBUG("malloc(%d) failed", len);
            break;
        }

        if(len != silfp_cfg_get_update_buffer(buffer, len, pcfgs)) {
            LOG_MSG_DEBUG("update buffer failed");
            break;
        }

        pcfgs_upd = silfp_cfg_malloc();
        if (pcfgs_upd == NULL) {
            LOG_MSG_DEBUG("malloc pcfgs_upd failed");
            break;
        }

        _xml_config_get_default(pcfgs_upd);
        silfp_cfg_update_config(buffer, len, pcfgs_upd);
        silfp_cfg_dump_data(pcfgs_upd, save_path, name, 1);
    } while (0);

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if (pcfgs_upd != NULL) {
        free(pcfgs_upd);
        pcfgs_upd = NULL;
    }

    return 0;
}

extern "C" int32_t silfp_xml_dump_all_sysparams(void)
{
    int32_t ret = -1;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    cf_set_t *pcfgs = NULL;
    const char *dirs[] = {FP_SYSPARAM_PATH1, FP_SYSPARAM_PATH2};
    uint32_t i = 0;

    for (i = 0 ; i < ARRAY_SIZE(dirs); i++) {
        pDir = opendir(dirs[i]);
        if (pDir == NULL) {
            continue;
        }

        while((pEntry = readdir(pDir)) != NULL) {
            if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0) {
                continue;
            } else if (pEntry->d_type == 4) { //dir
                pcfgs = silfp_cfg_malloc();
                if (pcfgs == NULL) {
                    break;
                }
                _xml_config_get_default(pcfgs);
                ret = _xml_param_get_from_xml(dirs[i], pEntry->d_name, pcfgs, 1);
                if (ret >= 0) {
                    _xml_config_get_from_xml(dirs[i], pEntry->d_name, pcfgs);
                    silfp_cfg_dump_data(pcfgs, NULL, pEntry->d_name, 0);

                    _xml_dump_check_update(pcfgs, NULL, pEntry->d_name);
                }
                silfp_cfg_free(pcfgs);
            }
        }

        closedir(pDir);
    }

    return 0;
}

extern "C" int32_t silfp_xml_dump_sysparams(const char *dir, const char *module, const char *output)
{
    int32_t ret = -1;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    cf_set_t *pcfgs = NULL;
    const char *save_path = output;

    if (dir == NULL) {
        LOG_MSG_ERROR("not set dir");
        return -1;
    }

    if (save_path == NULL) {
        save_path = "./output";
    }

    pDir = opendir(dir);
    if (pDir == NULL) {
        LOG_MSG_ERROR("open dir failed %s", dir);
        return -1;
    }

    while((pEntry = readdir(pDir)) != NULL) {
        if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0) {
            continue;
        } else if (pEntry->d_type == 4) { //dir
            if (module != NULL) {
                if (strcmp(pEntry->d_name, module) != 0) {
                    continue;
                }
            }

            pcfgs = silfp_cfg_malloc();
            if (pcfgs == NULL) {
                break;
            }
            _xml_config_get_default(pcfgs);
            ret = _xml_param_get_from_xml(dir, pEntry->d_name, pcfgs, 1);
            if (ret >= 0) {
                _xml_config_get_from_xml(dir, pEntry->d_name, pcfgs);
                silfp_cfg_dump_data(pcfgs, save_path, pEntry->d_name, 0);

                _xml_dump_check_update(pcfgs, save_path, pEntry->d_name);

                if (module != NULL) {
                    break;
                }
            }
            silfp_cfg_free(pcfgs);
        }
    }

    closedir(pDir);

    return 0;
}

