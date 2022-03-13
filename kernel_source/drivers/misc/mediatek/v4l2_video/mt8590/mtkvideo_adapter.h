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
 * mtkvideo_adapter.h - V4L2 adapter layer for display interface wrapping.
 *
 */

#ifndef __MTKVIDEO_ADAPTER_H
#define __MTKVIDEO_ADAPTER_H

#include "mtkvideo_headers.h"

struct mtkvideo_buf_info;
struct mtkvideo_ovl_param;
struct mtkvideo_drv_param;
struct mtkvideo_vb2_buffer;
enum mtkvideo_v4l2_output_port;

extern struct semaphore mtkfb_0_update_mutex;
extern struct semaphore mtkfb_1_update_mutex;

int mtkvideo_adpt_sync_state(int active);
long mtkvideo_adpt_disp_init(void);
long mtkvideo_adpt_disp_uninit(void);
long mtkvideo_adpt_hdmi_func(unsigned int cmd, unsigned long arg);
long mtkvideo_adpt_cvbs_func(unsigned int cmd, unsigned long arg);
int mtkvideo_adpt_mtkfb_func(unsigned int cmd, unsigned long arg);
long mtkvideo_adpt_mtk_disp(unsigned int cmd, unsigned long arg);
int mtkvideo_adpt_buf_init(struct mtkvideo_buf_info *bi);
int mtkvideo_adpt_buf_uninit(struct mtkvideo_buf_info *bi);
int mtkvideo_adpt_ovl_init(struct mtkvideo_ovl_param *ovl);
int mtkvideo_adpt_ovl_uninit(struct mtkvideo_ovl_param *ovl);

int mtkvideo_adpt_mdp_process(struct mtkvideo_drv_param *p, struct mtkvideo_vb2_buffer *b);
int mtkvideo_adpt_compose(struct mtkvideo_ovl_param *ovl);
int mtkvideo_adpt_send_buf_to_hdmi(struct mtkvideo_ovl_param *p);
int mtkvideo_adpt_send_buf_to_cvbs(struct mtkvideo_ovl_param *p);
int mtkvideo_adpt_send_buf_to_lcd(struct mtkvideo_ovl_param *p);
int mtkvideo_adpt_send_buf_to_lcd_ex(struct mtkvideo_ovl_param *p);
void mtkvideo_buf_before_display_notify(int mva);
void mtkvideo_buf_after_display_notify(int mva);
int mtkvideo_adpt_notify_res_change(enum mtkvideo_v4l2_output_port port, unsigned int w, unsigned int h);
int mtkvideo_adpt_pan_display_done(int fbidx);


#endif /* __MTKVIDEO_ADAPTER_H */
