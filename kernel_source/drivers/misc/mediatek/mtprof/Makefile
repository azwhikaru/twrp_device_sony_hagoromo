#
# Copyright (C) 2011-2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify it under the terms of the
# GNU General Public License version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program.
# If not, see <http://www.gnu.org/licenses/>.
#
obj-y := mtprof.o bootprof.o sched_monitor.o monitor_debug_out.o
obj-$(CONFIG_MT_LOCK_DEBUG) += lockprof.o
obj-$(CONFIG_MT_ENG_BUILD) += mt_debug_config.o
obj-$(CONFIG_MTK_WQ_DEBUG) += mt_wq_debug.o
mtprof-y += prof_ctl.o prof_main.o
obj-y += mt_prv_lock.o
