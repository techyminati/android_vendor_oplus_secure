#include <stdarg.h>
#include <string.h>

#ifdef _WINDOWS
#define strlcpy strncpy
#endif

int egist_snprintf(char* str, size_t size, const char* format, ...);
int plat_set_template_path_prefix(char* template_path_prefix, size_t size,
				  const char* path, const char* prefix);
int plat_set_ifaa_path_prefix(char* ifaa_path_prefix, size_t size,
			      const char* path, const char* prefix);
int get_app_ctx_path(char* path, size_t size, const char* app_ctx_path,
		     int index);
int get_version_string(char* data, unsigned int size, const char* lib_version,
		       const char* egis_lib_version, unsigned int minor,
		       unsigned int build);
