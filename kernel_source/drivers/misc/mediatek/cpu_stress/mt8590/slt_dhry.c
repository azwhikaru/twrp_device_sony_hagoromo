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

#ifdef CONFIG_ICX_SILENT_LOG
static const print_success = 0;
#else
static const print_success = 1;
#endif

static DEFINE_MUTEX(cpu_stress_mutex);
extern int fp1_dhry_start(void);

static int g_iCPU0_PassFail, g_iCPU1_PassFail, g_iCPU2_PassFail, g_iCPU3_PassFail, g_iCPU4_PassFail, g_iCPU5_PassFail, g_iCPU6_PassFail, g_iCPU7_PassFail;
static int g_iCPU_PassFail[8];
static int g_iDhryLoopCount;

static unsigned int parameter[8];
static struct device_driver slt_cpu0_dhry_drv = 
{   
	.name = "slt_cpu0_dhry",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu1_dhry_drv = 
{   
	.name = "slt_cpu1_dhry",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu2_dhry_drv = 
{   
	.name = "slt_cpu2_dhry",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu3_dhry_drv = 
{   
	.name = "slt_cpu3_dhry",	
	.bus = &platform_bus_type,  
	.owner = THIS_MODULE,   
};

static struct device_driver slt_cpu4_dhry_drv =
{
	.name = "slt_cpu4_dhry",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu5_dhry_drv =
{
	.name = "slt_cpu5_dhry",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu6_dhry_drv =
{
	.name = "slt_cpu6_dhry",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_cpu7_dhry_drv =
{
	.name = "slt_cpu7_dhry",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

static struct device_driver slt_dhry_loop_count_drv =
{
	.name = "slt_dhry_loop_count",
	.bus = &platform_bus_type,
	.owner = THIS_MODULE,
};

#define DEFINE_SLT_CPU_DHRY_SHOW(_N)	\
static ssize_t slt_cpu##_N##_dhry_show(struct device_driver *driver, char *buf) \
{   \
	if(g_iCPU##_N##_PassFail == -1) \
		return snprintf(buf, PAGE_SIZE, "CPU%d dhry - CPU%d is powered off\n",_N,_N); \
	else	\
		return snprintf(buf, PAGE_SIZE, "CPU%d dhry - %s(loop_count = %d)\n", _N, g_iCPU##_N##_PassFail != g_iDhryLoopCount ? "FAIL" : "PASS", g_iCPU##_N##_PassFail);  \
}

DEFINE_SLT_CPU_DHRY_SHOW(0)
DEFINE_SLT_CPU_DHRY_SHOW(1)
DEFINE_SLT_CPU_DHRY_SHOW(2)
DEFINE_SLT_CPU_DHRY_SHOW(3)
DEFINE_SLT_CPU_DHRY_SHOW(4)
DEFINE_SLT_CPU_DHRY_SHOW(5)
DEFINE_SLT_CPU_DHRY_SHOW(6)
DEFINE_SLT_CPU_DHRY_SHOW(7)

static ssize_t slt_dhry_loop_count_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "Dhry Test Loop Count = %d\n", g_iDhryLoopCount);
}

DEFINE_SPINLOCK(cpu_lock);	
unsigned long cpu_flags[8];	 
#define DEFINE_SLT_CPU_DHRY_STORE(_N)	\
static ssize_t slt_cpu##_N##_dhry_store(struct device_driver *driver, const char *buf, size_t count)	\
{   \
	unsigned int i, ret;	\
	unsigned long mask; \
	int retry=0;	\
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
	printk("\n>> CPU%d dhry test start (cpu id = %d) <<\n\n", _N, raw_smp_processor_id());  \
	\
	for (i = 0, g_iCPU##_N##_PassFail = 0; i < g_iDhryLoopCount; i++) {	\
		mutex_lock(&cpu_stress_mutex);\
		ret = fp1_dhry_start();	/* 1: PASS, 0:Fail, -1: target CPU power off */  \
		mutex_unlock(&cpu_stress_mutex);\
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
	if (g_iCPU##_N##_PassFail == g_iDhryLoopCount) {	\
		printk("\n>> CPU%d dhry test pass <<\n\n", _N); \
	}else { \
		printk("\n>> CPU%d dhry test fail (loop count = %d)<<\n\n", _N, g_iCPU##_N##_PassFail);  \
	}   \
	\
	return count;   \
}

DEFINE_SLT_CPU_DHRY_STORE(0)
DEFINE_SLT_CPU_DHRY_STORE(1)
DEFINE_SLT_CPU_DHRY_STORE(2)
DEFINE_SLT_CPU_DHRY_STORE(3)
DEFINE_SLT_CPU_DHRY_STORE(4)
DEFINE_SLT_CPU_DHRY_STORE(5)
DEFINE_SLT_CPU_DHRY_STORE(6)
DEFINE_SLT_CPU_DHRY_STORE(7)

static ssize_t slt_dhry_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{ 
	int result;
	 
	if ((result = sscanf(buf, "%d", &g_iDhryLoopCount)) == 1)
	{
		printk("set SLT dhry test loop count = %d successfully\n", g_iDhryLoopCount);
	}
	else
	{
		printk("bad argument!!\n");
		return -EINVAL;   
	}
	 
	return count;	
}
	
DRIVER_ATTR(slt_cpu0_dhry, 0644, slt_cpu0_dhry_show, slt_cpu0_dhry_store);
DRIVER_ATTR(slt_cpu1_dhry, 0644, slt_cpu1_dhry_show, slt_cpu1_dhry_store);
DRIVER_ATTR(slt_cpu2_dhry, 0644, slt_cpu2_dhry_show, slt_cpu2_dhry_store);
DRIVER_ATTR(slt_cpu3_dhry, 0644, slt_cpu3_dhry_show, slt_cpu3_dhry_store);
DRIVER_ATTR(slt_cpu4_dhry, 0644, slt_cpu4_dhry_show, slt_cpu4_dhry_store);
DRIVER_ATTR(slt_cpu5_dhry, 0644, slt_cpu5_dhry_show, slt_cpu5_dhry_store);
DRIVER_ATTR(slt_cpu6_dhry, 0644, slt_cpu6_dhry_show, slt_cpu6_dhry_store);
DRIVER_ATTR(slt_cpu7_dhry, 0644, slt_cpu7_dhry_show, slt_cpu7_dhry_store);
DRIVER_ATTR(slt_dhry_loop_count, 0644, slt_dhry_loop_count_show, slt_dhry_loop_count_store);

#define DEFINE_SLT_CPU_DHRY_INIT(_N)	\
int __init slt_cpu##_N##_dhry_init(void) \
{   \
	int ret;	\
	\
	ret = driver_register(&slt_cpu##_N##_dhry_drv);  \
	if (ret) {  \
		printk("fail to create SLT CPU%d dhry driver\n",_N);	\
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d dhry driver\n",_N); \
	}   \
	\
	ret = driver_create_file(&slt_cpu##_N##_dhry_drv, &driver_attr_slt_cpu##_N##_dhry);   \
	if (ret) {  \
		printk("fail to create SLT CPU%d dhry sysfs files\n",_N);   \
	}   \
	else if(print_success)	\
	{   \
		printk("success to create SLT CPU%d dhry sysfs files\n",_N);	\
	}   \
	\
	return 0;   \
}

DEFINE_SLT_CPU_DHRY_INIT(0)
DEFINE_SLT_CPU_DHRY_INIT(1)
DEFINE_SLT_CPU_DHRY_INIT(2)
DEFINE_SLT_CPU_DHRY_INIT(3)
DEFINE_SLT_CPU_DHRY_INIT(4)
DEFINE_SLT_CPU_DHRY_INIT(5)
DEFINE_SLT_CPU_DHRY_INIT(6)
DEFINE_SLT_CPU_DHRY_INIT(7)

extern void wdt_arch_reset(char mode);
int run_dhry(void *arg)
{
	unsigned int i, ret;	
	unsigned long mask; 
	unsigned int cpu_id;

	cpu_id = *((unsigned int*) arg);
	
	g_iCPU_PassFail[cpu_id] = 0;  
	
	mask = (1 << cpu_id); /* processor _N */ 
	printk("\n>> CPU%d dhry test start (cpu id = %d) <<\n\n", cpu_id, raw_smp_processor_id());  
	
	for (i = 0, g_iCPU_PassFail[cpu_id] = 0; i < g_iDhryLoopCount; i++) {	
	mutex_lock(&cpu_stress_mutex);
		ret = fp1_dhry_start();   /* 1: PASS, 0:Fail, -1: target CPU power off */  \
	mutex_unlock(&cpu_stress_mutex);
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
	
	if (g_iCPU_PassFail[cpu_id] == g_iDhryLoopCount) {	
		printk("\n>> CPU%d dhry test pass <<\n\n", cpu_id); 
	}else { 
		printk("\n>> CPU%d dhry test fail (loop count = %d)<<\n\n", cpu_id, g_iCPU_PassFail[cpu_id]);  
	}   
	if (cpu_id == num_online_cpus() - 1)
	wdt_arch_reset(0);
	return 0;
}


int __init slt_dhry_loop_count_init(void)
{
	int ret;
	int cpu; 
	struct task_struct *t_thread[8];



	ret = driver_register(&slt_dhry_loop_count_drv);
	if (ret) {
		printk("fail to create dhry loop count driver\n");
	}
	else if(print_success)
	{
		printk("success to create dhry loop count driver\n");	
	}
	

	ret = driver_create_file(&slt_dhry_loop_count_drv, &driver_attr_slt_dhry_loop_count);
	if (ret) {
		printk("fail to create dhry loop count sysfs files\n");
	}
	else if(print_success)
	{
		printk("success to create dhry loop count sysfs files\n");		
	}

	g_iDhryLoopCount = SLT_LOOP_CNT;

#ifdef SLT_64BIT	
	for_each_present_cpu(cpu)
	{
		printk("start cpu%d\n",cpu);
		parameter[cpu] = cpu;
		kthread_create(run_dhry,(void *) (parameter+cpu) , "thread/%d", cpu);

		t_thread[cpu] = kthread_create(run_dhry,(void *) (parameter+cpu) , "thread/%d", cpu);
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
arch_initcall(slt_cpu0_dhry_init);
arch_initcall(slt_cpu1_dhry_init);
arch_initcall(slt_cpu2_dhry_init);
arch_initcall(slt_cpu3_dhry_init);
arch_initcall(slt_cpu4_dhry_init);
arch_initcall(slt_cpu5_dhry_init);
arch_initcall(slt_cpu6_dhry_init);
arch_initcall(slt_cpu7_dhry_init);
arch_initcall(slt_dhry_loop_count_init);
