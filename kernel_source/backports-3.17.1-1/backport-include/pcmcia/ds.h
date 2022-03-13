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
#ifndef __BACKPORT_PCMCIA_DS_H
#define __BACKPORT_PCMCIA_DS_H
#include_next <pcmcia/ds.h>

#ifndef module_pcmcia_driver
/**
 * backport of:
 *
 * commit 6ed7ffddcf61f668114edb676417e5fb33773b59
 * Author: H Hartley Sweeten <hsweeten@visionengravers.com>
 * Date:   Wed Mar 6 11:24:44 2013 -0700
 *
 *     pcmcia/ds.h: introduce helper for pcmcia_driver module boilerplate
 */

/**
 * module_pcmcia_driver() - Helper macro for registering a pcmcia driver
 * @__pcmcia_driver: pcmcia_driver struct
 *
 * Helper macro for pcmcia drivers which do not do anything special in module
 * init/exit. This eliminates a lot of boilerplate. Each module may only use
 * this macro once, and calling it replaces module_init() and module_exit().
 */
#define module_pcmcia_driver(__pcmcia_driver) \
	module_driver(__pcmcia_driver, pcmcia_register_driver, \
			pcmcia_unregister_driver)
#endif

#endif /* __BACKPORT_PCMCIA_DS_H */
