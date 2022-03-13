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
#ifndef LINUX_BACKPORTS_PRIVATE_H
#define LINUX_BACKPORTS_PRIVATE_H

#include <linux/version.h>

#ifdef CPTCFG_BACKPORT_BUILD_CRYPTO_CCM
int crypto_ccm_module_init(void);
void crypto_ccm_module_exit(void);
#else
static inline int crypto_ccm_module_init(void)
{ return 0; }
static inline void crypto_ccm_module_exit(void)
{}
#endif

#endif /* LINUX_BACKPORTS_PRIVATE_H */
