/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * mtkvideo_v4l2.h - V4L2 display driver header file.
 *
 */

#ifndef __MTKVIDEO_IMPL_H
#define __MTKVIDEO_IMPL_H


#include "mtkvideo_headers.h"

struct mtkvideo_drv_param;

extern const struct v4l2_ioctl_ops g_mtkvideo_ioctl_ops;
extern const struct v4l2_file_operations g_mtkvideo_fops;
extern const struct vb2_ops g_mtkvideo_qobs;
extern const struct vb2_mem_ops g_mtkvideo_vb2_memops;

int mtkvideo_bh_init(struct mtkvideo_drv_param *p);
int mtkvideo_bh_uninit(void);
void mtkvideo_bh_fb_0_update(unsigned long long t);
void mtkvideo_bh_fb_1_update(unsigned long long t);
void mtkvideo_bh_wakeup(int disp_scenario);
void mtkvideo_bh_mdp_process_wakeup(void);

#endif /* __MTKVIDEO_IMPL_H */
