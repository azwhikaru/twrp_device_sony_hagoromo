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

#ifndef _TVE_COMMON_H_
#define _TVE_COMMON_H_

#define CVBS_DRV "/dev/cvbs"
extern unsigned int CVBS_LOG_ENABLE;

#define CVBS_DEF_LOG_EN                            (0x01)
#define CVBS_AUTO_DETECT_LOG_EN            (0x02)
#define CVBS_FUNC_LOG_EN                           (0x04)

#define CVBS_LOG(fmt, arg...) \
    do { \
        if (CVBS_LOG_ENABLE & CVBS_FUNC_LOG_EN) {printk("[cvbs]#%d ", __LINE__); printk(fmt, ##arg);} \
    }while (0)

#define CVBS_FUNC()    \
        do { \
            if (CVBS_LOG_ENABLE & CVBS_FUNC_LOG_EN) {printk("[cvbs]%s,%d \n", __func__, __LINE__);} \
        }while (0)

#define CVBS_DEF_LOG(fmt, arg...) \
        do { \
        if (CVBS_LOG_ENABLE & CVBS_DEF_LOG_EN) {printk(fmt, ##arg);} \
        }while (0)
#define CVBS_AUTO_DETECT_LOG(fmt, arg...) \
        do { \
            if (CVBS_LOG_ENABLE & CVBS_FUNC_LOG_EN) {printk("[cvbs]#%d ", __LINE__); printk(fmt, ##arg);} \
        }while (0)


#define CVBS_PRINTF(fmt, arg...)  \
			do { \
				 temp_len = sprintf(buf,fmt,##arg);  \
				 buf += temp_len; \
				 len += temp_len; \
			}while (0)

			
extern void Assert(const char* szExpress, const char* szFile, int i4Line);
#undef ASSERT				
#define ASSERT(x)        ((x) ? (void)0 : Assert(#x, __FILE__, __LINE__))

#endif
