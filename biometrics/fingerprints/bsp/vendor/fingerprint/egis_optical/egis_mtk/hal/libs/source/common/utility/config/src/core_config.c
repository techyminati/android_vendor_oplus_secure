
#include "egis_definition.h"
#include "egis_sprintf.h"
#include "plat_log.h"
#include "plat_mem.h"
#define LOG_TAG "RBS-config"

#if defined(__ET0XX__) || (defined(USE_CORE_CONFIG_INI) && defined(EGIS_DBG))
#include "core_config.h"
#include "ini_parser.h"
static char* g_ini_string = NULL;

int core_config_create(int buffer_type, const uint8_t* buf, int buf_len) {
    ex_log(LOG_DEBUG,"%s: %d, len=%d", __func__, buffer_type, buf_len);

    if (buf == NULL || buf_len <= 0) {
        ex_log(LOG_ERROR,"%s wrong param %d", __func__, buf_len);
        return EGIS_INCORRECT_PARAMETER;
    }

    PLAT_FREE(g_ini_string);
    g_ini_string = plat_alloc(buf_len);
    if (g_ini_string == NULL) {
        return EGIS_OUT_OF_MEMORY;
    }

    mem_move(g_ini_string, (unsigned char*)buf, buf_len);
    ini_parser_set_string(g_ini_string);

    return EGIS_OK;
}

int core_config_get_buf(int buffer_type, uint8_t* buf, int* buf_len) {
    ex_log(LOG_DEBUG,"%s: %d", __func__, buffer_type);

    if (g_ini_string != NULL) {
        *buf_len = egist_strnlen(g_ini_string, INI_CONFING_FILE_MAX_SIZE);
        ex_log(LOG_DEBUG,"%s: len=%d", __func__, *buf_len);
        mem_move(buf, g_ini_string, *buf_len);
    }

    return EGIS_OK;
}

int core_config_get_int(const char* section, const char* name, int default_value) {
    if (g_ini_string == NULL) {
        return default_value;
    }

    return ini_parser_get_int(section, name, default_value);
}

double core_config_get_double(const char* section, const char* name, double default_value) {
    if (g_ini_string == NULL) {
        return default_value;
    }

    return ini_parser_get_double(section, name, default_value);
}

int core_config_get_string(const char* section, const char* name, char* out_value, int size,
                           const char* default_value) {
    if (g_ini_string == NULL) {
        return 0;
    }

    return ini_parser_get_string(section, name, out_value, size, default_value);
}

void core_config_destroy() {
    PLAT_FREE(g_ini_string);
    ini_parser_set_string(g_ini_string);
}

#else

int core_config_create(int buffer_type, const uint8_t* buf, int buf_len) {
    ex_log(LOG_VERBOSE,"not supported. %d, %p, %d", buffer_type, buf, buf_len);
    return EGIS_OK;
}

int core_config_get_buf(int buffer_type, uint8_t* buf, int* buf_len) {
    ex_log(LOG_VERBOSE,"not supported. %d, %p, %p", buffer_type, buf, buf_len);
    return EGIS_OK;
}

int core_config_get_int(const char* section, const char* name, int default_value) {
    ex_log(LOG_VERBOSE,"not supported. %p, %p", section, name);
    return default_value;
}

double core_config_get_double(const char* section, const char* name, double default_value) {
    ex_log(LOG_VERBOSE,"not supported. %p, %p", section, name);
    return default_value;
}

void core_config_destroy() {}

#endif
