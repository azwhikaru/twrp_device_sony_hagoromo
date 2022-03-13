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
#include "vdec_hal_if_common.h"
#include "vdec_hw_common.h"
#include <mach/mt_typedefs.h>
//#include "x_ckgen.h"
//#include "drv_config.h"
//#include "x_debug.h"

#if (CONFIG_DRV_LINUX_DATA_CONSISTENCY)
//#include <config/arch/chip_ver.h>
#endif
#include "../verify/vdec_verify_mpv_prov.h"

#if CONFIG_SECTION_BUILD_LINUX_KO
#include <mach/irqs.h>
//#include <mach/mt8530.h>



#define MUSB_BASE3                        (IO_VIRT + 0xF000)
#define MUSB_ANAPHYBASE                    (0x700)

#define M_REG_AUTOPOWER                      0x80
#define M_REG_AUTOPOWER_DRAMCLK         0x01
#define M_REG_AUTOPOWER_ON                     0x02
#define M_REG_AUTOPOWER_PHYCLK             0x04
#define MUSB_BASE                       (IO_VIRT + 0xE000)
#define MUSB_PHYBASE                           (0x400)
#define MUSB_MISCBASE                     (0x600)

#define MGC_PHY_Read32(_pBase, r)      \
    *((volatile UINT32 *)(((UINT32)_pBase) + (MUSB_PHYBASE)+ (r)))
#define MGC_PHY_Write32(_pBase, r, v)  \
    (*((volatile UINT32 *)((((UINT32)_pBase) + MUSB_PHYBASE)+ (r))) = v)
#define MGC_MISC_Read32(_pBase, r)     \
    *((volatile UINT32 *)(((UINT32)_pBase) + (MUSB_MISCBASE)+ (r)))
#define MGC_MISC_Write32(_pBase, r, v) \
    (*((volatile UINT32 *)(((UINT32)_pBase) + (MUSB_MISCBASE)+ (r))) = v)
#define MGC_AnaPhy_Read32(_pBase, _offset) \
    (*((volatile UINT32 *)(((UINT32)_pBase) + MUSB_ANAPHYBASE + _offset)))
#define MGC_AnaPhy_Write32(_pBase, _offset, _data) \
    (*((volatile UINT32 *)(((UINT32)_pBase) + MUSB_ANAPHYBASE + _offset)) = _data)
#endif

#define VDEC_VLD_USE_USB  0
// **************************************************************************
// Function : INT32 i4VDEC_HAL_Common_Init(UINT32 u4ChipID);
// Description : Turns on video decoder HAL
// Parameter : u4ChipID
// Return      : >0: init OK.
//                  <0: init failed
// **************************************************************************
INT32 i4VDEC_HAL_Common_Init(UINT32 u4ChipID)
{
#if (!CONFIG_DRV_FPGA_BOARD)

#if CONFIG_SECTION_BUILD_LINUX_KO
    //In Linux environemnt, enable USB PLL for VDEC, if USB driver compile.
#if (VDEC_VLD_USE_USB  || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530))
    UINT32 pBase =  MUSB_BASE;
    UINT32 u4Reg = 0;
#endif

#endif
///TODO: Clock Setting
/*
   #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520)
    //set vdec clock
    CKGEN_WRITE32(REG_RW_CLK_SEL_0, (CKGEN_READ32(REG_RW_CLK_SEL_0)&(~(CLK_SEL_0_VDEC_MASK|CLK_SEL_0_MC_MASK)))
    |((CLK_SEL_0_MC_DMPLL_1_4 << 12) | (CLK_SEL_0_MC_DMPLL_1_4 << 16)));
    #else

    #if (CONFIG_DRV_VERIFY_SUPPORT)    
    if (_u4CodecVer[0] == VDEC_H264 || _u4CodecVer[0] == VDEC_AVS || _u4CodecVer[0] == VDEC_MPEG2 || _u4CodecVer[0] == VDEC_RM)
    	{
   #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
    	CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CFG1_VDEC_SEL_SYSPLL1_1_2);
   #else
    	CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CLK_VDEC_SEL_SYSPLL2_1_2);
   #endif
    	}
    else if (_u4CodecVer[0] == VDEC_MPEG4 || _u4CodecVer[0] == VDEC_WMV)
    	{
   #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
    	CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CFG1_VDEC_SEL_SYSPLL1_1_2);
   #else
    	CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CLK_VDEC_SEL_SYSPLL1_1_2);
   #endif
    	}    	
    else
    	{
   #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
    	CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CFG1_VDEC_SEL_SYSPLL1_1_2);
   #else
    	CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CLK_VDEC_SEL_SYSPLL2_1_2);
   #endif
    	}
   #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
    	CKGEN_AgtSelClk(e_CLK_MC,   CLK_CFG1_MC_SEL_SYSPLL2_1_2);
   #else
    	CKGEN_AgtSelClk(e_CLK_MC,   CLK_CLK_MC_SEL_SYSPLL2_1_2);
   #endif
    #else
#if (CONFIG_CHIP_VER_CURR  < CONFIG_CHIP_VER_MT8580)
#if VDEC_VLD_USE_USB    
    CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CLK_VDEC_SEL_USBPLL);
#else
   #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
    	CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CFG1_VDEC_SEL_27M);
        CKGEN_WRITE32(REG_RW_CLK_CFG8, (CKGEN_REG32(REG_RW_CLK_CFG8)&0xFFFFFFF8) | 0x07);
   #else
    CKGEN_AgtSelClk(e_CLK_VDEC, CLK_CLK_VDEC_SEL_SYSPLL1_1_2);
   #endif
#endif
   #if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8580)
//    	CKGEN_AgtSelClk(e_CLK_MC,   CLK_CFG1_MC_SEL_SYSPLL2_1_2);
   #else
    CKGEN_AgtSelClk(e_CLK_MC,   CLK_CLK_MC_SEL_SYSPLL2_1_2);
   #endif
    #endif
    #endif
    #endif
*/
    // reset common SRAM
    vVDecWriteVLD(0, RW_VLD_RESET_COMMOM_SRAM, 0x00000000);
    vVDecWriteVLD(0, RW_VLD_RESET_COMMOM_SRAM, 0x00000100);
    vVDecWriteVLD(0, RW_VLD_RESET_COMMOM_SRAM, 0x00000000);
    vVDecWriteVLD(1, RW_VLD_RESET_COMMOM_SRAM, 0x00000000);
    vVDecWriteVLD(1, RW_VLD_RESET_COMMOM_SRAM, 0x00000100);
    vVDecWriteVLD(1, RW_VLD_RESET_COMMOM_SRAM, 0x00000000);
#endif

    return HAL_HANDLE_OK;
}

#if CONFIG_DRV_FPGA_BOARD
void vVDec_Hal_EnableEmuClk(void)
{
   #if 1 //CONFIG_MT8130_FPGA
   //printk("vVDec_Hal_EnableEmuClk, 0xF600 0000:0x%x, MISC 0xF4(61):0x%x - before\n", u4ReadVDecReg(0x00), u4VDecReadMISC(0,0xF4));
   vWriteReg(0x00,0x01);
   printk("vVDec_Hal_EnableEmuClk, 0xF600 0000:0x%x, MISC 0xF4(61):0x%x - \n", u4ReadReg(0x00), u4VDecReadMISC(0,0xF4));
   vVDecWriteMISC(0,0xF4,(u4VDecReadMISC(0,0xF4) & 0xFFFFFFFE));
   printk("vVDec_Hal_EnableEmuClk, 0xF600 0000:0x%x, MISC 0xF4(61):0x%x - after\n", u4ReadReg(0x00), u4VDecReadMISC(0,0xF4));
   #endif
}
#endif


#if ((CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8580)  && (!CONFIG_DRV_FPGA_BOARD))
void vVDEC_HAL_CLK_Set(UINT32 u4CodeType)
{
    vVDecSetVldMcClk(0,u4CodeType);
}
#endif

#if VDEC_REMOVE_UNUSED_FUNC
// **************************************************************************
// Function : INT32 i4VDEC_HAL_Common_Uninit(void);
// Description : Turns off video decoder HAL
// Parameter : void
// Return      : >0: uninit OK.
//                  <0: uninit failed
// **************************************************************************
INT32 i4VDEC_HAL_Common_Uninit(void)
{
    return HAL_HANDLE_OK;
}
#endif

#if 0
/******************************************************************************
* Global Function
******************************************************************************/
void MPV_PllInit(void)
{
       UINT32 u4DramClk;
       UINT16 u2MS, u2NS, u2Counter;
       UINT8 u1Band;
       UINT8 ucDelay;

       u4DramClk = BSP_GetClock(CAL_SRC_DMPLL, &u1Band, &u2MS, &u2NS,
            &u2Counter);

       printk("DRAM CLK: %d\n", u4DramClk);
/*
	// set vpll test_gnd = 1, bs = 4, bw = 2, gs = 2, cpi = 8 (default)
	// 148.5MHz
	*((UINT32*)0x2000D030) = (UINT32)0x00082528;
*/
	ucDelay = 0;
	*((UINT32*)0x2000D03c) = (*((UINT32*)0x2000D03c) & 0xC0FFC0FF) | ((ucDelay<<24)|(ucDelay<<8));
	*((UINT32*)0x2000D040) = (*((UINT32*)0x2000D040) & 0xFFFFC0FF) | (ucDelay<<8);

	// set mdec_ck = ck_vpll (default)
	*((UINT32*)0x2000D048) = (*((UINT32*)0x2000D048) & 0xFF00FFFF) | 0x00010000;
	*((UINT32*)0x20007054) = (*((UINT32*)0x20007054) & 0xFFFF000F) | 0x0000FFF0;


}
#endif

#if VDEC_REMOVE_UNUSED_FUNC
// **************************************************************************
// Function : UINT32 u4VDEC_HAL_Common_GetHWResourceNum(UINT32 *pu4BSNum, UINT32 *pu4VLDNum);
// Description :Get hardware resource number
// Parameter : pu4BSNum : Pointer to barrel shifter number of every VLD
//                 pu4VLDNum : Pointer to VLD number
// Return      : None
// **************************************************************************
void vVDEC_HAL_Common_GetHWResourceNum(UINT32 *pu4BSNum, UINT32 *pu4VLDNum)
{
    *pu4BSNum = 2;
    *pu4VLDNum = 2;
  
    return;
}
#endif

// **************************************************************************
// Function : void vDEC_HAL_COMMON_SetVLDPower(UINT32 u4VDecID, BOOL fgOn);
// Description :Turn on or turn off VLD power
// Parameter : u4VDecID : video decoder hardware ID
//                 fgOn : Flag to vld power on or off
// Return      : None
// **************************************************************************
void vDEC_HAL_COMMON_SetVLDPower(UINT32 u4VDecID, BOOL fgOn)
{
    if (fgOn)
    {
        vVDecWriteVLD(u4VDecID, RW_VLD_PWRSAVE, 0);
    }
    else
    {
        vVDecWriteVLD(u4VDecID, RW_VLD_PWRSAVE, 1);
    }
    return;
}


// **************************************************************************
// Function : void vDEC_HAL_COMMON_PowerOn(UINT32 u4VDecID, BOOL fgOn);
// Description :Turn on or turn off VLD power
// Parameter : u4VDecID : video decoder hardware ID
//                 fgOn : Flag to vld power on or off
// Return      : None
// **************************************************************************
void vDEC_HAL_COMMON_PowerOn(void)
{    
#if 0
    CKGEN_AgtOnClk(e_CLK_VDEC);
    CKGEN_AgtOnClk(e_CLK_MC);
#endif    
    return;
}

// **************************************************************************
// Function : void vDEC_HAL_COMMON_PowerOff(UINT32 u4VDecID, BOOL fgOn);
// Description :Turn on or turn off VLD power
// Parameter : u4VDecID : video decoder hardware ID
//                 fgOn : Flag to vld power on or off
// Return      : None
// **************************************************************************
void vDEC_HAL_COMMON_PowerOff(void)
{    
#if 0
    CKGEN_AgtOffClk(e_CLK_VDEC);
    CKGEN_AgtOffClk(e_CLK_MC);
#endif    
    return;
}



INT32 i4VDEC_HAL_Dram_Busy( UINT32 u4ChipID, UINT32 u4StartAddr, UINT32 u4Offset)
{

    vWriteReg(0x7210, (u4StartAddr << 4));
    vWriteReg(0x7214, (u4Offset << 4));
    vWriteReg(0x7104, 0x0);    
    vWriteReg(0x7218, 0x8e0f110d);    
    return 0;
}

INT32 i4VDEC_HAL_Dram_Busy_Off( UINT32 u4ChipID, UINT32 u4StartAddr, UINT32 u4Offset)
{

    vWriteReg(0x7210, (u4StartAddr << 4));
    vWriteReg(0x7214, (u4Offset << 4));
    //vWriteReg(0x7104, 0x0);    
    vWriteReg(0x7218, 0x860f110d);    
    return 0;
}


#if 0

static int vcodec_probe(struct platform_device *pdev)
{
	int ret;
	MFV_LOGD("+vcodec_probe\n");

	mutex_lock(&DecEMILock);
	gu4DecEMICounter = 0;
	mutex_unlock(&DecEMILock);

	mutex_lock(&EncEMILock);
	gu4EncEMICounter = 0;
	mutex_unlock(&EncEMILock);

	mutex_lock(&PWRLock);
	gu4PWRCounter = 0;
	mutex_unlock(&PWRLock);

	mutex_lock(&L2CLock);
	gu4L2CCounter = 0;
	mutex_unlock(&L2CLock);

#ifdef CONFIG_OF
	clk_smi = devm_clk_get(&pdev->dev,"MMSYS_CLK_SMI_COMMON");
	BUG_ON(IS_ERR(clk_smi));
	clk_vdec = devm_clk_get(&pdev->dev,"MT_CG_VDEC0_VDEC");
	BUG_ON(IS_ERR(clk_vdec));
	clk_vdec_larb = devm_clk_get(&pdev->dev,"MT_CG_VDEC1_LARB");
	BUG_ON(IS_ERR(clk_vdec_larb));

	clk_venc0 = devm_clk_get(&pdev->dev,"MT_CG_VENC_CKE0");
	BUG_ON(IS_ERR(clk_venc0));
	clk_venc1 = devm_clk_get(&pdev->dev,"MT_CG_VENC_CKE1");
	BUG_ON(IS_ERR(clk_venc1));
	clk_venc2 = devm_clk_get(&pdev->dev,"MT_CG_VENC_CKE2");
	BUG_ON(IS_ERR(clk_venc2));
	clk_venc3 = devm_clk_get(&pdev->dev,"MT_CG_VENC_CKE3");
	BUG_ON(IS_ERR(clk_venc3));
#endif

	ret = register_chrdev_region(vcodec_devno, 1, VCODEC_DEVNAME);
	if (ret) {
		MFV_LOGE("[VCODEC_DEBUG][ERROR] Can't Get Major number for VCodec Device\n");
	}

	vcodec_cdev = cdev_alloc();
	vcodec_cdev->owner = THIS_MODULE;
	vcodec_cdev->ops = &vcodec_fops;

	ret = cdev_add(vcodec_cdev, vcodec_devno, 1);
	if (ret)
		MFV_LOGE("[VCODEC_DEBUG][ERROR] Can't add Vcodec Device\n");

	vcodec_class = class_create(THIS_MODULE, VCODEC_DEVNAME);
	if (IS_ERR(vcodec_class)) {
		ret = PTR_ERR(vcodec_class);
		MFV_LOGE("Unable to create class, err = %d", ret);
		return ret;
	}

	vcodec_device = device_create(vcodec_class, NULL, vcodec_devno, NULL, VCODEC_DEVNAME);

	/* if (request_irq(MT_VDEC_IRQ_ID , (irq_handler_t)video_intr_dlr, IRQF_TRIGGER_LOW, VCODEC_DEVNAME, NULL) < 0) */
	if (request_irq
	    (VDEC_IRQ_ID, (irq_handler_t) video_intr_dlr, IRQF_TRIGGER_LOW, VCODEC_DEVNAME,
	     NULL) < 0) {
		MFV_LOGD("[VCODEC_DEBUG][ERROR] error to request dec irq\n");
	} else {
		MFV_LOGD("[VCODEC_DEBUG] success to request dec irq: %d\n", VDEC_IRQ_ID);
	}

	/* disable_irq(MT_VDEC_IRQ_ID); */
	disable_irq(VDEC_IRQ_ID);
	

	MFV_LOGD("[VCODEC_DEBUG] vcodec_probe Done\n");

	return 0;
}

static int vcodec_remove(struct platform_device *pDev)
{
	MFV_LOGD("vcodec_remove\n");
	return 0;
}


static const struct of_device_id vcodec_of_ids[] = {
    { .compatible = "mediatek,VDEC_GCON", },
    {}
};

static struct platform_driver VCodecDriver =
{
    .probe   = vcodec_probe,
    .remove  = vcodec_remove,
    .driver  = {
        .name  = VCODEC_DEVNAME,
        .owner = THIS_MODULE,
        .of_match_table = vcodec_of_ids,
    }
};



#endif
INT32 i4VDEC_HAL_Common_Gcon_Enable(void)
{
#if 0
    struct clk *clk_smi, *clk_vdec, *clk_vdec_larb;
    int Ret = 0;

	if((Ret = platform_driver_register(&VCodecDriver)) < 0)
	{
			printk("platform_driver_register fail\n");
			return Ret;
	}
		
	clk_smi = devm_clk_get(&pdev->dev,"MMSYS_CLK_SMI_COMMON");
	BUG_ON(IS_ERR(clk_smi));
	clk_vdec = devm_clk_get(&pdev->dev,"MT_CG_VDEC0_VDEC");
	BUG_ON(IS_ERR(clk_vdec));
	clk_vdec_larb = devm_clk_get(&pdev->dev,"MT_CG_VDEC1_LARB");
	BUG_ON(IS_ERR(clk_vdec_larb));
	

    clk_prepare(clk_smi);
	clk_enable(clk_smi);
	clk_prepare(clk_vdec);
	clk_enable(clk_vdec);
	clk_prepare(clk_vdec_larb);
	clk_enable(clk_vdec_larb);
#endif
    vWriteGconReg(0, 0x1);
	vWriteGconReg(8, 0x1);

	printk("VDEC GCON REG 0 is 0x%x\n", u4ReadGconReg(0));
	printk("VDEC GCON REG 2 is 0x%x\n", u4ReadGconReg(8));

}


INT32 i4VDEC_HAL_Common_Gcon_Enable_After_Complete(UINT32 u4InstID, UINT32 u4VDecType)
{
    UINT32 u4Value;
	UINT32 u4VDecPDNCtrlSpec = 0;
    UINT32 u4VDecPDNCtrlModule = 0;
	UINT32 u4VDecPDNCtrlModule2 = 0;

	u4Value = u4VDecReadDV(u4InstID, 41*4);
	u4Value = u4Value | 0x11;
	vVDecWriteDV( u4InstID, 41*4, u4Value); 
	u4Value = u4VDecReadDV(u4InstID, 41*4);

	vVDecWriteDV( u4InstID, 41*4, u4Value & 0xFFFFFFEF); 
	u4Value = u4VDecReadDV(u4InstID, 41*4);
    mb();
	vWriteGconReg(0, 0x1);
	vWriteGconReg(8, 0x1);

	u4Value = u4VDecReadDV(u4InstID, 42*4);
	u4Value = u4Value & 0xFFFFFFFE;
	vVDecWriteDV( u4InstID, 42*4, u4Value); 
	u4Value = u4VDecReadDV(u4InstID, 42*4);

	printk("vdec_misc register 42 value is 0x%x\n", u4Value);
    mb();
	u4Value = u4VDecReadDV(u4InstID, 61*4);

	vVDecWriteDV( u4InstID, 61*4, u4Value & 0xFFFFFFFE);
	u4Value = u4VDecReadDV(u4InstID, 61*4);

	mb();

    switch(u4VDecType)
    {        
    case VDEC_MPEG:
    case VDEC_MPEG1:
    case VDEC_MPEG2:
        u4VDecPDNCtrlSpec = 0x1FE;
        u4VDecPDNCtrlModule = 0x7F6A151D;
		u4VDecPDNCtrlModule2 = 0x7F;
        break;
		
    case VDEC_DIVX3:
    case VDEC_MPEG4:
    case VDEC_H263:         
        //u4VDecPDNCtrlSpec = 0xFD;
        //u4VDecPDNCtrlModule = 0x3E6A1108;

		u4VDecPDNCtrlSpec = 0x1FD;
        u4VDecPDNCtrlModule = 0x7E6A1108;
		u4VDecPDNCtrlModule2 = 0x5F;
        break;
		
    case VDEC_WMV:
    case VDEC_WMV1:
    case VDEC_WMV2:
        u4VDecPDNCtrlSpec = 0xFA;
        u4VDecPDNCtrlModule = 0x3E6A1108;
        break;
		
    case VDEC_WMV3:
    case VDEC_VC1:
        u4VDecPDNCtrlSpec = 0xFA;
        u4VDecPDNCtrlModule = 0x3E6A1108;
        break;
		
    case VDEC_H264:
        u4VDecPDNCtrlSpec = 0xF7;
        u4VDecPDNCtrlModule = 0x13A20100;
        break;
		
    case VDEC_VP6:
        break;
		
    case VDEC_AVS:
        break;
		
    case VDEC_VP8:
		u4VDecPDNCtrlSpec = 0x7F;
        u4VDecPDNCtrlModule = 0x31A01100; 
        break;

    case VDEC_UNKNOWN:
    default:
        u4VDecPDNCtrlSpec = 0x0;
        u4VDecPDNCtrlModule = 0x0; 
        break;
    }
 
	 vVDecWriteDV( u4InstID, 0xC8, u4VDecPDNCtrlSpec);
    vVDecWriteDV( u4InstID, 0xCC, u4VDecPDNCtrlModule);
	vVDecWriteDV( u4InstID, 0x178, u4VDecPDNCtrlModule2);
	vVDecWriteDV( u4InstID, 0xF4, 0);

    printk("i4VDEC_HAL_Common_Gcon_Enable_After_Complete, DV 0xC8=0x%x, 0xCC=0x%x, 0x84=0x%x\n", 
     u4VDecReadDV(u4InstID, 0xC8), u4VDecReadDV(u4InstID, 0xCC), u4VDecReadDV(u4InstID, 0x84));
    mb();
	return 0;
}


#if ((CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560) && CONFIG_DRV_FTS_SUPPORT)
INT32 vDEC_HAL_COMMON_ReadLBDResult(UINT32 ucMpvId, UINT32* u4YUpbound, 
    UINT32* u4YLowbound, UINT32* u4CUpbound, UINT32* u4CLowbound)
{
    UINT32 u4YResult, u4CResult;

    
    vVDECReadLetetrBoxDetResult(ucMpvId, &u4YResult, &u4CResult);
#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8580)
    *u4YUpbound = (u4YResult&0xFFF);
    *u4YLowbound = ((u4YResult >> 16)&0xFFF);
    *u4CUpbound = (u4CResult&0xFFF);
    *u4CLowbound = ((u4CResult >> 16)&0xFFF);
#else
    *u4YUpbound = (u4YResult&0x7FF);
    *u4YLowbound = ((u4YResult >> 16)&0x7FF);
    *u4CUpbound = (u4CResult&0x7FF);
    *u4CLowbound = ((u4CResult >> 16)&0x7FF);
#endif
    return 0;
}
#endif

#ifdef CAPTURE_ESA_LOG
UINT32 u4VDEC_HAL_Print_ESA(UINT32 u4InstID)
{
    printk("%s,%d,%d,%d,%d,%d,%d,%d,%d\n", 
        _bFileStr1[u4InstID][1],
        u4VDecReadMC(u4InstID, 0x770),
        u4VDecReadMC(u4InstID, 0x8B8),
        u4VDecReadMC(u4InstID, 0xA28),
        u4VDecReadMC(u4InstID, 0x28),
        u4VDecReadMC(u4InstID, 0x2C),
        u4VDecReadMC(u4InstID, 0x9E0),
        u4VDecReadMC(u4InstID, 0x9E4),
        (u4VDecReadDV(u4InstID, 0xF0) & 0x1));
    
    return 0;
}


UINT32 u4VDEC_HAL_Read_ESA(UINT32 u4InstID , UINT32 u4Temp)
{
    UINT32 u4Temp1;

   _u4ESAValue[u4InstID][0] = u4VDecReadMC(u4InstID, 0x770); //MC NBM_DLE_NUM
   _u4ESAValue[u4InstID][1] = u4VDecReadMC(u4InstID, 0x8B8); //MC ESA_REQ_DATA_NUM
   _u4ESAValue[u4InstID][2] = u4VDecReadMC(u4InstID, 0xA28);     //MC MC_REQ_DATA_NUM
   _u4ESAValue[u4InstID][3] = u4VDecReadMC(u4InstID, 0x28);     //MC MC_MBX
   _u4ESAValue[u4InstID][4] = u4VDecReadMC(u4InstID, 0x2C);     //MC MC_MBY
   _u4ESAValue[u4InstID][5] = u4VDecReadMC(u4InstID, 0x9E0);    //MC CYC_SYS 
   _u4ESAValue[u4InstID][6] = u4VDecReadMC(u4InstID, 0x9E4);     //MC INTRA_CNT
   _u4ESAValue[u4InstID][7] = u4VDecReadDV(u4InstID, 0xF0) & 0x1;    //VDEC_TOP LAT_BUF_BYPASS
    u4Temp1= u4Temp + sprintf (_pucESALog[u4InstID]+u4Temp, "%d,%d,%d,%d,%d,%d,%d,%d,\n", 
    	_u4ESAValue[u4InstID][0] ,
    	_u4ESAValue[u4InstID][1] ,
    	_u4ESAValue[u4InstID][2] ,
    	_u4ESAValue[u4InstID][3] ,
    	_u4ESAValue[u4InstID][4] ,
    	_u4ESAValue[u4InstID][5] ,
    	_u4ESAValue[u4InstID][6] ,
    	_u4ESAValue[u4InstID][7]);
    printk("u4Temp = %d\n", u4Temp1);
    return u4Temp1;
}

#endif

#ifdef VDEC_BREAK_EN
#include <linux/random.h>

BOOL fgBreakVDec(UINT32 u4InstID)
{	
   UINT32 u4Cnt=0;
   UINT32 u4Mbx,u4Mby;
   char cid[3];
       get_random_bytes(cid, sizeof(cid));
       if (_u4FileCnt[u4InstID] % 3 == 0)
            u4Cnt = (cid[0]) | (cid[1] << 8) | (cid[2] << 16);
       else if (_u4FileCnt[u4InstID] % 3 == 1)
            u4Cnt = (cid[0]) | (cid[1] << 8);
       else 
            u4Cnt = (cid[0]);
       	
  //     printk("u4Cnt = 0x%x, cid[0] = 0x%x, cid[1] = 0x%x, cid[2] = 0x%x\n", u4Cnt, cid[0], cid[1], cid[2]);
       if (!(u4VDecReadDV(u4InstID, VDEC_DV_DEC_BREAK)&0x01))
       {
         while(u4Cnt--);
         vVDecWriteDV(u4InstID, VDEC_DV_DEC_BREAK, u4VDecReadDV(u4InstID, VDEC_DV_DEC_BREAK) |VDEC_DV_DEC_BREAK_EN);
         u4Cnt=0;
         while (!((u4VDecReadDV(u4InstID, VDEC_DV_DEC_BREAK_STATUS)&VDEC_BREAK_OK_0) && (u4VDecReadDV(u4InstID, VDEC_DV_DEC_BREAK_STATUS) & VDEC_BREAK_OK_1)))
           {
             u4Cnt++;
             if (u4Cnt >= 0x5000)
             {
               printk("not break\n");
               break;
             }
           }
       }
       else
       {
         printk("[1] fgBreakVDec Fail\n");
         return FALSE;
       }
       	
       u4Mbx= u4VDecReadMC(u4InstID, RO_MC_MBX);
       u4Mby = u4VDecReadMC(u4InstID, RO_MC_MBY);

 //      printk("u4Mbx = 0x%x, u4Mby = 0x%x\n", u4Mbx, u4Mby);

    if (u4Cnt == 0x5000)
    {
      printk("[2] fgBreakVDec Fail\n");
      return FALSE;
    }
//    printk("Break done, Count = 0x%x\n", u4Cnt);
    printk("fgBreakVDec Pass\n");
    return TRUE;
}
#endif


#if VMMU_SUPPORT
void vPage_Table(UINT32 u4InstID, UINT32  page_addr, UINT32 start, UINT32 end) // page_addr = table_base + (start/4KB)*4
{
  int page_num;
  int temp;
  int i;

  temp = (end-start)%(0x1000);
  page_num = (end-start)/(0x1000); 
  if (temp > 0)
  	page_num++;

  for (i = 0; i < page_num; i++)  	{
  *(UINT32P)(page_addr +4*i) = start + 0x02 + (0x1000) * (i+1); //PA = offset VA by 4KB
  	}
 }

void enable_vmmu0(UINT32 table_base)
{
//  vVDecWriteMC(u4InstID, 0xF2098000, table_base);
//  vVDecWriteMC(u4InstID, 0xF2098080, table_base);
//  vVDecWriteMC(u4InstID, 0xF2098300, table_base);
  *(UINT32P)(0xF2098000) = table_base; // set page table base
  *(UINT32P)(0xF2098080) = 0x0; // set prefetch distance = 0
  *(UINT32P)(0xF2098300) = 0x00000001; //enable vmmu0
  *(UINT32P)(0xF2098210) = 0x00000001; //enable vmmu0
}

void enable_vmmu1(UINT32 table_base)
{
  *(UINT32P)(0xF2099000) = table_base; // set page table base
  *(UINT32P)(0xF2099080) = 0x0; // set prefetch distance = 0
  *(UINT32P)(0xF2099300) = 0x00000001; //enable vmmu1
  *(UINT32P)(0xF2099210) = 0x00000001; //enable vmmu1
}

void vVDecVMMUEnable(UINT32 table_base)
{
enable_vmmu0(table_base);
enable_vmmu1(table_base);
}

#endif