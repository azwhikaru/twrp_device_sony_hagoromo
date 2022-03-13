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
#include <linux/of_address.h>

#ifdef CONFIG_ICX_SILENT_LOG
static const print_success = 0;
#else
static const print_success = 1;
#endif

#define SLT_LOOP_CNT_MAXTRANS (0x511840)
extern int fp0_maxtrans_start(int cpu_id);
//unsigned int cpu_mask_array[9] = {0x0,0x0,0x2,0x6,0xE,0x1E,0x3E,0x7E,0xFE};
//unsigned int u32OnlineCPUMask=0;

unsigned int pCntTrans[8]={0,0,0,0,0,0,0,0};
static int g_iCPU0_PassFail, g_iCPU1_PassFail, g_iCPU2_PassFail, g_iCPU3_PassFail, g_iCPU4_PassFail, g_iCPU5_PassFail, g_iCPU6_PassFail, g_iCPU7_PassFail;
static int g_iMaxTransLoopCount;
unsigned int pNumIters=SLT_LOOP_CNT_MAXTRANS;

static struct device_driver slt_cpu0_maxtrans_drv = 
{   
	.name = "slt_cpu0_maxtrans",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu1_maxtrans_drv = 
{   
	.name = "slt_cpu1_maxtrans",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu2_maxtrans_drv = 
{   
	.name = "slt_cpu2_maxtrans",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu3_maxtrans_drv = 
{   
	.name = "slt_cpu3_maxtrans",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu4_maxtrans_drv =
{
	.name = "slt_cpu4_maxtrans",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu5_maxtrans_drv =
{
	.name = "slt_cpu5_maxtrans",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu6_maxtrans_drv =
{
	.name = "slt_cpu6_maxtrans",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu7_maxtrans_drv =
{
	.name = "slt_cpu7_maxtrans",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_maxtrans_loop_count_drv =
{
	.name = "slt_maxtrans_loop_count",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

#define DEFINE_SLT_CPU_MAXTRNS_SHOW(_N)	\
static ssize_t slt_cpu##_N##_maxtrans_show(struct device_driver *driver, char *buf) \
{   \
	if(g_iCPU##_N##_PassFail == -1) \
		return snprintf(buf, PAGE_SIZE, "CPU%d MaxTrans - CPU%d is powered off\n",_N,_N); \
	else	\
		return snprintf(buf, PAGE_SIZE, "CPU%d MaxTrans - %s(loop_count = %d)\n", _N, g_iCPU##_N##_PassFail != g_iMaxTransLoopCount ? "FAIL" : "PASS", g_iCPU##_N##_PassFail);  \
}

DEFINE_SLT_CPU_MAXTRNS_SHOW(0)
DEFINE_SLT_CPU_MAXTRNS_SHOW(1)
DEFINE_SLT_CPU_MAXTRNS_SHOW(2)
DEFINE_SLT_CPU_MAXTRNS_SHOW(3)
DEFINE_SLT_CPU_MAXTRNS_SHOW(4)
DEFINE_SLT_CPU_MAXTRNS_SHOW(5)
DEFINE_SLT_CPU_MAXTRNS_SHOW(6)
DEFINE_SLT_CPU_MAXTRNS_SHOW(7)

static ssize_t slt_maxtrans_loop_count_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "MaxTrans Loop Count = %d\n", g_iMaxTransLoopCount);
}

#define DEFINE_SLT_CPU_MAXTRNS_STORE(_N)	\
	DEFINE_SPINLOCK(slt_cpu_trans##_N##_lock);	\
static ssize_t slt_cpu##_N##_maxtrans_store(struct device_driver *driver, const char *buf, size_t count)	\
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
	printk("\n>>CPU%d maxtrans test start (cpu id = %d) <<\n\n", _N, raw_smp_processor_id());  \
	\
	spin_lock_irqsave(&slt_cpu_trans##_N##_lock, cpu##_N##_flags);	\
	ret = fp0_maxtrans_start(_N);	/* 1: PASS, 0:Fail, -1: target CPU /ower off */  \
	spin_unlock_irqrestore(&slt_cpu_trans##_N##_lock, cpu##_N##_flags);   \
	\
	printk("\n>> CPU%d maxtrans test pass(cycle count:%x) <<\n\n", _N,ret); \
	if ( 0 == _N) \
	{ \
	for (i=0; i<8; i++) \
	{ \
		printk("CPU%d: pass count:%d\n",i,pCntTrans[i]);	\
	} \
	} \
	\
	return count;   \
}

DEFINE_SLT_CPU_MAXTRNS_STORE(0)
DEFINE_SLT_CPU_MAXTRNS_STORE(1)
DEFINE_SLT_CPU_MAXTRNS_STORE(2)
DEFINE_SLT_CPU_MAXTRNS_STORE(3)
DEFINE_SLT_CPU_MAXTRNS_STORE(4)
DEFINE_SLT_CPU_MAXTRNS_STORE(5)
DEFINE_SLT_CPU_MAXTRNS_STORE(6)
DEFINE_SLT_CPU_MAXTRNS_STORE(7)

static ssize_t slt_maxtrans_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{ 
	int result;
	 
	if ((result = sscanf(buf, "%d", &g_iMaxTransLoopCount)) == 1)
	{
		printk("set SLT MaxTrans test loop count = %d successfully\n", g_iMaxTransLoopCount);
	pNumIters = g_iMaxTransLoopCount;
	}
	else
	{
		printk("bad argument!!\n");
		return -EINVAL;
	}
	 
	return count;
}
	
DRIVER_ATTR(slt_cpu0_maxtrans, 0644, slt_cpu0_maxtrans_show, slt_cpu0_maxtrans_store);
DRIVER_ATTR(slt_cpu1_maxtrans, 0644, slt_cpu1_maxtrans_show, slt_cpu1_maxtrans_store);
DRIVER_ATTR(slt_cpu2_maxtrans, 0644, slt_cpu2_maxtrans_show, slt_cpu2_maxtrans_store);
DRIVER_ATTR(slt_cpu3_maxtrans, 0644, slt_cpu3_maxtrans_show, slt_cpu3_maxtrans_store);
DRIVER_ATTR(slt_cpu4_maxtrans, 0644, slt_cpu4_maxtrans_show, slt_cpu4_maxtrans_store);
DRIVER_ATTR(slt_cpu5_maxtrans, 0644, slt_cpu5_maxtrans_show, slt_cpu5_maxtrans_store);
DRIVER_ATTR(slt_cpu6_maxtrans, 0644, slt_cpu6_maxtrans_show, slt_cpu6_maxtrans_store);
DRIVER_ATTR(slt_cpu7_maxtrans, 0644, slt_cpu7_maxtrans_show, slt_cpu7_maxtrans_store);
DRIVER_ATTR(slt_maxtrans_loop_count, 0644, slt_maxtrans_loop_count_show, slt_maxtrans_loop_count_store);

#define DEFINE_SLT_CPU_MAXTRNS_INIT(_N)	\
int __init slt_cpu##_N##_maxtrans_init(void) \
{   \
	int ret;	\
	\
	ret = driver_register(&slt_cpu##_N##_maxtrans_drv);  \
	if (ret) {  \
		printk("fail to create SLT CPU%d MaxTrans driver\n",_N);	\
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d MaxTrans driver\n",_N); \
	}   \
	\
	ret = driver_create_file(&slt_cpu##_N##_maxtrans_drv, &driver_attr_slt_cpu##_N##_maxtrans);   \
	if (ret) {  \
		printk("fail to create SLT CPU%d MaxTrans sysfs files\n",_N);   \
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d MaxTrans sysfs files\n",_N);	\
	}   \
	\
	return 0;   \
}

DEFINE_SLT_CPU_MAXTRNS_INIT(0)
DEFINE_SLT_CPU_MAXTRNS_INIT(1)
DEFINE_SLT_CPU_MAXTRNS_INIT(2)
DEFINE_SLT_CPU_MAXTRNS_INIT(3)
DEFINE_SLT_CPU_MAXTRNS_INIT(4)
DEFINE_SLT_CPU_MAXTRNS_INIT(5)
DEFINE_SLT_CPU_MAXTRNS_INIT(6)
DEFINE_SLT_CPU_MAXTRNS_INIT(7)

unsigned int pMaxTransTestMem;
unsigned int pNumTrans[8];
int __init slt_maxtrans_loop_count_init(void)
{
	int ret;	

	pMaxTransTestMem = (unsigned int)vmalloc(8*1024*8);//we need to prepare 64KB memory for test, each cpu use 8KB
	if((void*)pMaxTransTestMem == NULL) 
	{  
		printk("allocate memory for cpu maxtrans test fail\n");
		return 0;   
	}
	else
	{
		printk("maxtrans test memory = 0x%x\n",pMaxTransTestMem);
	}	

	memset ( pNumTrans, 0x2,sizeof(pNumTrans));
	ret = driver_register(&slt_maxtrans_loop_count_drv);
	if (ret) {
		printk("fail to create MaxTrans loop count driver\n");
	}
	else if(print_success)
	{
		printk("success to create MaxTrans loop count driver\n");
	}
  

	ret = driver_create_file(&slt_maxtrans_loop_count_drv, &driver_attr_slt_maxtrans_loop_count);
	if (ret) {
		printk("fail to create MaxTrans loop count sysfs files\n");
	}
	else if(print_success)
	{
		printk("success to create MaxTrans loop count sysfs files\n");
	}

	g_iMaxTransLoopCount = SLT_LOOP_CNT_MAXTRANS;
	
	return 0;
}

arch_initcall(slt_cpu0_maxtrans_init);
arch_initcall(slt_cpu1_maxtrans_init);
arch_initcall(slt_cpu2_maxtrans_init);
arch_initcall(slt_cpu3_maxtrans_init);
arch_initcall(slt_cpu4_maxtrans_init);
arch_initcall(slt_cpu5_maxtrans_init);
arch_initcall(slt_cpu6_maxtrans_init);
arch_initcall(slt_cpu7_maxtrans_init);
arch_initcall(slt_maxtrans_loop_count_init);
