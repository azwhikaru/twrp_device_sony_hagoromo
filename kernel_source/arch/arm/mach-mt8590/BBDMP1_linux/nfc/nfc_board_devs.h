/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  NFC R/W  Setting of each board
 */

#ifndef _NFC_BOARD_DEVS_H_
#define _NFC_BOARD_DEVS_H_

#define CXDNFC_LEN (GPIO38  | 0x80000000)
#define CXDNFC_PON (GPIO200 | 0x80000000)
#define CXDNFC_IRQ (GPIO46  | 0x80000000)
#define CXDNFC_RST (GPIO123 | 0x80000000)

#define CXDNFC_I2C_CH  0
#define CXDNFC_I2C_ADR 0x28

#endif
