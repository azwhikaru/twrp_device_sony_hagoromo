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
/*
 * mtkvideo_debug.h - V4L2 display driver debug method header file.
 *
 */

#ifndef __MTKVIDEO_DEBUG_H
#define __MTKVIDEO_DEBUG_H


#include "mtkvideo_headers.h"

extern int mtkvideo_log_on;

#define MTKVIDEO_LOG(fmt, arg...) \
    do { \
        if (mtkvideo_log_on) {printk("[mtkvideo] "fmt" %s, %d\n", ##arg,  __FUNCTION__, __LINE__);}  \
    } while (0)

#define MTKVIDEO_API_ENTRY()    \
    do { \
        if(mtkvideo_log_on) {printk("[mtkvideo] %s, %d\n", __FUNCTION__, __LINE__);} \
    } while (0)

#define MTKVIDEO_MSG(fmt, arg...) \
    do { \
        {printk("[mtkvideo] "fmt" %s, %d\n", ##arg, __FUNCTION__, __LINE__);} \
    } while (0)

#define MTKVIDEO_ERROR(fmt, arg...) \
        do { \
            {printk("[mtkvideo] "fmt" %s, %d\n", ##arg, __FUNCTION__, __LINE__);} \
        } while (0)

#define MTKVIDEO_ASSERT(expr) \
    do { \
        if (!(expr)) { printk("[mtkvideo] assertion failed at %s, %d\n", __FUNCTION__, __LINE__); return -1;} \
    } while (0)

#endif /* __MTKVIDEO_DEBUG_H */
