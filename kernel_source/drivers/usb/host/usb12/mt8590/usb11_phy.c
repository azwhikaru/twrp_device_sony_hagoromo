#include <mach/mt_clkmgr.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/musb11/mtk11_musb.h>
#include "usb11.h"

#define FRA (48)
#define PARA (28)

extern void enable_phy_clock(bool enable);

#ifdef FPGA_PLATFORM
bool usb11_enable_clock(bool enable)
{
	return true;
}

void usb11_phy_poweron(void)
{
}

void usb11_phy_savecurrent(void)
{
}

void usb11_phy_recover(void)
{
}

// BC1.2
#if 0
void Charger_Detect_Init(void)
{
}


void Charger_Detect_Release(void)
{
}
#endif

void usb11_phy_context_save(void)
{
}

void usb11_phy_context_restore(void)
{
}

#ifdef MTK_UART_USB_SWITCH
bool usb11_phy_check_in_uart_mode()
{
	UINT8 usb_port_mode;
    usb11_enable_clock(true);
    udelay(50);

	usb_port_mode = USB11_PHY_Read_Register8(0x6B);
    usb11_enable_clock(false);

	if ((usb_port_mode == 0x5C) ||
		(usb_port_mode == 0x5E))
		return true;
	else
		return false;
}

void usb11_phy_switch_to_uart(void)
{
	int var;
#if 0
	//SW disconnect
	var = USB11_PHY_Read_Register8(0x68);
	printk("[MUSB]addr: 0x68, value: %x\n", var);
	USB11_PHY_Write_Register8(0x15, 0x68);
	printk("[MUSB]addr: 0x68, value after: %x\n", USB11_PHY_Read_Register8(0x68));

	var = USB11_PHY_Read_Register8(0x6A);
	printk("[MUSB]addr: 0x6A, value: %x\n", var);
	USB11_PHY_Write_Register8(0x0, 0x6A);
	printk("[MUSB]addr: 0x6A, value after: %x\n", USB11_PHY_Read_Register8(0x6A));
	//SW disconnect
#endif
	/* Set ru_uart_mode to 2'b01 */
	var = USB11_PHY_Read_Register8(0x6B);
	printk("[MUSB]addr: 0x6B, value: %x\n", var);
	USB11_PHY_Write_Register8(var | 0x7C, 0x6B);
	printk("[MUSB]addr: 0x6B, value after: %x\n", USB11_PHY_Read_Register8(0x6B));

	/* Set RG_UART_EN to 1 */
	var = USB11_PHY_Read_Register8(0x6E);
	printk("[MUSB]addr: 0x6E, value: %x\n", var);
	USB11_PHY_Write_Register8(var | 0x07, 0x6E);
	printk("[MUSB]addr: 0x6E, value after: %x\n", USB11_PHY_Read_Register8(0x6E));

	/* Set RG_USB20_DM_100K_EN to 1 */
	var = USB11_PHY_Read_Register8(0x22);
	printk("[MUSB]addr: 0x22, value: %x\n", var);
	USB11_PHY_Write_Register8(var | 0x02, 0x22);
	printk("[MUSB]addr: 0x22, value after: %x\n", USB11_PHY_Read_Register8(0x22));

	var = DRV_Reg8(UART1_BASE + 0x90);
	printk("[MUSB]addr: 0x11002090 (UART1), value: %x\n", var);
	DRV_WriteReg8(UART1_BASE + 0x90, var | 0x01);
	printk("[MUSB]addr: 0x11002090 (UART1), value after: %x\n\n", DRV_Reg8(UART1_BASE + 0x90));

	//SW disconnect
	mt_usb11_disconnect();
}

void usb11_phy_switch_to_usb(void)
{
	int var;
	/* Set RG_UART_EN to 0 */
	var = USB11_PHY_Read_Register8(0x6E);
	printk("[MUSB]addr: 0x6E, value: %x\n", var);
	USB11_PHY_Write_Register8(var & ~0x01, 0x6E);
	printk("[MUSB]addr: 0x6E, value after: %x\n", USB11_PHY_Read_Register8(0x6E));

	/* Set RG_USB20_DM_100K_EN to 0 */
	var = USB11_PHY_Read_Register8(0x22);
	printk("[MUSB]addr: 0x22, value: %x\n", var);
	USB11_PHY_Write_Register8(var & ~0x02, 0x22);
	printk("[MUSB]addr: 0x22, value after: %x\n", USB11_PHY_Read_Register8(0x22));

	var = DRV_Reg8(UART1_BASE + 0x90);
	printk("[MUSB]addr: 0x11002090 (UART1), value: %x\n", var);
	DRV_WriteReg8(UART1_BASE + 0x90, var & ~0x01);
	printk("[MUSB]addr: 0x11002090 (UART1), value after: %x\n\n", DRV_Reg8(UART1_BASE + 0x90));
#if 0
	//SW connect
	var = USB11_PHY_Read_Register8(0x68);
	printk("[MUSB]addr: 0x68, value: %x\n", var);
	USB11_PHY_Write_Register8(0x0, 0x68);
	printk("[MUSB]addr: 0x68, value after: %x\n", USB11_PHY_Read_Register8(0x68));

	var = USB11_PHY_Read_Register8(0x6A);
	printk("[MUSB]addr: 0x6A, value: %x\n", var);
	USB11_PHY_Write_Register8(0x0, 0x6A);
	printk("[MUSB]addr: 0x6A, value after: %x\n", USB11_PHY_Read_Register8(0x6A));
	//SW connect
#endif
	//SW connect
	mt_usb11_connect();
}
#endif

#else

#ifdef MTK_UART_USB_SWITCH
static bool in_uart_mode = false;
#endif

static DEFINE_SPINLOCK(musb11_reg_clock_lock);

void usb11_enable_clock(bool enable)
{
	unsigned long flags;
	static int count1;

	spin_lock_irqsave(&musb11_reg_clock_lock, flags);
	if(enable && count1 == 0){
		enable_phy_clock(true);				
        enable_clock(MT_CG_PERI_USB0, "PERI_USB");  		
		enable_clock(MT_CG_PERI_USB1, "USB11");
	}else if (!enable && count1 == 1) {
         disable_clock(MT_CG_PERI_USB1, "USB11");
         disable_clock(MT_CG_PERI_USB0, "PERI_USB");   
		 enable_phy_clock(false);
	}
	if (enable)
		count1++;
	else
		count1 = (count1 == 0) ? 0 : (count1 - 1);
	spin_unlock_irqrestore(&musb11_reg_clock_lock, flags);

	printk(KERN_DEBUG "usb11_enable_clock enable(%d) out\n", enable);
	return;
}

static void usb11_hs_slew_rate_cal(void){
	  unsigned long data;
	  unsigned long x;
	  unsigned char value;
	  unsigned long start_time, timeout;
	  unsigned int timeout_flag = 0;
	  //4 s1:enable usb ring oscillator.
	  USB11PHY_WRITE8(0x15,0x80);
	  
	  //4 s2:wait 1us.
	  udelay(1);
	  
	  //4 s3:enable free run clock
	  USB11PHY_WRITE8 (0xf00-0x900+0x11,0x01);
	  //4 s4:setting cyclecnt.
	  USB11PHY_WRITE8 (0xf00-0x900+0x01,0x04);
	  //4 s5:enable frequency meter
	  USB11PHY_SET8 (0xf00-0x900+0x03,0x05);
	  
	  //4 s6:wait for frequency valid.
	  start_time = jiffies;
	  timeout = jiffies + 3 * HZ;
	  while(!(USB11PHY_READ8(0xf00-0x900+0x10)&0x1)){
		if(time_after(jiffies, timeout)){
			timeout_flag = 1;
			break;
			}
		}
		
	  //4 s7: read result.
	  if(timeout_flag){
		printk("[USBPHY] Slew Rate Calibration: Timeout\n");
		value = 0x4;
		}
	  else{
		  data = USB11PHY_READ32 (0xf00-0x900+0x0c);
		  x = ((1024*FRA*PARA)/data);
		  value = (unsigned char)(x/1000);
		  if((x-value*1000)/100>=5)
			value += 1;
			// printk("[USB11PHY]slew calibration:FM_OUT =%d, x=%d,value=%d\n",data,x,value);
		}
	   
	  //4 s8: disable Frequency and run clock.
	  USB11PHY_CLR8 (0xf00-0x900+0x03,0x05);//disable frequency meter
	  USB11PHY_CLR8 (0xf00-0x900+0x11,0x01);//disable free run clock
	  
	  //4 s9: 
	  USB11PHY_WRITE8(0x15,value<<4);
	  
	  //4 s10:disable usb ring oscillator.
	  USB11PHY_CLR8(0x15,0x80);  

#if 0
  unsigned long data;
  unsigned long x;
  unsigned char value;
  unsigned long start_time, timeout;
  unsigned int timeout_flag = 0;
  //4 s1:enable usb ring oscillator.
  USB11PHY_WRITE8(0x15,0x80);

  //4 s2:wait 1us.
  udelay(1);

  //4 s3:enable free run clock
  USB11PHY_WRITE8 (0xf00-0x800+0x11,0x01);
  //4 s4:setting cyclecnt.
  USB11PHY_WRITE8 (0xf00-0x800+0x01,0x04);
  //4 s5:enable frequency meter
  USB11PHY_SET8 (0xf00-0x800+0x03,0x01);

  //4 s6:wait for frequency valid.
  start_time = jiffies;
  timeout = jiffies + 3 * HZ;

  while(!(USB11PHY_READ8(0xf00-0x800+0x10)&0x1)){
    if(time_after(jiffies, timeout)){
        timeout_flag = 1;
        break;
        }
    }

  //4 s7: read result.
  if(timeout_flag){
    printk("[USBPHY] Slew Rate Calibration: Timeout\n");
    value = 0x4;
    }
  else{
      data = USB11PHY_READ32 (0xf00-0x800+0x0c);
      x = ((1024*FRA*PARA)/data);
      value = (unsigned char)(x/1000);
      if((x-value*1000)/100>=5)
        value += 1;
        printk("[USBPHY]slew calibration:FM_OUT =%lu,x=%lu,value=%d\n",data,x,value);
    }

  //4 s8: disable Frequency and run clock.
  USB11PHY_CLR8 (0xf00-0x800+0x03,0x01);//disable frequency meter
  USB11PHY_CLR8 (0xf00-0x800+0x11,0x01);//disable free run clock

  //4 s9:
  USB11PHY_WRITE8(0x15,value<<4);

  //4 s10:disable usb ring oscillator.
  USB11PHY_CLR8(0x15,0x80);
  #endif
}

#ifdef MTK_UART_USB_SWITCH
bool usb11_phy_check_in_uart_mode()
{
	UINT8 usb_port_mode;

    usb11_enable_clock(true);
    udelay(50);
	usb_port_mode = USB11PHY_READ8(0x6B);
    usb11_enable_clock(false);

	if ((usb_port_mode == 0x5C) ||
		(usb_port_mode == 0x5E))
		return true;
	else
		return false;
}

void usb11_phy_switch_to_uart(void)
{
	if (usb11_phy_check_in_uart_mode()) {
		return;
	}
	//ALPS00775710
	printk("%s, line %d: force to uart mode!!\n", __func__, __LINE__);
	//ALPS00775710

    usb11_enable_clock(true);
	udelay(50);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USB11PHY_CLR8(0x1a, 0x80);

	/* Set RG_SUSPENDM to 1 */
	USB11PHY_SET8(0x68, 0x08);

	/* force suspendm = 1 */
	USB11PHY_SET8(0x6a, 0x04);

	/* Set ru_uart_mode to 2'b01 */
	USB11PHY_SET8(0x6B, 0x5C);

	/* Set RG_UART_EN to 1 */
	USB11PHY_SET8(0x6E, 0x07);

	/* Set RG_USB20_DM_100K_EN to 1 */
	USB11PHY_SET8(0x22, 0x02);
    usb11_enable_clock(false);

	/* GPIO Selection */
	DRV_WriteReg32(UART1_BASE + 0x90, 0x1);	//set
}


void usb11_phy_switch_to_usb(void)
{
	/* GPIO Selection */
	DRV_WriteReg32(UART1_BASE + 0x90, 0x0);	//set

    usb11_enable_clock(true);
    udelay(50);
	/* clear force_uart_en */
	USB11PHY_WRITE8(0x6B, 0x00);
    usb11_enable_clock(false);
	usb11_phy_poweron();
	/* disable the USB clock turned on in usb11_phy_poweron() */
    usb11_enable_clock(false);
}
#endif

void usb11_phy_poweron(void){

	#ifdef MTK_UART_USB_SWITCH
	if (usb11_phy_check_in_uart_mode())
		return;
	#endif

    //4 s1: enable USB MAC clock.
    usb11_enable_clock(true);

    //4 s2: wait 50 usec for PHY3.3v/1.8v stable.
    udelay(50);

    //4 s3: swtich to USB function. (system register, force ip into usb mode.
    USB11PHY_CLR8(0x6b, 0x04);
    USB11PHY_CLR8(0x6e, 0x01);

    //4 s4: RG_USB20_BC11_SW_EN 1'b0
    USB11PHY_CLR8(0x1a, 0x80);

    //5 s5: RG_USB20_DP_100K_EN 1'b0, RG_USB20_DM_100K_EN 1'b0
    USB11PHY_CLR8(0x22, 0x03);

    //6 s6: release force suspendm.
    USB11PHY_CLR8(0x6a, 0x04);

    //7 s7: wait for 800 usec.
    udelay(800);

	#ifdef MUSB11_ID_PIN_USE_EX_EINT
	// force enter device mode
    USB11PHY_CLR8(0x6c, 0x10);
    USB11PHY_SET8(0x6c, 0x2E);
    USB11PHY_SET8(0x6d, 0x3E);
	#endif
	
    printk("usb power on success\n");
}

#ifdef MTK_UART_USB_SWITCH
static bool skipDisableUartMode = true;
#endif

static void usb11_phy_savecurrent_internal(void){

    //4 1. swtich to USB function. (system register, force ip into usb mode.

	#ifdef MTK_UART_USB_SWITCH
	if (!usb11_phy_check_in_uart_mode()) {
		//4 s1: enable USB MAC clock.
		usb11_enable_clock(true);

		//4 s2: wait 50 usec for PHY3.3v/1.8v stable.
		udelay(50);

		USB11PHY_CLR8(0x6b, 0x04);
		USB11PHY_CLR8(0x6e, 0x01);

		//4 2. release force suspendm.
		USB11PHY_CLR8(0x6a, 0x04);
		usb11_enable_clock(false);
	} else {
		if (skipDisableUartMode)
			skipDisableUartMode = false;
		else
			return;
	}
	#else
    USB11PHY_CLR8(0x6b, 0x04);
    USB11PHY_CLR8(0x6e, 0x01);

    //4 2. release force suspendm.
    USB11PHY_CLR8(0x6a, 0x04);
	#endif

    //4 3. RG_DPPULLDOWN./RG_DMPULLDOWN.
    USB11PHY_SET8(0x68, 0xc0);
    //4 4. RG_XCVRSEL[1:0] =2'b01.
    USB11PHY_CLR8(0x68, 0x30);
    USB11PHY_SET8(0x68, 0x10);
    //4 5. RG_TERMSEL = 1'b1
	USB11PHY_SET8(0x68, 0x04);
    //4 6. RG_DATAIN[3:0]=4'b0000
	USB11PHY_CLR8(0x69, 0x3c);
    //4 7.force_dp_pulldown, force_dm_pulldown, force_xcversel,force_termsel.
    USB11PHY_SET8(0x6a, 0xba);

    //4 8.RG_USB20_BC11_SW_EN 1'b0
    USB11PHY_CLR8(0x1a, 0x80);
    //4 9.RG_USB20_OTG_VBUSSCMP_EN 1'b0
    USB11PHY_CLR8(0x1a, 0x10);
    //4 10. delay 800us.
    udelay(800);
    //4 11. rg_usb20_pll_stable = 1
    USB11PHY_SET8(0x63, 0x02);

//ALPS00427972, implement the analog register formula
    printk("%s: USB11PHY_READ8(0x05) = 0x%x \n", __func__, USB11PHY_READ8(0x05));
    printk("%s: USB11PHY_READ8(0x07) = 0x%x \n", __func__, USB11PHY_READ8(0x07));
//ALPS00427972, implement the analog register formula

    udelay(1);
    //4 12.  force suspendm = 1.
    USB11PHY_SET8(0x6a, 0x04);
    //4 13.  wait 1us
    udelay(1);

	#ifdef MUSB11_ID_PIN_USE_EX_EINT
	// force enter device mode
    USB11PHY_CLR8(0x6c, 0x10);
    USB11PHY_SET8(0x6c, 0x2E);
    USB11PHY_SET8(0x6d, 0x3E);
	#endif
}

void usb11_phy_savecurrent(void){

    usb11_phy_savecurrent_internal();

    //4 14. turn off internal 48Mhz PLL.    
    usb11_enable_clock(false);
    printk("usb11 save current success\n");
}

void usb11_phy_recover(void){

    //4 1. turn on USB reference clock.
    usb11_enable_clock(true);
    //4 2. wait 50 usec.
    udelay(50);

	#ifdef MTK_UART_USB_SWITCH
	if (!usb11_phy_check_in_uart_mode()) {
    // clean PUPD_BIST_EN
    // PUPD_BIST_EN = 1'b0
    // PMIC will use it to detect charger type
    USB11PHY_CLR8(0x1d, 0x10);

    //4 3. force_uart_en = 1'b0
    USB11PHY_CLR8(0x6b, 0x04);
    //4 4. RG_UART_EN = 1'b0
    USB11PHY_CLR8(0x6e, 0x1);
	//4 5. release force suspendm.
    USB11PHY_CLR8(0x6a, 0x04);

	skipDisableUartMode = false;
	} else {
		if (!skipDisableUartMode)
			return;
	}
	#else
    // clean PUPD_BIST_EN
    // PUPD_BIST_EN = 1'b0
    // PMIC will use it to detect charger type
    USB11PHY_CLR8(0x1d, 0x10);

    //4 3. force_uart_en = 1'b0
    USB11PHY_CLR8(0x6b, 0x04);
    //4 4. RG_UART_EN = 1'b0
    USB11PHY_CLR8(0x6e, 0x1);
    //4 5. force_uart_en = 1'b0
    USB11PHY_CLR8(0x6a, 0x04);
	#endif

    //4 6. RG_DPPULLDOWN = 1'b0
    USB11PHY_CLR8(0x68, 0x40);
    //4 7. RG_DMPULLDOWN = 1'b0
    USB11PHY_CLR8(0x68, 0x80);
    //4 8. RG_XCVRSEL = 2'b00
    USB11PHY_CLR8(0x68, 0x30);
    //4 9. RG_TERMSEL = 1'b0
    USB11PHY_CLR8(0x68, 0x04);
    //4 10. RG_DATAIN[3:0] = 4'b0000
    USB11PHY_CLR8(0x69, 0x3c);
   //4 11. force_dp_pulldown = 1b'0
    USB11PHY_CLR8(0x6a, 0x10);
    //4 12. force_dm_pulldown = 1b'0
    USB11PHY_CLR8(0x6a, 0x20);
    //4 13. force_xcversel = 1b'0
    USB11PHY_CLR8(0x6a, 0x08);
    //4 14. force_termsel = 1b'0
    USB11PHY_CLR8(0x6a, 0x02);
    //4 15. force_datain = 1b'0
    USB11PHY_CLR8(0x6a, 0x80);
    //4 16. RG_USB20_BC11_SW_EN 1'b0
    USB11PHY_CLR8(0x1a, 0x80);
    //4 17. RG_USB20_OTG_VBUSSCMP_EN 1'b1
    USB11PHY_SET8(0x1a, 0x10);
    //4 18. wait 800 usec.
    udelay(800);

   #ifdef MUSB11_ID_PIN_USE_EX_EINT
   // force enter device mode
   USB11PHY_CLR8(0x6c, 0x10);
   USB11PHY_SET8(0x6c, 0x2E);
   USB11PHY_SET8(0x6d, 0x3E);
   #endif
    usb11_hs_slew_rate_cal();
    printk("USB11 HW reg: index14=0x%x\n", get_devinfo_with_index(14));
    if (get_devinfo_with_index(14) & (0x01<<22)){
		USB11PHY_CLR8(0x00, 0x20);
		printk("USB11 HW reg: write RG_USB20_INTR_EN 0x%x\n", USB11PHY_READ8(0x00));
	}
	if (get_devinfo_with_index(14) & (0x07<<19)) {
		//RG_USB20_VRT_VREF_SEL[2:0]=5 (ori:4) (0x11110804[14:12])
		USB11PHY_CLR8(0x05, 0x70);
		USB11PHY_SET8(0x05, ((get_devinfo_with_index(14)>>19)<<4)&0x70);
		printk("USB11 HW reg: overwrite RG_USB20_VRT_VREF_SEL 0x%x\n", USB11PHY_READ8(0x05));
	}
    printk("usb11 recovery success\n");
    return;
}

// BC1.2
#if 0
void Charger_Detect_Init(void)
{
#if 0
    //turn on USB reference clock.
    usb11_enable_clock(true);
    //wait 50 usec.
    udelay(50);
    /* RG_USB20_BC11_SW_EN = 1'b1 */
    USB11PHY_SET8(0x1a, 0x80);
    printk("Charger_Detect_Init\n");
#endif
}


void Charger_Detect_Release(void)
{
#if 0
    /* RG_USB20_BC11_SW_EN = 1'b0 */
    USB11PHY_CLR8(0x1a, 0x80);
    udelay(1);
    //4 14. turn off internal 48Mhz PLL.
    usb11_enable_clock(false);
    printk("Charger_Detect_Release\n");
#endif
}
#endif

void usb11_phy_context_save(void)
{
#ifdef MTK_UART_USB_SWITCH
	in_uart_mode = usb11_phy_check_in_uart_mode();
#endif
}

void usb11_phy_context_restore(void)
{
#ifdef MTK_UART_USB_SWITCH
	if (in_uart_mode) {
		usb11_phy_switch_to_uart();
	}
#endif
    usb11_phy_savecurrent_internal();
}

#endif
