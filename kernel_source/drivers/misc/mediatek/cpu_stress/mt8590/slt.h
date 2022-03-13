/*
 *  linux/arch/arm/mach-versatile/clock.h
 *
 *  Copyright (C) 2004 ARM Limited.
 *  Written by Deep Blue Solutions Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#if !defined(__SLT__)
#define __SLT__

#define SLT_LOOP_CNT	(100)
//#define SLT_64BIT

enum{
	SLT_CACHE_MISS=0,
	SLT_MAX_POWER,
	SLT_DHRYSTONE,
	SLT_SAXPY
};

#endif  /*  __SLT__ */
