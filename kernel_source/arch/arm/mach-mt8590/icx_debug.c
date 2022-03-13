/*
 *	Copyright 2016 Sony Corporation
 *	File changed on 2016-01-22
 */
/*
 *	icx_debug.c -- sony icx debug module
 *
 *	Copyright 2016 Sony Corporation
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  version 2 of the  License.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include "../../fs/proc/internal.h"

#include <linux/icx_debug.h>

/***********/
/*@ define */
/***********/

/* message level */
#define KERN_MSG KERN_ERR

/* parameters */
#define PROC_MAX_ARGS          32

/* basic */
#define maximum(_a,_b) ((_a)>(_b)?(_a):(_b))
#define minimum(_a,_b) ((_a)<(_b)?(_a):(_b))
#define atoi(str) simple_strtoul(((str != NULL) ? (str)   : ""), NULL, 10)
#define htoi(str) simple_strtoul(((str != NULL) ? (str)+2 : ""), NULL, 16)

/* common */
#define MODULE_NAME "icx_debug"

/* command_info */
typedef struct {
	char name[32];
	int (*function)(int argc, char *argv[]);
} COMMAND_INFO;

/**************/
/*@ prototype */
/**************/

/* initialize_exit_entry */
int  __init icx_debug_init(void);
void __exit icx_debug_exit(void);
arch_initcall(icx_debug_init);
module_exit(icx_debug_exit);

int  __init icx_debug_proc_init(void);
void __exit icx_debug_proc_exit(void);
module_init(icx_debug_proc_init);
module_exit(icx_debug_proc_exit);

/* user_access_entry */
static ssize_t icx_debug_write_proc(
	struct file       *file,
	const char __user *buf,
	size_t             size,
	loff_t            *pos
);
static ssize_t icx_debug_read_proc(
	struct file *file,
	char __user *buf,
	size_t       size,
	loff_t      *pos
);

/* common_routine */
static void run_command(char *cmd);
static int  parse_command(char *cmd, char *argv[]);
static int  command_help(int argc, char *argv[]);
static void usage_read(void);
static int  command_read(int argc, char *argv[]);

/* statistics_command */
static void usage_statistics(void);
static int  command_statistics(int argc, char *argv[]);
static int  read_statistics(char __user * buf, loff_t pos, size_t size);

/* basic_routine */
static void delnl(char *string);

/********************/
/*@ global_variable */
/********************/

/* common_variable */
//static struct proc_dir_entry *ProcDir   = NULL;
static struct proc_dir_entry *Entry = NULL;
static int                    Node;
static spinlock_t             Spinlock;

/* proc_operation */
static struct file_operations Operations = {
	.write = icx_debug_write_proc,
	.read  = icx_debug_read_proc,
};

/* command_table */
static COMMAND_INFO Command[] = {
	{"help",       command_help},
	{"read",       command_read},
	{"statistics", command_statistics},
};
static int CommandCount = sizeof(Command) / sizeof(COMMAND_INFO);

static int (*ReadFunction)(char __user * buf, loff_t pos, size_t size) = NULL;

/**************************/
/*@ initialize_exit_entry */
/**************************/

int __init icx_debug_init(void)
{
	printk(KERN_INFO "starting icx debug module.\n");

	spin_lock_init(&Spinlock);

	return(0);
}

void __exit icx_debug_exit(void)
{
	printk(KERN_INFO "stopping icx debug module.\n");

	return;
}

int __init icx_debug_proc_init(void)
{
#if 0
	ProcDir = proc_mkdir(MODULE_NAME, NULL);
	if (ProcDir == NULL) {
		printk(KERN_ERR "%s: proc_mkdir error\n", __FUNCTION__);
		return(-ENOMEM);
	}
#endif

	Entry = proc_create_data(MODULE_NAME, 0600, NULL, &Operations, NULL);
	if (Entry == NULL) {
		printk(KERN_ERR "%s: create_proc_entry error\n", __FUNCTION__);
		return(-ENOMEM);
	}

	Node = Entry->low_ino;

	return(0);
}

void __exit icx_debug_proc_exit(void)
{
	if (Entry != NULL)
		remove_proc_entry(MODULE_NAME, NULL);

	return;
}

/**********************/
/*@ user_access_entry */
/**********************/

static ssize_t icx_debug_write_proc(
	struct file       *file,
	const char __user *buf,
	size_t             size,
	loff_t            *pos
)
{
	unsigned int node;
	char tmp[512];
	int count;
	int rv;

	node = file->f_dentry->d_inode->i_ino;

	if (node != Node) {
		printk(KERN_ERR "%s: invalid node %d\n", __FUNCTION__,node);
		return(-ENODEV);
	}

	count = minimum(512, size);

	rv = copy_from_user(
		(void *)(tmp+(long)*pos),
		(void *)buf,
		count
	);
	if (rv != 0) {
		printk(KERN_ERR "%s: copy_from_user error\n", __FUNCTION__);
		return(-EIO);
	}

	delnl(tmp);

	run_command(tmp);

	*pos = *pos + count;

	return(count);
}

static ssize_t icx_debug_read_proc(
	struct file *file,
	char __user *buf,
	size_t       size,
	loff_t      *pos
)
{
	unsigned int node;
	int rv;

	node = file->f_dentry->d_inode->i_ino;

	if (node != Node) {
		printk(KERN_ERR "invalid node %d\n", node);
		return(-ENODEV);
	}

	if (ReadFunction) {
		rv = ReadFunction(buf, *pos, size);
		if (rv < 0) {
			printk(KERN_ERR "%s: ReadFunction error\n", __FUNCTION__);
			return(rv);
		}
	}
	else{
		rv = 0;
	}

	*pos = *pos + rv;

	return(rv);
}

/*******************/
/*@ common_routine */
/*******************/

static void run_command(char *cmd)
{
	char *argv[PROC_MAX_ARGS];
	int  argc;
	int  i;

	argc = parse_command(cmd, argv);

	if (argc == 0)
		return;

	for (i = 0; i < CommandCount; i++) {
		if (strcmp(argv[0], Command[i].name) == 0)
			break;
	}

	if (i == CommandCount) {
		printk(KERN_ERR "invalid command\n");
		return;
	}

	Command[i].function(argc, argv);

	return;
}

static int parse_command(char *cmd, char *argv[])
{
	int argc;
	int spc;
	int tdq;
	int dq;

	argc = 0;
	spc  = 1;
	tdq  = 0;
	dq   = 0;

	while (*cmd != 0) {

		if (*cmd == '"') {
			tdq  = 1 - dq;
			*cmd = ' ';
		}

		if (spc && (dq || *cmd!=' ')) {
			if (argc >= PROC_MAX_ARGS)
				break;
			argv[argc] = cmd;
			argc++;
		}

		if (!dq && *cmd == ' ')
			spc = 1;
		else
			spc = 0;

		if ((!dq || !tdq) && *cmd == ' ')
			*cmd = 0;

		dq = tdq;

		cmd++;

	}

	return(argc);
}

static int command_help(int argc, char *argv[])
{
	int i;

	printk(KERN_MSG "available command\n");

	for (i=0; i < CommandCount; i++)
		printk(KERN_MSG "  %s\n", Command[i].name);

	return(0);
}

static void usage_read()
{
	printk(KERN_MSG "usage: read READ_SOURCE\n");
	printk(KERN_MSG "         READ_SOURCE\n");
	printk(KERN_MSG "           none\n");
	printk(KERN_MSG "           statistics\n");
	printk(KERN_MSG "\n");

	return;
}

static int command_read(int argc, char *argv[])
{
	if (argc != 2) {
		usage_read();
		return(-1);
	}

	if (strcmp(argv[1], "none") == 0) {
		ReadFunction = NULL;
	}
	else if (strcmp(argv[1], "statistics") == 0) {
		ReadFunction = read_statistics;
	}
	else{
		usage_read();
		return(-1);
	}

	return(0);
}

/***********************/
/*@ statistics_routine */
/***********************/

#define STATISTICS_STAT_STOP    0
#define STATISTICS_STAT_START   1

#define STATISTICS_MODE_ROUND   0
#define STATISTICS_MODE_ONETIME 1

static STATISTICS_DATA *StatisticsData  = NULL;
static int StatisticsDataPoint          = 0;
static int StatisticsDataCount          = 0;
static int StatisticsDataMaxCount       = 0;
static int StatisticsState = STATISTICS_STAT_STOP;
static int StatisticsMode  = STATISTICS_MODE_ONETIME;

static void usage_statistics(void)
{
	printk(KERN_MSG "usage: statistics alloc DATA_COUNT\n");
	printk(KERN_MSG "       statistics free\n");
	printk(KERN_MSG "       statistics clear\n");
	printk(KERN_MSG "       statistics start\n");
	printk(KERN_MSG "       statistics stop\n");
	printk(KERN_MSG "       statistics mode round|onetime\n");
	printk(KERN_MSG "       statistics stat\n");
	printk(KERN_MSG "\n");
}

static int command_statistics(int argc, char *argv[])
{
	int rv;

	if (argc < 2) {
		usage_statistics();
		return(-1);
	}

	if (strcmp(argv[1], "alloc") == 0) {
		if (argc != 3) {
			usage_statistics();
			return(-1);
		}
		if (strncmp(argv[2], "0x", 2) == 0) {
			rv = icx_debug_statistics_alloc(htoi(argv[2]));
			if (rv < 0)
				return(rv);
		}
		else{
			rv = icx_debug_statistics_alloc(atoi(argv[2]));
			if (rv < 0)
				return(rv);
		}
		printk(KERN_MSG "ok\n");
	}
	else if (strcmp(argv[1], "free") == 0) {
		if (argc != 2) {
			usage_statistics();
			return(-1);
		}
		rv = icx_debug_statistics_free();
		if (rv < 0)
			return(rv);
		printk(KERN_MSG "ok\n");
	}
	else if (strcmp(argv[1], "clear") == 0) {
		if (argc != 2) {
			usage_statistics();
			return(-1);
		}
		rv = icx_debug_statistics_clear();
		if (rv < 0)
			return(rv);
		printk(KERN_MSG "ok\n");
	}
	else if (strcmp(argv[1], "start") == 0) {
		if (argc != 2) {
			usage_statistics();
			return(-1);
		}
		rv = icx_debug_statistics_start();
		if (rv < 0)
			return(rv);
		printk(KERN_MSG "ok\n");
	}
	else if (strcmp(argv[1], "stop") == 0) {
		if (argc != 2) {
			usage_statistics();
			return(-1);
		}
		rv = icx_debug_statistics_stop();
		if (rv < 0)
			return(rv);
		printk(KERN_MSG "ok\n");
	}
	else if (strcmp(argv[1], "mode") == 0) {
		if (argc != 3) {
			usage_statistics();
			return(-1);
		}
		if (strcmp(argv[2],"round") == 0) {
			rv = icx_debug_statistics_mode(STATISTICS_MODE_ROUND);
			if (rv < 0)
				return(rv);
		}
		else if (strcmp(argv[2], "onetime") == 0) {
			rv = icx_debug_statistics_mode(STATISTICS_MODE_ONETIME);
			if (rv < 0)
				return(rv);
		}
		else{
			usage_statistics();
			return(-1);
		}
		printk(KERN_MSG "ok\n");
	}
	else if (strcmp(argv[1], "stat") == 0) {
		if (argc != 2) {
			usage_statistics();
			return(-1);
		}
		rv = icx_debug_statistics_stat();
		if (rv < 0)
			return(rv);
	}
	else if (strcmp(argv[1], "debug") == 0) {
		if (argc != 2) {
			usage_statistics();
			return(-1);
		}
		rv = icx_debug_statistics_debug();
		if (rv < 0)
			return(rv);
		printk(KERN_MSG "ok\n");
	}
	else{
		usage_statistics();
		return(-1);
	}

	return(0);
}

static int read_statistics(char __user * buf, loff_t pos, size_t size)
{
	unsigned long flags;
	int point;
	int offset;
	int count;
	int rv;

	spin_lock_irqsave(&Spinlock, flags);

	if (StatisticsData == NULL) {
		printk(KERN_ERR "%s: data buffer is not allocated.\n", __FUNCTION__);
		spin_unlock_irqrestore(&Spinlock, flags);
		return(-1);
	}

	StatisticsState = STATISTICS_STAT_STOP;

	spin_unlock_irqrestore(&Spinlock, flags);

	point = (StatisticsDataPoint + StatisticsDataMaxCount - StatisticsDataCount) % StatisticsDataMaxCount;

	offset = sizeof(STATISTICS_DATA) * point + (int)pos;
	offset = offset % (sizeof(STATISTICS_DATA) * StatisticsDataMaxCount);

	count = minimum((int)size, sizeof(STATISTICS_DATA) * StatisticsDataCount - (int)pos);
	count = minimum(count, sizeof(STATISTICS_DATA) * StatisticsDataMaxCount - offset);

	rv = copy_to_user(
		(void *)buf,
		(void *)((char *)StatisticsData + offset),
		count
	);

	if (rv != 0) {
		printk(KERN_ERR "%s: copy_to_user error\n", __FUNCTION__);
		return(-EIO);
	}

	return(count);
}

int icx_debug_statistics_alloc(int data_count)
{
	STATISTICS_DATA *tmp;
	unsigned long   flags;

	icx_debug_statistics_free();

	tmp = (STATISTICS_DATA *)vmalloc(sizeof(STATISTICS_DATA)*data_count);
	if (tmp == NULL) {
		printk(KERN_ERR "%s: vmalloc error\n",__FUNCTION__);
		return(-ENOMEM);
	}

	printk("used memory = %d KByte\n", (sizeof(STATISTICS_DATA)*data_count)>>10);

	spin_lock_irqsave(&Spinlock, flags);

	StatisticsData = tmp;

	memset((void *)StatisticsData, 0, sizeof(STATISTICS_DATA)*data_count);

	StatisticsDataPoint    = 0;
	StatisticsDataCount    = 0;
	StatisticsDataMaxCount = data_count;
	StatisticsState        = STATISTICS_STAT_STOP;
	/* StatisticsMode         = STATISTICS_MODE_ROUND; */

	spin_unlock_irqrestore(&Spinlock, flags);

	return(0);
}

int icx_debug_statistics_free(void)
{
	STATISTICS_DATA *tmp;
	unsigned long   flags;

	spin_lock_irqsave(&Spinlock, flags);

	tmp = StatisticsData;
	StatisticsData = NULL;

	StatisticsDataPoint    = 0;
	StatisticsDataCount    = 0;
	StatisticsDataMaxCount = 0;
	StatisticsState        = STATISTICS_STAT_STOP;
	/* StatisticsMode         = STATISTICS_MODE_ROUND; */

	spin_unlock_irqrestore(&Spinlock, flags);

	if (tmp != NULL)
		vfree((void *)tmp);

	return(0);
}

int icx_debug_statistics_clear(void)
{
	unsigned long flags;

	if (StatisticsData == NULL) {
		printk(KERN_ERR "%s: data buffer is not allocated.\n", __FUNCTION__);
		return(-1);
	}

	spin_lock_irqsave(&Spinlock, flags);

	memset((void *)StatisticsData, 0, sizeof(STATISTICS_DATA)*StatisticsDataMaxCount);

	StatisticsDataPoint = 0;
	StatisticsDataCount = 0;

	spin_unlock_irqrestore(&Spinlock, flags);

	return(0);
}

int icx_debug_statistics_start(void)
{
	unsigned long flags;

	if (StatisticsData == NULL) {
		printk(KERN_ERR "%s: data buffer is not allocated.\n", __FUNCTION__);
		return(-1);
	}

	spin_lock_irqsave(&Spinlock, flags);
	StatisticsState = STATISTICS_STAT_START;
	spin_unlock_irqrestore(&Spinlock, flags);

	return(0);
}

int icx_debug_statistics_stop(void)
{
	unsigned long flags;

	if (StatisticsData == NULL) {
		printk(KERN_ERR "%s: data buffer is not allocated.\n", __FUNCTION__);
		return(-1);
	}

	spin_lock_irqsave(&Spinlock, flags);
	StatisticsState = STATISTICS_STAT_STOP;
	spin_unlock_irqrestore(&Spinlock, flags);

	return(0);
}

int icx_debug_statistics_mode(int mode)
{
	unsigned long flags;

	if (StatisticsData == NULL) {
		printk(KERN_ERR "%s: data buffer is not allocated.\n", __FUNCTION__);
		return(-1);
	}

	if ((mode != STATISTICS_MODE_ROUND) && (mode != STATISTICS_MODE_ONETIME)) {
		printk(KERN_ERR "%s: invalid mode\n", __FUNCTION__);
		return(-1);
	}

	spin_lock_irqsave(&Spinlock, flags);
	StatisticsMode = mode;
	spin_unlock_irqrestore(&Spinlock, flags);

	return(0);
}

int icx_debug_statistics_stat(void)
{
	char stat[2][8] = {"stop","start"};
	char mode[2][8] = {"round","onetime"};

	printk(KERN_MSG "StatisticsData         = 0x%08X\n", (int)StatisticsData);
	printk(KERN_MSG "StatisticsDataMaxCount = %10d\n", StatisticsDataMaxCount);
	printk(KERN_MSG "StatisticsDataPoint    = %10d\n", StatisticsDataPoint);
	printk(KERN_MSG "StatisticsDataCount    = %10d\n", StatisticsDataCount);
	printk(KERN_MSG "StatisticsState        = %s\n", stat[StatisticsState]);
	printk(KERN_MSG "StatisticsMode         = %s\n", mode[StatisticsMode]);

	return(0);
}

int icx_debug_statistics_debug(void)
{
	int i;

	for (i = 0; i < (StatisticsDataMaxCount + 1); i++){
		icx_debug_statistics_save(i, 0, 0, 0);
	}

	return(0);
}

int icx_debug_statistics_save(
	unsigned int p0,
	unsigned int p1,
	unsigned int p2,
	unsigned int p3
)
{
	unsigned long flags;

	if (StatisticsData == NULL)
		return(0);

	if (StatisticsState != STATISTICS_STAT_START) {
		return(0);
	}

	spin_lock_irqsave(&Spinlock, flags);

	StatisticsData[StatisticsDataPoint].p0 = p0;
	StatisticsData[StatisticsDataPoint].p1 = p1;
	StatisticsData[StatisticsDataPoint].p2 = p2;
	StatisticsData[StatisticsDataPoint].p3 = p3;

	StatisticsDataPoint++;

	if (StatisticsDataCount < StatisticsDataMaxCount)
		StatisticsDataCount++;

	if (StatisticsDataPoint >= StatisticsDataMaxCount) {

		StatisticsDataPoint = 0;

		if (StatisticsMode == STATISTICS_MODE_ONETIME)
			StatisticsState = STATISTICS_STAT_STOP;
	}

	spin_unlock_irqrestore(&Spinlock, flags);

	return(0);
}

/******************/
/*@ basic_routine */
/******************/

extern ktime_t ktime_get(void);

int icx_debug_get_usec(ktime_t starttime)
{
	ktime_t calltime;
	s64 usecs64;
	int usecs;

	calltime = ktime_get();
	usecs64 = ktime_to_ns(ktime_sub(calltime, starttime));
	do_div(usecs64, NSEC_PER_USEC);
	usecs = usecs64;
	if (usecs == 0)
		usecs = 1;

	return usecs;
}

static void delnl(char *string)
{
	while (*string != 0) {

		if (*string == 0x0a)
			*string = 0;

		string++;
	}
}
