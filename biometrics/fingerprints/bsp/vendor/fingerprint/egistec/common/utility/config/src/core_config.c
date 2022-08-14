
#include "egis_definition.h"
#include "plat_mem.h"
#include "ini_definition.h"
#include "plat_log.h"
#define LOG_TAG "RBS-config"

#if defined(USE_CORE_CONFIG_INI) && defined(EGIS_DBG)
#include "core_config.h"
#include "ini_parser.h"
static char* g_ini_string = NULL;

int core_config_create(int buffer_type, const uint8_t* buf, int buf_len)
{
	egislog_d("%s: %d, len=%d", __func__, buffer_type, buf_len);

	if (buf == NULL || buf_len <= 0) {
		egislog_e("%s wrong param %d", __func__, buf_len);
		return EGIS_INCORRECT_PARAMETER;
	}

	PLAT_FREE(g_ini_string);
	g_ini_string = plat_alloc(buf_len);	
	if (g_ini_string == NULL) {
		return EGIS_OUT_OF_MEMORY;
	}	

	memcpy(g_ini_string, buf, buf_len);
	ini_parser_set_string(g_ini_string);

	return EGIS_OK;
}

int core_config_get_int(const char* section, const char* name, int default_value)
{
	// if (g_ini_string == NULL) {
	// 	return default_value;
	// }

	// return ini_parser_get_int(section, name, default_value);
	if (g_ini_string == NULL) {
		return default_value;
	}

	if (memcmp(name, KEY_FINGER_INDEX, 10) != 0)
		return default_value;

	return ini_parser_get_int(section, name, default_value);
}

void core_config_destroy()
{
	PLAT_FREE(g_ini_string);
	ini_parser_set_string(g_ini_string);
}

#else

int core_config_create(int buffer_type, const uint8_t* buf, int buf_len)
{
	if(buffer_type < 0 || NULL == buf || buf_len < 0) {
		egislog_e("%s wrong param", __func__);
		return EGIS_INCORRECT_PARAMETER;
	}

	return EGIS_OK;
}

int core_config_get_int(const char* section, const char* name, int default_value)
{
	if(NULL == section || NULL == name || default_value < 0) {
		egislog_e("%s wrong param", __func__);
		return EGIS_INCORRECT_PARAMETER;
	}

	return default_value;
}

void core_config_destroy() {}

#endif
