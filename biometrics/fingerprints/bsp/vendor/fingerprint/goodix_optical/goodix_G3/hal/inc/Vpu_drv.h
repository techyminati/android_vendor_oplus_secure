/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 *Version:
 *Description:
 *History:
 */

#ifndef _VPU_DRV_H_
#define _VPU_DRV_H_

/*---------------------------------------------------------------------------*/
/*  IOCTL Command                                                            */
/*---------------------------------------------------------------------------*/
#define VPU_MAGICNO 'v'

#define VPU_IOCTL_SDSP_POWER_ON     (_IOW(VPU_MAGICNO, 60, int))
#define VPU_IOCTL_SDSP_POWER_OFF    (_IOW(VPU_MAGICNO, 61, int))

#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_SEC_INIT _IOW(MTK_M4U_MAGICNO, 50, int)
#endif  // #ifndef _VPU_DRV_H_