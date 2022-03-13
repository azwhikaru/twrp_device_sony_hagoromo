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
#ifndef __CCCI_SYSFS_H__
#define __CCCI_SYSFS_H__

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include "ccci_core.h"
#include "ccci_debug.h"

#define CCCI_KOBJ_NAME "ccci"

struct ccci_info {
	struct kobject kobj;
	unsigned int ccci_attr_count;
};

struct ccci_attribute {
	struct attribute attr;
	 ssize_t(*show) (char *buf);
	 ssize_t(*store) (const char *buf, size_t count);
};

#define CCCI_ATTR(_name, _mode, _show, _store)				\
static struct ccci_attribute ccci_attr_##_name = {					\
	.attr = {.name = __stringify(_name), .mode = _mode },	\
	.show = _show,											\
	.store = _store,										\
}

struct ccci_md_attribute {
	struct attribute attr;
	struct ccci_modem *modem;
	 ssize_t(*show) (struct ccci_modem *md, char *buf);
	 ssize_t(*store) (struct ccci_modem *md, const char *buf, size_t count);
};

#define CCCI_MD_ATTR(_modem, _name, _mode, _show, _store)				\
static struct ccci_md_attribute ccci_md_attr_##_name = {					\
	.attr = {.name = __stringify(_name), .mode = _mode },	\
	.modem = _modem,										\
	.show = _show,											\
	.store = _store,										\
}

#endif				/* __CCCI_SYSFS_H__ */
