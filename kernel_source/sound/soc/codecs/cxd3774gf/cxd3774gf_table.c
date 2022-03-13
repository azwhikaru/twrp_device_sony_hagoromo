/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3774gf_table.c
 *
 * CXD3774GF CODEC driver
 *
 * Copyright (c) 2013,2014 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* #define TRACE_PRINT_ON */
/* #define DEBUG_PRINT_ON */
#define TRACE_TAG "------- "
#define DEBUG_TAG "        "

#include "cxd3774gf_common.h"

#define TABLE_SIZE_OUTPUT_VOLUME (sizeof(struct cxd3774gf_master_volume)*(MASTER_VOLUME_MAX+1)*(MASTER_VOLUME_TABLE_MAX+1)*2)
#define TABLE_SIZE_DEVICE_GAIN   (sizeof(struct cxd3774gf_device_gain)*(INPUT_DEVICE_MAX+1))
#define TABLE_SIZE_TONE_CONTROL  (sizeof(struct cxd3774gf_tone_control)*(TONE_CONTROL_TABLE_MAX+1))

struct port_info {
	char                  * name;
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry * entry;
#else
	void *                  entry;
#endif
	int                     node;
	int                     size;
	unsigned char *         table;
	int                     columns; /* for debug */
	int                     rows;    /* for debug */
	int                     width;   /* for debug */
};

#ifdef CONFIG_PROC_FS

static ssize_t write_table(
	struct file       * file,
	const char __user * buf,
	size_t              size,
	loff_t            * pos
);

static ssize_t read_table(
	struct file * file,
	char __user * buf,
	size_t        size,
	loff_t      * pos
);

static int check_data(unsigned char * buf, int size);
static void dump_data(void * buf, int size, int columns, int rows, int width);

#endif

/* muting all */
struct cxd3774gf_master_volume cxd3774gf_master_volume_table
								[2]
								[MASTER_VOLUME_TABLE_MAX+1]
								[MASTER_VOLUME_MAX+1]
									= {{{{0x33,0x33,0x33,0x33,0xC0,0xC0,0x00,0x3F,0x00,0x00}}}};

unsigned char cxd3774gf_master_gain_table[MASTER_GAIN_MAX+1] =
{
	/* 00 */ 0xC0,
	/* 01 */ 0xC5,
	/* 02 */ 0xC8,
	/* 03 */ 0xCC,
	/* 04 */ 0xD0,
	/* 05 */ 0xD3,
	/* 06 */ 0xD6,
	/* 07 */ 0xD9,
	/* 08 */ 0xDC,
	/* 09 */ 0xDF,
	/* 10 */ 0xE2,
	/* 11 */ 0xE4,
	/* 12 */ 0xE6,
	/* 13 */ 0xE8,
	/* 14 */ 0xEA,
	/* 15 */ 0xEC,
	/* 16 */ 0xEE,
	/* 17 */ 0xF0,
	/* 18 */ 0xF2,
	/* 19 */ 0xF3,
	/* 20 */ 0xF4,
	/* 21 */ 0xF5,
	/* 22 */ 0xF6,
	/* 23 */ 0xF7,
	/* 24 */ 0xF8,
	/* 25 */ 0xF9,
	/* 26 */ 0xFA,
	/* 27 */ 0xFB,
	/* 28 */ 0xFC,
	/* 29 */ 0xFE,
	/* 30 */ 0x00,
};

struct cxd3774gf_device_gain cxd3774gf_device_gain_table[INPUT_DEVICE_MAX+1] =
{
/*    PGA1  ADC1                 */
	{ 0x00, 0x00 }, /* OFF       */
	{ 0x06, 0x00 }, /* TUNER     */
	{ 0x00, 0x00 }, /* MIC       */
	{ 0xF8, 0x00 }, /* LINE      */
	{ 0x00, 0x00 }, /* DIRECTMIC */
};

struct cxd3774gf_tone_control cxd3774gf_tone_control_table[TONE_CONTROL_TABLE_MAX+1] =
{
	{{
		/* 0: non HP                                                                                                   */
		/*           a0                  a1                  a2                  b1                  b2                */
		/* DEQ 1 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 2 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 3 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 4 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 5 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
	}},
	{{
		/* 1: class-H : general HP                                                                                     */
		/*           a0                  a1                  a2                  b1                  b2                */
		/* DEQ 1 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 2 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 3 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 4 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 5 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
	}},
	{{
		/* 2: class-H : bundle NCHP                                                                                    */
		/*           a0                  a1                  a2                  b1                  b2                */
		/* DEQ 1 */ {{0x1F, 0xF5, 0x32}, {0xE0, 0x2A, 0xA3}, {0x00, 0x00, 0x00}, {0x1F, 0xCA, 0x8F}, {0x00, 0x00, 0x00}},
		/* DEQ 2 */ {{0x1B, 0xF0, 0xF9}, {0xF0, 0xE0, 0xEF}, {0x00, 0x00, 0x00}, {0x13, 0x2E, 0x19}, {0x00, 0x00, 0x00}},
		/* DEQ 3 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 4 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 5 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
	}},
	{{
		/* 3: class-H : model 1 NCHP                                                                                   */
		/*           a0                  a1                  a2                  b1                  b2                */
		/* DEQ 1 */ {{0x2F, 0xBE, 0x12}, {0xA5, 0x4B, 0x18}, {0x2A, 0xF7, 0x6E}, {0x39, 0x3A, 0x2F}, {0xE6, 0xC3, 0x01}},
		/* DEQ 2 */ {{0x2C, 0xF2, 0x4C}, {0xA6, 0x9F, 0x68}, {0x2C, 0x6F, 0x42}, {0x3F, 0x46, 0x38}, {0xE0, 0xB9, 0x1A}},
		/* DEQ 3 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 4 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 5 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
	}},
	{{
		/* 4: S-master : general HP                                                                                    */
		/*           a0                  a1                  a2                  b1                  b2                */
		/* DEQ 1 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 2 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 3 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 4 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 5 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
	}},
	{{
		/* 5: S-master : bundle NCHP                                                                                   */
		/*           a0                  a1                  a2                  b1                  b2                */
		/* DEQ 1 */ {{0x1F, 0xF3, 0xAD}, {0xE0, 0x22, 0x75}, {0x00, 0x00, 0x00}, {0x1F, 0xD1, 0x38}, {0x00, 0x00, 0x00}},
		/* DEQ 2 */ {{0x1F, 0xFF, 0x54}, {0xE0, 0x04, 0xB1}, {0x00, 0x00, 0x00}, {0x1F, 0xFA, 0xA4}, {0x00, 0x00, 0x00}},
		/* DEQ 3 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 4 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 5 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
	}},
	{{
		/* 6: S-master : model 1 NCHP                                                                                  */
		/*           a0                  a1                  a2                  b1                  b2                */
		/* DEQ 1 */ {{0x2F, 0xBE, 0x12}, {0xA5, 0x4B, 0x18}, {0x2A, 0xF7, 0x6E}, {0x39, 0x3A, 0x2F}, {0xE6, 0xC3, 0x01}},
		/* DEQ 2 */ {{0x2C, 0xF2, 0x4C}, {0xA6, 0x9F, 0x68}, {0x2C, 0x6F, 0x42}, {0x3F, 0x46, 0x38}, {0xE0, 0xB9, 0x1A}},
		/* DEQ 3 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 4 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
		/* DEQ 5 */ {{0x20, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}},
	}},
};

static struct port_info port_info_table[] =
{
	{ "ovt", NULL, -1, TABLE_SIZE_OUTPUT_VOLUME, (unsigned char *)cxd3774gf_master_volume_table, 10, 31, 1 },
	{ "dgt", NULL, -1, TABLE_SIZE_DEVICE_GAIN,   (unsigned char *)cxd3774gf_device_gain_table,    2,  5, 1 },
	{ "tct", NULL, -1, TABLE_SIZE_TONE_CONTROL,  (unsigned char *)cxd3774gf_tone_control_table,  15,  5, 1 },
	{ NULL,  NULL, -1, 0,                                         NULL,                           0,  0, 0 }
};

static struct file_operations port_operations = {
	.write= write_table,
	.read = read_table,
};

static int initialized = FALSE;
static struct mutex * global_mutex = NULL;
static struct proc_dir_entry * port_directory = NULL;
static unsigned char * work_buf = NULL;

int cxd3774gf_table_initialize(struct mutex * mutex)
{
#ifdef CONFIG_PROC_FS

	struct proc_dir_entry * entry;
	int n;

#endif

	print_trace("%s()\n",__FUNCTION__);

	global_mutex=mutex;

#ifdef CONFIG_PROC_FS

	port_directory=proc_mkdir("icx_audio_data",NULL);
	if(port_directory==NULL){
		print_fail("proc_mkdir(): error occurred.\n");
		back_trace();
		return(-1);
	}

	n=0;

	while(1){

		if(port_info_table[n].table==NULL)
			break;

		entry=proc_create_data(port_info_table[n].name,0600,port_directory,&port_operations,NULL);
		if(entry==NULL){
			print_fail("create_proc_entry(): error occurred.\n");
			back_trace();
			return(-ENOMEM);
		}

//		entry->proc_fops = &port_operations;
		port_info_table[n].entry = entry;
		port_info_table[n].node  = entry->low_ino;

		n++;
	}

#endif

	initialized=TRUE;

	return(0);
}

int cxd3774gf_table_finalize(void)
{
#ifdef CONFIG_PROC_FS
	int n;
#endif

	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

#ifdef CONFIG_PROC_FS

	n=0;

	while(1){

		if(port_info_table[n].table==NULL)
			break;

		remove_proc_entry(port_info_table[n].name,port_directory);

		n++;
	}

	remove_proc_entry("icx_audio_data",NULL);

#endif

	global_mutex=NULL;

	initialized=FALSE;

	return(0);
}

#ifdef CONFIG_PROC_FS

static ssize_t write_table(
	struct file       * file,
	const char __user * buf,
	size_t              size,
	loff_t            * pos
)
{
	int index;
	int count;
	int rv;

	index=0;

	while(1){

		if(port_info_table[index].table==NULL){
			print_error("invalid node %d\n",(int)file->f_dentry->d_inode->i_ino);
			return(-ENODEV);
		}

		if(port_info_table[index].node==file->f_dentry->d_inode->i_ino)
			break;

		index++;
	}

	if(*pos==0){
		mutex_lock(global_mutex);
		print_info("target table = %s\n",port_info_table[index].name);
		work_buf=(unsigned char *)kmalloc(port_info_table[index].size+8,GFP_KERNEL);
		if(work_buf==NULL){
			print_fail("kmalloc(): no memory.\n");
			back_trace();
			mutex_unlock(global_mutex);
			return(-ENOMEM);
		}
	}

	/* count=minimum(512,size); */
	/* count=minimum(count,port_info_table[index].size+8-*pos); */
	count=minimum(size,port_info_table[index].size+8-*pos);

	if(count<=0)
		return(0);

	print_debug("index = %d\n",index);
	print_debug("pos = %d\n",(int)*pos);
	print_debug("count = %d\n",count);

	rv=copy_from_user(
		(void *)(work_buf+*pos),
		(void *)buf,
		count
	);
	if(rv!=0){
		print_fail("copy_from_user(): code %d error occurred.\n",rv);
		back_trace();
		kfree(work_buf);
		mutex_unlock(global_mutex);
		return(-EIO);
	}

	*pos=*pos+count;

	if(*pos>=port_info_table[index].size+8){

		rv=check_data(work_buf,port_info_table[index].size);
		if(rv<0){
			back_trace();
			kfree(work_buf);
			mutex_unlock(global_mutex);
			return(-EINVAL);
		}

		memcpy(
			(void *)port_info_table[index].table,
			work_buf,
			port_info_table[index].size
		);

		/* dump_data( */
			/* (unsigned char *)port_info_table[index].table, */
			/* port_info_table[index].size, */
			/* port_info_table[index].columns, */
			/* port_info_table[index].rows, */
			/* port_info_table[index].width */
		/* ); */

		cxd3774gf_apply_table_change(index);

		kfree(work_buf);
		print_info("done\n");
		mutex_unlock(global_mutex);
	}

	return(count);
}

static ssize_t read_table(
	struct file * file,
	char __user * buf,
	size_t        size,
	loff_t      * pos
)
{
	int index;
	int count;
	int rv;

	index=0;

	while(1){

		if(port_info_table[index].table==NULL){
			print_error("invalid node %d\n",(int)file->f_dentry->d_inode->i_ino);
			return(-ENODEV);
		}

		if(port_info_table[index].node==file->f_dentry->d_inode->i_ino)
			break;

		index++;
	}

	if(*pos==0){
		mutex_lock(global_mutex);
		print_info("target table = %s\n",port_info_table[index].name);
	}

	/* count=minimum(512,size); */
	/* count=minimum(count,port_info_table[index].size-*pos); */
	count=minimum(size,port_info_table[index].size-*pos);

	if(count<=0)
		return(0);

	print_debug("index = %d\n",index);
	print_debug("pos = %d\n",(int)*pos);
	print_debug("count = %d\n",count);

	rv=copy_to_user(
		(void *)buf,
		(void *)((unsigned char *)port_info_table[index].table+*pos),
		count
	);
	if(rv!=0){
		print_fail("copy_to_user(): code %d error occurred.\n",rv);
		back_trace();
		mutex_unlock(global_mutex);
		return(-EIO);
	}

	*pos=*pos+count;

	if(*pos>=port_info_table[index].size){
		dump_data(
			(unsigned char *)port_info_table[index].table,
			port_info_table[index].size,
			port_info_table[index].columns,
			port_info_table[index].rows,
			port_info_table[index].width
		);
		print_info("done\n");
		mutex_unlock(global_mutex);
	}

	return(count);
}

static int check_data(unsigned char * buf, int size)
{
	unsigned int sum=0;
	unsigned int xor=0;
	int n;

	for(n=0;n<size;n++){
		sum=sum+buf[n];
		xor=xor^(buf[n]<<(n%4)*8);
	}

	print_info("sum = 0x%08X, xor = 0x%08X\n",sum,xor);

	if(sum==0){
		print_error("all zero\n");
		return(-1);
	}

	if(sum!=*(unsigned int *)(buf+size)){
		print_error("sum error %08X,%08X\n",sum,*(unsigned int *)(buf+size));
		return(-1);
	}

	if(xor!=*(unsigned int *)(buf+size+4)){
		print_error("xor error %08X,%08X\n",xor,*(unsigned int *)(buf+size+4));
		return(-1);
	}

	return(0);
}

static void dump_data(void * buf, int size, int columns, int rows, int width)
{
	unsigned char * uc;
	unsigned int  * ui;
	int n;
	int m;

	printk(KERN_INFO "target = %08X, size = %d\n",(unsigned int)buf,size);

	if(width==1){

		uc=buf;

		for(n=0; n<size; n=n+columns){

			printk(KERN_INFO "%02d:",(n/columns)%rows);

			for(m=0; m<columns; m++){

				if(n+m>=size)
					break;

				printk(" %02X",uc[n+m]);

			}

			printk("\n");
		}
	}

	if(width==4){

		ui=buf;
		size=size/width;

		for(n=0; n<size; n=n+columns){

			printk(KERN_INFO "%02d:",(n/columns)%rows);

			for(m=0; m<columns; m++){

				if(n+m>=size)
					break;

				printk(" %08X",ui[n+m]);

			}

			printk("\n");
		}
	}

	return;
}

#endif

