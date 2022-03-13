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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/sync_write.h>
#include <mach/mt_dcm.h>
#include <mach/mt_clkmgr.h>

/* support mtk in-house tee */
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
/* enable protection for SMI DCM due to DAPC protecting SMI control register */
#define SUPPORT_MTEE_SMI_DCM_PROT 1
#else
#define SUPPORT_MTEE_SMI_DCM_PROT 0
#endif

/* #define USING_XLOG */

#ifdef USING_XLOG

#include <linux/xlog.h>
#define TAG	"Power/dcm"

#define dcm_err(fmt, args...)	\
	xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
#define dcm_warn(fmt, args...)	\
	xlog_printk(ANDROID_LOG_WARN, TAG, fmt, ##args)
#define dcm_info(fmt, args...)	\
	xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
#define dcm_dbg(fmt, args...)	\
	xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define dcm_ver(fmt, args...)	\
	xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args)

#else /* !USING_XLOG */

#define TAG	"[Power/dcm] "

#define dcm_err(fmt, args...) 	pr_err(TAG fmt, ##args)
#define dcm_warn(fmt, args...)	pr_warn(TAG fmt, ##args)
#define dcm_info(fmt, args...)	pr_notice(TAG fmt, ##args)
#define dcm_dbg(fmt, args...)	pr_info(TAG fmt, ##args)
#define dcm_ver(fmt, args...)	pr_debug(TAG fmt, ##args)

#endif

#if SUPPORT_MTEE_SMI_DCM_PROT
#include "trustzone/kree/system.h"
#include "tz_cross/trustzone.h"
#include "tz_cross/ta_dcm.h"

/*
	control SMI DCM in MTEE
*/
int i4MTEE_SMI_DCM_Ctrl(unsigned int ui4_enable)
{
	TZ_RESULT ret;
	KREE_SESSION_HANDLE dcm_session;
	MTEEC_PARAM param[4];
	uint32_t cmd;
	uint32_t paramTypes;

	dcm_info("Ctrl SMI DCM in kernel (%d) - start\n", ui4_enable);

	if (0 == ui4_enable)
		cmd = TZCMD_DCM_DISABLE_DCM;
	else
		cmd = TZCMD_DCM_ENABLE_DCM;

	ret = KREE_CreateSession(TZ_TA_DCM_UUID, &dcm_session);
	if (ret != TZ_RESULT_SUCCESS) {
		dcm_info("Error: create dcm_session error %d\n", ret);
		return -1;
	}

	paramTypes = TZ_ParamTypes1(TZPT_VALUE_INPUT);

	param[0].value.a = SMI_DCM;
	ret = KREE_TeeServiceCall(dcm_session, cmd, paramTypes, param);
	if (TZ_RESULT_SUCCESS != ret)
		dcm_info("Error: fail to control SMI DCM in MTEE (%d)\n", ret);

	ret = KREE_CloseSession(dcm_session);
	if (ret != TZ_RESULT_SUCCESS) {
		dcm_info("Error: close dcm_session error %d\n", ret);
		return -1;
	}

	dcm_info("Ctrl SMI DCM in kernel (%d) - end\n", ui4_enable);
	return 0;
}

/*
	Dump SMI DCM register in MTEE
*/
int i4MTEE_SMI_DCM_GetStatus(
	unsigned int *pui4_smi_com_dcm,
	unsigned int *pui4_smi_sec_dcm,
	unsigned int *pui4_m4u_dcm)
{
	TZ_RESULT ret;
	KREE_SESSION_HANDLE dcm_session;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;

	dcm_info("Get SMI DCM status in kernel - start\n");

	if ((NULL == pui4_smi_com_dcm) ||
		(NULL == pui4_smi_sec_dcm) ||
		(NULL == pui4_m4u_dcm)) {
		dcm_warn("Error: NULL pointers for get SMI DCM status !!!\n");
		return -1;
	}

	ret = KREE_CreateSession(TZ_TA_DCM_UUID, &dcm_session);
	if (ret != TZ_RESULT_SUCCESS) {
		dcm_warn("Error: create dcm_session error %d\n", ret);
		return -1;
	}

	paramTypes = TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT);
	param[0].value.a = SMI_DCM;
	param[1].value.a = 0;
	param[1].value.b = 0;
	param[2].value.a = 0;

	ret = KREE_TeeServiceCall(dcm_session, TZCMD_DCM_GET_DCM_STATUS, paramTypes, param);
	if (TZ_RESULT_SUCCESS != ret)
		dcm_info("Error: fail to get status of SMI DCM in MTEE (%d)\n", ret);

	ret = KREE_CloseSession(dcm_session);
	if (ret != TZ_RESULT_SUCCESS) {
		dcm_info("Error: close dcm_session error %d\n", ret);
		return -1;
	}

	*pui4_smi_com_dcm = param[1].value.a;
	*pui4_smi_sec_dcm = param[1].value.b;
	*pui4_m4u_dcm     = param[2].value.a;

	dcm_info("Get SMI DCM status in kernel - done\n");
	return 0;
}

/*
	Dump SMI DCM operation register in MTEE
*/
int i4MTEE_SMI_DCM_GetOpStatus(
	unsigned int *pui4_smi_com_set,
	unsigned int *pui4_smi_com_clr)
{
	TZ_RESULT ret;
	KREE_SESSION_HANDLE dcm_session;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;

	dcm_info("Get SMI DCM op status in kernel - start\n");

	if ((NULL == pui4_smi_com_set) ||
		(NULL == pui4_smi_com_clr)) {
		dcm_warn("Error: NULL pointers for get SMI DCM op status !!!\n");
		return -1;
	}

	ret = KREE_CreateSession(TZ_TA_DCM_UUID, &dcm_session);
	if (ret != TZ_RESULT_SUCCESS) {
		dcm_warn("Error: create dcm_session error %d\n", ret);
		return -1;
	}

	paramTypes = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT);
	param[0].value.a = SMI_DCM;
	param[1].value.a = 0;
	param[1].value.b = 0;

	ret = KREE_TeeServiceCall(dcm_session, TZCMD_DCM_GET_DCM_OP_STATUS, paramTypes, param);
	if (TZ_RESULT_SUCCESS != ret)
		dcm_info("Error: fail to get op status of SMI DCM in MTEE (%d)\n", ret);

	ret = KREE_CloseSession(dcm_session);
	if (ret != TZ_RESULT_SUCCESS) {
		dcm_info("Error: close dcm_session error %d\n", ret);
		return -1;
	}

	*pui4_smi_com_set = param[1].value.a;
	*pui4_smi_com_clr = param[1].value.b;

	dcm_info("Get SMI DCM op status in kernel - done\n");
	return 0;
}
#endif /* SUPPORT_MTEE_SMI_DCM_PROT */

#ifndef BDP_DISPSYS_CONFIG_BASE
#define BDP_DISPSYS_CONFIG_BASE		0xfc000000
#endif

#define BDP_DISPSYS_HW_DCM_DIS0		(BDP_DISPSYS_CONFIG_BASE + 0x0120)
#define BDP_DISPSYS_HW_DCM_DIS_SET0	(BDP_DISPSYS_CONFIG_BASE + 0x0124)
#define BDP_DISPSYS_HW_DCM_DIS_CLR0	(BDP_DISPSYS_CONFIG_BASE + 0x0128)
#define BDP_DISPSYS_HW_DCM_DIS1		(BDP_DISPSYS_CONFIG_BASE + 0x012c)
#define BDP_DISPSYS_HW_DCM_DIS_SET1	(BDP_DISPSYS_CONFIG_BASE + 0x0130)
#define BDP_DISPSYS_HW_DCM_DIS_CLR1	(BDP_DISPSYS_CONFIG_BASE + 0x0124)

#ifndef SMILARB5_BASE
#define SMILARB5_BASE	0xf7002000
#endif

#define SMILARB5_DCM_STA		(SMILARB5_BASE + 0x00)
#define SMILARB5_DCM_CON		(SMILARB5_BASE + 0x10)
#define SMILARB5_DCM_SET		(SMILARB5_BASE + 0x14)
#define SMILARB5_DCM_CLR		(SMILARB5_BASE + 0x18)

#define IMG_HW_DCM_DIS0			(IMGSYS_CONFG_BASE + 0x0000)
#define IMG_HW_DCM_DIS_SET0		(IMGSYS_CONFG_BASE + 0x0004)
#define IMG_HW_DCM_DIS_CLR0		(IMGSYS_CONFG_BASE + 0x0008)

#define MSDC3_BASE2			0xf12C0000
#define MSDC3_IP_DCM			(MSDC3_BASE2 + 0x00B4)

#ifndef GENMASK
#define GENMASK(h, l)	(((1U << ((h) - (l) + 1)) - 1) << (l))
#endif

#define ALT_BITS(o, h, l, v) \
	(((o) & ~GENMASK(h, l)) | (((v) << (l)) & GENMASK(h, l)))

#define BIT(_bit_)		(u32)(1U << (_bit_))

#define dcm_readl(addr)		DRV_Reg32(addr)
#define dcm_writel(addr, val)	mt65xx_reg_sync_writel((val), ((void *)addr))
#define dcm_setl(addr, val)	dcm_writel(addr, dcm_readl(addr) | (val))
#define dcm_clrl(addr, val)	dcm_writel(addr, dcm_readl(addr) & ~(val))
#define dcm_alt_bits(addr, h, l, v) \
		dcm_writel(addr, ALT_BITS(dcm_readl(addr), h, l, v))

static DEFINE_MUTEX(dcm_lock);

static unsigned int dcm_sta;

static void pr_reg_val(const char *type, const char *regname, u32 addr, u32 val)
{
	dcm_info("[%12s] %-23s: [0x%08x]: 0x%08x\n",
		type, regname, addr, val);
}

static void pr_reg(const char *type, const char *regname, u32 addr)
{
	pr_reg_val(type, regname, addr, dcm_readl(addr));
}

#define PR_REG(t, r)		pr_reg(t, #r, r)
#define PR_REG_VAL(t, r, v)	pr_reg(t, #r, r, v)

void dcm_dump_regs(unsigned int type)
{
	mutex_lock(&dcm_lock);

	if (type & CPU_DCM) {
		PR_REG("CPUSYS_DCM", MCU_BIU_CON);
		PR_REG("CPUSYS_DCM", CA7_MISC_CONFIG);
	}

	if (type & TOPCKGEN_DCM) {
		PR_REG("TOPCKGEN_DCM", DCM_CFG);
		PR_REG("TOPCKGEN_DCM", CLK_SCP_CFG_0);
		PR_REG("TOPCKGEN_DCM", CLK_SCP_CFG_1);
	}

	if (type & IFR_DCM) {
		PR_REG("IFR_DCM", TOP_CKDIV1);
		PR_REG("IFR_DCM", TOP_DCMCTL);
		PR_REG("IFR_DCM", TOP_DCMDBC);
		PR_REG("IFR_DCM", INFRA_DCMCTL);
		PR_REG("IFR_DCM", INFRA_DCMDBC);
		PR_REG("IFR_DCM", INFRA_DCMFSEL);
		PR_REG("IFR_DCM", DRAMC_PD_CTRL);
	}

	if (type & PER_DCM) {
		PR_REG("PERI_DCM", PERI_GLOBALCON_DCMCTL);
		PR_REG("PERI_DCM", PERI_GLOBALCON_DCMDBC);
		PR_REG("PERI_DCM", PERI_GLOBALCON_DCMFSEL);
		PR_REG("PERI_DCM", MSDC0_IP_DCM);
		PR_REG("PERI_DCM", MSDC1_IP_DCM);
		PR_REG("PERI_DCM", MSDC2_IP_DCM);
		PR_REG("PERI_DCM", MSDC3_IP_DCM);
		PR_REG("PERI_DCM", PERI_USB0_DCM);
		PR_REG("PERI_DCM", PMIC_WRAP_DCM_EN);
		PR_REG("PERI_DCM", I2C0_I2CREG_HW_CG_EN);
		PR_REG("PERI_DCM", I2C1_I2CREG_HW_CG_EN);
		PR_REG("PERI_DCM", I2C2_I2CREG_HW_CG_EN);
	}

	if (type & SMI_DCM) {
		/* No SMI_COMMON_AO_SMI_* in MT8590 */
	}

	if (type & MFG_DCM) {
		if (subsys_is_on(SYS_MFG))
			PR_REG("MFG_DCM", MFG_DCM_CON_0);
		else
			dcm_info("[MFG_DCM] subsy MFG is off\n");
	}

	if (type & DIS_DCM) {
		if (subsys_is_on(SYS_DIS)) {
			PR_REG("DIS_DCM", DISP_HW_DCM_DIS0);
			PR_REG("DIS_DCM", DISP_HW_DCM_DIS_SET0);
			PR_REG("DIS_DCM", DISP_HW_DCM_DIS_CLR0);
			PR_REG("DIS_DCM", DISP_HW_DCM_DIS1);
			PR_REG("DIS_DCM", DISP_HW_DCM_DIS_SET1);
			PR_REG("DIS_DCM", DISP_HW_DCM_DIS_CLR1);
			PR_REG("DIS_DCM", SMILARB0_DCM_STA);
			PR_REG("DIS_DCM", SMILARB0_DCM_SET);
			PR_REG("DIS_DCM", SMILARB0_DCM_CLR);
		} else {
			dcm_info("[DIS_DCM] subsys DIS is off\n");
		}
	}

	if (type & ISP_DCM) {
		if (subsys_is_on(SYS_ISP))
			PR_REG("ISP_DCM", IMG_HW_DCM_DIS0);
		else
			dcm_info("[ISP_DCM] subsys ISP is off\n");
	}

	if (type & VDE_DCM) {
		if (subsys_is_on(SYS_VDE)) {
			PR_REG("VDE_DCM", VDEC_DCM_CON);
			PR_REG("VDE_DCM", SMILARB1_DCM_STA);
			PR_REG("VDE_DCM", SMILARB1_DCM_CON);
		} else {
			dcm_info("[VDE_DCM] subsys VDE is off\n");
		}
	}

	if (type & BDP_DCM) {
		if (subsys_is_on(SYS_BDP)) {
			PR_REG("BDP_DCM", BDP_DISPSYS_HW_DCM_DIS0);
			PR_REG("BDP_DCM", SMILARB5_DCM_STA);
			PR_REG("BDP_DCM", SMILARB5_DCM_CON);
		} else {
			dcm_info("[BDP_DCM] subsys BDP is off\n");
		}
	}

	mutex_unlock(&dcm_lock);
}

void dcm_enable(unsigned int type)
{
	unsigned int temp;

	dcm_info("[%s]type:0x%08x\n", __func__, type);

	mutex_lock(&dcm_lock);

	if (type & CPU_DCM) {
		dcm_info("[%s][CPU_DCM     ]=0x%08x\n", __func__, CPU_DCM);

		dcm_setl(MCU_BIU_CON, 0x1 << 12);
		dcm_setl(CA7_MISC_CONFIG, 0x1 << 9);
		dcm_sta |= CPU_DCM;
	}


	if (type & TOPCKGEN_DCM) {
		dcm_info("[%s][TOPCKGEN_DCM]=0x%08x\n", __func__, TOPCKGEN_DCM);

		dcm_setl(CLK_SCP_CFG_0, 0x3FF); /* set bit0~bit9=1, SCP control register 1 */
		dcm_setl(CLK_SCP_CFG_1, ((0x1 << 4) | 0x1)); /* set bit0=1 and bit4=1, SCP control register 1 */
		dcm_sta |= TOPCKGEN_DCM;
	}

	/* Infrasys_dcm */
	if (type & IFR_DCM) {
		dcm_info("[%s][IFR_DCM     ]=0x%08x\n", __func__, IFR_DCM);

		dcm_clrl(TOP_CKDIV1, 0x0000001f); /* 5'h0, 00xxx: 1/1 */
		dcm_setl(TOP_DCMCTL, 0x00000007); /* set bit0~bit2=1 */
		dcm_setl(TOP_DCMDBC, 0x00000001); /* set bit0=1, force to 26M */
		dcm_setl(INFRA_DCMCTL, 0x00000303); /* set bit0, bit1, bit8, bit9=1, DCM debouncing counter=0 */
		dcm_setl(INFRA_DCMDBC, 0x00000300); /* set bit8, bit9=1 first */
		dcm_clrl(INFRA_DCMDBC, 0x0000007F); /* then clear b0~b6 */

		dcm_writel(INFRA_DCMFSEL, 0xFFF0F0F8); /* clear bit0~bit2, clear bit8~bit11, set bit20=1 */

		dcm_sta |= IFR_DCM;
	}

	if (type & PER_DCM) {
		dcm_info("[%s][PER_DCM     ]=0x%08x\n", __func__, PER_DCM);

		dcm_clrl(PERI_GLOBALCON_DCMCTL, 0x00001F00); /* clear bit8~bit12=0 */
		dcm_setl(PERI_GLOBALCON_DCMCTL, 0x000000F3); /* set bit0, bit1, bit4~bit7=1 */

		dcm_setl(PERI_GLOBALCON_DCMDBC, 0x1<<7); /* set bit7=1 */
		dcm_clrl(PERI_GLOBALCON_DCMDBC, 0x0000007F); /* clear bit0~bit6=0 */

		dcm_clrl(PERI_GLOBALCON_DCMFSEL, 0x00000007); /* clear bit0~bit2 */
		dcm_clrl(PERI_GLOBALCON_DCMFSEL, 0x00000F00); /* clear bit8~bit11 */
		dcm_clrl(PERI_GLOBALCON_DCMFSEL, 0x001F0000); /* clear bit16~bit20 */

		/* MSDC module */
		dcm_clrl(MSDC0_IP_DCM, GENMASK(31, 20));
		dcm_clrl(MSDC1_IP_DCM, GENMASK(31, 20));
		dcm_clrl(MSDC2_IP_DCM, GENMASK(31, 20));
		dcm_clrl(MSDC3_IP_DCM, GENMASK(31, 18));

		/* USB */
		dcm_clrl(PERI_USB0_DCM, 0x00070000); /* clear bit16~bit18=0 */

		/* PMIC */
		dcm_setl(PMIC_WRAP_DCM_EN, 0x1); /* set bit0=1 */

		/* I2C */
		dcm_setl(I2C0_I2CREG_HW_CG_EN, 0x1); /* set bit0=1 */
		dcm_setl(I2C1_I2CREG_HW_CG_EN, 0x1); /* set bit0=1 */
		dcm_setl(I2C2_I2CREG_HW_CG_EN, 0x1); /* set bit0=1 */

		dcm_sta |= PER_DCM;
	}

	if (type & SMI_DCM) {
#if SUPPORT_MTEE_SMI_DCM_PROT
		int iret;
#endif  /* SUPPORT_MTEE_SMI_DCM_PROT */

		dcm_info("[%s][SMI_DCM     ]=0x%08x\n", __func__, SMI_DCM);

#if SUPPORT_MTEE_SMI_DCM_PROT
		/* SMI_SECURE_XXX register is protected by MTEE */
		/* Note: driver initialization should not call this function due to driver iniitializaion sequence */
		iret = i4MTEE_SMI_DCM_Ctrl(1);
#else
		/* smi_common */
		dcm_writel(SMI_DCM_CONTROL, 0x1); /* set bit 0=1 */

		/* m4u_dcm */
		dcm_setl(MMU_DCM, 0x1); /* set bit0=1 */
#endif  /* SUPPORT_MTEE_SMI_DCM_PROT */
		dcm_sta |= SMI_DCM;
	}

	if (type & MFG_DCM) {
		dcm_info("[%s][MFG_DCM     ]=0x%08x, subsys_is_on(SYS_MFG)=%d\n",
			__func__, MFG_DCM, subsys_is_on(SYS_MFG));

		if (subsys_is_on(SYS_MFG)) {
			temp = dcm_readl(MFG_DCM_CON_0);
			temp &= 0xFFFE0000; /* set B[0:6]=0111111, B[8:13]=0,, B[14]=1,, B[15]=1,, B[16]=0 */
			temp |= 0x0000C03F;
			dcm_writel(MFG_DCM_CON_0, temp);
			dcm_sta |= MFG_DCM;
		}
	}

	if (type & DIS_DCM) {
		dcm_info("[%s][DIS_DCM     ]=0x%08x, subsys_is_on(SYS_DIS)=%d\n",
			__func__, DIS_DCM, subsys_is_on(SYS_DIS));

		if (subsys_is_on(SYS_DIS)) {
			dcm_writel(DISP_HW_DCM_DIS_CLR0, GENMASK(31, 0));
			dcm_writel(DISP_HW_DCM_DIS_CLR1, GENMASK(31, 0));

			/* LARB0 „³ DISP, MDP */
			/* RO, bootup set once status = 1'b0, DCM off setting=N/A */
			dcm_readl(SMILARB0_DCM_STA);
			/* RO, bootup set once status = 1'b1, DCM off setting=1'b0 */
			dcm_readl(SMILARB0_DCM_CON);
			dcm_setl(SMILARB0_DCM_SET, 0x1<<15); /* set bit15=1 */
			/* N/A */
			dcm_readl(SMILARB0_DCM_CON);

			dcm_sta |= DIS_DCM;
		}
	}

	if (type & ISP_DCM) {
		dcm_info("[%s][ISP_DCM     ]=0x%08x, subsys_is_on(SYS_ISP)=%d\n",
			__func__, ISP_DCM, subsys_is_on(SYS_ISP));

		if (subsys_is_on(SYS_ISP)) {
			dcm_writel(IMG_HW_DCM_DIS_CLR0, GENMASK(31, 0));

			/* LARB2 „³ ISP, VENC */
			/* RO, bootup set once status = 1'b0, DCM off setting=N/A */
			dcm_readl(SMILARB2_DCM_STA);
			/* RO, bootup set once status = 1'b1, DCM off setting=1'b0 */
			dcm_readl(SMILARB2_DCM_CON);
			dcm_setl(SMILARB2_DCM_SET, 0x1<<15); /* set bit15=1 */
			/* N/A */
			dcm_readl(SMILARB2_DCM_CON);

			dcm_sta |= ISP_DCM;
		}
	}

	if (type & VDE_DCM) {
		dcm_info("[%s][VDE_DCM     ]=0x%08x, subsys_is_on(SYS_VDE)=%d\n",
			__func__, VDE_DCM, subsys_is_on(SYS_VDE));

		if (subsys_is_on(SYS_VDE)) {
			dcm_clrl(VDEC_DCM_CON, 0x1); /* clear bit0 */

			/* LARB1 „³ VDEC */
			/* RO, bootup set once status = 1'b0, DCM off setting=N/A */
			dcm_readl(SMILARB1_DCM_STA);
			/* RO, bootup set once status = 1'b1, DCM off setting=1'b0 */
			dcm_readl(SMILARB1_DCM_CON);
			dcm_setl(SMILARB1_DCM_SET, 0x1<<15); /* set bit15=1 */
			/* N/A */
			dcm_readl(SMILARB1_DCM_SET);

			dcm_sta |= VDE_DCM;
		}
	}

	if (type & BDP_DCM) {
		dcm_info("[%s][BDP_DCM     ]=0x%08x, subsys_is_on(SYS_BDP)=%d\n",
			__func__, BDP_DCM, subsys_is_on(SYS_BDP));

		if (subsys_is_on(SYS_BDP)) {
			dcm_writel(BDP_DISPSYS_HW_DCM_DIS_CLR0, GENMASK(31, 0));
			dcm_setl(SMILARB5_DCM_SET, BIT(15));

			dcm_sta |= BDP_DCM;
		}
	}

	mutex_unlock(&dcm_lock);
}

void dcm_disable(unsigned int type)
{
	dcm_info("[%s]type:0x%08x\n", __func__, type);

	mutex_lock(&dcm_lock);

	if (type & CPU_DCM) {
		dcm_info("[%s][CPU_DCM     ]=0x%08x\n", __func__, CPU_DCM);

		dcm_clrl(MCU_BIU_CON, 0x1 << 12); /* set bit12=0 */
		dcm_clrl(CA7_MISC_CONFIG, 0x1 << 9); /* set bit9=0 */
		dcm_sta &= ~CPU_DCM;
	}

	if (type & TOPCKGEN_DCM) {
		dcm_info("[%s][TOPCKGEN_DCM]=0x%08x\n", __func__, TOPCKGEN_DCM);

		dcm_setl(CLK_SCP_CFG_0, 0x3FF); /* set bit0~bit9=1, SCP control register 1 */
		dcm_setl(CLK_SCP_CFG_1, ((0x1 << 4) | 0x1)); /* set bit0=1 and bit4=1, SCP control register 1 */
		dcm_sta &= ~TOPCKGEN_DCM;
	}

	if (type & PER_DCM) {
		dcm_info("[%s][PER_DCM     ]=0x%08x\n", __func__, PER_DCM);

		dcm_clrl(PERI_GLOBALCON_DCMCTL, 0x00001F00); /* clear bit8~bit12=0 */
		dcm_clrl(PERI_GLOBALCON_DCMCTL, 0x000000F3); /* set bit0, bit1, bit4~bit7=0 */

		dcm_setl(PERI_GLOBALCON_DCMDBC, 0x1<<7); /* set bit7=1 */
		dcm_clrl(PERI_GLOBALCON_DCMDBC, 0x0000007F); /* clear bit0~bit6=0 */

		dcm_clrl(PERI_GLOBALCON_DCMFSEL, 0x00000007); /* clear bit0~bit2 */
		dcm_clrl(PERI_GLOBALCON_DCMFSEL, 0x00000F00); /* clear bit8~bit11 */
		dcm_clrl(PERI_GLOBALCON_DCMFSEL, 0x001F0000); /* clear bit16~bit20 */

		/* MSDC module */
		dcm_setl(MSDC0_IP_DCM, GENMASK(31, 20));
		dcm_setl(MSDC1_IP_DCM, GENMASK(31, 20));
		dcm_setl(MSDC2_IP_DCM, GENMASK(31, 20));
		dcm_setl(MSDC3_IP_DCM, GENMASK(31, 18));

		/* USB */
		dcm_setl(PERI_USB0_DCM, 0x00070000); /* set bit16~bit18=1 */

		/* PMIC */
		dcm_clrl(PMIC_WRAP_DCM_EN, 0x1); /* set bit0=0 */

		/* I2C */
		dcm_clrl(I2C0_I2CREG_HW_CG_EN, 0x1); /* set bit0=0 */
		dcm_clrl(I2C1_I2CREG_HW_CG_EN, 0x1); /* set bit0=0 */
		dcm_clrl(I2C2_I2CREG_HW_CG_EN, 0x1); /* set bit0=0 */

		dcm_sta &= ~PER_DCM;
	}

	/* Infrasys_dcm */
	if (type & IFR_DCM) {
		dcm_info("[%s][IFR_DCM     ]=0x%08x\n", __func__, IFR_DCM);

		dcm_clrl(TOP_DCMCTL, 0x00000006); /* clear bit1, bit2=0, bit0 doesn't need to clear */

		dcm_clrl(INFRA_DCMCTL, 0x00000303); /* set bit0, bit1, bit8, bit9=1, DCM debouncing counter=0 */

		dcm_sta &= ~IFR_DCM;
	}

	if (type & SMI_DCM) {
#if SUPPORT_MTEE_SMI_DCM_PROT
		int iret;
#endif  /* SUPPORT_MTEE_SMI_DCM_PROT */

		dcm_info("[%s][SMI_DCM     ]=0x%08x\n", __func__, SMI_DCM);

#if SUPPORT_MTEE_SMI_DCM_PROT
		/* SMI_SECURE_XXX register is protected by MTEE */
		iret = i4MTEE_SMI_DCM_Ctrl(0);
#else
		/* smi_common */
		dcm_clrl(SMI_DCM_CONTROL, 0x1); /* set bit0=0 */

		/* m4u_dcm */
		dcm_clrl(MMU_DCM, 0x1); /* set bit0=0 */
#endif /* SUPPORT_MTEE_SMI_DCM_PROT */
		dcm_sta &= ~SMI_DCM;
	}

	if (type & MFG_DCM) {
		dcm_info("[%s][MFG_DCM     ]=0x%08x\n", __func__, MFG_DCM);

		dcm_clrl(MFG_DCM_CON_0, 0x8000); /* disable dcm, clear bit 15 */

		dcm_sta &= ~MFG_DCM;
	}

	if (type & DIS_DCM) {
		dcm_info("[%s][DIS_DCM     ]=0x%08x\n", __func__, DIS_DCM);

		dcm_writel(DISP_HW_DCM_DIS_SET0, GENMASK(31, 0));
		dcm_writel(DISP_HW_DCM_DIS_SET1, GENMASK(31, 0));

		/* LARB0 „³ DISP, MDP */
		/* RO, bootup set once status = 1'b0, DCM off setting=N/A */
		dcm_readl(SMILARB0_DCM_STA);
		/* RO, bootup set once status = 1'b1, DCM off setting=1'b0 */
		dcm_readl(SMILARB0_DCM_CON);
		/* N/A */
		dcm_readl(SMILARB0_DCM_SET);
		dcm_setl(SMILARB0_DCM_CLR, (0x1 << 15)); /* set bit15=1 */

		dcm_sta &= ~DIS_DCM;
	}

	if (type & ISP_DCM) {
		dcm_info("[%s][ISP_DCM     ]=0x%08x\n", __func__, ISP_DCM);

		dcm_writel(IMG_HW_DCM_DIS_SET0, GENMASK(31, 0));

		/* LARB2 „³ ISP, VENC */
		/* RO, bootup set once status = 1'b0, DCM off setting=N/A */
		dcm_readl(SMILARB2_DCM_STA);
		/* RO, bootup set once status = 1'b1, DCM off setting=1'b0 */
		dcm_readl(SMILARB2_DCM_CON);
		/* N/A */
		dcm_readl(SMILARB2_DCM_SET);
		dcm_setl(SMILARB2_DCM_CLR, (0x1 << 15)); /* set bit15=1 */

		dcm_sta &= ~ISP_DCM;
	}

	if (type & VDE_DCM) {
		dcm_info("[%s][VDE_DCM     ]=0x%08x\n", __func__, VDE_DCM);

		dcm_setl(VDEC_DCM_CON, 0x1); /* set bit0=1 */

		/* LARB1 „³ VDEC */
		/* RO, bootup set once status = 1'b0, DCM off setting=N/A */
		dcm_readl(SMILARB1_DCM_STA);
		/* RO, bootup set once status = 1'b1, DCM off setting=1'b0 */
		dcm_readl(SMILARB1_DCM_CON);
		/* N/A */
		dcm_readl(SMILARB1_DCM_SET);
		dcm_setl(SMILARB1_DCM_CLR, (0x1 << 15)); /* set bit15=1 */

		dcm_sta &= ~VDE_DCM;
	}

	if (type & BDP_DCM) {
		dcm_info("[%s][BDP_DCM     ]=0x%08x, subsys_is_on(SYS_BDP)=%d\n",
			__func__, BDP_DCM, subsys_is_on(SYS_BDP));

		if (subsys_is_on(SYS_BDP)) {
			dcm_writel(BDP_DISPSYS_HW_DCM_DIS_SET0, GENMASK(31, 0));
			dcm_setl(SMILARB5_DCM_CLR, BIT(15));

			dcm_sta |= BDP_DCM;
		}
	}

	mutex_unlock(&dcm_lock);
}

void bus_dcm_enable(void)
{
	dcm_writel(DCM_CFG, 0x1 << 7 | 0xF);
}

void bus_dcm_disable(void)
{
	dcm_clrl(DCM_CFG, 0x1 << 7);
}

static unsigned int infra_dcm;

void disable_infra_dcm(void)
{
	infra_dcm = dcm_readl(INFRA_DCMCTL);
	dcm_clrl(INFRA_DCMCTL, 0x100);
}

void restore_infra_dcm(void)
{
	dcm_writel(INFRA_DCMCTL, infra_dcm);
}

static unsigned int peri_dcm;

void disable_peri_dcm(void)
{
	peri_dcm = dcm_readl(PERI_GLOBALCON_DCMCTL);
	dcm_clrl(PERI_GLOBALCON_DCMCTL, 0x1);
}

void restore_peri_dcm(void)
{
	dcm_writel(PERI_GLOBALCON_DCMCTL, peri_dcm);
}

#define dcm_attr(_name)                         \
static struct kobj_attribute _name##_attr = {   \
	.attr = {                                   \
		.name = __stringify(_name),             \
		.mode = 0644,                           \
	},                                          \
	.show = _name##_show,                       \
	.store = _name##_store,                     \
}

static const char *dcm_name[NR_DCMS] = {
	"     CPU_DCM",
	"     IFR_DCM",
	"     PER_DCM",
	"     SMI_DCM",
	"     MFG_DCM",
	"     DIS_DCM",
	"     ISP_DCM",
	"     VDE_DCM",
	"TOPCKGEN_DCM",
	"     BDP_DCM"
};

static ssize_t dcm_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int len = 0;
	char *p = buf;

	int i;
	unsigned int sta;

	p += sprintf(p, "********** dcm_state dump **********\n");
	mutex_lock(&dcm_lock);

	for (i = 0; i < NR_DCMS; i++) {
		sta = dcm_sta & (0x1 << i);
		p += sprintf(p, "[%d][%s]%s\n", i, dcm_name[i], sta ? "on" : "off");
	}

	mutex_unlock(&dcm_lock);

	p += sprintf(p, "\n********** dcm_state help *********\n");
	p += sprintf(p, "enable dcm:    echo enable mask(dec) > /sys/power/dcm_state\n");
	p += sprintf(p, "disable dcm:   echo disable mask(dec) > /sys/power/dcm_state\n");
	p += sprintf(p, "dump reg:      echo dump mask(dec) > /sys/power/dcm_state\n");

	len = p - buf;
	return len;
}

static ssize_t dcm_state_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	char cmd[10];
	unsigned int mask;

	if (sscanf(buf, "%s %x", cmd, &mask) == 2) {
		mask &= ALL_DCM;

		/*
		Need to enable MM clock before setting Smi_secure register
		to avoid system crash while screen is off(screen off with USB cable)
		*/
		enable_mux(MT_MUX_MM, "DCM");

		if (!strcmp(cmd, "enable")) {
			dcm_dump_regs(mask);
			dcm_enable(mask);
			dcm_dump_regs(mask);
		} else if (!strcmp(cmd, "disable")) {
			dcm_dump_regs(mask);
			dcm_disable(mask);
			dcm_dump_regs(mask);
		} else if (!strcmp(cmd, "dump"))
			dcm_dump_regs(mask);

		disable_mux(MT_MUX_MM, "DCM");

		return n;
	}

	return -EINVAL;
}
dcm_attr(dcm_state);

void mt_dcm_init(void)
{
	int err = 0;

	dcm_info("[%s]entry!!, ALL_DCM=%d\n", __func__, ALL_DCM);
#if SUPPORT_MTEE_SMI_DCM_PROT
	/*
	Note:
	 1. SMI_SECURE_XXX register is protected by MTEE
	    SMI_DCM is enabled by DCM driver in MTEE
	 2. Although initialization sequence for DCM kernel driver and MTEE driver is not guarantee in kernel,
	    it is ok to make status of SMI_DCM to be set to "enable" in DCM kernel driver initialization
	*/
	dcm_enable(ALL_DCM & (~SMI_DCM));
	dcm_sta |= SMI_DCM;
#else
	dcm_enable(ALL_DCM);
#endif

	err = sysfs_create_file(power_kobj, &dcm_state_attr.attr);

	if (err)
		dcm_err("[%s]: fail to create sysfs\n", __func__);
}
