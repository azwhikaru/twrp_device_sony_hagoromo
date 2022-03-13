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
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#if 0
/* declare function prototype */
#endif

#define CMD_LEN 10
static char myCmdBuf[CMD_LEN];

void evdo_host_wakeup(void)
{
#if 0
	/* call correct wake up func here */
#endif
}

ssize_t evdo_host_reset_proc_read(struct file *file_ptr, char __user *user_buffer, size_t count,
				  loff_t *position)
{
	pr_warn("count : %d, position : %d\n", count, *position);

	if (*position >= CMD_LEN)
		return 0;

	if (*position + count > CMD_LEN)
		count = CMD_LEN - *position;

	if (copy_to_user(user_buffer, myCmdBuf, count) != 0) {
		return -EFAULT;
	}

	*position += count;
	return count;
}

ssize_t evdo_host_reset_proc_write(struct file *file_ptr, char __user *user_buffer, size_t count,
				   loff_t *position)
{
	int ret = copy_from_user((char *)myCmdBuf, user_buffer, count);

	myCmdBuf[count] = '\0';
	pr_warn("%s(), myCmdBuf : %s\n", __func__, myCmdBuf);

	if (ret != 0) {
		return -EFAULT;
	}

	/*
	   do things here
	 */

	return count;
}

struct file_operations evdo_host_reset_proc_fops = {
	.read = evdo_host_reset_proc_read,
	.write = evdo_host_reset_proc_write
};

static int __init mtk_evdo_host_interface_init(void)
{
	/* temporaily obsoleted */
/*
	struct proc_dir_entry *prEntry;
	pr_warn("mtk_evdo_host_interface_init\n");

	prEntry = proc_create("EVDO_HOST_RESET", 0660, 0, &evdo_host_reset_proc_fops);
	if (prEntry)
	{
		pr_warn("add /proc/EVDO_HOST_RESET ok\n");
	}
	else
	{
		pr_warn("add /proc/EVDO_HOST_RESET fail\n");
	}
*/

	return 0;
}

static void __exit mtk_evdo_host_interface_exit(void)
{
	pr_warn("mtk_evdo_host_interface_exit\n");

}
module_init(mtk_evdo_host_interface_init);
module_exit(mtk_evdo_host_interface_exit);