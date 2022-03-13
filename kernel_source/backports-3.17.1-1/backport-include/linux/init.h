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
#ifndef __BACKPORT_INIT_H
#define __BACKPORT_INIT_H
#include_next <linux/init.h>

/*
 * Backports 312b1485fb509c9bc32eda28ad29537896658cb8
 * Author: Sam Ravnborg <sam@ravnborg.org>
 * Date:   Mon Jan 28 20:21:15 2008 +0100
 * 
 * Introduce new section reference annotations tags: __ref, __refdata, __refconst
 */
#ifndef __ref
#define __ref		__init_refok
#endif
#ifndef __refdata
#define __refdata	__initdata_refok
#endif

#endif /* __BACKPORT_INIT_H */
