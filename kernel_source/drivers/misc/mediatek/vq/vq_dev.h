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


#ifndef VQ_DEV_H
#define VQ_DEV_H

#include "vq_ctrl.h"

#define VQ_IOCTL_MAGIC        'v'

#define VQ_IOCTL_POWER_SWITCH       _IOW(VQ_IOCTL_MAGIC, 1, int)
#define VQ_IOCTL_PROCESS            _IOW(VQ_IOCTL_MAGIC, 2, struct VQ_PARAM_T)

#endif /* VQ_DEV_H */
