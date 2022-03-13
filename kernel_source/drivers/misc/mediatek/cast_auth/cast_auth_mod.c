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
#include <linux/module.h>
#include "cast_auth_ko.h"
static int __init cast_auth_init(void)
{
    if (cast_auth_ko_init() != 0)
    {
        pr_err("[cast_auth KO] cast_auth_ko_init() fail!!\n");
    }

    return 0;
}

static void __exit cast_auth_exit(void)
{
    if (cast_auth_ko_uninit() != 0)
    {
        pr_err("[cast_auth KO] cast_auth_ko_uninit() fail!!\n");
    }
}

module_init(cast_auth_init);
module_exit(cast_auth_exit);

/* TODO module license & information */
MODULE_DESCRIPTION("MEDIATEK Auth Driver For Cast Audio");
MODULE_AUTHOR("weiping.xiao<weiping.xiao@mediatek.com>");
MODULE_LICENSE("GPL");
