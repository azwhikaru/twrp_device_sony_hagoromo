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
#ifndef SEC_MOD_CORE_H
#define SEC_MOD_CORE_H

/******************************************************************************
 *  EXPORT FUNCTION
 ******************************************************************************/
extern long sec_core_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
extern void sec_core_init(void);
extern void sec_core_exit(void);
extern int sec_get_random_id(unsigned int *rid);
extern void sec_update_lks(unsigned char tr, unsigned char dn, unsigned char fb_ulk);

#endif				/* SEC_MOD_CORE_H */
