//
//    Copyright 2017 Egis Technology Inc.
//
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Egis in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//
#include "ini_parser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "egis_definition.h"
#include "egis_sprintf.h"
#include "ini.h"
#include "ini_definition.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "plat_std.h"

#define LOG_TAG "RBS-config"

static char* g_ini_string = NULL;

#define VALUE_TYPE_STRING 0
#define VALUE_TYPE_INT 1
#define VALUE_TYPE_DOUBLE 2

struct _iniHelperValue {
    const char* in_setction;
    const char* in_name;
    int in_type;
    BOOL found;

    void* out_value;
    int out_string_max;
};

#ifdef INI_HASH_MAP

#include "hashmap.h"
#ifdef _WINDOWS
#define snprintf _snprintf
#endif

map_t g_map = NULL;

static int _map_ini_handler(void* user, const char* section, const char* name, const char* value) {
    int key_len = MAX_SECTION + MAX_NAME;
    char* map_value = plat_alloc(MAX_VALUE);
    char* map_key = plat_alloc(key_len);

#ifdef QSEE
    egist_snprintf(map_key, key_len, "%s:%s", section, name);
    egist_snprintf(map_value, MAX_VALUE, "%s", value);
#elif __TRUSTONIC__
    strcpy(map_key, section);
    strcat(map_key, ":");
    strcat(map_key, name);
    strcpy(map_value, value);
#else
    snprintf(map_key, key_len, "%s:%s", section, name);
    snprintf(map_value, MAX_VALUE, "%s", value);
#endif

    hashmap_put(g_map, map_key, map_value);
    egislog_d("user %p, put %s ,value %s ", user, map_key, map_value);

    return EGIS_OK;
}

static char* int_get_value(const char* setction, const char* in_name) {
    char* value = NULL;
    int key_len = MAX_SECTION + MAX_NAME;
    char* key = plat_alloc(key_len);
#ifdef QSEE
    egist_snprintf(key, key_len, "%s:%s", setction, in_name);
#elif __TRUSTONIC__
    strcpy(key, setction);
    strcat(key, ":");
    strcat(key, in_name);
#else
    snprintf(key, key_len, "%s:%s", setction, in_name);
#endif
    int error = hashmap_get(g_map, key, (void**)(&value));
    PLAT_FREE(key);

    if (error == 0) return value;

    egislog_d("get %s ,value %s,ret %d", key, value, error);

    return NULL;
}
#else

static void _init_helper_value(struct _iniHelperValue* iniHelperValue, const char* section,
                               const char* name, int data_type, void* out_value,
                               int out_string_max) {
    // egislog_d("%s [%s] %s - type(%d)", __func__, section, name, data_type);
    iniHelperValue->in_setction = section;
    iniHelperValue->in_name = name;
    iniHelperValue->in_type = data_type;
    iniHelperValue->found = FALSE;

    iniHelperValue->out_value = out_value;
    iniHelperValue->out_string_max = out_string_max;
}

static int _ini_handler(void* user, const char* section, const char* name, const char* value) {
    struct _iniHelperValue* pconfig = (struct _iniHelperValue*)user;
    if (pconfig->found) {
        return 1;
    }
    // egislog_d("    %s [%s] %s entry", __func__, section, name);

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH(pconfig->in_setction, pconfig->in_name)) {
        pconfig->found = TRUE;
        switch (pconfig->in_type) {
            case VALUE_TYPE_STRING:
                plat_strncpy((char*)(pconfig->out_value), value, pconfig->out_string_max);
                break;
            case VALUE_TYPE_INT:
                *((int*)pconfig->out_value) = plat_atoi(value);
                break;
            case VALUE_TYPE_DOUBLE:
                *((double*)pconfig->out_value) = plat_atof(value);
                break;
            default:
                egislog_e("!! Error. bad value type %d", pconfig->in_type);
                break;
        }
    }

    return 1;
}
#endif

void check_ini_version() {
    if (g_ini_string != NULL) {
        int version_flag = ini_parser_get_int(INI_SECTION_EGIS, KEY_INI_VERSION, 0);

        if (version_flag != INI_VERSION_FLAG) {
            egislog_e("rbs_config.ini version is %d (expected version is %d)", version_flag,
                      INI_VERSION_FLAG);
        } else {
            egislog_v("rbs_config.ini version %d", version_flag);
        }
    }
}

void ini_parser_set_string(char* string) {
    egislog_d("%s (%p -> %p)", __func__, g_ini_string, string);
    g_ini_string = string;

#ifdef INI_HASH_MAP
    if (g_map != NULL) {
        hashmap_free(g_map);
        g_map = NULL;
    }
    if (g_ini_string != NULL) {
        g_map = hashmap_new();
        ini_parse_string(g_ini_string, _map_ini_handler, NULL);
    }

#endif

    check_ini_version();
}

int ini_parser_get_int(const char* section, const char* name, int default_value) {
    if (g_ini_string == NULL) return default_value;

    int ret_value = default_value;

#ifdef INI_HASH_MAP
    char* value = NULL;
    value = int_get_value(section, name);
    if (value != NULL) ret_value = plat_atoi(value);
#else
    int out_int;
    struct _iniHelperValue helperValue;
    _init_helper_value(&helperValue, section, name, VALUE_TYPE_INT, (void*)&out_int, 0);

    ini_parse_string(g_ini_string, _ini_handler, &helperValue);
    if (helperValue.found) {
        egislog_d("%s [%s] %s => %d", __func__, section, name, out_int);
        return out_int;
    }
#endif
    egislog_d("%s [%s] %s => %d,default %d", __func__, section, name, ret_value, default_value);

    return ret_value;
}

int ini_parser_get_string(const char* section, const char* name, char* out_value, int size,
                          const char* default_value) {
    if (g_ini_string == NULL) {
        return EGIS_CANCEL;
    }
#ifdef INI_HASH_MAP
    char* value = NULL;

    value = int_get_value(section, name);
    if (value == NULL) {
        plat_strncpy((char*)out_value, default_value, size);
    } else {
        plat_strncpy((char*)out_value, value, size);
    }
#else
    struct _iniHelperValue helperValue;
    _init_helper_value(&helperValue, section, name, VALUE_TYPE_STRING, (void*)out_value, size);

    ini_parse_string(g_ini_string, _ini_handler, &helperValue);
    if (helperValue.found) {
        egislog_d("%s [%s] %s => %s\n", __func__, section, name, out_value);
        return 1;
    } else {
        egislog_d("%s [%s] %s (default)=> %s\n", __func__, section, name, default_value);
        plat_strncpy(out_value, default_value, size);
        return EGIS_OK;
    }
#endif
    egislog_d("%s [%s] %s => %s,default %s ", __func__, section, name, out_value, default_value);

    return EGIS_OK;
}

double ini_parser_get_double(const char* section, const char* name, double default_value) {
    if (g_ini_string == NULL) {
        return default_value;
    }

    double ret_value = default_value;

#ifdef INI_HASH_MAP
    char* value = NULL;
    value = int_get_value(section, name);
    if (value != NULL) {
        ret_value = plat_atof(value);
    }
#else
    double out_double;
    struct _iniHelperValue helperValue;
    _init_helper_value(&helperValue, section, name, VALUE_TYPE_DOUBLE, (void*)&out_double, 0);

    ini_parse_string(g_ini_string, _ini_handler, &helperValue);
    if (helperValue.found) {
        egislog_d("%s [%s] %s => %f", __func__, section, name, out_double);
        return out_double;
    }
#endif
    egislog_d("%s [%s] %s => %d (x1000), default %d (x1000)", __func__, section, name,
              (int)(ret_value * 1000), (int)(default_value * 1000));

    return ret_value;
}
