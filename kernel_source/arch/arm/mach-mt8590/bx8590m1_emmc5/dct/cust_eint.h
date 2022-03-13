/*
 * Generated by MTK SP DrvGen Version 03.13.6 for MT8590. Copyright MediaTek Inc. (C) 2013.
 * Mon Apr 27 15:13:49 2015
 * Do Not Modify the File.
 */

#ifndef __CUST_EINTH
#define __CUST_EINTH
#ifdef __cplusplus
extern "C" {
#endif
#define CUST_EINTF_TRIGGER_RISING     			1    //High Polarity and Edge Sensitive
#define CUST_EINTF_TRIGGER_FALLING    			2    //Low Polarity and Edge Sensitive
#define CUST_EINTF_TRIGGER_HIGH      				4    //High Polarity and Level Sensitive
#define CUST_EINTF_TRIGGER_LOW       				8    //Low Polarity and Level Sensitive
#define CUST_EINT_DEBOUNCE_DISABLE          0
#define CUST_EINT_DEBOUNCE_ENABLE           1
//////////////////////////////////////////////////////////////////////////////


#define CUST_EINT_WIFI_NUM              0
#define CUST_EINT_WIFI_DEBOUNCE_CN      0
#define CUST_EINT_WIFI_TYPE							CUST_EINTF_TRIGGER_HIGH
#define CUST_EINT_WIFI_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE

#define CUST_EINT_COMBO_BGF_NUM              1
#define CUST_EINT_COMBO_BGF_DEBOUNCE_CN      0
#define CUST_EINT_COMBO_BGF_TYPE							CUST_EINTF_TRIGGER_HIGH
#define CUST_EINT_COMBO_BGF_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE

#define CUST_EINT_COMBO_ALL_NUM              2
#define CUST_EINT_COMBO_ALL_DEBOUNCE_CN      0
#define CUST_EINT_COMBO_ALL_TYPE							CUST_EINTF_TRIGGER_HIGH
#define CUST_EINT_COMBO_ALL_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE

#define CUST_EINT_MC1_DET_NUM              3
#define CUST_EINT_MC1_DET_DEBOUNCE_CN      1
#define CUST_EINT_MC1_DET_TYPE							CUST_EINTF_TRIGGER_LOW
#define CUST_EINT_MC1_DET_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE

#define CUST_EINT_MSDC2_INS_NUM              4
#define CUST_EINT_MSDC2_INS_DEBOUNCE_CN      1
#define CUST_EINT_MSDC2_INS_TYPE							CUST_EINTF_TRIGGER_LOW
#define CUST_EINT_MSDC2_INS_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE

#define CUST_EINT_ACCDET_NUM              5
#define CUST_EINT_ACCDET_DEBOUNCE_CN      256
#define CUST_EINT_ACCDET_TYPE							CUST_EINTF_TRIGGER_LOW
#define CUST_EINT_ACCDET_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE

#define CUST_EINT_MC1_WP_NUM              7
#define CUST_EINT_MC1_WP_DEBOUNCE_CN      1
#define CUST_EINT_MC1_WP_TYPE							CUST_EINTF_TRIGGER_LOW
#define CUST_EINT_MC1_WP_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE

#define CUST_EINT_IRQ_NFC_NUM              75
#define CUST_EINT_IRQ_NFC_DEBOUNCE_CN      0
#define CUST_EINT_IRQ_NFC_TYPE							CUST_EINTF_TRIGGER_HIGH
#define CUST_EINT_IRQ_NFC_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE

#define CUST_EINT_NFC_NUM              76
#define CUST_EINT_NFC_DEBOUNCE_CN      0
#define CUST_EINT_NFC_TYPE							CUST_EINTF_TRIGGER_HIGH
#define CUST_EINT_NFC_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE

#define CUST_EINT_MT6323_PMIC_NUM              150
#define CUST_EINT_MT6323_PMIC_DEBOUNCE_CN      1
#define CUST_EINT_MT6323_PMIC_TYPE							CUST_EINTF_TRIGGER_HIGH
#define CUST_EINT_MT6323_PMIC_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE

#define CUST_EINT_TOUCH_PANEL_NUM              154
#define CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN      0
#define CUST_EINT_TOUCH_PANEL_TYPE							CUST_EINTF_TRIGGER_FALLING
#define CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE



//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif //_CUST_EINT_H


