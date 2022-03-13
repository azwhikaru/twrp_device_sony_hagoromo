#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wakelock.h>
#include <linux/module.h>
#include <asm/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gfp.h>
#include <asm/io.h>
#include <asm/memory.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>

#ifdef CONFIG_ARM64
#include <asm/neon.h>
#endif

#include <linux/sched.h>
#include <linux/vmalloc.h>
#include "../slt.h"
#include <linux/io.h>
#include <asm/pgtable.h>
#define sleep_millisecs 1000*60

#ifdef CONFIG_ICX_SILENT_LOG
static const print_success = 0;
#else
static const print_success = 1;
#endif

extern int fp5_saxpy_start(int N, float a[], float b[], float constant);


#define NLOOPS 10 // should be more than enough to get to stationary state
#define ARRAY_SIZE_SAXPY 208
float x[ARRAY_SIZE_SAXPY];
float y[ARRAY_SIZE_SAXPY];
float null_a;
float null_b;


static int g_iCPU0_PassFail, g_iCPU1_PassFail, g_iCPU2_PassFail, g_iCPU3_PassFail, g_iCPU4_PassFail, g_iCPU5_PassFail, g_iCPU6_PassFail, g_iCPU7_PassFail;
static int g_iSaxpyLoopCount;
static int g_iCPU_PassFail[8];
static int g_iSaxpyLoopCount;
static unsigned int parameter[8];
static struct device_driver slt_cpu0_saxpy_drv =
{
	.name = "slt_cpu0_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu1_saxpy_drv =
{
	.name = "slt_cpu1_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu2_saxpy_drv =
{
	.name = "slt_cpu2_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu3_saxpy_drv =
{
	.name = "slt_cpu3_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu4_saxpy_drv =
{
	.name = "slt_cpu4_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu5_saxpy_drv =
{
	.name = "slt_cpu5_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu6_saxpy_drv =
{
	.name = "slt_cpu6_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu7_saxpy_drv =
{
	.name = "slt_cpu7_saxpy",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_saxpy_loop_count_drv =
{
	.name = "slt_saxpy_loop_count",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

#define DEFINE_SLT_CPU_SAXPY_SHOW(_N)	\
static ssize_t slt_cpu##_N##_saxpy_show(struct device_driver *driver, char *buf) \
{   \
	if(g_iCPU##_N##_PassFail == -1) \
		return snprintf(buf, PAGE_SIZE, "CPU%d SAXPY - CPU%d is powered off\n",_N,_N); \
	else	\
		return snprintf(buf, PAGE_SIZE, "CPU%d SAXPY - %s(loop_count = %d)\n", _N, g_iCPU##_N##_PassFail != g_iSaxpyLoopCount ? "FAIL" : "PASS", g_iCPU##_N##_PassFail);  \
}

DEFINE_SLT_CPU_SAXPY_SHOW(0)
DEFINE_SLT_CPU_SAXPY_SHOW(1)
DEFINE_SLT_CPU_SAXPY_SHOW(2)
DEFINE_SLT_CPU_SAXPY_SHOW(3)
DEFINE_SLT_CPU_SAXPY_SHOW(4)
DEFINE_SLT_CPU_SAXPY_SHOW(5)
DEFINE_SLT_CPU_SAXPY_SHOW(6)
DEFINE_SLT_CPU_SAXPY_SHOW(7)

static ssize_t slt_saxpy_loop_count_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "SAXPY Test Loop Count = %d\n", g_iSaxpyLoopCount);
}

static void backup_fpsimd()
{
#ifdef CONFIG_ARM64  
  kernel_neon_begin();
#endif  
  return;
}

static void restore_fpsimd()
{
#ifdef CONFIG_ARM64  
  kernel_neon_end();
#endif  
  return;
}

#define DEFINE_SLT_CPU_SAXPY_STORE(_N)	\
	DEFINE_SPINLOCK(saxpy_cpu##_N##_lock);	\
static ssize_t slt_cpu##_N##_saxpy_store(struct device_driver *driver, const char *buf, size_t count)	\
{   \
	unsigned int i, ret;	\
	unsigned long mask; \
	int retry=0;	\
	unsigned long cpu##_N##_flags;	 \
	const unsigned int int_const_c = 0xC94DAD43; \
	const unsigned int int_const_d = 0x48DD73F1; \
	const float float_const_c = *((float *) &int_const_c); \
	const float float_const_d = *((float *) &int_const_d); \
	float constant; \	
	\
	g_iCPU##_N##_PassFail = 0;  \
	\
	mask = (1 << _N); /* processor _N */ \
	while(sched_setaffinity(0, (struct cpumask*) &mask) < 0)	\
	{   \
		printk("Could not set cpu%d affinity for current process(%d).\n", _N, retry);  \
		g_iCPU##_N##_PassFail = -1; \
		retry++;	\
		if(retry > 100) \
		{   \
			return count;   \
		}   \
	}   \
	\
	printk("\n>> CPU%d saxpy test start (cpu id = %d) <<\n\n", _N, raw_smp_processor_id());  \
	\
	/* backup fpsimd registers */ \
	backup_fpsimd();\
	\
	for (i = 0, g_iCPU##_N##_PassFail = 0; i < g_iSaxpyLoopCount; i++) {	\
	constant = (i & 0x1) ? float_const_c : float_const_d; \
		spin_lock_irqsave(&saxpy_cpu##_N##_lock, cpu##_N##_flags);	\
		ret = fp5_saxpy_start(_N, x, y, constant);   /* 1: PASS, 0:Fail, -1: target CPU power off */  \
	/*ret = fp5_saxpy_start();*/ \
		spin_unlock_irqrestore(&saxpy_cpu##_N##_lock, cpu##_N##_flags);   \
		if(ret != -1)   \
		{   \
			 g_iCPU##_N##_PassFail += ret;  \
		}   \
		else	\
		{   \
			 g_iCPU##_N##_PassFail = -1;	\
			 break; \
		}   \
	}   \
	\
	if (g_iCPU##_N##_PassFail == g_iSaxpyLoopCount) {	\
		printk("\n>> CPU%d saxpy test pass <<\n\n", _N); \
	}else { \
		printk("\n>> CPU%d saxpy test fail (loop count = %d)<<\n\n", _N, g_iCPU##_N##_PassFail);  \
	}   \
	\
	/* restor fpsimd registers */ \
	restore_fpsimd();\
	\
	return count;   \
}
int run_saxpy(void *arg)
{
	unsigned int i, ret;
	unsigned long mask;
	int retry=0;
	const unsigned int int_const_c = 0xC94DAD43;
	const unsigned int int_const_d = 0x48DD73F1;
	const float float_const_c = *((float *) &int_const_c);
	const float float_const_d = *((float *) &int_const_d);
	float constant;
	unsigned int cpu_id;
	cpu_id = *((unsigned int*) arg);

	g_iCPU_PassFail[cpu_id] = 0;

	mask = (1 << cpu_id); /* processor _N */

	printk("\n>> CPU%d saxpy test start (cpu id = %d) <<\n\n", cpu_id, raw_smp_processor_id());

	for (i = 0, g_iCPU_PassFail[cpu_id] = 0; i < g_iSaxpyLoopCount; i++)
	{
		constant = (i & 0x1) ? float_const_c : float_const_d;
		//spin_lock_irqsave(&saxpy_cpu##_N##_lock, cpu##_N##_flags);
		ret = fp5_saxpy_start(cpu_id, x, y, constant);   /* 1: PASS, 0:Fail, -1: target CPU power off */
		//spin_unlock_irqrestore(&saxpy_cpu##_N##_lock, cpu##_N##_flags);
		if(ret != -1)
		{
			g_iCPU_PassFail[cpu_id] += ret;
		}
		else
		{
			g_iCPU_PassFail[cpu_id] = -1;
			break;
		}
	}

	if (g_iCPU_PassFail[cpu_id] == g_iSaxpyLoopCount)
	{
		printk("\n>> CPU%d saxpy test pass <<\n\n", cpu_id);
	}
	else
	{
		printk("\n>> CPU%d saxpy test fail (loop count = %d)<<\n\n", cpu_id, g_iCPU_PassFail[cpu_id]);
	}
	return 0;
}

DEFINE_SLT_CPU_SAXPY_STORE(0)
DEFINE_SLT_CPU_SAXPY_STORE(1)
DEFINE_SLT_CPU_SAXPY_STORE(2)
DEFINE_SLT_CPU_SAXPY_STORE(3)
DEFINE_SLT_CPU_SAXPY_STORE(4)
DEFINE_SLT_CPU_SAXPY_STORE(5)
DEFINE_SLT_CPU_SAXPY_STORE(6)
DEFINE_SLT_CPU_SAXPY_STORE(7)

static ssize_t slt_saxpy_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{
	int result;

	if ((result = sscanf(buf, "%d", &g_iSaxpyLoopCount)) == 1)
	{
		printk("set SLT saxpy test loop count = %d successfully\n", g_iSaxpyLoopCount);
	}
	else
	{
		printk("bad argument!!\n");
		return -EINVAL;
	}

	return count;
}

DRIVER_ATTR(slt_cpu0_saxpy, 0644, slt_cpu0_saxpy_show, slt_cpu0_saxpy_store);
DRIVER_ATTR(slt_cpu1_saxpy, 0644, slt_cpu1_saxpy_show, slt_cpu1_saxpy_store);
DRIVER_ATTR(slt_cpu2_saxpy, 0644, slt_cpu2_saxpy_show, slt_cpu2_saxpy_store);
DRIVER_ATTR(slt_cpu3_saxpy, 0644, slt_cpu3_saxpy_show, slt_cpu3_saxpy_store);
DRIVER_ATTR(slt_cpu4_saxpy, 0644, slt_cpu4_saxpy_show, slt_cpu4_saxpy_store);
DRIVER_ATTR(slt_cpu5_saxpy, 0644, slt_cpu5_saxpy_show, slt_cpu5_saxpy_store);
DRIVER_ATTR(slt_cpu6_saxpy, 0644, slt_cpu6_saxpy_show, slt_cpu6_saxpy_store);
DRIVER_ATTR(slt_cpu7_saxpy, 0644, slt_cpu7_saxpy_show, slt_cpu7_saxpy_store);
DRIVER_ATTR(slt_saxpy_loop_count, 0644, slt_saxpy_loop_count_show, slt_saxpy_loop_count_store);

#define DEFINE_SLT_CPU_SAXPY_INIT(_N)	\
int __init slt_cpu##_N##_saxpy_init(void) \
{   \
	int ret;	\
	\
	ret = driver_register(&slt_cpu##_N##_saxpy_drv);  \
	if (ret) {  \
		printk("fail to create SLT CPU%d saxpy driver\n",_N);	\
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d saxpy driver\n",_N); \
	}   \
	\
	ret = driver_create_file(&slt_cpu##_N##_saxpy_drv, &driver_attr_slt_cpu##_N##_saxpy);   \
	if (ret) {  \
		printk("fail to create SLT CPU%d saxpy sysfs files\n",_N);   \
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d saxpy sysfs files\n",_N);	\
	}   \
	\
	return 0;   \
}

DEFINE_SLT_CPU_SAXPY_INIT(0)
DEFINE_SLT_CPU_SAXPY_INIT(1)
DEFINE_SLT_CPU_SAXPY_INIT(2)
DEFINE_SLT_CPU_SAXPY_INIT(3)
DEFINE_SLT_CPU_SAXPY_INIT(4)
DEFINE_SLT_CPU_SAXPY_INIT(5)
DEFINE_SLT_CPU_SAXPY_INIT(6)
DEFINE_SLT_CPU_SAXPY_INIT(7)

int __init slt_saxpy_loop_count_init(void)
{
	int ret;

	int index;
	int cpu;
	struct task_struct *t_thread[8];
	// Want to get those 2 hex numbers into the float variables
	// without a int->float conversion. Got to use pointers to
	// prevent C from casting to float.
	const unsigned int int_const_a = 0x51523DC9;
	const unsigned int int_const_b = 0x0A4C2AD3;
	const float float_const_a = *((float *) &int_const_a);
	const float float_const_b = *((float *) &int_const_b);
	for (index = 0; index < ARRAY_SIZE_SAXPY; index++)
	{
		// Note: be careful of data type promotion here, and loss of accuracy,
		// as we are storing integers into float variables.
		x[index] = float_const_a;
		y[index] = float_const_b;
	}
	null_a = float_const_a;
	null_b = float_const_b;


	ret = driver_register(&slt_saxpy_loop_count_drv);
	if (ret)
	{
		printk("fail to create saxpy loop count driver\n");
	}
	else if(print_success)
	{
		printk("success to create saxpy loop count driver\n");
	}


	ret = driver_create_file(&slt_saxpy_loop_count_drv, &driver_attr_slt_saxpy_loop_count);
	if (ret)
	{
		printk("fail to create saxpy loop count sysfs files\n");
	}
	else if(print_success)
	{
		printk("success to create saxpy loop count sysfs files\n");
	}

	g_iSaxpyLoopCount = SLT_LOOP_CNT;

#ifdef SLT_64BIT
	for_each_present_cpu(cpu)
	{
		printk("start cpu%d\n",cpu);
		parameter[cpu] = cpu;
		t_thread[cpu] = kthread_create(run_saxpy,(void *) (parameter+cpu) , "thread/%d", cpu);
		if (IS_ERR(t_thread[cpu]))
		{
			printk(KERN_ERR "[thread/%d]: creating kthread failed\n", cpu);
			return -1;
		}
		kthread_bind(t_thread[cpu], cpu);
		wake_up_process(t_thread[cpu]);
	}
#endif
	return 0;
}
arch_initcall(slt_cpu0_saxpy_init);
arch_initcall(slt_cpu1_saxpy_init);
arch_initcall(slt_cpu2_saxpy_init);
arch_initcall(slt_cpu3_saxpy_init);
arch_initcall(slt_cpu4_saxpy_init);
arch_initcall(slt_cpu5_saxpy_init);
arch_initcall(slt_cpu6_saxpy_init);
arch_initcall(slt_cpu7_saxpy_init);
arch_initcall(slt_saxpy_loop_count_init);
