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
#define MHL_DRIVER_IOCTL_C
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include "mhl_linuxdrv.h"
#include "sii_hal.h"
#include "mhl_linuxdrv_ioctl.h"
#include "osal/include/osal.h"
#include "si_mhl_tx_api.h"
#include "si_drv_mhl_tx.h"
#include "si_drvisrconfig.h"
static SiiOsTimer_t TestDelay;
static void SiiMhlTimerTestCB(void *pArg)
{
	SiiOsTimerDelete(TestDelay);
	TestDelay = NULL;
	SiiMhlTriggerSoftInt();
}

long SiiMhlIoctl(struct file *pFile, unsigned int ioctlCode, unsigned long ioctlParam)
{
	long retStatus = 0;
	Register_t RegisterInfo;
	uint8_t reg;
	UserControl_t user_control;
	if (HalAcquireIsrLock() != HAL_RET_SUCCESS) {
		return -ERESTARTSYS;
	}
	switch (ioctlCode) {
	case SII_IOCTRL_REGISTER_READ:
		retStatus = copy_from_user(&RegisterInfo, (Register_t *) ioctlParam,
					   sizeof(Register_t));
		if (!retStatus) {
			RegisterInfo.value =
			    I2C_ReadByte(RegisterInfo.dev_address, RegisterInfo.subaddr);
			retStatus =
			    copy_to_user((Register_t *) ioctlParam, &RegisterInfo,
					 sizeof(Register_t));
		} else {
			pr_info("register read error!\n");
		}
		break;
	case SII_IOCTRL_REGISTER_WRITE:
		retStatus = copy_from_user(&RegisterInfo, (Register_t *) ioctlParam,
					   sizeof(Register_t));
		reg = I2C_ReadByte(RegisterInfo.dev_address, RegisterInfo.subaddr);
		reg = (reg & (~RegisterInfo.mask)) | (RegisterInfo.mask & RegisterInfo.value);
		I2C_WriteByte(RegisterInfo.dev_address, RegisterInfo.subaddr, reg);
		break;
	case SII_IOCTRL_USER:
		retStatus = copy_from_user(&user_control, (UserControl_t *) ioctlParam,
					   sizeof(UserControl_t));
		switch (user_control.ControlID) {
		case USER_GPIO_GET:
			/* HalGpioGetPin(user_control.SubCommand.GpioCtrl.GpioIndex, &user_control.SubCommand.GpioCtrl.Value); */
			break;
		case USER_GPIO_SET:
			/* HalGpioSetPin(user_control.SubCommand.GpioCtrl.GpioIndex, user_control.SubCommand.GpioCtrl.Value); */
			break;
		case USER_TRIGGER_EXT_INT:
			/* SiiTriggerExtInt(); */
			break;
		case USER_TRIGGER_MHL_INT:
			{
				if (TestDelay != NULL) {
					SiiOsTimerDelete(TestDelay);
					TestDelay = NULL;
				}
				SiiOsTimerCreate("Abort Time Out", SiiMhlTimerTestCB, NULL, true,
						 2000, false, &TestDelay);
			}
			break;
		case USER_ON_OFF_MHL_INT:
			HalEnableIrq(user_control.SubCommand.iSubCommand ? 1 : 0);
			break;
		case USER_RESET_MHL_CHIP:
			SiiMhlTxInitialize(EVENT_POLL_INTERVAL_MS);
			break;
		case USER_READ_SINK_EDID:
			{
#define _MASK_(aByte, bitMask, setBits) ((setBits) ? (aByte | bitMask) : (aByte & ~bitMask))
				int RepeatNums = 5, i;
				int iRepeatCnt = 0;
				uint8_t reg;
				uint8_t reg_save;
				reg_save = reg = I2C_ReadByte(0x72, 0xC7);
				reg = _MASK_(reg, BIT0, 1);
				I2C_WriteByte(0x72, 0xc7, reg);
				do {
					if (++iRepeatCnt > RepeatNums)
						break;
					reg = I2C_ReadByte(0x72, 0xC7);
					HalTimerWait(10);
				} while (!(reg & BIT1));
				if (iRepeatCnt > RepeatNums) {
					pr_info("try time out\n");
				} else {
					reg = I2C_ReadByte(0x72, 0xC7);
					reg = _MASK_(reg, BIT2, 1);
					I2C_WriteByte(0x72, 0xc7, reg);
					for (i = 0; i < 256; i++) {
					}
					I2C_ReadBlock(0xa0, 0, user_control.SubCommand.EDID, 128);
					I2C_ReadBlock(0xa0, 128, &user_control.SubCommand.EDID[128],
						      128);
					I2C_WriteByte(0x72, 0xc7, reg_save);
				}
			}
			break;
		}
		retStatus = copy_to_user((UserControl_t *) ioctlParam,
					 &user_control, sizeof(UserControl_t));
		break;
	default:
		SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
				"SiiMhlIoctl, unrecognized ioctlCode 0x%0x received!\n", ioctlCode);
		retStatus = -EINVAL;
		break;
	}
	HalReleaseIsrLock();
	return retStatus;
}
