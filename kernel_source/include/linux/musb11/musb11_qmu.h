#ifndef _MUSB11_QMU_H_
#define _MUSB11_QMU_H_

#ifdef MUSB11_QMU_SUPPORT
#include "musb11_core.h"		/* for struct musb */

extern int musb11_qmu_init(struct musb *musb);
extern void musb11_qmu_exit(struct musb *musb); 
extern void musb11_kick_D_CmdQ(struct musb *musb, struct musb_request *request);
extern void musb11_disable_q_all(struct musb *musb);
extern irqreturn_t musb11_q_irq(struct musb *musb);
extern void musb11_flush_qmu(u32 ep_num,  u8 isRx);
extern void musb11_restart_qmu(struct musb *musb, u32 ep_num, u8 isRx);
extern bool musb11_is_qmu_stop(u32 ep_num, u8 isRx);
extern void musb11_tx_zlp_qmu(struct musb *musb, u32 ep_num);

/*FIXME, not good layer present */
extern void mtk_qmu11_enable(struct musb *musb, u8 EP_Num, u8 isRx);

#endif
#endif
