/******************************************************************************
 * @file   silead_xml.cpp
 * @brief  Contains XML parse functions.
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

#ifdef __cplusplus
extern "C" {
#endif

#define FILE_TAG "silead_xml"
#define LOG_DBG_VERBOSE 0
#include "log/logmsg.h"

#include "silead_util_ext.h"

#ifdef __cplusplus
}
#endif

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>

#include "tinyxml2.h"
#include "silead_config_upd.h"

using namespace tinyxml2;

#define FP_SYSPARAM_PATH1 "/vendor/etc/silead/sysparms"
#define FP_SYSPARAM_PATH2 "/system/etc/silead/sysparms"
#define FP_SYSPARAM_CONFIG_FILE_NAME "silead_config.xml"
#define FP_SYSPARAM_PARAM_FILE_NAME "silead_param.xml"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define XML_PATH_SEPRATE "/"
#define CHUNK_SIZE 1024

typedef struct xml_parse_item {
    const char *parent_name;
    const char *sub_parent_name;
    const char *third_parent_name;
    const char *item;
    uint32_t id;
} xml_parse_item_t;

#define PARAM_UPD_ID_FROM(f, a, e, v)
#define PARAM_UPD_ID_FROM_2(f, a, b, e, v)
#define PARAM_UPD_ITEM(f, a, e, p, t)           {#a, NULL, NULL, #e, GEN_CFG_UPD_ID(f, a, e)},
#define PARAM_UPD_ITEM_2(f, a, b, e, p, t)      {#a, #b, NULL, #e, GEN_CFG_UPD_ID_2(f, a, b, e)},
#define PARAM_UPD_ITEM_3(f, a, b, c, e, p, t)   {#a, #b, #c, #e, GEN_CFG_UPD_ID_3(f, a, b, c, e)},

xml_parse_item_t m_param_parse_item[] {
#include "silead_config_upd_param.h"
};
xml_parse_item_t m_config_parse_item[] {
#include "silead_config_upd_config.h"
};

#undef PARAM_UPD_ID_FROM
#undef PARAM_UPD_ID_FROM_2
#undef PARAM_UPD_ITEM
#undef PARAM_UPD_ITEM_2

static int32_t _xml_upd_item_array(void **ppbuf, uint32_t *psize, uint32_t *poffset, uint32_t id, void *array, uint32_t len)
{
    uint8_t *p = NULL;
    uint32_t size = 0;
    uint32_t offset = 0;
    uint32_t item_data_size = 0;

    if ((ppbuf == NULL) || (psize == NULL) || (poffset == NULL)) {
        return -1;
    }

    offset = *poffset;
    size = *psize;

    item_data_size = len + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
    if ((*ppbuf == NULL) || (size < offset + item_data_size)) {
        size += ((item_data_size + CHUNK_SIZE - 1) & (~(CHUNK_SIZE - 1)));
        p = (uint8_t *)malloc(size);
        if (p != NULL) {
            memset(p, 0, size);
            if ((*ppbuf) != NULL) {
                memcpy(p, (*ppbuf), offset);
                free(*ppbuf);
            }
            (*ppbuf) = p;
            (*psize) = size;
        } else {
            LOG_MSG_ERROR("buf malloc failed");
        }
    } else {
        p = (uint8_t *)(*ppbuf);
    }

    if (p != NULL) {
        /* item total size */
        memcpy(p + offset, &item_data_size, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        /* item id */
        memcpy(p + offset, &id, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        /* item data size & data */
        memcpy(p + offset, &len, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        if ((array != NULL) && (len > 0)) {
            memcpy(p + offset, array, len);
            offset += len;
        }

        (*poffset) = offset;

        return 0;
    }

    return -1;
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

static int32_t _xml_param_check_dev_ver(const XMLElement *rootElement, void **buf, uint32_t *size, uint32_t *offset, uint32_t cid, uint32_t sid, uint32_t vid, int32_t dump)
{
    int32_t ret = -1;
    uint32_t id[3] = {0};
    uint32_t id_mask[3] = {0};
    uint32_t count = 0;

    uint32_t dev_ver_size = 0;
    dev_ver_t *p_dev_ver = NULL;
    uint32_t dev_ver_count = 0;

    const char *VER_LIST_DELIM = " ";
    const char *VER_ID_DELIM = "_";

    memset(id, 0, sizeof(id));
    memset(id_mask, 0, sizeof(id_mask));

    do {
        if (rootElement == NULL || strcmp("device", rootElement->Name())) {
            break;
        }

        if (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item == NULL) {
            break;
        }

        const XMLAttribute *verAttribute = rootElement->FindAttribute(m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item);
        if (verAttribute == NULL) {
            break;
        }

        char *sVerList = (char *)verAttribute->Value();
        if (sVerList == NULL) {
            break;
        }

        // get id mask
        const XMLElement *element = rootElement;
        if (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name != NULL) {
            element = element->FirstChildElement(m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name);
            if (element == NULL) {
                break;
            }
        }

        if (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name != NULL) {
            element = element->FirstChildElement(m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name);
            if (element == NULL) {
                break;
            }
        }

        if (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item != NULL) {
            element = element->FirstChildElement(m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item);
            if (element == NULL) {
                break;
            }
        }

        char *sVerMask = (char *)element->GetText();
        if (sVerMask == NULL) {
            break;
        }

        count = _xml_str_to_uint32_array(sVerMask, VER_ID_DELIM, id_mask, ARRAY_SIZE(id_mask));
        if (count != ARRAY_SIZE(id_mask)) {
            break;
        }

        if (dump) {
            _xml_upd_item_array(buf, size, offset, m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].id, id_mask, sizeof(id_mask));
            LOG_MSG_VERBOSE("update mask: (%s%s%s%s%s) (0x%08x) %08x:%08x:%08x",
                            (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].parent_name,
                            (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].sub_parent_name,
                            (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].item,
                            m_param_parse_item[PARAM_UPD_ITEM_MASK_INDEX].id, id_mask[0], id_mask[1], id_mask[2]);
        }

        // get id value and check
        char *saveptr = NULL;
        char *sVerValue = strtok_r(sVerList, VER_LIST_DELIM, &saveptr);
        while (sVerValue != NULL) {
            count = _xml_str_to_uint32_array(sVerValue, VER_ID_DELIM, id, ARRAY_SIZE(id));
            if (count == ARRAY_SIZE(id_mask)) {
                if (dump) {
                    if (dev_ver_count + 1 > dev_ver_size) {
                        p_dev_ver = (dev_ver_t *)realloc(p_dev_ver, sizeof(dev_ver_t) * (dev_ver_size + 4));
                        if (p_dev_ver != NULL) {
                            dev_ver_size += 4;
                        } else { /* realloc fail */
                            dev_ver_size = 0;
                            p_dev_ver = 0;
                        }
                    }

                    if (p_dev_ver != NULL) {
                        p_dev_ver[dev_ver_count].id = id[0];
                        p_dev_ver[dev_ver_count].sid = id[1];
                        p_dev_ver[dev_ver_count].vid = id[2];
                        dev_ver_count++;
                    }
                    ret = 0;
                } else {
                    if (((cid & id_mask[0]) == (id[0] & id_mask[0])) && ((sid & id_mask[1]) == (id[1] & id_mask[1]))
                        && ((vid & id_mask[2]) == (id[2] & id_mask[2]))) {
                        ret = 0;
                        break;
                    }
                }
            }

            sVerValue = strtok_r(NULL, VER_LIST_DELIM, &saveptr);
        }
    } while (0);

    if (dump) {
        _xml_upd_item_array(buf, size, offset, m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].id, p_dev_ver, dev_ver_count * sizeof(dev_ver_t));
        LOG_MSG_VERBOSE("update dev_ver: (%s%s%s%s%s) (0x%08x) %d",
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].parent_name,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].sub_parent_name == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].sub_parent_name,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item == NULL) ? "" : XML_PATH_SEPRATE,
                        (m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item == NULL) ? "" : m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].item,
                        m_param_parse_item[PARAM_UPD_ITEM_DEV_VER_INDEX].id, dev_ver_count);

        if (p_dev_ver != NULL) {
            free(p_dev_ver);
        }
    }

    return ret;
}

static int32_t _xml_param_get_int32_array(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, uint32_t index)
{
    uint32_t number = 0;

    if ((element == NULL) || (index > ARRAY_SIZE(m_param_parse_item))) {
        return -1;
    }

    do {
        const XMLAttribute *numAttribute = element->FindAttribute("n");
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

        const char *sParamValues = element->GetText();
        if (sParamValues == NULL) {
            break;
        }

        int32_t *params = (int32_t *)malloc(sizeof(int32_t) * number);
        if (params == NULL) {
            break;
        }

        memset(params, 0, sizeof(int32_t) * number);

        uint32_t count = _xml_str_to_int32_array(sParamValues, params, number);
        if (count != number) {
            LOG_MSG_ERROR("should fix me? (n mismatch, %d but %d)", count, number);
        }

        if (count > 0) {
            _xml_upd_item_array(buf, size, offset, m_param_parse_item[index].id, params, count * sizeof(int32_t));
            LOG_MSG_VERBOSE("update ArrI: (%s%s%s%s%s) (0x%08x) %d",
                            (m_param_parse_item[index].parent_name == NULL) ? "" : m_param_parse_item[index].parent_name,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : m_param_parse_item[index].sub_parent_name,
                            (m_param_parse_item[index].item == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].item == NULL) ? "" : m_param_parse_item[index].item,
                            m_param_parse_item[index].id, count);
        }

        free(params);
    } while (0);

    return 0;
}

static int32_t _xml_param_get_registers(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, uint32_t index)
{
    uint32_t value[2] = {0};
    uint32_t count = 0;
    uint32_t number = 0;

    const char *REGS_LIST_DELIM = "\n";
    const char *REG_ITEM_DELIM = ":";

    if ((element == NULL) || (index > ARRAY_SIZE(m_param_parse_item))) {
        return -1;
    }

    do {
        char *sRegsList = (char *)element->GetText();
        if (sRegsList == NULL) {
            break;
        }

        number = _xml_str_get_character_count(sRegsList, REG_ITEM_DELIM[0]);
        if (number <= 0) {
            break;
        }

        reg_cfg_t *preg = (reg_cfg_t *)malloc(sizeof(reg_cfg_t) * number);
        if (preg == NULL) {
            break;
        }

        memset(preg, 0, sizeof(reg_cfg_t) * number);

        char *saveptr = NULL;
        char *sRegItem = strtok_r(sRegsList, REGS_LIST_DELIM, &saveptr); // get item 0xXXXXXXXX:0xXXXXXXXX
        while (sRegItem != NULL) {
            if (_xml_str_to_uint32_array(sRegItem, REG_ITEM_DELIM, value, ARRAY_SIZE(value)) == ARRAY_SIZE(value)) {
                preg[count].addr = value[0];
                preg[count].val = value[1];
                count++;
            }
            sRegItem = strtok_r(NULL, REGS_LIST_DELIM, &saveptr);
        }

        if (count > 0) {
            _xml_upd_item_array(buf, size, offset, m_param_parse_item[index].id, preg, count * sizeof(reg_cfg_t));
            LOG_MSG_VERBOSE("update ArrUU: (%s%s%s%s%s) (0x%08x) %d",
                            (m_param_parse_item[index].parent_name == NULL) ? "" : m_param_parse_item[index].parent_name,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : m_param_parse_item[index].sub_parent_name,
                            (m_param_parse_item[index].item == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].item == NULL) ? "" : m_param_parse_item[index].item,
                            m_param_parse_item[index].id, count);
        }

        free(preg);
    } while (0);

    return 0;
}

static int32_t _xml_param_get_item(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, uint32_t index)
{
    int32_t valuei32 = 0;
    uint32_t value32 = 0;

    if ((element == NULL) || (index > ARRAY_SIZE(m_param_parse_item))) {
        return -1;
    }

    const char *svalue = element->GetText();
    if (svalue != NULL) {
        if ((strlen(svalue) > 2) && (svalue[0] == '0') && (svalue[1] == 'x')) { // 16bit data
            value32 = _xml_str_to_uint32(svalue, NULL, 16);
            _xml_upd_item_array(buf, size, offset, m_param_parse_item[index].id, &value32, sizeof(uint32_t));
            LOG_MSG_VERBOSE("update: (%s%s%s%s%s) (0x%08x) = 0x%x",
                            (m_param_parse_item[index].parent_name == NULL) ? "" : m_param_parse_item[index].parent_name,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : m_param_parse_item[index].sub_parent_name,
                            (m_param_parse_item[index].item == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].item == NULL) ? "" : m_param_parse_item[index].item,
                            m_param_parse_item[index].id, value32);
        } else {
            valuei32 = _xml_str_to_int32(svalue, NULL, 10);
            _xml_upd_item_array(buf, size, offset, m_param_parse_item[index].id, &valuei32, sizeof(int32_t));
            LOG_MSG_VERBOSE("update: (%s%s%s%s%s) (0x%08x) = %d",
                            (m_param_parse_item[index].parent_name == NULL) ? "" : m_param_parse_item[index].parent_name,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].sub_parent_name == NULL) ? "" : m_param_parse_item[index].sub_parent_name,
                            (m_param_parse_item[index].item == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_param_parse_item[index].item == NULL) ? "" : m_param_parse_item[index].item,
                            m_param_parse_item[index].id, valuei32);
        }
    }

    return 0;
}

static int32_t _xml_param_parse(const XMLElement *rootElement, void **buf, uint32_t *size, uint32_t *offset)
{
    const XMLElement *parent_element = NULL;
    const XMLElement *sub_parent_element = NULL;
    const XMLElement *third_parent_element = NULL;
    const XMLElement *element = NULL;
    const XMLAttribute *type_attribute = NULL;
    char *data_type = NULL;

    uint32_t i = 0;
    int8_t sub_seach = 1;

    for (i = PARAM_UPD_ITEM_INDEX_START; i < ARRAY_SIZE(m_param_parse_item); i++) {
        if (m_param_parse_item[i].item == NULL) {
            continue;
        }

        sub_seach = 1;
        if (m_param_parse_item[i].parent_name == NULL) {
            element = rootElement->FirstChildElement(m_param_parse_item[i].item);
            sub_seach = 0;
        } else {
            if ((parent_element == NULL) || strcmp(m_param_parse_item[i].parent_name, parent_element->Name())) {
                parent_element = rootElement->FirstChildElement(m_param_parse_item[i].parent_name);
            }
            if (parent_element == NULL) {
                continue;
            }
        }

        if (sub_seach) {
            if (m_param_parse_item[i].sub_parent_name == NULL) {
                element = parent_element->FirstChildElement(m_param_parse_item[i].item);
                sub_seach = 0;
            } else {
                if ((sub_parent_element == NULL) || strcmp(m_param_parse_item[i].sub_parent_name, sub_parent_element->Name())) {
                    sub_parent_element = parent_element->FirstChildElement(m_param_parse_item[i].sub_parent_name);
                }
                if (sub_parent_element == NULL) {
                    continue;
                }
            }
        }

        if (sub_seach) {
            if (m_param_parse_item[i].third_parent_name == NULL) {
                element = sub_parent_element->FirstChildElement(m_param_parse_item[i].item);
                sub_seach = 0;
            } else {
                if ((third_parent_element == NULL) || strcmp(m_param_parse_item[i].third_parent_name, third_parent_element->Name())) {
                    third_parent_element = sub_parent_element->FirstChildElement(m_param_parse_item[i].third_parent_name);
                }
                if (third_parent_element == NULL) {
                    continue;
                }
            }
        }

        if (sub_seach) {
            element = third_parent_element->FirstChildElement(m_param_parse_item[i].item);
        }

        if (element != NULL) {
            data_type = NULL;
            type_attribute = element->FindAttribute("st");
            if (type_attribute != NULL) {
                data_type = (char *)type_attribute->Value();
            }

            if (data_type != NULL) {
                if (!strcmp("ArrI", data_type)) {
                    _xml_param_get_int32_array(element, buf, size, offset, i);
                } else if (!strcmp("ArrUU", data_type)) {
                    _xml_param_get_registers(element, buf, size, offset, i);
                } else {
                    data_type = NULL;
                }
            }

            if (data_type == NULL) {
                _xml_param_get_item(element, buf, size, offset, i);
            }
        }
    }

    return 0;
}

static int32_t _xml_param_get_from_xml(const char *dir, const char *module, void **buf, uint32_t *size, uint32_t *offset, uint32_t cid, uint32_t sid, uint32_t vid, int32_t dump)
{
    int32_t ret = -1;
    XMLDocument doc;
    char path[PATH_MAX] = {0};

    do {
        if (dir == NULL) {
            break;
        }

        if (module != NULL) {
            sprintf(path, "%s/%s/%s", dir, module, FP_SYSPARAM_PARAM_FILE_NAME);
        } else {
            sprintf(path, "%s/%s", dir, FP_SYSPARAM_PARAM_FILE_NAME);
        }
        LOG_MSG_VERBOSE("path = %s", path);

        doc.LoadFile(path);

        const XMLElement* rootElement = doc.RootElement();
        if (rootElement == NULL || strcmp("device", rootElement->Name())) {
            break;
        }

        if (_xml_param_check_dev_ver(rootElement, buf, size, offset, cid, sid, vid, dump) < 0) {
            break;
        }

        LOG_MSG_DEBUG("use param file: %s", path);

        _xml_param_parse(rootElement, buf, size, offset);

        ret = 0;
    } while (0);

    return ret;
}

static int32_t _xml_config_get_item(const XMLElement *element, void **buf, uint32_t *size, uint32_t *offset, uint32_t index)
{
    int32_t valuei32 = 0;
    uint32_t value32 = 0;

    if ((element == NULL) || (index > ARRAY_SIZE(m_config_parse_item))) {
        return -1;
    }

    const char *svalue = element->GetText();
    if (svalue != NULL) {
        if ((strlen(svalue) > 2) && (svalue[0] == '0') && (svalue[1] == 'x')) { // 16bit data
            value32 = _xml_str_to_uint32(svalue, NULL, 16);
            _xml_upd_item_array(buf, size, offset, m_config_parse_item[index].id, &value32, sizeof(uint32_t));
            LOG_MSG_VERBOSE("update cfg: (%s%s%s%s%s) (0x%08x) = 0x%x",
                            (m_config_parse_item[index].parent_name == NULL) ? "" : m_config_parse_item[index].parent_name,
                            (m_config_parse_item[index].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_config_parse_item[index].sub_parent_name == NULL) ? "" : m_config_parse_item[index].sub_parent_name,
                            (m_config_parse_item[index].item == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_config_parse_item[index].item == NULL) ? "" : m_config_parse_item[index].item,
                            m_config_parse_item[index].id, value32);
        } else {
            valuei32 = _xml_str_to_int32(svalue, NULL, 10);
            _xml_upd_item_array(buf, size, offset, m_config_parse_item[index].id, &valuei32, sizeof(int32_t));
            LOG_MSG_VERBOSE("update cfg: (%s%s%s%s%s) (0x%08x) = %d",
                            (m_config_parse_item[index].parent_name == NULL) ? "" : m_config_parse_item[index].parent_name,
                            (m_config_parse_item[index].sub_parent_name == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_config_parse_item[index].sub_parent_name == NULL) ? "" : m_config_parse_item[index].sub_parent_name,
                            (m_config_parse_item[index].item == NULL) ? "" : XML_PATH_SEPRATE,
                            (m_config_parse_item[index].item == NULL) ? "" : m_config_parse_item[index].item,
                            m_config_parse_item[index].id, valuei32);
        }
    }

    return 0;
}

static int32_t _xml_config_parse(const XMLElement *rootElement, void **buf, uint32_t *size, uint32_t *offset)
{
    const XMLElement *parent_element = NULL;
    const XMLElement *sub_parent_element = NULL;
    const XMLElement *third_parent_element = NULL;
    const XMLElement *element = NULL;

    uint32_t i = 0;
    int8_t sub_seach = 1;

    for (i = 0; i < ARRAY_SIZE(m_config_parse_item); i++) {
        if (m_config_parse_item[i].item == NULL) {
            continue;
        }


        sub_seach = 1;
        if (m_config_parse_item[i].parent_name == NULL) {
            element = rootElement->FirstChildElement(m_config_parse_item[i].item);
            sub_seach = 0;
        } else {
            if ((parent_element == NULL) || strcmp(m_config_parse_item[i].parent_name, parent_element->Name())) {
                parent_element = rootElement->FirstChildElement(m_config_parse_item[i].parent_name);
            }
            if (parent_element == NULL) {
                continue;
            }
        }

        if (sub_seach) {
            if (m_config_parse_item[i].sub_parent_name == NULL) {
                element = parent_element->FirstChildElement(m_config_parse_item[i].item);
                sub_seach = 0;
            } else {
                if ((sub_parent_element == NULL) || strcmp(m_config_parse_item[i].sub_parent_name, sub_parent_element->Name())) {
                    sub_parent_element = parent_element->FirstChildElement(m_config_parse_item[i].sub_parent_name);
                }
                if (sub_parent_element == NULL) {
                    continue;
                }
            }
        }

        if (sub_seach) {
            if (m_config_parse_item[i].third_parent_name == NULL) {
                element = sub_parent_element->FirstChildElement(m_config_parse_item[i].item);
                sub_seach = 0;
            } else {
                if ((third_parent_element == NULL) || strcmp(m_config_parse_item[i].third_parent_name, third_parent_element->Name())) {
                    third_parent_element = sub_parent_element->FirstChildElement(m_config_parse_item[i].third_parent_name);
                }
                if (third_parent_element == NULL) {
                    continue;
                }
            }
        }

        if (sub_seach) {
            element = third_parent_element->FirstChildElement(m_config_parse_item[i].item);
        }

        if (element != NULL) {
            _xml_config_get_item(element, buf, size, offset, i);
        }
    }

    return 0;
}

static int32_t _xml_config_get_from_xml(const char *dir, const char *module, void **buf, uint32_t *size, uint32_t *offset)
{
    XMLDocument doc;
    char path[PATH_MAX] = {0};

    do {
        if (dir == NULL) {
            break;
        }

        if (module != NULL) {
            sprintf(path, "%s/%s/%s", dir, module, FP_SYSPARAM_CONFIG_FILE_NAME);
        } else {
            sprintf(path, "%s/%s", dir, FP_SYSPARAM_CONFIG_FILE_NAME);
        }
        LOG_MSG_VERBOSE("path = %s", path);

        doc.LoadFile(path);

        const XMLElement* rootElement = doc.RootElement();
        if (rootElement == NULL || strcmp("device", rootElement->Name())) {
            break;
        }

        LOG_MSG_DEBUG("use config file: %s", path);

        _xml_config_parse(rootElement, buf, size, offset);
    } while (0);

    return 0;
}

static int32_t _xml_parase(const char *dir, const char *module, void **buf, uint32_t *size, uint32_t *offset, uint32_t cid, uint32_t sid, uint32_t vid, int32_t dump)
{
    int32_t ret = -1;
    uint8_t *data = NULL;
    uint32_t magic = CFG_UPD_MAGIC;

    if (dir == NULL || module == NULL) {
        LOG_MSG_ERROR("path param invalid");
        return ret;
    }

    if (buf == NULL || size == NULL || offset == NULL) {
        LOG_MSG_ERROR("buf param invalid");
        return ret;
    }

    if (*buf == NULL) {
        *size = CHUNK_SIZE;
        *buf = malloc(*size);
        if (*buf == NULL) {
            LOG_MSG_ERROR("buf malloc failed");
            return ret;
        }
    }

    memset(*buf, 0, *size);
    *offset = sizeof(uint32_t) * 2; // buf size & magic num

    ret = _xml_param_get_from_xml(dir, module, buf, size, offset, cid, sid, vid, dump);
    if (ret >= 0) {
        _xml_config_get_from_xml(dir, module, buf, size, offset);
    }

    if ((ret >= 0) && (*offset > sizeof(uint32_t)) && (*buf != NULL)) {
        ret = (int32_t)*offset;
        data = (uint8_t *)(*buf);
        memcpy(data, offset, sizeof(uint32_t));
        memcpy(data + 4, &magic, sizeof(uint32_t));
    } else {
        ret = -1;
    }

    return ret;
}

extern "C" int32_t silfp_xml_get_sysparams(void **ppbuf, uint32_t cid, uint32_t sid, uint32_t vid)
{
    int32_t ret = -1;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    const char *dirs[] = {FP_SYSPARAM_PATH1, FP_SYSPARAM_PATH2};

    uint32_t i = 0;
    uint32_t size = 0;
    uint32_t offset = 0;

    if ((ppbuf == NULL) || *ppbuf != NULL) {
        LOG_MSG_ERROR("param invalid");
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
            } else if (silfp_util_dir_get_type((char *)dirs[i], pEntry) == 4) { // dir
                ret = _xml_parase(dirs[i], pEntry->d_name, ppbuf, &size, &offset, cid, sid, vid, 0);
                if (ret >= 0) {
                    break;
                }
            }
        }

        closedir(pDir);
        if (ret >= 0) {
            break;
        }
    }

    if (ret < 0) {
        if (*ppbuf != NULL) {
            free(*ppbuf);
            *ppbuf = NULL;
        }
    }

    LOG_MSG_DEBUG("ret = %d, size = %u, offset = %u", ret, size, offset);

    return ret;
}

extern "C" int32_t silfp_xml_dump(const char *dir, const char *module, void **buf, uint32_t *size, uint32_t *offset)
{
    return _xml_parase(dir, module, buf, size, offset, 0, 0, 0, 1);
}