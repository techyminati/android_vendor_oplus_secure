/******************************************************************************
 * @file   silead_txt.c
 * @brief  Contains Bitmap file operate functions.
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
 * Lyman Xue  2019/1/22   0.1.0      Init version
 *
 *****************************************************************************/
#include "log/logmsg.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "silead_const.h"
#include "silead_error.h"
#include "silead_util.h"
#include "silead_txt.h"

int32_t silfp_txt_get_save_path(char *path, uint32_t len, const char *dir, const char *prefix)
{
	if (path != NULL && len > 0 && dir != NULL) {
		if (prefix != NULL) {
			snprintf(path, len, "%s/%s.txt", dir, prefix);
		} else {
			snprintf(path, len, "%s/default.txt", dir);
		}
		return 0;
	}

	return -SL_ERROR_BAD_PARAMS;
}

static int32_t silfp_txt_status(const char *path)
{
	FILE *fp;
	char ch;

	if ((fp = fopen(path, "r")) != NULL) {
		if ((ch = fgetc(fp)) == SILEADFILE_INITIAL_CHAR) {
			fclose(fp);
			return 0;
		}else {
			fclose(fp);
			return -1;
		}
	}

	return -1;
}

int32_t silfp_txt_save(const char *path, ft_info_t ft, uint32_t step, int32_t reserve)
{
	int32_t ret = 0;
	void *pbuff = NULL;
	char datastr[64] = {0};
	char result[10] = {0};
	int32_t len = 0;
	int32_t s_len = 0;
	int32_t fd = -1;
	int32_t max_extra_len = 300;

	uint64_t second = silfp_util_get_seconds();
	silfp_util_seconds_to_date(second, datastr, sizeof(datastr));

	len = sizeof(ft) + sizeof(step) + sizeof(datastr) + max_extra_len * sizeof(char);

	if (path == NULL) {
		LOG_MSG_ERROR("path invalid");
		return -SL_ERROR_BASE;
	}

	pbuff = malloc(len);
	if (pbuff == NULL) {
		LOG_MSG_ERROR("malloc fail");
		return -SL_ERROR_OUT_OF_MEMORY;
	}

	memset(pbuff, 0, len);
	snprintf(result, sizeof(result), "%s", (reserve >= 0)?"SUCCESS":"FAIL");
	do {
		fd = silfp_util_open_file(path, 1);
		if (fd < 0) {
			break;
		}
		LOG_MSG_INFO("file:%s", path);

		if (silfp_txt_status(path) == 0) {
			s_len = snprintf(pbuff, len, "%s %s : %s %s %s %s %s %s %s %s %s %s %s %s %s\n", \
				"YearMonthDay-Time", "TestResult", "dead_pixels", "circle", "diameter", "mean_w", "mean_b", "p_percent", "p_wb_percent", \
				"noise", "blot", "blot_glass", "status", "shading", "shading_unit");
			ret = silfp_util_write_file(fd, pbuff, s_len);
			if (ret >= 0) {
				memset(pbuff, 0, len);
			}
		}
		/* dead_pixels, circle, diameter, mean_w, mean_b, p_percent, p_wb_percent, noise, blot, blot_glass,
			status, shading, shading_unit*/
		s_len = snprintf(pbuff, len, "%s %s : %d %d %d %d %d %d %d %d %d %d %d %d %d\n", \
			datastr, result, ft.dead_pixels, ft.circle, ft.diameter, ft.mean_w, ft.mean_b, \
			ft.p_percent, ft.p_wb_percent, ft.noise, ft.blot, ft.blot_glass, ft.status, ft.shading, ft.shading_unit);

		ret = silfp_util_write_file(fd, pbuff, s_len);
	} while (0);

	if (fd >= 0) {
		silfp_util_close_file(fd);
	}

	if (pbuff != NULL) {
		free(pbuff);
		pbuff = NULL;
	}
	return 0;
}

