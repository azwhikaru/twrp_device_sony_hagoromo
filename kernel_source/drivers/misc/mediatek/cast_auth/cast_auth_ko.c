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
#include <linux/fcntl.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/major.h>
#include <linux/semaphore.h>
#include <linux/file.h>
#include <linux/mutex.h>
#include <linux/shmem_fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <tz_cross/ta_cast_auth.h>
#include <tz_cross/trustzone.h>
#include <tz_cross/ta_icnt.h>
#include <tz_cross/ta_mem.h>

#include <trustzone/kree/system.h>
#include <trustzone/kree/mem.h>
#include <trustzone/kree/system.h>

#include "cast_auth_ko.h"


#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/mach-types.h>
#include <linux/mutex.h>
#include <linux/hardirq.h>
#include <linux/sched.h>


#define CAST_AUTH_KO_SW_ALIGN_SZ(sz, align) ((((sz) + (align) - 1) / (align) ) * (align))
#define CAST_AUTH_KO_SEC_SW_ALIGN_32(sz)        CAST_AUTH_KO_SW_ALIGN_SZ((sz), 32)

/*-----------------------------------------------------------------------------
                    global data
 ----------------------------------------------------------------------------*/

static UINT8    ui1RefCnt;         //Reference count(debug use)
static DEFINE_MUTEX(hSema);


//for debug usage
void cast_auth_dump(char *name, const unsigned char *data, int len)
{

#ifdef ENABLE_CAST_AUTH_LOG
    int pos = 0;

    if (NULL == name || NULL == data || 0 == len)
    {
        return;
    }

    printk("\n\n[CAST_AUTH KO]--dump:%s--len %d\n", name,len);

    while (pos < len) // only show 1 bytes to check message ID
    {
        if (pos % 32 == 0)
            printk("\n[%04x]|", pos);

        printk("%02x ", data[pos]);

        pos ++;
    }

    printk("\n------------------------------\n\n");

#endif
}

static void cast_auth_lock(void)
{
    mutex_lock(&hSema);
    printk("[CAST_AUTH KO]  cast_auth_lock()\n");

}

static void cast_auth_unlock(void)
{
    mutex_unlock(&hSema);
    printk("[CAST_AUTH KO]  cast_auth_unlock()\n");
}

static int cast_auth_do_tz_cmd
(
    unsigned int  emode ,
    UINT8 * in_enc_key,
    UINT32  in_enc_key_len,
    UINT8 * in_data_1,
    UINT32  in_data_1_len,
    UINT8 * in_data_2,
    UINT32  in_data_2_len,
    UINT8 * out_data_1,
    UINT32  out_data_1_len,
    UINT8 * out_data_2,
    UINT32* out_data_2_len
)
{
    int ret = -1;
    UINT32 * mem_size_pool;
    uint32_t paramTypes;
    MTEEC_PARAM param[4];
    KREE_SESSION_HANDLE pHandle ;
    UINT8 * shm_ptr ;

    if (0 == in_data_1_len || NULL == in_data_1 ||
        0 == out_data_1_len || NULL == out_data_1 ||
        0 == in_enc_key_len || NULL == in_enc_key ||
        (CAST_AUTH_KO_OP_SIGN_HASH == emode && NULL == in_data_2) ||
        (CAST_AUTH_KO_OP_GENERATE_DEVICECERT_AND_KEY == emode && NULL == out_data_2))
    {
        printk("cast_auth_do_tz_cmd invalid parameters\n");
        return -1;
    }

    //seperate first 8 K as input data, second 8K is output data
    //use mem_size to record the size of each para
    //can only support 8 inputs 8 outputs and total memsize less than 16K
    shm_ptr = (UINT8 *) kmalloc(SHARED_MEM_SIZE, GFP_KERNEL);

    if (NULL == shm_ptr)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc SHARED_MEM error\n");
        return -2;
    }
    memset(shm_ptr, 0, SHARED_MEM_SIZE);

    mem_size_pool = (UINT32 *) kmalloc(SUPPORT_INOUT_PARAMETER * sizeof(UINT32), GFP_KERNEL);

    if (NULL == mem_size_pool)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc mem_size pool error\n");
        kfree(shm_ptr);
        return -3;
    }
    memset(mem_size_pool, 0, SUPPORT_INOUT_PARAMETER*sizeof(UINT32));

    ret = KREE_CreateSession(TZ_TA_CAST_AUTH_UUID, &pHandle);

    if (ret != TZ_RESULT_SUCCESS)
    {
        printk("[KERNEL]KREE_CreateSession Error: %s\n", TZ_GetErrorString(ret));
        return ret;
    }

    paramTypes = TZ_ParamTypes3(TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_VALUE_INOUT);

    param[0].mem.buffer = (void *)shm_ptr;
    param[0].mem.size = SHARED_MEM_SIZE ;
    param[1].mem.buffer = (void *)mem_size_pool;
    param[1].mem.size = SUPPORT_INOUT_PARAMETER * sizeof(UINT32);
    param[2].value.a = emode; // mode
    param[2].value.b = 1;  //return value 0 is right

    memcpy(shm_ptr, in_enc_key, in_enc_key_len);
    mem_size_pool[0] = in_enc_key_len;
    cast_auth_dump("cast ko der",shm_ptr,mem_size_pool[0]);
    printk("[KERNEL]der size: %lu\n", mem_size_pool[0]);
    
    memcpy(shm_ptr + mem_size_pool[0] , in_data_1, in_data_1_len);
    mem_size_pool[1] = in_data_1_len;
    cast_auth_dump("cast ko in_data_1",(UINT8 *)shm_ptr + mem_size_pool[0],mem_size_pool[1]);
    printk("[KERNEL]in_data_1 size: %lu\n", mem_size_pool[1]);


    if (NULL != in_data_2)
    {
        memcpy((UINT8 *)shm_ptr + mem_size_pool[0] + mem_size_pool[1] , in_data_2, in_data_2_len);
        mem_size_pool[2] = in_data_2_len;

        cast_auth_dump("cast ko in_data_2", (UINT8 *)shm_ptr + mem_size_pool[0] + mem_size_pool[1], mem_size_pool[2]);
        printk("[KERNEL]in_data_2 size: %lu\n", mem_size_pool[2]);
    }

    if (NULL != out_data_1)
    {
        memcpy((UINT8 *)shm_ptr + SHARED_MEM_SIZE_OFFSET, out_data_1, out_data_1_len);
        mem_size_pool[SUPPORT_INOUT_PARAMETER_OFFSET] = out_data_1_len;
        cast_auth_dump("cast ko out_data_1", (UINT8 *)shm_ptr + SHARED_MEM_SIZE_OFFSET, mem_size_pool[SUPPORT_INOUT_PARAMETER_OFFSET]);
        printk("[KERNEL]out_data_1 size: %lu\n", mem_size_pool[SUPPORT_INOUT_PARAMETER_OFFSET]);
    }

    if (NULL != out_data_2)
    {
        mem_size_pool[SUPPORT_INOUT_PARAMETER_OFFSET + 1] = 3 * 1024;
        printk("[KERNEL]out_data_2 size: %lu\n", mem_size_pool[SUPPORT_INOUT_PARAMETER_OFFSET+1]);
    }

    printk("[KERNEL] cast_auth_do_tz_cmd begin     :\n");

    ret = KREE_TeeServiceCall(pHandle, TZCMD_CAST_AUTH , paramTypes, param);

    if (ret != TZ_RESULT_SUCCESS)
    {
        if (NULL  != shm_ptr)
        {
            kfree(shm_ptr);
            shm_ptr = NULL;
        }

        if (NULL != mem_size_pool)
        {
            kfree(mem_size_pool);
            mem_size_pool = NULL;
        }

        printk("[KERNEL] KREE_TeeServiceCall Error: %s\n", TZ_GetErrorString(ret));

        return ret;
    }

    printk("[KERNEL] cast_auth_do_tz_cmd end       :\n");

    //Copy output from continuous memory
    memcpy(out_data_1, shm_ptr + SHARED_MEM_SIZE_OFFSET, out_data_1_len);


    if ((mem_size_pool[1+ SUPPORT_INOUT_PARAMETER_OFFSET] != 0) && (emode == CAST_AUTH_KO_OP_GENERATE_DEVICECERT_AND_KEY) && (param[2].value.b == 0))
    {
        *out_data_2_len  = ((UINT32 *)(param[1].mem.buffer))[1+ SUPPORT_INOUT_PARAMETER_OFFSET];
        printk("[KERNEL] out_data_2_len end     %lu  :\n", ((UINT32 *)(param[1].mem.buffer))[1+ SUPPORT_INOUT_PARAMETER_OFFSET]);
        memcpy(out_data_2, shm_ptr + SHARED_MEM_SIZE_OFFSET  + out_data_1_len, *out_data_2_len);
    }

    ret = param[2].value.b;

    printk("[KERNEL] cast_auth_do_tz_cmd pt_param->ret[%d] ret[%d] \n",  param[2].value.b, ret);

    if (NULL != shm_ptr)
        kfree(shm_ptr);

    if (NULL != mem_size_pool)
        kfree(mem_size_pool);
    
    if(ret != 0)
        {
            return ret;
        }
    ret = KREE_CloseSession(pHandle);

    if (ret != TZ_RESULT_SUCCESS)
    {
        printk("[KERNEL]KREE_CloseSession Error: %s\n", TZ_GetErrorString(ret));
        return ret;
    }

    return ret;
}

static int cast_auth_get_enc_key_size(void)
{
    UINT32 file_len = 0;

    struct file * f_key = NULL;

    struct inode * innode = NULL;

    if (IS_ERR((f_key = filp_open(ENC_RSA_KEY_PATH, O_RDONLY, 0))))
    {
        printk("can not open file %s\n\n", ENC_RSA_KEY_PATH);
        return -1;
    }

    else
    {
        innode = f_key->f_dentry->d_inode;
        file_len = (int)(innode->i_size);
        filp_close(f_key, NULL);
        printk(" open file %s len= %lu\n\n", ENC_RSA_KEY_PATH, file_len);
        return file_len;
    }
}

static int cast_auth_get_enc_key(UINT8 * in_enc_key , UINT32  in_enc_key_len)
{

    int count = 0;

    struct file * f_key = NULL;
    mm_segment_t fs;
    loff_t pos = 0;

    if (IS_ERR((f_key = filp_open(ENC_RSA_KEY_PATH, O_RDONLY, 0))))
    {
        printk("can not open file %s\n\n", ENC_RSA_KEY_PATH);
        return -1;
    }

    else
    {
        fs = get_fs();
        set_fs(KERNEL_DS);
        count = vfs_read(f_key, in_enc_key, in_enc_key_len, &pos);

        if (-1 == count)
        {
            printk("read file %s fail\n", ENC_RSA_KEY_PATH);
            return -1;
        }

        filp_close(f_key, NULL);

        set_fs(fs);
    }

    return 0;
}

static int cast_auth_process_cmd_sign_hash(unsigned long arg)
{
    int i4Ret = CAST_AUTHR_OK;
    AUDIO_CAST_DRM_SIGN_HASH_T* rArg;
    AUDIO_CAST_DRM_SIGN_HASH_T pAudioCast;
    UINT8 *k_in_enc_key = NULL;   /*kernel space data*/
    UINT32 in_enc_key_len = 0;
    const UINT8 *u_in_data_1 = NULL;   /*user space data*/
    UINT8 *k_in_data_1 = NULL;   /*kernel space data*/
    UINT32 in_data_1_len = 0;
    const UINT8 *u_in_data_2 = NULL;   /*user space data*/
    UINT8 *k_in_data_2 = NULL;   /*kernel space data*/
    UINT32 in_data_2_len = 0;
    UINT8 *u_out_data = NULL;   /*user space data*/
    UINT8 *k_out_data = NULL;   /*kernel space data*/
    UINT32 out_data_len = 0;

    printk("cast_auth_process_cmd sign hash\n\n");

    rArg = (AUDIO_CAST_DRM_SIGN_HASH_T __user *)arg;

    if (NULL == rArg)
    {
        return -1;
    }

    /*save the structur from user-space*/

    if (copy_from_user(&pAudioCast, (void*)(rArg), sizeof(AUDIO_CAST_DRM_SIGN_HASH_T)))
    {
        return -2;
    }

    /*get arguments from user space */
    in_data_1_len   = pAudioCast.in_hash_len;

    in_data_2_len   = pAudioCast.in_wrapped_device_key_len;

    out_data_len    = pAudioCast.out_signature_len;

    u_in_data_1     = pAudioCast.in_hash;

    u_in_data_2     = pAudioCast.in_wrapped_device_key;

    u_out_data      = pAudioCast.out_signature;

    if (0 == in_data_1_len || 0 == out_data_len || 0 == in_data_2_len ||
        NULL == u_in_data_1 ||  NULL == u_in_data_2  || NULL == u_out_data)
    {
        printk("[KERNEL] cast_auth_process_cmd: invalid parameters\n");
        return -3;
    }

    printk("[KERNEL] sign hash  cast_auth_get_enc_key: get rsa key\n");

    in_enc_key_len = cast_auth_get_enc_key_size();
    k_in_enc_key = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(in_enc_key_len), GFP_KERNEL);

    if (NULL == k_in_enc_key)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_in_enc_key error\n");
        i4Ret = -4;
        goto err;
    }

    if (cast_auth_get_enc_key(k_in_enc_key, in_enc_key_len))
    {
        printk("[KERNEL] cast_auth_process_cmd: cast_auth_get_enc_key error\n");
        i4Ret = -5;
        goto err;
    }

    k_in_data_1 = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(in_data_1_len), GFP_KERNEL);

    if (NULL == k_in_data_1)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_in_data_1 error\n");
        i4Ret = -6;
        goto err;
    }

    k_in_data_2 = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(in_data_2_len), GFP_KERNEL);

    if (NULL == k_in_data_2)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_in_data_2 error\n");
        i4Ret = -7;
        goto err;
    }

    k_out_data = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(out_data_len), GFP_KERNEL);

    if (NULL == k_out_data)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_out_data error\n");
        i4Ret = -8;
        goto err;
    }

    if (copy_from_user((void*)k_in_data_1, (void*)u_in_data_1, in_data_1_len))
    {
        i4Ret = -9;
        goto err;
    }

    if (copy_from_user((void*)k_in_data_2, (void*)u_in_data_2, in_data_2_len))
    {
        i4Ret = -10;
        goto err;
    }

    printk("[KERNEL] cast_auth_process_cmd sign hash  data into user space ok:\n");

    i4Ret = cast_auth_do_tz_cmd(CAST_AUTH_KO_OP_SIGN_HASH,
                                k_in_enc_key,
                                in_enc_key_len,
                                k_in_data_1,
                                in_data_1_len,
                                k_in_data_2,
                                in_data_2_len,
                                k_out_data,
                                out_data_len,
                                NULL,
                                NULL
                               );

    printk("[KERNEL] cast_auth_do_tz_cmd sign hash ret[%d]:\n", i4Ret);
    pAudioCast.result = i4Ret;

    if (copy_to_user((void*)u_out_data, (void*)k_out_data, out_data_len))
    {
        i4Ret = -11;
        goto err;
    }

    /*copy the result into  user-space*/

    if (copy_to_user((void*)(rArg), (void*)&pAudioCast,  sizeof(AUDIO_CAST_DRM_SIGN_HASH_T)))
    {

        i4Ret = -12;
        goto err;
    }

err:

    if (NULL != k_in_enc_key)
    {
        kfree(k_in_enc_key);
        k_in_enc_key = NULL;
    }

    if (NULL != k_in_data_1)
    {
        kfree(k_in_data_1);
        k_in_data_1 = NULL;
    }

    if (NULL != k_in_data_2)
    {
        kfree(k_in_data_2);
        k_in_data_2 = NULL;
    }

    if (NULL != k_out_data)
    {
        kfree(k_out_data);
        k_out_data = NULL;
    }

    return i4Ret;
}

static int cast_auth_process_cmd_generate_device_cert_and_key(unsigned long arg)
{
    int i4Ret = CAST_AUTHR_OK;
    AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T* rArg;
    AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T pAudioCast;
    UINT8 *k_in_enc_key = NULL;   /*kernel space data*/
    UINT32 in_enc_key_len = 0;
    const UINT8 *u_in_data_1 = NULL;   /*user space data*/
    UINT8 *k_in_data_1 = NULL;   /*kernel space data*/
    UINT32 in_data_1_len = 0;
    UINT8 *u_out_data_1 = NULL;   /*user space data*/
    UINT8 *k_out_data_1 = NULL;   /*kernel space data*/
    UINT32 out_data_1_len = 0;
    UINT8 *u_out_data_2 = NULL;   /*user space data*/
    UINT8 *k_out_data_2 = NULL;   /*kernel space data*/
    UINT32 out_data_2_len = 0;


    printk("cast_auth_process_cmd generate_device_cert_and_key\n\n");

    rArg = (AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T __user *)arg;

    if (NULL == rArg)
    {
        return -1;
    }

    /*save the structur from user-space*/

    if (copy_from_user(&pAudioCast, (void*)(rArg), sizeof(AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T)))
    {
        return -2;
    }

    /*get arguments from user space */
    in_data_1_len     = pAudioCast.in_mac_address_len;

    out_data_1_len    = pAudioCast.in_out_device_certificate_len;

    out_data_2_len    = pAudioCast.in_out_wrapped_device_key_len;

    u_in_data_1       = pAudioCast.in_mac_address;

    u_out_data_1      = pAudioCast.in_out_device_certificate;

    u_out_data_2      = pAudioCast.out_wrapped_device_key;

    if (0 == in_data_1_len ||  0 == out_data_1_len || 0 == out_data_2_len ||
        NULL == u_in_data_1 ||  NULL == u_out_data_1 || NULL == u_out_data_2)
    {
        printk("[KERNEL] cast_auth_process_cmd: invalid parameters\n");
        return -3;
    }

    printk("[KERNEL] generate_device_cert_and_key cast_auth_get_enc_key: get rsa key\n");

    in_enc_key_len = cast_auth_get_enc_key_size();
    k_in_enc_key = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(in_enc_key_len), GFP_KERNEL);

    if (NULL == k_in_enc_key)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_in_enc_key error\n");
        i4Ret = -4;
        goto err;
    }

    if (cast_auth_get_enc_key(k_in_enc_key, in_enc_key_len))
    {
        printk("[KERNEL] cast_auth_process_cmd: cast_auth_get_enc_key error\n");
        i4Ret = -5;
        goto err;
    }

    k_in_data_1 = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(in_data_1_len), GFP_KERNEL);


    if (NULL == k_in_data_1)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_in_data_1 error\n");
        i4Ret = -6;
        goto err;
    }

    k_out_data_1 = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(out_data_1_len), GFP_KERNEL);

    if (NULL == k_out_data_1)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_out_data_1 error\n");
        i4Ret = -7;
        goto err;
    }

    printk("[KERNEL] cast_auth_process_cmd: malloc k_out_data_1 %lu\n", CAST_AUTH_KO_SEC_SW_ALIGN_32(out_data_1_len));

    k_out_data_2 = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(out_data_2_len), GFP_KERNEL);
    printk("[KERNEL] cast_auth_process_cmd: malloc k_out_data_2 %lu\n", CAST_AUTH_KO_SEC_SW_ALIGN_32(out_data_2_len));


    if (NULL == k_out_data_2)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_out_data_2 error\n");
        i4Ret = -8;
        goto err;
    }

    if (copy_from_user((void*)k_in_data_1, (void*)u_in_data_1, in_data_1_len))
    {
        i4Ret = -9;
        goto err;
    }

    /*  This is in out data device certificate */
    if (copy_from_user((void*)k_out_data_1, (void*)u_out_data_1, out_data_1_len))
    {
        i4Ret = -10;
        goto err;
    }

    printk("[KERNEL] generate_device_cert_and_key cast_auth_process_cmd data into user space ok:\n");
    i4Ret = cast_auth_do_tz_cmd(CAST_AUTH_KO_OP_GENERATE_DEVICECERT_AND_KEY,
                                k_in_enc_key,
                                in_enc_key_len,
                                k_in_data_1,
                                in_data_1_len,
                                NULL,
                                0,
                                k_out_data_1,
                                out_data_1_len,
                                k_out_data_2,
                                &out_data_2_len
                               );

    printk("[KERNEL] generate_device_cert_and_key cast_auth_do_tz_cmd ret[%d]:\n", i4Ret);

    pAudioCast.result = i4Ret;
    /* get right wrapped device key len*/
    pAudioCast.in_out_wrapped_device_key_len = out_data_2_len;
    printk("[KERNEL] generate_device_cert_and_key out_wrapped_device_key_len %lu:\n", out_data_2_len);

    if (copy_to_user((void*)u_out_data_1, (void*)k_out_data_1, out_data_1_len))
    {
        i4Ret =  -11;
        goto err;
    }

    if (copy_to_user((void*)u_out_data_2, (void*)k_out_data_2, out_data_2_len))
    {
        i4Ret = -12;
        goto err;
    }

    /*copy the result into  user-space*/

    if (copy_to_user((void*)(rArg), (void*)&pAudioCast,  sizeof(AUDIO_CAST_DRM_GENERATE_DEVICECERT_AND_KEY_T)))
    {
        i4Ret = -13;
        goto err;
    }


err:

    if (NULL != k_in_enc_key)
    {
        kfree(k_in_enc_key);
        k_in_enc_key = NULL;
    }

    if (NULL != k_in_data_1)
    {
        kfree(k_in_data_1);
        k_in_data_1 = NULL;
    }

    if (NULL != k_out_data_1)
    {
        kfree(k_out_data_1);
        k_out_data_1 = NULL;
    }

    if (NULL != k_out_data_2)
    {
        kfree(k_out_data_2);
        k_out_data_2 = NULL;
    }

    return i4Ret;
}


static int cast_auth_process_cmd_get_uid(unsigned long arg)
{
    int i4Ret = CAST_AUTHR_OK;
    AUDIO_CAST_UID_T* rArg;
    AUDIO_CAST_UID_T  pAudioCast;
    
    UINT8       _kernel_  * k_in_enc_key = NULL;   /*kernel space data*/
    UINT32                in_enc_key_len = 0;
    
    const UINT8 _user_     * u_in_data_1 = NULL;   /*user space data*/
    UINT8       _kernel_   * k_in_data_1 = NULL;   /*kernel space data*/
    UINT32                 in_data_1_len = 0;
    
    UINT8       _user_      * u_out_data = NULL;   /*user space data*/
    UINT8                   * k_out_data = NULL;   /*kernel space data*/
    UINT32      _kernel_    out_data_len = 0;
	
    /* needn't use data 2 */
    UINT8       _kernel_   * k_in_data_2 = NULL;   
    UINT32                 in_data_2_len = 0;
    

    printk("cast_auth_process_cmd get uid \n\n");

    rArg = (AUDIO_CAST_UID_T __user *)arg;

    if (NULL == rArg)
    {
        return -1;
    }

    /*save the structur from user-space*/

    if (copy_from_user(&pAudioCast, (void*)(rArg), sizeof(AUDIO_CAST_UID_T)))
    {
        return -2;
    }

    /*get arguments from user space */
    in_data_1_len   = pAudioCast.in_mac_address_len;

    out_data_len    = pAudioCast.out_uniqueID_len;

    u_in_data_1     = pAudioCast.in_mac_address;

    u_out_data      = pAudioCast.out_uniqueID;

    if (0 == in_data_1_len || 0 == out_data_len ||
        NULL == u_in_data_1 || NULL == u_out_data)
    {
        printk("[KERNEL] cast_auth_process_cmd: invalid parameters\n");
        return -3;
    }

    if (WIFI_MAC_ADDRESS_LEN != in_data_1_len || UNIQUE_ID_LEN != out_data_len)
    {
        printk("[KERNEL] cast_auth_process_cmd: invalid parameters, in_data_1_len(%ld), out_data_len(%ld)\n", in_data_1_len, out_data_len);
        return -3;
    }

    printk("[KERNEL] sign hash  cast_auth_get_enc_key: get rsa key\n");

    in_enc_key_len = cast_auth_get_enc_key_size();
    k_in_enc_key = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(in_enc_key_len), GFP_KERNEL);

    if (NULL == k_in_enc_key)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_in_enc_key error\n");
        i4Ret = -4;
        goto err;
    }

    if (cast_auth_get_enc_key(k_in_enc_key, in_enc_key_len))
    {
        printk("[KERNEL] cast_auth_process_cmd: cast_auth_get_enc_key error\n");
        i4Ret = -5;
        goto err;
    }

    k_in_data_1 = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(in_data_1_len), GFP_KERNEL);

    if (NULL == k_in_data_1)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_in_data_1 error\n");
        i4Ret = -6;
        goto err;
    }

    k_out_data = (UINT8 *)kmalloc(CAST_AUTH_KO_SEC_SW_ALIGN_32(out_data_len), GFP_KERNEL);

    if (NULL == k_out_data)
    {
        printk("[KERNEL] cast_auth_process_cmd: malloc k_out_data error\n");
        i4Ret = -8;
        goto err;
    }

    if (copy_from_user((void*)k_in_data_1, (void*)u_in_data_1, in_data_1_len))
    {
        i4Ret = -9;
        goto err;
    }

    printk("[KERNEL] cast_auth_process_cmd get uid  data into user space ok:\n");

    i4Ret = cast_auth_do_tz_cmd(CAST_AUTH_KO_OP_GET_UID,
                                k_in_enc_key,
                                in_enc_key_len,
                                k_in_data_1,
                                in_data_1_len,
                                k_in_data_2,
                                in_data_2_len,
                                k_out_data,
                                out_data_len,
                                NULL,
                                NULL
                               );

    printk("[KERNEL] cast_auth_do_tz_cmd get uid ret[%d]:\n", i4Ret);
    pAudioCast.result = i4Ret;

    if (copy_to_user((void*)u_out_data, (void*)k_out_data, out_data_len))
    {
        i4Ret = -11;
        goto err;
    }

    /*copy the result into  user-space*/

    if (copy_to_user((void*)(rArg), (void*)&pAudioCast,  sizeof(AUDIO_CAST_UID_T)))
    {

        i4Ret = -12;
        goto err;
    }

err:

    if (NULL != k_in_enc_key)
    {
        kfree(k_in_enc_key);
        k_in_enc_key = NULL;
    }

    if (NULL != k_in_data_1)
    {
        kfree(k_in_data_1);
        k_in_data_1 = NULL;
    }

    if (NULL != k_out_data)
    {
        kfree(k_out_data);
        k_out_data = NULL;
    }

    return i4Ret;
}


static long cast_auth_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = -1;

    printk("[KERNEL] cast_auth_process_cmd cmd:%x\n", cmd);

    if (CAST_AUTH_IOCTL_CMD_DRM_SIGN_HASH == cmd)
    {
        ret = cast_auth_process_cmd_sign_hash(arg);
    }

    else if (CAST_AUTH_IOCTL_CMD_DRM_GENERATE_DEVICECERT_AND_KEY == cmd)
    {
        ret = cast_auth_process_cmd_generate_device_cert_and_key(arg);
    }

    else if (CAST_AUTH_IOCTL_CMD_DRM_GET_UID == cmd)
    {
        ret = cast_auth_process_cmd_get_uid(arg);
    }
    
    else
    {
        printk("cast_auth_ioctl cmd(%x) invalid\n", cmd);
        ret = -1;
    }
    return ret;
}

static int cast_auth_open(struct inode *inode, struct file *file)
{

    cast_auth_lock();
    ++ui1RefCnt;
    cast_auth_unlock();

    printk("[CAST_AUTH KO] cast_auth_open() ref=%d.\n", ui1RefCnt);
    return 0 ;
}

static int cast_auth_release(struct inode *inode, struct file *file)
{

    cast_auth_lock();
    --ui1RefCnt;
    cast_auth_unlock();

    printk("[CAST_AUTH KO] cast_auth_release() ref=%d.\n", ui1RefCnt);
    return 0;
}

static const struct file_operations cast_auth_fops =
{
    .owner          = THIS_MODULE,
    .open           = cast_auth_open,
    .release        = cast_auth_release,
    .unlocked_ioctl = cast_auth_ioctl,
};

static struct miscdevice cast_auth_misc =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "cast_auth",
    .fops = &cast_auth_fops,
};

/*-----------------------------------------------------------------------------
                    public functions
 ----------------------------------------------------------------------------*/

int cast_auth_ko_init(void)
{
    int ret;

    ret = misc_register(&cast_auth_misc);

    if (ret)
    {
        printk("[CAST_AUTH KO] fail to register misc devices!\n");
        return ret;
    }

    ui1RefCnt = 0;

    printk("[CAST_AUTH KO] cast_auth_ko_init() \n");

    return 0;
}

int cast_auth_ko_uninit(void)
{
    int ret;
    printk("[CAST_AUTH KO] cast_auth_ko_uninit() enter.\n");

    ret = misc_deregister(&cast_auth_misc);

    if (ret)
        printk("CAST_AUTH KO: failed to unregister misc device!\n");

    printk("[CAST_AUTH KO] cast_auth_ko_uninit() \n");

    return 0;
}
