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

#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>

#include <mach/mt_typedefs.h>


#include "tve_fb_ctrl.h"
#include "tve_drv.h"
#include "disp_drv_platform.h"



//void DBG_Init(void);
//void DBG_Deinit(void);

//extern void hdmi_log_enable(int enable);
//extern void hdmi_cable_fake_plug_in(void);
//extern void hdmi_cable_fake_plug_out(void);
//extern void hdmi_mmp_enable(int enable);


// ---------------------------------------------------------------------------
//  External variable declarations
// ---------------------------------------------------------------------------

//extern LCM_DRIVER *lcm_drv;
// ---------------------------------------------------------------------------
//  Debug Options
// ---------------------------------------------------------------------------


static char STR_HELP[] =
    "\n"
    "USAGE\n"
    "        echo [ACTION]... > tve\n"
    "\n"
    "ACTION\n"
    "        tve:[on|off]\n"
    "             enable tve video output\n"
    "\n";

//extern void hdmi_log_enable(int enable);
extern void tve_draw_color_bar_ex(enum RDMA1_COLOR_BAR_TYPE rdma1_cb_fmt);
extern void cvbs_state_callback(CVBS_STATE state);
extern int tve_test_cmd(int psInput);
extern void TveSetFmt(int fmt);
extern void tve_set_dump_on(unsigned dump_on);

#ifdef MTK_FOR_CVBS_DITHERING
extern void tve_set_filter_on(unsigned filter_on);
#endif


// TODO: this is a temp debug solution
//extern void hdmi_cable_fake_plug_in(void);
//extern int hdmi_drv_init(void);
static void process_dbg_opt(const char *opt)
{
    if (0 == strncmp(opt, "callback:", 9))
    {
    	CVBS_STATE state = CVBS_STATE_NO_DEVICE;
		
		if (0 == strncmp(opt + 9, "in", 2))
		{
			state = CVBS_STATE_ACTIVE;
		}
		else if (0 == strncmp(opt + 9, "out", 3))
		{
			state = CVBS_STATE_NO_DEVICE;
		}
		
		cvbs_state_callback( state);
    }
    else if (0 == strncmp(opt, "test:", 5))
    {
        char *p = (char *)opt + 5;
        int psInput = (int) simple_strtoul(p, &p, 10);
        tve_test_cmd(psInput);
    }
    else if (0 == strncmp(opt, "fmt", 3))
    {
        // 2 480p, 3 576p
        char *p = (char *)opt + 3;
        int fmt = (int) simple_strtoul(p, &p, 10);
        TveSetFmt(fmt);
    }
    else if (0 == strncmp(opt, "dump_on:", 8))
    {
        char *p = (char *)opt + 8;
        int dump_on = (int) simple_strtoul(p, &p, 10);
        tve_set_dump_on(dump_on);
    }
    #ifdef MTK_FOR_CVBS_DITHERING
    else if(0 == strncmp(opt, "filter_on:", 10))
    {
        char *p = (char *)opt + 10;
        int filter_on = (int) simple_strtoul(p, &p, 10);
        tve_set_filter_on(filter_on);
    }
    #endif
    else if (0 == strncmp(opt, "suspend", 7))
    {
        //hdmi_suspend();
    }
    else if (0 == strncmp(opt, "resume", 6))
    {
        //hdmi_resume();
    }
    else if (0 == strncmp(opt, "colorbar:", 9))
    {
    	enum RDMA1_COLOR_BAR_TYPE rdma1_cb_fmt = RDMA1_COLOR_BAR_UYUV ;

		if (0 == strncmp(opt + 9, "yuyv", 3))
        {
           rdma1_cb_fmt = RDMA1_COLOR_BAR_YUYV;
        }
        else if (0 == strncmp(opt + 9, "uyvy", 3))
        {
           rdma1_cb_fmt = RDMA1_COLOR_BAR_UYUV ;
        }
		else if (0 == strncmp(opt + 9, "rgb", 3))
        {
           rdma1_cb_fmt = RDMA1_COLOR_BAR_RGB ;
        }
        else if (0 == strncmp(opt + 9, "bgr", 3))
        {
           rdma1_cb_fmt = RDMA1_COLOR_BAR_BGR ;
        }
        else
        {
            goto Error;
        }
		
		tve_draw_color_bar_ex( rdma1_cb_fmt);
    }
    else if (0 == strncmp(opt, "ldooff", 6))
    {

    }
    else if (0 == strncmp(opt, "log:", 4))
    {
        if (0 == strncmp(opt + 4, "on", 2))
        {
           // hdmi_log_enable(true);
        }
        else if (0 == strncmp(opt + 4, "off", 3))
        {
           // hdmi_log_enable(false);
        }
        else
        {
            goto Error;
        }
    }
    else if (0 == strncmp(opt, "fakecablein:", 12))
    {
        if (0 == strncmp(opt + 12, "enable", 6))
        {
           // hdmi_cable_fake_plug_in();
        }
        else if (0 == strncmp(opt + 12, "disable", 7))
        {
           // hdmi_cable_fake_plug_out();
        }
        else
        {
            goto Error;
        }
    }

    else if (0 == strncmp(opt, "hdmimmp:", 8))
    {
        if (0 == strncmp(opt + 8, "on", 2))
        {
           // hdmi_mmp_enable(1);
        }
        else if (0 == strncmp(opt + 8, "off", 3))
        {
           // hdmi_mmp_enable(0);
        }
        else if (0 == strncmp(opt + 8, "img", 3))
        {
           // hdmi_mmp_enable(7);
        }
        else
        {
            goto Error;
        }
    }
    else if(
        (0 == strncmp(opt, "dbgtype:", 8))||
        (0 == strncmp(opt, "w:", 2))||
        (0 == strncmp(opt, "r:", 2))||
        (0 == strncmp(opt, "status", 6))||
        (0 == strncmp(opt, "help", 4))||
        (0 == strncmp(opt, "dpicb:", 6))||
        (0 == strncmp(opt, "tvecb:", 6))||
        (0 == strncmp(opt, "mipipll", 7))||
        (0 == strncmp(opt, "tvdpll", 6))||
        (0 == strncmp(opt, "pal", 3))||
         (0 == strncmp(opt, "cps:", 4))||
          (0 == strncmp(opt, "asp:", 4))||
        (0 == strncmp(opt, "ntsc", 4))
        )
    {
        tve_driver_debug((char *)opt);
    }    
    else
    {
        goto Error;
    }

    return;

Error:
    printk("[cvbs] parse command error!\n\n%s", STR_HELP);
}

static void process_dbg_cmd(char *cmd)
{
    char *tok;

    printk("[cvbs] %s\n", cmd);

    while ((tok = strsep(&cmd, " ")) != NULL)
    {
        process_dbg_opt(tok);
    }
}

// ---------------------------------------------------------------------------
//  Debug FileSystem Routines
// ---------------------------------------------------------------------------

struct dentry *tve_dbgfs = NULL;


static ssize_t tve_debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;
    return 0;
}


static char tve_debug_buffer[4096];

static ssize_t tve_debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    int n = 0;
    n = strlen(tve_debug_buffer);
    tve_debug_buffer[n++] = 0;

    return simple_read_from_buffer(ubuf, count, ppos, tve_debug_buffer, n);
}


static ssize_t tve_debug_write(struct file *file,
                           const char __user *ubuf, size_t count, loff_t *ppos)
{
    const int debug_bufmax = sizeof(tve_debug_buffer) - 1;
    size_t ret;

    ret = count;

    if (count > debug_bufmax)
    {
        count = debug_bufmax;
    }

    if (copy_from_user(&tve_debug_buffer, ubuf, count))
    {
        return -EFAULT;
    }

    tve_debug_buffer[count] = 0;

    process_dbg_cmd(tve_debug_buffer);

    return ret;
}


static struct file_operations tve_debug_fops =
{
    .read  = tve_debug_read,
    .write = tve_debug_write,
    .open  = tve_debug_open,
};


void tve_debug_init(void)
{
    tve_dbgfs = debugfs_create_file("tve",
                                       S_IFREG | S_IRUGO, NULL, (void *)0, &tve_debug_fops);
}


void tve_debug_deinit(void)
{
    debugfs_remove(tve_dbgfs);
}


