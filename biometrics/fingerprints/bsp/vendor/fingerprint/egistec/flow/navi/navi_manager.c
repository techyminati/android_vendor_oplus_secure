#include "navi_manager.h"
#include "packager.h"
#include "message_handler.h"

int navi_init_navigation(navigation_info_t* nav_info)
{
	unsigned int data_len = sizeof(navigation_info_t);

	return transfer_data(NAVIGATION_TYPE, NAVI_CMD_NAVIGATION_INIT, 0, 0,
			     data_len, (unsigned char*)nav_info, NULL, NULL);
}

int navi_do_navigation(navi_ctx_t* navi_ctx)
{
	unsigned int data_len = sizeof(navi_ctx_t);

	return transfer_data(NAVIGATION_TYPE, NAVI_CMD_NAVIGATION, 0, 0,
			     data_len, (unsigned char*)navi_ctx, &data_len,
			     (unsigned char*)navi_ctx);
}
