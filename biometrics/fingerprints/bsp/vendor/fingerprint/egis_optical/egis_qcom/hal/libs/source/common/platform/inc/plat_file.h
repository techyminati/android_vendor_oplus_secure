#ifndef __PLAT_FILE_H_
#define __PLAT_FILE_H_

#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#define PLAT_FILE_REMOVE_SUCCESS 0
#define PLAT_FILE_CMD_FAIL -1
#define PLAT_FILE_CMD_INVALID_PARAM -2

#define PLAT_FILE_NOT_EXIST -205
#define PLAT_FILE_OUT_OF_MEMORY -206

/**
 * @brief Save image buffer to a file at the path
 *
 * @return >0 success. negative on failure
 *
 */
int plat_save_raw_image(char* path, unsigned char* pImage, unsigned int width,
			unsigned int height);

/**
 * @brief Load image buffer from a file at the path
 *
 * @return >0 success. negative on failure
 *
 */
int plat_load_raw_image(char* path, unsigned char* pImage, unsigned int width,
			unsigned int height);

int plat_save_file(char* path, unsigned char* buf, unsigned int len);

int plat_load_file(char* path, unsigned char* buf, unsigned int len,
		   unsigned int* real_size);

int plat_remove_file(char* path);

#endif