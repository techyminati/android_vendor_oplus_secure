#ifndef __NAVI_MANAGER_H__
#define __NAVI_MANAGER_H__

#include "navi_def.h"

int navi_init_navigation(navigation_info_t* nav_info);

int navi_do_navigation(navi_ctx_t* navi_ctx);

#endif
