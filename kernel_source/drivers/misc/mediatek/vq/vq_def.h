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


#ifndef VQ_DEF_H
#define VQ_DEF_H

#include "vq_ctrl.h"

#if VQ_CTP_TEST
/* #include "string.h" */
#include "sys_io.h"
#include "stdio.h"
#include "CTP_type.h"
#include "CTP_shell.h"
#include "api.h"
#include "mmu.h"
#endif

#if !VQ_CTP_TEST
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/mmprofile.h>
#include <mach/mt_irq.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_spm_mtcmos.h>
#include <mach/mt_pm_ldo.h>
#include <mach/m4u.h>
#endif

#ifdef CONFIG_ION_MTK
	#define VQ_ION_SUPPORT      1
#else
	#define VQ_ION_SUPPORT      0
#endif

#if VQ_CTP_TEST
#define VQ_IO_BASE              0x1C000000
#else
#define VQ_IO_BASE              0xFC000000
#endif

#define VQ_ASSERT               { do { } while (1); }

/* debug */
#if VQ_LOG_ALL

#define VQ_LOG_DEFAULT		0
#define VQ_LOG_CMD		0
#define VQ_LOG_ERROR		0
#define VQ_LOG_IOCTL		0
#define VQ_LOG_PARAM		0
#define VQ_LOG_ADDRESS		0
#define VQ_LOG_FLOW		0
#define VQ_LOG_TIME		0
#define VQ_LOG_IRQ		0
#define VQ_LOG_CTP		0

#else

#define VQ_LOG_DEFAULT		0
#define VQ_LOG_CMD		0
#define VQ_LOG_ERROR		0
#define VQ_LOG_IOCTL		1
#define VQ_LOG_PARAM		2
#define VQ_LOG_ADDRESS		3
#define VQ_LOG_FLOW		4
#define VQ_LOG_TIME		5
#define VQ_LOG_IRQ		6
#define VQ_LOG_CTP		7

#endif

extern unsigned int _vq_dbg_level;

#if VQ_TIME_CHECK
extern unsigned int _au4VqTimeRec[];

#define VQ_TIME_REC(x) \
	{ \
		struct timeval TimeRec; \
		do_gettimeofday(&TimeRec); \
		_au4VqTimeRec[x] = TimeRec.tv_sec * 1000000 + TimeRec.tv_usec; \
	}
#endif

#if VQ_CTP_TEST
#define VQ_Printf(level, string, args...)  \
	{ \
		if (_vq_dbg_level & (1 << level)) { \
			printf("[VQ] "string"\n", ##args); \
		} \
	}
#else
#define VQ_Printf(level, string, args...)  \
	{ \
		if (_vq_dbg_level & (1 << level)) { \
			pr_err("[VQ] "string"\n", ##args); \
		} \
	}
#endif

/* reg */
#define BITS(high, low)                             ((1 << (high - low + 1)) - 1)
#define VQ_REG_READ(reg)                            (*(volatile unsigned int*)(reg))
#define VQ_REG_WRITE(reg, value)                    ((*((volatile unsigned int*)(reg))) = (value))
#define VQ_REG_READ_BITS(reg, bits, shift)          (((VQ_REG_READ(reg)) >> (shift)) & (bits))
#define VQ_REG_WRITE_BITS(reg, bits, shift, value)  \
	(VQ_REG_WRITE((reg), \
	(((VQ_REG_READ((reg))) & (~((unsigned int)(bits) << (shift)))) |\
	(((unsigned int)(value) & (unsigned int)(bits)) << (shift)))))

#define VQ_ReadReg(u4Addr)                          (*(volatile unsigned int*)(u4Addr))
#define VQ_WriteReg(u4Addr, u4Value)                (*(volatile unsigned int*)(u4Addr) = u4Value)

#define VQ_REG_WRITE_MASK(reg, value, mask) \
	(VQ_REG_WRITE(reg, (((VQ_REG_READ(reg)) & (~((unsigned int)(mask)))) | ((unsigned int)(value)))))

#define TIMING_SIZE_720_480_H               720
#define TIMING_SIZE_720_480_V               480
#define TIMING_SIZE_720_576_H               720
#define TIMING_SIZE_720_576_V               576
#define TIMING_SIZE_1280_720_H              1280
#define TIMING_SIZE_1280_720_V              720
#define TIMING_SIZE_1920_1080_H             1920
#define TIMING_SIZE_1920_1080_V             1080


enum VQ_TIMING_TYPE_E {
	VQ_TIMING_TYPE_480P,
	VQ_TIMING_TYPE_576P,
	VQ_TIMING_TYPE_720P,
	VQ_TIMING_TYPE_1080P,
	VQ_TIMING_TYPE_MAX
};

#endif /* VQ_DEF_H */

