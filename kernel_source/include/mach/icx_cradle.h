/*
 * Copyright 2016 Sony Corporation
 * File added and changed on 2016-01-20
 */
/*
 *  icx_cradle.h
 *
 *  Copyright 2009 Sony Corporation
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  version 2 of the  License.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#ifndef __ASM_ARM_ARCH_ICX_CRADLE_H
#define __ASM_ARM_ARCH_ICX_CRADLE_H

#include <asm/ioctl.h>

#define ICX_CRADLE_NAME "icx_cradle"
#define ICX_CRADLE_DEV  "/dev/" ICX_CRADLE_NAME

struct icx_cradle_driver_data {
    unsigned long acc_det_gpio;
    unsigned long acc_det_eint;
    unsigned long uart_tx_gpio;
    unsigned long uart_rx_gpio;
    int adc_ch_acc1;
    int adc_ch_acc2;
};

typedef struct _icx_cradle_state {
    int connect;                /* 0: disconnect, 1: connect */
    int id;                     /* ICX_CRADLE_ID_X */
    int id2;                    /* ICX_CRADLE_ID2_X */
    int count;                  /* detected count */
} icx_cradle_state_t;

#define ICX_CRADLE_IOC_MAGIC  'c'
#define ICX_CRADLE_GET_STATE _IOR(ICX_CRADLE_IOC_MAGIC,1,icx_cradle_state_t)
#define ICX_CRADLE_SET_SLEEP_SIGNAL _IOR(ICX_CRADLE_IOC_MAGIC,2,int)
#define ICX_CRADLE_GET_SLEEP_SIGNAL _IOR(ICX_CRADLE_IOC_MAGIC,3,int)

/* for compatible */
#define ICX_CRADLE_DISABLE   _IO( ICX_CRADLE_IOC_MAGIC,2)
#define ICX_CRADLE_ENABLE    _IO( ICX_CRADLE_IOC_MAGIC,3)

#define ICX_CRADLE_ID_0  0x01
#define ICX_CRADLE_ID_1  0x02
#define ICX_CRADLE_ID_2  0x03
#define ICX_CRADLE_ID_3  0x04
#define ICX_CRADLE_ID_4  0x05
#define ICX_CRADLE_ID_5  0x06
#define ICX_CRADLE_ID_6  0x11
#define ICX_CRADLE_ID_7  0x12
#define ICX_CRADLE_ID_8  0x13
#define ICX_CRADLE_ID_9  0x14
#define ICX_CRADLE_ID_A  0x15
#define ICX_CRADLE_ID_B  0xFF

#define ICX_CRADLE_ID2_0  0x01
#define ICX_CRADLE_ID2_1  0x02
#define ICX_CRADLE_ID2_2  0x03
#define ICX_CRADLE_ID2_3  0x04
#define ICX_CRADLE_ID2_4  0x05
#define ICX_CRADLE_ID2_5  0x06
#define ICX_CRADLE_ID2_6  0x11
#define ICX_CRADLE_ID2_7  0x12
#define ICX_CRADLE_ID2_8  0x13
#define ICX_CRADLE_ID2_9  0x14
#define ICX_CRADLE_ID2_A  0x15
#define ICX_CRADLE_ID2_B  0xFF

void icx_cradle_register_event_notifier(void (*func)(void));
int  icx_cradle_get_accessary_status(icx_cradle_state_t * state);

#endif
