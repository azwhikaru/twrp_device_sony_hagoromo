#ifndef __LINUX_USB_NOP_XCEIV_H
#define __LINUX_USB_NOP_XCEIV_H

#include <linux/usb/otg.h>

struct nop_usb_xceiv_platform_data {
	enum usb_phy_type type;
	unsigned long clk_rate;

	/* if set fails with -EPROBE_DEFER if can't get regulator */
	unsigned int needs_vcc:1;
	unsigned int needs_reset:1;
};

#if defined(CONFIG_NOP_USB_XCEIV) || (defined(CONFIG_NOP_USB_XCEIV_MODULE) && defined(MODULE))
/* sometimes transceivers are accessed only through e.g. ULPI */
extern void usb_nop_xceiv_register(void);
extern void usb_nop_xceiv_unregister(void);
extern void mtk_usb_nop_xceiv_register(int id);
extern void mtk_usb_nop_xceiv_unregister(int id);
#else
static inline void usb_nop_xceiv_register(void)
{
}

static inline void usb_nop_xceiv_unregister(void)
{
}
#endif

#endif /* __LINUX_USB_NOP_XCEIV_H */