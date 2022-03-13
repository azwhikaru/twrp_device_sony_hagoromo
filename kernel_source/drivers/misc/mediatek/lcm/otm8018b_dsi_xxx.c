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
    LCM_PRINT ("[LCM-MT8590-OTM8018b-DSI-XXX] %s (l:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                         (480)
#define FRAME_HEIGHT                                        (854)

#define REGFLAG_DELAY                                       0xFFFE
#define REGFLAG_END_OF_TABLE                                0xFFFF   // END OF REGISTERS MARKER

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
    //command2_enable
    {0xff, 3, {0x80, 0x09, 0x01}},

    {0xff80, 2, {0x80, 0x09}},
    {REGFLAG_DELAY, 20, {}},
    //sleep out
    {0x11, 0, {}},
    {REGFLAG_DELAY, 120, {}},

    {0xc080, 5, {0x00, 0x57, 0x00, 0x15, 0x15}},
    {0xc0b4, 1, {0x50}},
    {0xc582, 1, {0xa3}},
    {0xc590, 2, {0xd6, 0x76}},
    {0xd800, 2, {0xaf, 0xaf}},
    {0xd900, 1, {0x86}},
    {0xc181, 1, {0x66}},
    {0xc1a0, 1, {0xea}},
    {0xc1a1, 1, {0x08}},
    {0xc487, 1, {0x00}},
    {0xc488, 1, {0x80}},
    {0xc489, 1, {0x00}},
    {0xc0a3, 1, {0x1b}},
    {0xc481, 1, {0x83}},
    {0xc592, 1, {0x01}},
    {0xc5b1, 1, {0xa9}},

    {0xb390, 1, {0x02}},
    {0xb392, 1, {0x45}},
    {0xb08b, 1, {0x40}},
    {0xc090, 6, {0x00, 0x44, 0x00, 0x00, 0x00, 0x03}},
    {0xc1a6, 3, {0x01, 0x00, 0x00}},
    {0xce80, 6, {0x85, 0x03, 0x0a, 0x84, 0x03, 0x0a}},
    {0xce90, 6, {0x33, 0x5e, 0x0a, 0x33, 0x5f, 0x0a}},

    {0xcea0, 14,
        {0x38, 0x0b, 0x83, 0x58, 0x87, 0x0a, 0x00, 0x38,
        0x0a, 0x83, 0x59, 0x87, 0x0a, 0x00}},
    {0xceb0, 14,
        {0x38, 0x09, 0x83, 0x5a, 0x87, 0x0a, 0x00, 0x38,
        0x08, 0x83, 0x5b, 0x87, 0x0a, 0x00}},

    {0xcec0, 14,
        {0x38, 0x07, 0x83, 0x5c, 0x87, 0x0a, 0x00, 0x38,
        0x06, 0x83, 0x5d, 0x87, 0x0a, 0x00}},
    {0xced0, 14,
        {0x38, 0x05, 0x83, 0x5e, 0x87, 0x0a, 0x00, 0x38,
        0x04, 0x83, 0x5f, 0x87, 0x0a, 0x00}},

    {0xcfc7, 1, {0x01}},
    {0xcbc0, 15,
        {0x00, 0x00, 0x00, 0x00, 0x54, 0x54, 0x54, 0x54,
        0x00, 0x54, 0x00, 0x54, 0x00, 0x00, 0x00}},
    {0xcbd0, 15,
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x54, 0x54, 0x54, 0x54, 0x00, 0x54}},
    {0xcbe0, 9,
        {0x00, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00}},
    {0xcc80, 10,
        {0x00, 0x00, 0x00, 0x00, 0x0c, 0x0a, 0x10, 0x0e,
        0x00, 0x02}},
    {0xcc90, 15,
        {0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b}},
    {0xcca0, 15,
        {0x09, 0x0f, 0x0d, 0x00, 0x01, 0x00, 0x05, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {0xccb0, 10,
        {0x00, 0x00, 0x00, 0x00, 0x0d, 0x0f, 0x09, 0x0b,
        0x00, 0x05}},

    {0xccc0, 15,
        {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e}},
    {0xccd0, 15,
        {0x10, 0x0a, 0x0c, 0x00, 0x06, 0x00, 0x02, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {0xf580, 12,
        {0x01, 0x18, 0x02, 0x18, 0x10, 0x18, 0x02, 0x18,
        0x0e, 0x18, 0x0f, 0x20}},
    {0xf590, 10,
        {0x02, 0x18, 0x08, 0x18, 0x06, 0x18, 0x0d, 0x18,
        0x0b, 0x18}},
    {0xf5a0, 8,
        {0x10, 0x18, 0x01, 0x18, 0x14, 0x18, 0x14, 0x18}},
    {0xf5b0, 12,
        {0x14, 0x18, 0x12, 0x18, 0x13, 0x18, 0x11, 0x18,
        0x13, 0x18, 0x00, 0x00}},

    {REGFLAG_DELAY, 20, {}},

    /* CEEN */
    {0xd680, 1, {0x28}},

    {0xe100, 16,
        {0x05, 0x13, 0x22, 0x0c, 0x08, 0x0d, 0x0b, 0x0b,
        0x03, 0x06, 0x0d, 0x07, 0x0c, 0x15, 0x15, 0x0c}},
    {0xe200, 16,
        {0x05, 0x13, 0x22, 0x0c, 0x08, 0x0d, 0x0b, 0x0b,
        0x03, 0x06, 0x0d, 0x07, 0x0c, 0x15, 0x15, 0x0c}},

    {REGFLAG_DELAY, 20, {}},

    {0x3500, 1, {0x00}}, /* TE enable */

    //sleep out
    {0x29, 0, {}},

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

    LCM_DBG("in %d ",count);
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
                if(0x00FF < cmd || cmd==0x0000){
                    cmd_data = cmd&0xFF;
                    dsi_set_cmdq_V2(0x00, 1, &cmd_data, force_update);
                    dsi_set_cmdq_V2((cmd>>8)&0xFF, table[i].count, table[i].para_list, force_update);
                }
                else{
                    dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
                }
                break;
        }
    }
   LCM_DBG("out");
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
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_LSB_FIRST; //LSB!
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_MSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active                = 2;//tmp 1?
    params->dsi.vertical_backporch                  = 16;
    params->dsi.vertical_frontporch                 = 15;
    params->dsi.vertical_active_line                = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active              = 4;
    params->dsi.horizontal_backporch                = 44;
    params->dsi.horizontal_frontporch               = 46;
    params->dsi.horizontal_active_pixel             = FRAME_WIDTH;

    params->dsi.PLL_CLOCK = 200;//450; //this value must be in MTK suggested table

    params->dsi.CLK_ZERO = 57;//need?
    params->dsi.HS_ZERO = 22;//need?
    params->dsi.HS_TRAIL = 3;//for Teot fail

}

static int show_otm8018b_reg(unsigned int cmd, char* buffer, int buffer_size){
    unsigned char buf = 0;
    int i=0;
    unsigned char cmd_params=0;

    if (buffer==NULL || buffer_size < 0)
    {
        return -1;
    }

    for (i = 0; i < buffer_size; ++i)
    {
        cmd_params = (cmd & 0xFF) + i;
        dsi_set_cmdq_V2(0x00, 1, &cmd_params, 1);
        read_reg_v2(cmd>>8,&buf,1);
        LCM_DBG("read_reg_v2 test!!  %xh  = %2x ",((cmd)+i),buf);
    }
    return 0;
}

static void otm8018b_reg_dump(void){
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
                show_otm8018b_reg(cmd,buf, lcm_initialization_setting[i].count);
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

LCM_DRIVER otm8018b_dsi_xxx_lcm_drv = 
{
    .name           = "otm8018b_vdo_cmd",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
};
