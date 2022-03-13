/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  Copyright 2015 SONY corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "lcm_drv.h"

#ifdef BUILD_LK
    #include <string.h>
    #include <platform/mt_pmic.h>
    #include <platform/mt_gpio.h>
    #include <platform/upmu_common.h>
    #include <platform/mt_i2c.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
    #include <platform/upmu_common.h>
#else
    #include <mach/mt_gpio.h>
    #include <mach/upmu_common.h>
    #include <mach/mt_pm_ldo.h>
#endif

#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#if defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif
#endif

#define LCM_DBG(fmt, arg...) \
    LCM_PRINT ("[LCM-MT8590-HX8379C-DSI-XXX] %s (l:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                         (480)
#define FRAME_HEIGHT                                        (800)//(854)

#define REGFLAG_DELAY                                       0xFFFE
#define REGFLAG_END_OF_TABLE                                0xFFFF   // END OF REGISTERS MARKER

#define BACKLIGHT_I2C_ADDRESS (0x36)
#define BACKLIGHT_ENABLE (124)
#define BACKLIGHT_PWM (203)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                  lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)              lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                            lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size) (lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
    //SET PASSWORD
    {0xB9, 3, {0xFF,0x83,0x79}},
    {REGFLAG_DELAY, 10, {}},

    //SET POWER
    {0xB1, 16, {0x44, 0x18, 0x18, 0x31,
                0x51, 0x50, 0xD0, 0xD8,
                0x58, 0x80, 0x38, 0x38,
                0xF8, 0x33, 0x32,0x22}},

    //SET DISPLAY RELATED REGISTER
    {0xB2, 9, {0x80, 0x3C, 0x0A, 0x03,
                0x70, 0x50, 0x11, 0x42,
                0x1D}},

    //SET CYC
    {0xB4, 10, {0x02, 0x7C, 0x02, 0x7C,
                0x02, 0x7C, 0x22, 0x86,
                0x23, 0x86}},

    //SET TCON
    {0xC7, 4, {0x00, 0x00, 0x00, 0xC0}},

    //SET PANEL
    {0xCC, 1, {0x02}},

    //SET OFFSET
    {0xD2, 1, {0x77}},

    //SET GIP_0
    {0xD3, 37, {0x00, 0x07, 0x00, 0x00,
                0x00, 0x08, 0x08, 0x32,
                0x10, 0x01, 0x00, 0x01,
                0x03, 0x72, 0x03, 0x72,
                0x00, 0x08, 0x00, 0x08,
                0x33, 0x33, 0x05, 0x05,
                0x37, 0x05, 0x05, 0x37,
                0x08, 0x00, 0x00, 0x00,
                0x0A, 0x00, 0x01, 0x01,
                0x0F}},

    //SET GIP_1
    {0xD5, 34, {0x18, 0x18, 0x18, 0x18,
                0x18, 0x18, 0x07, 0x06,
                0x05, 0x04, 0x03, 0x02,
                0x01, 0x00, 0x18, 0x18,
                0x21, 0x20, 0x18, 0x18,
                0x19, 0x19, 0x23, 0x22,
                0x38, 0x38, 0x78, 0x78,
                0x18, 0x18, 0x18, 0x18,
                0x00, 0x00}},

    //SET GIP_2
    {0xD6, 32, {0x18, 0x18, 0x18, 0x18,
                0x18, 0x18, 0x00, 0x01,
                0x02, 0x03, 0x04, 0x05,
                0x06, 0x07, 0x18, 0x18,
                0x22, 0x23, 0x19, 0x19,
                0x18, 0x18, 0x20, 0x21,
                0x38, 0x38, 0x38, 0x38,
                0x18, 0x18, 0x18, 0x18}},

    //SET GAMMA
    {0xE0, 42, {0x00, 0x01, 0x04, 0x20,
                0x24, 0x3F, 0x11, 0x33,
                0x09, 0x0A, 0x0C, 0x17,
                0x0F, 0x12, 0x15, 0x13,
                0x14, 0x0A, 0x15, 0x16,
                0x18, 0x00, 0x01, 0x04,
                0x20, 0x24, 0x3F, 0x11,
                0x33, 0x09, 0x0B, 0x0C,
                0x17, 0x0E, 0x11, 0x14,
                0x13, 0x14, 0x0A, 0x15,
                0x16, 0x18}},

    //SET VCOM
    {0xB6, 2, {0x5E,0x5E}},

    //sleep out
    {0x11, 0, {}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 0, {}},
    {REGFLAG_DELAY, 10, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 0, {}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 0, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 0, {}},

    // Sleep Mode On
    {0x10, 0, {}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;
    unsigned char cmd_data;

    LCM_DBG("%s(): in cmd[0]=%x ",__func__,table[0].cmd);
    for(i = 0; i < count; i++) {
        unsigned cmd;
        cmd = table[i].cmd;
        
        switch (cmd) {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
            case REGFLAG_END_OF_TABLE :
                break;
            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
                break;
        }
    }
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode                 = LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;
    params->dsi.mode   = SYNC_EVENT_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM                = LCM_TWO_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active                = 2;
    params->dsi.vertical_backporch                  = 10;
    params->dsi.vertical_frontporch                 = 5;
    params->dsi.vertical_active_line                = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active              = 37;
    params->dsi.horizontal_backporch                = 35;
    params->dsi.horizontal_frontporch               = 35;
    params->dsi.horizontal_active_pixel             = FRAME_WIDTH;

    params->dsi.PLL_CLOCK = 187;//200; //this value must be in MTK suggested table
    params->dsi.HS_TRAIL = 3;//for Teot fail
    //params->dsi.cont_clock=1;
}

static int show_hx8379c_reg(unsigned int cmd, char* buffer, int buffer_size){
    unsigned char buf = 0;
    int i=0;
    unsigned char cmd_params=0;
    int rv=0;

    if (buffer==NULL || buffer_size < 0)
    {
        return -1;
    }

    for (i = 0; i < buffer_size; ++i)
    {
        cmd_params = (cmd & 0xFF) + i;
        //dsi_set_cmdq_V2(0x00, 1, &cmd_params, 1);
        //read_reg_v2(cmd>>8,&buf,1);
        rv = read_reg_v2(cmd_params,&buf,1);
        LCM_DBG("read_reg_v2 %xh  = %2x rv=%d\n",cmd_params,buf,rv);
    }
    return 0;
}

static void hx8379c_reg_dump(void){
    int count = sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table);
    int buf[64]={0};
    int i=0;

    for(i = 0; i < count; i++) {
        LCM_DBG();
        unsigned cmd;
        cmd = lcm_initialization_setting[i].cmd;

        switch (cmd) {
            case REGFLAG_DELAY :
                MDELAY(lcm_initialization_setting[i].count);
                break;
            case REGFLAG_END_OF_TABLE :
                break;
            default:
                show_hx8379c_reg(cmd,buf, lcm_initialization_setting[i].count);
                break;
        }
    }
    return;
}
static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(0);
    UDELAY(100);
    SET_RESET_PIN(1);
    MDELAY(20);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_resume(void)
{
    lcm_init();
}

LCM_DRIVER hx8379c_dsi_xxx_lcm_drv = 
{
    .name           = "hx8379c_vdo_cmd",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
//    .esd_check     = lcm_esd_check,
//    .esd_recover  = lcm_esd_recover,
};
