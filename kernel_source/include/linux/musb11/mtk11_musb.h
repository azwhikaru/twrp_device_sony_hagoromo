#ifndef __MUSB11_MTK_MUSB_H__
#define __MUSB11_MTK_MUSB_H__

#include <mach/mt_reg_base.h>

#define USB11PHY_READ8(offset)          readb((void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USB11PHY_WRITE8(offset, value)  writeb(value, (void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USB11PHY_SET8(offset, mask)     USB11PHY_WRITE8(offset, (USB11PHY_READ8(offset)) | (mask))
#define USB11PHY_CLR8(offset, mask)     USB11PHY_WRITE8(offset, (USB11PHY_READ8(offset)) & (~mask))

#define USB11PHY_READ16(offset)          readw((void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USB11PHY_WRITE16(offset, value)  writew(value, (void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USB11PHY_SET16(offset, mask)     USB11PHY_WRITE16(offset, (USB11PHY_READ16(offset)) | (mask))
#define USB11PHY_CLR16(offset, mask)     USB11PHY_WRITE16(offset, (USB11PHY_READ16(offset)) & (~mask))

#define USB11PHY_READ32(offset)          readl((void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USB11PHY_WRITE32(offset, value)  writel(value, (void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USB11PHY_SET32(offset, mask)     USB11PHY_WRITE32(offset, (USB11PHY_READ32(offset)) | (mask))
#define USB11PHY_CLR32(offset, mask)     USB11PHY_WRITE32(offset, (USB11PHY_READ32(offset)) & (~mask))

struct musb;

#if 0
typedef enum {
	USB_SUSPEND = 0,
	USB_UNCONFIGURED,
	USB_CONFIGURED
} usb_state_enum;
#endif

/* USB phy and clock */
extern void usb11_phy_poweron(void);
extern void usb11_phy_recover(void);
extern void usb11_phy_savecurrent(void);
extern void usb11_phy_context_restore(void);
extern void usb11_phy_context_save(void);
extern void usb11_enable_clock(bool enable);

/* general USB */
extern bool mt_usb_is_device(void);
extern void mt_usb11_connect(void);
extern void mt_usb11_disconnect(void);
/* ALPS00775710 */
/* extern bool usb_iddig_state(void); */
/* ALPS00775710 */
#if (defined(CONFIG_USB11_MTK_HDRC_GADGET))
extern bool usb_cable_connected(void);
#endif
//extern void pmic_chrdet_int_en(int is_on);
extern void musb11_platform_reset(struct musb *musb);
extern void musb11_sync_with_bat(struct musb *musb, int usb_state);

/* USB switch charger */
extern bool usb11_is_switch_charger(void);

/* host and otg */
extern void mt_usb11_otg_init(struct musb *musb);
extern void mt_usb_otg_exit(struct musb *musb);
extern void mt_usb11_init_drvvbus(void);
extern void mt_usb11_set_vbus(struct musb *musb, int is_on);
extern int mt_usb11_get_vbus_status(struct musb *musb);
extern void mt_usb11_iddig_int(struct musb *musb);
extern void usb11_switch_int_to_device(struct musb *musb);
extern void usb11_switch_int_to_host(struct musb *musb);
extern void usb11_switch_int_to_host_and_mask(struct musb *musb);
extern void musb11_session_restart(struct musb *musb);
#endif
