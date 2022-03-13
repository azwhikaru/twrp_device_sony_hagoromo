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
#include <asm/outercache.h>
#include <linux/spinlock.h>

//#include <linux/leds-mt65xx.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include "slt.h"
#include <linux/io.h>
#include <asm/pgtable.h>

#ifdef CONFIG_ICX_SILENT_LOG
static const print_success = 0;
#else
static const print_success = 1;
#endif

extern int fp8_imdct_start(int cpu_id);

static int g_iCPU0_PassFail, g_iCPU1_PassFail, g_iCPU2_PassFail, g_iCPU3_PassFail, g_iCPU4_PassFail, g_iCPU5_PassFail, g_iCPU6_PassFail, g_iCPU7_PassFail;
static int g_iImdctLoopCount;

static struct device_driver slt_cpu0_imdct_drv = 
{   
	.name = "slt_cpu0_imdct",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu1_imdct_drv = 
{   
	.name = "slt_cpu1_imdct",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu2_imdct_drv = 
{   
	.name = "slt_cpu2_imdct",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu3_imdct_drv = 
{   
	.name = "slt_cpu3_imdct",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu4_imdct_drv =
{
	.name = "slt_cpu4_imdct",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu5_imdct_drv =
{
	.name = "slt_cpu5_imdct",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu6_imdct_drv =
{
	.name = "slt_cpu6_imdct",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu7_imdct_drv =
{
	.name = "slt_cpu7_imdct",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_imdct_loop_count_drv =
{
	.name = "slt_imdct_loop_count",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

#define DEFINE_SLT_CPU_IMDCT_SHOW(_N)	\
static ssize_t slt_cpu##_N##_imdct_show(struct device_driver *driver, char *buf) \
{   \
	if(g_iCPU##_N##_PassFail == -1) \
		return snprintf(buf, PAGE_SIZE, "CPU%d imdct - CPU%d is powered off\n",_N,_N); \
	else	\
		return snprintf(buf, PAGE_SIZE, "CPU%d imdct - %s(loop_count = %d)\n", _N, g_iCPU##_N##_PassFail != g_iImdctLoopCount ? "FAIL" : "PASS", g_iCPU##_N##_PassFail);  \
}

DEFINE_SLT_CPU_IMDCT_SHOW(0)
DEFINE_SLT_CPU_IMDCT_SHOW(1)
DEFINE_SLT_CPU_IMDCT_SHOW(2)
DEFINE_SLT_CPU_IMDCT_SHOW(3)
DEFINE_SLT_CPU_IMDCT_SHOW(4)
DEFINE_SLT_CPU_IMDCT_SHOW(5)
DEFINE_SLT_CPU_IMDCT_SHOW(6)
DEFINE_SLT_CPU_IMDCT_SHOW(7)

static ssize_t slt_imdct_loop_count_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "IMDCT Test Loop Count = %d\n", g_iImdctLoopCount);
}

#define DEFINE_SLT_CPU_IMDCT_STORE(_N)	\
	DEFINE_SPINLOCK(imdct_cpu##_N##_lock);	\
static ssize_t slt_cpu##_N##_imdct_store(struct device_driver *driver, const char *buf, size_t count)	\
{   \
	unsigned int i, ret;	\
	unsigned long mask; \
	int retry=0;	\	
	unsigned long cpu##_N##_flags;	 \
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
	printk("\n>> CPU%d imdct test start (cpu id = %d) <<\n\n", _N, raw_smp_processor_id());  \
	\
	for (i = 0, g_iCPU##_N##_PassFail = 0; i < g_iImdctLoopCount; i++) {	\
		spin_lock_irqsave(&imdct_cpu##_N##_lock, cpu##_N##_flags);	\
		ret = fp8_imdct_start(_N);	/* 1: PASS, 0:Fail, -1: target CPU power off */  \
		spin_unlock_irqrestore(&imdct_cpu##_N##_lock, cpu##_N##_flags);   \
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
	if (g_iCPU##_N##_PassFail == g_iImdctLoopCount) {	\
		printk("\n>> CPU%d imdct test pass <<\n\n", _N); \
	}else { \
		printk("\n>> CPU%d imdct test fail (loop count = %d)<<\n\n", _N, g_iCPU##_N##_PassFail);  \
	}   \
	\
	return count;   \
}

DEFINE_SLT_CPU_IMDCT_STORE(0)
DEFINE_SLT_CPU_IMDCT_STORE(1)
DEFINE_SLT_CPU_IMDCT_STORE(2)
DEFINE_SLT_CPU_IMDCT_STORE(3)
DEFINE_SLT_CPU_IMDCT_STORE(4)
DEFINE_SLT_CPU_IMDCT_STORE(5)
DEFINE_SLT_CPU_IMDCT_STORE(6)
DEFINE_SLT_CPU_IMDCT_STORE(7)

static ssize_t slt_imdct_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{ 
	int result;
	 
	if ((result = sscanf(buf, "%d", &g_iImdctLoopCount)) == 1)
	{
		printk("set SLT imdct test loop count = %d successfully\n", g_iImdctLoopCount);
	}
	else
	{
		printk("bad argument!!\n");
		return -EINVAL;
	}
	 
	return count;
}
	
DRIVER_ATTR(slt_cpu0_imdct, 0644, slt_cpu0_imdct_show, slt_cpu0_imdct_store);
DRIVER_ATTR(slt_cpu1_imdct, 0644, slt_cpu1_imdct_show, slt_cpu1_imdct_store);
DRIVER_ATTR(slt_cpu2_imdct, 0644, slt_cpu2_imdct_show, slt_cpu2_imdct_store);
DRIVER_ATTR(slt_cpu3_imdct, 0644, slt_cpu3_imdct_show, slt_cpu3_imdct_store);
DRIVER_ATTR(slt_cpu4_imdct, 0644, slt_cpu4_imdct_show, slt_cpu4_imdct_store);
DRIVER_ATTR(slt_cpu5_imdct, 0644, slt_cpu5_imdct_show, slt_cpu5_imdct_store);
DRIVER_ATTR(slt_cpu6_imdct, 0644, slt_cpu6_imdct_show, slt_cpu6_imdct_store);
DRIVER_ATTR(slt_cpu7_imdct, 0644, slt_cpu7_imdct_show, slt_cpu7_imdct_store);
DRIVER_ATTR(slt_imdct_loop_count, 0644, slt_imdct_loop_count_show, slt_imdct_loop_count_store);

#define DEFINE_SLT_CPU_IMDCT_INIT(_N)	\
int __init slt_cpu##_N##_imdct_init(void) \
{   \
	int ret;	\
	\
	ret = driver_register(&slt_cpu##_N##_imdct_drv);  \
	if (ret) {  \
		printk("fail to create SLT CPU%d imdct driver\n",_N);	\
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d imdct driver\n",_N); \
	}   \
	\
	ret = driver_create_file(&slt_cpu##_N##_imdct_drv, &driver_attr_slt_cpu##_N##_imdct);   \
	if (ret) {  \
		printk("fail to create SLT CPU%d imdct sysfs files\n",_N);   \
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d imdct sysfs files\n",_N);	\
	}   \
	\
	return 0;   \
}

DEFINE_SLT_CPU_IMDCT_INIT(0)
DEFINE_SLT_CPU_IMDCT_INIT(1)
DEFINE_SLT_CPU_IMDCT_INIT(2)
DEFINE_SLT_CPU_IMDCT_INIT(3)
DEFINE_SLT_CPU_IMDCT_INIT(4)
DEFINE_SLT_CPU_IMDCT_INIT(5)
DEFINE_SLT_CPU_IMDCT_INIT(6)
DEFINE_SLT_CPU_IMDCT_INIT(7)

int __init slt_imdct_loop_count_init(void)
{
	int ret;

	ret = driver_register(&slt_imdct_loop_count_drv);
	if (ret) {
		printk("fail to create imdct loop count driver\n");
	}
	else if(print_success)
	{
		printk("success to create imdct loop count driver\n");
	}
	

	ret = driver_create_file(&slt_imdct_loop_count_drv, &driver_attr_slt_imdct_loop_count);
	if (ret) {
		printk("fail to create imdct loop count sysfs files\n");
	}
	else if(print_success)
	{
		printk("success to create imdct loop count sysfs files\n");
	}

	g_iImdctLoopCount = SLT_LOOP_CNT;
	
	return 0;
}
arch_initcall(slt_cpu0_imdct_init);
arch_initcall(slt_cpu1_imdct_init);
arch_initcall(slt_cpu2_imdct_init);
arch_initcall(slt_cpu3_imdct_init);
arch_initcall(slt_cpu4_imdct_init);
arch_initcall(slt_cpu5_imdct_init);
arch_initcall(slt_cpu6_imdct_init);
arch_initcall(slt_cpu7_imdct_init);
arch_initcall(slt_imdct_loop_count_init);
