/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * cxd3778gf_register.c
 *
 * CXD3778GF CODEC driver
 *
 * Copyright (c) 2013-2016 Sony Corporation
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

#include <linux/dma-mapping.h>
#include "cxd3778gf_common.h"

static int cxd3778gf_register_write_safe(
	unsigned int    address,
	unsigned char * value,
	int             size
);

static int cxd3778gf_register_read_safe(
	unsigned int    address,
	unsigned char * value,
	int             size
);

static int cxd3778gf_register_write_core(
	unsigned int    address,
	unsigned char * value,
	int             size
);

static int cxd3778gf_register_read_core(
	unsigned int    address,
	unsigned char * value,
	int             size
);

static int initialized = FALSE;
static struct i2c_client * i2c_client = NULL;
static struct mutex access_mutex;

int cxd3778gf_register_initialize(struct i2c_client * client)
{
	print_trace("%s()\n",__FUNCTION__);

	mutex_init(&access_mutex);

	i2c_client=client;

	initialized=TRUE;

	return(0);
}

int cxd3778gf_register_finalize(void)
{
	print_trace("%s()\n",__FUNCTION__);

	if(!initialized)
		return(0);

	initialized=FALSE;

	return(0);
}

int cxd3778gf_register_write_multiple(
	unsigned int    address,
	unsigned char * value,
	int             size
)
{
	int rv;

	print_trace("%s(0x%02X,0x%02X...,%d)\n",__FUNCTION__,address,value[0],size);

	mutex_lock(&access_mutex);

	rv=cxd3778gf_register_write_safe(address,value,size);
	if(rv<0) {
		back_trace();
		mutex_unlock(&access_mutex);
		return(rv);
	}

#ifdef DEBUG_PRINT_ON
	{
		int n;

		for(n=0;n<size;n++){

			if(n==0)
				printk(KERN_INFO DEBUG_TAG "W %02X :",address);
			else if((n&0xF)==0)
				printk(KERN_INFO DEBUG_TAG "     :");

			printk(" %02X",value[n]);

			if((n&0xF)==0xF)
				printk("\n");
		}

		if((n&0xF)!=0x0)
			printk("\n");
	}
#endif

	mutex_unlock(&access_mutex);

	return(0);
}

int cxd3778gf_register_read_multiple(
	unsigned int    address,
	unsigned char * value,
	int             size
)
{
	int rv;

	print_trace("%s(0x%02X,%d)\n",__FUNCTION__,address,size);

	mutex_lock(&access_mutex);

	rv=cxd3778gf_register_read_safe(address,value,size);
	if(rv<0) {
		back_trace();
		mutex_unlock(&access_mutex);
		return(rv);
	}

#ifdef DEBUG_PRINT_ON
	{
		int n;

		for(n=0;n<size;n++){

			if(n==0)
				printk(KERN_INFO DEBUG_TAG "R %02X :",address);
			else if((n&0xF)==0)
				printk(KERN_INFO DEBUG_TAG "     :");

			printk(" %02X",value[n]);

			if((n&0xF)==0xF)
				printk("\n");
		}

		if((n&0xF)!=0x0)
			printk("\n");
	}
#endif

	mutex_unlock(&access_mutex);

	return(0);
}

int cxd3778gf_register_modify(
	unsigned int address,
	unsigned int value,
	unsigned int mask
)
{
	unsigned int old;
	unsigned int now;
	unsigned char uc;
	int rv;

	print_trace("%s(0x%02X,0x%02X,0x%02X)\n",__FUNCTION__,address,value,mask);

	mutex_lock(&access_mutex);

	rv=cxd3778gf_register_read_safe(address,&uc,1);
	if(rv<0) {
		back_trace();
		mutex_unlock(&access_mutex);
		return(rv);
	}

	old=uc;

	now=(old&~mask)|(value&mask);

	uc=now;

	rv=cxd3778gf_register_write_safe(address,&uc,1);
	if(rv<0) {
		back_trace();
		mutex_unlock(&access_mutex);
		return(rv);
	}

	print_debug("M %02X : %02X -> %02X\n",address,old,now);

	mutex_unlock(&access_mutex);

	return(0);
}

int cxd3778gf_register_write(
	unsigned int address,
	unsigned int value
)
{
	unsigned char uc;
	int rv;

	print_trace("%s(0x%02X,0x%02X)\n",__FUNCTION__,address,value);

	mutex_lock(&access_mutex);

	uc=value;

	rv=cxd3778gf_register_write_safe(address,&uc,1);
	if(rv<0) {
		back_trace();
		mutex_unlock(&access_mutex);
		return(rv);
	}

	print_debug("W %02X : %02X\n",address,value);

	mutex_unlock(&access_mutex);

	return(0);
}

int cxd3778gf_register_read(
	unsigned int   address,
	unsigned int * value
)
{
	unsigned char uc;
	int rv;

	print_trace("%s(0x%02X)\n",__FUNCTION__,address);

	mutex_lock(&access_mutex);

	rv=cxd3778gf_register_read_safe(address,&uc,1);
	if(rv<0) {
		back_trace();
		mutex_unlock(&access_mutex);
		return(rv);
	}

	*value=uc;

	print_debug("R %02X : %02X\n",address,*value);

	mutex_unlock(&access_mutex);

	return(0);
}

static int cxd3778gf_register_write_safe(
	unsigned int    address,
	unsigned char * value,
	int             size
)
{
	int retry;
	int rv;

	retry=REGISTER_ACCESS_RETRY_COUNT;

	while(retry>=0){
		rv=cxd3778gf_register_write_core(address,value,size);
		if(rv==0)
			break;

		retry--;
	}

	return(rv);
}

static int cxd3778gf_register_read_safe(
	unsigned int    address,
	unsigned char * value,
	int             size
)
{
	int retry;
	int rv;

	retry=REGISTER_ACCESS_RETRY_COUNT;

	while(retry>=0){
		rv=cxd3778gf_register_read_core(address,value,size);
		if(rv==0)
			break;

		retry--;
	}

	return(rv);
}

static int cxd3778gf_register_write_core(
	unsigned int    address,
	unsigned char * value,
	int             size
)
{
	struct i2c_msg msg;
	int rv;

#ifdef CONFIG_MTK_I2C
	u8 *buffer =NULL;
	u32 phyAddr = 0;
#else
	unsigned char buf[512];
#endif

	if(i2c_client==NULL){
		print_error("not initialized.\n");
		return(-ENODEV);
	}

	if(size>320){
		print_error("invalid size.\n");
		return(-EINVAL);
	}

	msg.addr  = i2c_client->addr;
	msg.flags = 0;
	msg.len   = size+1;
#ifdef CONFIG_MTK_I2C
	buffer = dma_alloc_coherent(0, size, &phyAddr, GFP_KERNEL);
	buffer[0]=(unsigned char)address;
	memcpy(buffer+1,value,size);

	msg.buf   = (u8*)phyAddr;
	msg.ext_flag = I2C_DMA_FLAG;
	msg.timing = 400;
#else
	buf[0]=(unsigned char)address;
	memcpy(buf+1,value,size);
	msg.buf   = buf;
#endif
	rv=i2c_transfer(i2c_client->adapter,&msg,1);
	if(rv<0) {
		print_fail("i2c_transfer(): code %d error occurred.\n",rv);
		back_trace();
#ifdef CONFIG_MTK_I2C
		dma_free_coherent(0, size, buffer, phyAddr);
#endif
		return(rv);
	}

	if(rv!=1){
		print_error("count mismacth.\n");
#ifdef CONFIG_MTK_I2C
		dma_free_coherent(0, size, buffer, phyAddr);
#endif
		return(-EIO);
	}

#ifdef CONFIG_MTK_I2C
	dma_free_coherent(0, size, buffer, phyAddr);
#endif
	return(0);
}

static int cxd3778gf_register_read_core(
	unsigned int    address,
	unsigned char * value,
	int             size
)
{
	struct i2c_msg msg[2];
	unsigned char addr;
	int rv;

#ifdef CONFIG_MTK_I2C
	u32 phyAddr = 0;
	u8 *buffer =NULL;
#endif

	if(i2c_client==NULL){
		print_error("not initialized.\n");
		return(-ENODEV);
	}

	if(size>320){
		print_error("invalid size.\n");
		return(-EINVAL);
	}

	addr=(unsigned char)address;

	msg[0].addr  = i2c_client->addr;
	msg[0].flags = 0;
	msg[0].len   = 1;
	msg[0].buf   = &addr;
#ifdef CONFIG_MTK_I2C
	msg[0].ext_flag   = 0;
	msg[0].timing = 400;
#endif
	msg[1].addr  = i2c_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = size;
#ifdef CONFIG_MTK_I2C
	buffer = dma_alloc_coherent(0, size, &phyAddr, GFP_KERNEL);
	msg[1].buf   = (u8 *)phyAddr;
	msg[1].ext_flag = I2C_DMA_FLAG;
	msg[1].timing = 400;
#else
	msg[1].buf   = value;
#endif
	rv=i2c_transfer(i2c_client->adapter,msg,2);
	if(rv<0) {
		print_fail("i2c_transfer(): code %d error occurred.\n",rv);
		back_trace();
#ifdef CONFIG_MTK_I2C
		dma_free_coherent(0, size, buffer, phyAddr);
#endif
		return(rv);
	}

	if(rv!=2){
		print_error("count mismacth.\n");
#ifdef CONFIG_MTK_I2C
		dma_free_coherent(0, size, buffer, phyAddr);
#endif
		return(-EIO);
	}

#ifdef CONFIG_MTK_I2C
	memcpy(value, buffer, size);
	dma_free_coherent(0, size, buffer, phyAddr);
#endif
	return(0);
}

