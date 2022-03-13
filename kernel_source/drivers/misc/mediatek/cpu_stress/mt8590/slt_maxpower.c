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


#include <linux/sched.h>
#include <linux/vmalloc.h>
#include "slt.h"
#include <linux/io.h>
#include <asm/pgtable.h>
#include <linux/memblock.h>

#ifdef CONFIG_ICX_SILENT_LOG
static const print_success = 0;
#else
static const print_success = 1;
#endif

extern int fp0_maxpower_start(int cpu_id);
static unsigned int parameter[8];
static DEFINE_MUTEX(cpu_stress_mutex);


static int g_iCPU0_PassFail, g_iCPU1_PassFail, g_iCPU2_PassFail, g_iCPU3_PassFail, g_iCPU4_PassFail, g_iCPU5_PassFail, g_iCPU6_PassFail, g_iCPU7_PassFail;
static int g_iMaxPowerLoopCount;

static int g_iCPU_PassFail[8];
static struct device_driver slt_cpu0_maxpower_drv = 
{   
	.name = "slt_cpu0_maxpower",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu1_maxpower_drv = 
{   
	.name = "slt_cpu1_maxpower",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu2_maxpower_drv = 
{   
	.name = "slt_cpu2_maxpower",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu3_maxpower_drv = 
{   
	.name = "slt_cpu3_maxpower",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu4_maxpower_drv =
{
	.name = "slt_cpu4_maxpower",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu5_maxpower_drv =
{
	.name = "slt_cpu5_maxpower",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu6_maxpower_drv =
{
	.name = "slt_cpu6_maxpower",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu7_maxpower_drv =
{
	.name = "slt_cpu7_maxpower",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_maxpower_loop_count_drv =
{
	.name = "slt_maxpower_loop_count",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

#define DEFINE_SLT_CPU_MAXPOWER_SHOW(_N)	\
static ssize_t slt_cpu##_N##_maxpower_show(struct device_driver *driver, char *buf) \
{   \
	if(g_iCPU##_N##_PassFail == -1) \
		return snprintf(buf, PAGE_SIZE, "CPU%d MaxPower - CPU%d is powered off\n",_N,_N); \
	else	\
		return snprintf(buf, PAGE_SIZE, "CPU%d MaxPower - %s(loop_count = %d)\n", _N, g_iCPU##_N##_PassFail != g_iMaxPowerLoopCount ? "FAIL" : "PASS", g_iCPU##_N##_PassFail);  \
}

DEFINE_SLT_CPU_MAXPOWER_SHOW(0)
DEFINE_SLT_CPU_MAXPOWER_SHOW(1)
DEFINE_SLT_CPU_MAXPOWER_SHOW(2)
DEFINE_SLT_CPU_MAXPOWER_SHOW(3)
DEFINE_SLT_CPU_MAXPOWER_SHOW(4)
DEFINE_SLT_CPU_MAXPOWER_SHOW(5)
DEFINE_SLT_CPU_MAXPOWER_SHOW(6)
DEFINE_SLT_CPU_MAXPOWER_SHOW(7)

static ssize_t slt_maxpower_loop_count_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "MaxPower Loop Count = %d\n", g_iMaxPowerLoopCount);
}

#define DEFINE_SLT_CPU_MAXPOWER_STORE(_N)	\
	DEFINE_SPINLOCK(slt_cpu##_N##_lock);	\
static ssize_t slt_cpu##_N##_maxpower_store(struct device_driver *driver, const char *buf, size_t count)	\
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
	printk("\n>> CPU%d maxpower test start (cpu id = %d) <<\n\n", _N, raw_smp_processor_id());  \
	\
	for (i = 0, g_iCPU##_N##_PassFail = 0; i < g_iMaxPowerLoopCount; i++) {	\
		spin_lock_irqsave(&slt_cpu##_N##_lock, cpu##_N##_flags);	\
		ret = fp0_maxpower_start(_N);	/* 1: PASS, 0:Fail, -1: target CPU power off */  \
		spin_unlock_irqrestore(&slt_cpu##_N##_lock, cpu##_N##_flags);   \
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
	if (g_iCPU##_N##_PassFail == g_iMaxPowerLoopCount) {	\
		printk("\n>> CPU%d maxpower test pass <<\n\n", _N); \
	}else { \
		printk("\n>> CPU%d maxpower test fail (loop count = %d)<<\n\n", _N, g_iCPU##_N##_PassFail);  \
	}   \
	\
	return count;   \
}

DEFINE_SLT_CPU_MAXPOWER_STORE(0)
DEFINE_SLT_CPU_MAXPOWER_STORE(1)
DEFINE_SLT_CPU_MAXPOWER_STORE(2)
DEFINE_SLT_CPU_MAXPOWER_STORE(3)
DEFINE_SLT_CPU_MAXPOWER_STORE(4)
DEFINE_SLT_CPU_MAXPOWER_STORE(5)
DEFINE_SLT_CPU_MAXPOWER_STORE(6)
DEFINE_SLT_CPU_MAXPOWER_STORE(7)

static ssize_t slt_maxpower_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{ 
	int result;
	 
	if ((result = sscanf(buf, "%d", &g_iMaxPowerLoopCount)) == 1)
	{
		printk("set SLT MaxPower test loop count = %d successfully\n", g_iMaxPowerLoopCount);
	}
	else
	{
		printk("bad argument!!\n");
		return -EINVAL;
	}
	 
	return count;
}
	
DRIVER_ATTR(slt_cpu0_maxpower, 0644, slt_cpu0_maxpower_show, slt_cpu0_maxpower_store);
DRIVER_ATTR(slt_cpu1_maxpower, 0644, slt_cpu1_maxpower_show, slt_cpu1_maxpower_store);
DRIVER_ATTR(slt_cpu2_maxpower, 0644, slt_cpu2_maxpower_show, slt_cpu2_maxpower_store);
DRIVER_ATTR(slt_cpu3_maxpower, 0644, slt_cpu3_maxpower_show, slt_cpu3_maxpower_store);
DRIVER_ATTR(slt_cpu4_maxpower, 0644, slt_cpu4_maxpower_show, slt_cpu4_maxpower_store);
DRIVER_ATTR(slt_cpu5_maxpower, 0644, slt_cpu5_maxpower_show, slt_cpu5_maxpower_store);
DRIVER_ATTR(slt_cpu6_maxpower, 0644, slt_cpu6_maxpower_show, slt_cpu6_maxpower_store);
DRIVER_ATTR(slt_cpu7_maxpower, 0644, slt_cpu7_maxpower_show, slt_cpu7_maxpower_store);
DRIVER_ATTR(slt_maxpower_loop_count, 0644, slt_maxpower_loop_count_show, slt_maxpower_loop_count_store);

#define DEFINE_SLT_CPU_MAXPOWER_INIT(_N)	\
int __init slt_cpu##_N##_maxpower_init(void) \
{   \
	int ret;	\
	\
	ret = driver_register(&slt_cpu##_N##_maxpower_drv);  \
	if (ret) {  \
		printk("fail to create SLT CPU%d MaxPower driver\n",_N);	\
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d MaxPower driver\n",_N); \
	}   \
	\
	ret = driver_create_file(&slt_cpu##_N##_maxpower_drv, &driver_attr_slt_cpu##_N##_maxpower);   \
	if (ret) {  \
		printk("fail to create SLT CPU%d MaxPower sysfs files\n",_N);   \
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d MaxPower sysfs files\n",_N);	\
	}   \
	\
	return 0;   \
}

DEFINE_SLT_CPU_MAXPOWER_INIT(0)
DEFINE_SLT_CPU_MAXPOWER_INIT(1)
DEFINE_SLT_CPU_MAXPOWER_INIT(2)
DEFINE_SLT_CPU_MAXPOWER_INIT(3)
DEFINE_SLT_CPU_MAXPOWER_INIT(4)
DEFINE_SLT_CPU_MAXPOWER_INIT(5)
DEFINE_SLT_CPU_MAXPOWER_INIT(6)
DEFINE_SLT_CPU_MAXPOWER_INIT(7)

int run_maxpower(void *arg)
{
	unsigned int i, ret;
	unsigned long mask;
	int retry=0;
	unsigned int cpu_id;

	cpu_id = *((unsigned int*) arg);

	g_iCPU_PassFail[cpu_id] = 0;

	mask = (1 << cpu_id); /* processor _N */
	printk("\n>> CPU%d maxpower test start (cpu id = %d) <<\n\n", cpu_id, raw_smp_processor_id());

	for (i = 0, g_iCPU_PassFail[cpu_id] = 0; i < g_iMaxPowerLoopCount; i++) {
		ret = fp0_maxpower_start(cpu_id);   /* 1: PASS, 0:Fail, -1: target CPU power off */  
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

	if (g_iCPU_PassFail[cpu_id] == g_iMaxPowerLoopCount) {
		printk("\n>> CPU%d maxpower test pass <<\n\n", cpu_id);
	}else {
		printk("\n>> CPU%d maxpower test fail (loop count = %d)<<\n\n", cpu_id, g_iCPU_PassFail[cpu_id]);
	}
	return 0;
}

dma_addr_t pMaxPowerTestMem;
int __init slt_maxpower_loop_count_init(void)
{
	int ret;	
  int cpu;
  struct task_struct *t_thread[8];

	//reserve for cpu stress test
	pMaxPowerTestMem = (dma_addr_t)vmalloc(0x10000);
	if((void*)pMaxPowerTestMem == NULL) 
	{  
		printk("allocate memory for cpu maxpower test fail\n");
		return 0;   
	}
	else
	{
#ifdef CONFIG_ARM64
		printk("maxpower test memory = 0x%llx\n",pMaxPowerTestMem);
#else
		printk("maxpower test memory = 0x%x\n",pMaxPowerTestMem);
#endif
	//memset ( pMaxPowerTestMem, 0xffffffff ,8*1024);
	}	

	ret = driver_register(&slt_maxpower_loop_count_drv);
	if (ret) {
		printk("fail to create MaxPower loop count driver\n");
	}
	else if(print_success)
	{
		printk("success to create MaxPower loop count driver\n");
	}
  

	ret = driver_create_file(&slt_maxpower_loop_count_drv, &driver_attr_slt_maxpower_loop_count);
	if (ret) {
		printk("fail to create MaxPower loop count sysfs files\n");
	}
	else if(print_success)
	{
		printk("success to create MaxPower loop count sysfs files\n");
	}

	g_iMaxPowerLoopCount = SLT_LOOP_CNT;

#ifdef SLT_64BIT	
	for_each_present_cpu(cpu)
	{
		printk("start cpu%d\n",cpu);
		parameter[cpu] = cpu;
		t_thread[cpu] = kthread_create(run_maxpower,(void *) (parameter+cpu) , "thread/%d", cpu);
		if (IS_ERR(t_thread[cpu])) {
			printk(KERN_ERR "[thread/%d]: creating kthread failed\n", cpu);
			return -1;
		}
		kthread_bind(t_thread[cpu], cpu);
		wake_up_process(t_thread[cpu]);
	}
#endif
	
	return 0;
}
arch_initcall(slt_cpu0_maxpower_init);
arch_initcall(slt_cpu1_maxpower_init);
arch_initcall(slt_cpu2_maxpower_init);
arch_initcall(slt_cpu3_maxpower_init);
arch_initcall(slt_cpu4_maxpower_init);
arch_initcall(slt_cpu5_maxpower_init);
arch_initcall(slt_cpu6_maxpower_init);
arch_initcall(slt_cpu7_maxpower_init);
arch_initcall(slt_maxpower_loop_count_init);
