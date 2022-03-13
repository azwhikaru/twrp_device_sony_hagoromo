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
#include <linux/mu3d/hal/mu3d_hal_comm.h>
#include <linux/mu3d/hal/mu3d_hal_osal.h>

DEV_INT32 wait_for_value(DEV_INT32 addr, DEV_INT32 msk, DEV_INT32 value, DEV_INT32 ms_intvl,
			 DEV_INT32 count)
{
	DEV_UINT32 i;

	for (i = 0; i < count; i++) {
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		os_ms_delay(ms_intvl);
	}

	return RET_FAIL;
}

#ifdef NEVER

DEV_INT32 wait_for_value_tmout(DEV_INT32 addr, DEV_INT32 msk, DEV_INT32 value, DEV_INT32 ms_intvl,
			       DEV_INT32 count)
{
	DEV_UINT32 i;

	for (i = 0; i < count; i++) {
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		os_ms_sleep(ms_intvl);
	}

	return RET_FAIL;
}

DEV_INT32 wait_until_true(DEV_INT32 addr, DEV_INT32 msk, DEV_INT32 value, DEV_INT32 ms_intvl,
			  DEV_INT32 count)
{
	DEV_UINT32 i;

	for (i = 0; i < count; i++) {
		if ((os_readl(addr) & msk) == value)
			return RET_SUCCESS;
		os_ms_sleep(ms_intvl);
	}

	return RET_FAIL;
}
#endif
