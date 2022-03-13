/*
 * Copyright 2011,2012,2013,2014,2015,2016 Sony Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef _ICX_NVP_H_
#define _ICX_NVP_H_

typedef struct {
	int           sector;
	int           page;
	unsigned char oob[256];
	unsigned char dummy[12];
	unsigned char data[4096];
}icx_nvp_ioc_t;

#define ICX_NVP_IOC_MAGIC 'n'

/* Common to NVP for eMMC */
#define ICX_NVP_IOC_SHOW_STAT    _IOW (ICX_NVP_IOC_MAGIC,0,int)
#define ICX_NVP_IOC_ERASE_ALL    _IO  (ICX_NVP_IOC_MAGIC,1)
#define ICX_NVP_IOC_WRITE_SECTOR _IOW (ICX_NVP_IOC_MAGIC,2,icx_nvp_ioc_t *)
#define ICX_NVP_IOC_READ_SECTOR  _IOWR(ICX_NVP_IOC_MAGIC,3,icx_nvp_ioc_t *)
/* Only for NVP for NAND */
#define ICX_NVP_IOC_GET_KSB       _IOR (ICX_NVP_IOC_MAGIC, 4, int)
#define ICX_NVP_IOC_GET_KBC       _IOR (ICX_NVP_IOC_MAGIC, 5, int)
#define ICX_NVP_IOC_GET_PSB       _IOR (ICX_NVP_IOC_MAGIC, 6, int)
#define ICX_NVP_IOC_GET_PBC       _IOR (ICX_NVP_IOC_MAGIC, 7, int)
#define ICX_NVP_IOC_GET_NSB       _IOR (ICX_NVP_IOC_MAGIC, 8, int)
#define ICX_NVP_IOC_GET_NBC       _IOR (ICX_NVP_IOC_MAGIC, 9, int)
#define ICX_NVP_IOC_GET_CSB       _IOR (ICX_NVP_IOC_MAGIC,10, int)
#define ICX_NVP_IOC_GET_NAND_INFO _IOWR(ICX_NVP_IOC_MAGIC,12, unsigned long)
#define ICX_NVP_IOC_ERASE         _IOW (ICX_NVP_IOC_MAGIC, 13, icx_nvp_ioc_t *)

#define ICX_NVP_IOC_MAXNR 14

extern int icx_nvp_write_data_wrapper(int zn, unsigned char *buf, int size);
extern int icx_nvp_read_data_wrapper(int zn, unsigned char *buf, int size);

int icx_nvp_write_data(int zn, unsigned char *buf, int size);
int icx_nvp_read_data(int zn, unsigned char *buf, int size);

#define icx_nvp_write_syi(_a,_b) icx_nvp_write_data(1,(_a),(_b))
#define icx_nvp_read_syi(_a,_b)  icx_nvp_read_data(1,(_a),(_b))

#define icx_nvp_write_ubp(_a,_b) icx_nvp_write_data(2,(_a),(_b))
#define icx_nvp_read_ubp(_a,_b)  icx_nvp_read_data(2,(_a),(_b))

#define icx_nvp_write_fup(_a,_b) icx_nvp_write_data(3,(_a),(_b))
#define icx_nvp_read_fup(_a,_b)  icx_nvp_read_data(3,(_a),(_b))

#define icx_nvp_write_prk(_a,_b) icx_nvp_write_data(4,(_a),(_b))
#define icx_nvp_read_prk(_a,_b)  icx_nvp_read_data(4,(_a),(_b))

#define icx_nvp_write_hld(_a,_b) icx_nvp_write_data(5,(_a),(_b))
#define icx_nvp_read_hld(_a,_b)  icx_nvp_read_data(5,(_a),(_b))

#define icx_nvp_write_rtc(_a,_b) icx_nvp_write_data(6,(_a),(_b))
#define icx_nvp_read_rtc(_a,_b)  icx_nvp_read_data(6,(_a),(_b))

#define icx_nvp_write_mid(_a,_b) icx_nvp_write_data(7,(_a),(_b))
#define icx_nvp_read_mid(_a,_b)  icx_nvp_read_data(7,(_a),(_b))

#define icx_nvp_write_pcd(_a,_b) icx_nvp_write_data(8,(_a),(_b))
#define icx_nvp_read_pcd(_a,_b)  icx_nvp_read_data(8,(_a),(_b))

#define icx_nvp_write_ser(_a,_b) icx_nvp_write_data(9,(_a),(_b))
#define icx_nvp_read_ser(_a,_b)  icx_nvp_read_data(9,(_a),(_b))

#define icx_nvp_write_ufn(_a,_b) icx_nvp_write_data(10,(_a),(_b))
#define icx_nvp_read_ufn(_a,_b)  icx_nvp_read_data(10,(_a),(_b))

#define icx_nvp_write_kas(_a,_b) icx_nvp_write_data(11,(_a),(_b))
#define icx_nvp_read_kas(_a,_b)  icx_nvp_read_data(11,(_a),(_b))

#define icx_nvp_write_shp(_a,_b) icx_nvp_write_data(12,(_a),(_b))
#define icx_nvp_read_shp(_a,_b)  icx_nvp_read_data(12,(_a),(_b))

#define icx_nvp_write_tst(_a,_b) icx_nvp_write_data(13,(_a),(_b))
#define icx_nvp_read_tst(_a,_b)  icx_nvp_read_data(13,(_a),(_b))

#define icx_nvp_write_gty(_a,_b) icx_nvp_write_data(14,(_a),(_b))
#define icx_nvp_read_gty(_a,_b)  icx_nvp_read_data(14,(_a),(_b))

#define icx_nvp_write_fmp(_a,_b) icx_nvp_write_data(15,(_a),(_b))
#define icx_nvp_read_fmp(_a,_b)  icx_nvp_read_data(15,(_a),(_b))

#define icx_nvp_write_sdp(_a,_b) icx_nvp_write_data(16,(_a),(_b))
#define icx_nvp_read_sdp(_a,_b)  icx_nvp_read_data(16,(_a),(_b))

#define icx_nvp_write_ncp(_a,_b) icx_nvp_write_data(17,(_a),(_b))
#define icx_nvp_read_ncp(_a,_b)  icx_nvp_read_data(17,(_a),(_b))

#define icx_nvp_write_psk(_a,_b) icx_nvp_write_data(18,(_a),(_b))
#define icx_nvp_read_psk(_a,_b)  icx_nvp_read_data(18,(_a),(_b))

#define icx_nvp_write_nvr(_a,_b) icx_nvp_write_data(77,(_a),(_b))
#define icx_nvp_read_nvr(_a,_b)  icx_nvp_read_data(77,(_a),(_b))

#define icx_nvp_write_she(_a,_b) icx_nvp_write_data(84,(_a),(_b))
#define icx_nvp_read_she(_a,_b)  icx_nvp_read_data(84,(_a),(_b))

#define icx_nvp_write_btc(_a,_b) icx_nvp_write_data(85,(_a),(_b))
#define icx_nvp_read_btc(_a,_b)  icx_nvp_read_data(85,(_a),(_b))

#define icx_nvp_write_ins(_a,_b) icx_nvp_write_data(89,(_a),(_b))
#define icx_nvp_read_ins(_a,_b)  icx_nvp_read_data(89,(_a),(_b))

#define icx_nvp_write_ctr(_a,_b) icx_nvp_write_data(90,(_a),(_b))
#define icx_nvp_read_ctr(_a,_b)  icx_nvp_read_data(90,(_a),(_b))

#define icx_nvp_write_sku(_a,_b) icx_nvp_write_data(91,(_a),(_b))
#define icx_nvp_read_sku(_a,_b)  icx_nvp_read_data(91,(_a),(_b))

#define icx_nvp_write_bpr(_a,_b) icx_nvp_write_data(19,(_a),(_b))
#define icx_nvp_read_bpr(_a,_b)  icx_nvp_read_data(19,(_a),(_b))

#define icx_nvp_write_bfp(_a,_b) icx_nvp_write_data(20,(_a),(_b))
#define icx_nvp_read_bfp(_a,_b)  icx_nvp_read_data(20,(_a),(_b))

#define icx_nvp_write_bfd(_a,_b) icx_nvp_write_data(21,(_a),(_b))
#define icx_nvp_read_bfd(_a,_b)  icx_nvp_read_data(21,(_a),(_b))

#define icx_nvp_write_bml(_a,_b) icx_nvp_write_data(22,(_a),(_b))
#define icx_nvp_read_bml(_a,_b)  icx_nvp_read_data(22,(_a),(_b))

#define icx_nvp_write_apd(_a,_b) icx_nvp_write_data(78,(_a),(_b))
#define icx_nvp_read_apd(_a,_b)  icx_nvp_read_data(78,(_a),(_b))

#define icx_nvp_write_blf(_a,_b) icx_nvp_write_data(79,(_a),(_b))
#define icx_nvp_read_blf(_a,_b)  icx_nvp_read_data(79,(_a),(_b))

#define icx_nvp_write_slp(_a,_b) icx_nvp_write_data(80,(_a),(_b))
#define icx_nvp_read_slp(_a,_b)  icx_nvp_read_data(80,(_a),(_b))

#define icx_nvp_write_vrt(_a,_b) icx_nvp_write_data(81,(_a),(_b))
#define icx_nvp_read_vrt(_a,_b)  icx_nvp_read_data(81,(_a),(_b))

#define icx_nvp_write_fni(_a,_b) icx_nvp_write_data(82,(_a),(_b))
#define icx_nvp_read_fni(_a,_b)  icx_nvp_read_data(82,(_a),(_b))

#define icx_nvp_write_sid(_a,_b) icx_nvp_write_data(83,(_a),(_b))
#define icx_nvp_read_sid(_a,_b)  icx_nvp_read_data(83,(_a),(_b))

#define icx_nvp_write_mso(_a,_b) icx_nvp_write_data(86,(_a),(_b))
#define icx_nvp_read_mso(_a,_b)  icx_nvp_read_data(86,(_a),(_b))

#define icx_nvp_write_cng(_a,_b) icx_nvp_write_data(23,(_a),(_b))
#define icx_nvp_read_cng(_a,_b)  icx_nvp_read_data(23,(_a),(_b))

#define icx_nvp_write_lyr(_a,_b) icx_nvp_write_data(24,(_a),(_b))
#define icx_nvp_read_lyr(_a,_b)  icx_nvp_read_data(24,(_a),(_b))

#define icx_nvp_write_dbv(_a,_b) icx_nvp_write_data(25,(_a),(_b))
#define icx_nvp_read_dbv(_a,_b)  icx_nvp_read_data(25,(_a),(_b))

#define icx_nvp_write_fur(_a,_b) icx_nvp_write_data(26,(_a),(_b))
#define icx_nvp_read_fur(_a,_b)  icx_nvp_read_data(26,(_a),(_b))

#define icx_nvp_write_ums(_a,_b) icx_nvp_write_data(27,(_a),(_b))
#define icx_nvp_read_ums(_a,_b)  icx_nvp_read_data(27,(_a),(_b))

#define icx_nvp_write_skd(_a,_b) icx_nvp_write_data(28,(_a),(_b))
#define icx_nvp_read_skd(_a,_b)  icx_nvp_read_data(28,(_a),(_b))

#define icx_nvp_write_ups(_a,_b) icx_nvp_write_data(29,(_a),(_b))
#define icx_nvp_read_ups(_a,_b)  icx_nvp_read_data(29,(_a),(_b))

#define icx_nvp_write_mdk(_a,_b) icx_nvp_write_data(30,(_a),(_b))
#define icx_nvp_read_mdk(_a,_b)  icx_nvp_read_data(30,(_a),(_b))

#define icx_nvp_write_fvi(_a,_b) icx_nvp_write_data(31,(_a),(_b))
#define icx_nvp_read_fvi(_a,_b)  icx_nvp_read_data(31,(_a),(_b))

#define icx_nvp_write_mac(_a,_b) icx_nvp_write_data(32,(_a),(_b))
#define icx_nvp_read_mac(_a,_b)  icx_nvp_read_data(32,(_a),(_b))

#define icx_nvp_write_fpi(_a,_b) icx_nvp_write_data(33,(_a),(_b))
#define icx_nvp_read_fpi(_a,_b)  icx_nvp_read_data(33,(_a),(_b))

#define icx_nvp_write_tr0(_a,_b) icx_nvp_write_data(34,(_a),(_b))
#define icx_nvp_read_tr0(_a,_b)  icx_nvp_read_data(34,(_a),(_b))

#define icx_nvp_write_tr1(_a,_b) icx_nvp_write_data(35,(_a),(_b))
#define icx_nvp_read_tr1(_a,_b)  icx_nvp_read_data(35,(_a),(_b))

#define icx_nvp_write_e00(_a,_b) icx_nvp_write_data(36,(_a),(_b))
#define icx_nvp_read_e00(_a,_b)  icx_nvp_read_data(36,(_a),(_b))

#define icx_nvp_write_e01(_a,_b) icx_nvp_write_data(37,(_a),(_b))
#define icx_nvp_read_e01(_a,_b)  icx_nvp_read_data(37,(_a),(_b))

#define icx_nvp_write_e02(_a,_b) icx_nvp_write_data(38,(_a),(_b))
#define icx_nvp_read_e02(_a,_b)  icx_nvp_read_data(38,(_a),(_b))

#define icx_nvp_write_e03(_a,_b) icx_nvp_write_data(39,(_a),(_b))
#define icx_nvp_read_e03(_a,_b)  icx_nvp_read_data(39,(_a),(_b))

#define icx_nvp_write_e04(_a,_b) icx_nvp_write_data(40,(_a),(_b))
#define icx_nvp_read_e04(_a,_b)  icx_nvp_read_data(40,(_a),(_b))

#define icx_nvp_write_e05(_a,_b) icx_nvp_write_data(41,(_a),(_b))
#define icx_nvp_read_e05(_a,_b)  icx_nvp_read_data(41,(_a),(_b))

#define icx_nvp_write_e06(_a,_b) icx_nvp_write_data(42,(_a),(_b))
#define icx_nvp_read_e06(_a,_b)  icx_nvp_read_data(42,(_a),(_b))

#define icx_nvp_write_e07(_a,_b) icx_nvp_write_data(43,(_a),(_b))
#define icx_nvp_read_e07(_a,_b)  icx_nvp_read_data(43,(_a),(_b))

#define icx_nvp_write_e08(_a,_b) icx_nvp_write_data(44,(_a),(_b))
#define icx_nvp_read_e08(_a,_b)  icx_nvp_read_data(44,(_a),(_b))

#define icx_nvp_write_e09(_a,_b) icx_nvp_write_data(45,(_a),(_b))
#define icx_nvp_read_e09(_a,_b)  icx_nvp_read_data(45,(_a),(_b))

#define icx_nvp_write_e10(_a,_b) icx_nvp_write_data(46,(_a),(_b))
#define icx_nvp_read_e10(_a,_b)  icx_nvp_read_data(46,(_a),(_b))

#define icx_nvp_write_e11(_a,_b) icx_nvp_write_data(47,(_a),(_b))
#define icx_nvp_read_e11(_a,_b)  icx_nvp_read_data(47,(_a),(_b))

#define icx_nvp_write_e12(_a,_b) icx_nvp_write_data(48,(_a),(_b))
#define icx_nvp_read_e12(_a,_b)  icx_nvp_read_data(48,(_a),(_b))

#define icx_nvp_write_e13(_a,_b) icx_nvp_write_data(49,(_a),(_b))
#define icx_nvp_read_e13(_a,_b)  icx_nvp_read_data(49,(_a),(_b))

#define icx_nvp_write_e14(_a,_b) icx_nvp_write_data(50,(_a),(_b))
#define icx_nvp_read_e14(_a,_b)  icx_nvp_read_data(50,(_a),(_b))

#define icx_nvp_write_e15(_a,_b) icx_nvp_write_data(51,(_a),(_b))
#define icx_nvp_read_e15(_a,_b)  icx_nvp_read_data(51,(_a),(_b))

#define icx_nvp_write_e16(_a,_b) icx_nvp_write_data(52,(_a),(_b))
#define icx_nvp_read_e16(_a,_b)  icx_nvp_read_data(52,(_a),(_b))

#define icx_nvp_write_e17(_a,_b) icx_nvp_write_data(53,(_a),(_b))
#define icx_nvp_read_e17(_a,_b)  icx_nvp_read_data(53,(_a),(_b))

#define icx_nvp_write_e18(_a,_b) icx_nvp_write_data(54,(_a),(_b))
#define icx_nvp_read_e18(_a,_b)  icx_nvp_read_data(54,(_a),(_b))

#define icx_nvp_write_e19(_a,_b) icx_nvp_write_data(55,(_a),(_b))
#define icx_nvp_read_e19(_a,_b)  icx_nvp_read_data(55,(_a),(_b))

#define icx_nvp_write_e20(_a,_b) icx_nvp_write_data(56,(_a),(_b))
#define icx_nvp_read_e20(_a,_b)  icx_nvp_read_data(56,(_a),(_b))

#define icx_nvp_write_e21(_a,_b) icx_nvp_write_data(57,(_a),(_b))
#define icx_nvp_read_e21(_a,_b)  icx_nvp_read_data(57,(_a),(_b))

#define icx_nvp_write_e22(_a,_b) icx_nvp_write_data(58,(_a),(_b))
#define icx_nvp_read_e22(_a,_b)  icx_nvp_read_data(58,(_a),(_b))

#define icx_nvp_write_e23(_a,_b) icx_nvp_write_data(59,(_a),(_b))
#define icx_nvp_read_e23(_a,_b)  icx_nvp_read_data(59,(_a),(_b))

#define icx_nvp_write_e24(_a,_b) icx_nvp_write_data(60,(_a),(_b))
#define icx_nvp_read_e24(_a,_b)  icx_nvp_read_data(60,(_a),(_b))

#define icx_nvp_write_e25(_a,_b) icx_nvp_write_data(61,(_a),(_b))
#define icx_nvp_read_e25(_a,_b)  icx_nvp_read_data(61,(_a),(_b))

#define icx_nvp_write_e26(_a,_b) icx_nvp_write_data(62,(_a),(_b))
#define icx_nvp_read_e26(_a,_b)  icx_nvp_read_data(62,(_a),(_b))

#define icx_nvp_write_e27(_a,_b) icx_nvp_write_data(63,(_a),(_b))
#define icx_nvp_read_e27(_a,_b)  icx_nvp_read_data(63,(_a),(_b))

#define icx_nvp_write_e28(_a,_b) icx_nvp_write_data(64,(_a),(_b))
#define icx_nvp_read_e28(_a,_b)  icx_nvp_read_data(64,(_a),(_b))

#define icx_nvp_write_e29(_a,_b) icx_nvp_write_data(65,(_a),(_b))
#define icx_nvp_read_e29(_a,_b)  icx_nvp_read_data(65,(_a),(_b))

#define icx_nvp_write_e30(_a,_b) icx_nvp_write_data(66,(_a),(_b))
#define icx_nvp_read_e30(_a,_b)  icx_nvp_read_data(66,(_a),(_b))

#define icx_nvp_write_e31(_a,_b) icx_nvp_write_data(67,(_a),(_b))
#define icx_nvp_read_e31(_a,_b)  icx_nvp_read_data(67,(_a),(_b))

#define icx_nvp_write_clv(_a,_b) icx_nvp_write_data(68,(_a),(_b))
#define icx_nvp_read_clv(_a,_b)  icx_nvp_read_data(68,(_a),(_b))

#define icx_nvp_write_sps(_a,_b) icx_nvp_write_data(69,(_a),(_b))
#define icx_nvp_read_sps(_a,_b)  icx_nvp_read_data(69,(_a),(_b))

#define icx_nvp_write_rbt(_a,_b) icx_nvp_write_data(70,(_a),(_b))
#define icx_nvp_read_rbt(_a,_b)  icx_nvp_read_data(70,(_a),(_b))

#define icx_nvp_write_edw(_a,_b) icx_nvp_write_data(71,(_a),(_b))
#define icx_nvp_read_edw(_a,_b)  icx_nvp_read_data(71,(_a),(_b))

#define icx_nvp_write_bti(_a,_b) icx_nvp_write_data(72,(_a),(_b))
#define icx_nvp_read_bti(_a,_b)  icx_nvp_read_data(72,(_a),(_b))

#define icx_nvp_write_hdi(_a,_b) icx_nvp_write_data(73,(_a),(_b))
#define icx_nvp_read_hdi(_a,_b)  icx_nvp_read_data(73,(_a),(_b))

#define icx_nvp_write_lbi(_a,_b) icx_nvp_write_data(74,(_a),(_b))
#define icx_nvp_read_lbi(_a,_b)  icx_nvp_read_data(74,(_a),(_b))

#define icx_nvp_write_fui(_a,_b) icx_nvp_write_data(75,(_a),(_b))
#define icx_nvp_read_fui(_a,_b)  icx_nvp_read_data(75,(_a),(_b))

#define icx_nvp_write_eri(_a,_b) icx_nvp_write_data(76,(_a),(_b))
#define icx_nvp_read_eri(_a,_b)  icx_nvp_read_data(76,(_a),(_b))

#define icx_nvp_write_pci(_a,_b) icx_nvp_write_data(87,(_a),(_b))
#define icx_nvp_read_pci(_a,_b)  icx_nvp_read_data(87,(_a),(_b))

#define icx_nvp_write_dbi(_a,_b) icx_nvp_write_data(88,(_a),(_b))
#define icx_nvp_read_dbi(_a,_b)  icx_nvp_read_data(88,(_a),(_b))

#define ICX_NVP_NODE_BASE "/dev/icx_nvp/"

#define ICX_NVP_NODE_DBG ICX_NVP_NODE_BASE "000"
#define ICX_NVP_NODE_SYI ICX_NVP_NODE_BASE "001"
#define ICX_NVP_NODE_UBP ICX_NVP_NODE_BASE "002"
#define ICX_NVP_NODE_FUP ICX_NVP_NODE_BASE "003"
#define ICX_NVP_NODE_PRK ICX_NVP_NODE_BASE "004"
#define ICX_NVP_NODE_HLD ICX_NVP_NODE_BASE "005"
#define ICX_NVP_NODE_RTC ICX_NVP_NODE_BASE "006"
#define ICX_NVP_NODE_MID ICX_NVP_NODE_BASE "007"
#define ICX_NVP_NODE_PCD ICX_NVP_NODE_BASE "008"
#define ICX_NVP_NODE_SER ICX_NVP_NODE_BASE "009"
#define ICX_NVP_NODE_UFN ICX_NVP_NODE_BASE "010"
#define ICX_NVP_NODE_KAS ICX_NVP_NODE_BASE "011"
#define ICX_NVP_NODE_SHP ICX_NVP_NODE_BASE "012"
#define ICX_NVP_NODE_TST ICX_NVP_NODE_BASE "013"
#define ICX_NVP_NODE_GTY ICX_NVP_NODE_BASE "014"
#define ICX_NVP_NODE_FMP ICX_NVP_NODE_BASE "015"
#define ICX_NVP_NODE_SDP ICX_NVP_NODE_BASE "016"
#define ICX_NVP_NODE_NCP ICX_NVP_NODE_BASE "017"
#define ICX_NVP_NODE_PSK ICX_NVP_NODE_BASE "018"
#define ICX_NVP_NODE_NVR ICX_NVP_NODE_BASE "077"
#define ICX_NVP_NODE_SHE ICX_NVP_NODE_BASE "084"
#define ICX_NVP_NODE_BTC ICX_NVP_NODE_BASE "085"
#define ICX_NVP_NODE_INS ICX_NVP_NODE_BASE "089"
#define ICX_NVP_NODE_CTR ICX_NVP_NODE_BASE "090"
#define ICX_NVP_NODE_SKU ICX_NVP_NODE_BASE "091"
#define ICX_NVP_NODE_BPR ICX_NVP_NODE_BASE "019"
#define ICX_NVP_NODE_BFP ICX_NVP_NODE_BASE "020"
#define ICX_NVP_NODE_BFD ICX_NVP_NODE_BASE "021"
#define ICX_NVP_NODE_BML ICX_NVP_NODE_BASE "022"
#define ICX_NVP_NODE_APD ICX_NVP_NODE_BASE "078"
#define ICX_NVP_NODE_BLF ICX_NVP_NODE_BASE "079"
#define ICX_NVP_NODE_SLP ICX_NVP_NODE_BASE "080"
#define ICX_NVP_NODE_VRT ICX_NVP_NODE_BASE "081"
#define ICX_NVP_NODE_FNI ICX_NVP_NODE_BASE "082"
#define ICX_NVP_NODE_SID ICX_NVP_NODE_BASE "083"
#define ICX_NVP_NODE_MSO ICX_NVP_NODE_BASE "086"
#define ICX_NVP_NODE_CNG ICX_NVP_NODE_BASE "023"
#define ICX_NVP_NODE_LYR ICX_NVP_NODE_BASE "024"
#define ICX_NVP_NODE_DBV ICX_NVP_NODE_BASE "025"
#define ICX_NVP_NODE_FUR ICX_NVP_NODE_BASE "026"
#define ICX_NVP_NODE_UMS ICX_NVP_NODE_BASE "027"
#define ICX_NVP_NODE_SKD ICX_NVP_NODE_BASE "028"
#define ICX_NVP_NODE_UPS ICX_NVP_NODE_BASE "029"
#define ICX_NVP_NODE_MDK ICX_NVP_NODE_BASE "030"
#define ICX_NVP_NODE_FVI ICX_NVP_NODE_BASE "031"
#define ICX_NVP_NODE_MAC ICX_NVP_NODE_BASE "032"
#define ICX_NVP_NODE_FPI ICX_NVP_NODE_BASE "033"
#define ICX_NVP_NODE_TR0 ICX_NVP_NODE_BASE "034"
#define ICX_NVP_NODE_TR1 ICX_NVP_NODE_BASE "035"
#define ICX_NVP_NODE_E00 ICX_NVP_NODE_BASE "036"
#define ICX_NVP_NODE_E01 ICX_NVP_NODE_BASE "037"
#define ICX_NVP_NODE_E02 ICX_NVP_NODE_BASE "038"
#define ICX_NVP_NODE_E03 ICX_NVP_NODE_BASE "039"
#define ICX_NVP_NODE_E04 ICX_NVP_NODE_BASE "040"
#define ICX_NVP_NODE_E05 ICX_NVP_NODE_BASE "041"
#define ICX_NVP_NODE_E06 ICX_NVP_NODE_BASE "042"
#define ICX_NVP_NODE_E07 ICX_NVP_NODE_BASE "043"
#define ICX_NVP_NODE_E08 ICX_NVP_NODE_BASE "044"
#define ICX_NVP_NODE_E09 ICX_NVP_NODE_BASE "045"
#define ICX_NVP_NODE_E10 ICX_NVP_NODE_BASE "046"
#define ICX_NVP_NODE_E11 ICX_NVP_NODE_BASE "047"
#define ICX_NVP_NODE_E12 ICX_NVP_NODE_BASE "048"
#define ICX_NVP_NODE_E13 ICX_NVP_NODE_BASE "049"
#define ICX_NVP_NODE_E14 ICX_NVP_NODE_BASE "050"
#define ICX_NVP_NODE_E15 ICX_NVP_NODE_BASE "051"
#define ICX_NVP_NODE_E16 ICX_NVP_NODE_BASE "052"
#define ICX_NVP_NODE_E17 ICX_NVP_NODE_BASE "053"
#define ICX_NVP_NODE_E18 ICX_NVP_NODE_BASE "054"
#define ICX_NVP_NODE_E19 ICX_NVP_NODE_BASE "055"
#define ICX_NVP_NODE_E20 ICX_NVP_NODE_BASE "056"
#define ICX_NVP_NODE_E21 ICX_NVP_NODE_BASE "057"
#define ICX_NVP_NODE_E22 ICX_NVP_NODE_BASE "058"
#define ICX_NVP_NODE_E23 ICX_NVP_NODE_BASE "059"
#define ICX_NVP_NODE_E24 ICX_NVP_NODE_BASE "060"
#define ICX_NVP_NODE_E25 ICX_NVP_NODE_BASE "061"
#define ICX_NVP_NODE_E26 ICX_NVP_NODE_BASE "062"
#define ICX_NVP_NODE_E27 ICX_NVP_NODE_BASE "063"
#define ICX_NVP_NODE_E28 ICX_NVP_NODE_BASE "064"
#define ICX_NVP_NODE_E29 ICX_NVP_NODE_BASE "065"
#define ICX_NVP_NODE_E30 ICX_NVP_NODE_BASE "066"
#define ICX_NVP_NODE_E31 ICX_NVP_NODE_BASE "067"
#define ICX_NVP_NODE_CLV ICX_NVP_NODE_BASE "068"
#define ICX_NVP_NODE_SPS ICX_NVP_NODE_BASE "069"
#define ICX_NVP_NODE_RBT ICX_NVP_NODE_BASE "070"
#define ICX_NVP_NODE_EDW ICX_NVP_NODE_BASE "071"
#define ICX_NVP_NODE_BTI ICX_NVP_NODE_BASE "072"
#define ICX_NVP_NODE_HDI ICX_NVP_NODE_BASE "073"
#define ICX_NVP_NODE_LBI ICX_NVP_NODE_BASE "074"
#define ICX_NVP_NODE_FUI ICX_NVP_NODE_BASE "075"
#define ICX_NVP_NODE_ERI ICX_NVP_NODE_BASE "076"
#define ICX_NVP_NODE_PCI ICX_NVP_NODE_BASE "087"
#define ICX_NVP_NODE_DBI ICX_NVP_NODE_BASE "088"

#define ICX_NVP_NODE_NO_DBG 0
#define ICX_NVP_NODE_NO_SYI 1
#define ICX_NVP_NODE_NO_UBP 2
#define ICX_NVP_NODE_NO_FUP 3
#define ICX_NVP_NODE_NO_PRK 4
#define ICX_NVP_NODE_NO_HLD 5
#define ICX_NVP_NODE_NO_RTC 6
#define ICX_NVP_NODE_NO_MID 7
#define ICX_NVP_NODE_NO_PCD 8
#define ICX_NVP_NODE_NO_SER 9
#define ICX_NVP_NODE_NO_UFN 10
#define ICX_NVP_NODE_NO_KAS 11
#define ICX_NVP_NODE_NO_SHP 12
#define ICX_NVP_NODE_NO_TST 13
#define ICX_NVP_NODE_NO_GTY 14
#define ICX_NVP_NODE_NO_FMP 15
#define ICX_NVP_NODE_NO_SDP 16
#define ICX_NVP_NODE_NO_NCP 17
#define ICX_NVP_NODE_NO_PSK 18
#define ICX_NVP_NODE_NO_NVR 77
#define ICX_NVP_NODE_NO_SHE 84
#define ICX_NVP_NODE_NO_BTC 85
#define ICX_NVP_NODE_NO_INS 89
#define ICX_NVP_NODE_NO_CTR 90
#define ICX_NVP_NODE_NO_SKU 91
#define ICX_NVP_NODE_NO_BPR 19
#define ICX_NVP_NODE_NO_BFP 20
#define ICX_NVP_NODE_NO_BFD 21
#define ICX_NVP_NODE_NO_BML 22
#define ICX_NVP_NODE_NO_APD 78
#define ICX_NVP_NODE_NO_BLF 79
#define ICX_NVP_NODE_NO_SLP 80
#define ICX_NVP_NODE_NO_VRT 81
#define ICX_NVP_NODE_NO_FNI 82
#define ICX_NVP_NODE_NO_SID 83
#define ICX_NVP_NODE_NO_MSO 86
#define ICX_NVP_NODE_NO_CNG 23
#define ICX_NVP_NODE_NO_LYR 24
#define ICX_NVP_NODE_NO_DBV 25
#define ICX_NVP_NODE_NO_FUR 26
#define ICX_NVP_NODE_NO_UMS 27
#define ICX_NVP_NODE_NO_SKD 28
#define ICX_NVP_NODE_NO_UPS 29
#define ICX_NVP_NODE_NO_MDK 30
#define ICX_NVP_NODE_NO_FVI 31
#define ICX_NVP_NODE_NO_MAC 32
#define ICX_NVP_NODE_NO_FPI 33
#define ICX_NVP_NODE_NO_TR0 34
#define ICX_NVP_NODE_NO_TR1 35
#define ICX_NVP_NODE_NO_E00 36
#define ICX_NVP_NODE_NO_E01 37
#define ICX_NVP_NODE_NO_E02 38
#define ICX_NVP_NODE_NO_E03 39
#define ICX_NVP_NODE_NO_E04 40
#define ICX_NVP_NODE_NO_E05 41
#define ICX_NVP_NODE_NO_E06 42
#define ICX_NVP_NODE_NO_E07 43
#define ICX_NVP_NODE_NO_E08 44
#define ICX_NVP_NODE_NO_E09 45
#define ICX_NVP_NODE_NO_E10 46
#define ICX_NVP_NODE_NO_E11 47
#define ICX_NVP_NODE_NO_E12 48
#define ICX_NVP_NODE_NO_E13 49
#define ICX_NVP_NODE_NO_E14 50
#define ICX_NVP_NODE_NO_E15 51
#define ICX_NVP_NODE_NO_E16 52
#define ICX_NVP_NODE_NO_E17 53
#define ICX_NVP_NODE_NO_E18 54
#define ICX_NVP_NODE_NO_E19 55
#define ICX_NVP_NODE_NO_E20 56
#define ICX_NVP_NODE_NO_E21 57
#define ICX_NVP_NODE_NO_E22 58
#define ICX_NVP_NODE_NO_E23 59
#define ICX_NVP_NODE_NO_E24 60
#define ICX_NVP_NODE_NO_E25 61
#define ICX_NVP_NODE_NO_E26 62
#define ICX_NVP_NODE_NO_E27 63
#define ICX_NVP_NODE_NO_E28 64
#define ICX_NVP_NODE_NO_E29 65
#define ICX_NVP_NODE_NO_E30 66
#define ICX_NVP_NODE_NO_E31 67
#define ICX_NVP_NODE_NO_CLV 68
#define ICX_NVP_NODE_NO_SPS 69
#define ICX_NVP_NODE_NO_RBT 70
#define ICX_NVP_NODE_NO_EDW 71
#define ICX_NVP_NODE_NO_BTI 72
#define ICX_NVP_NODE_NO_HDI 73
#define ICX_NVP_NODE_NO_LBI 74
#define ICX_NVP_NODE_NO_FUI 75
#define ICX_NVP_NODE_NO_ERI 76
#define ICX_NVP_NODE_NO_PCI 87
#define ICX_NVP_NODE_NO_DBI 88

#endif /* _ICX_NVP_H_ */

