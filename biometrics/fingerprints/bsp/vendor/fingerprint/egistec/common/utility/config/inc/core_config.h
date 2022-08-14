#ifndef __CORE_CONFIG_H__
#define __CORE_CONFIG_H__

#include <stdint.h>

#define CONFIG_BUF_TYPE_INI 0

int core_config_create(int buffer_type, const uint8_t* buf, int buf_len);
void core_config_destroy();

int core_config_get_int(const char* section, const char* name, int default_value);

#endif