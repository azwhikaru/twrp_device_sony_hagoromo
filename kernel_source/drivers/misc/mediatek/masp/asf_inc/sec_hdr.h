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
#ifndef SEC_HDR_H
#define SEC_HDR_H

/******************************************************************************
 *  INCLUDE LIBRARY
 ******************************************************************************/
#include "sec_boot_lib.h"

/**************************************************************************
 * EXPORT FUNCTIONS
 **************************************************************************/
extern unsigned int shdr_magic(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned char *shdr_cust_name(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_cust_name_len(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_img_ver(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_img_len(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_img_offset(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_sign_len(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_sign_offset(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_sig_len(SEC_IMG_HEADER_U *sec_hdr);
extern unsigned int shdr_sig_offset(SEC_IMG_HEADER_U *sec_hdr);
extern void set_shdr_magic(SEC_IMG_HEADER_U *sec_hdr, unsigned int val);
extern void set_shdr_img_ver(SEC_IMG_HEADER_U *sec_hdr, unsigned int ver);
extern void set_shdr_cust_name(SEC_IMG_HEADER_U *sec_hdr, unsigned char *name, unsigned int len);
extern void set_shdr_sign_len(SEC_IMG_HEADER_U *sec_hdr, unsigned int val);
extern void set_shdr_sign_offset(SEC_IMG_HEADER_U *sec_hdr, unsigned int val);
extern void set_shdr_ver(SEC_IMG_HEADER_VER ver);
extern SEC_IMG_HEADER_VER get_shdr_ver(void);

#endif				/* SEC_HDR_H */
