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

#include <linux/of.h>
#include <linux/of_address.h>

#ifdef CONFIG_ICX_SILENT_LOG
static const print_success = 0;
#else
static const print_success = 1;
#endif

extern int cache_miss_start(int cpu_id);

static int g_iCPU0_PassFail, g_iCPU1_PassFail, g_iCPU2_PassFail, g_iCPU3_PassFail, g_iCPU4_PassFail, g_iCPU5_PassFail, g_iCPU6_PassFail, g_iCPU7_PassFail;
static int g_iCache_missLoopCount;
static int g_iCPU_PassFail[8];
static unsigned int parameter[8];
static void __iomem *result_base;
static struct device_driver slt_cpu0_cache_miss_drv = 
{   
    .name = "slt_cpu0_cache_miss",    
    .bus = &platform_bus_type,  
    .owner = THIS_MODULE,   
};

static struct device_driver slt_cpu1_cache_miss_drv = 
{   
    .name = "slt_cpu1_cache_miss",    
    .bus = &platform_bus_type,  
    .owner = THIS_MODULE,   
};

static struct device_driver slt_cpu2_cache_miss_drv = 
{   
    .name = "slt_cpu2_cache_miss",    
    .bus = &platform_bus_type,  
    .owner = THIS_MODULE,   
};

static struct device_driver slt_cpu3_cache_miss_drv = 
{   
    .name = "slt_cpu3_cache_miss",    
    .bus = &platform_bus_type,  
    .owner = THIS_MODULE,   
};

static struct device_driver slt_cpu4_cache_miss_drv =
{
    .name = "slt_cpu4_cache_miss",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver slt_cpu5_cache_miss_drv =
{
    .name = "slt_cpu5_cache_miss",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver slt_cpu6_cache_miss_drv =
{
    .name = "slt_cpu6_cache_miss",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver slt_cpu7_cache_miss_drv =
{
    .name = "slt_cpu7_cache_miss",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver slt_cache_miss_loop_count_drv =
{
    .name = "slt_cache_miss_loop_count",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

#define DEFINE_SLT_CPU_CACHE_MISS_SHOW(_N)    \
static ssize_t slt_cpu##_N##_cache_miss_show(struct device_driver *driver, char *buf) \
{   \
    if(g_iCPU##_N##_PassFail == -1) \
        return snprintf(buf, PAGE_SIZE, "CPU%d cache_miss - CPU%d is powered off\n",_N,_N); \
    else    \
        return snprintf(buf, PAGE_SIZE, "CPU%d cache_miss - %s(loop_count = %d)\n", _N, g_iCPU##_N##_PassFail != g_iCache_missLoopCount ? "FAIL" : "PASS", g_iCPU##_N##_PassFail);  \
}

DEFINE_SLT_CPU_CACHE_MISS_SHOW(0)
DEFINE_SLT_CPU_CACHE_MISS_SHOW(1)
DEFINE_SLT_CPU_CACHE_MISS_SHOW(2)
DEFINE_SLT_CPU_CACHE_MISS_SHOW(3)
DEFINE_SLT_CPU_CACHE_MISS_SHOW(4)
DEFINE_SLT_CPU_CACHE_MISS_SHOW(5)
DEFINE_SLT_CPU_CACHE_MISS_SHOW(6)
DEFINE_SLT_CPU_CACHE_MISS_SHOW(7)

static ssize_t slt_cache_miss_loop_count_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "CACHE_MISS Test Loop Count = %d\n", g_iCache_missLoopCount);
}

#define DEFINE_SLT_CPU_CACHE_MISS_STORE(_N)    \
    DEFINE_SPINLOCK(cache_miss_cpu##_N##_lock);    \
static ssize_t slt_cpu##_N##_cache_miss_store(struct device_driver *driver, const char *buf, size_t count)    \
{   \
    unsigned int i, ret;    \
    unsigned long mask; \
    int retry=0;    \
    unsigned long cpu##_N##_flags;     \
    \
    g_iCPU##_N##_PassFail = 0;  \
    \
    mask = (1 << _N); /* processor _N */ \
    while(sched_setaffinity(0, (struct cpumask*) &mask) < 0)    \
    {   \
        printk("Could not set cpu%d affinity for current process(%d).\n", _N, retry);  \
        g_iCPU##_N##_PassFail = -1; \
        retry++;    \
        if(retry > 100) \
        {   \
            return count;   \
        }   \
    }   \
    \
    printk("\n>> CPU%d cache_miss test start (cpu id = %d) <<\n\n", _N, raw_smp_processor_id());  \
    \
    for (i = 0, g_iCPU##_N##_PassFail = 0; i < g_iCache_missLoopCount; i++) {    \
        spin_lock_irqsave(&cache_miss_cpu##_N##_lock, cpu##_N##_flags);    \
        ret = cache_miss_start(_N);    /* 1: PASS, 0:Fail, -1: target CPU power off */  \
        spin_unlock_irqrestore(&cache_miss_cpu##_N##_lock, cpu##_N##_flags);   \
        if(ret != -1)   \
        {   \
             g_iCPU##_N##_PassFail += ret;  \
        }   \
        else    \
        {   \
             g_iCPU##_N##_PassFail = -1;    \
             break; \
        }   \
    }   \
    \
    if (g_iCPU##_N##_PassFail == g_iCache_missLoopCount) {    \
        printk("\n>> CPU%d cache_miss test pass <<\n\n", _N); \
    }else { \
        printk("\n>> CPU%d cache_miss test fail (loop count = %d)<<\n\n", _N, g_iCPU##_N##_PassFail);  \
    }   \
    \
    return count;   \
}
int run_cache_miss(void *arg)
{
    unsigned int i, ret;    
    unsigned long mask; 
    unsigned int cpu_id;
    cpu_id = *((unsigned int*) arg);
    
    g_iCPU_PassFail[cpu_id] = 0;  
    
    mask = (1 << cpu_id); /* processor _N */ 
   
    printk("\n>> CPU%d cache miss test start (cpu id = %d) <<\n\n", cpu_id, raw_smp_processor_id());  
    
    for (i = 0, g_iCPU_PassFail[cpu_id] = 0; i < g_iCache_missLoopCount; i++) {    
        //spin_lock_irqsave(&saxpy_cpu##_N##_lock, cpu##_N##_flags);    
        ret = cache_miss_start(cpu_id);   /* 1: PASS, 0:Fail, -1: target CPU power off */
        //spin_unlock_irqrestore(&saxpy_cpu##_N##_lock, cpu##_N##_flags);   
        if(ret >= 0)   
        {   
             g_iCPU_PassFail[cpu_id] += 1;  
        }   
        else    
        {   
             g_iCPU_PassFail[cpu_id] = -1;    
             break; 
        }   
    }   
    
    if (g_iCPU_PassFail[cpu_id] == g_iCache_missLoopCount) {    
	*(unsigned long*)(result_base+SLT_CACHE_MISS*sizeof(unsigned long*)) |= (1 << cpu_id);
        printk("\n>> CPU%d cache_miss test pass(%ld) <<\n\n", cpu_id,*(unsigned long*)(result_base+SLT_CACHE_MISS*sizeof(unsigned long*))); 
    }else { 
        printk("\n>> CPU%d cache_miss test fail (loop count = %d)<<\n\n", cpu_id, g_iCPU_PassFail[cpu_id]);  
    }   
    return 0;
}
DEFINE_SLT_CPU_CACHE_MISS_STORE(0)
DEFINE_SLT_CPU_CACHE_MISS_STORE(1)
DEFINE_SLT_CPU_CACHE_MISS_STORE(2)
DEFINE_SLT_CPU_CACHE_MISS_STORE(3)
DEFINE_SLT_CPU_CACHE_MISS_STORE(4)
DEFINE_SLT_CPU_CACHE_MISS_STORE(5)
DEFINE_SLT_CPU_CACHE_MISS_STORE(6)
DEFINE_SLT_CPU_CACHE_MISS_STORE(7)

static ssize_t slt_cache_miss_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{ 
    int result;
     
    if ((result = sscanf(buf, "%d", &g_iCache_missLoopCount)) == 1)
    {
        printk("set SLT cache_miss test loop count = %d successfully\n", g_iCache_missLoopCount);
    }
    else
    {
        printk("bad argument!!\n");
        return -EINVAL;
    }
     
    return count;
}
    
DRIVER_ATTR(slt_cpu0_cache_miss, 0644, slt_cpu0_cache_miss_show, slt_cpu0_cache_miss_store);
DRIVER_ATTR(slt_cpu1_cache_miss, 0644, slt_cpu1_cache_miss_show, slt_cpu1_cache_miss_store);
DRIVER_ATTR(slt_cpu2_cache_miss, 0644, slt_cpu2_cache_miss_show, slt_cpu2_cache_miss_store);
DRIVER_ATTR(slt_cpu3_cache_miss, 0644, slt_cpu3_cache_miss_show, slt_cpu3_cache_miss_store);
DRIVER_ATTR(slt_cpu4_cache_miss, 0644, slt_cpu4_cache_miss_show, slt_cpu4_cache_miss_store);
DRIVER_ATTR(slt_cpu5_cache_miss, 0644, slt_cpu5_cache_miss_show, slt_cpu5_cache_miss_store);
DRIVER_ATTR(slt_cpu6_cache_miss, 0644, slt_cpu6_cache_miss_show, slt_cpu6_cache_miss_store);
DRIVER_ATTR(slt_cpu7_cache_miss, 0644, slt_cpu7_cache_miss_show, slt_cpu7_cache_miss_store);
DRIVER_ATTR(slt_cache_miss_loop_count, 0644, slt_cache_miss_loop_count_show, slt_cache_miss_loop_count_store);

#define DEFINE_SLT_CPU_CACHE_MISS_INIT(_N)    \
int __init slt_cpu##_N##_cache_miss_init(void) \
{   \
    int ret;    \
    \
    ret = driver_register(&slt_cpu##_N##_cache_miss_drv);  \
    if (ret) {  \
        printk("fail to create SLT CPU%d cache_miss driver\n",_N);    \
    }   \
    else if(print_success) \
    {   \
        printk("success to create SLT CPU%d cache_miss driver\n",_N); \
    }   \
    \
    ret = driver_create_file(&slt_cpu##_N##_cache_miss_drv, &driver_attr_slt_cpu##_N##_cache_miss);   \
    if (ret) {  \
        printk("fail to create SLT CPU%d cache_miss sysfs files\n",_N);   \
    }   \
    else if(print_success) \
    {   \
        printk("success to create SLT CPU%d cache_miss sysfs files\n",_N);    \
    }   \
    \
    return 0;   \
}

DEFINE_SLT_CPU_CACHE_MISS_INIT(0)
DEFINE_SLT_CPU_CACHE_MISS_INIT(1)
DEFINE_SLT_CPU_CACHE_MISS_INIT(2)
DEFINE_SLT_CPU_CACHE_MISS_INIT(3)
DEFINE_SLT_CPU_CACHE_MISS_INIT(4)
DEFINE_SLT_CPU_CACHE_MISS_INIT(5)
DEFINE_SLT_CPU_CACHE_MISS_INIT(6)
DEFINE_SLT_CPU_CACHE_MISS_INIT(7)

int __init slt_cache_miss_loop_count_init(void)
{
    int ret;
    int cpu; 
    struct task_struct *t_thread[8];
    struct device_node *node;


    /* DTS version */
    node = of_find_compatible_node(NULL, NULL, "mediatek,INTERSRAM");
    if(node) {
        result_base = of_iomap(node, 0);
        printk("get SLT result_base@ %p\n", result_base);
    }
    else {
        printk("can't find compatible node\n");
        return -1;                                                                                                           }          
    ret = driver_register(&slt_cache_miss_loop_count_drv);
    if (ret) {
        printk("fail to create cache_miss loop count driver\n");
    }
    else if(print_success)
    {
        printk("success to create cache_miss loop count driver\n");
    }
    

    ret = driver_create_file(&slt_cache_miss_loop_count_drv, &driver_attr_slt_cache_miss_loop_count);
    if (ret) {
        printk("fail to create cache_miss loop count sysfs files\n");
    }
    else if(print_success)
    {
        printk("success to create cache_miss loop count sysfs files\n");
    }

    g_iCache_missLoopCount = SLT_LOOP_CNT;
    
    for_each_present_cpu(cpu)
    {
        printk("start cpu%d\n",cpu);                                                                                                                      parameter[cpu] = cpu;
        t_thread[cpu] = kthread_create(run_cache_miss,(void *) (parameter+cpu) , "thread/%d", cpu);
        if (IS_ERR(t_thread[cpu])) {
            printk(KERN_ERR "[thread/%d]: creating kthread failed\n", cpu);
            return -1;
        }
        kthread_bind(t_thread[cpu], cpu);
        wake_up_process(t_thread[cpu]);
    }

    return 0;
}
arch_initcall(slt_cpu0_cache_miss_init);
arch_initcall(slt_cpu1_cache_miss_init);
arch_initcall(slt_cpu2_cache_miss_init);
arch_initcall(slt_cpu3_cache_miss_init);
arch_initcall(slt_cpu4_cache_miss_init);
arch_initcall(slt_cpu5_cache_miss_init);
arch_initcall(slt_cpu6_cache_miss_init);
arch_initcall(slt_cpu7_cache_miss_init);
arch_initcall(slt_cache_miss_loop_count_init);
