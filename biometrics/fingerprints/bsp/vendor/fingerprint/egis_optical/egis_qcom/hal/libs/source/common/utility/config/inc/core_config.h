#ifndef __CORE_CONFIG_H__
#define __CORE_CONFIG_H__

#include <stdint.h>

#define CONFIG_BUF_TYPE_INI 0
#define INI_CONFING_FILE_MAX_SIZE 10*1024

int core_config_create(int buffer_type, const uint8_t* buf, int buf_len);
void core_config_destroy();

int core_config_get_buf(int buffer_type, uint8_t* buf, int* buf_len);
int core_config_get_int(const char* section, const char* name, int default_value);
double core_config_get_double(const char* section, const char* name, double default_value);
int core_config_get_string(const char* section,const char* name, char* out_value, int size,const char* default_value);

#endif