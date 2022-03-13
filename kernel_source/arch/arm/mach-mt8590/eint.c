/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
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
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wakelock.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <asm/delay.h>
#include <mach/mt_reg_base.h>
#include <mach/eint.h>
#include <mach/eint_drv.h>
#include <mach/irqs.h>
#include <mach/sync_write.h>
#if (defined(CONFIG_ARCH_MT8590_ICX))
#include <mach/icx_pm_helper.h>
#include <linux/moduleparam.h>
#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */

#define MD_EINT
#define EINT_TEST
#define DEINT_SUPPORT
#define DEFINE_FIQ 1 /*for fiq*/
#ifdef MD_EINT
#include <cust_eint.h>
//#include <cust_eint_md1.h>
//#include <cust_eint_md2.h>

typedef enum
{
    SIM_HOT_PLUG_EINT_NUMBER,
    SIM_HOT_PLUG_EINT_DEBOUNCETIME,
    SIM_HOT_PLUG_EINT_POLARITY,
    SIM_HOT_PLUG_EINT_SENSITIVITY,
    SIM_HOT_PLUG_EINT_SOCKETTYPE,
}sim_hot_plug_eint_queryType;

typedef enum
{
    ERR_SIM_HOT_PLUG_NULL_POINTER=-13,
    ERR_SIM_HOT_PLUG_QUERY_TYPE,
    ERR_SIM_HOT_PLUG_QUERY_STRING,
}sim_hot_plug_eint_queryErr;
#endif

#define EINT_TEST_LATENCY (0)

#define EINT_USE_PRECISE_HRTIMER (1)
#if EINT_USE_PRECISE_HRTIMER
#include <linux/ktime.h>
#include <linux/hrtimer.h>

typedef struct 
{
    struct hrtimer rTimer;
    unsigned int eint_num;
} EINT_HR_TIMER_T;

#define EINT_JUDGE_ST_IN_TIMER_CB (1)
#else
#define EINT_JUDGE_ST_IN_TIMER_CB (1)
#endif

typedef struct 
{
    void (*eint_func[EINT_MAX_CHANNEL]) (void);
    unsigned int eint_auto_umask[EINT_MAX_CHANNEL];
    /*is_deb_en: 1 means enable, 0 means disable */
    unsigned int is_deb_en[EINT_MAX_CHANNEL];
    unsigned int deb_time[EINT_MAX_CHANNEL];
#if defined(EINT_TEST)
    unsigned int softisr_called[EINT_MAX_CHANNEL];
#endif

#if EINT_USE_PRECISE_HRTIMER
    EINT_HR_TIMER_T eint_sw_deb_hr_timer[EINT_MAX_CHANNEL];
#else
    struct timer_list eint_sw_deb_timer[EINT_MAX_CHANNEL];
#endif
} eint_func;

static eint_func EINT_FUNC;

#ifdef DEINT_SUPPORT
/*
typedef struct
{
    void (*deint_func[DEINT_MAX_CHANNEL]) (void);
}deint_func;

static deint_func DEINT_FUNC;
*/
#endif

static const unsigned int EINT_IRQ = MT_EINT_IRQ_ID;
struct wake_lock EINT_suspend_lock;
static unsigned int cur_eint_num;
static DEFINE_SPINLOCK(eint_lock);

/*
 * mt_eint_get_mask: To get the eint mask
 * @eint_num: the EINT number to get
 */
static unsigned int mt_eint_get_mask(unsigned int eint_num)
{
    unsigned int base;
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_MASK_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }

    st = readl(IOMEM(base));
    if (st & bit) {
        st = 1; //masked
    } else {
        st = 0; //unmasked
    }

    return st;
}

#if 0
void mt_eint_mask_all(void)
{
    unsigned int base;
    unsigned int val = 0xFFFFFFFF, ap_cnt = (EINT_MAX_CHANNEL / 32), i;
    if (EINT_MAX_CHANNEL % 32)
        ap_cnt++;
    dbgmsg("[EINT] cnt:%d\n", ap_cnt);

    base = EINT_MASK_SET_BASE;
    for (i = 0; i < ap_cnt; i++) {
        writel(val, IOMEM(base + (i * 4)));
        dbgmsg("[EINT] mask addr:%x = %x\n", EINT_MASK_BASE + (i * 4),
               readl(IOMEM(EINT_MASK_BASE + (i * 4))));
    }
}

/*
 * mt_eint_unmask_all: Mask the specified EINT number.
 */
void mt_eint_unmask_all(void)
{
    unsigned int base;
    unsigned int val = 0xFFFFFFFF, ap_cnt = (EINT_MAX_CHANNEL / 32), i;
    if (EINT_MAX_CHANNEL % 32)
        ap_cnt++;
    dbgmsg("[EINT] cnt:%d\n", ap_cnt);

    base = EINT_MASK_CLR_BASE;
    for (i = 0; i < ap_cnt; i++) {
        writel(val, IOMEM(base + (i * 4)));
        dbgmsg("[EINT] unmask addr:%x = %x\n", EINT_MASK_BASE + (i * 4),
               readl(IOMEM(EINT_MASK_BASE + (i * 4))));
    }
}

/*
 * mt_eint_get_soft: To get the eint mask
 * @eint_num: the EINT number to get
 */
unsigned int mt_eint_get_soft(unsigned int eint_num)
{
    unsigned int base;
    unsigned int st;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_SOFT_BASE;
    } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    st = readl(IOMEM(base));

    return st;
}
#endif

#if 0
/*
 * mt_eint_emu_set: Trigger the specified EINT number.
 * @eint_num: EINT number to set
 */
void mt_eint_emu_set(unsigned int eint_num)
{
        unsigned int base = 0;
        unsigned int bit = 1 << (eint_num % 32);
    unsigned int value = 0;

        if (eint_num < EINT_AP_MAXNUMBER) {
                base = (eint_num / 32) * 4 + EINT_EMUL_BASE;
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
        }
    value = readl(IOMEM(base));
    value = bit | value;
        writel(value, IOMEM(base));
    value = readl(IOMEM(base));

        dbgmsg("[EINT] emul set addr:%x = %x, bit=%x\n", base, value, bit);


}

/*
 * mt_eint_emu_clr: Trigger the specified EINT number.
 * @eint_num: EINT number to clr
 */
void mt_eint_emu_clr(unsigned int eint_num)
{
        unsigned int base = 0;
        unsigned int bit = 1 << (eint_num % 32);
    unsigned int value = 0;

        if (eint_num < EINT_AP_MAXNUMBER) {
                base = (eint_num / 32) * 4 + EINT_EMUL_BASE;
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
        }
    value = readl(IOMEM(base));
    value = (~bit) & value;
        writel(value, IOMEM(base));
    value = readl(IOMEM(base));

        dbgmsg("[EINT] emul clr addr:%x = %x, bit=%x\n", base, value, bit);

}

/*
 * eint_send_pulse: Trigger the specified EINT number.
 * @eint_num: EINT number to send
 */
inline void mt_eint_send_pulse(unsigned int eint_num)
{
    unsigned int base_set = (eint_num / 32) * 4 + EINT_SOFT_SET_BASE;
    unsigned int base_clr = (eint_num / 32) * 4 + EINT_SOFT_CLR_BASE;
    unsigned int bit = 1 << (eint_num % 32);
    if (eint_num < EINT_AP_MAXNUMBER) {
        base_set = (eint_num / 32) * 4 + EINT_SOFT_SET_BASE;
        base_clr = (eint_num / 32) * 4 + EINT_SOFT_CLR_BASE;
    } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }

    writel(bit, IOMEM(base_set));
    writel(bit, IOMEM(base_clr));
}
#endif

#if defined(EINT_TEST)
/*
 * mt_eint_soft_set: Trigger the specified EINT number.
 * @eint_num: EINT number to set
 */
void mt_eint_soft_set(unsigned int eint_num)
{

    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_SOFT_SET_BASE;
    } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    writel(bit, IOMEM(base));

    dbgmsg("[EINT] soft set addr:%x = %x\n", base, bit);

}

/*
 * mt_eint_soft_clr: Unmask the specified EINT number.
 * @eint_num: EINT number to clear
 */
static void mt_eint_soft_clr(unsigned int eint_num)
{
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_SOFT_CLR_BASE;
    } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    writel(bit, IOMEM(base));

    dbgmsg("[EINT] soft clr addr:%x = %x\n", base, bit);

}
#endif

/*
 * mt_eint_mask: Mask the specified EINT number.
 * @eint_num: EINT number to mask
 */
void mt_eint_mask(unsigned int eint_num)
{
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_MASK_SET_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    mt65xx_reg_sync_writel(bit, base);

    dbgmsg("[EINT] mask addr:%x = %x\n", base, bit);
}

/*
 * mt_eint_unmask: Unmask the specified EINT number.
 * @eint_num: EINT number to unmask
 */
void mt_eint_unmask(unsigned int eint_num)
{
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_MASK_CLR_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    mt65xx_reg_sync_writel(bit, base);

    dbgmsg("[EINT] unmask addr:%x = %x\n", base, bit);
}

/*
 * mt_eint_set_polarity: Set the polarity for the EINT number.
 * @eint_num: EINT number to set
 * @pol: polarity to set
 */
void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol)
{
    unsigned int count;
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (pol == MT_EINT_POL_NEG) {
        if (eint_num < EINT_AP_MAXNUMBER) {
            base = (eint_num / 32) * 4 + EINT_POL_CLR_BASE;
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
            return;
        }
    } else {
        if (eint_num < EINT_AP_MAXNUMBER) {
            base = (eint_num / 32) * 4 + EINT_POL_SET_BASE;
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
            return;
        }
    }
    mt65xx_reg_sync_writel(bit, base);

    for (count = 0; count < 250; count++) ;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_INTACK_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    mt65xx_reg_sync_writel(bit, base);
    dbgmsg("[EINT] %s :%x, bit: %x\n", __func__, base, bit);
}

/*
 * mt_eint_get_polarity: Set the polarity for the EINT number.
 * @eint_num: EINT number to get
 * Return: polarity type
 */
unsigned int mt_eint_get_polarity(unsigned int eint_num)
{
    unsigned int val;
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);
    unsigned int pol;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_POL_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    val = readl(IOMEM(base));

    dbgmsg("[EINT] %s :%x, bit:%x, val:%x\n", __func__, base, bit, val);
    if (val & bit) {
        pol = MT_EINT_POL_POS;
    } else {
        pol = MT_EINT_POL_NEG;
    }
    return pol;  
}

/*
 * mt_eint_set_sens: Set the sensitivity for the EINT number.
 * @eint_num: EINT number to set
 * @sens: sensitivity to set
 * Always return 0.
 */
unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens)
{
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (sens == MT_EDGE_SENSITIVE) {
        if (eint_num < EINT_AP_MAXNUMBER) {
            base = (eint_num / 32) * 4 + EINT_SENS_CLR_BASE;
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
            return 0;
        }
    } else if (sens == MT_LEVEL_SENSITIVE) {
        if (eint_num < EINT_AP_MAXNUMBER) {
            base = (eint_num / 32) * 4 + EINT_SENS_SET_BASE;
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
            return 0;
        }
    } else {
        printk("%s invalid sensitivity value\n", __func__);
        return 0;
    }
    mt65xx_reg_sync_writel(bit, base);
    dbgmsg("[EINT] %s :%x, bit: %x\n", __func__, base, bit);
    return 0;
}

/*
 * mt_eint_get_sens: To get the eint sens
 * @eint_num: the EINT number to get
 */
static unsigned int mt_eint_get_sens(unsigned int eint_num)
{
    unsigned int base, sens;
    unsigned int bit = 1 << (eint_num % 32), st;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_SENS_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    st = readl(IOMEM(base));
    if (st & bit) {
        sens = MT_LEVEL_SENSITIVE;
    } else {
        sens = MT_EDGE_SENSITIVE;
    }
    return sens;
}

/*
 * mt_eint_ack: To ack the interrupt 
 * @eint_num: the EINT number to set
 */
unsigned int mt_eint_ack(unsigned int eint_num)
{
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    #if EINT_TEST_LATENCY
    printk("Test latency, eint %d ack\n", eint_num);
    #endif

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_INTACK_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    mt65xx_reg_sync_writel(bit, base);

    dbgmsg("[EINT] %s :%x, bit: %x\n", __func__, base, bit);
    return 0;
}

/*
 * mt_eint_read_status: To read the interrupt status
 * @eint_num: the EINT number to set
 */
static unsigned int mt_eint_read_status(unsigned int eint_num)
{
    unsigned int base;
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_STA_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    st = readl(IOMEM(base));

    return (st & bit);
}

/*
 * mt_eint_get_status: To get the interrupt status 
 * @eint_num: the EINT number to get
 */
static unsigned int mt_eint_get_status(unsigned int eint_num)
{
    unsigned int base;
    unsigned int st;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_STA_BASE;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }

    st = readl(IOMEM(base));
    return st;
}

/*
 * mt_eint_en_hw_debounce: To enable hw debounce
 * @eint_num: the EINT number to set
 */
static void mt_eint_en_hw_debounce(unsigned int eint_num)
{
    unsigned int base, bit;
    base = (eint_num / 4) * 4 + EINT_DBNC_SET_BASE;
    bit = (EINT_DBNC_SET_EN << EINT_DBNC_SET_EN_BITS) << ((eint_num % 4) * 8);
    writel(bit, IOMEM(base)); 
    EINT_FUNC.is_deb_en[eint_num] = 1;
}

/*
 * mt_eint_dis_hw_debounce: To disable hw debounce
 * @eint_num: the EINT number to set
 */
static void mt_eint_dis_hw_debounce(unsigned int eint_num)
{
    unsigned int clr_base, bit;
    clr_base = (eint_num / 4) * 4 + EINT_DBNC_CLR_BASE;
    bit = (EINT_DBNC_CLR_EN << EINT_DBNC_CLR_EN_BITS) << ((eint_num % 4) * 8);
    writel(bit, IOMEM(clr_base));
    EINT_FUNC.is_deb_en[eint_num] = 0;
}

/*
 * mt_eint_dis_sw_debounce: To set EINT_FUNC.is_deb_en[eint_num] disable
 * @eint_num: the EINT number to set
 */
static void mt_eint_dis_sw_debounce(unsigned int eint_num)
{
    if(eint_num < EINT_MAX_CHANNEL)
        EINT_FUNC.is_deb_en[eint_num] = 0;
}

/*
 * mt_eint_en_sw_debounce: To set EINT_FUNC.is_deb_en[eint_num] enable
 * @eint_num: the EINT number to set
 */
static void mt_eint_en_sw_debounce(unsigned int eint_num)
{
    if(eint_num < EINT_MAX_CHANNEL)    
        EINT_FUNC.is_deb_en[eint_num] = 1;
}

/*
 * mt_can_en_debounce: Check the EINT number is able to enable debounce or not
 * @eint_num: the EINT number to set
 */
static unsigned int mt_can_en_debounce(unsigned int eint_num)
{
    unsigned int sens = mt_eint_get_sens(eint_num);
    /* debounce: debounce time is not 0 && it is not edge sensitive */
    if (EINT_FUNC.deb_time[eint_num] != 0 && sens != MT_EDGE_SENSITIVE)
        return 1;
    else {
        dbgmsg
            ("Can't enable debounce of eint_num:%d, deb_time:%d, sens:%d\n",
             eint_num, EINT_FUNC.deb_time[eint_num], sens);
        return 0;
    }
}

/*
 * mt_eint_set_hw_debounce: Set the de-bounce time for the specified EINT number.
 * @eint_num: EINT number to acknowledge
 * @ms: the de-bounce time to set (in miliseconds)
 */
void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms)
{
    unsigned int dbnc, base, bit, clr_bit, clr_base, rst, unmask = 0;

    base = (eint_num / 4) * 4 + EINT_DBNC_SET_BASE;
    clr_base = (eint_num / 4) * 4 + EINT_DBNC_CLR_BASE;
    EINT_FUNC.deb_time[eint_num] = ms;

    /* 
     * Don't enable debounce once debounce time is 0 or 
     * its type is edge sensitive.
     */
    if (!mt_can_en_debounce(eint_num)) {
        dbgmsg("Can't enable debounce of eint_num:%d in %s\n", eint_num,
               __func__);
        return;
    }

    if (ms == 0) {
        dbnc = 0;
        dbgmsg("ms should not be 0. eint_num:%d in %s\n", eint_num,
               __func__);
    } else if (ms <= 1) {
        dbnc = 1;
    } else if (ms <= 16) {
        dbnc = 2;
    } else if (ms <= 32) {
        dbnc = 3;
    } else if (ms <= 64) {
        dbnc = 4;
    } else if (ms <= 128) {
        dbnc = 5;
    } else if (ms <= 256) {
        dbnc = 6;
    } else {
        dbnc = 7;
    }

    /* setp 1: mask the EINT */
    if(!mt_eint_get_mask(eint_num)) {
        mt_eint_mask(eint_num);
        unmask = 1;
    }
    /* step 2: Check hw debouce number to decide which type should be used */
    if (eint_num >= MAX_HW_DEBOUNCE_CNT)
        mt_eint_en_sw_debounce(eint_num);
    else {
        /* step 2.1: set hw debounce flag*/
        EINT_FUNC.is_deb_en[eint_num] = 1;
        
        /* step 2.2: disable hw debounce */
        clr_bit = 0xFF << ((eint_num % 4) * 8);
        mt65xx_reg_sync_writel(clr_bit, clr_base);
        
        /* step 2.3: set new debounce value */
        bit =
            ((dbnc << EINT_DBNC_SET_DBNC_BITS) |
             (EINT_DBNC_SET_EN << EINT_DBNC_SET_EN_BITS)) << 
                    ((eint_num % 4) * 8);
        mt65xx_reg_sync_writel(bit, base);

        /* step 2.4: Delay a while (more than 2T) to wait for hw debounce enable work correctly */
        udelay(500);
        
        /* step 2.5: Reset hw debounce counter to avoid unexpected interrupt */
        rst = (EINT_DBNC_RST_BIT << EINT_DBNC_SET_RST_BITS) <<
          ((eint_num % 4) * 8);
        mt65xx_reg_sync_writel(rst, base);

        /* step 2.6: Delay a while (more than 2T) to wait for hw debounce counter reset work correctly */
        udelay(500);   
    }
    /* step 3: unmask the EINT */
    if(unmask == 1)
        mt_eint_unmask(eint_num);
}

/*
 * eint_do_tasklet: EINT tasklet function.
 * @unused: not use.
 */
static void eint_do_tasklet(unsigned long unused)
{
    wake_lock_timeout(&EINT_suspend_lock, HZ / 2);
}

DECLARE_TASKLET(eint_tasklet, eint_do_tasklet, 0);

/*
 * mt_eint_timer_event_handler: EINT sw debounce handler
 * @eint_num: the EINT number and use unsigned long to prevent 
 *            compile warning of timer usage.
 */
static void mt_eint_timer_event_handler(unsigned long eint_num)
{
    unsigned int status;
    unsigned long flags;

    /* disable interrupt for core 0 and it will run on core 0 only */
    local_irq_save(flags);
    
#if EINT_JUDGE_ST_IN_TIMER_CB
    #if EINT_TEST_LATENCY
    printk("Test latency, eint %d, st:0x%x, jif:0x%x, 1\n", eint_num, status, jiffies);
    #endif
    
    mt_eint_unmask(eint_num);
    status = mt_eint_read_status(eint_num);

    #if EINT_TEST_LATENCY
    printk("Test latency, eint %d, st:0x%x, jif:0x%x, 2\n", eint_num, status, jiffies);
    #endif
    
    dbgmsg("EINT Module - EINT_STA = 0x%x, in %s\n", status, __func__);    

    #if 0 //EINT_TEST_LATENCY
    if (24 == eint_num)
    {
        printk("Test latency, eint 24, sel:0x%x, 1\n", mt_get_gpio_pull_select(2));

        if (0 == mt_get_gpio_pull_select(2))
        {
            mt_set_gpio_pull_select(2, GPIO_PULL_UP);  
        }
        else
        {
            mt_set_gpio_pull_select(2, GPIO_PULL_DOWN);  
        } 

        printk("Test latency, eint 24, sel:0x%x, 2\n", mt_get_gpio_pull_select(2));
    }
    #endif
    
    if (status) {
        mt_eint_mask(eint_num);

        #if EINT_TEST_LATENCY
        printk("Test latency, eint %d, get st:0x%x\n", eint_num, status);
        #endif
            
        if (EINT_FUNC.eint_func[eint_num])
            EINT_FUNC.eint_func[eint_num] ();
        mt_eint_ack(eint_num);
    }
#else
    #if EINT_TEST_LATENCY
    printk("Test latency, eint %d, get st:0x%x\n", eint_num, status);
    #endif
        
    if (EINT_FUNC.eint_func[eint_num])
        EINT_FUNC.eint_func[eint_num] ();
    
    mt_eint_ack(eint_num);   
#endif
    
    local_irq_restore(flags);
    if (EINT_FUNC.eint_auto_umask[eint_num])
        mt_eint_unmask(eint_num);
}

#if EINT_USE_PRECISE_HRTIMER
static enum hrtimer_restart mt_eint_hr_timer_event_handler(struct hrtimer *eint_timer)
{
    //todo:
	EINT_HR_TIMER_T *prEintHrTimer = NULL;

	prEintHrTimer = container_of(eint_timer, EINT_HR_TIMER_T, rTimer);
    
    if (prEintHrTimer->eint_num >= EINT_MAX_CHANNEL)
    {
        printk("Fatal error !!!!!!!!!!!!!! \n");
        return HRTIMER_NORESTART;
    }

    #if EINT_TEST_LATENCY
    printk("eint %d hr timer 0x%x receive callback\n", prEintHrTimer->eint_num, prEintHrTimer);    
    #endif

    mt_eint_timer_event_handler(prEintHrTimer->eint_num); 
    
    #if EINT_TEST_LATENCY
    printk("eint %d handler done\n", prEintHrTimer->eint_num);  
    #endif
    
    //spin_lock(&eint_lock);
    prEintHrTimer->eint_num = EINT_MAX_CHANNEL; 
    //spin_unlock(&eint_lock);

    return HRTIMER_NORESTART;
}
#endif

/*
 * mt_eint_set_timer_event: To set a timer event for sw debounce.
 * @eint_num: the EINT number to set
 */
static void mt_eint_set_timer_event(unsigned int eint_num)
{
#if EINT_USE_PRECISE_HRTIMER
    if (eint_num >= EINT_MAX_CHANNEL)
    {
        printk("Error !!!!!!!!!!!!!! \n");
        return;
    }

    //if (EINT_FUNC.deb_time[eint_num] == 1)
    {
        struct hrtimer *prHrTimer = &EINT_FUNC.eint_sw_deb_hr_timer[eint_num].rTimer; 
        ktime_t ktime;

        #if EINT_TEST_LATENCY
        printk("eint %d set timer 0x%x\n", eint_num, prHrTimer);
        #endif
		
        //spin_lock(&eint_lock);
        if (EINT_MAX_CHANNEL == EINT_FUNC.eint_sw_deb_hr_timer[eint_num].eint_num)
        {
            EINT_FUNC.eint_sw_deb_hr_timer[eint_num].eint_num = eint_num;  
        }
        else
        {
            #if EINT_TEST_LATENCY
            printk("eint %d set timer 0x%x with eint num %d, but already set before !!!!!!\n", eint_num, prHrTimer, EINT_FUNC.eint_sw_deb_hr_timer[eint_num].eint_num);
            #endif
            
            //spin_unlock(&eint_lock);
            return;
        }
        //spin_unlock(&eint_lock);

        ktime = ktime_set(0, EINT_FUNC.deb_time[eint_num] * NSEC_PER_MSEC);
        hrtimer_init(prHrTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

        prHrTimer->function = mt_eint_hr_timer_event_handler;
        hrtimer_start(prHrTimer, ktime, HRTIMER_MODE_REL);
    }
    //else
#else
    {
        struct timer_list *eint_timer = &EINT_FUNC.eint_sw_deb_timer[eint_num];
        /* assign this handler to execute on core 0 */
        int cpu = 0;

        /* register timer for this sw debounce eint */
        eint_timer->expires =
            jiffies + msecs_to_jiffies(EINT_FUNC.deb_time[eint_num]);
        
        printk("Test latency, deb time:0x%x, jif:0x%x, expires:0x%x, HZ:0x%x\n", EINT_FUNC.deb_time[eint_num], jiffies, eint_timer->expires, HZ);
        
        dbgmsg("EINT Module - expires:%llu, jiffies:%llu, deb_in_jiffies:%llu, ", eint_timer->expires, jiffies, msecs_to_jiffies(EINT_FUNC.deb_time[eint_num]));
        dbgmsg("deb:%d, in %s\n", EINT_FUNC.deb_time[eint_num], __func__);    
            
        eint_timer->data = eint_num;
        eint_timer->function = &mt_eint_timer_event_handler;
        init_timer(eint_timer);
        add_timer_on(eint_timer, cpu);
    }
#endif
}

#if EINT_TEST_LATENCY
unsigned int  _uTrigBeforeTimer = 0;
#include <mach/mt_gpio.h>
#endif

/*
 * mt_eint_isr: EINT interrupt service routine.
 * @irq: EINT IRQ number
 * @dev_id:
 * Return IRQ returned code.
 */
static irqreturn_t mt_eint_isr(int irq, void *dev_id)
{
    unsigned int index, rst, base;
    unsigned int status = 0;
    unsigned int status_check;
    unsigned int reg_base,offset;
        
     /* 
     * NoteXXX: Need to get the wake up for 0.5 seconds when an EINT intr tirggers.
     *          This is used to prevent system from suspend such that other drivers
     *          or applications can have enough time to obtain their own wake lock.
     *          (This information is gotten from the power management owner.)
     */

    tasklet_schedule(&eint_tasklet);
    dbgmsg("EINT Module - %s ISR Start\n", __func__);
    //printk("EINT acitve: %s, acitve status: %d\n", mt_irq_is_active(EINT_IRQ) ? "yes" : "no", mt_irq_is_active(EINT_IRQ));

    for (reg_base = 0; reg_base < EINT_MAX_CHANNEL; reg_base+=32) {
            /* read status register every 32 interrupts */
            status = mt_eint_get_status(reg_base);
            if(status){
                dbgmsg("EINT Module - index:%d,EINT_STA = 0x%x\n",
                  reg_base, status);
            }
            else{
                continue;
            }
        for(offset = 0; offset < 32; offset++){
            index = reg_base + offset;
            if (index >= EINT_MAX_CHANNEL) break;

        status_check = status & (1 << (index % 32));
        if (status_check) {
                dbgmsg("Got eint:%d\n",index);
            mt_eint_mask(index);
            
            if ((EINT_FUNC.is_deb_en[index] == 1) &&
                (index >= MAX_HW_DEBOUNCE_CNT)) {

                #if EINT_TEST_LATENCY
                if (24 == index)
                {
                    printk("Test latency, 3, dir:0x%x, mode:0x%x, en:0x%x, sel:0x%x\n", mt_get_gpio_dir(2), mt_get_gpio_mode(2), mt_get_gpio_pull_enable(2), mt_get_gpio_pull_select(2));
                    
                    if (1 == _uTrigBeforeTimer)
                    {
                        if (0 == mt_get_gpio_pull_select(2))
                        {
                            mt_set_gpio_pull_select(2, GPIO_PULL_UP);  
                        }
                        else
                        {
                            mt_set_gpio_pull_select(2, GPIO_PULL_DOWN);  
                        }  
                    }

                    printk("Test latency, 4, dir:0x%x, mode:0x%x, en:0x%x, sel:0x%x\n", mt_get_gpio_dir(2), mt_get_gpio_mode(2), mt_get_gpio_pull_enable(2), mt_get_gpio_pull_select(2));
                }
                #endif
                
                /* if its debounce is enable and it is a sw debounce */
                mt_eint_set_timer_event(index);
            } else {
                /* HW debounce or no use debounce */
                if (EINT_FUNC.eint_func[index]) {
                    EINT_FUNC.eint_func[index] ();
                }
                mt_eint_ack(index);
#if 1                 /* Don't need to use reset ? */
                /* reset debounce counter */
                base = (index / 4) * 4 + EINT_DBNC_SET_BASE;
                rst =
                    (EINT_DBNC_RST_BIT << EINT_DBNC_SET_RST_BITS) <<
                    ((index % 4) * 8);
                mt65xx_reg_sync_writel(rst, base);
#endif
#if(EINT_DEBUG == 1)
                status = mt_eint_get_status(index);
                dbgmsg
                    ("EINT Module - EINT_STA after ack = 0x%x\n",
                     status);
#endif
                if (EINT_FUNC.eint_auto_umask[index]) {
                    mt_eint_unmask(index);
                }
            }
        }
        }
    }

    dbgmsg("EINT Module - %s ISR END\n", __func__);
    return IRQ_HANDLED;
}

static int mt_eint_max_channel(void)
{
    return EINT_MAX_CHANNEL;
}

/*
 * mt_eint_dis_debounce: To disable debounce.
 * @eint_num: the EINT number to disable
 */
void mt_eint_dis_debounce(unsigned int eint_num)
{
    /* This function is used to disable debounce whether hw or sw */
    if (eint_num < MAX_HW_DEBOUNCE_CNT)
        mt_eint_dis_hw_debounce(eint_num);
    else
        mt_eint_dis_sw_debounce(eint_num);
}

/*
 * mt_eint_registration: register a EINT.
 * @eint_num: the EINT number to register
 * @flag: the interrupt line behaviour to select
 * @EINT_FUNC_PTR: the ISR callback function
 * @is_auto_unmask: the indication flag of auto unmasking after ISR callback is processed
 */
void mt_eint_registration(unsigned int eint_num, unsigned int flag, 
              void (EINT_FUNC_PTR) (void), unsigned int is_auto_umask)
{
    if (eint_num < EINT_MAX_CHANNEL) {
        printk("eint register for %d\n", eint_num);
        spin_lock(&eint_lock);
        mt_eint_mask(eint_num);
        
        if (flag & (EINTF_TRIGGER_RISING | EINTF_TRIGGER_FALLING)) {
            mt_eint_set_polarity(eint_num, (flag & EINTF_TRIGGER_FALLING) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(eint_num, MT_EDGE_SENSITIVE);
        } else if (flag & (EINTF_TRIGGER_HIGH | EINTF_TRIGGER_LOW)) {
            mt_eint_set_polarity(eint_num, (flag & EINTF_TRIGGER_LOW) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(eint_num, MT_LEVEL_SENSITIVE);    
        } else {
            printk("[EINT]: Wrong EINT Pol/Sens Setting 0x%x\n", flag);
            spin_unlock(&eint_lock);
            return ;
        }
        
        EINT_FUNC.eint_func[eint_num] = EINT_FUNC_PTR;
        spin_unlock(&eint_lock);
        EINT_FUNC.eint_auto_umask[eint_num] = is_auto_umask;
        mt_eint_ack(eint_num);
        mt_eint_unmask(eint_num);
    } else {
        printk("[EINT]: Wrong EINT Number %d\n", eint_num);
    }
}


#ifdef DEINT_SUPPORT
/*
static irqreturn_t  mt_deint_isr(int irq, void *dev_id)
{
    int deint_num = irq - MT_EINT_DIRECT0_IRQ_ID;
    printk("IRQ = %d\n", irq);
    if (deint_num < 0) {
        printk(KERN_ERR "DEINT IRQ Number %d IS NOT VALID!! \n", deint_num);
    }
    
    if (DEINT_FUNC.deint_func[deint_num]) { 
        DEINT_FUNC.deint_func[deint_num]();
    }else
       printk("NULL EINT POINTER\n");
    
    
    printk("EXIT DEINT ISR\n");
    return IRQ_HANDLED;
}
*/

/*
 * mt_deint_registration: register a DEINT fiq.
 * @eint_num: the DEINT number to register
 * @flag: the interrupt line behaviour to select
 * @DEINT_FUNC_PTR: the ISR callback function
 * THIS FUNCTION IS INTERNAL USE ONLY, DON'T EXPORT
 * 
 * NOTE: this function is not exported to our customer for now
 *       but if we want to export it in the future, we must let them know that
 *       DEINT should register its ISR with prototype same as ordinal ISR
 *       unlike EINT's void (*func) (void)
 */

void mt_deint_registration(unsigned int deint_num, unsigned int flag,
#if (DEFINE_FIQ == 0)
              irqreturn_t (DEINT_FUNC_PTR) (int, void *)
#else
		fiq_isr_handler DEINT_FUNC_PTR
#endif
	      )
{
    int num = deint_num;
    if (deint_num < DEINT_MAX_CHANNEL) {

        // deint and eint numbers are not linear mapping
        if(deint_num == 69)
            deint_num = 8;
        /*
        if(!mt_eint_get_mask(num)) {
            printk("[Wrong DEint-%d] has been registered as EINT pin.!!! \n", deint_num);
            return ;
        }
        */

        spin_lock(&eint_lock);
        if(EINT_FUNC.eint_func[num] != NULL){
            printk("[Wrong DEint-%d] has been registered as EINT pin.!!! \n", deint_num);
            spin_unlock(&eint_lock);
            return;
        }
        spin_unlock(&eint_lock);

        mt_eint_mask(num);

        if (flag & (EINTF_TRIGGER_RISING | EINTF_TRIGGER_FALLING)) {
            mt_eint_set_polarity(num, (flag & EINTF_TRIGGER_FALLING) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(num, MT_EDGE_SENSITIVE);
        } else if (flag & (EINTF_TRIGGER_HIGH | EINTF_TRIGGER_LOW)) {
            mt_eint_set_polarity(num, (flag & EINTF_TRIGGER_LOW) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(num, MT_LEVEL_SENSITIVE);
        } else {
            printk("[DEINT]: Wrong DEINT Pol/Sens Setting 0x%x\n", flag);
            return ;
        }

        //DEINT_FUNC.deint_func[deint_num] = DEINT_FUNC_PTR;

        /* direct EINT IRQs registration */
        //if (request_irq(MT_EINT_DIRECT0_IRQ_ID + deint_num, mt_deint_isr, IRQF_TRIGGER_HIGH, "DEINT", NULL )) {
        #if(DEFINE_FIQ == 0) 
		#ifdef CONFIG_FIQ_GLUE
        if (request_irq(MT_EINT_DIRECT0_IRQ_ID + deint_num, DEINT_FUNC_PTR, IRQF_TRIGGER_HIGH, "DEINT", NULL )) {
            printk(KERN_ERR "DEINT IRQ LINE NOT AVAILABLE!!\n");
        }
		#endif
		#else
		#ifdef CONFIG_FIQ_GLUE
		if(( deint_num >= 0) && (deint_num <= 3)){
			if (request_fiq(MT_EINT_DIRECT0_IRQ_ID + deint_num, DEINT_FUNC_PTR, EINTF_TRIGGER_RISING, "firqDEINT" )) {
	            printk(KERN_ERR "DEINT FAST IRQ LINE NOT AVAILABLE!!\n");
	        }
		} else {
			if (request_irq(MT_EINT_DIRECT0_IRQ_ID + deint_num, DEINT_FUNC_PTR, EINTF_TRIGGER_RISING, "DEINT", NULL )) {
				printk(KERN_ERR "DEINT IRQ LINE NOT AVAILABLE!!\n");
			}
		}
		#endif
		#endif
          
    } else {
        printk("[DEINT]: Wrong DEINT Number %d\n", num);
    }
}
/*
 * mt_deint_registration_irq: register a DEINT irq.
 * @eint_num: the DEINT number to register
 * @flag: the interrupt line behaviour to select
 * @DEINT_FUNC_PTR: the ISR callback function
 * THIS FUNCTION IS INTERNAL USE ONLY, DON'T EXPORT
 *
 * NOTE: this function is not exported to our customer for now
 *       but if we want to export it in the future, we must let them know that
 *       DEINT should register its ISR with prototype same as ordinal ISR
 *       unlike EINT's void (*func) (void)
 */

void mt_deint_registration_irq(unsigned int deint_num, unsigned int flag,
              irqreturn_t (DEINT_FUNC_PTR) (int, void *))
{
    int num = deint_num;

    if (deint_num < DEINT_MAX_CHANNEL) {

        // deint and eint numbers are not linear mapping
        if(deint_num == 69)
            deint_num = 8;
        /*
        if(!mt_eint_get_mask(num)) {
            printk("[Wrong DEint-%d] has been registered as EINT pin.!!! \n", deint_num);
            return ;
        }
        */
        spin_lock(&eint_lock);
        if(EINT_FUNC.eint_func[num] != NULL){
            printk("[Wrong DEint-%d] has been registered as EINT pin.!!! \n", deint_num);
            spin_unlock(&eint_lock);
            return;
        }
        spin_unlock(&eint_lock);
        mt_eint_mask(num);

        if (flag & (EINTF_TRIGGER_RISING | EINTF_TRIGGER_FALLING)) {
            mt_eint_set_polarity(num, (flag & EINTF_TRIGGER_FALLING) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(num, MT_EDGE_SENSITIVE);
        } else if (flag & (EINTF_TRIGGER_HIGH | EINTF_TRIGGER_LOW)) {
            mt_eint_set_polarity(num, (flag & EINTF_TRIGGER_LOW) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(num, MT_LEVEL_SENSITIVE);
        } else {
            printk("[DEINT]: Wrong DEINT Pol/Sens Setting 0x%x\n", flag);
            return ;
        }

        //DEINT_FUNC.deint_func[deint_num] = DEINT_FUNC_PTR;

        /* direct EINT IRQs registration */
        //if (request_irq(MT_EINT_DIRECT0_IRQ_ID + deint_num, mt_deint_isr, IRQF_TRIGGER_HIGH, "DEINT", NULL )) {
		if(( deint_num >= 0) && (deint_num <= 3)){
	            printk(KERN_ERR "CAN NOT REQUEST FAST IRQ(TZ NOT ALLOWED) LINE FOR IRQ!!\n");
				return;
		} else {
			if (request_irq(MT_EINT_DIRECT0_IRQ_ID + deint_num, DEINT_FUNC_PTR, EINTF_TRIGGER_RISING, "DEINT", NULL )) {
				printk(KERN_ERR "DEINT IRQ LINE NOT AVAILABLE!!\n");
			}
		}

    } else {
        printk("[DEINT]: Wrong DEINT Number %d\n", num);
    }
}
#endif

static unsigned int mt_eint_get_debounce_cnt(unsigned int cur_eint_num)
{
    unsigned int dbnc, deb, base;
    base = (cur_eint_num / 4) * 4 + EINT_DBNC_BASE;

    if (cur_eint_num >= MAX_HW_DEBOUNCE_CNT)
        deb = EINT_FUNC.deb_time[cur_eint_num];
    else {
        dbnc = readl(IOMEM(base));
        dbnc = ((dbnc >> EINT_DBNC_SET_DBNC_BITS) >> ((cur_eint_num % 4) * 8) & EINT_DBNC);

        switch (dbnc) {
        case 0:
            deb = 0;    /* 0.5 actually, but we don't allow user to set. */
            dbgmsg(KERN_CRIT"ms should not be 0. eint_num:%d in %s\n",
                   cur_eint_num, __func__);
            break;
        case 1:
            deb = 1;
            break;
        case 2:
            deb = 16;
            break;
        case 3:
            deb = 32;
            break;
        case 4:
            deb = 64;
            break;
        case 5:
            deb = 128;
            break;
        case 6:
            deb = 256;
            break;
        case 7:
            deb = 512;
            break;
        default:
            deb = 0;
            printk("invalid deb time in the EIN_CON register, dbnc:%d, deb:%d\n", dbnc, deb);
            break;
        }
    }

    return deb;
}

static int mt_eint_is_debounce_en(unsigned int cur_eint_num)
{
    unsigned int base, val, en;
    if (cur_eint_num < MAX_HW_DEBOUNCE_CNT) {
        base = (cur_eint_num / 4) * 4 + EINT_DBNC_BASE;
        val = readl(IOMEM(base));
        val = val >> ((cur_eint_num % 4) * 8);
        if (val & EINT_DBNC_EN_BIT) {
            en = 1;
        } else {
            en = 0;
        }
    } else {
        en = EINT_FUNC.is_deb_en[cur_eint_num];
    }

    return en;
}

static void mt_eint_enable_debounce(unsigned int cur_eint_num)
{
    mt_eint_mask(cur_eint_num);
    if (cur_eint_num < MAX_HW_DEBOUNCE_CNT) {
        /* HW debounce */
        mt_eint_en_hw_debounce(cur_eint_num);
    } else {
        /* SW debounce */
        mt_eint_en_sw_debounce(cur_eint_num);
    }
    mt_eint_unmask(cur_eint_num);
}

static void mt_eint_disable_debounce(unsigned int cur_eint_num)
{
    mt_eint_mask(cur_eint_num);
    if (cur_eint_num < MAX_HW_DEBOUNCE_CNT) {
        /* HW debounce */
        mt_eint_dis_hw_debounce(cur_eint_num);
    } else {
        /* SW debounce */
        mt_eint_dis_sw_debounce(cur_eint_num);
    }
    mt_eint_unmask(cur_eint_num);
}

#if defined(EINT_TEST)
static ssize_t cur_eint_soft_set_show(struct device_driver *driver, char *buf)
{
    unsigned int ret = EINT_FUNC.softisr_called[cur_eint_num];
    /* reset to 0 for the next testing. */
    EINT_FUNC.softisr_called[cur_eint_num] = 0;
    return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}
/*
 * set 1 to trigger and set 0 to clr this interrupt
 */
static ssize_t cur_eint_soft_set_store(struct device_driver *driver, const char *buf, size_t count)
{
    char *p = (char *)buf;
    unsigned int num;

    num = simple_strtoul(p, &p, 10);
    if (num == 1) {
#if 0 //defined(CONFIG_MT6582_FPGA)||defined(CONFIG_MT6589_FPGA)
    mt_eint_emu_set(cur_eint_num);
#else
        mt_eint_soft_set(cur_eint_num);
#endif
    } else if (num == 0) {
#if 0 //defined(CONFIG_MT6582_FPGA)||defined(CONFIG_MT6589_FPGA)
    mt_eint_emu_clr(cur_eint_num);
#else
        mt_eint_soft_clr(cur_eint_num);
#endif
    } else {
        printk("invalid number:%d it should be 1 to trigger interrupt or 0 to clr.\n", num);
    }

    return count;
}
DRIVER_ATTR(current_eint_soft_set, 0664, cur_eint_soft_set_show, cur_eint_soft_set_store);
static void mt_eint_soft_isr(void)
{
    EINT_FUNC.softisr_called[cur_eint_num] = 1;
    dbgmsg("in mt_eint_soft_isr\n");
#if 0 //defined(CONFIG_MT6582_FPGA)||defined(CONFIG_MT6589_FPGA)
    mt_eint_emu_clr(cur_eint_num);
#else
    mt_eint_soft_clr(cur_eint_num);
#endif
    return;
}
static ssize_t cur_eint_reg_isr_show(struct device_driver *driver, char *buf)
{
    /* if ISR has been registered return 1
     * else return 0
     */
    unsigned int sens, pol, deb, autounmask, base, dbnc;
    base = (cur_eint_num / 4) * 4 + EINT_DBNC_BASE;
    sens = mt_eint_get_sens(cur_eint_num);
    pol = mt_eint_get_polarity(cur_eint_num);
    autounmask = EINT_FUNC.eint_auto_umask[cur_eint_num];

    if (cur_eint_num >= MAX_HW_DEBOUNCE_CNT)
        deb = EINT_FUNC.deb_time[cur_eint_num];
    else {
        dbnc = readl(IOMEM(base));
        dbnc = ((dbnc >> EINT_DBNC_SET_DBNC_BITS) >> ((cur_eint_num % 4) * 8) & EINT_DBNC);

        switch (dbnc) {
        case 0:
            deb = 0;/* 0.5 actually, but we don't allow user to set. */
            dbgmsg("ms should not be 0. eint_num:%d in %s\n",
                   cur_eint_num, __func__);
            break;
        case 1:
            deb = 1;
            break;
        case 2:
            deb = 16;
            break;
        case 3:
            deb = 32;
            break;
        case 4:
            deb = 64;
            break;
        case 5:
            deb = 128;
            break;
        case 6:
            deb = 256;
            break;
        case 7:
            deb = 512;
            break;
        default:
            deb = 0;
            printk("invalid deb time in the EIN_CON register, dbnc:%d, deb:%d\n", dbnc, deb);
            break;
        }
    }
    dbgmsg("sens:%d, pol:%d, deb:%d, isr:%p, autounmask:%d\n", sens, pol, deb, EINT_FUNC.eint_func[cur_eint_num], autounmask);
    return snprintf(buf, PAGE_SIZE, "%d\n", deb);
}
static ssize_t cur_eint_reg_isr_store(struct device_driver *driver,
                     const char *buf, size_t count)
{
    /* Get eint number */
    char *p = (char *)buf;
    unsigned int num;
    num = simple_strtoul(p, &p, 10);
    if (num != 1) {
        //dbgmsg("Unregister soft isr\n");
        printk("Unregister soft isr\n");
        mt_eint_mask(cur_eint_num);
    } else {
        /* register its ISR: mt_eint_soft_isr */
        /* level, high, deb time: 64ms, not auto unmask */
        mt_eint_set_sens(cur_eint_num, MT_LEVEL_SENSITIVE); 
        mt_eint_set_hw_debounce(cur_eint_num, EINT_FUNC.deb_time[cur_eint_num] );
        mt_eint_registration(cur_eint_num, EINTF_TRIGGER_HIGH, mt_eint_soft_isr, 0);
    }
    return count;
}
DRIVER_ATTR(current_eint_reg_isr, 0644, cur_eint_reg_isr_show, cur_eint_reg_isr_store);
#endif /*!EINT_TEST */

/*
 * mt_eint_setdomain0: set all eint_num to domain 0.
 */
static void mt_eint_setdomain0(void)
{
    unsigned int base;
    unsigned int val = 0xFFFFFFFF, ap_cnt = (EINT_MAX_CHANNEL / 32), i;
    if (EINT_MAX_CHANNEL % 32)
        ap_cnt++;
    dbgmsg("[EINT] cnt:%d\n", ap_cnt);

    base = EINT_D0_EN_BASE;
    for (i = 0; i < ap_cnt; i++) {
        mt65xx_reg_sync_writel(val, base + (i * 4));
        dbgmsg("[EINT] domain addr:%x = %x\n", base, readl(IOMEM(base)));
    }
}

#ifdef MD_EINT
typedef struct{
  char  name[24];
  int   eint_num;
  int   eint_deb;
  int   eint_pol;
  int   eint_sens;
  int   socket_type;
}MD_SIM_HOTPLUG_INFO;

#define MD_SIM_MAX 16
MD_SIM_HOTPLUG_INFO md_sim_info[MD_SIM_MAX];
unsigned int md_sim_counter = 0;

int get_eint_attribute(char *name, unsigned int name_len, unsigned int type, char *result, unsigned int *len)
{
    int i;
    int ret = 0;
    int *sim_info = (int *)result;
    printk("in %s\n",__func__);
    //printk("[EINT]CUST_EINT_MD1_CNT:%d,CUST_EINT_MD2_CNT:%d\n",CUST_EINT_MD1_CNT,CUST_EINT_MD2_CNT);
    //printk("[EINT]CUST_EINT_MD1_CNT:%d",CUST_EINT_MD1_CNT);
    printk("query info: name:%s, type:%d, len:%d\n", name,type,name_len);
    if (len == NULL || name == NULL || result == NULL)
    	return ERR_SIM_HOT_PLUG_NULL_POINTER;

    for (i = 0; i < md_sim_counter; i++){
        printk("compare string:%s\n", md_sim_info[i].name);
        if (!strncmp(name, md_sim_info[i].name, name_len))
        {
            switch(type)
            {
                case SIM_HOT_PLUG_EINT_NUMBER:
                    *len = sizeof(md_sim_info[i].eint_num);
                    memcpy(sim_info, &md_sim_info[i].eint_num, *len);
                    printk("[EINT]eint_num:%d\n", md_sim_info[i].eint_num);
                    break;

                case SIM_HOT_PLUG_EINT_DEBOUNCETIME:
                    *len = sizeof(md_sim_info[i].eint_deb);
                    memcpy(sim_info, &md_sim_info[i].eint_deb, *len);
                    printk("[EINT]eint_deb:%d\n", md_sim_info[i].eint_deb);
                    break;

                case SIM_HOT_PLUG_EINT_POLARITY:
                    *len = sizeof(md_sim_info[i].eint_pol);
                    memcpy(sim_info, &md_sim_info[i].eint_pol, *len);
                    printk("[EINT]eint_pol:%d\n", md_sim_info[i].eint_pol);
                    break;

                case SIM_HOT_PLUG_EINT_SENSITIVITY:
                    *len = sizeof(md_sim_info[i].eint_sens);
                    memcpy(sim_info, &md_sim_info[i].eint_sens, *len);
                    printk("[EINT]eint_sens:%d\n", md_sim_info[i].eint_sens);
                    break;

                case SIM_HOT_PLUG_EINT_SOCKETTYPE:
                    *len = sizeof(md_sim_info[i].socket_type);
                    memcpy(sim_info, &md_sim_info[i].socket_type, *len);
                    printk("[EINT]socket_type:%d\n", md_sim_info[i].socket_type);
                    break;
  
                default:
                    ret = ERR_SIM_HOT_PLUG_QUERY_TYPE;
                    *len = sizeof(int);
                    memset(sim_info, 0xff, *len);
                    break;
            }
            return ret;
        }
    }

    *len = sizeof(int);
    memset(sim_info, 0xff, *len);
 
    return ERR_SIM_HOT_PLUG_QUERY_STRING;
}
int get_type(char *name)
{

        int type1 = 0x0;
        int type2 = 0x0;
#if defined(MTK_SIM1_SOCKET_TYPE) || defined(MTK_SIM2_SOCKET_TYPE)
    char *p;
#endif

#ifdef MTK_SIM1_SOCKET_TYPE
    p = (char *)MTK_SIM1_SOCKET_TYPE;
    type1 = simple_strtoul(p, &p, 10);
#endif
#ifdef MTK_SIM2_SOCKET_TYPE
    p = (char *)MTK_SIM2_SOCKET_TYPE;
    type2 = simple_strtoul(p, &p, 10);
#endif
    if (!strncmp(name, "MD1_SIM1_HOT_PLUG_EINT", strlen("MD1_SIM1_HOT_PLUG_EINT")))
        return type1;
    else if (!strncmp(name, "MD1_SIM1_HOT_PLUG_EINT", strlen("MD1_SIM1_HOT_PLUG_EINT")))
        return type1;
    else if (!strncmp(name, "MD2_SIM2_HOT_PLUG_EINT", strlen("MD2_SIM2_HOT_PLUG_EINT")))
        return type2;
    else if (!strncmp(name, "MD2_SIM2_HOT_PLUG_EINT", strlen("MD2_SIM2_HOT_PLUG_EINT")))
        return type2;
    else 
        return 0;
}
#endif 

static void setup_MD_eint(void)
{
#ifdef MD_EINT
 //printk("[EINT]CUST_EINT_MD1_CNT:%d,CUST_EINT_MD2_CNT:%d\n",CUST_EINT_MD1_CNT,CUST_EINT_MD2_CNT);
 //printk("[EINT]CUST_EINT_MD1_CNT:%d",CUST_EINT_MD1_CNT);

#if defined(CUST_EINT_MD1_0_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD1_0_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD1_0_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD1_0_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD1_0_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD1_0_DEBOUNCE_CN;
        printk("[EINT] MD1 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD1 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD1_1_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD1_1_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD1_1_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD1_1_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD1_1_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD1_1_DEBOUNCE_CN;
        printk("[EINT] MD1 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD1 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD1_2_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD1_2_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD1_2_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD1_2_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD1_2_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD1_2_DEBOUNCE_CN;
        printk("[EINT] MD1 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD1 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD1_3_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD1_3_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD1_3_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD1_3_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD1_3_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD1_3_DEBOUNCE_CN;
        printk("[EINT] MD1 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD1 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD1_4_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD1_4_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD1_4_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD1_4_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD1_4_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD1_4_DEBOUNCE_CN;
        printk("[EINT] MD1 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD1 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif

#if defined(CUST_EINT_MD2_0_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD2_0_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD2_0_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD2_0_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD2_0_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD2_0_DEBOUNCE_CN;
        printk("[EINT] MD2 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD2 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD2_1_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD2_1_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD2_1_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD2_1_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD2_1_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD2_1_DEBOUNCE_CN;
        printk("[EINT] MD2 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD2 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD2_2_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD2_2_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD2_2_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD2_2_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD2_2_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD2_2_DEBOUNCE_CN;
        dbgmsg("[EINT] MD2 name = %s\n", md_sim_info[md_sim_counter].name);
        dbgmsg("[EINT] MD2 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD2_3_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD2_3_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD2_3_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD2_3_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD2_3_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD2_3_DEBOUNCE_CN;
        printk("[EINT] MD2 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD2 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#if defined(CUST_EINT_MD2_4_NAME)
        sprintf(md_sim_info[md_sim_counter].name, CUST_EINT_MD2_4_NAME);
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_MD2_4_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_MD2_4_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_MD2_4_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = get_type(md_sim_info[md_sim_counter].name);
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_MD2_4_DEBOUNCE_CN;
        printk("[EINT] MD2 name = %s\n", md_sim_info[md_sim_counter].name);
        printk("[EINT] MD2 type = %d\n", md_sim_info[md_sim_counter].socket_type);
        md_sim_counter++;
#endif
#endif //MD_EINT
}

#if 0
void mt_eint_test()
{
    int cur_eint_num = 200;
    int sens = 1;
    int pol = 1;
    int is_en_db = 0;
    int is_auto_umask = 0;
    
    mt_eint_mask(cur_eint_num);
    mt_eint_set_polarity(cur_eint_num, pol);
    mt_eint_set_sens(cur_eint_num, sens);
    mt_eint_registration(cur_eint_num, EINTF_TRIGGER_HIGH, mt_eint_soft_isr, is_auto_umask);
    mt_eint_unmask(cur_eint_num);
    mt_eint_emu_set(cur_eint_num);
}
#endif

/*
 * mt_eint_print_status: Print the EINT status register.
 */
void mt_eint_print_status(void)
{
    unsigned int status,index;
    unsigned int offset,reg_base,status_check;
    printk(KERN_DEBUG"EINT_STA:");
     for (reg_base = 0; reg_base < EINT_MAX_CHANNEL; reg_base+=32) {
            /* read status register every 32 interrupts */
            status = mt_eint_get_status(reg_base);
            if(status){
                //dbgmsg(KERN_DEBUG"EINT Module - index:%d,EINT_STA = 0x%x\n",
                //  reg_base, status);
            }
            else{
                continue;
            }
            for(offset = 0; offset < 32; offset++){
                index = reg_base + offset;
                if (index >= EINT_MAX_CHANNEL) break;

                status_check = status & (1 << offset);
                if (status_check) 
                        printk(KERN_DEBUG" %d",index);
            }

    }
    printk(KERN_DEBUG"\n");
}

#if (defined(CONFIG_ARCH_MT8590_ICX))

#define	MT_EINT_DEBUG_EINT_SHOW_STATUS	(0x00000001)
static uint mt_eint_debug_icx=0x00000000;
module_param_named(debug_eint_icx, mt_eint_debug_icx, uint, S_IWUSR | S_IRUGO);

#define	IS_MT_EINT_DEBUG_ICX_SHOW_STATUS() \
	((mt_eint_debug_icx & MT_EINT_DEBUG_EINT_SHOW_STATUS) != 0)

/*! Snap the EINT status register.
*/
void mt_eint_snap_status_icx(struct icx_pm_helper_spm_stat *sps)
{	uint32_t	status;
	unsigned int	reg_base;
	unsigned int	ei;
	char		*p;
	size_t		ofs;
	unsigned	ei_actives;

	/* @note We use static buffer,
	  it may not happen reentrant calling to mt_eint_print_status_icx.
	*/
	static const char print_tag[]="%s: EINT_STATUS. non_zero_sta={";
	static const char eint_format[]="%u";
	static const char eint_delimiter[]=",";
	static const char end_of_line[]="}\n";

#if (EINT_MAX_CHANNEL >= 1000)
#error Too many EINT channels.
#endif /* (EINT_MAX_CHANNEL >= 1000) */

	static char buf[
		 sizeof(print_tag)
		+sizeof(__func__)+32
		+(3+sizeof(eint_delimiter))*(EINT_MAX_CHANNEL+31)/32
		+sizeof(end_of_line)
		];

	p = buf;
	*p = 0;
	ofs = 0;
	if (IS_MT_EINT_DEBUG_ICX_SHOW_STATUS()) {
		ofs += snprintf(p, sizeof(buf)-ofs, print_tag, __func__);
	}
	reg_base = 0;
	ei = 0;
	ei_actives = 0;
	while ((reg_base < EINT_MAX_CHANNEL) && (ofs < sizeof(buf))) {
		/* read status register every 32 interrupts */

		status = mt_eint_get_status(reg_base);
		sps->eint_sta[ei] = status;
		if ((status) && (IS_MT_EINT_DEBUG_ICX_SHOW_STATUS())) {
			/* Some interrups are pending, and show EINT status. */
			size_t		remains;
			unsigned int	bit;
			bit = 0;
			while (bit < 32) {
				/* test every factor bits. */
				if ((status & 0x00000001) != 0) {
					/* come on interrupt. */
					remains = sizeof(buf)-ofs;
					if (ei_actives > 0) {
						ofs += snprintf(p+ofs, remains, "%s", eint_delimiter);
						remains = sizeof(buf)-ofs;
						if (ofs >= sizeof(buf)) {
							remains = 0;
							break;
						}
					}
					ofs += snprintf(p+ofs, remains, eint_format, reg_base + bit);
					ei_actives++;
				}
				status>>=1;
				bit++;
			}
		}
		ei++;
		reg_base += 32;
	}
	if (IS_MT_EINT_DEBUG_ICX_SHOW_STATUS()) {
		snprintf(p+ofs, sizeof(buf)-ofs, "%s", end_of_line);
		printk(buf);
	}
}
#endif /* (defined(CONFIG_ARCH_MT8590_ICX)) */

#ifdef DEINT_SUPPORT
void mt_eint_test(void)

{
    printk("test for eint re-registration\n");
}

static irqreturn_t  mt_deint_test_isr(int irq, void *dev_id)
{
    printk("test for deint isr\n");
    //mt_irq_is_active(irq);
}
#endif

/*
 * mt_eint_init: initialize EINT driver.
 * Always return 0.
 */
static int __init mt_eint_init(void)
{
    unsigned int i;
    struct mt_eint_driver *eint_drv;

    /* assign to domain 0 for AP */
    mt_eint_setdomain0();

    wake_lock_init(&EINT_suspend_lock, WAKE_LOCK_SUSPEND, "EINT wakelock");

    setup_MD_eint();
    for (i = 0; i < EINT_MAX_CHANNEL; i++) {
        EINT_FUNC.eint_func[i] = NULL;
        EINT_FUNC.is_deb_en[i] = 0;
        EINT_FUNC.deb_time[i] = 0;
        #if EINT_USE_PRECISE_HRTIMER
        EINT_FUNC.eint_sw_deb_hr_timer[i].eint_num = EINT_MAX_CHANNEL; //init for unused 
        #if EINT_TEST_LATENCY
        if (24 == i)
        {
            printk(KERN_ERR "EINT init, eint 24 timer: 0x%x\n", &(EINT_FUNC.eint_sw_deb_hr_timer[i]));
        }
        #endif
        #else
        EINT_FUNC.eint_sw_deb_timer[i].expires = 0;
        EINT_FUNC.eint_sw_deb_timer[i].data = 0;
        EINT_FUNC.eint_sw_deb_timer[i].function = NULL;
        #endif
#if defined(EINT_TEST)
        EINT_FUNC.softisr_called[i] = 0;
#endif
    }

    if (request_irq(EINT_IRQ, mt_eint_isr, IRQF_TRIGGER_HIGH, "EINT", NULL)) {
        printk(KERN_ERR "EINT IRQ LINE NOT AVAILABLE!!\n");
    }

    /* register EINT driver */
    eint_drv = get_mt_eint_drv();
    eint_drv->eint_max_channel = mt_eint_max_channel;
    eint_drv->enable = mt_eint_unmask;
    eint_drv->disable = mt_eint_mask;
    eint_drv->is_disable = mt_eint_get_mask;
    eint_drv->get_sens =  mt_eint_get_sens;
    eint_drv->set_sens = mt_eint_set_sens;
    eint_drv->get_polarity = mt_eint_get_polarity;
    eint_drv->set_polarity = mt_eint_set_polarity;
    eint_drv->get_debounce_cnt =  mt_eint_get_debounce_cnt;
    eint_drv->set_debounce_cnt = mt_eint_set_hw_debounce;
    eint_drv->is_debounce_en = mt_eint_is_debounce_en;
    eint_drv->enable_debounce = mt_eint_enable_debounce;
    eint_drv->disable_debounce = mt_eint_disable_debounce;

    /* Test */ 
    //mt_eint_registration(11, EINTF_TRIGGER_LOW, mt_eint_test, true);
    //mt_deint_registration(11, EINTF_TRIGGER_HIGH, mt_deint_test_isr);
    

    return 0;
}

#if(EINT_DEBUG == 1)
void mt_eint_dump_status(unsigned int eint)
{
   if (eint >= EINT_MAX_CHANNEL)
       return;
   printk("[EINT] eint:%d,mask:%x,pol:%x,deb:%x,sens:%x\n",eint,mt_eint_get_mask(eint),mt_eint_get_polarity(eint),mt_eint_get_debounce_cnt(eint),mt_eint_get_sens(eint));
}
#endif

arch_initcall(mt_eint_init);

EXPORT_SYMBOL(mt_eint_dis_debounce);
EXPORT_SYMBOL(mt_eint_registration);
EXPROT_SYMBOL(mt_deint_registration);
EXPORT_SYMBOL(mt_eint_set_hw_debounce);
EXPORT_SYMBOL(mt_eint_set_polarity);
EXPORT_SYMBOL(mt_eint_get_polarity);
EXPORT_SYMBOL(mt_eint_set_sens);
EXPORT_SYMBOL(mt_eint_mask);
EXPORT_SYMBOL(mt_eint_unmask);
EXPORT_SYMBOL(mt_eint_print_status);
EXPORT_SYSMBOL(mt_eint_dump_status);
EXPORT_SYSMBOL(mt_eint_ack);

#if defined(EINT_TEST)
EXPORT_SYMBOL(mt_eint_soft_set);
#endif
