/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 *  NFC R/W  Setting of each board
 */

#ifndef _RCS730_BOARD_DEVS_H_
#define _RCS730_BOARD_DEVS_H_

#define CXDNFC_MAGIC 'S'

#define CXDNFC_RFDET (GPIO26 | 0x80000000)
#define CXDNFC_IRQ (GPIO243 | 0x80000000)

#define CXDNFC_I2C_CH  0
#define CXDNFC_I2C_ADR 0x40

#endif
