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

#include <linux/uaccess.h>
#include <linux/debugfs.h>

#include "vq_def.h"

/*********************************** define ***********************************/

#define VQ_CMD_LOG_LEVEL		"log_level"
#define VQ_CMD_DISPFMT_PATTERN		"disp_pattern"
#define VQ_CMD_DI_MODE			"di_mode"
#define VQ_CMD_NR_LEVEL			"nr_level"
#define VQ_CMD_COLOR_FMT		"color_fmt"

/*********************************** variable *********************************/

struct dentry *_VqDebugFs = NULL;

static char _vq_cmd_buf[512];

static char _vq_dbg_buf[2048];

static char _VQ_STR_HELP[] = {
	VQ_CMD_LOG_LEVEL
};

/*********************************** function *********************************/

static void vq_process_dbg_cmd(char *cmd)
{
	char *opt;

	VQ_Printf(VQ_LOG_CMD, "[CMD] dbg cmd %s\n", cmd);

	memset(_vq_dbg_buf, 0, sizeof(_vq_dbg_buf));

	opt = strsep(&cmd, " ");

	if (NULL == opt) {

		VQ_Printf(VQ_LOG_CMD, "[CMD] string is null\n");
		return;
	}

	VQ_Printf(VQ_LOG_CMD, "[CMD] param1 %s\n", opt);

	if (0 == strncmp(opt, VQ_CMD_LOG_LEVEL, strlen(VQ_CMD_LOG_LEVEL))) {

		unsigned int log_level;
		unsigned int log_switch;

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			VQ_Printf(VQ_LOG_CMD, "[CMD] current log level is 0x%x\n", _vq_dbg_level);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param2 %s\n", opt);
		kstrtoul(opt, 10, &log_level);

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			VQ_Printf(VQ_LOG_CMD, "[CMD] current log level is 0x%x\n", _vq_dbg_level);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param3 %s\n", opt);
		kstrtoul(opt, 10, &log_switch);

		if (0 == log_level) {

			VQ_Printf(VQ_LOG_CMD, "[CMD] set log level to 0\n");
			_vq_dbg_level = 1;

		} else if (VQ_INVALID_DECIMAL == log_level) {

			VQ_Printf(VQ_LOG_CMD, "[CMD] set log level to 0xffffffff\n");
			_vq_dbg_level = 0xffffffff;

		} else {

			if (1 == log_switch) {

				VQ_Printf(VQ_LOG_CMD, "[CMD] set %d log level 0x%x to 0x%x\n",
					log_level, _vq_dbg_level, (_vq_dbg_level | (1 << log_level)));

				_vq_dbg_level |= (1 << log_level);

			} else if (0 == log_switch) {

				VQ_Printf(VQ_LOG_CMD, "[CMD] set %d log level 0x%x to 0x%x\n",
					log_level, _vq_dbg_level, (_vq_dbg_level & (~(1 << log_level))));

				_vq_dbg_level &= (~(1 << log_level));

			} else {

				VQ_Printf(VQ_LOG_CMD, "[CMD] set dbg error switch %d\n", log_switch);
			}
		}

	} else if (0 == strncmp(opt, VQ_CMD_DISPFMT_PATTERN, strlen(VQ_CMD_DISPFMT_PATTERN))) {

		unsigned int enable;
		unsigned int width;

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetDispfmtPattern(0, enable, width);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param2 %s\n", opt);
		kstrtoul(opt, 10, &enable);

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetDispfmtPattern(0, enable, width);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param3 %s\n", opt);
		kstrtoul(opt, 10, &width);

		iVQ_CmdSetDispfmtPattern(1, enable, width);

	} else if (0 == strncmp(opt, VQ_CMD_DI_MODE, strlen(VQ_CMD_DI_MODE))) {

		unsigned int DiMode;
		unsigned int CurrField;
		unsigned int TopFieldFirst;

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetDiMode(0, DiMode, CurrField, TopFieldFirst);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param2 %s\n", opt);
		kstrtoul(opt, 10, &DiMode);

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetDiMode(0, DiMode, CurrField, TopFieldFirst);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param3 %s\n", opt);
		kstrtoul(opt, 10, &CurrField);

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetDiMode(0, DiMode, CurrField, TopFieldFirst);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param4 %s\n", opt);
		kstrtoul(opt, 10, &TopFieldFirst);

		iVQ_CmdSetDiMode(1, DiMode, CurrField, TopFieldFirst);

	} else if (0 == strncmp(opt, VQ_CMD_NR_LEVEL, strlen(VQ_CMD_NR_LEVEL))) {

		unsigned int MnrLevel;
		unsigned int BnrLevel;

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetNrlevel(0, MnrLevel, BnrLevel);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param2 %s\n", opt);
		kstrtoul(opt, 10, &MnrLevel);

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetNrlevel(0, MnrLevel, BnrLevel);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param3 %s\n", opt);
		kstrtoul(opt, 10, &BnrLevel);

		iVQ_CmdSetNrlevel(1, MnrLevel, BnrLevel);

	} else if (0 == strncmp(opt, VQ_CMD_COLOR_FMT, strlen(VQ_CMD_COLOR_FMT))) {

		unsigned int SrcColor;
		unsigned int DstColor;

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetColorFmt(0, SrcColor, DstColor);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param2 %s\n", opt);
		kstrtoul(opt, 10, &SrcColor);

		opt += strlen(opt);
		opt = strsep(&cmd, " ");
		if (NULL == opt) {

			iVQ_CmdSetColorFmt(0, SrcColor, DstColor);
			return;
		}
		VQ_Printf(VQ_LOG_CMD, "[CMD] param3 %s\n", opt);
		kstrtoul(opt, 10, &DstColor);

		iVQ_CmdSetColorFmt(1, SrcColor, DstColor);

	} else {

		VQ_Printf(VQ_LOG_CMD, "[CMD] invalid cmd %s\n", opt);
	}

	return;
}

static ssize_t vq_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;

	return 0;
}

static ssize_t vq_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	if (strlen(_vq_dbg_buf)) {

		/*for keep brace*/
		return simple_read_from_buffer(ubuf, count, ppos, _vq_dbg_buf, strlen(_vq_dbg_buf));

	} else {

		/*for keep brace*/
		return simple_read_from_buffer(ubuf, count, ppos, _VQ_STR_HELP, strlen(_VQ_STR_HELP));
	}
}

static ssize_t vq_debug_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	const int debug_bufmax = sizeof(_vq_cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax) {

		/*for keep brace*/
		count = debug_bufmax;
	}

	if (copy_from_user(&_vq_cmd_buf, ubuf, count)) {

		/*for keep brace*/
		return -EFAULT;
	}

	_vq_cmd_buf[count] = 0;

	vq_process_dbg_cmd(_vq_cmd_buf);

	return ret;
}

static const struct file_operations _vq_debug_fops = {
	.read  = vq_debug_read,
	.write = vq_debug_write,
	.open  = vq_debug_open,
};

void vq_debug_init(void)
{
	_VqDebugFs = debugfs_create_file("vq_dbg", S_IFREG|S_IRUGO, NULL, (void *)0, &_vq_debug_fops);
}

void vq_debug_exit(void)
{
	debugfs_remove(_VqDebugFs);
}


