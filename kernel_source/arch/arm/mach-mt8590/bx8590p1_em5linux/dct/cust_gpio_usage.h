/*
 * Generated by MTK SP DrvGen Version 03.13.6 for MT8590. Copyright MediaTek Inc. (C) 2013.
 * Thu Jul 23 09:53:14 2015
 * Do Not Modify the File.
 */

#ifndef __CUST_GPIO_USAGE_H__
#define __CUST_GPIO_USAGE_H__


#define GPIO_PWRAP_SPI0_MI_PIN         (GPIO0 | 0x80000000)
#define GPIO_PWRAP_SPI0_MI_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_PWRAP_SPI0_MI_PIN_M_PWRAP_SPIDO   GPIO_MODE_01

#define GPIO_PWRAP_SPI0_MO_PIN         (GPIO1 | 0x80000000)
#define GPIO_PWRAP_SPI0_MO_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_PWRAP_SPI0_MO_PIN_M_PWRAP_SPIDI   GPIO_MODE_01

#define GPIO_CTP_RST_PIN         (GPIO5 | 0x80000000)
#define GPIO_CTP_RST_PIN_M_GPIO   GPIO_MODE_00

#define GPIO_CTP_EINT_PIN         (GPIO6 | 0x80000000)
#define GPIO_CTP_EINT_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_CTP_EINT_PIN_M_EINT   GPIO_CTP_EINT_PIN_M_GPIO

#define GPIO_SRCLKENA_PIN         (GPIO12 | 0x80000000)
#define GPIO_SRCLKENA_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_SRCLKENA_PIN_M_CLK   GPIO_MODE_01

#define GPIO_NFC_OSC_EN_PIN         (GPIO13 | 0x80000000)
#define GPIO_NFC_OSC_EN_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_NFC_OSC_EN_PIN_M_CLK   GPIO_MODE_01

#define GPIO_WIFI_EINT_PIN         (GPIO23 | 0x80000000)
#define GPIO_WIFI_EINT_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_WIFI_EINT_PIN_M_KCOL   GPIO_MODE_03
#define GPIO_WIFI_EINT_PIN_M_EINT   GPIO_WIFI_EINT_PIN_M_GPIO

#define GPIO_GPS_LNA_PIN         (GPIO24 | 0x80000000)
#define GPIO_GPS_LNA_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_GPS_LNA_PIN_M_KCOL   GPIO_MODE_03
#define GPIO_GPS_LNA_PIN_M_EINT   GPIO_GPS_LNA_PIN_M_GPIO

#define GPIO_ACCDET_EINT_PIN         (GPIO27 | 0x80000000)
#define GPIO_ACCDET_EINT_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_ACCDET_EINT_PIN_M_KROW   GPIO_MODE_03
#define GPIO_ACCDET_EINT_PIN_M_EINT   GPIO_ACCDET_EINT_PIN_M_GPIO

#define GPIO_OTG_IDDIG_EINT_PIN         (GPIO28 | 0x80000000)
#define GPIO_OTG_IDDIG_EINT_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_OTG_IDDIG_EINT_PIN_M_KROW   GPIO_MODE_03
#define GPIO_OTG_IDDIG_EINT_PIN_M_EINT   GPIO_OTG_IDDIG_EINT_PIN_M_GPIO

#define GPIO_UART_URXD1_PIN         (GPIO81 | 0x80000000)
#define GPIO_UART_URXD1_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_UART_URXD1_PIN_M_URXD   GPIO_MODE_01

#define GPIO_UART_UTXD1_PIN         (GPIO82 | 0x80000000)
#define GPIO_UART_UTXD1_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_UART_UTXD1_PIN_M_UTXD   GPIO_MODE_01

#define GPIO_NFC_VENB_PIN         (GPIO101 | 0x80000000)
#define GPIO_NFC_VENB_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_NFC_VENB_PIN_M_KROW   GPIO_MODE_04

#define GPIO_IRQ_NFC_PIN         (GPIO102 | 0x80000000)
#define GPIO_IRQ_NFC_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_IRQ_NFC_PIN_M_KROW   GPIO_MODE_04
#define GPIO_IRQ_NFC_PIN_M_EINT   GPIO_IRQ_NFC_PIN_M_GPIO

#define GPIO_NFC_EINT_PIN         (GPIO103 | 0x80000000)
#define GPIO_NFC_EINT_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_NFC_EINT_PIN_M_KROW   GPIO_MODE_04
#define GPIO_NFC_EINT_PIN_M_EINT   GPIO_NFC_EINT_PIN_M_GPIO

#define GPIO_NFC_RST_PIN         (GPIO104 | 0x80000000)
#define GPIO_NFC_RST_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_NFC_RST_PIN_M_KROW   GPIO_MODE_04

#define GPIO_LCM_PWR         (GPIO203 | 0x80000000)
#define GPIO_LCM_PWR_M_GPIO   GPIO_MODE_00
#define GPIO_LCM_PWR_M_PWM   GPIO_MODE_01

#define GPIO_LCD_BIAS_ENP_PIN         (GPIO204 | 0x80000000)
#define GPIO_LCD_BIAS_ENP_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_LCD_BIAS_ENP_PIN_M_CLK   GPIO_MODE_02
#define GPIO_LCD_BIAS_ENP_PIN_M_PWM   GPIO_MODE_01
#define GPIO_LCD_BIAS_ENP_PIN_CLK     CLK_OUT3
#define GPIO_LCD_BIAS_ENP_PIN_FREQ    GPIO_CLKSRC_NONE

#define GPIO_COMBO_PMU_EN_PIN         (GPIO206 | 0x80000000)
#define GPIO_COMBO_PMU_EN_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_COMBO_PMU_EN_PIN_M_CLK   GPIO_MODE_02
#define GPIO_COMBO_PMU_EN_PIN_M_PWM   GPIO_MODE_01
#define GPIO_COMBO_PMU_EN_PIN_CLK     CLK_OUT1
#define GPIO_COMBO_PMU_EN_PIN_FREQ    GPIO_CLKSRC_NONE

#define GPIO_COMBO_RST_PIN         (GPIO207 | 0x80000000)
#define GPIO_COMBO_RST_PIN_M_GPIO   GPIO_MODE_00
#define GPIO_COMBO_RST_PIN_M_CLK   GPIO_MODE_02
#define GPIO_COMBO_RST_PIN_M_PWM   GPIO_MODE_01
#define GPIO_COMBO_RST_PIN_CLK     CLK_OUT0
#define GPIO_COMBO_RST_PIN_FREQ    GPIO_CLKSRC_NONE


/*Output for default variable names*/
/*@XXX_XX_PIN in gpio.cmp          */



#endif /* __CUST_GPIO_USAGE_H__ */


